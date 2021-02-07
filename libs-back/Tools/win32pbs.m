/*
   win32pbs.m

   GNUstep pasteboard server - Win32 extension

   Copyright (C) 2003 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: December 2003

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/* Access Windows 2000 (and later) API.  Required for HWND_MESSAGE.  */
#define WINVER 0x500

#include <Foundation/Foundation.h>
#include <Foundation/NSUserDefaults.h>
#include <AppKit/NSPasteboard.h>

#include <windows.h>

#ifdef __CYGWIN__
#include <sys/file.h>
#endif

@interface Win32PbOwner : NSObject
{
  NSPasteboard	*_pb;
  HINSTANCE _hinstance;
  HWND _hwnd;
  BOOL _ignore;
}

- (id) initWithOSPb: (NSPasteboard*) ospb;
- (void) clipboardHasData;
- (void) setClipboardData;
- (void) grapClipboard;
- (void) setupRunLoopInputSourcesForMode: (NSString*)mode;

@end

static Win32PbOwner *wpb = nil;
static HWND hwndNextViewer = NULL;
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg,
                             WPARAM wParam, LPARAM lParam);


@implementation Win32PbOwner

+ (BOOL) initializePasteboard
{
  if (self == [Win32PbOwner class])
    {
      wpb = [[Win32PbOwner alloc] initWithOSPb: 
                                    [NSPasteboard generalPasteboard]];
      [wpb clipboardHasData];
    }
  return YES;
}

+ (id) ownerByOsPb: (NSString*)p
{
  if ([p isEqual: [[NSPasteboard generalPasteboard] name]])
    {
      return wpb;
    }
  else
    {
      return nil;
    }
}

- (id) initWithOSPb: (NSPasteboard*) ospb
{
  WNDCLASSEX wc; 

  _ignore = NO;  
  _hinstance = (HINSTANCE)GetModuleHandle(NULL);

  // Register the main window class. 
  wc.cbSize = sizeof(wc);          
  //wc.style = CS_OWNDC;
  wc.style = CS_HREDRAW | CS_VREDRAW; 
  wc.lpfnWndProc = (WNDPROC) MainWndProc; 
  wc.cbClsExtra = 0; 
  wc.cbWndExtra = 0; 
  wc.hInstance = _hinstance; 
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = NULL; 
  wc.lpszMenuName =  NULL; 
  wc.lpszClassName = "GNUstepClipboardClass"; 
  wc.hIconSm = NULL;

  if (RegisterClassEx(&wc)) 
    {
      _hwnd = CreateWindowEx(0, "GNUstepClipboardClass", "GNUstepClipboard", 
                             0, 0, 0, 10, 10, 
                             HWND_MESSAGE, (HMENU)NULL, _hinstance, NULL); 
    }

  ASSIGN(_pb, ospb);
  [self setupRunLoopInputSourcesForMode: NSDefaultRunLoopMode]; 

  return self;  
}

- (void) dealloc
{
  RELEASE(_pb);
  DestroyWindow(_hwnd);
  UnregisterClass("GNUstepClipboardClass", _hinstance);
  [super dealloc];
}

- (void) clipboardHasString
{
  if ((_hwnd != GetClipboardOwner()) &&
      IsClipboardFormatAvailable(CF_UNICODETEXT)) 
    {
      [_pb declareTypes: [NSArray arrayWithObject: NSStringPboardType]
           owner: self];
    }
}

/* 
   The owner of the Windows clipboard did change. Check if this 
   results in some action for us.
 */
- (void) clipboardHasData
{
  if (!_ignore)
    {
      _ignore = YES;
      [self clipboardHasString];
      _ignore = NO;
    }
}

- (void) setClipboardString
{
  HGLOBAL hglb; 
  LPWSTR lpwstr; 
  NSString *s;
  unsigned int len;

  s = [_pb stringForType: NSStringPboardType];
  if (s == nil)
    {
      return;
    }

  len = [s length];
  hglb = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(WCHAR)); 
  if (hglb == NULL) 
    { 
      return; 
    } 
  
  // Lock the handle and copy the text to the buffer. 
  lpwstr = GlobalLock(hglb); 
  [s getCharacters: lpwstr];
  lpwstr[len] = (WCHAR)0;
  GlobalUnlock(hglb); 
  SetClipboardData(CF_UNICODETEXT, hglb);
}

/* 
   Data is requested from the Windows clipboard. We are already the 
   owner of the clipboard.
 */
- (void) setClipboardData
{
  if (!_ignore)
    {
      _ignore = YES;  
      [self setClipboardString];
      _ignore = NO;;  
    }
}

/* 
   Take over the ownership of the Windows clipboard, but don't provide data 
 */
- (void) grapClipboard
{
  if (!OpenClipboard(_hwnd)) 
     {
       NSLog(@"Failed to get the Win32 clipboard. %d", GetLastError());
       return; 
     }
  if (!EmptyClipboard())
    {
      NSLog(@"Failed to get the Win32 clipboard. %d", GetLastError());
      CloseClipboard();
      return;
    }
  SetClipboardData(CF_UNICODETEXT, NULL);

  CloseClipboard();
}

