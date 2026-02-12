/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc. All rights reserved.
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

#include	"StdAfx.h"

#include	"CommonServices.h"
#include	"DebugServices.h"
#include	"WinServices.h"
#include	"dns_sd.h"

#include	"ExplorerBar.h"
#include	"LoginDialog.h"
#include	"Resource.h"

#include	"ExplorerBarWindow.h"
#include	"ExplorerPlugin.h"

// MFC Debugging

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 0
#pragma mark == Constants ==
#endif

//===========================================================================================================================
//	Constants
//===========================================================================================================================

// Control IDs

#define	IDC_EXPLORER_TREE				1234

// Private Messages

#define WM_PRIVATE_SERVICE_EVENT				( WM_USER + 0x100 )

// TXT records

#define	kTXTRecordKeyPath				"path"

// IE Icon resource

#define kIEIconResource					32529


#if 0
#pragma mark == Prototypes ==
#endif

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

DEBUG_LOCAL int			FindServiceArrayIndex( const ServiceInfoArray &inArray, const ServiceInfo &inService, int &outIndex );

#if 0
#pragma mark == Message Map ==
#endif

//===========================================================================================================================
//	Message Map
//===========================================================================================================================

BEGIN_MESSAGE_MAP( ExplorerBarWindow, CWnd )
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY( NM_DBLCLK, IDC_EXPLORER_TREE, OnDoubleClick )
	ON_MESSAGE( WM_PRIVATE_SERVICE_EVENT, OnServiceEvent )
END_MESSAGE_MAP()

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	ExplorerBarWindow
//===========================================================================================================================

ExplorerBarWindow::ExplorerBarWindow( void )
{
	mOwner				= NULL;
	mResolveServiceRef	= NULL;
}

//===========================================================================================================================
//	~ExplorerBarWindow
//===========================================================================================================================

ExplorerBarWindow::~ExplorerBarWindow( void )
{
	//
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	OnCreate
//===========================================================================================================================

int	ExplorerBarWindow::OnCreate( LPCREATESTRUCT inCreateStruct )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	HINSTANCE		module = NULL;
	OSStatus		err;
	CRect			rect;
	CBitmap			bitmap;
	CString			s;
	
	err = CWnd::OnCreate( inCreateStruct );
	require_noerr( err, exit );
	
	GetClientRect( rect );
	mTree.Create( WS_TABSTOP | WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_NOHSCROLL , rect, this, 
		IDC_EXPLORER_TREE );
	
	ServiceHandlerEntry *		e;
	
	s.LoadString( IDS_ABOUT );
	m_about = mTree.InsertItem( s, 0, 0 );

	// Web Site Handler
	
	e = new ServiceHandlerEntry;
	check( e );
	e->type				= "_http._tcp";
	e->urlScheme		= "http://";
	e->ref				= NULL;
	e->obj				= this;
	e->needsLogin		= false;
	mServiceHandlers.Add( e );

	err = DNSServiceBrowse( &e->ref, 0, 0, e->type, NULL, BrowseCallBack, e );
	require_noerr( err, exit );

	err = WSAAsyncSelect((SOCKET) DNSServiceRefSockFD(e->ref), m_hWnd, WM_PRIVATE_SERVICE_EVENT, FD_READ|FD_CLOSE);
	require_noerr( err, exit );

	m_serviceRefs.push_back(e->ref);

#if defined( _BROWSE_FOR_HTTPS_ )
	e = new ServiceHandlerEntry;
	check( e );
	e->type				= "_https._tcp";
	e->urlScheme		= "https://";
	e->ref				= NULL;
	e->obj				= this;
	e->needsLogin		= false;
	mServiceHandlers.Add( e );

	err = DNSServiceBrowse( &e->ref, 0, 0, e->type, NULL, BrowseCallBack, e );
	require_noerr( err, exit );

	err = WSAAsyncSelect((SOCKET) DNSServiceRefSockFD(e->ref), m_hWnd, WM_PRIVATE_SERVICE_EVENT, FD_READ|FD_CLOSE);
	require_noerr( err, exit );

	m_serviceRefs.push_back(e->ref);
#endif
	
	m_imageList.Create( 16, 16, ILC_MASK | ILC_COLOR16, 2, 0);

	bitmap.Attach( ::LoadBitmap( GetNonLocalizedResources(), MAKEINTRESOURCE( IDB_LOGO ) ) );
	m_imageList.Add( &bitmap, (CBitmap*) NULL );
	bitmap.Detach();

	mTree.SetImageList(&m_imageList, TVSIL_NORMAL);
	
exit:

	if ( module )
	{
		FreeLibrary( module );
		module = NULL;
	}

	// Cannot talk to the mDNSResponder service. Show the error message and exit (with kNoErr so they can see it).
	if ( err )
	{
		if ( err == kDNSServiceErr_Firewall )
		{
			s.LoadString( IDS_FIREWALL );
		}
		else
		{
			s.LoadString( IDS_MDNSRESPONDER_NOT_AVAILABLE );
		}
		
		mTree.DeleteAllItems();
		mTree.InsertItem( s, 0, 0, TVI_ROOT, TVI_LAST );
		
		err = kNoErr;
	}

	return( err );
}

