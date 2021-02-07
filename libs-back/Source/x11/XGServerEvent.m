/*
   XGServerEvent - Window/Event code for X11 backends.

   Copyright (C) 1998-2015 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: Nov 1998
   
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
*/

#include "config.h"

#include <AppKit/AppKitExceptions.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSGraphics.h>
#include <AppKit/NSMenu.h>
#include <AppKit/NSPasteboard.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSScreen.h>
#include <Foundation/NSException.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSData.h>
#include <Foundation/NSNotification.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSString.h>
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDistributedNotificationCenter.h>

#include "x11/XGServerWindow.h"
#include "x11/XGInputServer.h"
#include "x11/XGDragView.h"
#include "x11/XGGeneric.h"
#include "x11/xdnd.h"

#ifdef HAVE_WRASTER_H
#include "wraster.h"
#else
#include "x11/wraster.h"
#endif
#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

#include "math.h"
#include <X11/keysym.h>
#include <X11/Xproto.h>

#if LIB_FOUNDATION_LIBRARY
# include <Foundation/NSPosixFileDescriptor.h>
#elif defined(NeXT_PDO)
# include <Foundation/NSFileHandle.h>
# include <Foundation/NSNotification.h>
#endif

#define cWin ((gswindow_device_t*)generic.cachedWindow)

// NumLock's mask (it depends on the keyboard mapping)
static unsigned int _num_lock_mask;
// Modifier state
static char _shift_pressed = 0;
static char _control_pressed = 0;
static char _command_pressed = 0;
static char _alt_pressed = 0;
static char _help_pressed = 0;
/*
Keys used for the modifiers (you may set them with user preferences).
Note that the first and second key sym for a modifier must be different.
Otherwise, the _*_pressed tracking will be confused.
*/
static KeySym _control_keysyms[2];
static KeySym _command_keysyms[2];
static KeySym _alt_keysyms[2];
static KeySym _help_keysyms[2];

static BOOL _is_keyboard_initialized = NO;
static BOOL _mod_ignore_shift = NO;

/*
  Mouse properties. In comments below specified defaults key and default value.
*/
static NSInteger   clickTime;             // "GSDoubleClickTime" - milisecond (250)
static NSInteger   clickMove;             // "GSMouseMoveThreshold" - in pixels (3)
static NSInteger   mouseScrollMultiplier; // "GSMouseScrollMultiplier" - times (1)
static NSEventType menuMouseButton;       // "GSMenuButtonEvent" - (NSRightMouseButon)
static BOOL        menuButtonEnabled;     // "GSMenuButtonEnabled" - BOOL
static BOOL        swapMouseButtons;      // YES if "GSMenuButtonEvent" == NSLeftMouseButton

void __objc_xgcontextevent_linking (void)
{
}

static SEL procSel = 0;
static void (*procEvent)(id, SEL, XEvent*) = 0;

#ifdef XSHM
@interface NSGraphicsContext (SharedMemory)
-(void) gotShmCompletion: (Drawable)d;
@end
#endif

@interface XGServer (Private)
- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode;
- (void) setupRunLoopInputSourcesForMode: (NSString*)mode; 
- (NSDate*) timedOutEvent: (void*)data
                     type: (RunLoopEventType)type
                  forMode: (NSString*)mode;
- (int) XGErrorHandler: (Display*)display : (XErrorEvent*)err;
- (void) processEvent: (XEvent *) event;
- (NSEvent *)_handleTakeFocusAtom: (XEvent)xEvent 
        	       forContext: (NSGraphicsContext *)gcontext;
@end


int
XGErrorHandler(Display *display, XErrorEvent *err)
{
  XGServer *ctxt = (XGServer*)GSCurrentServer();

  return [ctxt XGErrorHandler: display : err];
}

static NSEvent*process_key_event (XEvent* xEvent, XGServer* ctxt, 
                                  NSEventType eventType, 
                                  NSMutableArray *event_queue, BOOL keyRepeat);

static unichar process_char (KeySym keysym, unsigned *eventModifierFlags);

static unsigned process_modifier_flags(unsigned int state);

static void initialize_keyboard (void);

static void set_up_num_lock (void);

// checks whether a GNUstep modifier (key_sym) is pressed when we're only able
// to check whether X keycodes are pressed in xEvent->xkeymap;
static int check_modifier (XEvent *xEvent, KeySym key_sym)
{
  char *key_vector;
  int by,bi;
  int key_code = XKeysymToKeycode(xEvent->xkeymap.display, key_sym);
  
  if (key_code != NoSymbol)
    {
      by = key_code / 8;
      bi = key_code % 8;
      key_vector = xEvent->xkeymap.key_vector;
      return (key_vector[by] & (1 << bi));
    }
  return 0;
}

@interface XGServer (WindowOps)
- (void) styleoffsets: (float *) l : (float *) r : (float *) t : (float *) b
                     : (unsigned int) style : (Window) win;
- (NSRect) _XWinRectToOSWinRect: (NSRect)r for: (void*)windowNumber;
@end

@implementation XGServer (EventOps)

- (int) XGErrorHandler: (Display*)display : (XErrorEvent*)err
{
  int length = 1024;
  char buffer[length+1];

  /*
   * Ignore attempts to set input focus to unmapped window, except for noting
   * if the most recent request failed (mark the request serial number to 0)
   * in which case we should repeat the request when the window becomes
   * mapped again.
   */
  if (err->error_code == BadMatch && err->request_code == X_SetInputFocus)
    {
      if (err->serial == generic.focusRequestNumber)
        {
          generic.focusRequestNumber = 0;
        }
      return 0;
    }

  XGetErrorText(display, err->error_code, buffer, length);
  if (err->type == 0
      && GSDebugSet(@"XSynchronize") == NO)
    {
      NSLog(@"X-Windows error - %s\n\
          on display: %s\n\
                type: %d\n\
       serial number: %lu\n\
        request code: %d\n",
        buffer,
        XDisplayName(DisplayString(display)),
        err->type, err->serial, err->request_code);
      return 0;
    }
  [NSException raise: NSWindowServerCommunicationException
    format: @"X-Windows error - %s\n\
          on display: %s\n\
                type: %d\n\
       serial number: %lu\n\
        request code: %d\n",
        buffer,
        XDisplayName(DisplayString(display)),
        err->type, err->serial, err->request_code];
  return 0;
}

- (void) setupRunLoopInputSourcesForMode: (NSString*)mode
{
  int xEventQueueFd = XConnectionNumber(dpy);
  NSRunLoop *currentRunLoop = [NSRunLoop currentRunLoop];

#if defined(LIB_FOUNDATION_LIBRARY)
  {
    id fileDescriptor = [[[NSPosixFileDescriptor alloc]
        initWithFileDescriptor: xEventQueueFd]
        autorelease];

    // Invoke limitDateForMode: to setup the current
    // mode of the run loop (the doc says that this
    // method and acceptInputForMode: beforeDate: are
    // the only ones that setup the current mode).

    [currentRunLoop limitDateForMode: mode];

    [fileDescriptor setDelegate: self];
    [fileDescriptor monitorFileActivity: NSPosixReadableActivity];
  }
#elif defined(NeXT_PDO)
  {
    id fileDescriptor = [[[NSFileHandle alloc]
        initWithFileDescriptor: xEventQueueFd]
        autorelease];

    [[NSNotificationCenter defaultCenter] addObserver: self
        selector: @selector(activityOnFileHandle:)
        name: NSFileHandleDataAvailableNotification
        object: fileDescriptor];
    [fileDescriptor waitForDataInBackgroundAndNotifyForModes:
        [NSArray arrayWithObject: mode]];
  }
#else
  [currentRunLoop addEvent: (void*)(gsaddr)xEventQueueFd
                      type: ET_RDESC
                   watcher: (id<RunLoopEvents>)self
                   forMode: mode];
#endif
  if (procSel == 0)
    {
      procSel = @selector(processEvent:);
      procEvent = (void (*)(id, SEL, XEvent*))
        [self methodForSelector: procSel];
    }
}

#if LIB_FOUNDATION_LIBRARY
- (void) activity: (NSPosixFileActivities)activity
posixFileDescriptor: (NSPosixFileDescriptor*)fileDescriptor
{
  [self receivedEvent: 0 type: 0 extra: 0 forMode: nil];
}
#elif defined(NeXT_PDO)
- (void) activityOnFileHandle: (NSNotification*)notification
{
  id fileDescriptor = [notification object];
  id runLoopMode = [[NSRunLoop currentRunLoop] currentMode];

  [fileDescriptor waitForDataInBackgroundAndNotifyForModes:
        [NSArray arrayWithObject: runLoopMode]];
  [self receivedEvent: 0 type: 0 extra: 0 forMode: nil];
}
#endif

- (BOOL) runLoopShouldBlock: (BOOL*)trigger
{
  *trigger = YES;        //  Should trigger this event
  if (XPending(dpy) > 0)
    {
      return NO;        // Don't block
    }
  return YES;
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode
{
  XEvent xEvent;

  // loop and grab all of the events from the X queue
  while (XPending(dpy) > 0)
    {
      XNextEvent(dpy, &xEvent);

#ifdef USE_XIM
      if (XFilterEvent(&xEvent, None)) 
        {
          NSDebugLLog(@"NSKeyEvent", @"Event filtered (by XIM?)\n");
          continue;
        }
#endif

      (*procEvent)(self, procSel, &xEvent);
    }
}

/*
 */
- (NSPoint) _XPointToOSPoint: (NSPoint)x for: (void*)window
{
  gswindow_device_t *win = (gswindow_device_t*)window;
  unsigned int style = win->win_attrs.window_style;
  NSPoint o;
  float t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];
  o.x = x.x + l;
  o.y = NSHeight(win->xframe) - x.y + b;

  NSDebugLLog(@"Frame", @"X2OP %lu, %x, %@, %@", win->number, style,
    NSStringFromPoint(x), NSStringFromPoint(o));
  return o;
}

- (void) mouseOptionsChanged: (NSNotification *)aNotif
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];

  clickTime = [defs integerForKey:@"GSDoubleClickTime"];
  if (clickTime < 200)
    clickTime = 300;
  
  clickMove = [defs integerForKey:@"GSMouseMoveThreshold"];
  if (clickMove < 3)
    clickMove = 3;

  mouseScrollMultiplier = [defs integerForKey:@"GSMouseScrollMultiplier"];
  if (mouseScrollMultiplier == 0)
    mouseScrollMultiplier = 1;

  if ([defs objectForKey:@"GSMenuButtonEnabled"])
    menuButtonEnabled = [defs boolForKey:@"GSMenuButtonEnabled"];
  else
    menuButtonEnabled = YES;

  if ([defs objectForKey:@"GSMenuButtonEvent"])
    menuMouseButton = [defs integerForKey:@"GSMenuButtonEvent"];
  else
    menuMouseButton = NSRightMouseDown;
  
  switch (menuMouseButton)
    {
    case NSLeftMouseDown:
      swapMouseButtons = YES;
      break;
    default:
      swapMouseButtons = NO;
      break;
    }
}

