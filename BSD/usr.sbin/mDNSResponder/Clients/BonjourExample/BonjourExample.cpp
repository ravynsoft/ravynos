/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include	"stdafx.h"

#include	<assert.h>
#include	<stdio.h>

#include	"dns_sd.h"

// Constants

#define BONJOUR_EVENT		( WM_USER + 0x100 )	// Message sent to the Window when a Bonjour event occurs.

// Prototypes

static LRESULT CALLBACK WndProc( HWND inWindow, UINT inMsg, WPARAM inWParam, LPARAM inLParam );

static void DNSSD_API
	BrowserCallBack( 
		DNSServiceRef		inServiceRef, 
		DNSServiceFlags		inFlags,
		uint32_t			inIFI, 
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain, 
		void *				inContext );

// Globals

DNSServiceRef		gServiceRef = NULL;

// Main entry point for application.

int _tmain( int argc, _TCHAR *argv[] )
{
	HINSTANCE				instance;
	WNDCLASSEX				wcex;
	HWND					wind;
	MSG						msg;
	DNSServiceErrorType		err;
	
	(void) argc; // Unused
	(void) argv; // Unused
	
	// Create the window. This window won't actually be shown, but it demonstrates how to use Bonjour
	// with Windows GUI applications by having Bonjour events processed as messages to a Window.
	
	instance = GetModuleHandle( NULL );
	assert( instance );
	
	wcex.cbSize			= sizeof( wcex );
	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= instance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT( "BonjourExample" );
	wcex.hIconSm		= NULL;
	RegisterClassEx( &wcex );
	
	wind = CreateWindow( wcex.lpszClassName, wcex.lpszClassName, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 
		0, NULL, NULL, instance, NULL );
	assert( wind );
	
	// Start browsing for services and associate the Bonjour browser with our window using the 
	// WSAAsyncSelect mechanism. Whenever something related to the Bonjour browser occurs, our 
	// private Windows message will be sent to our window so we can give Bonjour a chance to 
	// process it. This allows Bonjour to avoid using a secondary thread (and all the issues 
	// with synchronization that would introduce), but still process everything asynchronously.
	// This also simplifies app code because Bonjour will only run when we explicitly call it.
	
	err = DNSServiceBrowse( 
		&gServiceRef, 					// Receives reference to Bonjour browser object.
		0,								// No flags.
		kDNSServiceInterfaceIndexAny,	// Browse on all network interfaces.
		"_http._tcp",					// Browse for HTTP service types.
		NULL,							// Browse on the default domain (e.g. local.).
		BrowserCallBack, 				// Callback function when Bonjour events occur.
		NULL );							// No callback context needed.
	assert( err == kDNSServiceErr_NoError );
	
	err = WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( gServiceRef ), wind, BONJOUR_EVENT, FD_READ | FD_CLOSE );
	assert( err == kDNSServiceErr_NoError );
	
	fprintf( stderr, "Browsing for _http._tcp\n" );
	
	// Main event loop for the application. All Bonjour events are dispatched while in this loop.
	
	while( GetMessage( &msg, NULL, 0, 0 ) ) 
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	
	// Clean up Bonjour. This is not strictly necessary since the normal process cleanup will 
	// close Bonjour socket(s) and release memory, but it's here to demonstrate how to do it.
	
	if( gServiceRef )
	{
		WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( gServiceRef ), wind, BONJOUR_EVENT, 0 );
		DNSServiceRefDeallocate( gServiceRef );
	}
	return( 0 );
}

// Callback for the Window. Bonjour events are delivered here.

static LRESULT CALLBACK WndProc( HWND inWindow, UINT inMsg, WPARAM inWParam, LPARAM inLParam )
{
	LRESULT					result;
	DNSServiceErrorType		err;
	
	switch( inMsg )
	{
		case BONJOUR_EVENT:
			
			// Process the Bonjour event. All Bonjour callbacks occur from within this function.
			// If an error occurs while trying to process the result, it most likely means that
			// something serious has gone wrong with Bonjour, such as it being terminated. This 
			// does not normally occur, but code should be prepared to handle it. If the error 
			// is ignored, the window will receive a constant stream of BONJOUR_EVENT messages so
			// if an error occurs, we disassociate the DNSServiceRef from the window, deallocate
			// it, and invalidate the reference so we don't try to deallocate it again on quit. 
			// Since this is a simple example app, if this error occurs, we quit the app too.
			
			err = DNSServiceProcessResult( gServiceRef );
			if( err != kDNSServiceErr_NoError )
			{
				fprintf( stderr, "### ERROR! serious Bonjour error: %d\n", err );
				
				WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( gServiceRef ), inWindow, BONJOUR_EVENT, 0 );
				DNSServiceRefDeallocate( gServiceRef );
				gServiceRef = NULL;
				
				PostQuitMessage( 0 );
			}
			result = 0;
			break;
		
		default:
			result = DefWindowProc( inWindow, inMsg, inWParam, inLParam );
			break;
	}
	return( result );
}

// Callback for Bonjour browser events. Called when services are added or removed.

static void DNSSD_API
	BrowserCallBack( 
		DNSServiceRef		inServiceRef, 
		DNSServiceFlags		inFlags,
		uint32_t			inIFI, 
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain, 
		void *				inContext )
{
	(void) inServiceRef;	// Unused
	(void) inContext;		// Unused
	
	if( inError == kDNSServiceErr_NoError )
	{
		const char *		action;
		const char *		more;
		
		if( inFlags & kDNSServiceFlagsAdd )			action	= "ADD";
		else										action	= "RMV";
		if( inFlags & kDNSServiceFlagsMoreComing )	more	= " (MORE)";
		else										more	= "";
		
		fprintf( stderr, "%s %30s.%s%s on interface %d%s\n", action, inName, inType, inDomain, (int) inIFI, more );
	}
	else
	{
		fprintf( stderr, "Bonjour browser error occurred: %d\n", inError );
	}
}
