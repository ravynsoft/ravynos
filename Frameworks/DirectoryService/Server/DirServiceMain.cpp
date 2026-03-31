/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*!
 * @header DirServiceMain
 */

#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>		// for file and dir stat calls
#include <sys/mount.h>		// for file system check
#include <sys/wait.h>		// for waitpid() et al
#include <sys/resource.h>	// for getrlimit()
#include <fcntl.h>			// for open() and O_* flags
#include <paths.h>			// for _PATH_DEVNULL
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>			// for signal handling
#include <time.h>
#include <sys/syscall.h>
#include <sys/kauth.h>

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <sys/sysctl.h>				// for struct kinfo_proc and sysctl()
#include <syslog.h>					// for syslog()
#include <asl.h>
#include <vproc.h>

#define USE_SYSTEMCONFIGURATION_PUBLIC_APIS
#include <SystemConfiguration/SystemConfiguration.h>	//required for the configd kicker operation
#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/IOPMLib.h>		//required for power management handling
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_init.h>
#include <mach/port.h>
#include <dispatch/dispatch.h>
#include "DirServicesConstPriv.h"

#include "DirServiceMain.h"
#include "ServerControl.h"
#include "CLog.h"
#include "PrivateTypes.h"
#include "DirServicesPriv.h"
#include "DirServicesConst.h"
#include "CPlugInList.h"
#include "CHandlers.h"

#include "DirServicesTypes.h"
#include "COSUtils.h"
#include "od_passthru.h"

#define kDSPIDFile			"/var/run/DirectoryService.pid"
#define kDSRunningFile		"/Library/Preferences/DirectoryService/.DSIsRunning"

using namespace std;

typedef CFTypeRef _XSEventPortCreate( CFAllocatorRef allocator );
typedef int _XSEventPortPostEvent( CFTypeRef inPortRef, CFStringRef inEventType, CFDictionaryRef inEventData );

static _XSEventPortPostEvent *_xsEventPortPostEvent = NULL;
static _XSEventPortCreate *_xsEventPortCreate		= NULL;

bool	gCacheFlushDisabled		= false;
dsBool	gServerOS				= false;	//indicates whether this is running on Server or not
bool	gLogAPICalls			= false;
bool	gDebugLogging			= false;
dsBool	gDSFWCSBPDebugLogging   = false;
bool	gIgnoreSunsetTime		= false;
dsBool	gDSDebugMode			= false;
dsBool	gDSLocalOnlyMode		= false;
dsBool	gDSInstallDaemonMode	= false;
dsBool	gProperShutdown			= false;
dsBool	gSafeBoot				= false;
CFTypeRef		gEventPort		= NULL;
uint32_t	gNumberOfCores		= 0;

//Used for Power Management
io_object_t			gPMDeregisterNotifier;
io_connect_t		gPMKernelPort;
CFRunLoopRef		gPluginRunLoop = NULL;	// this is not our main runloop, this is our plugin runloop
DSMutexSemaphore    *gKerberosMutex = NULL;
DSEventSemaphore	gPluginRunLoopEvent;

mach_port_t			gLibinfoMachPort	= MACH_PORT_NULL;
mach_port_t			gAPIMachPort		= MACH_PORT_NULL;
mach_port_t			gMembershipMachPort	= MACH_PORT_NULL;
mach_port_t			gLegacyMachPort		= MACH_PORT_NULL;

extern const double DirectoryServiceVersionNumber;

extern "C" void _si_disable_opendirectory(void);

#ifndef DISABLE_LOCAL_PLUGIN
	extern CDSLocalPlugin	*gLocalNode;
#endif

const char *gStrDaemonBuildVersion = PROJECT_SOURCE_VERSION;
const char *gStrDaemonAppleVersion = "7.0"; // TODO: remove this once all plugins have migrated

// ---------------------------------------------------------------------------
//	* _Usage ()
//
// ---------------------------------------------------------------------------

static void _Usage ( FILE *fp, const char *argv0 )
{
	static const char * const	_szpUsage =
		"Usage:\t%s [-hv]\n"
		"	-h	Display this list of options.\n"
		"	-v	Display the release version.\n";
	::fprintf( fp, _szpUsage, argv0 );

} // _Usage


// ---------------------------------------------------------------------------
//	* _Version ()
//
// ---------------------------------------------------------------------------

static void _Version ( FILE *fp )
{
	fprintf(fp, "%s\n", gStrDaemonBuildVersion);
} // _Version


