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

#include	"comutil.h"
#include	"ShObjIdl.h"

#include	"DebugServices.h"

#include	"Resource.h"

#include	"ExplorerBar.h"

#include	"About.h"
// MFC Debugging

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================================================================================
//	Constants
//===========================================================================================================================
#define LENGTHOF(a) (sizeof(a) == sizeof(&*a)) ? 0 : (sizeof(a) / sizeof(*a))
#define MIN_SIZE_X		10
#define MIN_SIZE_Y		10

//===========================================================================================================================
//	ExplorerBar
//===========================================================================================================================

ExplorerBar::ExplorerBar( void )
{
	++gDLLRefCount;
	
	mRefCount		= 1;
	mSite 			= NULL;
	mWebBrowser		= NULL;
	mParentWindow	= NULL;
	mFocus			= FALSE;
	mViewMode		= 0;
	mBandID			= 0;
}

//===========================================================================================================================
//	~ExplorerBar
//===========================================================================================================================

ExplorerBar::~ExplorerBar( void )
{
	if( mWebBrowser )
	{
		mWebBrowser->Release();
		mWebBrowser = NULL;
	}
	if( mSite )
	{
		mSite->Release();
		mSite = NULL;
	}
	
	--gDLLRefCount;
}

#if 0
#pragma mark -
#pragma mark == IUnknown implementation ==
#endif

//===========================================================================================================================
//	QueryInterface
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::QueryInterface( REFIID inID, LPVOID *outResult )
{
	HRESULT		err;
	
	if( IsEqualIID( inID, IID_IUnknown ) )				// IUnknown
	{
		*outResult = this;
	}
	else if( IsEqualIID( inID, IID_IOleWindow ) )		// IOleWindow
	{
		*outResult = (IOleWindow *) this;
	}
	else if( IsEqualIID( inID, IID_IDockingWindow ) )	// IDockingWindow
	{
		*outResult = (IDockingWindow *) this;
	}
	else if( IsEqualIID( inID, IID_IDeskBand ) )		// IDeskBand
	{
		*outResult = (IDeskBand *) this;
	}
	else if( IsEqualIID( inID, IID_IInputObject ) )		// IInputObject
	{
		*outResult = (IInputObject *) this;
	}
	else if( IsEqualIID( inID, IID_IObjectWithSite ) )	// IObjectWithSite
	{
		*outResult = (IObjectWithSite *) this;
	}
	else if( IsEqualIID( inID, IID_IPersistStream ) )	// IPersistStream
	{
		*outResult = (IPersistStream *) this;
	}
	else if( IsEqualIID( inID, IID_IContextMenu ) )     // IContextMenu
	{
		*outResult = (IContextMenu *) this;
	}
	else
	{
		*outResult = NULL;
		err = E_NOINTERFACE;
		goto exit;
	}
	
	( *( (LPUNKNOWN *) outResult ) )->AddRef();
	err = S_OK;

exit:
	return( err );
}

//===========================================================================================================================
//	AddRef
//===========================================================================================================================

STDMETHODIMP_( DWORD )	ExplorerBar::AddRef( void )
{
	return( ++mRefCount );
}

//===========================================================================================================================
//	Release
//===========================================================================================================================

STDMETHODIMP_( DWORD )	ExplorerBar::Release( void )
{
	DWORD		count;

	count = --mRefCount;
	if( count == 0 )
	{
		delete this;
	}
	return( count );
}

#if 0
#pragma mark -
#pragma mark == IOleWindow implementation ==
#endif

//===========================================================================================================================
//	GetWindow
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::GetWindow( HWND *outWindow )
{
	*outWindow = mWindow.GetSafeHwnd();
	return( S_OK );
}

//===========================================================================================================================
//	ContextSensitiveHelp
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::ContextSensitiveHelp( BOOL inEnterMode )
{
	DEBUG_UNUSED( inEnterMode );
	
	return( E_NOTIMPL );
}

#if 0
#pragma mark -
#pragma mark == IDockingWindow implementation ==
#endif

//===========================================================================================================================
//	ShowDW
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::ShowDW( BOOL inShow )
{
	if( mWindow.GetSafeHwnd() )
	{
		mWindow.ShowWindow( inShow ? SW_SHOW : SW_HIDE );
	}
	return( S_OK );
}

//===========================================================================================================================
//	CloseDW
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::CloseDW( DWORD inReserved )
{
	DEBUG_UNUSED( inReserved );

	ShowDW( FALSE );
	if( mWindow.GetSafeHwnd() )
	{
		mWindow.SendMessage( WM_CLOSE );
	}
	return( S_OK );
}

//===========================================================================================================================
//	ResizeBorderDW
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::ResizeBorderDW( LPCRECT inBorder, IUnknown *inPunkSite, BOOL inReserved )
{
	DEBUG_UNUSED( inBorder );
	DEBUG_UNUSED( inPunkSite );
	DEBUG_UNUSED( inReserved );
	
	return( E_NOTIMPL );
}

#if 0
#pragma mark -
#pragma mark == IDeskBand implementation ==
#endif

