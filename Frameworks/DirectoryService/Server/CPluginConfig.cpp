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
 * @header CPluginConfig
 */

#include <sys/stat.h>				// for file and dir stat calls
#include <syslog.h>					// for syslog()
#include <CoreFoundation/CFDictionary.h>

#include "CPluginConfig.h"
#include "CServerPlugin.h"
#include "CPluginList.h"
#include "ServerControl.h"
#include "CFile.h"
#include "CLog.h"
#include "od_passthru.h"

extern	bool			gServerOS;
extern  UInt32			gRefCountWarningLimit;
extern  UInt32			gDelayFailedLocalAuthReturnsDeltaInSeconds;
extern	UInt32			gMaxHandlerThreadCount;

//--------------------------------------------------------------------------------------------------
//	* CPluginConfig ()
//
//--------------------------------------------------------------------------------------------------

CPluginConfig::CPluginConfig ( void )
{
	fPlistRef		= nil;
	fDictRef		= nil;
} // CPluginConfig


//--------------------------------------------------------------------------------------------------
//	* ~CPluginConfig ()
//
//--------------------------------------------------------------------------------------------------

CPluginConfig::~CPluginConfig()
{
	if ( fPlistRef != nil )
	{
		::CFRelease( fPlistRef );
		fPlistRef = nil;
	}
} // ~CPluginConfig



//--------------------------------------------------------------------------------------------------
//	* Initialize ()
//
//--------------------------------------------------------------------------------------------------