- (void) initializeMouse
{
  [self mouseOptionsChanged: nil];
  [[NSDistributedNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(mouseOptionsChanged:)
           name: NSUserDefaultsDidChangeNotification
         object: nil];
}

- (void) processEvent: (XEvent *) event
{
  static int clickCount = 1;
  static unsigned int eventFlags;
  static NSPoint eventLocation;
  NSEvent *e = nil;
  XEvent xEvent;
  NSWindow *nswin;
  Window xWin;
  NSEventType eventType;
  NSGraphicsContext *gcontext;
  float deltaX;
  float deltaY;
  int buttonNumber;

  gcontext = GSCurrentContext();
  xEvent = *event;

  switch (xEvent.type)
    {
        // mouse button events
      case ButtonPress:
        NSDebugLLog(@"NSEvent", @"%lu ButtonPress: \
                xEvent.xbutton.time %lu timeOfLastClick %lu \n",
                    xEvent.xbutton.window, xEvent.xbutton.time,
                    generic.lastClick);
        /*
         * hardwired test for a double click
         *
         * For multiple clicks, the clicks must remain in the same
         * region of the same window and must occur in a limited time.
         *
         * default time of 300 should be user set;
         * perhaps the movement of 3 should also be a preference?
         */
        {
          BOOL incrementCount = YES;
          
          if (clickTime == 0) [self initializeMouse];
         
          if (xEvent.xbutton.time
            >= (unsigned long)(generic.lastClick + clickTime))
            incrementCount = NO;
          else if (generic.lastClickWindow != xEvent.xbutton.window)
            incrementCount = NO;
          else if ((generic.lastClickX - xEvent.xbutton.x) > clickMove)
            incrementCount = NO;
          else if ((generic.lastClickX - xEvent.xbutton.x) < -clickMove)
            incrementCount = NO;
          else if ((generic.lastClickY - xEvent.xbutton.y) > clickMove)
            incrementCount = NO;
          else if ((generic.lastClickY - xEvent.xbutton.y) < -clickMove)
            incrementCount = NO;

          if (incrementCount == YES)
            {
              clickCount++;
            }
          else
            {
              /*
               * Not a multiple-click, so we must set the stored
               * location of the click to the new values and
               * reset the counter.
               */
              clickCount = 1;
              generic.lastClickWindow = xEvent.xbutton.window;
              generic.lastClickX = xEvent.xbutton.x;
              generic.lastClickY = xEvent.xbutton.y;
            }
        }
        generic.lastClick = xEvent.xbutton.time;
        [self setLastTime: generic.lastClick];
	deltaX = 0.0;
        deltaY = 0.0;

        if (xEvent.xbutton.button == generic.lMouse)
          {
            if (swapMouseButtons)
              {
                eventType = NSRightMouseDown;
                buttonNumber = generic.rMouse;
              }
            else
              {
                eventType = NSLeftMouseDown;
                buttonNumber = generic.lMouse;
              }
          }
        else if (xEvent.xbutton.button == generic.rMouse
                 && generic.rMouse != 0)
          {
            if (swapMouseButtons)
              {
                eventType = NSLeftMouseDown;
                buttonNumber = generic.lMouse;
              }
            else
              {
                eventType = NSRightMouseDown;
                buttonNumber = generic.rMouse;
              }
          }
        else if (xEvent.xbutton.button == generic.mMouse
                 && generic.mMouse != 0)
          {
            eventType = NSOtherMouseDown;
            buttonNumber = generic.mMouse;
          }
        else if (xEvent.xbutton.button == generic.upMouse
                 && generic.upMouse != 0)
          {
            deltaY = 1. * mouseScrollMultiplier;
            eventType = NSScrollWheel;
            buttonNumber = generic.upMouse;
          }
        else if (xEvent.xbutton.button == generic.downMouse
                 && generic.downMouse != 0)
          {
            deltaY = -1. * mouseScrollMultiplier;
            eventType = NSScrollWheel;
            buttonNumber = generic.downMouse;
          }
        else if (xEvent.xbutton.button == generic.scrollLeftMouse
                 && generic.scrollLeftMouse != 0)
          {
            deltaX = -1. * mouseScrollMultiplier;
            eventType = NSScrollWheel;
            buttonNumber = generic.scrollLeftMouse;
          }
        else if (xEvent.xbutton.button == generic.scrollRightMouse
                 && generic.scrollRightMouse != 0)
          {
            deltaX = 1. * mouseScrollMultiplier;
            eventType = NSScrollWheel;
            buttonNumber = generic.scrollRightMouse;
          }
        else
          {
            break;                /* Unknown button */
          }

        if (menuButtonEnabled == NO && eventType == menuMouseButton)
          break; // disabled menu button was pressed

        eventFlags = process_modifier_flags(xEvent.xbutton.state);
        // if pointer is grabbed use grab window
        xWin = (grabWindow == 0) ? xEvent.xbutton.window : grabWindow;
        if (cWin == 0 || xWin != cWin->ident)
          generic.cachedWindow = [XGServer _windowForXWindow: xWin];
        if (cWin == 0)
          break;
        eventLocation.x = xEvent.xbutton.x;
        eventLocation.y = xEvent.xbutton.y;
        eventLocation = [self _XPointToOSPoint: eventLocation
                                           for: cWin];

        if (generic.flags.useWindowMakerIcons == 1)
          {
            /*
             * We must hand over control of our icon/miniwindow
             * to Window Maker.
             */
            if ((cWin->win_attrs.window_style
                 & (NSMiniWindowMask | NSIconWindowMask)) != 0
                && eventType == NSLeftMouseDown)
              {
                if (cWin->parent == None)
                  break;
                xEvent.xbutton.window = cWin->parent;
                XUngrabPointer(dpy, CurrentTime);
                XSendEvent(dpy, cWin->parent, True,
                           ButtonPressMask, &xEvent);
                XFlush(dpy);
                if (clickCount == 1)
                  break;
              }
          }

        // create NSEvent
        e = [NSEvent mouseEventWithType: eventType
                     location: eventLocation
                     modifierFlags: eventFlags
         timestamp: (NSTimeInterval)generic.lastClick / 1000.0
                     windowNumber: cWin->number
                     context: gcontext
                     eventNumber: xEvent.xbutton.serial
                     clickCount: clickCount
                     pressure: 1.0
                     buttonNumber: buttonNumber /* FIXME */
                     deltaX: deltaX
                     deltaY: deltaY
                     deltaZ: 0.];
        break;

      case ButtonRelease:
        NSDebugLLog(@"NSEvent", @"%lu ButtonRelease\n",
                    xEvent.xbutton.window);
        [self setLastTime: xEvent.xbutton.time];
        if (xEvent.xbutton.button == generic.lMouse)
          {
            if (swapMouseButtons)
              {
                eventType = NSRightMouseUp;
                buttonNumber = generic.rMouse;
              }
            else
              {
                eventType = NSLeftMouseUp;
                buttonNumber = generic.lMouse;
              }
          }
        else if (xEvent.xbutton.button == generic.rMouse
                 && generic.rMouse != 0)
          {
            if (swapMouseButtons)
              {
                eventType = NSLeftMouseUp;
                buttonNumber = generic.lMouse;
              }
            else
              {
                eventType = NSRightMouseUp;
                buttonNumber = generic.rMouse;
              }
          }
        else if (xEvent.xbutton.button == generic.mMouse
                 && generic.mMouse != 0)
          {
            eventType = NSOtherMouseUp;
            buttonNumber = generic.mMouse;
          }
        else
          {
            // we ignore release of scrollUp or scrollDown
            break;                /* Unknown button */
          }
        
        if (menuButtonEnabled == NO && eventType == menuMouseButton)
          break; // disabled menu button was released

        eventFlags = process_modifier_flags(xEvent.xbutton.state);
        // if pointer is grabbed use grab window
        xWin = (grabWindow == 0) ? xEvent.xbutton.window : grabWindow;
        if (cWin == 0 || xWin != cWin->ident)
          generic.cachedWindow = [XGServer _windowForXWindow: xWin];
        if (cWin == 0)
          break;
        eventLocation.x = xEvent.xbutton.x;
        eventLocation.y = xEvent.xbutton.y;
        eventLocation = [self _XPointToOSPoint: eventLocation
                                           for: cWin];

        e = [NSEvent mouseEventWithType: eventType
                     location: eventLocation
                     modifierFlags: eventFlags
                     timestamp: (NSTimeInterval)generic.lastTime / 1000.0
                     windowNumber: cWin->number
                     context: gcontext
                     eventNumber: xEvent.xbutton.serial
                     clickCount: clickCount
                     pressure: 1.0
                     buttonNumber: buttonNumber        /* FIXMME */
                     deltaX: 0.0
                     deltaY: 0.0
                     deltaZ: 0.0];
        break;

      case CirculateNotify:
        NSDebugLLog(@"NSEvent", @"%lu CirculateNotify\n",
                    xEvent.xcirculate.window);
        break;

      case CirculateRequest:
        NSDebugLLog(@"NSEvent", @"%lu CirculateRequest\n",
                    xEvent.xcirculaterequest.window);
        break;

      case ClientMessage:
        {
          NSTimeInterval time;
          DndClass dnd = xdnd ();
              
          NSDebugLLog(@"NSEvent", @"%lu ClientMessage - %s\n",
	    xEvent.xclient.window,
	    XGetAtomName(dpy, xEvent.xclient.message_type));
          if (cWin == 0 || xEvent.xclient.window != cWin->ident)
            {
              generic.cachedWindow
                = [XGServer _windowForXWindow: xEvent.xclient.window];
            }
          if (cWin == 0)
            break;
          if (xEvent.xclient.message_type == generic.WM_PROTOCOLS_ATOM)
            {
              [self setLastTime: (Time)xEvent.xclient.data.l[1]];
              NSDebugLLog(@"NSEvent", @"WM Protocol - %s\n",
                          XGetAtomName(dpy, xEvent.xclient.data.l[0]));

              if ((Atom)xEvent.xclient.data.l[0] == generic.WM_DELETE_WINDOW_ATOM)
                {
                  /*
                   * WM is asking us to close a window
                   */
                  eventLocation = NSMakePoint(0,0);
                  e = [NSEvent otherEventWithType: NSAppKitDefined
                               location: eventLocation
                               modifierFlags: 0
                               timestamp: 0
                               windowNumber: cWin->number
                               context: gcontext
                               subtype: GSAppKitWindowClose
                               data1: 0
                               data2: 0];
                }
              else if ((Atom)xEvent.xclient.data.l[0]
                == generic._GNUSTEP_WM_MINIATURIZE_WINDOW_ATOM)
                {
		  NSDebugLLog(@"Miniaturize", @"%lu miniaturized", cWin->number);
                  eventLocation = NSMakePoint(0,0);
                  e = [NSEvent otherEventWithType: NSAppKitDefined
                               location: eventLocation
                               modifierFlags: 0
                               timestamp: 0
                               windowNumber: cWin->number
                               context: gcontext
                               subtype: GSAppKitWindowMiniaturize
                               data1: 0
                               data2: 0];
                }
              else if ((Atom)xEvent.xclient.data.l[0]
                       == generic._GNUSTEP_WM_HIDE_APP_ATOM)
                {
                  NSDebugLLog(@"Hide", @"%lu application will be hidden", cWin->number);
                  eventLocation = NSMakePoint(0,0);
                  e = [NSEvent otherEventWithType: NSAppKitDefined
                                         location: eventLocation
                                    modifierFlags: 0
                                        timestamp: 0
                                     windowNumber: cWin->number
                                          context: gcontext
                                          subtype: GSAppKitAppHide
                                            data1: 0
                                            data2: 0];
                }              
              else if ((Atom)xEvent.xclient.data.l[0]
                == generic.WM_TAKE_FOCUS_ATOM)
                {
                  e = [self _handleTakeFocusAtom: xEvent 
                                      forContext: gcontext];
                }
              else if ((Atom)xEvent.xclient.data.l[0]
                == generic._NET_WM_PING_ATOM)
                {
                  xEvent.xclient.window = RootWindow(dpy, cWin->screen_id);
                  XSendEvent(dpy, xEvent.xclient.window, False, 
                    (SubstructureRedirectMask | SubstructureNotifyMask), 
                    &xEvent);
                }
#ifdef HAVE_X11_EXTENSIONS_SYNC_H
	      else if ((Atom)xEvent.xclient.data.l[0]
		== generic._NET_WM_SYNC_REQUEST_ATOM)
		{
		  cWin->net_wm_sync_request_counter_value_low = (Atom)xEvent.xclient.data.l[2];
		  cWin->net_wm_sync_request_counter_value_high = (Atom)xEvent.xclient.data.l[3];
		}
#endif
            }
          else if (xEvent.xclient.message_type == dnd.XdndEnter)
            {
              Window source;

              NSDebugLLog(@"NSDragging", @"  XdndEnter message\n");
              source = XDND_ENTER_SOURCE_WIN(&xEvent);
              eventLocation = NSMakePoint(0,0);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingEnter
                           data1: source
                           data2: 0];
              /* If this is a non-local drag, set the dragInfo */
              if ([XGServer _windowForXWindow: source] == NULL)
                {
                  [[XGDragView sharedDragView] setupDragInfoFromXEvent:
                                                 &xEvent];
                }
            }
          else if (xEvent.xclient.message_type == dnd.XdndPosition)
            {
              Window                source;
              Atom                action;
              NSDragOperation        operation;
              int root_x, root_y;
              Window root_child;

              NSDebugLLog(@"NSDragging", @"  XdndPosition message\n");
              source = XDND_POSITION_SOURCE_WIN(&xEvent);
              /*
                Work around a bug/feature in WindowMaker that does not
                send ConfigureNotify events for app icons.
              */
              XTranslateCoordinates(dpy, xEvent.xclient.window,
                                    RootWindow(dpy, cWin->screen_id),
                                    0, 0,
                                    &root_x, &root_y,
                                    &root_child);
              cWin->xframe.origin.x = root_x;
              cWin->xframe.origin.y = root_y;

              eventLocation.x = XDND_POSITION_ROOT_X(&xEvent) - 
                NSMinX(cWin->xframe);
              eventLocation.y = XDND_POSITION_ROOT_Y(&xEvent) - 
                NSMinY(cWin->xframe);
              eventLocation = [self _XPointToOSPoint: eventLocation
                                                 for: cWin];
              time = XDND_POSITION_TIME(&xEvent);
              action = XDND_POSITION_ACTION(&xEvent);
              operation = GSDragOperationForAction(action);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: time / 1000.0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingUpdate
                           data1: source
                           data2: operation];
              /* If this is a non-local drag, update the dragInfo */
              if ([XGServer _windowForXWindow: source] == NULL)
                {
                  [[XGDragView sharedDragView] updateDragInfoFromEvent:
                                                 e];
                }
            }
          else if (xEvent.xclient.message_type == dnd.XdndStatus)
            {
              Window target;
              Atom action;
              NSDragOperation operation;

              NSDebugLLog(@"NSDragging", @"  XdndStatus message\n");
              target = XDND_STATUS_TARGET_WIN(&xEvent);
              eventLocation = NSMakePoint(0, 0);
              if (XDND_STATUS_WILL_ACCEPT (&xEvent))
                {
                  action = XDND_STATUS_ACTION(&xEvent);
                }
              else
                {
                  action = NSDragOperationNone;
                }
                  
              operation = GSDragOperationForAction(action);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingStatus
                           data1: target
                           data2: operation];
            }
          else if (xEvent.xclient.message_type == dnd.XdndLeave)
            {
              Window source;

              NSDebugLLog(@"NSDragging", @"  XdndLeave message\n");
              source = XDND_LEAVE_SOURCE_WIN(&xEvent);
              eventLocation = NSMakePoint(0, 0);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingExit
                           data1: 0
                           data2: 0];
              /* If this is a non-local drag, reset the dragInfo */
              if ([XGServer _windowForXWindow: source] == NULL)
                {
                  [[XGDragView sharedDragView] resetDragInfo];
                }
            }
          else if (xEvent.xclient.message_type == dnd.XdndDrop)
            {
              Window source;

              NSDebugLLog(@"NSDragging", @"  XdndDrop message\n");
              source = XDND_DROP_SOURCE_WIN(&xEvent);
              eventLocation = NSMakePoint(0, 0);
              time = XDND_DROP_TIME(&xEvent);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: time / 1000.0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingDrop
                           data1: source
                           data2: 0];
            }
          else if (xEvent.xclient.message_type == dnd.XdndFinished)
            {
              Window target;

              NSDebugLLog(@"NSDragging", @"  XdndFinished message\n");
              target = XDND_FINISHED_TARGET_WIN(&xEvent);
              eventLocation = NSMakePoint(0, 0);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: eventLocation
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: cWin->number
                           context: gcontext
                           subtype: GSAppKitDraggingFinished
                           data1: target
                           data2: 0];
            }
        }
        break;

      case ColormapNotify:
        // colormap attribute
        NSDebugLLog(@"NSEvent", @"%lu ColormapNotify\n",
                    xEvent.xcolormap.window);
        break;

            // the window has been resized, change the width and height
            // and update the window so the changes get displayed
      case ConfigureNotify:
        NSDebugLLog(@"NSEvent", @"%lu ConfigureNotify "
                    @"x:%d y:%d w:%d h:%d b:%d %c", xEvent.xconfigure.window,
                    xEvent.xconfigure.x, xEvent.xconfigure.y,
                    xEvent.xconfigure.width, xEvent.xconfigure.height,
                    xEvent.xconfigure.border_width,
                    xEvent.xconfigure.send_event ? 'T' : 'F');
        if (cWin == 0 || xEvent.xconfigure.window != cWin->ident)
          {
            generic.cachedWindow
              = [XGServer _windowForXWindow:xEvent.xconfigure.window];
          }

        if (cWin != 0)
          {
            NSRect r, x, n, h;
            NSTimeInterval ts = (NSTimeInterval)generic.lastMotion;

            r = cWin->xframe;

            x = NSMakeRect(xEvent.xconfigure.x,
                           xEvent.xconfigure.y,
                           xEvent.xconfigure.width,
                           xEvent.xconfigure.height);

            /*
            According to the ICCCM, coordinates in synthetic events
            (ie. non-zero send_event) are in root space, while coordinates
            in real events are in the parent window's space. The parent
            window might be some window manager window, so we can't
            directly use those coordinates.

            Thus, if the event is real, we use XTranslateCoordinates to
            get the root space coordinates.
            */
            if (xEvent.xconfigure.send_event == 0)
              {
                int root_x, root_y;
                Window root_child;
                XTranslateCoordinates(dpy, xEvent.xconfigure.window,
                                      RootWindow(dpy, cWin->screen_id),
                                      0, 0,
                                      &root_x, &root_y,
                                      &root_child);
                x.origin.x = root_x;
                x.origin.y = root_y;
              }

            cWin->xframe = x;
            n = [self _XFrameToOSFrame: x for: cWin];
            NSDebugLLog(@"Moving",
                        @"Update win %lu:\n   original:%@\n   new:%@",
                        cWin->number, NSStringFromRect(r), 
                        NSStringFromRect(x));
            /*
             * Set size hints info to be up to date with new size.
             */
            h = [self _XFrameToXHints: x for: cWin];
            cWin->siz_hints.width = h.size.width;
            cWin->siz_hints.height = h.size.height;

            /*
            We only update the position hints if we're on the screen.
            Otherwise, the window manager might not have added decorations
            (if any) to the window yet. Since we compensate for decorations
            when we set the position, this will confuse us and we'll end
            up compensating twice, which makes windows drift.
            */
            if (cWin->map_state == IsViewable)
              {
                cWin->siz_hints.x = h.origin.x;
                cWin->siz_hints.y = h.origin.y;
              }

            /*
             * create GNUstep event(s)
             */
            if (!NSEqualSizes(r.size, x.size))
              {
		NSEvent *r;

                /* Resize events move the origin. There's no good
                   place to pass this info back, so we put it in
                   the event location field */
                r = [NSEvent otherEventWithType: NSAppKitDefined
                             location: n.origin
                             modifierFlags: eventFlags
                             timestamp: ts / 1000.0
                             windowNumber: cWin->number
                             context: gcontext
                             subtype: GSAppKitWindowResized
                             data1: n.size.width
                             data2: n.size.height];

		/* We don't add this event in event_queue, to don't delay
		 * its sent. Instead, send it directly to the window. If not,
		 * the programa can move/resize the window while we send
		 * this event, causing a confusion.
		 */
		[[NSApp windowWithWindowNumber: cWin->number] sendEvent: r];
              }
            if (!NSEqualPoints(r.origin, x.origin))
              {
		NSEvent *r;
                NSWindow *window;

                r = [NSEvent otherEventWithType: NSAppKitDefined
                             location: eventLocation
                             modifierFlags: eventFlags
                             timestamp: ts / 1000.0
                             windowNumber: cWin->number
                             context: gcontext
                             subtype: GSAppKitWindowMoved
                             data1: n.origin.x
                             data2: n.origin.y];

		/* We don't add this event in event_queue, to don't delay
		 * its sent. Instead, send it directly to the window. If not,
		 * the programa can move/resize the window while we send
		 * this event, causing a confusion.
		 */
                window = [NSApp windowWithWindowNumber: cWin->number];
                [window sendEvent: r];
                /* Update monitor_id of the backend window.
                   NSWindow may change screen pointer while processing
                   the event. */
                cWin->monitor_id = [[window screen] screenNumber];
              }
          }	
        break;

            // same as ConfigureNotify but we get this event
            // before the change has actually occurred
      case ConfigureRequest:
        NSDebugLLog(@"NSEvent", @"%lu ConfigureRequest\n",
                    xEvent.xconfigurerequest.window);
        break;

            // a window has been created
      case CreateNotify:
        NSDebugLLog(@"NSEvent", @"%lu CreateNotify\n",
                    xEvent.xcreatewindow.window);
        break;

            // a window has been destroyed
      case DestroyNotify:
        NSDebugLLog(@"NSEvent", @"%lu DestroyNotify\n",
                    xEvent.xdestroywindow.window);
        break;

            // when the pointer enters a window
      case EnterNotify:
        NSDebugLLog(@"NSEvent", @"%lu EnterNotify\n",
                    xEvent.xcrossing.window);
        break;
              
            // when the pointer leaves a window
      case LeaveNotify:
        NSDebugLLog(@"NSEvent", @"%lu LeaveNotify\n",
                    xEvent.xcrossing.window);
        if (cWin == 0 || xEvent.xcrossing.window != cWin->ident)
          {
            generic.cachedWindow
              = [XGServer _windowForXWindow: xEvent.xcrossing.window];
          }
        if (cWin == 0)
          break;
        eventLocation = NSMakePoint(-1,-1);
        e = [NSEvent otherEventWithType: NSAppKitDefined
                     location: eventLocation
                     modifierFlags: 0
                     timestamp: 0
                     windowNumber: cWin->number
                     context: gcontext
                     subtype: GSAppKitWindowLeave
                     data1: 0
                     data2: 0];
        break;

            // the visibility of a window has changed
      case VisibilityNotify:
        NSDebugLLog(@"NSEvent", @"%lu VisibilityNotify %d\n", 
                    xEvent.xvisibility.window, xEvent.xvisibility.state);
        if (cWin == 0 || xEvent.xvisibility.window != cWin->ident)
          {
            generic.cachedWindow
              = [XGServer _windowForXWindow:xEvent.xvisibility.window];
          }
        // sub-window ?
	/*
          {
            Window xw;
            xw = xEvent.xvisibility.window;
            while (cWin == 0)
              {
                Window rw, *cw; unsigned int nc;
                if ( !XQueryTree(dpy, xw, &rw, &xw, &cw, &nc) )
                  continue;
                if ( cw != NULL )
                  XFree(cw);
                generic.cachedWindow
                  = [XGServer _windowForXWindow:xw];
              }
          }
	*/
        if (cWin != 0)
          cWin->visibility = xEvent.xvisibility.state;
        break;

        // a portion of the window has become visible and
        // we must redisplay it
      case Expose:
        NSDebugLLog(@"NSEvent", @"%lu Expose\n",
                    xEvent.xexpose.window);
        {
          if (cWin == 0 || xEvent.xexpose.window != cWin->ident)
            {
              generic.cachedWindow
                = [XGServer _windowForXWindow:xEvent.xexpose.window];
            }
          // sub-window ?
	  /*
          BOOL isSubWindow = NO;

            {
              Window xw;
              xw = xEvent.xexpose.window;
              XWindowAttributes wa;
              // We should not found more than one level, but who knows ?
              while (cWin == 0)
                {
                  Window rw, *cw; unsigned int nc;
                  XGetWindowAttributes(dpy,xEvent.xexpose.window,&wa);
                  xEvent.xexpose.x += wa.x;
                  xEvent.xexpose.y += wa.y;
                  if ( !XQueryTree(dpy, xw, &rw, &xw, &cw, &nc) )
                    continue;
                  if ( cw != NULL )
                    XFree(cw);
                  generic.cachedWindow
                    = [XGServer _windowForXWindow:xw];
                }
              if ( xw != xEvent.xexpose.window )
                {
                  isSubWindow = YES;
                }
            }
	  */
          if (cWin != 0)
            {
              XRectangle rectangle;

              rectangle.x = xEvent.xexpose.x;
              rectangle.y = xEvent.xexpose.y;
              rectangle.width = xEvent.xexpose.width;
              rectangle.height = xEvent.xexpose.height;

              NSDebugLLog(@"NSEvent", @"Expose frame %d %d %d %d\n",
                          rectangle.x, rectangle.y,
                          rectangle.width, rectangle.height);
#if 0
              // ignore backing if sub-window
              [self _addExposedRectangle: rectangle : cWin->number : isSubWindow];

              if (xEvent.xexpose.count == 0)
                [self _processExposedRectangles: cWin->number];
#else
              {
                NSRect rect;
                NSTimeInterval ts = (NSTimeInterval)generic.lastMotion;
                
                rect = [self _XWinRectToOSWinRect: NSMakeRect(
                        rectangle.x, rectangle.y, rectangle.width, rectangle.height)
                             for: cWin];
                e = [NSEvent otherEventWithType: NSAppKitDefined
                             location: rect.origin
                             modifierFlags: eventFlags
                             timestamp: ts / 1000.0
                             windowNumber: cWin->number
                             context: gcontext
                             subtype: GSAppKitRegionExposed
                             data1: rect.size.width
                             data2: rect.size.height];
              }
              
#endif
            }
          break;
        }

      // keyboard focus entered a window
      case FocusIn:
        NSDebugLLog(@"NSEvent", @"%lu FocusIn\n",
                    xEvent.xfocus.window);
        if (cWin == 0 || xEvent.xfocus.window != cWin->ident)
          {
            generic.cachedWindow
                = [XGServer _windowForXWindow:xEvent.xfocus.window];
          }
        if (cWin == 0)
          break;
        NSDebugLLog(@"Focus", @"%lu got focus on %lu\n",
                    xEvent.xfocus.window, cWin->number);
        // Store this for debugging, may not be the real focus window
        generic.currentFocusWindow = cWin->number;
        if (xEvent.xfocus.serial == generic.focusRequestNumber)
          {
            /*
             * This is a response to our own request - so we mark the
             * request as complete.
             */
            generic.desiredFocusWindow = 0;
            generic.focusRequestNumber = 0;
          }
        break;
        
            // keyboard focus left a window
      case FocusOut:
        {
          Window fw;
          int rev;
          
          /*
           * See where the focus has moved to -
           * If it has gone to 'none' or 'PointerRoot' then 
           * it's not one of ours.
           * If it has gone to our root window - use the icon window.
           * If it has gone to a window - we see if it is one of ours.
           */
          XGetInputFocus(xEvent.xfocus.display, &fw, &rev);
          NSDebugLLog(@"NSEvent", @"%lu FocusOut\n",
                      xEvent.xfocus.window);
          if (fw != None && fw != PointerRoot)
            {
              generic.cachedWindow = [XGServer _windowForXWindow: fw];
              if (cWin == 0)
                {
                  generic.cachedWindow = [XGServer _windowForXParent: fw];
                }
              if (cWin == 0)
                {
                  nswin = nil;
                }
              else
                {
                  nswin = GSWindowWithNumber(cWin->number);
                }
            }
          else
            {
              nswin = nil;
            }
          NSDebugLLog(@"Focus", @"Focus went to %lu (xwin %lu)\n", 
                      (nswin != nil) ? cWin->number : 0, fw);

          // Focus went to a window not in this application.
          if (nswin == nil)
            {
              [NSApp deactivate];
            }

          // Clean up old focus request
          generic.cachedWindow
              = [XGServer _windowForXWindow: xEvent.xfocus.window];
          NSDebugLLog(@"Focus", @"%lu lost focus on %lu\n",
                      xEvent.xfocus.window, (cWin) ? cWin->number : 0);
          generic.currentFocusWindow = 0;
          if (cWin && generic.desiredFocusWindow == cWin->number)
            {
              /* Request not valid anymore since we lost focus */
              generic.focusRequestNumber = 0;
            }
        }
        break;

      case GraphicsExpose:
        NSDebugLLog(@"NSEvent", @"%lu GraphicsExpose\n",
                    xEvent.xexpose.window);
        break;

      case NoExpose:
        NSDebugLLog(@"NSEvent", @"NoExpose\n");
        break;

      // window is moved because of a change in the size of its parent
      case GravityNotify:
        NSDebugLLog(@"NSEvent", @"%lu GravityNotify\n",
                    xEvent.xgravity.window);
        break;

        // a key has been pressed
      case KeyPress:
        NSDebugLLog(@"NSEvent", @"%lu KeyPress\n",
                    xEvent.xkey.window);
        [self setLastTime: xEvent.xkey.time];
        e = process_key_event (&xEvent, self, NSKeyDown, event_queue, NO);
        break;

        // a key has been released
      case KeyRelease:
        NSDebugLLog(@"NSEvent", @"%lu KeyRelease\n",
                    xEvent.xkey.window);
        [self setLastTime: xEvent.xkey.time];

        /*
          For key repeats X creates two corresponding KeyRelease/KeyPress events.
          So, first we check for the KeyRelease event, take a look at the next
          event in the queue and look if they are a matching KeyRelease/KeyPress
          pair. If so, we ignore the current KeyRelease event.
        */
        if (XEventsQueued(dpy, QueuedAfterReading))
          {
            XEvent nev;
            XPeekEvent(dpy, &nev);

            if (nev.type == KeyPress && 
                nev.xkey.window == xEvent.xkey.window &&
                nev.xkey.time == xEvent.xkey.time &&
                nev.xkey.keycode == xEvent.xkey.keycode)
              {
                // Ignore the current KeyRelease event.
              }
            else
              {
                /*
                if (nev.type == ClientMessage)
                  {
                    NSDebugLLog(@"NSEvent", @"Next event ClientMessage type %ld %s",
                                xEvent.xclient.message_type, 
                                XGetAtomName(dpy, xEvent.xclient.message_type));
                  }
                */
                e = process_key_event(&xEvent, self, NSKeyUp, event_queue, NO);
              }
          }
        else
          {
            e = process_key_event(&xEvent, self, NSKeyUp, event_queue, NO);
          }
        break;

        // reports the state of the keyboard when pointer or
        // focus enters a window
      case KeymapNotify:
        {
          if (_is_keyboard_initialized == NO)
            initialize_keyboard ();

          NSDebugLLog(@"NSEvent", @"%lu KeymapNotify\n",
                      xEvent.xkeymap.window);

          // Check if shift is pressed
          _shift_pressed = 0;
          if (check_modifier (&xEvent, XK_Shift_L))
            {
              _shift_pressed |= 1;
            }
          if (check_modifier (&xEvent, XK_Shift_R))
            {
              _shift_pressed |= 2;
            }

          // Check if control is pressed
          _control_pressed = 0;
          if ((_control_keysyms[0] != NoSymbol)
              && check_modifier (&xEvent, _control_keysyms[0]))
            {
              _control_pressed |= 1;
            }
          if ((_control_keysyms[1] != NoSymbol)
              && check_modifier (&xEvent, _control_keysyms[1]))
            {
              _control_pressed |= 2;
            }

          // Check if command is pressed
          _command_pressed = 0;
          if ((_command_keysyms[0] != NoSymbol)
              && check_modifier (&xEvent, _command_keysyms[0]))
            {
              _command_pressed |= 1;
            }
          if ((_command_keysyms[1] != NoSymbol)
              && check_modifier (&xEvent, _command_keysyms[1]))
            {
              _command_pressed |= 2;
            }

          // Check if alt is pressed
          _alt_pressed = 0;
          if ((_alt_keysyms[0] != NoSymbol)
              && check_modifier (&xEvent, _alt_keysyms[0]))
            {
              _alt_pressed |= 1;
            }
          if ((_alt_keysyms[1] != NoSymbol)
              && check_modifier (&xEvent, _alt_keysyms[1]))
            {
              _alt_pressed |= 2;
            }

          // Check if help is pressed
          _help_pressed = 0;
          if ((_help_keysyms[0] != NoSymbol)
              && check_modifier (&xEvent, _help_keysyms[0]))
            {
              _help_pressed |= 1;
            }
          if ((_help_keysyms[1] != NoSymbol)
              && check_modifier (&xEvent, _help_keysyms[1]))
            {
              _help_pressed |= 2;
            }
        }
        break;

            // when a window changes state from ummapped to
            // mapped or vice versa
      case MapNotify:
        NSDebugLLog(@"NSEvent", @"%lu MapNotify\n",
                    xEvent.xmap.window);
        if (cWin == 0 || xEvent.xmap.window != cWin->ident)
          {
            generic.cachedWindow
                = [XGServer _windowForXWindow:xEvent.xmap.window];
          }
        if (cWin != 0)
          {
            cWin->map_state = IsViewable;
            /*
             * if the window that was just mapped wants the input
             * focus, re-do the request.
             */
            if (generic.desiredFocusWindow == cWin->number
                && generic.focusRequestNumber == 0)
              {
                NSDebugLLog(@"Focus", @"Refocusing %lu on map notify", 
                            cWin->number);
                [self setinputfocus: cWin->number];
              }
            /*
             * Make sure that the newly mapped window displays.
             */
            nswin = GSWindowWithNumber(cWin->number);
            [nswin update];
          }
        break;

            // Window is no longer visible.
      case UnmapNotify:
        NSDebugLLog(@"NSEvent", @"%lu UnmapNotify\n",
                    xEvent.xunmap.window);
        if (cWin == 0 || xEvent.xunmap.window != cWin->ident)
          {
            generic.cachedWindow
              = [XGServer _windowForXWindow:xEvent.xunmap.window];
          }
        if (cWin != 0)
          {
            cWin->map_state = IsUnmapped;
            cWin->visibility = -1;
          }
        break;

            // like MapNotify but occurs before the request is carried out
      case MapRequest:
        NSDebugLLog(@"NSEvent", @"%lu MapRequest\n",
                    xEvent.xmaprequest.window);
        break;

            // keyboard or mouse mapping has been changed by another client
      case MappingNotify:
        NSDebugLLog(@"NSEvent", @"%lu MappingNotify\n",
                    xEvent.xmapping.window);
        if ((xEvent.xmapping.request == MappingModifier) 
            || (xEvent.xmapping.request == MappingKeyboard))
          {
            XRefreshKeyboardMapping (&xEvent.xmapping);
            set_up_num_lock ();
          }
        break;

      case MotionNotify:
        NSDebugLLog(@"NSMotionEvent", @"%lu MotionNotify - %d %d\n",
                    xEvent.xmotion.window, xEvent.xmotion.x, xEvent.xmotion.y);
        {
          unsigned int        state;

          /*
           * Compress motion events to avoid flooding.
           */
          while (XPending(xEvent.xmotion.display))
            {
              XEvent        peek;

              XPeekEvent(xEvent.xmotion.display, &peek);
              if (peek.type == MotionNotify
                  && xEvent.xmotion.window == peek.xmotion.window
                  && xEvent.xmotion.subwindow == peek.xmotion.subwindow)
                {
                  XNextEvent(xEvent.xmotion.display, &xEvent);
                }
              else
                {
                  break;
                }
            }

          generic.lastMotion = xEvent.xmotion.time;
          [self setLastTime: generic.lastMotion];
          state = xEvent.xmotion.state;
          if (state & generic.lMouseMask)
            {
              eventType = NSLeftMouseDragged;
            }
          else if (state & generic.rMouseMask)
            {
              eventType = NSRightMouseDragged;
            }
          else if (state & generic.mMouseMask)
            {
              eventType = NSOtherMouseDragged;
            }
          else
            {
              eventType = NSMouseMoved;
            }

          eventFlags = process_modifier_flags(state);
          // if pointer is grabbed use grab window instead
          xWin = (grabWindow == 0)
            ? xEvent.xmotion.window : grabWindow;
          if (cWin == 0 || xWin != cWin->ident)
            generic.cachedWindow = [XGServer _windowForXWindow: xWin];
          if (cWin == 0)
            break;

          deltaX = - eventLocation.x;
          deltaY = - eventLocation.y;
          eventLocation = NSMakePoint(xEvent.xmotion.x, xEvent.xmotion.y);
          eventLocation = [self _XPointToOSPoint: eventLocation
                                             for: cWin];
          deltaX += eventLocation.x;
          deltaY += eventLocation.y;

          e = [NSEvent mouseEventWithType: eventType
                       location: eventLocation
                       modifierFlags: eventFlags
                       timestamp: (NSTimeInterval)generic.lastTime / 1000.0
                       windowNumber: cWin->number
                       context: gcontext
                       eventNumber: xEvent.xbutton.serial
                       clickCount: clickCount
                       pressure: 1.0
                       buttonNumber: 0 /* FIXME */
                       deltaX: deltaX
                       deltaY: deltaY
                       deltaZ: 0];
          break;
        }

      // a window property has changed or been deleted
      case PropertyNotify:
        NSDebugLLog(@"NSEvent", @"%lu PropertyNotify - '%s'\n",
                    xEvent.xproperty.window,
                    XGetAtomName(dpy, xEvent.xproperty.atom));
	if (xEvent.xproperty.atom == generic.WM_STATE_ATOM)
	  {
	    if (cWin == 0 || xEvent.xproperty.window != cWin->ident)
	      {
		generic.cachedWindow
		  = [XGServer _windowForXWindow: xEvent.xproperty.window];
	      }
	    if (cWin != 0)
	      {
		int new_state;

		/* Get the new window state */
		if (xEvent.xproperty.state == PropertyNewValue)
		  new_state = [self _wm_state: xEvent.xproperty.window];
		else
		  new_state = WithdrawnState;

		switch (new_state)
		  {
		  case IconicState:
		    /* Post miniaturize event upon transition from NormalState
		       to IconicState. If the window manager supports the ewmh
		       specification, also check that the _NET_WM_STATE
		       property includes _NET_WM_STATE_HIDDEN. */
		    /* Note: Don't rely on WM_STATE (nor on _NET_WM_STATE) with
		       Window Maker, since it is impossible to distinguish
		       miniaturized windows from hidden windows by their window
		       properties. Fortunately, Window Maker sends us a client
		       message when a window is miniaturized. */
		    if ((generic.wm & XGWM_WINDOWMAKER) == 0 &&
			cWin->wm_state == NormalState &&
			((generic.wm & XGWM_EWMH) == 0 ||
			 [self _ewmh_isHidden: xEvent.xproperty.window] == YES))
		      {
			/* Same event as when we get ClientMessage with the
			 * atom equal to generic._GNUSTEP_WM_MINIATURIZE_WINDOW_ATOM
			 */
			NSDebugLLog(@"Miniaturize", @"%lu miniaturized",
				    cWin->number);
			eventLocation = NSMakePoint(0,0);
			e = [NSEvent otherEventWithType: NSAppKitDefined
				     location: eventLocation
				     modifierFlags: 0
				     timestamp: xEvent.xproperty.time / 1000
				     windowNumber: cWin->number
				     context: gcontext
				     subtype: GSAppKitWindowMiniaturize
				     data1: 0
				     data2: 0];
		      }
		    break;

		  case NormalState:
		    /* Post deminiaturize event upon transition from IconicState
		       to NormalState, but only if our window is actually
		       miniaturized. */
		    if (cWin->wm_state == IconicState &&
			[GSWindowWithNumber(cWin->number) isMiniaturized])
		      {
			NSDebugLLog(@"Miniaturize", @"%lu deminiaturized",
				    cWin->number);
			eventLocation = NSMakePoint(0,0);
			e = [NSEvent otherEventWithType: NSAppKitDefined
				     location: eventLocation
				     modifierFlags: 0
				     timestamp: xEvent.xproperty.time / 1000
				     windowNumber: cWin->number
				     context: gcontext
				     subtype: GSAppKitWindowDeminiaturize
				     data1: 0
				     data2: 0];
		      }
		    break;
		  }

		/* save the new state */
		cWin->wm_state = new_state;
	      }
	  }
        break;

            // a client successfully reparents a window
      case ReparentNotify:
        NSDebugLLog(@"NSEvent", @"%lu ReparentNotify - offset %d %d\n",
                    xEvent.xreparent.window, xEvent.xreparent.x,
                    xEvent.xreparent.y);
        if (cWin == 0 || xEvent.xreparent.window != cWin->ident)
          {
            generic.cachedWindow
              = [XGServer _windowForXWindow:xEvent.xreparent.window];
          }
        if (cWin != 0)
          {
            cWin->parent = xEvent.xreparent.parent;
          }

        if (cWin != 0 && xEvent.xreparent.parent != cWin->root
          && (xEvent.xreparent.x != 0 || xEvent.xreparent.y != 0))
          {
            Window parent = xEvent.xreparent.parent;
            XWindowAttributes wattr;
            float l;
            float r;
            float t;
            float b;
            Offsets *o;

            /* Get the WM offset info which we hope is the same
             * for all parented windows with the same style.
             * The coordinates in the event are insufficient to determine
             * the offsets as the new parent window may have a border,
             * so we must get the attributes of that window and use them
             * to determine our offsets.
             */
            XGetWindowAttributes(dpy, parent, &wattr);
            NSDebugLLog(@"NSEvent", @"Parent border,width,height %d,%d,%d\n",
              wattr.border_width, wattr.width, wattr.height);
            l = xEvent.xreparent.x + wattr.border_width;
            t = xEvent.xreparent.y + wattr.border_width;

            /* Find total parent size and subtract window size and
             * top-left-corner offset to determine bottom-right-corner
             * offset.
             */
            r = wattr.width + wattr.border_width * 2;
            r -= (cWin->xframe.size.width + l);
            b = wattr.height + wattr.border_width * 2;
            b -= (cWin->xframe.size.height + t);

            // Some window manager e.g. KDE2 put in multiple windows,
            // so we have to find the right parent, closest to root
            /* FIXME: This section of code has caused problems with
               certain users. An X error occurs in XQueryTree and
               later a seg fault in XFree. It's 'commented' out for
               now unless you set the default 'GSDoubleParentWindows'
            */
            if (generic.flags.doubleParentWindow)
              {
                Window new_parent = parent;

                r = wattr.width + wattr.border_width * 2;
                b = wattr.height + wattr.border_width * 2;
                while (new_parent && (new_parent != cWin->root))
                  {
                    Window root;
                    Window *children;
                    unsigned int nchildren;

                    parent = new_parent;
                    NSLog(@"QueryTree window is %lu (cwin root %lu)", 
                          parent, cWin->root);
                    if (!XQueryTree(dpy, parent, &root, &new_parent, 
                      &children, &nchildren))
                      {
                        new_parent = None;
                        if (children)
                          {
                            NSLog(@"Bad pointer from failed X call?");
                            children = 0;
                          }
                      }
                    if (children)
                      {
                        XFree(children);
                      }
                    if (new_parent && new_parent != cWin->root)
                      {
                        XWindowAttributes pattr;

                        XGetWindowAttributes(dpy, parent, &pattr);
                        l += pattr.x + pattr.border_width;
                        t += pattr.y + pattr.border_width;
                        r = pattr.width + pattr.border_width * 2;
                        b = pattr.height + pattr.border_width * 2;
                      }
                  } /* while */
                r -= (cWin->xframe.size.width + l);
                b -= (cWin->xframe.size.height + t);
              } /* generic.flags.doubleParentWindow */

            o = generic.offsets + (cWin->win_attrs.window_style & 15);
            if (o->known == NO)
              {
                o->l = l;
                o->r = r;
                o->t = t;
                o->b = b;
                o->known = YES;
                /* FIXME: if offsets have changed, from previously guessed
                 * versions, we should go through window list and fix up
                 * hints.
                 */
              }
            else
              {
                BOOL changed = NO;

                if (l != o->l)
                  {
                    NSLog(@"Ignore left offset change from %d to %d",
                      (int)o->l, (int)l);
                    changed = YES;
                  }
                if (r != o->r)
                  {
                    NSLog(@"Ignore right offset change from %d to %d",
                      (int)o->r, (int)r);
                    changed = YES;
                  }
                if (t != o->t)
                  {
                    NSLog(@"Ignore top offset change from %d to %d",
                      (int)o->t, (int)t);
                    changed = YES;
                  }
                if (b != o->b)
                  {
                    NSLog(@"Ignore bottom offset change from %d to %d",
                      (int)o->b, (int)b);
                    changed = YES;
                  }
                if (changed == YES)
                  {
                    NSLog(@"Reparent was with offset %d %d\n",
                      xEvent.xreparent.x, xEvent.xreparent.y);
                    NSLog(@"Parent border,width,height %d,%d,%d\n",
                      wattr.border_width, wattr.width, wattr.height);
                  }
              }

            /* Work around a bug in Window Maker, which does not preserve
             * the document edited status and uses the wrong close button
             * when a window is shown again after hiding it
             */
            if (generic.wm & XGWM_WINDOWMAKER)
              {
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
                XChangeProperty(dpy, cWin->ident, generic._GNUSTEP_WM_ATTR_ATOM,
                                generic._GNUSTEP_WM_ATTR_ATOM, 32, PropModeReplace,
                                (unsigned char *)&cWin->win_attrs,
                                sizeof(GNUstepWMAttributes)/sizeof(CARD32));
              }
          }
        break;

            // another client attempts to change the size of a window
      case ResizeRequest:
        NSDebugLLog(@"NSEvent", @"%lu ResizeRequest\n",
                    xEvent.xresizerequest.window);
        break;

            // events dealing with the selection
      case SelectionClear:
        NSDebugLLog(@"NSEvent", @"%lu SelectionClear\n",
                    xEvent.xselectionclear.window);
        break;

      case SelectionNotify:
        NSDebugLLog(@"NSEvent", @"%lu SelectionNotify\n",
                    xEvent.xselection.requestor);
        break;

      case SelectionRequest:
        NSDebugLLog(@"NSEvent", @"%lu SelectionRequest\n",
                    xEvent.xselectionrequest.requestor);
        {
          NSPasteboard *pb = [NSPasteboard pasteboardWithName: NSDragPboard];
          NSArray *types = [pb types];
          NSData *data = nil;
          Atom xType = xEvent.xselectionrequest.target;

          if (((xType == generic.UTF8_STRING_ATOM) || 
               (xType == XA_STRING) || 
               (xType == generic.TEXT_ATOM)) &&
              [types containsObject: NSStringPboardType])
            {
              NSString *s = [pb stringForType: NSStringPboardType];

              if (xType == generic.UTF8_STRING_ATOM)
                {
                  data = [s dataUsingEncoding: NSUTF8StringEncoding];
                }
              else if ((xType == XA_STRING) || (xType == generic.TEXT_ATOM))
                {
                  data = [s dataUsingEncoding: NSISOLatin1StringEncoding];
                }
            }
          // FIXME: Add support for more types. See: xpbs.m

          if (data != nil)
            {
              DndClass dnd = xdnd();

              // Send the data to the other process
              xdnd_selection_send(&dnd, &xEvent.xselectionrequest, 
                                  (unsigned char *)[data bytes], [data length]);        
            }
        }
        break;

            // We shouldn't get here unless we forgot to trap an event above
      default:
