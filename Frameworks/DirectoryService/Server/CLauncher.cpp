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
 * @header CLauncher
 */

#include "CLauncher.h"
#include "ServerControl.h"
#include "DSCThread.h"
#include "PrivateTypes.h"
#include "CLog.h"
#include "CServerPlugin.h"
#include "PluginData.h"
#include "CPlugInList.h"
#include "CRefTable.h"
#include "CPlugInList.h"
#include "CServerPlugin.h"
#include "SharedConsts.h"
#include "CPluginConfig.h"
#include "CInternalDispatch.h"
#include <CoreFoundation/CFRunLoop.h>

#include <servers/bootstrap.h>
#include <time.h>

// --------------------------------------------------------------------------------
// * Globals
// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// * Externs
// --------------------------------------------------------------------------------
extern CFRunLoopRef			gPluginRunLoop;
#ifndef DISABLE_CONFIGURE_PLUGIN
extern CPluginConfig	   *gPluginConfig;
#endif
extern DSMutexSemaphore    *gKerberosMutex;

//--------------------------------------------------------------------------------------------------
// * CLauncher()
//
//--------------------------------------------------------------------------------------------------

CLauncher::CLauncher ( CServerPlugin *inPlugin ) : DSCThread(kTSLauncherThread)
{
	fThreadSignature = kTSLauncherThread;

	if ( inPlugin == nil )
	{
		ErrLog( kLogApplication, "Launcher failed create with no plugin pointer provided" );
		throw((SInt32)eParameterError);
	}

	fPlugin = inPlugin;

} // CLauncher



//--------------------------------------------------------------------------------------------------
// * ~CLauncher()
//
//--------------------------------------------------------------------------------------------------

CLauncher::~CLauncher()
{
} // ~CLauncher



//--------------------------------------------------------------------------------------------------
// * StartThread()
//
//--------------------------------------------------------------------------------------------------

void CLauncher::StartThread ( void )
{
	if ( this == nil )
	{
		ErrLog( kLogApplication, "Launcher StartThread failed with memory error on itself" );
		throw((SInt32)eMemoryError);
	}

	this->Resume();
} // StartThread



//--------------------------------------------------------------------------------------------------
// * StopThread()
//
//--------------------------------------------------------------------------------------------------

void CLauncher::StopThread ( void )
{
	if ( this == nil )
	{
		ErrLog( kLogApplication, "Launcher StopThread failed with memory error on itself" );
		throw((SInt32)eMemoryError);
	}

	// Check that the current thread context is not our thread context

	SetThreadRunState( kThreadStop );		// Tell our thread to stop

} // StopThread


//--------------------------------------------------------------------------------------------------
// * ThreadMain()
//
//--------------------------------------------------------------------------------------------------

SInt32 CLauncher::ThreadMain ( void )
{
    			bool		done		= false;
				SInt32		siResult 	= eDSNoErr;
				UInt32		uiCntr		= 0;
				UInt32		uiAttempts	= 100;
    volatile	UInt32		uiWaitTime 	= 1;
				sHeader		aHeader;
				ePluginState		pluginState	= kUnknownState;

	CInternalDispatch::AddCapability();

	if ( gPlugins != nil )
	{
		while ( !done )
		{
			uiCntr++;

			// Attempt to initialize it
			siResult = fPlugin->Initialize();
			if ( ( siResult != eDSNoErr ) && ( uiCntr == 1 ) )
			{
				ErrLog( kLogApplication, "Attempt #%l to initialize plug-in %s failed.\n  Will retry initialization at most 100 times every %l second.", uiCntr, fPlugin->GetPluginName(), uiWaitTime );
				DbgLog( kLogApplication, "Attempt #%l to initialize plug-in %s failed.\n  Will retry initialization at most 100 times every %l second.", uiCntr, fPlugin->GetPluginName(), uiWaitTime );
			}
			
			if ( siResult == eDSNoErr )
			{
				DbgLog( kLogApplication, "Initialization of plug-in %s succeeded with #%l attempt.", fPlugin->GetPluginName(), uiCntr );

				gPlugins->SetState( fPlugin->GetPluginName(), kInitialized );

				//provide the CFRunLoop to the plugins that need it
				if (gPluginRunLoop != NULL)
				{
					aHeader.fType			= kServerRunLoop;
					aHeader.fResult			= eDSNoErr;
					aHeader.fContextData	= (void *)gPluginRunLoop;
					siResult = fPlugin->ProcessRequest( (void*)&aHeader ); //don't handle return
				}
				
				// provide the Kerberos Mutex to plugins that need it
				if (gKerberosMutex != NULL)
				{
					aHeader.fType			= kKerberosMutex;
					aHeader.fResult			= eDSNoErr;
					aHeader.fContextData	= (void *)gKerberosMutex;
					fPlugin->ProcessRequest( (void*)&aHeader ); // don't handle return
				}

#ifndef DISABLE_CONFIGURE_PLUGIN
				pluginState = gPluginConfig->GetPluginState( fPlugin->GetPluginName() );
#else
				pluginState = kActive;
#endif
				if ( pluginState == kInactive )
				{
					siResult = fPlugin->SetPluginState( kInactive );
					if ( siResult == eDSNoErr )
					{
						SrvrLog( kLogApplication, "Plug-in %s state is now inactive.", fPlugin->GetPluginName() );
				
						gPlugins->SetState( fPlugin->GetPluginName(), kInactive );
					}
					else
					{
						ErrLog( kLogApplication, "Unable to set %s plug-in state to inactive.  Received error %l.", fPlugin->GetPluginName(), siResult );
					}
				}
				else
				{
					// we assume the plugin is going to activate since we initialized successfully
					// this is because we expect they will start accepting answers the second we send to them
					siResult = gPlugins->SetState( fPlugin->GetPluginName(), kActive );
					if ( siResult == eDSNoErr )
					{
						SrvrLog( kLogApplication, "Plug-in %s state is now active.", fPlugin->GetPluginName() );
					}
					else
					{
						gPlugins->SetState( fPlugin->GetPluginName(), kInactive );
						ErrLog( kLogApplication, "Unable to set %s plug-in state to active.  Received error %l.", fPlugin->GetPluginName(), siResult );
					}
				}
				
				done = true;
			}

			if ( !done )
			{
				// We will try this 100 times before we bail
				if ( uiCntr == uiAttempts )
				{
					ErrLog( kLogApplication, "%l attempts to initialize plug-in %s failed.\n  Setting plug-in state to inactive.", uiCntr, fPlugin->GetPluginName() );

					siResult = gPlugins->SetState( fPlugin->GetPluginName(), kInactive | kFailedToInit );

					done = true;
				}
				else
				{
					fWaitToInit.WaitForEvent( uiWaitTime * kMilliSecsPerSec );
				}
			}
		}
	}
	
	return( 0 );

} // ThreadMain