SInt32 CPluginConfig::Initialize ( void )
{
	bool			bSuccess	= false;
	int				iResult		= 0;
	UInt32			uiDataSize	= 0;
	char		   *pData		= nil;
	CFile		   *pFile		= nil;
	struct stat		statbuf;
	CFDataRef		dataRef		= nil;
	CFStringRef		keyStrRef	= nil;
	unsigned char   cfNumBool	= false;
	CFNumberRef		cfNumber	= 0;
	

	try
	{
		// Does the config file exist
		iResult = ::stat( kConfigFilePath, &statbuf );
		if ( iResult == 0 )
		{
			// Attempt to get config info from file
			pFile = new CFile( kConfigFilePath );
			if ( pFile != nil )
			{
				if ( (pFile->is_open()) && (pFile->FileSize() > 0) )
				{
					// Allocate space for the file data
					pData = (char *)::malloc( pFile->FileSize() + 1 );
					if ( pData != nil )
					{
						// Read from the config file
						uiDataSize = pFile->ReadBlock( pData, pFile->FileSize() );
						dataRef = ::CFDataCreate( nil, (const UInt8 *)pData, uiDataSize );
						if ( dataRef != nil )
						{
							// Is it valid XML data
							fPlistRef = ::CFPropertyListCreateFromXMLData( kCFAllocatorDefault, dataRef, kCFPropertyListMutableContainersAndLeaves, nil );
							if ( fPlistRef != nil )
							{
								// Is it a plist type
								if ( ::CFDictionaryGetTypeID() == ::CFGetTypeID( fPlistRef ) )
								{
									fDictRef = (CFMutableDictionaryRef)fPlistRef;

									bSuccess = true;
								}
							}
							CFRelease( dataRef );
							dataRef = nil;
						}
						free( pData );
						pData = nil;
					}
				}
				
				delete( pFile );
				pFile = nil;
			}
		}

		if ( bSuccess == true && fDictRef != NULL )
		{
			// Check versioning here.  If file version is old, we need to convert to the latest
			CFStringRef		configVersion = (CFStringRef)CFDictionaryGetValue( fDictRef, CFSTR(kVersionKey) );
			
			if ( configVersion == NULL )
			{
				// this isn't properly configured, throw this away so we can start with a fresh default
				CFRelease( fDictRef );
				fDictRef = NULL;
				bSuccess = false;

				SrvrLog( kLogApplication, "Plugin configuration file has no version, resetting to current defaults." );
			}
			else
			{
				Boolean		bSaveFile = false;
				
				CFRetain( configVersion );  // release at end
				
				if ( CFStringCompare( configVersion, CFSTR("1.1"), kCFCompareNumerically ) == kCFCompareLessThan )
				{
					// This is a pre 1.1 version file, upgrade to 1.1 defaults
					// First the version value
					CFDictionarySetValue( fDictRef, CFSTR(kVersionKey), CFSTR("1.1") );
					
					// Now the change, we want to force AppleTalk plugin to be active by default
					CFDictionarySetValue( fDictRef, CFSTR(kAppleTalkPluginKey), CFSTR(kActiveValue) );
					
					bSaveFile = true;
					SrvrLog( kLogApplication, "Plugin configuration file upgraded to 1.1, AppleTalk plugin now Active." );
				}
/*				
				if ( CFStringCompare( configVersion, CFSTR("1.2"), kCFCompareNumerically ) == kCFCompareLessThan )
				{
					// This is a pre 1.2 version file, upgrade from 1.1 to 1.2 defaults
					...
					// This is a placeholder for future versioning changes.  If we always check the version, we can
					// do the same upgrades between versions so that whatever the current version state is will be
					// consistent.

					bUpgradedConfigVersion = true;
					SrvrLog( kLogApplication, "Plugin configuration file upgraded to 1.2." );
				}
*/

				if ( bSaveFile == true ) {
					SaveConfigData();
				}
				
				CFRelease( configVersion );  // release at end
			}
		}
		
		// Either the file didn't exist or we didn't have valid data in the file
		if ( bSuccess == false )
		{
			// Does the Jaguar Update file exist indicating this was an upgrade install?
			// We don't make AppleTalk inactive by default on an upgrade install
			iResult = ::stat( kJaguarUpdateFilePath, &statbuf );
			if (gServerOS)
			{
				if (iResult == 0)
				{
					uiDataSize = ::strlen( kServerDefaultUpgradeConfig );
					dataRef = ::CFDataCreate( nil, (const UInt8 *)kServerDefaultUpgradeConfig, uiDataSize );
				}
				else
				{
					uiDataSize = ::strlen( kServerDefaultConfig );
					dataRef = ::CFDataCreate( nil, (const UInt8 *)kServerDefaultConfig, uiDataSize );
				}
			}
			else
			{
				if (iResult == 0)
				{
					uiDataSize = ::strlen( kDefaultUpgradeConfig );
					dataRef = ::CFDataCreate( nil, (const UInt8 *)kDefaultUpgradeConfig, uiDataSize );
				}
				else
				{
					uiDataSize = ::strlen( kDefaultConfig );
					dataRef = ::CFDataCreate( nil, (const UInt8 *)kDefaultConfig, uiDataSize );
				}
			}
			if ( dataRef != nil )
			{
				fPlistRef = (CFMutableDictionaryRef)::CFPropertyListCreateFromXMLData( kCFAllocatorDefault, dataRef, kCFPropertyListMutableContainersAndLeaves, nil );
				if ( fPlistRef != nil )
				{
					if ( ::CFDictionaryGetTypeID() == ::CFGetTypeID( fPlistRef ) )
					{
						bool bSaveFile = false;
						
						fDictRef = (CFMutableDictionaryRef)fPlistRef;

						//make the new Active Directory plugin InActive by default if not already installed and setup
						if ( CFDictionaryContainsKey(fDictRef, CFSTR("Active Directory")) == false ) {
							CFDictionarySetValue( fDictRef, CFSTR("Active Directory"), CFSTR(kInactiveValue) );
							bSaveFile = true;
						}
						
						if ( bSaveFile == true ) {
							//let's make sure we don't run into these non-config file problems again
							//ie. write the file since we have it figured out
							SaveConfigData();
						}
						
						bSuccess = true;
					}
				}
				CFRelease( dataRef );
				dataRef = nil;
			}
		}

		if (fDictRef != nil)
		{
			bool bSaveFile = false;
			
			// we need to ensure BSD, Local and Cache are always enabled by default
			CFStringRef cfTemp = (CFStringRef) CFDictionaryGetValue( fDictRef, CFSTR("BSD") );
			if ( cfTemp == NULL || CFEqual( cfTemp, CFSTR(kInactiveValue) ) == true )
			{
				CFDictionarySetValue( fDictRef, CFSTR("BSD"), CFSTR(kActiveValue) );
				bSaveFile = true;
			}
			
			cfTemp = (CFStringRef) CFDictionaryGetValue( fDictRef, CFSTR("Local") );
			if ( cfTemp == NULL || CFEqual( cfTemp, CFSTR(kInactiveValue) ) == true )
			{
				CFDictionarySetValue( fDictRef, CFSTR("Local"), CFSTR(kActiveValue) );
				bSaveFile = true;
			}
			
			cfTemp = (CFStringRef) CFDictionaryGetValue( fDictRef, CFSTR("Cache") );
			if ( cfTemp == NULL || CFEqual( cfTemp, CFSTR(kInactiveValue) ) == true )
			{
				CFDictionarySetValue( fDictRef, CFSTR("Cache"), CFSTR(kActiveValue) );
				bSaveFile = true;
			}
			
			if ( bSaveFile == true ) {
				SaveConfigData();
			}
			
			keyStrRef = ::CFStringCreateWithCString( NULL, kTooManyReferencesWarningCount, kCFStringEncodingMacRoman );
			if ( keyStrRef != nil )
			{
				if ( CFDictionaryContainsKey( fDictRef, keyStrRef ) )
				{
					cfNumber = (CFNumberRef)CFDictionaryGetValue( fDictRef, keyStrRef );
					if ( cfNumber != nil )
					{
						cfNumBool = CFNumberGetValue(cfNumber, kCFNumberIntType, &gRefCountWarningLimit);
						//CFRelease(cfNumber); // no since pointer only from Get
					}
				}
				::CFRelease( keyStrRef );
				keyStrRef = nil;
			}
			keyStrRef = ::CFStringCreateWithCString( NULL, kDelayFailedLocalAuthReturnsDeltaInSeconds, kCFStringEncodingMacRoman );
			if ( keyStrRef != nil )
			{
				if ( CFDictionaryContainsKey( fDictRef, keyStrRef ) )
				{
					cfNumber = (CFNumberRef)CFDictionaryGetValue( fDictRef, keyStrRef );
					if ( cfNumber != nil )
					{
						cfNumBool = CFNumberGetValue(cfNumber, kCFNumberIntType, &gDelayFailedLocalAuthReturnsDeltaInSeconds);
						//CFRelease(cfNumber); // no since pointer only from Get
					}
				}
				::CFRelease( keyStrRef );
				keyStrRef = nil;
			}
			keyStrRef = ::CFStringCreateWithCString( NULL, kMaxHandlerThreadCount, kCFStringEncodingMacRoman );
			if ( keyStrRef != nil )
			{
				if ( CFDictionaryContainsKey( fDictRef, keyStrRef ) )
				{
					cfNumber = (CFNumberRef)CFDictionaryGetValue( fDictRef, keyStrRef );
					if ( cfNumber != nil )
					{
						cfNumBool = CFNumberGetValue(cfNumber, kCFNumberIntType, &gMaxHandlerThreadCount);
						//CFRelease(cfNumber); // no since pointer only from Get
						if (gMaxHandlerThreadCount < kMaxHandlerThreads)
						{
							gMaxHandlerThreadCount = kMaxHandlerThreads;
							syslog(LOG_ALERT,"Maximum handler thread count cannot be set less than %u", kMaxHandlerThreads);
						}
						else if (gMaxHandlerThreadCount > 256)
						{
							gMaxHandlerThreadCount = kMaxHandlerThreads;
							syslog(LOG_ALERT,"Maximum handler thread count cannot be set greater than 256 so resetting to default of %u", kMaxHandlerThreads);
						}
					}
				}
				::CFRelease( keyStrRef );
				keyStrRef = nil;
			}
		}
	}
	
	catch ( ... )
	{
	}

	return( eDSNoErr );

} // Initialize