#ifdef XSHM
        if (xEvent.type == XShmGetEventBase(dpy)+ShmCompletion
            && [gcontext respondsToSelector: @selector(gotShmCompletion:)])
          {
            [gcontext gotShmCompletion: 
                        ((XShmCompletionEvent *)&xEvent)->drawable];
            break;
          }
#endif
#ifdef HAVE_XRANDR
        int randr_event_type = randrEventBase + RRScreenChangeNotify;
        if (xEvent.type == randr_event_type
            && (xEvent.xconfigure.window == RootWindow(dpy, defScreen)))
          {
            // Check if other RandR events are waiting in the queue.
            XSync(dpy, 0);
            while (XCheckTypedEvent(dpy, randr_event_type, &xEvent)) {;}
                    
            XRRUpdateConfiguration(event);
            // Regenerate NSScreens
            [NSScreen resetScreens];
            // Notify application about screen parameters change
            [[NSNotificationCenter defaultCenter]
                      postNotificationName: NSApplicationDidChangeScreenParametersNotification
                                    object: NSApp];
          }
        break;
#endif
        NSLog(@"Received an untrapped event\n");
        break;
    }
  if (e)
    {
      [event_queue addObject: e];
    }
  e = nil;
}

/*
 * WM is asking us to take the keyboard focus
 */
