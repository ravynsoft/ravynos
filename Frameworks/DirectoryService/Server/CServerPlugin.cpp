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
 * @header CServerPlugin
 */

#include "CServerPlugin.h"
#include "ServerControl.h"
#include "CNodeList.h"
#include "CPlugInList.h"
#include "CString.h"
#include "DSUtils.h"
#include "CLog.h"
#include "CDSPluginUtils.h"
#include "CPluginConfig.h"
#ifndef DISABLE_CONFIGURE_PLUGIN
	#include "CConfigurePlugin.h"
#endif
#ifndef DISABLE_LOCAL_PLUGIN
	#endif
#ifndef DISABLE_CACHE_PLUGIN
	#include "CCachePlugin.h"
#endif
#ifndef DISABLE_SEARCH_PLUGIN
	#include "CSearchPlugin.h"
#endif
#ifndef DISABLE_BSD_PLUGIN
	#include "BSDPlugin.h"
#endif
#ifndef DISABLE_LDAPV3_PLUGIN
	#include "CLDAPv3Plugin.h"
#endif
#include "od_passthru.h"

#include <stdlib.h>	// for rand()
#include <syslog.h>

#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>

#ifndef DISABLE_CONFIGURE_PLUGIN
extern CPluginConfig	   *gPluginConfig;
#endif
extern dsBool				gDSInstallDaemonMode;
extern dsBool				gDSLocalOnlyMode;
extern dsBool				gDSDebugMode;

using namespace DSServerPlugin;

// ----------------------------------------------------------------------------
//	* Private stuff
//	These are not declared statically in the class definition because I want
//	to hide the implementation details and reduce unrelated dependencies in
//	the class header.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//	* CServerPlugin Class Globals
//
//	These private typedefs, globals, and functions are not declared statically
//	in the class definition because I want to hide the implementation details
//	and reduce unrelated dependencies in the class header.
// ----------------------------------------------------------------------------

static void			_DebugLog			( const char *inFormat, va_list inArgs );
static void			_DebugLogWithType	( const UInt32 inSignature, const UInt32 inLogType, const char *inFormat, va_list inArgs );

static SvrLibFtbl	_Callbacks = { CServerPlugin::_RegisterNode, CServerPlugin::_UnregisterNode, _DebugLog, _DebugLogWithType };

const	SInt32	kNodeNotRegistered			= -8000;
const	SInt32	kNodeAlreadyRegistered		= -8001;
const	SInt32	kInvalidToken				= -8002;
const	SInt32	kNullNodeName				= -8003;
const	SInt32	kEmptyNodeName				= -8004;
const	SInt32	kServerError				= -8101;

// ----------------------------------------------------------------------------
//	* _RegisterNode()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::_RegisterNode ( const UInt32 inToken, tDataList *inNodeList, eDirNodeType inNodeType )
{
	return InternalRegisterNode( inToken, inNodeList, inNodeType, false );
}