//--------------------------------------------------------------------------------------------------
//	* GetPluginState ()
//
//--------------------------------------------------------------------------------------------------

ePluginState CPluginConfig::GetPluginState ( const char *inPluginName )
{
	ePluginState	epsResult	= kActive;		// If we don't explicitly have a plugin turned off, it should be active
	bool			bFound		= false;
	CFStringRef		cfStringRef	= nil;
	char		   *pValue		= nil;
	CFStringRef		keyStrRef	= nil;

	if (  (fDictRef != nil) && (inPluginName != nil) )
	{
		keyStrRef = ::CFStringCreateWithCString( NULL, inPluginName, kCFStringEncodingMacRoman );
		if ( keyStrRef != nil )
		{
			bFound = ::CFDictionaryContainsKey( fDictRef, keyStrRef );
			if ( bFound == true )
			{
				cfStringRef = (CFStringRef)::CFDictionaryGetValue( fDictRef, keyStrRef );
				if ( cfStringRef != nil )
				{
					if ( ::CFGetTypeID( cfStringRef ) == ::CFStringGetTypeID() )
					{
						pValue = (char *)::CFStringGetCStringPtr( cfStringRef, kCFStringEncodingMacRoman );
						if ( pValue != nil )
						{
							if ( ::strcmp( pValue, kInactiveValue ) == 0 )
							{
								epsResult = kInactive;
							}
						}
					}
				}
			}
			::CFRelease( keyStrRef );
			keyStrRef = nil;
		}
	}

	return( epsResult );

} // GetPluginState