- (NSEvent *)_handleTakeFocusAtom: (XEvent)xEvent 
                       forContext: (NSGraphicsContext *)gcontext
{
  NSWindow *keyWindow = [NSApp keyWindow];
  int key_num = [keyWindow windowNumber];
  NSEvent *e = nil;
  
  NSDebugLLog(@"Focus",
              @"TakeFocus received by: %li (%lu) (focused = %lu, key = %d)",
              cWin->number, xEvent.xfocus.window,
              generic.currentFocusWindow, key_num);

  /* Invalidate the previous request. It's possible the app lost focus
     before this request was fufilled and we are being focused again,
     or ??? */
  {
    generic.focusRequestNumber = 0;
    generic.desiredFocusWindow = 0;
  }
  
  /* Sometimes window managers lose the setinputfocus on the key window
   * e.g. when ordering out a window with focus then ordering in the key window.   
   * it might search for a window until one accepts its take focus request.
   */
  if (key_num == 0)
    {
      cWin->ignore_take_focus = NO;
    }
  else if (cWin->number == [[[NSApp mainMenu] window] windowNumber])
    {
      cWin->ignore_take_focus = NO;
    }

  /* We'd like to send this event directly to the front-end to handle,
     but the front-end polls events so slowly compared the speed at
     which X events could potentially come that we could easily get
     out of sync, particularly when there are a lot of window
     events */
  if ([NSApp isHidden])
    {
      if (generic.wm & XGWM_WINDOWMAKER)
        {
          /* If window receives WM_TAKE_FOCUS and application is in hidden
             state - it's time to unhide. There's no other method to
             tell us to unhide. */
          NSDebugLLog(@"Focus", @"WM take focus while hidden - unhiding.");
          [NSApp unhide: nil];
        }
      else
        {
          /* This often occurs when hidding an app, since a bunch of
             windows get hidden at once, and the WM is searching for a
             window to take focus after each one gets hidden. */
          NSDebugLLog(@"Focus", @"WM take focus while hiding");
        }
    }
  else if (cWin->ignore_take_focus == YES)
    {
      NSDebugLLog(@"Focus", @"Ignoring window focus request");
      cWin->ignore_take_focus = NO;
    }
  else if (cWin->number == key_num)
    {
      NSDebugLLog(@"Focus", @"Reasserting key window");
      [GSServerForWindow(keyWindow) setinputfocus: key_num];
    }
  else if (key_num 
           && cWin->number == [[[NSApp mainMenu] window] windowNumber])
    {
      gswindow_device_t *key_window = [XGServer _windowWithTag:key_num];
      /* This might occur when the window manager just wants someone
         to become key, so it tells the main menu (typically the first
         menu in the list), but since we already have a window that
         was key before, use that instead */
      NSDebugLLog(@"Focus", @"Key window is already %d", key_num);
      if (key_window->map_state == IsUnmapped) {
        /* `key_window` was unmapped by window manager. 
           this window and `key_window` are on the different workspace. */
        [GSServerForWindow(keyWindow) setinputfocus: cWin->number];
      }
      else {
        [GSServerForWindow(keyWindow) setinputfocus: key_num];
      }
    }
  else
    {
      NSPoint eventLocation;
      /*
       * Here the app asked for this (if keyWindow==nil) or there was a
       * click on the title bar or some other reason (window mapped,
       * etc). We don't necessarily want to forward the event for the
       * last reason but we just have to deal with that since we can
       * never be sure if it's necessary.
       */
      eventLocation = NSMakePoint(0,0);
      e = [NSEvent otherEventWithType:NSAppKitDefined
                   location: eventLocation
                   modifierFlags: 0
                   timestamp: 0
                   windowNumber: cWin->number
                   context: gcontext
                   subtype: GSAppKitWindowFocusIn
                   data1: 0
                   data2: 0];
    }
  return e;
}


