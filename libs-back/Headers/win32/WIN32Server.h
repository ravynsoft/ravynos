/* WIN32Server - Implements window handling for MSWindows

   Copyright (C) 2002,2005 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   Part of this code have been re-written by:
   Tom MacSween <macsweent@sympatico.ca>
   Date August 2005

   This file is part of the GNU Objective C User Interface Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _WIN32Server_h_INCLUDE
#define _WIN32Server_h_INCLUDE

#include <Foundation/NSNotification.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSString.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSConnection.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSTimer.h>
#include <AppKit/AppKitExceptions.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSGraphics.h>
#include <AppKit/NSMenu.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSView.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSCursor.h>
#include <AppKit/NSText.h>
#include <AppKit/NSPopUpButton.h>
#include <AppKit/NSPanel.h>
#include <AppKit/DPSOperators.h>
#include <AppKit/NSImage.h>

#include <GNUstepGUI/GSDisplayServer.h>
#include <config.h>
#include <windows.h>

/* MINGW64 hack to build */
#ifdef __MINGW64__
#undef GWL_USERDATA
#define GWL_USERDATA GWLP_USERDATA 
#endif

/*
 This standard windows macros are missing in MinGW.  The definition
 here is almost correct, but will fail for multi monitor systems
*/
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(p) ((int)(short)LOWORD(p))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(p) ((int)(short)HIWORD(p))
#endif

// The horizontal mousehweel message is missing in MinGW 
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

#define EVENT_WINDOW(lp) (GSWindowWithNumber((int)lp)) 

DWORD windowStyleForGSStyle(unsigned int style);
   
typedef struct w32serverFlags 
{
  BOOL HOLD_MINI_FOR_SIZE;        // override GS size event on miniturize
  BOOL _eventHandled;             // did we handle the event?
} serverFlags;

@interface WIN32Server : GSDisplayServer
{
  BOOL handlesWindowDecorations;
  BOOL usesNativeTaskbar;

  serverFlags flags;
  HINSTANCE hinstance;

  HWND currentFocus;
  HWND desiredFocus;
  HWND currentActive;
  HICON  currentAppIcon;
  NSMutableArray *monitorInfo;
}

- (BOOL) handlesWindowDecorations;
- (void) setHandlesWindowDecorations: (BOOL) b;

- (BOOL) usesNativeTaskbar;
- (void) setUsesNativeTaskbar: (BOOL) b;

+ (void) initializeBackend;

- (LRESULT) windowEventProc: (HWND)hwnd : (UINT)uMsg 
		       : (WPARAM)wParam : (LPARAM)lParam;
			
- (void) setFlagsforEventLoop: (HWND)hwnd;

- (DWORD) windowStyleForGSStyle: (unsigned int) style;

- (void) resizeBackingStoreFor: (HWND)hwnd;

- (HDC) createHdcForScreen: (int)screen;
- (void) deleteScreenHdc: (HDC)hdc;

@end

@interface WIN32Server (w32_activate)

- (LRESULT) decodeWM_MOUSEACTIVATEParams: (WPARAM)wParam : (LPARAM)lParam 
						: (HWND)hwnd;
- (LRESULT) decodeWM_ACTIVEParams: (WPARAM)wParam : (LPARAM)lParam 
				 : (HWND)hwnd;
- (LRESULT) decodeWM_ACTIVEAPPParams: (HWND)hwnd : (WPARAM)wParam 
				    : (LPARAM)lParam;
- (void) decodeWM_NCACTIVATEParams: (WPARAM)wParam : (LPARAM)lParam 
				  : (HWND)hwnd;

@end

@interface WIN32Server (w32_movesize)

- (LRESULT) decodeWM_SIZEParams: (HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam;
- (LRESULT) decodeWM_MOVEParams: (HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam;
- (void) decodeWM_NCCALCSIZEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_WINDOWPOSCHANGINGParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_WINDOWPOSCHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_GETMINMAXINFOParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_ENTERSIZEMOVEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_EXITSIZEMOVEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_MOVINGParams: (HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam;
- (LRESULT) decodeWM_SIZINGParams: (HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam;

@end

@interface WIN32Server (w32_create)

- (LRESULT) decodeWM_NCCREATEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_CREATEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;

@end

@interface WIN32Server (w32_windowdisplay)

- (void) decodeWM_SHOWWINDOWParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_NCPAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_ERASEBKGNDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_PAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_SYNCPAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_CAPTURECHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (HICON) decodeWM_GETICONParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (HICON) decodeWM_SETICONParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;

@end

@interface WIN32Server (w32_text_focus)

//- (LRESULT) decodeWM_SETTEXTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (LRESULT) decodeWM_SETFOCUSParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_KILLFOCUSParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_GETTEXTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;

@end

@interface WIN32Server (w32_General)

- (void) decodeWM_CLOSEParams: (WPARAM)wParam :(LPARAM)lParam :(HWND)hwnd;
- (void) decodeWM_DESTROYParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_NCDESTROYParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_QUERYOPENParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_SYSCOMMANDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_COMMANDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;
- (void) decodeWM_THEMECHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd;

@end

// FIXME: Keep all of the extra window data in one place

// Extra window data accessed via GetWindowLong

enum _WIN_EXTRA_BYTES
{
  OFF_LEVEL       = 0,
  OFF_ORDERED     = OFF_LEVEL + sizeof(DWORD),
  IME_INFO        = OFF_ORDERED + sizeof(DWORD),
  WIN_EXTRABYTES  = IME_INFO + sizeof(DWORD)
};


// Pointer to this struct set in IME_INFO extra bytes space for
// handling IME composition processing between various windows...
typedef struct IME_INFO_S
{
  DWORD   isOpened;
  BOOL    isComposing;
  
  LPVOID  readString;
  DWORD   readStringLength;
  LPVOID  compString;
  DWORD   compStringLength;
  
  DWORD   compositionMode;
  DWORD   sentenceMode;
} IME_INFO_T;


// Extra window data allocated using objc_malloc in WM_CREATE and accessed via
// the GWL_USERDATA pointer

typedef struct _win_intern {
  BOOL useHDC;
  BOOL backingStoreEmpty;
  HDC hdc; 
  HGDIOBJ old;
  MINMAXINFO minmax;
  NSBackingStoreType type;
#if (BUILD_GRAPHICS==GRAPHICS_cairo)
  void *surface;
#endif
} WIN_INTERN;

#endif /* _WIN32Server_h_INCLUDE */