SInt32 CServerPlugin::InternalRegisterNode ( const UInt32 inToken, tDataList *inNodeList, eDirNodeType inNodeType, bool isProxyRegistration )
{
	SInt32			siResult	= eDSNoErr;
	tDataList	   *pNodeList	= nil;
	char		   *pNodeName	= nil;
	CServerPlugin  *pPluginPtr	= nil;
	bool			bDone		= false;

	if ( inNodeType == kUnknownNodeType )
	{
		return( kNodeNotRegistered );
	}
	
	if ( inToken == 0 )
	{
		return( kInvalidToken );
	}
	// Get the plugin pointer for this token
	//this call associates the pluginPtr with the input token thus preventing any plugin
	//from registering a node for another plugin unless that plugin knows the other plugin's token/signature
	pPluginPtr = gPlugins->GetPlugInPtr( inToken, !isProxyRegistration );	// if this is a proxy registration (i.e. lazy loading), its ok to be nil
	if ( pPluginPtr == nil && !isProxyRegistration )
	{
		return( kInvalidToken );
	}
	
	if ( inNodeList == nil )
	{
		return( kNullNodeName );
	}

	if ( (inNodeList->fDataNodeCount == 0) || (inNodeList->fDataListHead == nil) )
	{
		return( kEmptyNodeName );
	}

	if ( gNodeList == nil )
	{
		return( kServerError );
	}

	// Create a path string
	pNodeName = ::dsGetPathFromListPriv( inNodeList, (char *)gNodeList->GetDelimiter() );
	if ( pNodeName != nil )
	{
		if ( inNodeType & kDirNodeType )
		{
			// Is this node registered - ie. bow out early if duplicate exists
			if ( gNodeList->IsPresent( pNodeName, kDirNodeType ) == false )
			{
				// Get our copy of the node list
				//this pNodeList either gets consumed by the AddNode OR we free it
				pNodeList = dsBuildFromPathPriv( pNodeName, (char *)gNodeList->GetDelimiter() );
				
				// Add the node to the node list
				gNodeList->AddNode( pNodeName, pNodeList, kDirNodeType, pPluginPtr, inToken );
				
				int32_t err = od_passthru_register_node(pNodeName, false);
				if (err == 0) {
					DbgLog(kLogPlugin, "Registered Directory Node %s with opendirectoryd", pNodeName);
				}
				else {
					DbgLog(kLogNotice, "Failed to Directory Node %s with opendirectoryd: %s", pNodeName, strerror(err));
				}

				DbgLog( kLogPlugin, "Registered Directory Node %s", pNodeName );
				
				if ( gDSInstallDaemonMode == false && gDSLocalOnlyMode == false && gDSDebugMode == false )
					dsPostNodeEvent();
			}
			else
			{
				siResult = eDSNoErr; // don't return error, just make it think it got registered
			}
			bDone = true;
		}
		
		if ( inNodeType & kLocalHostedType )
		{
			// Is this node registered - ie. bow out early if duplicate exists
			if ( gNodeList->IsPresent( pNodeName, kLocalHostedType ) == false )
			{
				// Get our copy of the node list
				//this pNodeList either gets consumed by the AddNode OR we free it
				pNodeList = dsBuildFromPathPriv( pNodeName, (char *)gNodeList->GetDelimiter() );
				
				// Add the node to the node list
				gNodeList->AddNode( pNodeName, pNodeList, kLocalHostedType, pPluginPtr, inToken );

				int32_t err = od_passthru_register_node(pNodeName, false);
				if (err == 0) {
					DbgLog(kLogPlugin, "Registered Locally Hosted %s with opendirectoryd", pNodeName);
				}
				else {
					DbgLog(kLogNotice, "Failed to register Locally Hosted %s with opendirectoryd: %s", pNodeName, strerror(err));
				}

				SrvrLog( kLogPlugin, "Registered Locally Hosted Node %s", pNodeName );
			}
			else
			{
				siResult = eDSNoErr; // don't return error, just make it think it got registered
			}
			bDone = true;
		}
		
		if ( inNodeType & kDefaultNetworkNodeType )
		{
			// Is this node registered - ie. bow out early if duplicate exists
			if ( gNodeList->IsPresent( pNodeName, kDefaultNetworkNodeType ) == false )
			{
				// Get our copy of the node list
				//this pNodeList either gets consumed by the AddNode OR we free it
				pNodeList = dsBuildFromPathPriv( pNodeName, (char *)gNodeList->GetDelimiter() );
				
				// Add the node to the node list
				gNodeList->AddNode( pNodeName, pNodeList, kDefaultNetworkNodeType, pPluginPtr, inToken );
				DbgLog( kLogPlugin, "Registered Default Network Node %s", pNodeName );
			}
			else
			{
				siResult = eDSNoErr; // don't return error, just make it think it got registered
			}
			bDone = true;
		}
		
		if ( !bDone && ( inNodeType & ( kLocalNodeType | kCacheNodeType | kSearchNodeType | kConfigNodeType | kContactsSearchNodeType | kNetworkSearchNodeType | kBSDNodeType ) ) )
		//specific node type added here - don't check for duplicates here
		//need to always be able to get in here regardless of mutexes ie. don't call IsPresent()
		{
			// Get our copy of the node list
			//this pNodeList either gets consumed by the AddNode OR we free it
			pNodeList = dsBuildFromPathPriv( pNodeName, (char *)gNodeList->GetDelimiter() );
			
			// Add the node to the node list
			siResult = gNodeList->AddNode( pNodeName, pNodeList, inNodeType, pPluginPtr, inToken );
			if (siResult == 0) //specific return from AddNode ie. not really a DS status
			{
				::dsDataListDeallocatePriv( pNodeList );
				free(pNodeList);
				pNodeList = nil;
				siResult = eDSNoErr; // don't return error, just make it think it got registered
			}
			else
			{
				bool hidden = true;
				
				if ((inNodeType & (kDirNodeType | kLocalNodeType | kConfigNodeType | kBSDNodeType | kSearchNodeType | kContactsSearchNodeType)) != 0) {
					hidden = false;
				}
				
				int32_t err = od_passthru_register_node(pNodeName, hidden);
				if (err == 0) {
					DbgLog(kLogPlugin, "Registered node %s with opendirectoryd", pNodeName);
				}
				else {
					DbgLog(kLogNotice, "Failed to register node %s with opendirectoryd: %s", pNodeName, strerror(err));
				}

				SrvrLog( kLogPlugin, "Registered node %s", pNodeName );
				siResult = eDSNoErr;
			}
		}

		free( pNodeName );
		pNodeName = nil;
	}
	else
	{
		siResult = kServerError;
	}

	return( siResult );

} // _RegisterNode