// Return the key_sym corresponding to the user defaults string given,
// or fallback if no default is registered.
static KeySym
key_sym_from_defaults (Display *display, NSUserDefaults *defaults,
                       NSString *keyDefaultKey, KeySym fallback)
{
  NSString *keyDefaultName;
  KeySym key_sym;

  keyDefaultName = [defaults stringForKey: keyDefaultKey];
  if (keyDefaultName == nil)
    return fallback;

  key_sym = XStringToKeysym ([keyDefaultName cString]);
#if 0
  if (key_sym == NoSymbol && [keyDefaultName intValue] > 0)
    {
      key_sym = [keyDefaultName intValue];
    }
#endif
  if (key_sym == NoSymbol)
    {
      // This is not necessarily an error.
      // If you want on purpose to disable a key, 
      // set its default to 'NoSymbol'.
      NSLog (@"KeySym %@ not found; disabling %@", keyDefaultName,
                                                   keyDefaultKey);
    }

  return key_sym;
}

// This function should be called before any keyboard event is dealed with.
static void
initialize_keyboard (void)
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  Display *display = [XGServer xDisplay];

  // Below must be stored and checked as keysyms, not keycodes, since
  // more than one keycode may be mapped t the same keysym
  // Initialize Control
  _control_keysyms[0] = key_sym_from_defaults(display, defaults,
                                              @"GSFirstControlKey",
                                              XK_Control_L);

  _control_keysyms[1] = key_sym_from_defaults(display, defaults,
                                              @"GSSecondControlKey",
                                              XK_Control_R);

  if (_control_keysyms[0] == _control_keysyms[1])
    _control_keysyms[1] = NoSymbol;

  // Initialize Command
  _command_keysyms[0] = key_sym_from_defaults(display, defaults,
                                              @"GSFirstCommandKey",
                                              XK_Alt_L);

  _command_keysyms[1] = key_sym_from_defaults(display, defaults,
                                              @"GSSecondCommandKey",
                                              NoSymbol);

  if (_command_keysyms[0] == _command_keysyms[1])
    _command_keysyms[1] = NoSymbol;

  // Initialize Alt
  _alt_keysyms[0] = key_sym_from_defaults(display, defaults,
                                          @"GSFirstAlternateKey",
                                          XK_Alt_R);
  if (XKeysymToKeycode(display, _alt_keysyms[0]) == 0)
    _alt_keysyms[0] = XK_Mode_switch;

  _alt_keysyms[1] = key_sym_from_defaults(display, defaults,
                                          @"GSSecondAlternateKey",
                                          NoSymbol);

  if (_alt_keysyms[0] == _alt_keysyms[1])
    _alt_keysyms[1] = NoSymbol;

  // Initialize Help
  _help_keysyms[0] = key_sym_from_defaults(display, defaults,
                                          @"GSFirstHelpKey",
                                          XK_Help);
  if (XKeysymToKeycode(display, _help_keysyms[0]) == 0)
    _help_keysyms[0] = NoSymbol;

  _help_keysyms[1] = key_sym_from_defaults(display, defaults,
                                          @"GSSecondHelpKey",
                                          XK_Super_L);

  if (_help_keysyms[0] == _help_keysyms[1])
    _help_keysyms[1] = NoSymbol;

  
  set_up_num_lock ();
  _mod_ignore_shift = ![defaults boolForKey: @"GSModifiersAreNotKeys"];
  
  _is_keyboard_initialized = YES;
}