//===========================================================================================================================
//	GetBandInfo
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::GetBandInfo( DWORD inBandID, DWORD inViewMode, DESKBANDINFO *outInfo )
{
	HRESULT		err;
	
	require_action( outInfo, exit, err = E_INVALIDARG );
	
	mBandID   = inBandID;
	mViewMode = inViewMode;
	
	if( outInfo->dwMask & DBIM_MINSIZE )
	{
		outInfo->ptMinSize.x = 100;
		outInfo->ptMinSize.y = 100;
	}
	if( outInfo->dwMask & DBIM_MAXSIZE )
	{
		// Unlimited max size.
		
		outInfo->ptMaxSize.x = -1;
		outInfo->ptMaxSize.y = -1;
	}
	if( outInfo->dwMask & DBIM_INTEGRAL )
	{
		outInfo->ptIntegral.x = 1;
		outInfo->ptIntegral.y = 1;
	}
	if( outInfo->dwMask & DBIM_ACTUAL )
	{
		outInfo->ptActual.x = 0;
		outInfo->ptActual.y = 0;
	}
	if( outInfo->dwMask & DBIM_TITLE )
	{
		CString		s;
		BOOL		ok;
		
		ok = s.LoadString( IDS_NAME );
		require_action( ok, exit, err = kNoResourcesErr );
		
		#ifdef UNICODE
			lstrcpyn( outInfo->wszTitle, s, sizeof_array( outInfo->wszTitle ) );
		#else
			DWORD		nChars;
			
			nChars = MultiByteToWideChar( CP_ACP, 0, s, -1, outInfo->wszTitle, sizeof_array( outInfo->wszTitle ) );
			err = translate_errno( nChars > 0, (OSStatus) GetLastError(), kUnknownErr );
			require_noerr( err, exit );
		#endif
	}
	if( outInfo->dwMask & DBIM_MODEFLAGS )
	{
		outInfo->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;
	}
	
	// Force the default background color.
	
	outInfo->dwMask &= ~DBIM_BKCOLOR;
	err = S_OK;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == IInputObject implementation ==
#endif

//===========================================================================================================================
//	UIActivateIO
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::UIActivateIO( BOOL inActivate, LPMSG inMsg )
{
	DEBUG_UNUSED( inMsg );
	
	if( inActivate )
	{
		mWindow.SetFocus();
	}
	return( S_OK );
}

//===========================================================================================================================
//	HasFocusIO
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::HasFocusIO( void )
{
	if( mWindow.GetFocus()->GetSafeHwnd() == mWindow.GetSafeHwnd() )
	{
		return( S_OK );
	}
	return( S_FALSE );
}

//===========================================================================================================================
//	TranslateAcceleratorIO
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::TranslateAcceleratorIO( LPMSG inMsg )
{
	DEBUG_UNUSED( inMsg );
		
	return( S_FALSE );
}

#if 0
#pragma mark -
#pragma mark == IObjectWithSite implementation ==
#endif

//===========================================================================================================================
//	SetSite
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::SetSite( IUnknown *inPunkSite )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	HRESULT		err;
	
	// Release the old interfaces.
	
	if( mWebBrowser )
	{
		mWebBrowser->Release();
		mWebBrowser = NULL;
	}
	if( mSite )
	{
		mSite->Release();
		mSite = NULL;
	}
	
	// A non-NULL site means we're setting the site. Otherwise, the site is being released (done above).
	
	if( !inPunkSite )
	{
		err = S_OK;
		goto exit;
	}
	
	// Get the parent window.
	
	IOleWindow *		oleWindow;
		
	mParentWindow = NULL;
	err = inPunkSite->QueryInterface( IID_IOleWindow, (LPVOID *) &oleWindow );
	require( SUCCEEDED( err ), exit );
	
	err = oleWindow->GetWindow( &mParentWindow );
	oleWindow->Release();
	require_noerr( err, exit );
	require_action( mParentWindow, exit, err = E_FAIL );
	
	// Get the IInputObject interface.
	
	err = inPunkSite->QueryInterface( IID_IInputObjectSite, (LPVOID *) &mSite );
	require( SUCCEEDED( err ), exit );
	check( mSite );
	
	// Get the IWebBrowser2 interface.
	
	IOleCommandTarget *		oleCommandTarget;
	
	err = inPunkSite->QueryInterface( IID_IOleCommandTarget, (LPVOID *) &oleCommandTarget );
	require( SUCCEEDED( err ), exit );
	
	IServiceProvider *		serviceProvider;
	
	err = oleCommandTarget->QueryInterface( IID_IServiceProvider, (LPVOID *) &serviceProvider );
	oleCommandTarget->Release();
	require( SUCCEEDED( err ), exit );
	
	err = serviceProvider->QueryService( SID_SWebBrowserApp, IID_IWebBrowser2, (LPVOID *) &mWebBrowser );
	serviceProvider->Release();
	require( SUCCEEDED( err ), exit );
	
	// Create the main window.
	
	err = SetupWindow();
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	GetSite
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::GetSite( REFIID inID, LPVOID *outResult )
{
	HRESULT		err;
	
	*outResult = NULL;
	require_action( mSite, exit, err = E_FAIL );
	
	err = mSite->QueryInterface( inID, outResult );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == IPersistStream implementation ==
#endif

//
// IPersistStream implementation
// 
// This is only supported to allow the desk band to be dropped on the desktop and to prevent multiple instances of 
// the desk band from showing up in the context menu. This desk band doesn't actually persist any data.
//

//===========================================================================================================================
//	GetClassID
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::GetClassID( LPCLSID outClassID )
{
	*outClassID = CLSID_ExplorerBar;
	return( S_OK );
}

//===========================================================================================================================
//	IsDirty
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::IsDirty( void )
{
	return( S_FALSE );
}

//===========================================================================================================================
//	Load
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::Load( LPSTREAM inStream )
{
	DEBUG_UNUSED( inStream );
	
	return( S_OK );
}

//===========================================================================================================================
//	Save
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::Save( LPSTREAM inStream, BOOL inClearDirty )
{
	DEBUG_UNUSED( inStream );
	DEBUG_UNUSED( inClearDirty );
	
	return( S_OK );
}

//===========================================================================================================================
//	GetSizeMax
//===========================================================================================================================

STDMETHODIMP	ExplorerBar::GetSizeMax( ULARGE_INTEGER *outSizeMax )
{
	DEBUG_UNUSED( outSizeMax );
	
	return( E_NOTIMPL );
}


//===========================================================================================================================
//	QueryContextMenu
//===========================================================================================================================

STDMETHODIMP  ExplorerBar::QueryContextMenu(HMENU hShellContextMenu, UINT iContextMenuFirstItem, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	DEBUG_UNUSED( idCmdLast );
	DEBUG_UNUSED( uFlags );

	CMenu menubar;

	menubar.LoadMenu(IDR_CONTEXT_MENU);
	CMenu * menu = menubar.GetSubMenu(0);

	CMenu shellmenu;

	shellmenu.Attach(hShellContextMenu);

	UINT iShellItem = iContextMenuFirstItem;	//! remove plus one
	UINT idShellCmd = idCmdFirst;

	int n = menu->GetMenuItemCount();

	for (int i=0; i<n; ++i)
	{
		MENUITEMINFO	mii;
		TCHAR				sz[128] = {0};
		
		ZeroMemory(&mii, sizeof(mii));
		mii.fMask		= MIIM_TYPE | MIIM_ID;
		mii.fType		= MFT_STRING;
		mii.cbSize		= sizeof(mii);
		mii.cch			= LENGTHOF(sz);
		mii.dwTypeData	= sz;
		
		menu->GetMenuItemInfo(i, &mii, TRUE);

		mii.wID = idShellCmd++;

		shellmenu.InsertMenuItem(iShellItem++, &mii, TRUE);
	}

	shellmenu.Detach();

	return n;
}


//===========================================================================================================================
//	GetCommandString
//===========================================================================================================================

// Not called for explorer bars
STDMETHODIMP ExplorerBar::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	DEBUG_UNUSED( idCmd );
	DEBUG_UNUSED( uType );
	DEBUG_UNUSED( pwReserved );
	DEBUG_UNUSED( pszName );
	DEBUG_UNUSED( cchMax );

	return E_NOTIMPL;
}

//===========================================================================================================================
//	InvokeCommand
//===========================================================================================================================

//	The shell sends either strings or indexes
//	IE never sends strings
//	The indexes are into an array of my commands
//	So the verb for the first command I added to the menu is always 0x0000
//	Here - because I don't have any submenus - 
//		I can treat the 'verb' as an menu item position	
STDMETHODIMP ExplorerBar::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	// IE doesn't send string commands
	if (HIWORD(lpici->lpVerb) != 0) return 0;

	CAbout dlg;

	dlg.DoModal();

	return S_OK;
}

