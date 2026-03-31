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
 * @header CPluginHandler
 */

#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>

#include "CPluginHandler.h"
#include "CPlugInList.h"
#include "ServerControl.h"
#include "DirServiceMain.h"
#include "CString.h"
#include "COSUtils.h"
#include "CLog.h"
#include "CServerPlugin.h"
#include "CDSPluginUtils.h"
#include "DSEventSemaphore.h"
#include "CInternalDispatch.h"

// --------------------------------------------------------------------------------
//	* Globals
// --------------------------------------------------------------------------------

extern const char *gStrDaemonAppleVersion;

//static plugin names
static const char	*sStaticPluginList[]	= { "Cache", "Configure", "Local", "LDAPv3", "Search", "BSD" };

enum ePluginIndex
{
	kPluginCache	= 0,
	kPluginConfigure,
	kPluginLocal,
	kPluginLDAPv3,
	kPluginSearch,
	kPluginBSD
};

extern	dsBool			gDSLocalOnlyMode;
extern	bool			gNetInfoPluginIsLoaded;
extern	dsBool			gDSInstallDaemonMode;
#ifndef DISABLE_CACHE_PLUGIN
extern  DSEventSemaphore gKickCacheRequests;
#endif

//--------------------------------------------------------------------------------------------------
//	* CPluginHandler()
//
//--------------------------------------------------------------------------------------------------

CPluginHandler::CPluginHandler ( void ) : DSCThread(kTSPlugInHndlrThread)
{
	fThreadSignature = kTSPlugInHndlrThread;
} // CPluginHandler



//--------------------------------------------------------------------------------------------------
//	* ~CPluginHandler()
//
//--------------------------------------------------------------------------------------------------

CPluginHandler::~CPluginHandler()
{
} // ~CPluginHandler


//--------------------------------------------------------------------------------------------------
//	* StartThread()
//
//--------------------------------------------------------------------------------------------------

void CPluginHandler::StartThread ( void )
{
	if ( this == nil ) throw((SInt32)eMemoryError);

	this->Resume();
} // StartThread



//--------------------------------------------------------------------------------------------------
//	* StopThread()
//
//--------------------------------------------------------------------------------------------------

void CPluginHandler::StopThread ( void )
{
	if ( this == nil ) throw((SInt32)eMemoryError);

	// Check that the current thread context is not our thread context
//	SignalIf_( this->IsCurrent() );

	SetThreadRunState( kThreadStop );		// Tell our thread to stop

} // StopThread



//--------------------------------------------------------------------------------------------------
//	* ThreadMain()
//
//--------------------------------------------------------------------------------------------------