static void
set_up_num_lock (void)
{
  XModifierKeymap *modifier_map;
  int i, j;
  unsigned int modifier_masks[8] = 
  {
    ShiftMask, LockMask, ControlMask, Mod1Mask, 
    Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
  };
  Display *display = [XGServer xDisplay];
  KeyCode _num_lock_keycode;
  
  // Get NumLock keycode
  _num_lock_keycode = XKeysymToKeycode (display, XK_Num_Lock);
  if (_num_lock_keycode == 0)
    {
      // Weird.  There is no NumLock in this keyboard.
      _num_lock_mask = 0; 
      return;
    }

  // Get the current modifier mapping
  modifier_map = XGetModifierMapping (display);
  
  // Scan the modifiers for NumLock
  for (j = 0; j < 8; j++)
    for (i = 0; i < (modifier_map->max_keypermod); i++)
      {
        if ((modifier_map->modifiermap)[i + j*modifier_map->max_keypermod] 
          == _num_lock_keycode)
          {
            _num_lock_mask = modifier_masks[j];
            XFreeModifiermap (modifier_map);
            return;
          }
      }
  // Weird.  NumLock is not among the modifiers
  _num_lock_mask = 0;
  XFreeModifiermap (modifier_map);
  return;
}

static BOOL
keysym_is_X_modifier (KeySym keysym)
{
  switch (keysym)
    {
    case XK_Num_Lock: 
    case XK_Caps_Lock:  
    case XK_Shift_Lock: 
      return YES;

    default:
      return NO;
    }
}