// ----------------------------------------------------------------------------
//	* _UnregisterNode()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::_UnregisterNode ( const UInt32 inToken, tDataList *inNode )
{
	SInt32			siResult	= 0;
	char		   *nodePath	= nil;
	
	if ( inNode == nil )
	{
		return( kNullNodeName );
	}

	if ( (inNode->fDataNodeCount == 0) || (inNode->fDataListHead == nil) )
	{
		return( kEmptyNodeName );
	}

	if ( gNodeList == nil )
	{
		return( kServerError );
	}

	nodePath = ::dsGetPathFromListPriv( inNode, (char *)gNodeList->GetDelimiter() );
	if ( nodePath != nil )
	{
		if ( gNodeList->DeleteNode( nodePath ) != true )
		{
			siResult = kNodeNotRegistered;
			DbgLog( kLogPlugin, "Can't unregister node %s because it is not registered", nodePath );
		}
		else
		{
			if ( gDSInstallDaemonMode == false && gDSLocalOnlyMode == false && gDSDebugMode == false )
				dsPostNodeEvent();
			DbgLog( kLogPlugin, "Unregistered node %s", nodePath );
		}
		free( nodePath );
		nodePath = nil;
	}

	return( siResult );

} // _UnregisterNode


// ----------------------------------------------------------------------------
//	* _DebugLog()
//
// ----------------------------------------------------------------------------

static void _DebugLog ( const char *inPattern, va_list args )
{
	CString		inLogStr( inPattern, args );

	DbgLog( kLogPlugin, inLogStr.GetData() );

} // _DebugLog


// ----------------------------------------------------------------------------
//	* _DebugLogWithType()
//
// ----------------------------------------------------------------------------

static void _DebugLogWithType ( const UInt32 inSignature, const UInt32 inLogType, const char *inPattern, va_list args )
{
	CString		inLogStr( inPattern, args );
	
	UInt32		aPriorityMask = kLogEmergency | kLogAlert | kLogCritical | kLogError | kLogWarning | kLogNotice | kLogInfo | kLogDebug;
	UInt32		aLogType = aPriorityMask & inLogType;

	//we can use inSignature to discriminate which plugins are allowed to log here if we wish
	
	if (aLogType == 0)
	{
		aLogType = kLogPlugin;
	}
	
	// log all alerts, etc. to the server error log
	if ( (inLogType & (kLogEmergency | kLogAlert | kLogCritical | kLogError)) != 0 )
		ErrLog( aLogType, inLogStr.GetData() );
	else
		DbgLog( aLogType, inLogStr.GetData() );

} // _DebugLogWithType


// ----------------------------------------------------------------------------
//	* CServerPlugin()
//
// ----------------------------------------------------------------------------

CServerPlugin::CServerPlugin ( void ) : mInstance( NULL )
{
	SvrLibFtbl stTemp	= _Callbacks;
	fPlugInSignature	= 0;
	fPlugInName			= nil;
}

// ----------------------------------------------------------------------------
//	* CServerPlugin()
//
// ----------------------------------------------------------------------------

CServerPlugin::CServerPlugin ( FourCharCode inSig, const char *inName ) : mInstance( NULL )
{
	SvrLibFtbl stTemp	= _Callbacks;
    
	fPlugInSignature	= inSig;
	fPlugInName			= nil;
	if ( inName != nil )
	{
		fPlugInName = strdup(inName);
	}
}


// ----------------------------------------------------------------------------
//	* CServerPlugin()
//
// ----------------------------------------------------------------------------

CServerPlugin::CServerPlugin ( CFPlugInRef		inThis,
								 CFUUIDRef		inFactoryID,
								 FourCharCode	inSig,
								 UInt32			inVers,
								 const char	   *inName )
	: mInstance( NULL )
{
	IUnknownVTbl	*spUnknown	= NULL;
	SvrLibFtbl		stTemp		= _Callbacks;

	fPlugInRef	= inThis;
	fPlugInVers	= inVers; //never used anywhere
	fPlugInName	= nil;
	
	spUnknown = (IUnknownVTbl *)::CFPlugInInstanceCreate( kCFAllocatorDefault,
														  inFactoryID,
														  kModuleTypeUUID );
	if ( spUnknown == NULL )
	{
		ErrLog( kLogPlugin, "*** Error in CServerPlugin:CFPlugInInstanceCreate -- spUnknown == NULL." );
		throw( (SInt32) 'ecom' );
	}

	spUnknown->QueryInterface( spUnknown,
								::CFUUIDGetUUIDBytes( kModuleInterfaceUUID ),
								(LPVOID *)(&mInstance) );

	// Now we are done with IUnknown
	spUnknown->Release( spUnknown );

	if ( mInstance == NULL )
	{
		ErrLog( kLogPlugin, "*** Error in CServerPlugin:QueryInterface -- mInstance == NULL." );
		throw( (SInt32) 'ecom' );
	}

	stTemp.fSignature = inSig;
	mInstance->linkLibFtbl( mInstance, &stTemp );

	if ( inName != nil )
	{
		fPlugInName = strdup(inName);
	}

	fPlugInSignature = inSig;

} // CServerPlugin