//===========================================================================================================================
//	OnDestroy
//===========================================================================================================================

void	ExplorerBarWindow::OnDestroy( void ) 
{
	// Stop any resolves that may still be pending (shouldn't be any).
	
	StopResolve();
	
	// Clean up the extant browses
	while (m_serviceRefs.size() > 0)
	{
		//
		// take the head of the list
		//
		DNSServiceRef ref = m_serviceRefs.front();

		//
		// Stop will remove it from the list
		//
		Stop( ref );
	}

	// Clean up the service handlers.
	
	int		i;
	int		n;
	
	n = (int) mServiceHandlers.GetSize();
	for( i = 0; i < n; ++i )
	{
		delete mServiceHandlers[ i ];
	}
	
	CWnd::OnDestroy();
}

//===========================================================================================================================
//	OnSize
//===========================================================================================================================

void	ExplorerBarWindow::OnSize( UINT inType, int inX, int inY ) 
{
	CWnd::OnSize( inType, inX, inY );
	mTree.MoveWindow( 0, 0, inX, inY );
}

//===========================================================================================================================
//	OnDoubleClick
//===========================================================================================================================

void	ExplorerBarWindow::OnDoubleClick( NMHDR *inNMHDR, LRESULT *outResult )
{
	HTREEITEM			item;
	ServiceInfo *		service;
	OSStatus			err;
	
	DEBUG_UNUSED( inNMHDR );
	
	item = mTree.GetSelectedItem();
	require( item, exit );
	
	// Tell Internet Explorer to go to the URL if it's about item
	
	if ( item == m_about )
	{
		CString url;

		check( mOwner );

		url.LoadString( IDS_ABOUT_URL );
		mOwner->GoToURL( url );
	}
	else
	{
		service = reinterpret_cast < ServiceInfo * > ( mTree.GetItemData( item ) );
		require_quiet( service, exit );
		
		err = StartResolve( service );
		require_noerr( err, exit );
	}

exit:
	*outResult = 0;
}


//===========================================================================================================================
//	OnServiceEvent
//===========================================================================================================================