static NSEvent*
process_key_event (XEvent* xEvent, XGServer* context, NSEventType eventType,
                   NSMutableArray *event_queue, BOOL keyRepeat)
{
  NSString *keys, *ukeys;
  KeySym keysym;
  NSPoint eventLocation;
  unsigned short keyCode;
  unsigned int eventFlags;
  unichar unicode;
  NSEvent *event = nil;
  NSEventType originalType;
  gswindow_device_t *window;
  int shift_key = 0;
  int control_key = 0;
  int command_key = 0;
  int alt_key = 0;
  int help_key = 0;
  KeySym modKeysym;  // process modifier independently of shift, etc.
  
  if (_is_keyboard_initialized == NO)
    initialize_keyboard ();

  window = [XGServer _windowWithTag: [[NSApp keyWindow] windowNumber]];
  if (!window)
    {
      // No key event if we don't have a key window
      return nil;
    }

  /* Process location */
  eventLocation.x = xEvent->xbutton.x;
  eventLocation.y = xEvent->xbutton.y;
  eventLocation = [context _XPointToOSPoint: eventLocation
                                        for: window];
    
  /* Process characters */
  keys = [context->inputServer lookupStringForEvent: (XKeyEvent *)xEvent
                 window: window
                 keysym: &keysym];

  /* Process keycode */
  keyCode = ((XKeyEvent *)xEvent)->keycode;
  //ximKeyCode = XKeysymToKeycode([XGServer currentXDisplay],keysym);

  /* Process NSFlagsChanged events.  We can't use a switch because we
     are not comparing to constants. Make sure keySym is not NoSymbol since
     XIM events can potentially return this. */
  /* Possibly ignore shift/other modifier state in determining KeySym to
     work around correct but undesired behavior with shifted modifiers.
     See Back defaults documentation for "GSModifiersAreNotKeys". */
  modKeysym = (_mod_ignore_shift == YES) ?
      XLookupKeysym((XKeyEvent *)xEvent, 0) : keysym;
  if (modKeysym != NoSymbol)
    {
      if (modKeysym == XK_Shift_L)
        {
          shift_key = 1;
        }
      else if (modKeysym == XK_Shift_R)
        {
          shift_key = 2;
        }
      else if (modKeysym == _control_keysyms[0]) 
        {
          control_key = 1;
        }
      else if (modKeysym == _control_keysyms[1])
        {
          control_key = 2;
        }
      else if (modKeysym == _command_keysyms[0]) 
        {
          command_key = 1;
        }
      else if (modKeysym == _command_keysyms[1]) 
        {
          command_key = 2;
        }
      else if (modKeysym == _alt_keysyms[0]) 
        {
          alt_key = 1;
        }
      else if (modKeysym == _alt_keysyms[1]) 
        {
          alt_key = 2;
        }
      else if (modKeysym == _help_keysyms[0]) 
        {
          help_key = 1;
        }
      else if (modKeysym == _help_keysyms[1]) 
        {
          help_key = 2;
        }
    }

  originalType = eventType;
  if (shift_key || control_key || command_key || alt_key || help_key)
    {
      eventType = NSFlagsChanged;
      if (xEvent->xkey.type == KeyPress)
        {
          if (shift_key)
            _shift_pressed |= shift_key;
          if (control_key)
            _control_pressed |= control_key;
          if (command_key)
            _command_pressed |= command_key;
          if (alt_key)
            _alt_pressed |= alt_key;
          if (help_key)
            _help_pressed |= help_key;
        }
      else if (xEvent->xkey.type == KeyRelease)
        {
          if (shift_key)
            _shift_pressed &= ~shift_key;
          if (control_key)
            _control_pressed &= ~control_key;
          if (command_key)
            _command_pressed &= ~command_key;
          if (alt_key)
            _alt_pressed &= ~alt_key;
          if (help_key)
            _help_pressed &= ~help_key;
        }
    }

  /* Process modifiers */
  eventFlags = process_modifier_flags (xEvent->xkey.state);

  /* Add NSNumericPadKeyMask if the key is in the KeyPad */
  if (IsKeypadKey (keysym))
    eventFlags = eventFlags | NSNumericPadKeyMask;

  NSDebugLLog (@"NSKeyEvent", @"keysym=%lu, keyCode=%d flags=%d (state=%d)",
              keysym, keyCode, eventFlags, ((XKeyEvent *)xEvent)->state);
  
  /* Add NSFunctionKeyMask if the key is a function or a misc function key */
  /* We prefer not to do this and do it manually in process_char
     because X's idea of what is a function key seems to be different
     from OPENSTEP's one */
  /* if (IsFunctionKey (keysym) || IsMiscFunctionKey (keysym))
       eventFlags = eventFlags | NSFunctionKeyMask; */

  /* First, check to see if the key event if a Shift, NumLock or
     CapsLock or ShiftLock keypress/keyrelease.  If it is, then use a
     NSFlagsChanged event type.  This will generate a NSFlagsChanged
     event each time you press/release a shift key, even if the flags
     haven't actually changed.  I don't see this as a problem - if we
     didn't, the shift keypress/keyrelease event would never be
     notified to the application.

     NB - to know if shift was pressed, we need to check the X keysym
     - it doesn't work to compare the X modifier flags of this
     keypress X event with the ones of the previous one, because when
     you press Shift, the X shift keypress event has the *same* X
     modifiers flags as the X keypress event before it - only
     keypresses coming *after* the shift keypress will get a different
     X modifier mask.  */
  if (keysym_is_X_modifier (keysym))
    {
      eventType = NSFlagsChanged;
    }


  if (help_key)
    {
      unicode = NSHelpFunctionKey;
      keys = [NSString stringWithCharacters: &unicode  length: 1];
      if (originalType == NSKeyDown)
        {
          event = [NSEvent keyEventWithType: NSKeyDown
                   location: eventLocation
                   modifierFlags: eventFlags
                   timestamp: (NSTimeInterval)xEvent->xkey.time / 1000.0
                   windowNumber: window->number
                   context: GSCurrentContext()
                   characters: keys
                   charactersIgnoringModifiers: keys
                   isARepeat: keyRepeat
                   keyCode: keyCode];
          [event_queue addObject: event];
          event = [NSEvent keyEventWithType: NSFlagsChanged
                   location: eventLocation
                   modifierFlags: eventFlags
                   timestamp: (NSTimeInterval)xEvent->xkey.time / 1000.0
                   windowNumber: window->number
                   context: GSCurrentContext()
                   characters: keys
                   charactersIgnoringModifiers: keys
                   isARepeat: NO
                   keyCode: keyCode];
          return event;
        }
      else
        {
          event = [NSEvent keyEventWithType: NSFlagsChanged
                   location: eventLocation
                   modifierFlags: eventFlags
                   timestamp: (NSTimeInterval)xEvent->xkey.time / 1000.0
                   windowNumber: window->number
                   context: GSCurrentContext()
                   characters: keys
                   charactersIgnoringModifiers: keys
                   isARepeat: NO
                   keyCode: keyCode];
          [event_queue addObject: event];
          event = [NSEvent keyEventWithType: NSKeyUp
                   location: eventLocation
                   modifierFlags: eventFlags
                   timestamp: (NSTimeInterval)xEvent->xkey.time / 1000.0
                   windowNumber: window->number
                   context: GSCurrentContext()
                   characters: keys
                   charactersIgnoringModifiers: keys
                   isARepeat: keyRepeat
                   keyCode: keyCode];
          return event;
        }
    }
  else
    {
      /* Now we get the unicode character for the pressed key using our
       * internal table.
       */
      unicode = process_char (keysym, &eventFlags);

      /* If that didn't work, we use what X gave us */
      if (unicode != 0)
        {
          keys = [NSString stringWithCharacters: &unicode  length: 1];
        }

      // Now the same ignoring modifiers, except Shift, ShiftLock, NumLock.
      xEvent->xkey.state = (xEvent->xkey.state & (ShiftMask | LockMask 
                                                  | _num_lock_mask));
      ukeys = [context->inputServer lookupStringForEvent: (XKeyEvent *)xEvent
                      window: window
                      keysym: &keysym];
      unicode = process_char (keysym, &eventFlags);
      if (unicode != 0)
        {
          ukeys = [NSString stringWithCharacters: &unicode  length: 1];
        }

      event = [NSEvent keyEventWithType: eventType
                   location: eventLocation
                   modifierFlags: eventFlags
                   timestamp: (NSTimeInterval)xEvent->xkey.time / 1000.0
                   windowNumber: window->number
                   context: GSCurrentContext()
                   characters: keys
                   charactersIgnoringModifiers: ukeys
                   isARepeat: keyRepeat
                   keyCode: keyCode];

      return event;
    }
}