// ----------------------------------------------------------------------------
//	* ~CServerPlugin()
//
// ----------------------------------------------------------------------------

CServerPlugin::~CServerPlugin ( void )
{
	if ( fPlugInName != nil )
	{
		delete( fPlugInName );
		fPlugInName = nil;
	}

	if ( mInstance != nil )
	{
		mInstance->Release( mInstance );
		mInstance = nil;
	}

} // ~CServerPlugin


// ----------------------------------------------------------------------------
//	* ProcessURL()
//
//		- Passed to _ProcessAllURLs() to validate each
//			bundle's property list and create the plugin object.
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::ProcessURL ( CFURLRef inURLPlugin )
{
	SInt32			siResult			= -1;
	char		   *pPIVersion			= nil;
	char		   *pPIName				= nil;
	char		   *pPIConfigAvail 		= nil;
	char		   *pPIConfigFile 		= nil;
	bool			bGotIt				= false;
	unsigned char	path[ PATH_MAX ]	= "\0";
	UInt32			ulVers				= 0;
	CServerPlugin  *cpPlugin			= nil;
	FourCharCode	fccPlugInSignature	= 0;
	CFPlugInRef		plgThis				= NULL;
	CFStringRef		cfsVersion			= nil;
	CFStringRef		cfsName				= nil;
	CFStringRef		cfsConfigAvail 		= nil;
	CFStringRef		cfsConfigFile 		= nil;
	CFArrayRef		cfaFactories		= nil;
	CFUUIDRef		cfuuidFactory		= nil;
	CFBundleRef		bdlThis				= nil;
	CFDictionaryRef	plInfo				= nil;
	CFStringRef		cfsOKToLoadPluginLazily	= nil;
	CFArrayRef		cfaLazyNodesToRegister	= nil;
	bool			loadPluginLazily	= false;
	UInt32			callocLength		= 0;
	
	try
	{
		if ( inURLPlugin == NULL )
			return (SInt32)ePlugInInitError;
		
		plgThis = ::CFPlugInCreate( kCFAllocatorDefault, inURLPlugin );

		::CFURLGetFileSystemRepresentation( inURLPlugin, true, path, sizeof( path ) );
		if ( plgThis == nil ) throw ( (SInt32)eCFMGetFileSysRepErr );

		CFDebugLog( kLogApplication, "CServerPlugin::ProcessURL called CFURLGetFileSystemRepresentation with path <%s>", path );

		bdlThis	= ::CFPlugInGetBundle( plgThis );
		if ( bdlThis == nil ) throw ( (SInt32)eCFPlugInGetBundleErr );

		plInfo	= ::CFBundleGetInfoDictionary( bdlThis );
		if ( plInfo == nil ) throw ( (SInt32)eCFBndleGetInfoDictErr );

		ulVers	= ::CFBundleGetVersionNumber( bdlThis );
		
#ifdef __LP64__
		CFArrayRef cfArchs = CFBundleCopyExecutableArchitectures( bdlThis );
		bool bSupported = false;
		
		if ( NULL != cfArchs )
		{
			CFIndex iCount = CFArrayGetCount( cfArchs );
			for ( int ii = 0; ii < iCount; ii++ )
			{
				CFNumberRef number = (CFNumberRef) CFArrayGetValueAtIndex( cfArchs, ii );
				int tempValue;
				
				if ( CFNumberGetValue(number, kCFNumberIntType, &tempValue) == true )
				{
					if ( tempValue == kCFBundleExecutableArchitectureX86_64 || tempValue == kCFBundleExecutableArchitecturePPC64 )
					{
						bSupported = true;
						break;
					}
				}
			}
			
			DSCFRelease( cfArchs );
		}
		
		if ( bSupported == false )
		{
			DbgLog( kLogError, "Unable to load plugin at <%s> because it does not support 64-bit architecture, please contact developer.", path );
			syslog( LOG_NOTICE, "Unable to load plugin at <%s> because it does not support 64-bit architecture, please contact developer.", path );
			throw ((SInt32) ePlugInInitError);
		}
#endif

		if ( ::CFDictionaryGetValue( plInfo, kCFPlugInTypesKey ) == nil ) throw ( (SInt32)eCFBndleGetInfoDictErr );
		if ( ::CFDictionaryGetValue( plInfo, kCFPlugInFactoriesKey ) == nil ) throw ( (SInt32)eCFBndleGetInfoDictErr );

		// Get the plugin version
		cfsVersion = (CFStringRef)::CFDictionaryGetValue( plInfo, kPluginShortVersionStr );
		if ( cfsVersion == nil ) throw( (SInt32)ePluginVersionNotFound );

		callocLength = (UInt32) CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfsVersion), kCFStringEncodingUTF8) + 1;
		pPIVersion = (char *)::calloc( 1, callocLength );
		if ( pPIVersion == nil ) throw( (SInt32)eMemoryError );

		// Convert it to a regular 'C' string 
		bGotIt = CFStringGetCString( cfsVersion, pPIVersion, callocLength, kCFStringEncodingUTF8 );
		if (bGotIt == false) throw( (SInt32)ePluginVersionNotFound );

		// Get the plugin configavail
		// if it does not exist then we use a default of "Not Available"
		cfsConfigAvail = (CFStringRef)::CFDictionaryGetValue( plInfo, kPluginConfigAvailStr );
		if (cfsConfigAvail)
		{
			callocLength = (UInt32) CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfsConfigAvail), kCFStringEncodingUTF8) + 1;
			pPIConfigAvail = (char *)::calloc( 1, callocLength );
			if ( pPIConfigAvail == nil ) throw( (SInt32)eMemoryError );
			// Convert it to a regular 'C' string 
			bGotIt = CFStringGetCString( cfsConfigAvail, pPIConfigAvail, callocLength, kCFStringEncodingUTF8 );
			if (bGotIt == false) throw( (SInt32)ePluginConfigAvailNotFound );
		}
		else
		{
			pPIConfigAvail = strdup("Not Available");
		}

		// Get the plugin configfile
		// if it does not exist then we use a default of "Not Available"
		cfsConfigFile = (CFStringRef)::CFDictionaryGetValue( plInfo, kPluginConfigFileStr );
		if (cfsConfigFile)
		{
			callocLength = (UInt32) CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfsConfigFile), kCFStringEncodingUTF8) + 1;
			pPIConfigFile = (char *)::calloc( 1, callocLength );
			if ( pPIConfigFile == nil ) throw( (SInt32)eMemoryError );
			// Convert it to a regular 'C' string 
			bGotIt = CFStringGetCString( cfsConfigFile, pPIConfigFile, callocLength, kCFStringEncodingUTF8 );
			if (bGotIt == false) throw( (SInt32)ePluginConfigFileNotFound );
		}
		else
		{
			pPIConfigFile = strdup("Not Available");
		}

		// Get the plugin name
		cfsName = (CFStringRef)::CFDictionaryGetValue( plInfo, kPluginNameStr );
		if ( cfsName == nil ) throw( (SInt32)ePluginNameNotFound );

		callocLength = (UInt32) CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfsName), kCFStringEncodingUTF8) + 1;
		pPIName = (char *)::calloc( 1, callocLength );
		if ( pPIName == nil ) throw( (SInt32)eMemoryError );

		// Convert it to a regular 'C' string 
		bGotIt = CFStringGetCString( cfsName, pPIName, callocLength, kCFStringEncodingUTF8 );
		if (bGotIt == false) throw( (SInt32)ePluginNameNotFound );

		// Check for plugin handler
		if ( gPlugins == nil ) throw( (SInt32)ePluginHandlerNotLoaded );

		// Do we already have a plugin with this prefix registered?
		siResult = gPlugins->IsPresent( pPIName );
		if (siResult == eDSNoErr) throw( (SInt32)ePluginAlreadyLoaded );