LRESULT
ExplorerBarWindow::OnServiceEvent(WPARAM inWParam, LPARAM inLParam)
{
	if (WSAGETSELECTERROR(inLParam) && !(HIWORD(inLParam)))
    {
		dlog( kDebugLevelError, "OnServiceEvent: window error\n" );
    }
    else
    {
		SOCKET sock = (SOCKET) inWParam;

		// iterate thru list
		ServiceRefList::iterator it;

		for (it = m_serviceRefs.begin(); it != m_serviceRefs.end(); it++)
		{
			DNSServiceRef ref = *it;

			check(ref != NULL);

			if ((SOCKET) DNSServiceRefSockFD(ref) == sock)
			{
				DNSServiceErrorType err;

				err = DNSServiceProcessResult(ref);

				if (err != 0)
				{
					CString s;

					s.LoadString( IDS_MDNSRESPONDER_NOT_AVAILABLE );
					mTree.DeleteAllItems();
					mTree.InsertItem( s, 0, 0, TVI_ROOT, TVI_LAST );

					Stop(ref);
				}

				break;
			}
		}
	}

	return ( 0 );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	BrowseCallBack
//===========================================================================================================================

void DNSSD_API
	ExplorerBarWindow::BrowseCallBack(
		DNSServiceRef 			inRef,
		DNSServiceFlags 		inFlags,
		uint32_t 				inInterfaceIndex,
		DNSServiceErrorType 	inErrorCode,
		const char *			inName,	
		const char *			inType,	
		const char *			inDomain,	
		void *					inContext )
{
	ServiceHandlerEntry *		obj;
	ServiceInfo *				service;
	OSStatus					err;
	
	DEBUG_UNUSED( inRef );
	
	obj		=	NULL;
	service = NULL;
	
	require_noerr( inErrorCode, exit );
	obj = reinterpret_cast < ServiceHandlerEntry * > ( inContext );
	check( obj );
	check( obj->obj );
	
	//
	// set the UI to hold off on updates
	//
	obj->obj->mTree.SetRedraw(FALSE);

	try
	{
		service = new ServiceInfo;
		require_action( service, exit, err = kNoMemoryErr );
		
		err = UTF8StringToStringObject( inName, service->displayName );
		check_noerr( err );

		service->name = _strdup( inName );
		require_action( service->name, exit, err = kNoMemoryErr );
		
		service->type = _strdup( inType );
		require_action( service->type, exit, err = kNoMemoryErr );
		
		service->domain = _strdup( inDomain );
		require_action( service->domain, exit, err = kNoMemoryErr );
		
		service->ifi 		= inInterfaceIndex;
		service->handler	= obj;

		service->refs		= 1;
		
		if (inFlags & kDNSServiceFlagsAdd) obj->obj->OnServiceAdd   (service);
		else                               obj->obj->OnServiceRemove(service);
	
		service = NULL;
	}
	catch( ... )
	{
		dlog( kDebugLevelError, "BrowseCallBack: exception thrown\n" );
	}
	
exit:
	//
	// If no more coming, then update UI
	//
	if (obj && obj->obj && ((inFlags & kDNSServiceFlagsMoreComing) == 0))
	{
		obj->obj->mTree.SetRedraw(TRUE);
		obj->obj->mTree.Invalidate();
	}

	if( service )
	{
		delete service;
	}
}

//===========================================================================================================================
//	OnServiceAdd
//===========================================================================================================================

LONG	ExplorerBarWindow::OnServiceAdd( ServiceInfo * service )
{
	ServiceHandlerEntry *		handler;
	int							cmp;
	int							index;
	
	
	check( service );
	handler = service->handler; 
	check( handler );
	
	cmp = FindServiceArrayIndex( handler->array, *service, index );
	if( cmp == 0 )
	{
		// Found a match so update the item. The index is index + 1 so subtract 1.
		
		index -= 1;
		check( index < handler->array.GetSize() );

		handler->array[ index ]->refs++;

		delete service;
	}
	else
	{
		HTREEITEM		afterItem;
		
		// Insert the new item in sorted order.
		
		afterItem = ( index > 0 ) ? handler->array[ index - 1 ]->item : m_about;
		handler->array.InsertAt( index, service );
		service->item = mTree.InsertItem( service->displayName, 0, 0, NULL, afterItem );
		mTree.SetItemData( service->item, (DWORD_PTR) service );
	}
	return( 0 );
}

//===========================================================================================================================
//	OnServiceRemove
//===========================================================================================================================

LONG	ExplorerBarWindow::OnServiceRemove( ServiceInfo * service )
{
	ServiceHandlerEntry *		handler;
	int							cmp;
	int							index;
	
	
	check( service );
	handler = service->handler; 
	check( handler );
	
	// Search to see if we know about this service instance. If so, remove it from the list.
	
	cmp = FindServiceArrayIndex( handler->array, *service, index );
	check( cmp == 0 );

	if( cmp == 0 )
	{
		// Possibly found a match remove the item. The index
		// is index + 1 so subtract 1.
		index -= 1;
		check( index < handler->array.GetSize() );

		if ( --handler->array[ index ]->refs == 0 )
		{
			mTree.DeleteItem( handler->array[ index ]->item );
			delete handler->array[ index ];
			handler->array.RemoveAt( index );
		}
	}

	delete service;
	return( 0 );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	StartResolve
//===========================================================================================================================

OSStatus	ExplorerBarWindow::StartResolve( ServiceInfo *inService )
{
	OSStatus		err;
	
	check( inService );
	
	// Stop any current resolve that may be in progress.
	
	StopResolve();
	
	// Resolve the service.
	err = DNSServiceResolve( &mResolveServiceRef, 0, 0, 
		inService->name, inService->type, inService->domain, (DNSServiceResolveReply) ResolveCallBack, inService->handler );
	require_noerr( err, exit );

	err = WSAAsyncSelect((SOCKET) DNSServiceRefSockFD(mResolveServiceRef), m_hWnd, WM_PRIVATE_SERVICE_EVENT, FD_READ|FD_CLOSE);
	require_noerr( err, exit );
	
	m_serviceRefs.push_back(mResolveServiceRef);

exit:
	return( err );
}

//===========================================================================================================================
//	StopResolve
//===========================================================================================================================

void	ExplorerBarWindow::StopResolve( void )
{
	if( mResolveServiceRef )
	{
		Stop( mResolveServiceRef );
		mResolveServiceRef = NULL;
	}
}

//===========================================================================================================================
//	ResolveCallBack
//===========================================================================================================================

void DNSSD_API
	ExplorerBarWindow::ResolveCallBack(
		DNSServiceRef			inRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inErrorCode,
		const char *			inFullName,	
		const char *			inHostName, 
		uint16_t 				inPort,
		uint16_t 				inTXTSize,
		const char *			inTXT,
		void *					inContext )
{
	ExplorerBarWindow *			obj;
	ServiceHandlerEntry *		handler;
	OSStatus					err;
	
	DEBUG_UNUSED( inRef );
	DEBUG_UNUSED( inFlags );
	DEBUG_UNUSED( inErrorCode );
	DEBUG_UNUSED( inFullName );
	
	require_noerr( inErrorCode, exit );
	handler = (ServiceHandlerEntry *) inContext;
	check( handler );
	obj = handler->obj;
	check( obj );
	
	try
	{
		ResolveInfo *		resolve;
		int					idx;
		
		dlog( kDebugLevelNotice, "resolved %s on ifi %d to %s\n", inFullName, inInterfaceIndex, inHostName );
		
		// Stop resolving after the first good result.
		
		obj->StopResolve();
		
		// Post a message to the main thread so it can handle it since MFC is not thread safe.
		
		resolve = new ResolveInfo;
		require_action( resolve, exit, err = kNoMemoryErr );
		
		UTF8StringToStringObject( inHostName, resolve->host );

		// rdar://problem/3841564
		// 
		// strip trailing dot from hostname because some flavors of Windows
		// have trouble parsing it.

		idx = resolve->host.ReverseFind('.');

		if ((idx > 1) && ((resolve->host.GetLength() - 1) == idx))
		{
			resolve->host.Delete(idx, 1);
		}

		resolve->port		= ntohs( inPort );
		resolve->ifi		= inInterfaceIndex;
		resolve->handler	= handler;
		
		err = resolve->txt.SetData( inTXT, inTXTSize );
		check_noerr( err );
		
		obj->OnResolve(resolve);
	}
	catch( ... )
	{
		dlog( kDebugLevelError, "ResolveCallBack: exception thrown\n" );
	}

exit:
	return;
}

//===========================================================================================================================
//	OnResolve
//===========================================================================================================================

LONG	ExplorerBarWindow::OnResolve( ResolveInfo * resolve )
{
	CString				url;
	uint8_t *			path;
	uint8_t				pathSize;
	char *				pathPrefix;
	CString				username;
	CString				password;
	
	
	check( resolve );
		
	// Get login info if needed.
	
	if( resolve->handler->needsLogin )
	{
		LoginDialog		dialog;
		
		if( !dialog.GetLogin( username, password ) )
		{
			goto exit;
		}
	}
	
	// If the HTTP TXT record is a "path=" entry, use it as the resource path. Otherwise, use "/".
	
	pathPrefix = "";
	if( strcmp( resolve->handler->type, "_http._tcp" ) == 0 )
	{
		uint8_t	*	txtData;
		uint16_t	txtLen;	

		resolve->txt.GetData( &txtData, &txtLen );

		path	 = (uint8_t*) TXTRecordGetValuePtr(txtLen, txtData, kTXTRecordKeyPath, &pathSize);

		if (path == NULL)
		{
			path = (uint8_t*) "";
			pathSize = 1;
		}
	}
	else
	{
		path		= (uint8_t *) "";
		pathSize	= 1;
	}

	// Build the URL in the following format:
	//
	// <urlScheme>[<username>[:<password>]@]<name/ip>[<path>]

	url.AppendFormat( TEXT( "%S" ), resolve->handler->urlScheme );					// URL Scheme
	if( username.GetLength() > 0 )
	{
		url.AppendFormat( TEXT( "%s" ), username );									// Username
		if( password.GetLength() > 0 )
		{
			url.AppendFormat( TEXT( ":%s" ), password );							// Password
		}
		url.AppendFormat( TEXT( "@" ) );
	}
	
	url += resolve->host;															// Host
	url.AppendFormat( TEXT( ":%d" ), resolve->port );								// :Port
	url.AppendFormat( TEXT( "%S" ), pathPrefix );									// Path Prefix ("/" or empty).
	url.AppendFormat( TEXT( "%.*S" ), (int) pathSize, (char *) path );				// Path (possibly empty).
	
	// Tell Internet Explorer to go to the URL.
	
	check( mOwner );
	mOwner->GoToURL( url );

exit:
	delete resolve;
	return( 0 );
}

//===========================================================================================================================
//	Stop
//===========================================================================================================================
void ExplorerBarWindow::Stop( DNSServiceRef ref )
{
	m_serviceRefs.remove( ref );

	WSAAsyncSelect(DNSServiceRefSockFD( ref ), m_hWnd, WM_PRIVATE_SERVICE_EVENT, 0);

	DNSServiceRefDeallocate( ref );
}


#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	FindServiceArrayIndex
//===========================================================================================================================

DEBUG_LOCAL int	FindServiceArrayIndex( const ServiceInfoArray &inArray, const ServiceInfo &inService, int &outIndex )
{
	int		result;
	int		lo;
	int		hi;
	int		mid;
	
	result 	= -1;
	mid		= 0;
	lo 		= 0;
	hi 		= (int)( inArray.GetSize() - 1 );
	while( lo <= hi )
	{
		mid = ( lo + hi ) / 2;
		result = inService.displayName.CompareNoCase( inArray[ mid ]->displayName );
#if 0
		if( result == 0 )
		{
			result = ( (int) inService.ifi ) - ( (int) inArray[ mid ]->ifi );
		}
#endif
		if( result == 0 )
		{
			break;
		}
		else if( result < 0 )
		{
			hi = mid - 1;
		}
		else
		{
			lo = mid + 1;
		}
	}
	if( result == 0 )
	{
		mid += 1;	// Bump index so new item is inserted after matching item.
	}
	else if( result > 0 )
	{
		mid += 1;
	}
	outIndex = mid;
	return( result );
}