/*
 * If this gets called, a GNUstep object has grabbed the pasteboard
 * or has changed the types of data available from the pasteboard
 * so we must tell the Windows system, that we have the current selection.
 */
- (void) pasteboardChangedOwner: (NSPasteboard*)sender
{
  if (!_ignore)
    {
      _ignore = YES;  
      [self grapClipboard];
      _ignore = NO;;  
    }
}

- (void) provideStringTo: (NSPasteboard*)pb
{
  HGLOBAL hglb; 
 
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || 
      !OpenClipboard(_hwnd)) 
    {
      return; 
    }
  
  hglb = GetClipboardData(CF_UNICODETEXT); 
  if (hglb != NULL) 
    { 
      LPWSTR lpwstr; 
      
      lpwstr = GlobalLock(hglb); 
      if (lpwstr != NULL) 
        {
          unsigned int len;
          NSString *s;
          
          len = lstrlenW(lpwstr);
          s = [NSString stringWithCharacters: lpwstr 
                        length: len]; 
          [pb setString: s forType: NSStringPboardType];
          GlobalUnlock(hglb); 
        } 
    } 
  CloseClipboard(); 
}

- (void) pasteboard: (NSPasteboard*)pb provideDataForType: (NSString*)type
{
  if (!_ignore)
    {
      if ([type isEqual: NSStringPboardType])
        {
          _ignore = YES;  
          [self provideStringTo: pb];
          _ignore = NO;;  
        }
    }
}

- (void) callback: (id) sender
{
  MSG msg;
  WINBOOL bRet; 

  while ((bRet = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) != 0)
    { 

      if (msg.message == WM_QUIT)
        {
          // Exit the program
          return;
        }
      if (bRet == -1)
        {
          // handle the error and possibly exit
        }
      else
        {
          // Don't translate messages, as this would give extra character messages.
          DispatchMessage(&msg); 
        } 
    } 
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode
{
#ifdef    __CYGWIN__
  if (type == ET_RDESC)
#else 
  if (type == ET_WINMSG)
#endif
    {
      MSG *m = (MSG*)extra;

      if (m->message == WM_QUIT)
        {
          //[NSApp terminate: nil];
          // Exit the program
          return;
        }
      else
        {
          DispatchMessage(m); 
        } 
    } 
  if (mode != nil)
    [self callback: mode];
}

- (void) setupRunLoopInputSourcesForMode: (NSString*)mode
{
  NSRunLoop *currentRunLoop = [NSRunLoop currentRunLoop];

#ifdef    __CYGWIN__
  int fdMessageQueue;
#define WIN_MSG_QUEUE_FNAME    "/dev/windows"

  // Open a file descriptor for the windows message queue
  fdMessageQueue = open (WIN_MSG_QUEUE_FNAME, O_RDONLY);
  if (fdMessageQueue == -1)
    {
      NSLog(@"Failed opening %s\n", WIN_MSG_QUEUE_FNAME);
      exit(1);
    }
  [currentRunLoop addEvent: (void*)fdMessageQueue
                  type: ET_RDESC
                  watcher: (id<RunLoopEvents>)self
                  forMode: mode];
#else 
  [currentRunLoop addEvent: (void*)0
                  type: ET_WINMSG
                  watcher: (id<RunLoopEvents>)self
                  forMode: mode];
#endif
}

@end

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam) 
{ 
  switch (uMsg) 
    { 
    case WM_CREATE: 
      // Add the window to the clipboard viewer chain. 
      hwndNextViewer = SetClipboardViewer(hwnd); 
      break;
      
    case WM_CHANGECBCHAIN: 
      // If the next window is closing, repair the chain. 
      if ((HWND) wParam == hwndNextViewer) 
        hwndNextViewer = (HWND) lParam; 
      // Otherwise, pass the message to the next link. 
      else if (hwndNextViewer != NULL) 
        SendMessage(hwndNextViewer, uMsg, wParam, lParam); 
      break;

    case WM_DESTROY: 
      ChangeClipboardChain(hwnd, hwndNextViewer); 
      PostQuitMessage(0); 
      break;

    case WM_DRAWCLIPBOARD:
      // clipboard contents changed. 
      if (wpb != nil)
        [wpb clipboardHasData];

      // Pass the message to the next window in clipboard 
      // viewer chain. 
      if (hwndNextViewer != NULL) 
        SendMessage(hwndNextViewer, uMsg, wParam, lParam); 
      break; 

    case WM_RENDERFORMAT: 
      [wpb setClipboardData];
      break; 
 
    case WM_RENDERALLFORMATS:
      if (!OpenClipboard(hwnd))
	{
	  NSWarnMLog(@"Failed to get the Win32 clipboard. %d", GetLastError());
	}
      else if (GetClipboardOwner() == hwnd)
	{
	  if (!EmptyClipboard())
	    {
	      NSWarnMLog(@"Failed to get the Win32 clipboard. %d", GetLastError());
	    }
	  else
	    {
	      SendMessage(hwnd, WM_RENDERFORMAT, CF_UNICODETEXT, 0);
	      CloseClipboard();
	    }
	}
      break;
      
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    } 

  return (LRESULT) NULL; 
}

