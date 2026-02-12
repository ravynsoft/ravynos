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

#ifndef __EXPLORER_BAR__
#define __EXPLORER_BAR__

#include    "StdAfx.h"

#include    "ExplorerBarWindow.h"
#include    "ExplorerPlugin.h"

//===========================================================================================================================
//	ExplorerBar
//===========================================================================================================================

class ExplorerBar : public IDeskBand,
    public IInputObject,
    public IObjectWithSite,
    public IPersistStream,
    public IContextMenu
{
protected:

DWORD mRefCount;
IInputObjectSite *      mSite;
IWebBrowser2 *          mWebBrowser;
HWND mParentWindow;
BOOL mFocus;
DWORD mViewMode;
DWORD mBandID;
ExplorerBarWindow mWindow;

public:

ExplorerBar( void );
~ExplorerBar( void );

// IUnknown methods

STDMETHODIMP            QueryInterface( REFIID inID, LPVOID *outResult );
STDMETHODIMP_( DWORD )  AddRef( void );
STDMETHODIMP_( DWORD )  Release( void );

// IOleWindow methods

STDMETHOD( GetWindow ) ( HWND *outWindow );
STDMETHOD( ContextSensitiveHelp ) ( BOOL inEnterMode );

// IDockingWindow methods

STDMETHOD( ShowDW ) ( BOOL inShow );
STDMETHOD( CloseDW ) ( DWORD inReserved );
STDMETHOD( ResizeBorderDW ) ( LPCRECT inBorder, IUnknown *inPunkSite, BOOL inReserved );

// IDeskBand methods

STDMETHOD( GetBandInfo ) ( DWORD inBandID, DWORD inViewMode, DESKBANDINFO *outInfo );

// IInputObject methods

STDMETHOD( UIActivateIO ) ( BOOL inActivate, LPMSG inMsg );
STDMETHOD( HasFocusIO ) ( void );
STDMETHOD( TranslateAcceleratorIO ) ( LPMSG inMsg );

// IObjectWithSite methods

STDMETHOD( SetSite ) ( IUnknown *inPunkSite );
STDMETHOD( GetSite ) ( REFIID inID, LPVOID *outResult );

// IPersistStream methods

STDMETHOD( GetClassID ) ( LPCLSID outClassID );
STDMETHOD( IsDirty ) ( void );
STDMETHOD( Load ) ( LPSTREAM inStream );
STDMETHOD( Save ) ( LPSTREAM inStream, BOOL inClearDirty );
STDMETHOD( GetSizeMax ) ( ULARGE_INTEGER *outSizeMax );

// IContextMenu methods

STDMETHOD( QueryContextMenu ) ( HMENU hContextMenu, UINT iContextMenuFirstItem, UINT idCmdFirst, UINT idCmdLast, UINT uFlags );
STDMETHOD( GetCommandString ) ( UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax );
STDMETHOD( InvokeCommand ) ( LPCMINVOKECOMMANDINFO lpici );

// Other

OSStatus    SetupWindow( void );
OSStatus    GoToURL( const CString &inURL );
};

#endif  // __EXPLORER_BAR__