#ifdef vDEBUB
	::printf( "Complete Info.plist is:\n" );
	::CFShow( plInfo );
	::fflush( stdout );
#endif

		cfaFactories = ::CFPlugInFindFactoriesForPlugInTypeInPlugIn( kModuleTypeUUID, plgThis );
		if ( cfaFactories == nil ) throw ( (SInt32)eNoPluginFactoriesFound );
		if (::CFArrayGetCount ( cfaFactories ) == 0) throw( (SInt32)eNoPluginFactoriesFound );

		cfuuidFactory = (CFUUIDRef)::CFArrayGetValueAtIndex( cfaFactories, 0 );

		try
		{
			fccPlugInSignature = ::rand();

			DbgLog( kLogApplication, "Loading plugin: \"%s\"", pPIName );

	// In order to do load plugins lazily, we want to know a couple of things:
	//	1) Is this plugin configured to be loaded lazily?
	//		We require that the plugin have the CFString "YES" as a value of key kPluginLazyNodesToRegStr ("DSOKToLoadLazily") in the
	//		plugin's plist.
	//
	//	2) Is this plugin configured to be disabled?  If so, we want to also load it lazily.
	//
	//	3) Does this plugin have a list of nodes in its plist that it wants us to publish
	//		for it?  If so, we will register those nodes on the plugin's behalf.
	//		
	//		Registering nodes in the plist requires the following data:
	//		a CFArray with key name kPluginLazyNodesToRegStr ("DSNodesToRegister").
	//			Each item in this array is a CFDictionary that represents a node.  Here are the types in that dictionary:
	//				kPluginNodeToRegisterType ("DSNodeToRegisterType")		which is a CFNumberRef (kCFNumberSInt32Type) matching the eDirNodeType mask
	//				kPluginNodeToRegisterPath ("DSNodeToRegisterPath")		which is a CFArray of CFStrings that represent the path.  Index 0 is the root
	//											of the path (i.e. "/BSD/Local" would be [BSD,Local]
	//	If not #1, we will just load this plugin like normal.  This way legacy plugins and 3rd party
	//	plugins will behave no differently.

#ifndef DISABLE_CONFIGURE_PLUGIN
			cfsOKToLoadPluginLazily = (CFStringRef)::CFDictionaryGetValue( plInfo, kPluginOKToLoadLazilyStr );
			
			if ( cfsOKToLoadPluginLazily && ::CFGetTypeID( cfsOKToLoadPluginLazily ) == ::CFStringGetTypeID() && CFStringCompare( (CFStringRef)cfsOKToLoadPluginLazily, CFSTR("YES"), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
				loadPluginLazily = true;
				
			if ( !loadPluginLazily )
				loadPluginLazily = ( gPluginConfig->GetPluginState( pPIName ) == kInactive );	// if not active, load lazily as well
#else
			loadPluginLazily = true; // always load lazily, we are now on-demand
#endif
			if ( loadPluginLazily )
			{
				gPlugins->AddPlugIn( pPIName, pPIVersion, pPIConfigAvail, pPIConfigFile, kAppleLoadedPlugin, fccPlugInSignature, cpPlugin, plgThis, cfuuidFactory, ulVers );
				SrvrLog( kLogApplication, "Plugin \"%s\", Version \"%s\", is set to load lazily.", pPIName, pPIVersion );

				// now we see if this plugin has any nodes it wants us to register on its behalf.
				cfaLazyNodesToRegister = (CFArrayRef)::CFDictionaryGetValue( plInfo, kPluginLazyNodesToRegStr );
				
				// This should either be a CFString with one node to register or a CFArray of CFStrings to register.
				
				if ( cfaLazyNodesToRegister && ::CFGetTypeID( cfaLazyNodesToRegister ) == ::CFArrayGetTypeID() )
				{
					CFIndex		numNodesToRegister = ::CFArrayGetCount( cfaLazyNodesToRegister );
					
					for ( CFIndex i=0; i<numNodesToRegister; i++ )
					{
						CFDictionaryRef		nodeDataRef		= (CFDictionaryRef)::CFArrayGetValueAtIndex( cfaLazyNodesToRegister, i );
						
						if ( nodeDataRef && ::CFGetTypeID( nodeDataRef ) == ::CFDictionaryGetTypeID() )
						{
							CFNumberRef		nodeTypeRef = (CFNumberRef)::CFDictionaryGetValue( nodeDataRef, kPluginNodeToRegisterType );
							CFArrayRef		nodePathRef = (CFArrayRef)::CFDictionaryGetValue( nodeDataRef, kPluginNodeToRegisterPath );
							
							if ( nodeTypeRef && ::CFGetTypeID( nodeTypeRef ) == ::CFNumberGetTypeID() && nodePathRef && ::CFGetTypeID( nodePathRef ) == ::CFArrayGetTypeID() )
							{
								CFIndex				numPieces = ::CFArrayGetCount( nodePathRef );
								tDataListPtr		nodeToRegisterList = ::dsDataListAllocatePriv();
								char				cBuf[1024];
								CFStringRef			curStringRef = nil;
								eDirNodeType		dirNodeType;
								
								if ( !nodeToRegisterList )
									throw( (SInt32)eMemoryError );
								
								for ( CFIndex j=0; j<numPieces; j++ )
								{
									curStringRef = (CFStringRef)::CFArrayGetValueAtIndex( nodePathRef, j );
									
									if ( curStringRef && ::CFGetTypeID( curStringRef ) == ::CFStringGetTypeID() )
									{
										if ( CFStringGetCString( curStringRef, cBuf, sizeof(cBuf), kCFStringEncodingUTF8 ) )
										{
											::dsAppendStringToListAllocPriv( nodeToRegisterList, cBuf );
										}
										else
										{
											ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has data too large (> %d bytes!) for node %d in key DSNodeToRegisterType or DSNodeToRegisterPath", sizeof(cBuf), pPIName, j );
											::dsDataListDeallocatePriv( nodeToRegisterList );
											free( nodeToRegisterList );
											nodeToRegisterList = nil;
											throw((SInt32)ePlugInInitError);
										}
									}
									else
									{
										ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has malformed plist data for node %d in key DSNodeToRegisterPath", pPIName, j );
										::dsDataListDeallocatePriv( nodeToRegisterList );
										free( nodeToRegisterList );
										nodeToRegisterList = nil;
										throw((SInt32)ePlugInInitError);
									}
								}
								
								if ( CFNumberGetValue( nodeTypeRef, kCFNumberSInt32Type, &dirNodeType ) )
								{
									CServerPlugin::InternalRegisterNode( fccPlugInSignature, nodeToRegisterList, dirNodeType, true );	// we are proxing the plugin
								}
								else
									ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has malformed plist data for node %d in key DSNodeToRegisterType", pPIName, i );

								::dsDataListDeallocatePriv( nodeToRegisterList );
								free( nodeToRegisterList );
								nodeToRegisterList = nil;
							}
							else
							{
								ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has malformed plist data for node %d in key DSNodeToRegisterType or DSNodeToRegisterPath", pPIName, i );
								throw((SInt32)ePlugInInitError);
							}
						}
						else
						{
							ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has malformed plist data for node %d in key DSNodesToRegister", pPIName, i );
							throw((SInt32)ePlugInInitError);
						}
					} // for i<numNodesToRegister
				}
				else if ( cfaLazyNodesToRegister )
				{
					ErrLog( kLogPlugin, "*** Error in Plugin: %s, it has malformed plist data for key %s", pPIName, kPluginLazyNodesToRegStr );
					throw((SInt32)ePlugInInitError);
				}
			}
			else
			{
				cpPlugin = new CServerPlugin( plgThis, cfuuidFactory, fccPlugInSignature, ulVers, pPIName );
				cpPlugin->Validate( pPIVersion, fccPlugInSignature );
				gPlugins->AddPlugIn( pPIName, pPIVersion, pPIConfigAvail, pPIConfigFile, kAppleLoadedPlugin, fccPlugInSignature, cpPlugin );
				SrvrLog( kLogApplication, "Plugin \"%s\", Version \"%s\", loaded successfully.", pPIName, pPIVersion );
			}
			
			DbgLog( kLogApplication, "Plugin \"%s\", Configure HI \"%s\", can be used to configure PlugIn with file \"%s\".", pPIName, pPIConfigAvail, pPIConfigFile );

			pPIName			= nil;
			pPIVersion		= nil;
			pPIConfigAvail	= nil;
			pPIConfigFile	= nil;
			cpPlugin		= nil;

			siResult = eDSNoErr;
		} // try

		catch( SInt32 err )
		{
			ErrLog( kLogPlugin, "*** Error in Plugin path = %s.", path );
			ErrLog( kLogPlugin, "*** Error attempting to load plugin %s.  Error = %d", pPIName, err );
		}
	}

	catch( SInt32 err )
	{
		siResult = err;
	}
	
	catch( ... )
	{
		DbgLog( kLogApplication, "CServerPlugin::ProcessURL just caught a throw that was UNEXPECTED" );
		SrvrLog( kLogApplication, "CServerPlugin::ProcessURL just caught a throw that was UNEXPECTED" );
		siResult = ePlugInNotFound;
	}

	if ( pPIConfigAvail != nil )
	{
		free( pPIConfigAvail );
		pPIConfigAvail = nil;
	}

	if ( pPIConfigFile != nil )
	{
		free( pPIConfigFile );
		pPIConfigFile = nil;
	}

	if ( pPIVersion != nil )
	{
		free( pPIVersion );
		pPIVersion = nil;
	}

	if ( pPIName != nil )
	{
		free( pPIName );
		pPIName = nil;
	}

	if ( plgThis != nil )
	{
		::CFRelease( plgThis );
		plgThis = nil;
	}

	if ( cfaFactories != nil )
	{
		::CFRelease( cfaFactories );
		cfaFactories = nil;
	}
	
#if DEBUG
	// Putting these here to flush any debugging output.
	::fflush( stdout );
	::fflush( stderr );
#endif

	return( siResult );

} // ProcessURL