#if 0
#pragma mark -
#pragma mark == Other ==
#endif

//===========================================================================================================================
//	SetupWindow
//===========================================================================================================================

OSStatus	ExplorerBar::SetupWindow( void )
{
	OSStatus		err;
	CWnd *			window;
	CRect			rect;
	CString			s;
	BOOL			ok;
	
	window = CWnd::FromHandle( mParentWindow );
	check( window );
	window->GetClientRect( rect );
	
	ok = s.LoadString( IDS_NAME );
	require_action( ok, exit, err = kNoResourcesErr );
	
	ok = mWindow.Create( NULL, s, WS_CHILD | WS_VISIBLE, rect, window, 100 ) != 0;
	require_action( ok, exit, err = kNoResourcesErr );
	
	mWindow.SetOwner( this );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	GoToURL
//===========================================================================================================================

OSStatus	ExplorerBar::GoToURL( const CString &inURL )
{
	OSStatus		err;
	BSTR			s;
	VARIANT			empty;
	
	s = inURL.AllocSysString();
	require_action( s, exit, err = kNoMemoryErr );
	
	VariantInit( &empty );
	err = mWebBrowser->Navigate( s, &empty, &empty, &empty, &empty );
	SysFreeString( s );
	require_noerr( err, exit );

exit:
	return( err );
}