// ---------------------------------------------------------------------------
//	* _AppleVersion ()
//
// ---------------------------------------------------------------------------

static void _AppleVersion ( FILE *fp )
{
	fprintf(fp, "dspluginhelperd-%s\n", gStrDaemonBuildVersion);

} // _AppleVersion


// ---------------------------------------------------------------------------
//	* NetworkChangeCallBack ()
//
// ---------------------------------------------------------------------------

void NetworkChangeCallBack(SCDynamicStoreRef aSCDStore, CFArrayRef changedKeys, void *callback_argument)
{                       
	bool bNotify = false;
	
	for( CFIndex i=0; i<CFArrayGetCount(changedKeys); i++ )
	{
		char		keyName[256];
		
		CFStringGetCString( (CFStringRef)CFArrayGetValueAtIndex( changedKeys, i ), keyName, sizeof(keyName), kCFStringEncodingUTF8 );
		
		// we do not care about lo0 changes
		if (strstr(keyName, "/lo0/") != NULL)
		{
			DbgLog( kLogApplication, "NetworkChangeCallBack key: %s - skipping loopback", keyName );
			continue;
		}

		DbgLog( kLogApplication, "NetworkChangeCallBack key: %s", keyName );

		bNotify = true;
	}
	
	if ( gSrvrCntl != nil && bNotify )
	{
		gSrvrCntl->HandleNetworkTransition(); //ignore return status
	}
}// NetworkChangeCallBack

// ---------------------------------------------------------------------------
//	* dsPMNotificationHandler ()
//
// ---------------------------------------------------------------------------

void dsPMNotificationHandler ( void *refContext, io_service_t service, natural_t messageType, void *notificationID )
{               
	//SrvrLog( kLogApplication, "dsPMNotificationHandler(): messageType=%d\n", messageType );
        
	switch (messageType)
	{
		case kIOMessageSystemWillPowerOn:      
			DbgLog( kLogApplication, "dsPMNotificationHandler(): kIOMessageSystemWillPowerOn\n" );
			gSrvrCntl->HandleSystemWillPowerOn();
			break;

		case kIOMessageSystemHasPoweredOn:
			break;
			
		case kIOMessageSystemWillSleep:
			DbgLog( kLogApplication, "dsPMNotificationHandler(): kIOMessageSystemWillSleep\n" );
			gSrvrCntl->HandleSystemWillSleep();

		case kIOMessageSystemWillPowerOff:
		case kIOMessageCanSystemSleep:
		case kIOMessageCanSystemPowerOff:
		case kIOMessageCanDevicePowerOff:
            IOAllowPowerChange(gPMKernelPort, (long) notificationID);	// don't want to slow up machine from going to sleep
		break;

		case kIOMessageSystemWillNotSleep:
		break;

#ifdef DEBUG
		case kIOMessageServiceIsTerminated:
		case kIOMessageServiceIsSuspended:
		case kIOMessageServiceIsRequestingClose:
		case kIOMessageServiceWasClosed:
		case kIOMessageServiceIsResumed:        
		case kIOMessageServiceBusyStateChange:
		case kIOMessageDeviceWillPowerOff:
		case kIOMessageDeviceWillNotPowerOff:
		case kIOMessageSystemWillNotPowerOff:
#endif

		default:
			//SrvrLog( kLogApplication, "dsPMNotificationHandler(): called but nothing done" );
			break;

	}
} // dsPMNotificationHandler

// ---------------------------------------------------------------------------
//	* dsPostEvent ()
//
// ---------------------------------------------------------------------------

int dsPostEvent( CFStringRef inEventType, CFDictionaryRef inEventData )
{
	return ((gEventPort != NULL && _xsEventPortPostEvent != NULL) ? _xsEventPortPostEvent(gEventPort, inEventType, inEventData) : -1);
}

// ---------------------------------------------------------------------------
//	* main ()
//
// ---------------------------------------------------------------------------