// ----------------------------------------------------------------------------
//	* Validate()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::Validate ( const char *inVersionStr, const UInt32 inSignature )
{
	if (mInstance != NULL)
		return( mInstance->validate( mInstance, inVersionStr, inSignature ) );
	
	fPlugInSignature = inSignature;
	return eDSNoErr;

} // Validate


// ----------------------------------------------------------------------------
//	* Initialize ()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::Initialize ( void )
{
	if (mInstance != NULL)
		return( mInstance->initialize( mInstance ) );
	
	return eDSNoErr;
} // Initialize


// --------------------------------------------------------------------------------
//	* Configure ()
// --------------------------------------------------------------------------------

SInt32 CServerPlugin::Configure ( void )
{
	//return( mInstance->configure( mInstance ) );
    return(eDSNoErr);
} // Configure


// ----------------------------------------------------------------------------
//	* SetPluginState()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::SetPluginState ( const UInt32 inState )
{
	if (mInstance != NULL)
		return( mInstance->setPluginState( mInstance, inState ) );
	
	return eNotHandledByThisNode;
}  // SetPluginState


// ----------------------------------------------------------------------------
//	* PeriodicTask ()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::PeriodicTask ( void )
{
	if (mInstance != NULL)
		return( mInstance->periodicTask( mInstance ) );
	
	return eDSNoErr;
} // PeriodicTask