//--------------------------------------------------------------------------------------------------
//	* SetPluginState ()
//
//--------------------------------------------------------------------------------------------------

SInt32 CPluginConfig::SetPluginState ( const char *inPluginName, const ePluginState inPluginState )
{
	CFStringRef		keyStrRef		= nil;

	// set the prefs for the plugin, but don't change the state of BSD, Cache, or Local
	if ( (fDictRef != nil) && (inPluginName != nil) && (strcmp(inPluginName, "BSD") != 0) && (strcmp(inPluginName, "Cache") != 0) &&
		 (strcmp(inPluginName, "Local") != 0) )
	{
		CServerPlugin*	plugin = gPlugins->GetPlugInPtr( inPluginName, false );		// don't load plugin if it isn't already...
		
		if ( plugin )
			plugin->SetPluginState( inPluginState );
			
		keyStrRef = ::CFStringCreateWithCString( NULL, inPluginName, kCFStringEncodingMacRoman );
		if ( keyStrRef != nil )
		{
			if ( inPluginState == kActive )
			{
				od_passthru_set_plugin_enabled(inPluginName, true);
				::CFDictionarySetValue( fDictRef, keyStrRef, CFSTR( kActiveValue ) );
			}
			else if ( inPluginState == kInactive )
			{
				od_passthru_set_plugin_enabled(inPluginName, false);
				::CFDictionarySetValue( fDictRef, keyStrRef, CFSTR( kInactiveValue ) );
			}

			::CFRelease( keyStrRef );
			keyStrRef = nil;
		}
		
		return eDSNoErr;
	}

	return eDSOperationFailed;

} // SetPluginState


//--------------------------------------------------------------------------------------------------
//	* SaveConfigData ()
//
//--------------------------------------------------------------------------------------------------

SInt32 CPluginConfig::SaveConfigData ( void )
{
	CFDataRef dataRef = nil;
	int result = 0;;
	struct stat	statResult;

	if ( fDictRef != nil )
	{
		// Get a new data ref with data in dictionary
		dataRef = ::CFPropertyListCreateXMLData( NULL, fDictRef );
		if ( dataRef != nil )
		{
			//step 1- see if the file exists
			//if not then make sure the directories exist or create them
			//then create a new file if necessary
			result = ::stat( kConfigFilePath, &statResult );
			//if file does not exist
			if (result != eDSNoErr)
				result = dsCreatePrefsDirectory();

			const UInt8 *pData = CFDataGetBytePtr( dataRef );
			CFIndex dataLen = CFDataGetLength( dataRef );
			if ( pData != NULL && dataLen > 0 )
			{
				try
				{
					CFile *pFile = new CFile( kConfigFilePath, true );
					if ( pFile != nil )
					{
						if ( pFile->is_open() )
						{
							pFile->seteof( 0 );
							pFile->write( pData, dataLen );
						}
						
						delete( pFile );
						pFile = nil;
						
						chmod( kConfigFilePath, 0600 );
					}
				}
				catch ( ... )
				{
				}
			}

			CFRelease( dataRef );
			dataRef = nil;
		}
	}

	return( eDSNoErr );

} // SaveConfigData