int main ( int argc, char * const *argv )
{
#if DEBUG
	OptionBits			debugOpts		= kLogEverything;
	bool				bProfiling		= true;
	OptionBits			profileOpts		= kLogEverything;
#else
	OptionBits			debugOpts		= kLogMeta;
	bool				bProfiling		= false;
	OptionBits			profileOpts		= kLogMeta;
#endif
	char			   *p				= nil;
	bool				bFound			= false;
	struct stat			statResult;
	pid_t				ourUID			= ::getuid();
	static CFBundleRef	coreServerBundle	= NULL;

	if (gStrDaemonBuildVersion == NULL || gStrDaemonBuildVersion[0] == '\0') {
		gStrDaemonBuildVersion = "engineering";
	}

	_si_disable_opendirectory();
	
    // this changes the logging format to show the PID and to cause syslog to launch
    openlog( "dspluginhelperd", LOG_PID | LOG_NOWAIT, LOG_DAEMON );

	if ( argc > 1 )
	{
		p = strstr( argv[1], "-" );

		if ( p != NULL )
		{
			if ( strstr( p, "appleversion" ) )
			{
				_AppleVersion( stdout );
				::exit( 1 );
			}

			if ( strstr( p, "v" ) )
			{
				_Version( stdout );
				::exit( 1 );
			}

			if ( strstr( p, "h" ) )
			{
				::_Usage( stderr, argv[0] );
				::exit( 1 );
			}

			if ( bFound == false )
			{
				::_Usage( stderr, argv[0] );
				::exit( 1 );
			}
		}
		else
		{
			::_Usage( stderr, argv[0] );
			::exit( 1 );
		}
	}
		
	if ( ourUID != 0 )
	{
		syslog(LOG_ALERT, "dspluginhelperd needs to be launched as root.\n");
		exit( 1 );
	}
	
	if ( gDSLocalOnlyMode == false && gDSInstallDaemonMode == false && gDSDebugMode == false &&
		lstat("/etc/rc.cdrom", &statResult) == 0 && lstat("/System/Installation", &statResult) == 0 )
	{
		gDSInstallDaemonMode = true;
		syslog( LOG_NOTICE, "Launched build %s - installer mode", gStrDaemonBuildVersion );
	}
	else if ( gDSDebugMode == true )
	{
		printf( "Debug mode enabled.\nTo access this daemon define environment variable DS_DEBUG_MODE.\n" );
	}
	else
	{
		syslog( LOG_INFO, "Launched build %s", gStrDaemonBuildVersion );
	}
	
	mach_port_t			priv_bootstrap_port	= MACH_PORT_NULL;
	int					status				= eDSNoErr;

	if (!gDSDebugMode)
	{
#ifndef DISABLE_SEARCH_PLUGIN
		char* usedPortName = (char *) (gDSLocalOnlyMode == true ? kDSStdMachLocalPortName : kDSStdMachPortName);

        /*
         * See if our service name is already registered and if we have privilege to check in.
         */
		status = bootstrap_check_in( bootstrap_port, usedPortName, &gAPIMachPort );
		if ( status == BOOTSTRAP_SERVICE_ACTIVE ) {
			syslog(LOG_ALERT, "dspluginhelperd %s instance is already running - exiting this instance", usedPortName );
			exit( 0 );
		}

		assert( status == BOOTSTRAP_SUCCESS );
#endif

		if ( gDSLocalOnlyMode == false ) {
			// checkin for our libinfo and membership name
#ifndef DISABLE_CACHE_PLUGIN
			status = bootstrap_check_in(bootstrap_port, (char *)kDSStdMachDSLookupPortName, &gLibinfoMachPort);
			assert( status == BOOTSTRAP_SUCCESS );
#endif
			
#ifndef DISABLE_MEMBERSHIP_CACHE
			status = bootstrap_check_in( bootstrap_port, (char *)kDSStdMachMembershipPortName, &gMembershipMachPort );
			assert( status == BOOTSTRAP_SUCCESS );
#endif
			
			status = bootstrap_check_in(bootstrap_port, (char *)"com.apple.system.DirectoryService.legacy", &gLegacyMachPort);
			assert( status == BOOTSTRAP_SUCCESS );
		}
	}
	else // this is only debug mode, we don't error anything
	{
        /*
         * See if our service name is already registered and if we have privilege to check in.
		 * This should never work for debug mode. - expect to get BOOTSTRAP_UNKNOWN_SERVICE
         */
		status = bootstrap_check_in( bootstrap_port, "com.apple.system.DirectoryService.legacyDebug", &gLegacyMachPort );
		if (status == BOOTSTRAP_SERVICE_ACTIVE) {
			syslog(LOG_ALERT, "dspluginhelperd debug instance is already running - exiting this instance" );
			exit(0);
		}
	}

	try
	{
		if (!gDSLocalOnlyMode)
		{
			// need to make sure this file is not present yet
			unlink( "/var/run/.DSRunningSP4" );
		}

		//global set to determine different behavior dependant on server build versus desktop
		if ( lstat("/System/Library/CoreServices/ServerVersion.plist", &statResult) == 0 ) {
			gServerOS = true;
		}
		
		// if not properly shut down, the SQL index for the local node needs to be deleted
		// we look for the pid and the special file since /var/run gets cleaned at boot
		// and we could have crashed just as we were shutting down
		if ( gDSDebugMode == true || 
			 gDSLocalOnlyMode == true || 
			 gDSInstallDaemonMode == true || 
			 (stat(kDSPIDFile, &statResult) != 0 && stat(kDSRunningFile, &statResult) != 0) )
		{
			// file not present, last shutdown was normal
			gProperShutdown = true;
		}
		
		if ( !gDSLocalOnlyMode && !gDSInstallDaemonMode && !gDSDebugMode )
		{
			// create pid file
			char pidStr[256];
			int fd = open( kDSPIDFile, (O_CREAT | O_TRUNC | O_WRONLY | O_EXLOCK), 0644 );
			if ( fd != -1 )
			{
				snprintf( pidStr, sizeof(pidStr), "%d", getpid() );
				write( fd, pidStr, strlen(pidStr) );
				close( fd );
			}
			
			// let's not log if the file is there
			if ( stat(kDSRunningFile, &statResult) != 0 )
				dsTouch( kDSRunningFile );
		}
		
		if (gDSDebugMode)
		{
			debugOpts |= kLogDebugHeader;
		}
		
		// Open the log files
		CLog::Initialize(kLogNone, kLogNone, debugOpts, profileOpts, gDebugLogging, bProfiling, gDSLocalOnlyMode, od_passthru_log_message);

		SrvrLog( kLogApplication, "\n\n" );
		SrvrLog(kLogApplication, "dspluginhelperd (build %s) starting up...", gStrDaemonBuildVersion);
		
		if ( gProperShutdown == false ) {
			DbgLog( kLogCritical, "Improper shutdown detected" );
			syslog( LOG_NOTICE, "Improper shutdown detected" );
		}
		
		int			sbmib[] = { CTL_KERN, KERN_SAFEBOOT };
		uint32_t	sb = 0;
		size_t		sbsz = sizeof(sb);
		
		if ( sysctl(sbmib, 2, &sb, &sbsz, NULL, 0) == -1 )
			gSafeBoot = false;
		
		if ( sb == true ) {
			gSafeBoot = true;
			SrvrLog( kLogApplication, "Safe Boot is enabled" );
		}
		
		struct rlimit rl = { FD_SETSIZE, FD_SETSIZE };
		
		if ( setrlimit(RLIMIT_NOFILE, &rl) != 0 ) {
			syslog( LOG_NOTICE, "Unable to increase file limit to %d", rl.rlim_cur );
		}
		
		dispatch_source_t source;
		
		// TERM
		source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0, dispatch_get_main_queue());
		assert(source != NULL);
		dispatch_source_set_event_handler(source, 
										  ^(void) {
											  DbgLog( kLogInfo, "dsdispatch - SIGTERM - attempting to stop main runloop" );
											  CFRunLoopStop( CFRunLoopGetMain() ); 
										  });
		assert( source != NULL );
		dispatch_resume(source);
		signal(SIGTERM, SIG_IGN);

		// HUP
		source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGHUP, 0, dispatch_get_main_queue());
		assert(source != NULL);
		dispatch_source_set_event_handler(source, 
										  ^(void) {
											  ServerControl::HandleNetworkTransition();
											  DbgLog( kLogInfo, "dsdispatch - SIGHUP - simulating network transition" );
										  });
		assert( source != NULL );
		dispatch_resume(source);
		signal( SIGHUP, SIG_IGN );

		if ( gDSDebugMode == false ) {
			signal( SIGINT, SIG_IGN );
		}
		signal( SIGPIPE, SIG_IGN );
		signal(SIGUSR1, SIG_IGN); // no longer control debug logging via signal
		signal(SIGUSR2, SIG_IGN); // no longer control API logging via signal

		// first thing we do is setup our plugin runloop for handling requests from plugins
		CPluginRunLoopThread *pluginRunLoopThread = new CPluginRunLoopThread();

		gPluginRunLoopEvent.ResetEvent();
		pluginRunLoopThread->StartThread();
		gPluginRunLoopEvent.WaitForEvent();
		
		//set up a mutex semaphore for all plugins using Kerberos
		gKerberosMutex = new DSMutexSemaphore("::gKerberosMutex");
		
		// Do setup after parent is removed if daemonizing
		gSrvrCntl = new ServerControl();
		if ( gSrvrCntl == nil ) throw( (SInt32)eMemoryAllocError );

		if ( gDebugLogging )
		{
			gSrvrCntl->ResetDebugging(); //ignore return status
		}

		// we stat instead of using CFBundlePreflightExecutable() to prevent a deadlock with CF text encoding issues
		struct stat statBlock;
		if ( stat("/System/Library/PrivateFrameworks/CoreDaemon.framework", &statBlock) == 0 ) {
			
			CFURLRef coreServerURL = CFURLCreateWithFileSystemPath( NULL, CFSTR("/System/Library/PrivateFrameworks/CoreDaemon.framework"), 
																    kCFURLPOSIXPathStyle, false );
			if ( coreServerURL != NULL ) {
				
				coreServerBundle = CFBundleCreate( kCFAllocatorDefault, coreServerURL );
				if ( coreServerBundle != NULL ) {
					
					Boolean isLoaded = CFBundleIsExecutableLoaded( coreServerBundle );
					if ( isLoaded == FALSE )
						isLoaded = CFBundleLoadExecutable( coreServerBundle );
					
					if ( isLoaded == TRUE ) {
						_xsEventPortPostEvent = (_XSEventPortPostEvent *) CFBundleGetFunctionPointerForName( coreServerBundle, CFSTR("XSEventPortPostEvent") );
						_xsEventPortCreate = (_XSEventPortCreate *) CFBundleGetFunctionPointerForName( coreServerBundle, CFSTR("XSEventPortCreate") );
						
						if ( _xsEventPortPostEvent != NULL && _xsEventPortCreate != NULL )
							SrvrLog( kLogApplication, "CoreDaemon.framework found using for events" );
						else
							DbgLog( kLogError, "CoreDaemon.framework found but unable to map functions" );
					}
				}
				
				DSCFRelease( coreServerURL );
			}
		}
		
		// Create an XSEventPort for the adaptive firewall
		if ( _xsEventPortCreate != NULL )
			gEventPort = _xsEventPortCreate( NULL );
		
		vproc_transaction_t vProcTransaction = NULL;
		if ( !gDSLocalOnlyMode && !gDSInstallDaemonMode && !gDSDebugMode )
		{
			vProcTransaction = vproc_transaction_begin( NULL );
		}
		
		// temporary until MIG dispatch
		// need a solution for kauth syscall work
		// helps lower thread cycling rates, but increases our base thread count
		sbsz = sizeof( gNumberOfCores );
		sysctlbyname( "hw.logicalcpu_max", &gNumberOfCores, &sbsz, NULL, 0 );
		SrvrLog( kLogApplication, "Detected %d logical CPUs", gNumberOfCores );
		if ( gNumberOfCores > 4 ) gNumberOfCores = 4;
		
		SInt32 startSrvr;
		startSrvr = gSrvrCntl->StartUpServer();
		if ( startSrvr != eDSNoErr ) throw( startSrvr );
		
		CFRunLoopRun();
		
		// stop our plugin runloop
		CFRunLoopStop( gPluginRunLoop );
		