SInt32 CPluginHandler::ThreadMain ( void )
{
	UInt32		uiPluginCnt		= 0;
	UInt32		statPluginCnt	= 0;
	SInt32		status			= eDSNoErr;

	CInternalDispatch::AddCapability();
	
	if (gDSLocalOnlyMode)
	{
#ifndef DISABLE_CONFIGURE_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginConfigure],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
#ifndef DISABLE_LOCAL_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginLocal],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
		statPluginCnt = uiPluginCnt;

		DbgLog( kLogApplication, "%d static plugins processed.", uiPluginCnt );

		DbgLog( kLogApplication, "Initializing static plugins." );
		gPlugins->InitPlugIns(kStaticPlugin);		
	}
	else if (gDSInstallDaemonMode)
	{
#ifndef DISABLE_CACHE_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginCache],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
#ifndef DISABLE_CONFIGURE_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginConfigure],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
#ifndef DISABLE_LOCAL_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginLocal],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
#ifndef DISABLE_SEARCH_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginSearch],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
#ifndef DISABLE_BSD_PLUGIN
		status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[kPluginBSD],
														gStrDaemonAppleVersion);
		if (status == eDSNoErr)
		{
			uiPluginCnt++;
		}
#endif
		statPluginCnt = uiPluginCnt;

		DbgLog( kLogApplication, "%d static plugins processed.", uiPluginCnt );

		DbgLog( kLogApplication, "Initializing static plugins." );
		gPlugins->InitPlugIns(kStaticPlugin);		
	}
	else
	{
		//process the static plugin modules
		for (UInt32 iPlugin = 0; iPlugin < sizeof(sStaticPluginList) / sizeof(const char *); iPlugin++)
		{	
			status = CServerPlugin::ProcessStaticPlugin(	sStaticPluginList[iPlugin],
															gStrDaemonAppleVersion);
			if (status == eDSNoErr)
			{
				uiPluginCnt++;
			}
		}
		statPluginCnt = uiPluginCnt;
		
		DbgLog( kLogApplication, "%d static plugins processed.", uiPluginCnt );

		DbgLog( kLogApplication, "Initializing static plugins." );
		gPlugins->InitPlugIns(kStaticPlugin);

#ifndef DISABLE_CACHE_PLUGIN
        DbgLog( kLogApplication, "Waiting on Cache node initialization" );
        gKickCacheRequests.WaitForEvent();
        DbgLog( kLogApplication, "Cache node initialization - succeeded" );
#endif
		
		uiPluginCnt = LoadPlugins(uiPluginCnt);
	}
	
	if ( (uiPluginCnt - statPluginCnt) == 0 )
	{
		if ( gDSLocalOnlyMode == false ) {
			ErrLog( kLogApplication, "*** WARNING: No Plugins loaded ***" );
		}
	}
	else
	{
		DbgLog( kLogApplication, "%d Plugins loaded.", uiPluginCnt - statPluginCnt );

		DbgLog( kLogApplication, "Initializing loaded plugins." );
		gPlugins->InitPlugIns(kAppleLoadedPlugin);
	}

	return( 0 );

} // ThreadMain


//--------------------------------------------------------------------------------------------------
//	* LoadPlugins ()
//
//--------------------------------------------------------------------------------------------------

UInt32 CPluginHandler::LoadPlugins ( UInt32 inCount )
{
	SInt32				status	= eDSNoErr;
	UInt32				uiCount	= inCount;
	CString				cOtherSubPath( COSUtils::GetStringFromList( kAppStringsListID, kStrOtherPlugInsFolder ) );
	CString				cPlugExt( COSUtils::GetStringFromList( kAppStringsListID, kStrPluginExtension ) );
	CFStringRef			sType	= nil;
	char				string	[ PATH_MAX ];
	CFURLRef	urlPath = 0;
	CFStringRef	sPath;
	CFArrayRef	aBundles;
    
	// retrieve any Third party developed plugins
	sType = ::CFStringCreateWithCString( kCFAllocatorDefault, cPlugExt.GetData(), kCFStringEncodingMacRoman );

	// Append the subpath.
	sPath = ::CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "/Library/%s" ), cOtherSubPath.GetData() );

	::CFStringGetCString( sPath, string, sizeof( string ), kCFStringEncodingMacRoman );
	DbgLog( kLogApplication, "Checking for plugins in:" );
	DbgLog( kLogApplication, "  %s", string );

	// Convert it back into a CFURL.
	urlPath = ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, sPath, kCFURLPOSIXPathStyle, true );
	::CFRelease( sPath );
	sPath = nil;

	// Enumerate all the plugins in this system directory.
	aBundles = ::CFBundleCopyResourceURLsOfTypeInDirectory( urlPath, sType, NULL );
	::CFRelease( urlPath );
	urlPath = nil;

	if ( aBundles != nil )
	{
		register CFIndex j = ::CFArrayGetCount( aBundles );
		while ( j-- )
		{
			status = CServerPlugin::ProcessURL( (CFURLRef)::CFArrayGetValueAtIndex( aBundles, j ) );
			if ( status == eDSNoErr )
			{
				uiCount++;
			}
			else
				SrvrLog( kLogApplication, "\tError loading 3rd party plugin, see /var/log/opendirectoryd.log for details" );
		}

		::CFRelease( aBundles );
		aBundles = nil;
	}

	if ( sType != nil )
	{
		::CFRelease( sType );
		sType = nil;
	}

	return( uiCount );

} // LoadPlugins