// ----------------------------------------------------------------------------
//	* ProcessRequest()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::ProcessRequest ( void *inData )
{
	if (mInstance != NULL)
        return( mInstance->processRequest( mInstance, inData ) );
	
	return eNotHandledByThisNode;
}  // ProcessRequest


// --------------------------------------------------------------------------------
//	* Shutdown ()
// --------------------------------------------------------------------------------

SInt32 CServerPlugin::Shutdown ( void )
{
	//return( mInstance->shutdown( mInstance ) );
    return(eDSNoErr);
} // Shutdown


// ----------------------------------------------------------------------------
//	* GetPluginName()
//
// ----------------------------------------------------------------------------

char* CServerPlugin::GetPluginName ( void )
{
	return( fPlugInName );
}  // GetPluginName


// ----------------------------------------------------------------------------
//	* GetSignature()
//
// ----------------------------------------------------------------------------

FourCharCode CServerPlugin::GetSignature ( void )
{
	return( fPlugInSignature );
}  // GetSignature


// ----------------------------------------------------------------------------
//	* ProcessStaticPlugin()
//
// ----------------------------------------------------------------------------

SInt32 CServerPlugin::ProcessStaticPlugin ( const char* inPluginName, const char* inPluginVersion )
{
	SInt32				siResult			= -1;
	CServerPlugin	   *cpPlugin			= nil;
	FourCharCode		fccPlugInSignature	= 0;

	try
	{
        fccPlugInSignature = ::rand();

        //need to decide which static plugin to create here
        if (strcmp(inPluginName,"Cache") == 0)
        {
#ifndef DISABLE_CACHE_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new CCachePlugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        else if (strcmp(inPluginName,"Configure") == 0)
        {
#ifndef DISABLE_CONFIGURE_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new CConfigurePlugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        else if (strcmp(inPluginName,"Local") == 0)
        {
#ifndef DISABLE_LOCAL_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new CDSLocalPlugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        else if (strcmp(inPluginName,"LDAPv3") == 0)
        {
#ifndef DISABLE_LDAPV3_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new CLDAPv3Plugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        else if (strcmp(inPluginName,"Search") == 0)
        {
#ifndef DISABLE_SEARCH_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new CSearchPlugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        else if (strcmp(inPluginName,"BSD") == 0)
        {
#ifndef DISABLE_BSD_PLUGIN
			DbgLog( kLogApplication, "Processing <%s> plugin.", inPluginName );
            cpPlugin = new BSDPlugin( fccPlugInSignature, inPluginName );
#else
			return ePlugInNotFound;
#endif
        }
        if ( cpPlugin != nil )
        {
            gPlugins->AddPlugIn( inPluginName, inPluginVersion, "Not Available", "Not Available", kStaticPlugin, fccPlugInSignature, cpPlugin );

            cpPlugin->Validate( inPluginVersion, fccPlugInSignature );

            SrvrLog( kLogApplication, "Plugin <%s>, Version <%s>, processed successfully.", inPluginName, inPluginVersion );
        }
        else
        {
            ErrLog( kLogApplication, "ERROR: <%s> plugin _FAILED_ to process.", inPluginName );
        }

        siResult = eDSNoErr;
	}

	catch( SInt32 err )
	{
		siResult = err;
        ErrLog( kLogPlugin, "*** Error attempting to process <%s> plugin .  Error = %ld", inPluginName, err );
	}

	return( siResult );

} // ProcessStaticPlugin