#ifndef DISABLE_LOCAL_PLUGIN
		gLocalNode->CloseDatabases();
#endif
		
		if ( gSrvrCntl != NULL )
		{
			SrvrLog( kLogApplication, "Shutting down dspluginhelperd..." );
			gSrvrCntl->ShutDownServer();
		}
		
		if ( !gDSLocalOnlyMode && !gDSInstallDaemonMode && !gDSDebugMode )
		{
			fcntl( 0, F_FULLFSYNC ); // ensure FS is flushed before we remove the PID files
			
			dsRemove( kDSRunningFile );
			dsRemove( kDSPIDFile );
			
			if ( vProcTransaction != NULL )
				vproc_transaction_end( NULL, vProcTransaction );
		}
	}

	catch ( SInt32 err )
	{
		DbgLog( kLogApplication, "File: %s. Line: %d", __FILE__, __LINE__ );
		DbgLog( kLogApplication, "  ***main() error = %d.", err );
	}

	catch( ... )
	{
		// if we got here we are in trouble.
		DbgLog( kLogApplication, "File: %s. Line: %d", __FILE__, __LINE__ );
		DbgLog( kLogApplication, "  *** Caught an unexpected exception in main()!!!!" );
	}

	exit( 0 );

} // main

CFArrayRef
dsCopyKerberosServiceList( void )
{
	return CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
}