static unichar 
process_char (KeySym keysym, unsigned *eventModifierFlags)
{
  switch (keysym)
    {
      /* NB: Whatever is explicitly put in this conversion table takes
         precedence over what is returned by XLookupString.  Not sure
         this is a good idea for latin-1 character input. */
    case XK_Return:       return NSCarriageReturnCharacter;
    case XK_KP_Enter:     return NSEnterCharacter;
    case XK_Linefeed:     return NSFormFeedCharacter;
    case XK_Tab:          return NSTabCharacter;
#ifdef XK_XKB_KEYS
    case XK_ISO_Left_Tab: return NSBackTabCharacter;
#endif
      /* FIXME: The following line ? */
    case XK_Escape:       return 0x1b;
    case XK_BackSpace:    return NSDeleteCharacter;

      /* The following keys need to be reported as function keys */
#define XGPS_FUNCTIONKEY \
*eventModifierFlags = *eventModifierFlags | NSFunctionKeyMask;

    case XK_F1:           XGPS_FUNCTIONKEY return NSF1FunctionKey;
    case XK_F2:           XGPS_FUNCTIONKEY return NSF2FunctionKey;
    case XK_F3:           XGPS_FUNCTIONKEY return NSF3FunctionKey;
    case XK_F4:           XGPS_FUNCTIONKEY return NSF4FunctionKey;
    case XK_F5:           XGPS_FUNCTIONKEY return NSF5FunctionKey;
    case XK_F6:           XGPS_FUNCTIONKEY return NSF6FunctionKey;
    case XK_F7:           XGPS_FUNCTIONKEY return NSF7FunctionKey;
    case XK_F8:           XGPS_FUNCTIONKEY return NSF8FunctionKey;
    case XK_F9:           XGPS_FUNCTIONKEY return NSF9FunctionKey;
    case XK_F10:          XGPS_FUNCTIONKEY return NSF10FunctionKey;
    case XK_F11:          XGPS_FUNCTIONKEY return NSF11FunctionKey;
    case XK_F12:          XGPS_FUNCTIONKEY return NSF12FunctionKey;
    case XK_F13:          XGPS_FUNCTIONKEY return NSF13FunctionKey;
    case XK_F14:          XGPS_FUNCTIONKEY return NSF14FunctionKey;
    case XK_F15:          XGPS_FUNCTIONKEY return NSF15FunctionKey;
    case XK_F16:          XGPS_FUNCTIONKEY return NSF16FunctionKey;
    case XK_F17:          XGPS_FUNCTIONKEY return NSF17FunctionKey;
    case XK_F18:          XGPS_FUNCTIONKEY return NSF18FunctionKey;
    case XK_F19:          XGPS_FUNCTIONKEY return NSF19FunctionKey;
    case XK_F20:          XGPS_FUNCTIONKEY return NSF20FunctionKey;
    case XK_F21:          XGPS_FUNCTIONKEY return NSF21FunctionKey;
    case XK_F22:          XGPS_FUNCTIONKEY return NSF22FunctionKey;
    case XK_F23:          XGPS_FUNCTIONKEY return NSF23FunctionKey;
    case XK_F24:          XGPS_FUNCTIONKEY return NSF24FunctionKey;
    case XK_F25:          XGPS_FUNCTIONKEY return NSF25FunctionKey;
    case XK_F26:          XGPS_FUNCTIONKEY return NSF26FunctionKey;
    case XK_F27:          XGPS_FUNCTIONKEY return NSF27FunctionKey;
    case XK_F28:          XGPS_FUNCTIONKEY return NSF28FunctionKey;
    case XK_F29:          XGPS_FUNCTIONKEY return NSF29FunctionKey;
    case XK_F30:          XGPS_FUNCTIONKEY return NSF30FunctionKey;
    case XK_F31:          XGPS_FUNCTIONKEY return NSF31FunctionKey;
    case XK_F32:          XGPS_FUNCTIONKEY return NSF32FunctionKey;
    case XK_F33:          XGPS_FUNCTIONKEY return NSF33FunctionKey;
    case XK_F34:          XGPS_FUNCTIONKEY return NSF34FunctionKey;
    case XK_F35:          XGPS_FUNCTIONKEY return NSF35FunctionKey;
    case XK_Delete:       XGPS_FUNCTIONKEY return NSDeleteFunctionKey;
    case XK_Home:         XGPS_FUNCTIONKEY return NSHomeFunctionKey;  
    case XK_Left:         XGPS_FUNCTIONKEY return NSLeftArrowFunctionKey;
    case XK_Right:        XGPS_FUNCTIONKEY return NSRightArrowFunctionKey;
    case XK_Up:           XGPS_FUNCTIONKEY return NSUpArrowFunctionKey;  
    case XK_Down:         XGPS_FUNCTIONKEY return NSDownArrowFunctionKey;
//  case XK_Prior:        XGPS_FUNCTIONKEY return NSPrevFunctionKey;
//  case XK_Next:         XGPS_FUNCTIONKEY return NSNextFunctionKey;
    case XK_End:          XGPS_FUNCTIONKEY return NSEndFunctionKey; 
    case XK_Begin:        XGPS_FUNCTIONKEY return NSBeginFunctionKey;
    case XK_Select:       XGPS_FUNCTIONKEY return NSSelectFunctionKey;
    case XK_Print:        XGPS_FUNCTIONKEY return NSPrintFunctionKey;  
    case XK_Execute:      XGPS_FUNCTIONKEY return NSExecuteFunctionKey;
    case XK_Insert:       XGPS_FUNCTIONKEY return NSInsertFunctionKey; 
    case XK_Undo:         XGPS_FUNCTIONKEY return NSUndoFunctionKey;
    case XK_Redo:         XGPS_FUNCTIONKEY return NSRedoFunctionKey;
    case XK_Menu:         XGPS_FUNCTIONKEY return NSMenuFunctionKey;
    case XK_Find:         XGPS_FUNCTIONKEY return NSFindFunctionKey;
    case XK_Help:         XGPS_FUNCTIONKEY return NSHelpFunctionKey;
    case XK_Break:        XGPS_FUNCTIONKEY return NSBreakFunctionKey;
    case XK_Mode_switch:  XGPS_FUNCTIONKEY return NSModeSwitchFunctionKey;
    case XK_Scroll_Lock:  XGPS_FUNCTIONKEY return NSScrollLockFunctionKey;
    case XK_Pause:        XGPS_FUNCTIONKEY return NSPauseFunctionKey;
    case XK_Clear:        XGPS_FUNCTIONKEY return NSClearDisplayFunctionKey;
#ifndef NeXT
    case XK_Page_Up:      XGPS_FUNCTIONKEY return NSPageUpFunctionKey;
    case XK_Page_Down:    XGPS_FUNCTIONKEY return NSPageDownFunctionKey;
    case XK_Sys_Req:      XGPS_FUNCTIONKEY return NSSysReqFunctionKey;  
#endif
    case XK_KP_F1:        XGPS_FUNCTIONKEY return NSF1FunctionKey;
    case XK_KP_F2:        XGPS_FUNCTIONKEY return NSF2FunctionKey;
    case XK_KP_F3:        XGPS_FUNCTIONKEY return NSF3FunctionKey;
    case XK_KP_F4:        XGPS_FUNCTIONKEY return NSF4FunctionKey;
#ifndef NeXT
    case XK_KP_Home:      XGPS_FUNCTIONKEY return NSHomeFunctionKey;
    case XK_KP_Left:      XGPS_FUNCTIONKEY return NSLeftArrowFunctionKey;
    case XK_KP_Up:        XGPS_FUNCTIONKEY return NSUpArrowFunctionKey;  
    case XK_KP_Right:     XGPS_FUNCTIONKEY return NSRightArrowFunctionKey;
    case XK_KP_Down:      XGPS_FUNCTIONKEY return NSDownArrowFunctionKey; 
//  case XK_KP_Prior:     return NSPrevFunctionKey;      
    case XK_KP_Page_Up:   XGPS_FUNCTIONKEY return NSPageUpFunctionKey;    
//  case XK_KP_Next:      return NSNextFunctionKey;      
    case XK_KP_Page_Down: XGPS_FUNCTIONKEY return NSPageDownFunctionKey;  
    case XK_KP_End:       XGPS_FUNCTIONKEY return NSEndFunctionKey;       
    case XK_KP_Begin:     XGPS_FUNCTIONKEY return NSBeginFunctionKey;     
    case XK_KP_Insert:    XGPS_FUNCTIONKEY return NSInsertFunctionKey;    
    case XK_KP_Delete:    XGPS_FUNCTIONKEY return NSDeleteFunctionKey;    
#endif
#undef XGPS_FUNCTIONKEY
    default:              return 0;
    }
}

// process_modifier_flags() determines which modifier keys (Command, Control,
// Shift,  and so forth) were held down while the event occured.
static unsigned int
process_modifier_flags(unsigned int state)
{
  unsigned int eventModifierFlags = 0;

  if (_shift_pressed != 0)
    eventModifierFlags = eventModifierFlags | NSShiftKeyMask;

  if (state & LockMask)
    eventModifierFlags = eventModifierFlags | NSAlphaShiftKeyMask;

  if (_control_pressed != 0)
    eventModifierFlags = eventModifierFlags | NSControlKeyMask;

  if (_command_pressed != 0)
    eventModifierFlags = eventModifierFlags | NSCommandKeyMask;

  if (_alt_pressed != 0)
    eventModifierFlags = eventModifierFlags | NSAlternateKeyMask;
  
  if (_help_pressed != 0)
    eventModifierFlags = eventModifierFlags | NSHelpKeyMask;
  
  // Other modifiers ignored for now. 

  return eventModifierFlags;
}


- (NSDate*) timedOutEvent: (void*)data
                     type: (RunLoopEventType)type
                  forMode: (NSString*)mode
{
  return nil;
}

/* Drag and Drop */
- (id <NSDraggingInfo>)dragInfo
{
  return [XGDragView sharedDragView];
}

@end

@implementation XGServer (XSync)
- (BOOL) xSyncMap: (void*)windowHandle
{
  gswindow_device_t *window = (gswindow_device_t*)windowHandle;

  /*
   * if the window is not mapped, make sure we have sent all requests to the
   * X-server, it may be that our mapping request was buffered.
   */
  if (window->map_state != IsViewable)
    {
      XSync(dpy, False);
      [self receivedEvent: 0 type: 0 extra: 0 forMode: nil];
    }
  /*
   * If the window is still not mapped, it may be that the window-manager
   * intercepted our mapping request, and hasn't dealt with it yet.
   * Listen for input for up to a second, in the hope of getting the mapping.
   */
  if (window->map_state != IsViewable)
    {
      NSDate *d = [NSDate dateWithTimeIntervalSinceNow: 1.0];
      NSRunLoop *l = [NSRunLoop currentRunLoop];
      NSString *m = [l currentMode];

      while (window->map_state != IsViewable && [d timeIntervalSinceNow] > 0)
        {
          [l runMode: m beforeDate: d];
        }
    }
  if (window->map_state != IsViewable)
    {
      NSLog(@"Window still not mapped a second after mapping request made");
      return NO;
    }
  return YES;
}
@end

@implementation XGServer (X11Ops)

/*
 * Return mouse location in base coords ignoring the event loop
 */
- (NSPoint) mouselocation
{
  return [self mouseLocationOnScreen: defScreen window: NULL];
}

- (NSPoint) mouseLocationOnScreen: (int)screen window: (int *)win
{
  Window rootWin;
  Window childWin;
  int currentX;
  int currentY;
  int winX;
  int winY;
  unsigned mask;
  BOOL ok;
  NSPoint p;
  int height;
  int screen_id;

  ok = XQueryPointer (dpy, [self xDisplayRootWindow],
                      &rootWin, &childWin, &currentX, &currentY,
                      &winX, &winY, &mask);
  p = NSMakePoint(-1,-1);
  /* FIXME: After multi-monitor support will be implemented `screen` method 
     parameter doesn't make sense. The `if{}` block should be removed since 
     we have only one screen and mouse can't be placed on "wrong" screen. 
     Also actually we need `height` of the whole Xlib screen (defScreen). */
  if (ok == False)
    {
      /* Mouse not on the specified screen_number */
      XWindowAttributes attribs;
      ok = XGetWindowAttributes(dpy, rootWin, &attribs);
      if (ok == False)
        {
          return p;
        }
      screen_id = XScreenNumberOfScreen(attribs.screen);
      if (screen >= 0 && screen != screen_id)
        {
          /* Mouse not on the requred screen, return an invalid point */
          return p;
        }
      height = attribs.height;
    }
  else
    {
      height = xScreenSize.height;
    }
  p = NSMakePoint(currentX, height - currentY);
  if (win)
    {
      gswindow_device_t *w = [XGServer _windowForXWindow: childWin];

      if (w == NULL)
        w = [XGServer _windowForXParent: childWin];
      if (w)
        *win = w->number;
      else
        *win = 0;
    }
  return p;
}

- (NSEvent*) getEventMatchingMask: (unsigned)mask
                       beforeDate: (NSDate*)limit
                           inMode: (NSString*)mode
                          dequeue: (BOOL)flag
{
  [self receivedEvent: 0 type: 0 extra: 0 forMode: nil];
  return [super getEventMatchingMask: mask
                          beforeDate: limit
                              inMode: mode
                             dequeue: flag];
}

- (void) discardEventsMatchingMask: (unsigned)mask
                       beforeEvent: (NSEvent*)limit
{
  [self receivedEvent: 0 type: 0 extra: 0 forMode: nil];
  [super discardEventsMatchingMask: mask
                       beforeEvent: limit];
}

@end

@implementation XGServer (TimeKeeping)
// Sync time with X server every 10 seconds
#define MAX_TIME_DIFF 10
// Regard an X time stamp as valid for half a second
#define OUT_DATE_TIME_DIFF 0.5

- (void) setLastTime: (Time)last
{
  if (generic.lastTimeStamp == 0 
      || generic.baseXServerTime + MAX_TIME_DIFF * 1000 < last)
    {
      // We have not sync'ed with the clock for at least
      // MAX_TIME_DIFF seconds ... so we do it now.
      generic.lastTimeStamp = [NSDate timeIntervalSinceReferenceDate];
      generic.baseXServerTime = last;
    }
  else
    {
      // Optimisation to compute the new time stamp instead.
      generic.lastTimeStamp += (last - generic.lastTime) / 1000.0;
    }

  generic.lastTime = last;
}

- (Time) lastTime
{
  // In the case of activation via DO the lastTime is outdated and cannot be used.
  if (generic.lastTimeStamp == 0 
      || ((generic.lastTimeStamp + OUT_DATE_TIME_DIFF)
          < [NSDate timeIntervalSinceReferenceDate]))
    {
      return CurrentTime;
    }
  else
    {
      return generic.lastTime;
    }
}

@end

