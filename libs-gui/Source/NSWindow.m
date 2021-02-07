/** <title>NSWindow</title>

   <abstract>The window class</abstract>

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
            Venkat Ajjanagadde <venkat@ocbi.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: June 1998
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: December 1998

   This file is part of the GNUstep GUI Library.

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

#import "config.h"
#include <math.h>
#include <float.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSException.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSUndoManager.h>

#import "AppKit/NSAnimation.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSCachedImageRep.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSColorList.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSDocumentController.h"
#import "AppKit/NSDocument.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSHelpManager.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSView.h"
#import "AppKit/NSViewController.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSWindowController.h"
#import "AppKit/PSOperators.h"

#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSTrackingRect.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSWindowDecorationView.h"
#import "GSBindingHelpers.h"
#import "GSGuiPrivate.h"
#import "GSToolTips.h"
#import "GSIconManager.h"
#import "NSToolbarFrameworkPrivate.h"
#import "NSViewPrivate.h"

#define GSI_ARRAY_TYPES 0
#define GSI_ARRAY_TYPE NSWindow *
#define GSI_ARRAY_NO_RELEASE 1
#define GSI_ARRAY_NO_RETAIN 1

#ifdef GSIArray
#undef GSIArray
#endif
#include <GNUstepBase/GSIArray.h>

static GSToolTips *toolTipVisible = nil;
static id<GSWindowDecorator> windowDecorator = nil;


BOOL GSViewAcceptsDrag(NSView *v, id<NSDraggingInfo> dragInfo);

@interface NSObject (DragInfoBackend)
- (void) dragImage: (NSImage*)anImage
                at: (NSPoint)screenLocation
            offset: (NSSize)initialOffset
             event: (NSEvent*)event
        pasteboard: (NSPasteboard*)pboard
            source: (id)sourceObject
         slideBack: (BOOL)slideFlag;
- (void) postDragEvent: (NSEvent*)event;
@end

@interface NSView (MoveToWindow)
// Normally this method is only used internally.
- (void) _viewWillMoveToWindow: (NSWindow*)newWindow;
@end

@interface NSScreen (PrivateMethods)
- (id) _initWithScreenNumber: (int)screen;
@end

@interface NSApplication(Inactive)
- (BOOL) _isWindowInactive: (NSWindow *)window;
- (void) _setWindow: (NSWindow *)window inactive: (BOOL)inactive;
@end

@interface GSWindowAnimationDelegate : NSObject
{
}
@end

@implementation GSWindowAnimationDelegate
- (void) animationDidEnd: (NSAnimation *)animation
{
  AUTORELEASE(animation);
}

- (void) animationDidStop: (NSAnimation *)animation
{
  AUTORELEASE(animation);
}

@end

static GSWindowAnimationDelegate *animationDelegate;

/*
 * Category for internal methods (for use only within the NSWindow class itself
 * or with other AppKit classes.
 */
@interface NSWindow (GNUstepPrivate)

+ (void) _addAutodisplayedWindow: (NSWindow *)w;
+ (void) _removeAutodisplayedWindow: (NSWindow *)w;
+ (void) _setToolTipVisible: (GSToolTips*)t;
+ (GSToolTips*) _toolTipVisible;

- (void) _lossOfKeyOrMainWindow;
- (NSView *) _windowView;
- (NSView *) _borderView;
- (NSScreen *) _screenForFrame: (NSRect)frame;
@end

@implementation NSWindow (GNUstepPrivate)

+ (void) _setToolTipVisible: (GSToolTips*)t
{
  toolTipVisible = t;
}

+ (GSToolTips*) _toolTipVisible
{
  return toolTipVisible;
}

/* Window autodisplay machinery. */
- (void) _handleAutodisplay
{
  if (_f.is_autodisplay && _f.views_need_display)
    {
      [self disableFlushWindow];
      [self displayIfNeeded];
      [self enableFlushWindow];
      [self flushWindowIfNeeded];
    }
}

static NSArray *modes = nil;

/* Array of windows we might need to handle autodisplay for (in practice
a list of windows that are, wrt. -gui, on-screen). */
static GSIArray_t autodisplayedWindows;

/*
This method handles all normal displaying. It is set to be run on each
runloop iteration when the first window is created

The reason why this performer is always added, as opposed to adding it
when display is needed and not re-adding it here, is that
-setNeedsDisplay* might be called from a method invoked by
-performSelector:target:argument:order:modes:, and if it is, the display
needs to happen in the same runloop iteration, before blocking for
events. If the performer were added in a call to another performer, it
wouldn't be called until the next runloop iteration, ie. after the runloop
has blocked and waited for events.
*/
+(void) _handleAutodisplay: (id)bogus
{
  int i;
  for (i = 0; i < GSIArrayCount(&autodisplayedWindows); i++)
    {
      [GSIArrayItemAtIndex(&autodisplayedWindows, i).ext _handleAutodisplay];
    }

  [[NSRunLoop currentRunLoop]
         performSelector: @selector(_handleAutodisplay:)
                  target: self
                argument: nil
                   order: 600000
                   modes: modes];
}

+(void) _addAutodisplayedWindow: (NSWindow *)w
{
  int i;
  /* If it's the first time we're called, set up the performer and modes
  array. */
  if (!modes)
    {
      modes = [[NSArray alloc] initWithObjects: NSDefaultRunLoopMode,
                               NSModalPanelRunLoopMode,
                               NSEventTrackingRunLoopMode, nil];
      [[NSRunLoop currentRunLoop]
         performSelector: @selector(_handleAutodisplay:)
                  target: self
                argument: nil
                   order: 600000
                   modes: modes];
      GSIArrayInitWithZoneAndCapacity(&autodisplayedWindows,
        NSDefaultMallocZone(), 1);
    }

  /* O(n), but it's much more important that _handleAutodisplay: can iterate
  quickly over the array. (_handleAutodisplay: is called once for every
  event, this method is only called when windows are ordered in or out.) */
  for (i = 0; i < GSIArrayCount(&autodisplayedWindows); i++)
    {
      if (GSIArrayItemAtIndex(&autodisplayedWindows, i).ext == w)
        {
          return;
        }
    }

  GSIArrayAddItem(&autodisplayedWindows, (GSIArrayItem)w);
}

+(void) _removeAutodisplayedWindow: (NSWindow *)w
{
  int i;
  for (i = 0; i < GSIArrayCount(&autodisplayedWindows); i++)
    {
      if (GSIArrayItemAtIndex(&autodisplayedWindows, i).ext == w)
        {
          GSIArrayRemoveItemAtIndex(&autodisplayedWindows, i);
          return;
        }
    }

  /* This happens eg. if a window is ordered out twice. In such cases,
  the window has already been removed from the list, so we don't need
  to do anything here. */
}


/* We get here if we were ordered out or miniaturized. In this case if
   we were the key or main window, go through the list of all windows
   and try to find another window that can take our place as key
   and/or main. Automatically ignore windows that cannot become
   key/main and skip the main menu window (which is the only
   non-obvious window that can become key) unless we have no choice
   (i.e. all the candidate windows were ordered out.)
*/
- (void) _lossOfKeyOrMainWindow
{
  NSArray *windowList = GSOrderedWindows();
  NSUInteger pos = [windowList indexOfObjectIdenticalTo: self];
  NSUInteger c = [windowList count];
  NSUInteger i;

  // Don't bother when application is closing.
  if ([NSApp isRunning] == NO)
    {
      return;
    }

  if (!c)
    {
      return;
    }

  if (pos == NSNotFound)
    {
      pos = c;
    }

  if ([self isKeyWindow])
    {
      NSWindow *w = [NSApp mainWindow];

      [self resignKeyWindow];
      if (w != nil && w != self
          && [w canBecomeKeyWindow])
        {
          [w makeKeyWindow];
        }
      else
        {
          NSWindow *menu_window = [[NSApp mainMenu] window];

          // try all windows front to back except self and menu
          for (i = 0; i < c; i++)
            {
              if (i != pos)
                {
                  w = [windowList objectAtIndex: i];
                  if ([w isVisible] && [w canBecomeKeyWindow]
                      && w != menu_window)
                    {
                      [w makeKeyWindow];
                      break;
                    }
                }
            }

          /*
           * if we didn't find a possible key window - use the main menu window
           */
          if (i == c)
            {
              if (menu_window != nil)
                {
                  // FIXME: Why this call and not makeKeyWindow?
                  [GSServerForWindow(menu_window) setinputfocus:
                                        [menu_window windowNumber]];
                }
            }
        }
    }

  if ([self isMainWindow])
    {
      NSWindow *w = [NSApp keyWindow];

      [self resignMainWindow];
      if (w != nil && [w canBecomeMainWindow])
        {
          [w makeMainWindow];
        }
      else
        {
         // try all windows front to back except self
          for (i = 0; i < c; i++)
            {
              if (i != pos)
                {
                  w = [windowList objectAtIndex: i];
                  if ([w isVisible] && [w canBecomeMainWindow])
                    {
                      [w makeMainWindow];
                      break;
                    }
                }
            }
        }
    }
}

- (NSView *) _windowView
{
  return _wv;
}

/*
 * This is just the Apple name for our _windowView method.
 */
- (NSView *) _borderView
{
  return _wv;
}

/* Support method to properly implement the 'screen' method for NSWindow.
   According to documentation the 'screen' method should return the screen
   that the window "show up the most or nil".  This method supports the 'screen'
   method and internal requests for the correct 'screen' based on the
   supplied frame request.
*/
- (NSScreen *) _screenForFrame: (NSRect)frame
{
  // FIXME: We always return the first screen, if there is no overlap.
  // Other code relies on [window screen] not returning nil.
  CGFloat    largest   = -1.0;
  NSArray   *screens   = [NSScreen screens];
  NSInteger  index     = 0;
  NSScreen  *theScreen = nil;

  for (index = 0; index < [screens count]; ++index)
    {
      NSScreen  *screen = [screens objectAtIndex: index];
      NSRect     sframe = [screen frame];
      NSRect     iframe = NSIntersectionRect(frame, sframe);
      CGFloat    isize  = NSWidth(iframe) * NSHeight(iframe);

      if (isize > largest)
        {
          largest   = isize;
          theScreen = screen;
        }
    }
  NSDebugLLog(@"NSWindow", @"%s: frame: %@ screen: %@ size: %ld\n", __PRETTY_FUNCTION__,
              NSStringFromRect(frame), theScreen, (long)largest);

  return theScreen;
}

@end


@interface NSMiniWindow : NSWindow
@end

@implementation NSMiniWindow

- (BOOL) canBecomeMainWindow
{
  return NO;
}

- (BOOL) canBecomeKeyWindow
{
  return NO;
}

- (void) _initDefaults
{
  [super _initDefaults];
  [self setExcludedFromWindowsMenu: YES];
  [self setReleasedWhenClosed: NO];
  /* App icons and mini windows are displayed at dock level by default. Yet,
     with the current window level mapping in -back, some window managers
     will order pop up and context menus behind app icons and mini windows.
     Therefore, it is possible to have app icons and mini windows displayed
     at normal window level under control of a user preference. */
  // See also NSIconWindow _initDefaults in NSApplication.m
  if ([[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSAllowWindowsOverIcons"] == YES)
    {
      _windowLevel = NSDockWindowLevel;
    }
}

@end

@interface NSMiniWindowView : NSView
{
  NSCell *imageCell;
  NSTextFieldCell *titleCell;
}
- (void) setImage: (NSImage*)anImage;
- (void) setTitle: (NSString*)aString;
@end

static NSCell *tileCell = nil;

static NSSize scaledIconSizeForSize(NSSize imageSize)
{
  NSSize iconSize, retSize;

  iconSize = GSGetIconSize();
  retSize.width = imageSize.width * iconSize.width / 64;
  retSize.height = imageSize.height * iconSize.height / 64;
  return retSize;
}

@implementation NSMiniWindowView

+ (void) initialize
{
  NSImage *tileImage;
  NSSize iconSize;

  iconSize = GSGetIconSize();

  tileImage = [[NSImage imageNamed: @"common_MiniWindowTile"] copy];
  [tileImage setScalesWhenResized: YES];
  [tileImage setSize: iconSize];

  tileCell = [[NSCell alloc] initImageCell: tileImage];
  RELEASE(tileImage);
  [tileCell setBordered: NO];
}

- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent
{
  return YES;
}

- (void) dealloc
{
  TEST_RELEASE(imageCell);
  TEST_RELEASE(titleCell);
  [super dealloc];
}

- (void) drawRect: (NSRect)rect
{
  NSSize iconSize = GSGetIconSize();

  [tileCell drawWithFrame: NSMakeRect(0, 0, iconSize.width, iconSize.height)
                   inView: self];
  [imageCell
       drawWithFrame: NSMakeRect(iconSize.width / 8,
                                 iconSize.height / 16,
                                 iconSize.width - ((iconSize.width / 8) * 2),
                                 iconSize.height - ((iconSize.height / 8) * 2))
              inView: self];
  [titleCell drawWithFrame: NSMakeRect(3, iconSize.height - 13,
                                       iconSize.width - 6, 10)
                    inView: self];
}

- (void) mouseDown: (NSEvent*)theEvent
{
  if ([theEvent clickCount] >= 2)
    {
      NSWindow *w = [_window counterpart];
      [w deminiaturize: self];
    }
  else
    {
      NSPoint lastLocation;
      NSPoint location;
      NSUInteger eventMask = NSLeftMouseDownMask | NSLeftMouseUpMask
        | NSPeriodicMask | NSOtherMouseUpMask | NSRightMouseUpMask;
      NSDate *theDistantFuture = [NSDate distantFuture];
      BOOL done = NO;

      lastLocation = [theEvent locationInWindow];
      [NSEvent startPeriodicEventsAfterDelay: 0.02 withPeriod: 0.02];

      while (!done)
        {
          theEvent = [NSApp nextEventMatchingMask: eventMask
                                        untilDate: theDistantFuture
                                           inMode: NSEventTrackingRunLoopMode
                                          dequeue: YES];

          switch ([theEvent type])
            {
              case NSRightMouseUp:
              case NSOtherMouseUp:
              case NSLeftMouseUp:
              /* right mouse up or left mouse up means we're done */
                done = YES;
                break;
              case NSPeriodic:
                location = [_window mouseLocationOutsideOfEventStream];
                if (NSEqualPoints(location, lastLocation) == NO)
                  {
                    NSPoint origin = [_window frame].origin;

                    origin.x += (location.x - lastLocation.x);
                    origin.y += (location.y - lastLocation.y);
                    [_window setFrameOrigin: origin];
                  }
                break;

              default:
                break;
            }
        }
      [NSEvent stopPeriodicEvents];
    }
}

- (void) setImage: (NSImage*)anImage
{
  NSImage *imgCopy = [anImage copy];

  [imgCopy setScalesWhenResized: YES];
  if (imgCopy != nil)
    {
      [imgCopy setSize: scaledIconSizeForSize([imgCopy size])];
    }

  if (imageCell == nil)
    {
      imageCell = [[NSCell alloc] initImageCell: imgCopy];
      [imageCell setBordered: NO];
    }
  else
    {
      [imageCell setImage: imgCopy];
    }
  RELEASE(imgCopy);
  [self setNeedsDisplay: YES];
}

- (void) setTitle: (NSString*)aString
{
  if (titleCell == nil)
    {
      CGFloat fontSize;

      titleCell = [[NSTextFieldCell alloc] initTextCell: aString];
      [titleCell setSelectable: NO];
      [titleCell setEditable: NO];
      [titleCell setBordered: NO];
      [titleCell setAlignment: NSCenterTextAlignment];
      [titleCell setDrawsBackground: NO];
      [titleCell setTextColor: [NSColor whiteColor]];
      fontSize = [NSFont systemFontSizeForControlSize: NSMiniControlSize];
      [titleCell setFont: [NSFont systemFontOfSize: fontSize]];
    }
  else
    {
      [titleCell setStringValue: aString];
    }
  [self setNeedsDisplay: YES];
}

@end



/*****************************************************************************
 *
 *        NSWindow
 *
 *****************************************************************************/

/**
  <unit>
  <heading>NSWindow</heading>

  <p> Instances of the NSWindow class handle on-screen windows, their
  associated NSViews, and events generate by the user.  An NSWindow's
  size is defined by its frame rectangle, which encompasses its entire
  structure, and its content rectangle, which includes only the
  content.
  </p>

  <p> Every NSWindow has a content view, the NSView which forms the
  root of the window's view hierarchy.  This view can be set using the
  <code>setContentView:</code> method, and accessed through the
  <code>contentView</code> method.  <code>setContentView:</code>
  replaces the default content view created by NSWindow.
  </p>

  <p> Other views may be added to the window by using the content
  view's <code>addSubview:</code> method.  These subviews can also
  have subviews added, forming a tree structure, the view hierarchy.
  When an NSWindow must display itself, it causes this hierarchy to
  draw itself.  Leaf nodes in the view hierarchy are drawn last,
  causing them to potentially obscure views further up in the
  hierarchy.
  </p>

  <p> A delegate can be specified for an NSWindow, which will receive
  notifications of events pertaining to the window.  The delegate is
  set using <code>setDelegate:</code>, and can be retrieved using
  <code>delegate</code>.  The delegate can restrain resizing by
  implementing the <code>windowWillResize: toSize:</code> method, or
  control the closing of the window by implementing
  <code>windowShouldClose:</code>.
  </p>

  </unit>
*/
@implementation NSWindow

/*
 * Class variables
 */
static SEL        ccSel;
static SEL        ctSel;
static IMP        ccImp;
static IMP        ctImp;
static Class      responderClass;
static Class      viewClass;
static NSMutableSet *autosaveNames;
static NSMapTable *windowmaps = NULL;
static NSMapTable *windowUndoManagers = NULL;
static NSNotificationCenter *nc = nil;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSWindow class])
    {
      [self setVersion: 3];
      ccSel = @selector(_checkCursorRectangles:forEvent:);
      ctSel = @selector(_checkTrackingRectangles:forEvent:);
      ccImp = [self instanceMethodForSelector: ccSel];
      ctImp = [self instanceMethodForSelector: ctSel];
      responderClass = [NSResponder class];
      viewClass = [NSView class];
      autosaveNames = [NSMutableSet new];
      windowmaps = NSCreateMapTable(NSIntMapKeyCallBacks,
                                    NSNonRetainedObjectMapValueCallBacks, 20);
      nc = [NSNotificationCenter defaultCenter];

      [self exposeBinding: NSTitleBinding];
    }
}

+ (instancetype) windowWithContentViewController: (NSViewController *)viewController
{
  NSView *view = [viewController view];
  NSRect frame = [view frame];
  NSString *title = [viewController title];
  NSUInteger style = NSTitledWindowMask |
    NSClosableWindowMask |
    NSMiniaturizableWindowMask |
    NSResizableWindowMask;
  NSWindow *window =  [[self alloc] initWithContentRect: frame
                                              styleMask: style
                                                backing: NSBackingStoreBuffered
                                                  defer: NO];
  [window setTitle: title];
  [window setContentView: view];
  [view setNeedsDisplay: YES];
  AUTORELEASE(window);
  return window;
}

+ (void) removeFrameUsingName: (NSString*)name
{
  if (name != nil)
    {
      NSString *key;

      key = [NSString stringWithFormat: @"NSWindow Frame %@", name];
      [[NSUserDefaults standardUserDefaults] removeObjectForKey: key];
    }
}

+ (NSRect) contentRectForFrameRect: (NSRect)aRect
                         styleMask: (NSUInteger)aStyle
{
  if (!windowDecorator)
    {
      windowDecorator = [GSWindowDecorationView windowDecorator];
    }

  return [windowDecorator contentRectForFrameRect: aRect
                                        styleMask: aStyle];
}

+ (NSRect) frameRectForContentRect: (NSRect)aRect
                         styleMask: (NSUInteger)aStyle
{
  if (!windowDecorator)
    {
      windowDecorator = [GSWindowDecorationView windowDecorator];
    }

  return [windowDecorator frameRectForContentRect: aRect
                                        styleMask: aStyle];
}

+ (CGFloat) minFrameWidthWithTitle: (NSString *)aTitle
                         styleMask: (NSUInteger)aStyle
{
  if (!windowDecorator)
    {
      windowDecorator = [GSWindowDecorationView windowDecorator];
    }

  return [windowDecorator minFrameWidthWithTitle: aTitle
                                       styleMask: aStyle];
}

/* default Screen and window depth */
+ (NSWindowDepth) defaultDepthLimit
{
  return [[NSScreen deepestScreen] depth];
}

+ (void) menuChanged: (NSMenu*)aMenu
{
  // FIXME: This method is for MS Windows only, does nothing
  // on other window systems
}

/*
 * Instance methods
 */
- (id) init
{
  NSUInteger style = NSTitledWindowMask | NSClosableWindowMask
    | NSMiniaturizableWindowMask | NSResizableWindowMask;

  return [self initWithContentRect: NSZeroRect
                         styleMask: style
                           backing: NSBackingStoreBuffered
                             defer: NO];
}

/*
It is important to make sure that the window is in a meaningful state after
this has been called, and that the backend window can be recreated later,
since one-shot windows may have their backend windows created and terminated
many times.
*/
- (void) _terminateBackendWindow
{
  if (_windowNum)
    {
      [_wv setWindowNumber: 0];

      /* Check for context also as it might have disappeared before us */
      if (_context && _gstate)
        {
          GSUndefineGState(_context, _gstate);
          _gstate = 0;
        }

      if (_context)
        {
          /*
             If there was a context, clear it and let it remove the
             window in that process. This indirection is needed so solve the
             circular references between the window and the context.
             But first undo the release call in _startBackendWindow.
          */
          RETAIN(self);
          DESTROY(_context);
        }

      [GSServerForWindow(self) termwindow: _windowNum];
      NSMapRemove(windowmaps, (void*)(intptr_t)_windowNum);
      _windowNum = 0;
    }
}

- (void) dealloc
{
  // Remove all key value bindings for this object.
  [GSKeyValueBinding unbindAllForObject: self];

  DESTROY(_toolbar);
  [nc removeObserver: self];
  [[self class] _removeAutodisplayedWindow: self];
  [NSApp removeWindowsItem: self];
  [NSApp _windowWillDealloc: self];

  NSAssert([NSApp keyWindow] != self, @"window being deallocated is key");
  NSAssert([NSApp mainWindow] != self, @"window being deallocated is main");

  if (windowUndoManagers != NULL)
    {
      NSMapRemove(windowUndoManagers, self);
    }

  if (_autosaveName != nil)
    {
      [autosaveNames removeObject: _autosaveName];
      _autosaveName = nil;
    }

  if (_counterpart != 0 && (_styleMask & NSMiniWindowMask) == 0)
    {
      NSWindow *mini = [NSApp windowWithWindowNumber: _counterpart];

      _counterpart = 0;
      RELEASE(mini);
    }

  /* Terminate backend window early so that the receiver is no longer returned
     from GSOrderedWindows() or GSAllWindows(). This helps to prevent crashes
     due to a race with some lame window managers under X that do not update
     the _NET_CLIENT_LIST_STACKING property quickly enough. GSOrderedWindows()
     may be called (indirectly) when a tooltip is visible while a window is
     deallocated. */
  if (_windowNum)
    {
      [self _terminateBackendWindow];
    }

  /* Clean references to this window - important if some of the views
     are not deallocated now */
  [_wv _viewWillMoveToWindow: nil];
  /* NB: releasing the window view does not necessarily result in the
     deallocation of the window's views ! - some of them might be
     retained for some other reason by the programmer or by other
     parts of the code */
  DESTROY(_wv);
  DESTROY(_fieldEditor);
  DESTROY(_backgroundColor);
  DESTROY(_representedFilename);
  DESTROY(_miniaturizedTitle);
  DESTROY(_miniaturizedImage);
  DESTROY(_windowTitle);
  DESTROY(_rectsBeingDrawn);
  DESTROY(_initialFirstResponder);
  DESTROY(_defaultButtonCell);
  DESTROY(_cachedImage);
  DESTROY(_children);
  DESTROY(_lastLeftMouseDownView);
  DESTROY(_lastRightMouseDownView);
  DESTROY(_lastOtherMouseDownView);
  DESTROY(_lastDragView);
  DESTROY(_screen);

  /*
   * FIXME This should not be necessary - the views should have removed
   * their drag types, so we should already have been removed.
   */
  [GSServerForWindow(self) removeDragTypes: nil fromWindow: self];

  if (_delegate != nil)
    {
      [nc removeObserver: _delegate  name: nil  object: self];
      _delegate = nil;
    }

  [super dealloc];
}

- (NSString*) description
{
  return [[super description] stringByAppendingFormat: @"Number: %ld Title: %@",
    (long) [self windowNumber], [self title]];
}

- (void) _startBackendWindow
{
  NSDictionary *info;

  NSMapInsert(windowmaps, (void*)(intptr_t)_windowNum, self);

  // Make sure not to create an autoreleased object,
  // as this will lead to problems when the window is deallocated.
  info = [[NSDictionary alloc]
             initWithObjects: &self
             forKeys: &NSGraphicsContextDestinationAttributeName
             count: 1];
  _context = [[NSGraphicsContext alloc] initWithContextInfo: info];
  RELEASE(info);
  if (_context)
    {
      // Now the context retains the window, release it once to make up
      RELEASE(self);
    }

  // Set window in new _gstate
  _gstate = GSDefineGState(_context);

  {
    NSRect frame = _frame;
    frame.origin = NSZeroPoint;
    [_wv setFrame: frame];
    [_wv setWindowNumber: _windowNum];
    [_wv setDocumentEdited: _f.is_edited];
    [_wv setNeedsDisplay: YES];
  }
}

- (void) _initBackendWindow
{
  NSCountedSet *dragTypes;
  GSDisplayServer *srv = GSCurrentServer();

  /* If we were deferred or one shot, our drag types may not have
     been registered properly in the backend. Remove them then re-add
     them when we create the window */
  dragTypes = [srv dragTypesForWindow: self];
  if (dragTypes)
    {
      // As this is the original entry, it will change soon.
      // We use a copy to reregister the same types later on.
      dragTypes = [dragTypes copy];

      /* Now we need to remove all the drag types for this window.  */
      [srv removeDragTypes: nil fromWindow: self];
    }

  _windowNum = [srv window: _frame
                    : _backingType
                    : _styleMask
                    : [_screen screenNumber]];
  if (_windowNum == 0)
    {
      [NSException raise: @"No Window" format: @"Failed to obtain window from the back end"];
    }
  [srv setwindowlevel: [self level] : _windowNum];
  if (_parent != nil)
    {
      [srv setParentWindow: [_parent windowNumber]
            forChildWindow: _windowNum];
    }

  // Set up context
  [self _startBackendWindow];

  /* Ok, now add the drag types back */
  if (dragTypes)
    {
      id type;
      NSMutableArray *dragTypesArray = [NSMutableArray array];
      NSEnumerator *enumerator = [dragTypes objectEnumerator];

      NSDebugLLog(@"NSWindow", @"Resetting drag types for window");
      /* Now we need to restore the drag types.  */

      /* Put all the drag types to the dragTypesArray - counted
       * with their multiplicity.
       */
      while ((type = [enumerator nextObject]) != nil)
        {
          NSUInteger i, count = [dragTypes countForObject: type];

          for (i = 0; i < count; i++)
            {
              [dragTypesArray addObject: type];
            }
        }

      /* Now store the array.  */
      [srv addDragTypes: dragTypesArray toWindow: self];
      // Free our local copy.
      RELEASE(dragTypes);
    }

  /* Other stuff we need to do for deferred windows */
  if (!NSEqualSizes(_minimumSize, NSZeroSize))
    {
      [self setMinSize: _minimumSize];
    }
  if (!NSEqualSizes(_maximumSize, NSZeroSize))
    {
      [self setMaxSize: _maximumSize];
    }
  if (!NSEqualSizes(_increments, NSZeroSize))
    {
      [self setResizeIncrements: _increments];
    }

  NSDebugLLog(@"NSWindow", @"Created NSWindow window frame %@",
              NSStringFromRect(_frame));
}

/*
 * Initializing and getting a new NSWindow object
 */
/**
  <p> Initializes the receiver with a content rect of
  <var>contentRect</var>, a style mask of <var>styleMask</var>, and a
  backing store type of <var>backingType</var>.  This is the designated
  initializer.
  </p>

  <p> The style mask values are <code>NSTitledWindowMask</code>, for a
  window with a title, <code>NSClosableWindowMask</code>, for a window
  with a close widget, <code>NSMiniaturizableWindowMask</code>, for a
  window with a miniaturize widget, and
  <code>NSResizableWindowMask</code>, for a window with a resizing
  widget.  These mask values can be OR'd in any combination.
  </p>

  <p> Backing store values are <code>NSBackingStoreBuffered</code>,
  <code>NSBackingStoreRetained</code> and
  <code>NSBackingStoreNonretained</code>.
  </p>
*/
- (id) initWithContentRect: (NSRect)contentRect
                 styleMask: (NSUInteger)aStyle
                   backing: (NSBackingStoreType)bufferingType
                     defer: (BOOL)flag
{
  NSRect  cframe;

  NSAssert(NSApp,
    @"The shared NSApplication instance must be created before windows "
    @"can be created.");

  NSDebugLLog(@"NSWindow", @"NSWindow start of init\n");

  // FIXME: This hack is here to work around a gorm decoding problem.
  if (_windowNum)
    {
      NSLog(@"Window already initialized %d", (int)_windowNum);
      return self;
    }

  /* Initialize attributes and flags */
  [super init];
  [self _initDefaults];

  _attachedSheet = nil;
  _backingType = bufferingType;
  _styleMask = aStyle;
  ASSIGN(_screen, [NSScreen mainScreen]);
  _depthLimit = [_screen depth];

  _frame = [NSWindow frameRectForContentRect: contentRect styleMask: aStyle];
  _minimumSize = NSMakeSize(_frame.size.width - contentRect.size.width + 1,
                            _frame.size.height - contentRect.size.height + 1);
  _maximumSize = NSMakeSize (10e4, 10e4);
  [self setFrame: _frame display: NO];

  [self setNextResponder: NSApp];

  _f.cursor_rects_enabled = YES;
  _f.cursor_rects_valid = NO;

  /* Create the window view */
  cframe.origin = NSZeroPoint;
  cframe.size = _frame.size;
  if (!windowDecorator)
    {
      windowDecorator = [GSWindowDecorationView windowDecorator];
    }

  _wv = [windowDecorator newWindowDecorationViewWithFrame: cframe
                                                   window: self];
  [_wv _viewWillMoveToWindow: self];
  [_wv setNextResponder: self];

  /* Create the content view */
  cframe.origin = NSZeroPoint;
  cframe.size = contentRect.size;
  [self setContentView: AUTORELEASE([[NSView alloc] initWithFrame: cframe])];

  /* rectBeingDrawn is variable used to optimize flushing the backing store.
     It is set by NSGraphicsContext during a lockFocus to tell NSWindow what
     part a view is drawing in, so NSWindow only has to flush that portion */
  _rectsBeingDrawn = [[NSMutableArray alloc] initWithCapacity: 10];

  /* Create window (if not deferred) */
  _windowNum = 0;
  _gstate = 0;
  if (flag == NO)
    {
      NSDebugLLog(@"NSWindow", @"Creating NSWindow\n");
      [self _initBackendWindow];
    }
  else
    {
      NSDebugLLog(@"NSWindow", @"Deferring NSWindow creation\n");
    }

  [nc addObserver: self
         selector: @selector(colorListChanged:)
             name: NSColorListDidChangeNotification
           object: nil];
  [nc addObserver: self
         selector: @selector(applicationDidChangeScreenParameters:)
             name: NSApplicationDidChangeScreenParametersNotification
           object: NSApp];

  NSDebugLLog(@"NSWindow", @"NSWindow end of init\n");
  return self;
}

/**
  <p> Initializes the receiver with a content rect of
  <var>contentRect</var>, a style mask of <var>styleMask</var>, a
  backing store type of <var>backingType</var> and a boolean
  <var>flag</var>.  <var>flag</var> specifies whether the window
  should be created now (<code>NO</code>), or when it is displayed
  (<code>YES</code>).
  </p>

  <p> The style mask values are <code>NSTitledWindowMask</code>, for a
  window with a title, <code>NSClosableWindowMask</code>, for a window
  with a close widget, <code>NSMiniaturizableWindowMask</code>, for a
  window with a miniaturize widget, and
  <code>NSResizableWindowMask</code>, for a window with a resizing
  widget.  These mask values can be OR'd in any combination.
  </p>

  <p> Backing store values are <code>NSBackingStoreBuffered</code>,
  <code>NSBackingStoreRetained</code> and
  <code>NSBackingStoreNonretained</code>.
  </p>
*/
- (id) initWithContentRect: (NSRect)contentRect
                 styleMask: (NSUInteger)aStyle
                   backing: (NSBackingStoreType)bufferingType
                     defer: (BOOL)flag
                    screen: (NSScreen*)aScreen
{
  self = [self initWithContentRect: contentRect
                         styleMask: aStyle
                           backing: bufferingType
                             defer: flag];
  if (self && aScreen != nil)
    {
      ASSIGN(_screen, aScreen);
      _depthLimit = [_screen depth];
    }
  return self;
}

- (id) initWithWindowRef: (void *)windowRef
{
  NSRect contentRect;
  unsigned int aStyle;
  NSBackingStoreType bufferingType;
  NSScreen* aScreen;
  int screen;
  NSInteger winNum;
  GSDisplayServer *srv = GSCurrentServer();

  // Get the properties for the underlying window
  winNum = [srv nativeWindow: windowRef : &contentRect : &bufferingType
                                : &aStyle : &screen];

  // Get the screen for the right screen number.
  aScreen = [[NSScreen alloc] _initWithScreenNumber: screen];

  // Set up a NSWindow with the same properties
  self = [self initWithContentRect: contentRect
                         styleMask: aStyle
                           backing: bufferingType
                             defer: YES
                            screen: aScreen];
  RELEASE(aScreen);

  // Fake the initialisation of the backend
  _windowNum = winNum;

  // Set up context
  [self _startBackendWindow];

  return self;
}

- (void) colorListChanged:(NSNotification*)notif
{
  if ([[notif object] isEqual: [NSColorList colorListNamed: @"System"]])
    {
      [_wv setNeedsDisplay: YES];
    }
}

- (NSRect) contentRectForFrameRect: (NSRect)frameRect
{
  return [_wv contentRectForFrameRect: frameRect styleMask: _styleMask];
}

- (NSRect) frameRectForContentRect: (NSRect)contentRect
{
  return [_wv frameRectForContentRect: contentRect styleMask: _styleMask];
}

/*
 * Accessing the content view
 */
- (id) contentView
{
  return _contentView;
}

/**
  Sets the window's content view to <var>aView</var>, replacing any
  previous content view.  */
- (void) setContentView: (NSView*)aView
{
  if (aView == _contentView)
    {
      return;
    }

  if (_contentView != nil)
    {
      [_contentView removeFromSuperview];
    }
  _contentView = aView;

  if (_contentView != nil)
    {
      [_wv setContentView: _contentView];
    }
}

/*
 * Window graphics
 */
- (NSColor*) backgroundColor
{
  return _backgroundColor;
}

- (NSString*) representedFilename
{
  return _representedFilename;
}

- (void) setBackgroundColor: (NSColor*)color
{
  ASSIGN(_backgroundColor, color);
  [_wv setBackgroundColor: color];
}

- (void) setRepresentedFilename: (NSString*)aString
{
  ASSIGN(_representedFilename, aString);
}

/** Sets the window's title to the string <var>aString</var>. */
- (void) setTitle: (NSString*)aString
{
  if ([_windowTitle isEqual: aString] == NO)
    {
      ASSIGNCOPY(_windowTitle, aString);
      [self setMiniwindowTitle: _windowTitle];
      [_wv setTitle: _windowTitle];
      if (_f.menu_exclude == NO && _f.has_opened == YES)
        {
          [NSApp changeWindowsItem: self
                             title: _windowTitle
                          filename: NO];
        }
    }
}

static NSString *
titleWithRepresentedFilename(NSString *representedFilename)
{
  return [NSString stringWithFormat: @"%@  --  %@",
		   [representedFilename lastPathComponent],
		   [[representedFilename stringByDeletingLastPathComponent]
		     stringByAbbreviatingWithTildeInPath]];
}

- (BOOL) _hasTitleWithRepresentedFilename
{
  NSString *aString = titleWithRepresentedFilename(_representedFilename);
  return [_windowTitle isEqualToString: aString];
}

- (void) setTitleWithRepresentedFilename: (NSString*)aString
{
  [self setRepresentedFilename: aString];
  aString = titleWithRepresentedFilename(aString);
  if (![_windowTitle isEqual: aString])
    {
      ASSIGNCOPY(_windowTitle, aString);
      [self setMiniwindowTitle: _windowTitle];
      [_wv setTitle: _windowTitle];
      if (_f.menu_exclude == NO && _f.has_opened == YES)
        {
          [NSApp changeWindowsItem: self
                             title: _windowTitle
                          filename: YES];
        }
    }
}

- (NSUInteger) styleMask
{
  return _styleMask;
}

/** Returns an NSString containing the text of the window's title. */
- (NSString*) title
{
  return _windowTitle;
}

- (void) setHasShadow: (BOOL)hasShadow
{
  _f.has_shadow = hasShadow;
  if (_windowNum)
    {
      [GSServerForWindow(self) setShadow: hasShadow : _windowNum];
    }
}

- (BOOL) hasShadow
{
  return _f.has_shadow;
}

- (void) invalidateShadow
{
// FIXME
}

- (void) setAlphaValue: (CGFloat)windowAlpha
{
  _alphaValue = windowAlpha;
  if (_windowNum)
    {
      [GSServerForWindow(self) setalpha: _alphaValue : _windowNum];
    }
}

- (CGFloat) alphaValue
{
  return _alphaValue;
}

- (void) setOpaque: (BOOL)isOpaque
{
  // FIXME
  _f.is_opaque = isOpaque;
}

- (BOOL) isOpaque
{
  return _f.is_opaque;
}

/*
 * Window device attributes
 */
- (NSBackingStoreType) backingType
{
  return _backingType;
}

- (NSDictionary*) deviceDescription
{
  return [[self screen] deviceDescription];
}

- (NSGraphicsContext*) graphicsContext
{
  return _context;
}

- (CGFloat) userSpaceScaleFactor
{
  if (_styleMask & NSUnscaledWindowMask)
    {
      return 1.0;
    }
  else if (_screen != nil)
     {
       return [_screen userSpaceScaleFactor];
     }
  else
    {
      return 1.0;
    }
}

- (NSInteger) gState
{
  if (_gstate <= 0)
    {
      NSDebugLLog(@"NSWindow", @"gState called on deferred window");
    }
  return _gstate;
}

- (BOOL) isOneShot
{
  return _f.is_one_shot;
}

- (void) setBackingType: (NSBackingStoreType)type
{
  _backingType = type;
}

- (void) setOneShot: (BOOL)flag
{
  _f.is_one_shot = flag;
}

- (NSInteger) windowNumber
{
  if (_windowNum <= 0)
    {
      NSDebugLLog(@"NSWindow", @"windowNumber called on deferred window");
    }
  return _windowNum;
}

/*
 * The miniwindow
 */
- (NSImage*) miniwindowImage
{
  return _miniaturizedImage;
}

- (NSString*) miniwindowTitle
{
  return _miniaturizedTitle;
}

- (void) setMiniwindowImage: (NSImage*)image
{
  ASSIGN(_miniaturizedImage, image);
  if (_counterpart != 0 && (_styleMask & NSMiniWindowMask) == 0)
    {
      NSMiniWindow      *mini;
      id                v;

      mini = (NSMiniWindow*)[NSApp windowWithWindowNumber: _counterpart];
      v = [mini contentView];
      if ([v respondsToSelector: @selector(setImage:)])
        {
          [v setImage: [self miniwindowImage]];
        }
    }
}

- (void) setMiniwindowTitle: (NSString*)title
{
  ASSIGN(_miniaturizedTitle, title);
  if (_counterpart != 0 && (_styleMask & NSMiniWindowMask) == 0)
    {
      NSMiniWindow      *mini;
      id                v;

      mini = (NSMiniWindow*)[NSApp windowWithWindowNumber: _counterpart];
      v = [mini contentView];
      if ([v respondsToSelector: @selector(setTitle:)])
        {
          [v setTitle: [self miniwindowTitle]];
        }
    }
}

- (NSWindow*) counterpart
{
  if (_counterpart == 0)
    {
      return nil;
    }
  return [NSApp windowWithWindowNumber: _counterpart];
}

/*
 * The field editor
 */
- (void) endEditingFor: (id)anObject
{
  NSText *t = [self fieldEditor: NO
                    forObject: anObject];

  if (t && (_firstResponder == t))
    {
      // Change first responder first to avoid recusion.
      _firstResponder = self;
      [_firstResponder becomeFirstResponder];
      [nc postNotificationName: NSTextDidEndEditingNotification
          object: t];
      [t setText: @""];
      [t setDelegate: nil];
      [t removeFromSuperview];
    }
}

- (NSText*) fieldEditor: (BOOL)createFlag forObject: (id)anObject
{
  /* ask delegate if it can provide a field editor */
  if ((_delegate != anObject)
    && [_delegate respondsToSelector:
    @selector(windowWillReturnFieldEditor:toObject:)])
    {
      NSText *editor;

      editor = [_delegate windowWillReturnFieldEditor: self
                                             toObject: anObject];

      if (editor != nil)
        {
          return editor;
        }
    }
  /*
   * Each window has a global text field editor, if it doesn't exist create it
   * if create flag is set
   */
  if (!_fieldEditor && createFlag)
    {
      _fieldEditor = [NSText new];
      [_fieldEditor setFieldEditor: YES];
    }

  return _fieldEditor;
}

/*
 * Window controller
 */
- (void) setWindowController: (NSWindowController*)windowController
{
  /* The window controller owns us, we only keep a weak reference to
     it */
  _windowController = windowController;
}

- (id) windowController
{
  return _windowController;
}

/*
 * Window status and ordering
 */
- (void) becomeKeyWindow
{
  if (_f.is_key == NO)
    {
      _f.is_key = YES;

      if ((!_firstResponder) || (_firstResponder == self))
        {
	  if (!_initialFirstResponder)
	    {
	      [self recalculateKeyViewLoop];
	    }
          if (_initialFirstResponder)
            {
              [self makeFirstResponder: _initialFirstResponder];
            }
        }

      [_firstResponder becomeFirstResponder];
      if ((_firstResponder != self)
        && [_firstResponder respondsToSelector: @selector(becomeKeyWindow)])
        {
          [_firstResponder becomeKeyWindow];
        }

      [_wv setInputState: GSTitleBarKey];
      [GSServerForWindow(self) setinputfocus: _windowNum];
      [self resetCursorRects];
      [nc postNotificationName: NSWindowDidBecomeKeyNotification object: self];
      NSDebugLLog(@"NSWindow", @"%@ is now key window", [self title]);
    }
}

- (void) becomeMainWindow
{
  if (_f.is_main == NO)
    {
      _f.is_main = YES;
      if (_f.is_key == NO)
        {
          [_wv setInputState: GSTitleBarMain];
        }
      [nc postNotificationName: NSWindowDidBecomeMainNotification object: self];
      NSDebugLLog(@"NSWindow", @"%@ is now main window", [self title]);
    }
}

/** Returns YES if the receiver can be made key. If this method returns
    NO, the window will not be made key. This implementation returns YES
    if the window is resizable or has a title bar. You can override this
    method to change it's behavior */
- (BOOL) canBecomeKeyWindow
{
  if ((NSResizableWindowMask | NSTitledWindowMask) & _styleMask)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/** Returns YES if the receiver can be the main window. If this method
    returns NO, the window will not become the main window. This
    implementation returns YES if the window is resizable or has a
    title bar and is visible and is not an NSPanel. You can override
    this method to change it's behavior */
- (BOOL) canBecomeMainWindow
{
  if (!_f.visible)
    {
      return NO;
    }
  if ((NSResizableWindowMask | NSTitledWindowMask) & _styleMask)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) hidesOnDeactivate
{
  return _f.hides_on_deactivate;
}

- (void) setCanHide: (BOOL)flag
{
  _f.can_hide = flag;
}

- (BOOL) canHide
{
  return _f.can_hide;
}

- (BOOL) isKeyWindow
{
  return _f.is_key;
}

- (BOOL) isMainWindow
{
  return _f.is_main;
}

- (BOOL) isMiniaturized
{
  return _f.is_miniaturized;
}

- (BOOL) isVisible
{
  return _f.visible;
}

- (NSInteger) level
{
  return _windowLevel;
}

- (void) setIsVisible: (BOOL)flag
{
  _f.visible = flag;
  if (flag)
    {
      [self orderFrontRegardless];
    }
  else
    {
      [self orderOut: nil];
    }
}

- (void) makeKeyAndOrderFront: (id)sender
{
  [self deminiaturize: self];
  [self orderFrontRegardless];

  if ([self canBecomeKeyWindow] != NO)
    {
      [self makeKeyWindow];
      /*
       * OPENSTEP makes a window the main window when it makes it the key window.
       * So we do the same (though the documentation doesn't mention it).
       */
      [self makeMainWindow];
      /*
       * If a window is ordered in, make sure that the application isn't hidden,
       * and is active.
       */
      [NSApp unhide: self];
    }
}

- (void) makeKeyWindow
{
  if (!_f.visible || _f.is_miniaturized || _f.is_key == YES)
    {
      return;
    }
  if (![self canBecomeKeyWindow])
    {
      return;
    }
  [[NSApp keyWindow] resignKeyWindow];

  [self becomeKeyWindow];
}

- (void) makeMainWindow
{
  if (!_f.visible || _f.is_miniaturized || _f.is_main == YES)
    {
      return;
    }
  if (![self canBecomeMainWindow])
    {
      return;
    }
  [[NSApp mainWindow] resignMainWindow];
  [self becomeMainWindow];
}

/**
 * Orders the window to the back of its level. Equivalent to
 * -orderWindow:relativeTo: with arguments NSWindowBelow and 0.
 */
- (void) orderBack: (id)sender
{
  [self orderWindow: NSWindowBelow relativeTo: 0];
}

/**
 * If the application is active, orders the window to the front in its
 * level. If the application is not active, the window is ordered in as
 * far forward as possible in its level without being ordered in front
 * of the key or main window of the currently active app. The current key
 * and main window status is not changed. Equivalent to
 * -orderWindow:relativeTo: with arguments NSWindowAbove and 0.
 */
- (void) orderFront: (id)sender
{
  [self orderWindow: NSWindowAbove relativeTo: 0];
}

/**
  Orders the window to the front in its level (even in front of the
  key and main windows of the current app) regardless of whether the
  app is current or not. This method should only be used in rare cases
  where the app is cooperating with another app that is displaying
  data for it.  The current key and main window status is not changed.
*/
- (void) orderFrontRegardless
{
  [self orderWindow: NSWindowAbove relativeTo: -1];
}

/**
 * Orders the window out from the screen. Equivalent to
 * -orderWindow:relativeTo: with arguments NSWindowOut and 0.
 */
- (void) orderOut: (id)sender
{
  [self orderWindow: NSWindowOut relativeTo: 0];
}

/**
  <p>
  If place is NSWindowOut, removes the window from the screen. If
  place is NSWindowAbove, places the window directly above otherWin,
  or directly above all windows in its level if otherWin is 0.  If
  place is NSWindowBelow, places the window directly below otherWin,
  or directly below all windows in its level if otherWin is 0.
  </p>
  <p>If otherWin is zero and the key window is at the same window level
  as the receiver, the receiver cannot be positioned above the key window.
  </p>
  <p>
  If place is NSWindowAbove or NSWindowBelow and the application is
  hidden, the application is unhidden.
  </p>
*/
/*
  As a special undocumented case (for -orderFrontRegardless), if otherWin
  is minus one, then the backend should not try to keep the window below the
  current key/main window
*/
- (void) orderWindow: (NSWindowOrderingMode)place relativeTo: (NSInteger)otherWin
{
  GSDisplayServer *srv = GSServerForWindow(self);
  BOOL display = NO;

  if (YES == [[NSUserDefaults standardUserDefaults]
    boolForKey: @"GSBackgroundApp"])
    {
      return;
    }

  if (place == NSWindowOut)
    {
      if (_windowNum == 0)
        {
          return;        /* This deferred window was never ordered in. */
        }
      _f.visible = NO;
      /*
       * Don't keep trying to update the window while it is ordered out
       */
      [[self class] _removeAutodisplayedWindow: self];
      [self _lossOfKeyOrMainWindow];
    }
  else
    {
      /* Windows need to be constrained when displayed or resized - but only
         titled windows are constrained. Also, and this is the tricky part,
         don't constrain if we are merely unhiding the window or if it's
         already visible and is just being reordered. */
      if ((_styleMask & NSTitledWindowMask)
          && [NSApp isHidden] == NO
          && _f.visible == NO)
        {
          NSRect nframe = [self constrainFrameRect: _frame
                                toScreen: [self screen]];
          [self setFrame: nframe display: NO];
        }
      // create deferred window
      if (_windowNum == 0)
        {
          [self _initBackendWindow];
          display = YES;
        }
    }

  /* If a hide on deactivate window is explicitly ordered in or out while
     the application is not active, remove it from the list of inactive
     windows. */
  if ([self hidesOnDeactivate] && ![NSApp isActive])
    {
      [NSApp _setWindow: self inactive: NO];
    }

  // Draw content before backend window ordering
  if (display)
    {
      [_wv display];
    }
  else if (place != NSWindowOut)
    {
      [_wv displayIfNeeded];
    }

  /* The backend will keep us below the current key window unless we
     force it not too */
  if ((otherWin == 0
       || otherWin == [[NSApp keyWindow] windowNumber]
       || otherWin == [[NSApp mainWindow] windowNumber])
      && [NSApp isActive])
    {
      otherWin = -1;
    }

  [srv orderwindow: place : otherWin : _windowNum];
  if (display)
    {
      [self display];
    }

  if (place != NSWindowOut)
    {
      /*
       * Once we are ordered back in, we will want to update the window
       * whenever there is anything to do.
       */
      [[self class] _addAutodisplayedWindow: self];

      if (_f.has_closed == YES)
        {
          _f.has_closed = NO;        /* A closed window has re-opened        */
        }
      if (_f.has_opened == NO)
        {
          _f.has_opened = YES;
          if (_f.menu_exclude == NO)
            {
              [NSApp addWindowsItem: self
                     title: _windowTitle
                     filename: [self _hasTitleWithRepresentedFilename]];
            }
        }
      if ([self isKeyWindow] == YES)
        {
          [_wv setInputState: GSTitleBarKey];
          [srv setinputfocus: _windowNum];
        }
      _f.visible = YES;
    }
  else if ([self isOneShot])
    {
      [self _terminateBackendWindow];
    }
}

- (void) resignKeyWindow
{
  if (_f.is_key == YES)
    {
      if ((_firstResponder != self)
          && [_firstResponder respondsToSelector: @selector(resignKeyWindow)])
        {
          [_firstResponder resignKeyWindow];
        }

      _f.is_key = NO;

      if (_f.is_main == YES)
        {
          [_wv setInputState: GSTitleBarMain];
        }
      else
        {
          [_wv setInputState: GSTitleBarNormal];
        }
      [self discardCursorRects];

      [nc postNotificationName: NSWindowDidResignKeyNotification object: self];
    }
}

- (void) resignMainWindow
{
  if (_f.is_main == YES)
    {
      _f.is_main = NO;
      if (_f.is_key == YES)
        {
          [_wv setInputState: GSTitleBarKey];
        }
      else
        {
          [_wv setInputState: GSTitleBarNormal];
        }
      [nc postNotificationName: NSWindowDidResignMainNotification object: self];
    }
}

- (void) setHidesOnDeactivate: (BOOL)flag
{
  if (flag != _f.hides_on_deactivate)
    {
      _f.hides_on_deactivate = flag;
      if (![NSApp isActive])
	{
	  if (flag)
	    {
	      if (_f.visible)
		{
		  /* Order is important here. We must first order out the window
		     and then add it to the inactive list, since -orderOut:
		     removes the receiver from the inactive list. */
		  [self orderOut: nil];
		  [NSApp _setWindow: self inactive: YES];
		}
	    }
	  else
	    {
	      if ([NSApp _isWindowInactive: self])
		{
		  [NSApp _setWindow: self inactive: NO];
		  [self orderFront: nil];
		}
	    }
	}
    }
}

- (void) setLevel: (NSInteger)newLevel
{
  if (_windowLevel != newLevel)
    {
      _windowLevel = newLevel;
      if (_windowNum > 0)
        {
          GSDisplayServer *srv = GSServerForWindow(self);
          [srv setwindowlevel: _windowLevel : _windowNum];
        }
    }
}

- (NSPoint) cascadeTopLeftFromPoint: (NSPoint)topLeftPoint
{
  NSRect cRect;

  if (NSEqualPoints(topLeftPoint, NSZeroPoint) == YES)
    {
      topLeftPoint.x = NSMinX(_frame);
      topLeftPoint.y = NSMaxY(_frame);
    }

  [self setFrameTopLeftPoint: topLeftPoint];
  cRect = [self contentRectForFrameRect: _frame];
  topLeftPoint.x = NSMinX(cRect);
  topLeftPoint.y = NSMaxY(cRect);

  /* make sure the new point is inside the screen */
  if ([self screen])
    {
      NSRect screenRect;

      screenRect = [[self screen] visibleFrame];
      if (topLeftPoint.x >= NSMaxX(screenRect))
        {
          topLeftPoint.x = NSMinX(screenRect);
        }
      if (topLeftPoint.y <= NSMinY(screenRect))
        {
          topLeftPoint.y = NSMaxY(screenRect);
        }
    }

  return topLeftPoint;
}

- (BOOL) showsResizeIndicator
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
        "showsResizeIndicator", "NSWindow");
  return YES;
}

- (void) setShowsResizeIndicator: (BOOL)show
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
        "setShowsResizeIndicator:", "NSWindow");
}

- (BOOL) preservesContentDuringLiveResize
{
  return _f.preserves_content_during_live_resize;
}

- (void) setPreservesContentDuringLiveResize: (BOOL)flag
{
  _f.preserves_content_during_live_resize = flag;
}

- (void) setFrame: (NSRect)frameRect
          display: (BOOL)displayFlag
          animate: (BOOL)animationFlag
{
  if (animationFlag && !NSEqualRects(_frame, frameRect))
    {
      // time that the resize is expected to take in seconds
      NSTimeInterval resizeTime;
      NSArray *animations;
      NSViewAnimation *viewAnimation;

      resizeTime = [self animationResizeTime: frameRect];
      animations = [NSArray arrayWithObject:
                              [NSDictionary dictionaryWithObjectsAndKeys:
                                              self, NSViewAnimationTargetKey,
                                                  [NSValue valueWithRect: frameRect], NSViewAnimationEndFrameKey,
                                            nil]];
      viewAnimation = [[NSViewAnimation alloc] initWithViewAnimations: animations];
      [viewAnimation setAnimationBlockingMode: NSAnimationNonblocking];
      [viewAnimation setDuration: resizeTime];
      if (animationDelegate == nil)
        {
          animationDelegate = [[GSWindowAnimationDelegate alloc] init];
        }
      // The delegate handles the release of the viewAnimation
      [viewAnimation setDelegate: animationDelegate];
      [viewAnimation startAnimation];
      //AUTORELEASE(viewAnimation);
    }
  else
    {
      [self setFrame: frameRect display: displayFlag];
    }
}

- (NSTimeInterval) animationResizeTime: (NSRect)newFrame
{
  static float resizeTime = 0;
  float maxDiff;

  if (resizeTime == 0)
    {
      NSNumber *num;
      num = [[NSUserDefaults standardUserDefaults]
        objectForKey: @"NSWindowResizeTime"];
      if (num != nil)
        {
          resizeTime = [num floatValue];
        }
      else
        {
          resizeTime = 0.20;
        }
    }

  // Find the biggest difference
  maxDiff = fabs(newFrame.origin.x - _frame.origin.x);
  maxDiff = MAX(maxDiff, fabs(newFrame.origin.y - _frame.origin.y));
  maxDiff = MAX(maxDiff, fabs(newFrame.size.width - _frame.size.width));
  maxDiff = MAX(maxDiff, fabs(newFrame.size.height - _frame.size.height));

  return (maxDiff * resizeTime) / 150;
}

- (void) center
{
  NSRect  screenFrame = [[NSScreen mainScreen] visibleFrame];
  NSSize  screenSize = screenFrame.size;
  NSPoint origin = screenFrame.origin;

  origin.x += (screenSize.width - _frame.size.width) / 2;
  origin.y += (screenSize.height - _frame.size.height) / 2;

  [self setFrameOrigin: origin];
}

/**
 * Given a proposed frame rectangle, return a modified version
 * which will fit inside the screen.
 */
- (NSRect) constrainFrameRect: (NSRect)frameRect toScreen: (NSScreen*)screen
{
  NSRect screenRect;
  CGFloat difference;

  if (nil == screen)
    {
      return frameRect;
    }

  screenRect = [screen visibleFrame];

  if (NSHeight(frameRect) < NSHeight(screenRect))
    {
      /* Move top edge of the window inside the screen */
      difference = NSMaxY (frameRect) - NSMaxY (screenRect);
      if (difference > 0)
        {
          frameRect.origin.y -= difference;
        }
      else
        {
          /* Move bottom edge of the window inside the screen */
          difference = screenRect.origin.y - frameRect.origin.y;
          if (difference > 0)
            {
              frameRect.origin.y += difference;
            }
        }
    }
  else
    {
      /* If the window is resizable, resize it so that
         it fits on the screen. */
      if (_styleMask & NSResizableWindowMask)
        {
          /* Ensure that resizing doesn't make window smaller than minimum */
          if (_minimumSize.height < NSHeight(screenRect))
            {
              frameRect.origin.y = NSMinY(screenRect);
              frameRect.size.height = NSHeight(screenRect);
            }
          else
            {
              frameRect.origin.y = NSMaxY(screenRect) - _minimumSize.height;
              frameRect.size.height = _minimumSize.height;
            }
        }
      else
        {
          /* Move top edge of the window to the screen limit */
          frameRect.origin.y -= NSMaxY(frameRect) - NSMaxY(screenRect);
        }
    }

  if (NSWidth(frameRect) < NSWidth(screenRect))
    {
      /* Move right edge of the window inside the screen */
      difference = NSMaxX (frameRect) - NSMaxX (screenRect);
      if (difference > 0)
        {
          frameRect.origin.x -= difference;
        }
      else
        {
          /* Move left edge of the window inside the screen */
          difference = screenRect.origin.x - frameRect.origin.x;
          if (difference > 0)
            {
              frameRect.origin.x += difference;
            }
        }
    }
  else
    {
      /* If the window is resizable, resize it so that
         it fits on the screen. */
      if (_styleMask & NSResizableWindowMask)
        {
          /* Ensure that resizing doesn't make window smaller than minimum */
          if (_minimumSize.width < NSWidth(screenRect))
            {
              frameRect.origin.x = NSMinX(screenRect);
              frameRect.size.width = NSWidth(screenRect);
            }
          else
            {
              frameRect.origin.x = NSMaxX(screenRect) - _minimumSize.width;
              frameRect.size.width = _minimumSize.width;
            }
        }
      else
        {
          /* Move right edge of the window to the screen limit */
          frameRect.origin.x -= NSMaxX(frameRect) - NSMaxX(screenRect);
        }
    }

  return frameRect;
}

- (NSRect) frame
{
  return _frame;
}

- (NSSize) minSize
{
  return _minimumSize;
}

- (NSSize) maxSize
{
  return _maximumSize;
}

- (void) setContentSize: (NSSize)aSize
{
  NSRect r = _frame;

  r.size = aSize;
  r = [self frameRectForContentRect: r];
  r.origin = _frame.origin;
  [self setFrame: r display: YES];
}

- (void) _applyFrame: (NSRect )frameRect
{
  if (_windowNum)
    {
      [GSServerForWindow(self) placewindow: frameRect : _windowNum];
    }
  else
    {
      _frame = frameRect;
      frameRect.origin = NSZeroPoint;
      [_wv setFrame: frameRect];
    }
}

- (void) setFrame: (NSRect)frameRect display: (BOOL)flag
{
  if (_maximumSize.width > 0 && frameRect.size.width > _maximumSize.width)
    {
      frameRect.size.width = _maximumSize.width;
    }
  if (_maximumSize.height > 0 && frameRect.size.height > _maximumSize.height)
    {
      frameRect.size.height = _maximumSize.height;
    }
  if (frameRect.size.width < _minimumSize.width)
    {
      frameRect.size.width = _minimumSize.width;
    }
  if (frameRect.size.height < _minimumSize.height)
    {
      frameRect.size.height = _minimumSize.height;
    }

  /* Windows need to be constrained when displayed or resized - but only
     titled windows are constrained */
  if (_styleMask & NSTitledWindowMask)
    {
      frameRect = [self constrainFrameRect: frameRect
                                  toScreen: [self _screenForFrame: frameRect]];
    }

  // If nothing changes, don't send it to the backend and don't redisplay
  if (NSEqualRects(_frame, frameRect))
    {
      return;
    }

  if (NSEqualPoints(_frame.origin, frameRect.origin) == NO)
    {
      [nc postNotificationName: NSWindowWillMoveNotification object: self];
    }

  /*
   * Now we can tell the graphics context to do the actual resizing.
   * We will recieve an event to tell us when the resize is done.
   */
  [self _applyFrame: frameRect];

  if (flag)
    {
      [self display];
    }
}

- (void) setFrameOrigin: (NSPoint)aPoint
{
  NSRect r = _frame;

  r.origin = aPoint;
  [self setFrame: r display: NO];
}

- (void) setFrameTopLeftPoint: (NSPoint)aPoint
{
  NSRect r = _frame;

  r.origin = aPoint;
  r.origin.y -= _frame.size.height;
  [self setFrame: r display: NO];
}

- (void) setMinSize: (NSSize)aSize
{
  if (aSize.width < 1)
    {
      aSize.width = 1;
    }
  if (aSize.height < 1)
    {
      aSize.height = 1;
    }
  _minimumSize = aSize;
  if (_windowNum > 0)
    {
      [GSServerForWindow(self) setminsize: aSize : _windowNum];
    }
}

- (void) setMaxSize: (NSSize)aSize
{
  /*
   * Documented maximum size for macOS-X - do we need this restriction?
   */
  if (aSize.width > 10000)
    {
      aSize.width = 10000;
    }
  if (aSize.height > 10000)
    {
      aSize.height = 10000;
    }
  _maximumSize = aSize;
  if (_windowNum > 0)
    {
      [GSServerForWindow(self) setmaxsize: aSize : _windowNum];
    }
}

- (NSSize) resizeIncrements
{
  return _increments;
}

- (void) setResizeIncrements: (NSSize)aSize
{
  _increments = aSize;
  if (_windowNum > 0)
    {
      [GSServerForWindow(self) setresizeincrements: aSize : _windowNum];
    }
}

- (NSSize) aspectRatio
{
  // FIXME: This method is missing
  return NSMakeSize(1, 1);
}

- (void) setAspectRatio: (NSSize)ratio
{
  // FIXME: This method is missing
}

- (NSSize) contentMaxSize
{
  // FIXME
  NSRect rect;

  rect.origin = NSMakePoint(0, 0);
  rect.size = [self maxSize];
  rect = [self contentRectForFrameRect: rect];
  return rect.size;
}

- (void) setContentMaxSize: (NSSize)size
{
  // FIXME
  NSRect rect;

  rect.origin = NSMakePoint(0, 0);
  rect.size = size;
  rect = [self frameRectForContentRect: rect];
  [self setMaxSize: rect.size];
}

- (NSSize) contentMinSize
{
  // FIXME
  NSRect rect;

  rect.origin = NSMakePoint(0, 0);
  rect.size = [self minSize];
  rect = [self contentRectForFrameRect: rect];
  return rect.size;
}

- (void) setContentMinSize: (NSSize)size
{
  // FIXME
  NSRect rect;

  rect.origin = NSMakePoint(0, 0);
  rect.size = size;
  rect = [self frameRectForContentRect: rect];
  [self setMinSize: rect.size];
}

- (NSSize) contentAspectRatio
{
  // FIXME
  return NSMakeSize(1, 1);
}

- (void) setContentAspectRatio: (NSSize)ratio
{
  // FIXME
}

- (NSSize) contentResizeIncrements
{
  // FIXME
  return [self resizeIncrements];
}

- (void) setContentResizeIncrements: (NSSize)increments
{
  // FIXME
  [self setResizeIncrements: increments];
}

/**
 * Convert from a point in the base coordinate system for the window
 * to a point in the screen coordinate system.
 */
- (NSPoint) convertBaseToScreen: (NSPoint)aPoint
{
  NSPoint screenPoint;

  screenPoint.x = _frame.origin.x + aPoint.x;
  screenPoint.y = _frame.origin.y + aPoint.y;
  return screenPoint;
}

/**
 * Convert from a point in the screen coordinate system to a point in the
 * screen coordinate system of the receiver.
 */
- (NSPoint) convertScreenToBase: (NSPoint)aPoint
{
  NSPoint basePoint;

  basePoint.x = aPoint.x - _frame.origin.x;
  basePoint.y = aPoint.y - _frame.origin.y;
  return basePoint;
}

/**
 * Converts aRect from the coordinate system of the screen
 * to the coordinate system of the window.
 */
- (NSRect) convertRectFromScreen: (NSRect)aRect
{
  NSRect result = aRect;
  NSPoint origin = result.origin;
  NSPoint newOrigin = [self convertScreenToBase: origin];
  result.origin = newOrigin;
  return result;
}

/**
 * Converts aRect from the window coordinate system to a rect in
 * the screen coordinate system.
 */
- (NSRect) convertRectToScreen: (NSRect)aRect
{
  NSRect result = aRect;
  NSPoint origin = result.origin;
  NSPoint newOrigin = [self convertBaseToScreen: origin];
  result.origin = newOrigin;
  return result;
}

/*
 * Managing the display
 */
- (void) disableFlushWindow
{
  _disableFlushWindow++;
}

- (void) display
{
  if (_gstate == 0 || _f.visible == NO)
    {
      return;
    }

  [_wv display];
  [self discardCachedImage];
  _f.views_need_display = NO;
}

- (void) displayIfNeeded
{
  if (_gstate == 0 || _f.visible == NO)
    {
      return;
    }

  if (_f.views_need_display)
    {
      [_wv displayIfNeeded];
      [self discardCachedImage];
      _f.views_need_display = NO;
    }
}

- (void) update
{
  [nc postNotificationName: NSWindowDidUpdateNotification object: self];
}

- (void) flushWindowIfNeeded
{
  if (_disableFlushWindow == 0 && _f.needs_flush == YES)
    {
      [self flushWindow];
    }
}

/**
 * Flush all drawing in the windows buffer to the screen unless the window
 * is not buffered or flushing is not enabled.
 */
- (void) flushWindow
{
  NSUInteger i;

  /*
   * If flushWindow is called while flush is disabled
   * mark self as needing a flush, then return
   */
  if (_disableFlushWindow)
    {
      _f.needs_flush = YES;
      return;
    }

  /*
   * Just flush graphics if backing is not buffered.
   * The documentation actually says that this is wrong ... the method
   * should do nothing when the backingType is NSBackingStoreNonretained
   */
  if (_backingType == NSBackingStoreNonretained)
    {
      [_context flushGraphics];
      return;
    }

  /* Check for special case of flushing while we are lock focused.
     For instance, when we are highlighting a button. */
  if (NSIsEmptyRect(_rectNeedingFlush))
    {
      if ([_rectsBeingDrawn count] == 0)
        {
          _f.needs_flush = NO;
          return;
        }
    }

  /*
   * Accumulate the rectangles from all nested focus locks.
   */
  i = [_rectsBeingDrawn count];
  while (i-- > 0)
    {
      _rectNeedingFlush = NSUnionRect(_rectNeedingFlush,
        [[_rectsBeingDrawn objectAtIndex: i] rectValue]);
    }

  if (_windowNum > 0)
    {
      [GSServerForWindow(self) flushwindowrect: _rectNeedingFlush
                                              : _windowNum];
    }
  _f.needs_flush = NO;
  _rectNeedingFlush = NSZeroRect;
}

- (void) enableFlushWindow
{
  if (_disableFlushWindow > 0)
    {
      _disableFlushWindow--;
    }
}

- (BOOL) isAutodisplay
{
  return _f.is_autodisplay;
}

- (BOOL) isFlushWindowDisabled
{
  return _disableFlushWindow == 0 ? NO : YES;
}

- (void) setAutodisplay: (BOOL)flag
{
  _f.is_autodisplay = flag;
}

- (void) setViewsNeedDisplay: (BOOL)flag
{
  if (_f.views_need_display != flag)
    {
      _f.views_need_display = flag;
      if (flag)
        {
          /* TODO: this call most likely shouldn't be here */
          [NSApp setWindowsNeedUpdate: YES];
        }
    }
}

- (BOOL) viewsNeedDisplay
{
  return _f.views_need_display;
}

- (void) cacheImageInRect: (NSRect)aRect
{
  NSView *cacheView;
  NSRect cacheRect;

  aRect = NSIntegralRect (NSIntersectionRect (aRect, [_wv frame]));
  _cachedImageOrigin = aRect.origin;
  DESTROY(_cachedImage);

  if (NSIsEmptyRect (aRect))
    {
      return;
    }

  cacheRect.origin = NSZeroPoint;
  cacheRect.size = aRect.size;
  _cachedImage = [[NSCachedImageRep alloc] initWithWindow: nil
                                           rect: cacheRect];
  cacheView = [[_cachedImage window] contentView];
  [cacheView lockFocus];
  NSCopyBits (_gstate, aRect, NSZeroPoint);
  [cacheView unlockFocus];
}

- (void) discardCachedImage
{
  DESTROY(_cachedImage);
}

- (void) restoreCachedImage
{
  if (_cachedImage == nil)
    {
      return;
    }
  [_wv lockFocus];
  NSCopyBits([[_cachedImage window] gState],
             [_cachedImage rect],
             _cachedImageOrigin);
  [_wv unlockFocus];
}

- (void) useOptimizedDrawing: (BOOL)flag
{
  _f.optimize_drawing = flag;
}

- (BOOL) canStoreColor
{
  if (NSNumberOfColorComponents(NSColorSpaceFromDepth(_depthLimit)) > 1)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/** Returns the screen the window is on. Unlike (apparently) OpenStep
    and MacOSX, GNUstep does not support windows being split across
    multiple screens */
- (NSScreen *) deepestScreen
{
  return [self screen];
}

- (NSWindowDepth) depthLimit
{
  return _depthLimit;
}

- (BOOL) hasDynamicDepthLimit
{
  return _f.dynamic_depth_limit;
}

/** Returns the screen the window is on. */
- (NSScreen *) screen
{
  // Only recompute the screen if the current screen
  // doesn't contain the whole window.
  // FIXME: Containing half the window would be enough
  if (_screen != nil)
    {
      NSRect sframe = [_screen frame];
      if (NSContainsRect(sframe, _frame))
        {
          return _screen;
        }
    }
  ASSIGN(_screen, [self _screenForFrame: _frame]);
  return _screen;
}

- (void) applicationDidChangeScreenParameters: (NSNotification *)aNotif
{
  NSRect       oldScreenFrame = [_screen frame];
  int          screenNumber = [_screen screenNumber];
  NSRect       newScreenFrame;
  NSRect       newFrame;
  NSEnumerator *e;
  NSScreen     *scr;

  // We need to get new screen from renewed screen list because
  // [NSScreen mainScreen] returns NSScreen object of key window and that object
  // will never be released.
  e = [[NSScreen screens] objectEnumerator];
  while ((scr = [e nextObject]))
    {
      if ([scr screenNumber] == screenNumber)
        {
          ASSIGN(_screen, scr);
          break;
        }
    }

  // Do not adjust frame for mini and appicon windows - it's a WM's job.
  if ([self isKindOfClass: [NSMiniWindow class]] || self == [NSApp iconWindow])
    {
      return;
    }

  newScreenFrame = [_screen frame];
  newFrame = _frame;
  // Screen Y origin change.
  newFrame.origin.y += newScreenFrame.origin.y - oldScreenFrame.origin.y;
  // Screen height change.
  newFrame.origin.y += newScreenFrame.size.height - oldScreenFrame.size.height;
  // Screen X origin change. Screen width change shouldn't affect our frame.
  newFrame.origin.x += newScreenFrame.origin.x - oldScreenFrame.origin.x;

  /* Call backend's `placewindow::` directly because our origin in OpenStep
     coordinates might be unchanged and `setFrame:display:` has check
     for it. */
  [self _applyFrame: newFrame];
  [self display];

  if (_autosaveName != nil)
    {
      [self saveFrameUsingName: _autosaveName];
    }
}

- (void) setDepthLimit: (NSWindowDepth)limit
{
  if (limit == 0)
    {
      limit = [[self class] defaultDepthLimit];
    }

  _depthLimit = limit;
}

- (void) setDynamicDepthLimit: (BOOL)flag
{
  _f.dynamic_depth_limit = flag;
}

- (NSWindowCollectionBehavior) collectionBehavior
{
  //TODO: we don't handle collections yet and perhaps never will fully
  return 0;
}

- (void) setCollectionBehavior: (NSWindowCollectionBehavior)props
{
  //TODO we don't handle collections yet. Perhaps certain features can be mapped on existing ones
  //other features are Expose specific or anyway probably not implementable
}

/*
 * Cursor management
 */
- (BOOL) areCursorRectsEnabled
{
  return _f.cursor_rects_enabled;
}

- (void) disableCursorRects
{
  _f.cursor_rects_enabled = NO;
}

static void
discardCursorRectsForView(NSView *theView)
{
  if (theView != nil)
    {
      if (theView->_rFlags.has_currects)
        {
          [theView discardCursorRects];
        }

      if (theView->_rFlags.has_subviews)
        {
          NSArray *s = theView->_sub_views;
          NSUInteger count = [s count];

          if (count)
            {
              NSView *subs[count];
              NSUInteger i;

              [s getObjects: subs];
              for (i = 0; i < count; i++)
                {
                  discardCursorRectsForView(subs[i]);
                }
            }
        }
    }
}

- (void) discardCursorRects
{
  discardCursorRectsForView(_wv);
}

- (void) enableCursorRects
{
  _f.cursor_rects_enabled = YES;
}

- (void) invalidateCursorRectsForView: (NSView*)aView
{
  if (aView->_rFlags.valid_rects)
    {
      [aView discardCursorRects];

      if (_f.cursor_rects_valid)
        {
          if (_f.is_key && _f.cursor_rects_enabled)
            {
              NSEvent *e = [NSEvent otherEventWithType: NSAppKitDefined
                                              location: NSMakePoint(-1, -1)
                                         modifierFlags: 0
                                             timestamp: 0
                                          windowNumber: _windowNum
                                               context: GSCurrentContext()
                                               subtype: -1
                                                 data1: 0
                                                 data2: 0];
              [self postEvent: e atStart: YES];
            }
          _f.cursor_rects_valid = NO;
        }
    }
}

static void
resetCursorRectsForView(NSView *theView)
{
  if (theView != nil)
    {
      [theView resetCursorRects];

      if (theView->_rFlags.has_subviews)
        {
          NSArray *s = theView->_sub_views;
          NSUInteger count = [s count];

          if (count)
            {
              NSView *subs[count];
              NSUInteger i;

              [s getObjects: subs];
              for (i = 0; i < count; i++)
                {
                  resetCursorRectsForView(subs[i]);
                }
            }
        }
    }
}

static void
checkCursorRectanglesEntered(NSView *theView,  NSEvent *theEvent, NSPoint lastPoint)
{

  /*
   * Check cursor rectangles for the subviews
   */
  if (theView->_rFlags.has_subviews)
    {
      NSArray *sb = theView->_sub_views;
      NSUInteger count = [sb count];

      if (count > 0)
        {
          NSView *subs[count];
          NSUInteger i;

          [sb getObjects: subs];
          for (i = 0; i < count; ++i)
            {
              if (![subs[i] isHidden])
                {
                  checkCursorRectanglesEntered(subs[i], theEvent, lastPoint);
                }
            }
        }
    }

  if (theView->_rFlags.valid_rects)
    {
      NSArray *tr = theView->_cursor_rects;
      NSUInteger count = [tr count];

      // Loop through cursor rectangles
      if (count > 0)
        {
          GSTrackingRect *rects[count];
          NSPoint loc = [theEvent locationInWindow];
          NSUInteger i;

          [tr getObjects: rects];

          for (i = 0; i < count; ++i)
            {
              GSTrackingRect *r = rects[i];
              BOOL last;
              BOOL now;

              if ([r isValid] == NO)
                {
                  continue;
                }

              /*
               * Check for presence of point in rectangle.
               */
              last = NSMouseInRect(lastPoint, r->rectangle, NO);
              now = NSMouseInRect(loc, r->rectangle, NO);

              // Mouse entered
              if ((!last) && (now))
                {
                  NSEvent *e;

                  e = [NSEvent enterExitEventWithType: NSCursorUpdate
                    location: loc
                    modifierFlags: [theEvent modifierFlags]
                    timestamp: 0
                    windowNumber: [theEvent windowNumber]
                    context: [theEvent context]
                    eventNumber: 0
                    trackingNumber: (int)YES
                    userData: (void*)r];
                  [NSApp postEvent: e atStart: YES];
                  //NSLog(@"Add enter event %@ for view %@ rect %@", e, theView, NSStringFromRect(r->rectangle));
                }
            }
        }
    }
}

static void
checkCursorRectanglesExited(NSView *theView,  NSEvent *theEvent, NSPoint lastPoint)
{
  if (theView->_rFlags.valid_rects)
    {
      NSArray *tr = theView->_cursor_rects;
      NSUInteger count = [tr count];

      // Loop through cursor rectangles
      if (count > 0)
        {
          GSTrackingRect *rects[count];
          NSPoint loc = [theEvent locationInWindow];
          NSUInteger i;

          [tr getObjects: rects];

          for (i = 0; i < count; ++i)
            {
              GSTrackingRect *r = rects[i];
              BOOL last;
              BOOL now;

              if ([r isValid] == NO)
                {
                  continue;
                }

              /*
               * Check for presence of point in rectangle.
               */
              last = NSMouseInRect(lastPoint, r->rectangle, NO);
              now = NSMouseInRect(loc, r->rectangle, NO);

              // Mouse exited
              if ((last) && (!now))
                {
                  NSEvent *e;

                  e = [NSEvent enterExitEventWithType: NSCursorUpdate
                    location: loc
                    modifierFlags: [theEvent modifierFlags]
                    timestamp: 0
                    windowNumber: [theEvent windowNumber]
                    context: [theEvent context]
                    eventNumber: 0
                    trackingNumber: (int)NO
                    userData: (void*)r];
                  [NSApp postEvent: e atStart: YES];
                  //[NSApp postEvent: e atStart: NO];
                  //NSLog(@"Add exit event %@ for view %@ rect %@", e, theView, NSStringFromRect(r->rectangle));
                }
            }
        }
    }

  /*
   * Check cursor rectangles for the subviews
   */
  if (theView->_rFlags.has_subviews)
    {
      NSArray *sb = theView->_sub_views;
      NSUInteger count = [sb count];

      if (count > 0)
        {
          NSView *subs[count];
          NSUInteger i;

          [sb getObjects: subs];
          for (i = 0; i < count; ++i)
            {
              if (![subs[i] isHidden])
                {
                  checkCursorRectanglesExited(subs[i], theEvent, lastPoint);
                }
            }
        }
    }
}

- (void) resetCursorRects
{
  [self discardCursorRects];
  resetCursorRectsForView(_wv);
  _f.cursor_rects_valid = YES;

  if (_f.is_key && _f.cursor_rects_enabled)
    {
      NSPoint loc = [self mouseLocationOutsideOfEventStream];
      if (NSMouseInRect(loc, [_wv bounds], NO))
        {
          NSEvent *e = [NSEvent mouseEventWithType: NSMouseMoved
                                          location: loc
                                     modifierFlags: 0
                                         timestamp: 0
                                      windowNumber: _windowNum
                                           context: GSCurrentContext()
                                       eventNumber: 0
                                        clickCount: 0
                                          pressure: 0];
          _lastPoint = NSMakePoint(-1,-1);
          checkCursorRectanglesEntered(_wv, e, _lastPoint);
          _lastPoint = loc;
        }
    }
}

/*
 * Handling user actions and events
 */
- (void) close
{
  if (_f.has_closed == NO)
    {
      CREATE_AUTORELEASE_POOL(pool);
      _f.has_closed = YES;

      /* The NSWindowCloseNotification might result in us being
         deallocated. To make sure self stays valid as long as is
         necessary, we retain ourselves here and balance it with a
         release later (unless we're supposed to release ourselves when
         we close).
      */
      if (!_f.is_released_when_closed)
        {
          RETAIN(self);
        }

      [nc postNotificationName: NSWindowWillCloseNotification object: self];
      _f.has_opened = NO;
      [NSApp removeWindowsItem: self];
      [self orderOut: self];

      if (_f.is_miniaturized == YES)
	{
	  NSWindow *mini = GSWindowWithNumber(_counterpart);
	  GSRemoveIcon(mini);
	}

      [pool drain];
      RELEASE(self);
    }
}

/* Private Method. Many X Window managers will just deminiaturize us without
   telling us to do it ourselves. Deal with it.
*/
- (void) _didDeminiaturize: sender
{
  if (_f.is_miniaturized == YES)
    {
      _f.is_miniaturized = NO;
      _f.visible = YES;
      if (self == [NSApp iconWindow])
	{
	  [self orderOut: self];
	  if ([NSApp isActive] == NO)
	    {
	      [NSApp activateIgnoringOtherApps: YES];
	    }
	  if ([NSApp isHidden] == YES)
	    {
	      [NSApp unhide: self];
	    }
	}
      [nc postNotificationName: NSWindowDidDeminiaturizeNotification
			object: self];
    }
}

/**
  Causes the window to deminiaturize. Normally you would not call this
  method directly. A window is automatically deminiaturized by the
  user via a mouse click event. Does nothing it the window isn't
  miniaturized.  */
- (void) deminiaturize: sender
{
  if (!_f.is_miniaturized)
    {
      return;
    }

  /* At least with X-Windows, the counterpart is tied to us, so it will
     automatically be ordered out when we are deminiaturized */
  if (_counterpart != 0)
    {
      NSWindow *mini = GSWindowWithNumber(_counterpart);

      GSRemoveIcon(mini);
      [mini orderOut: self];
    }

  _f.is_miniaturized = NO;
  [self makeKeyAndOrderFront: self];
  [self _didDeminiaturize: sender];
}

/**
   Returns YES, if the document has been changed.
*/
- (BOOL) isDocumentEdited
{
  return _f.is_edited;
}


/**
   Returns YES, if the window is released when it is closed.
*/
- (BOOL) isReleasedWhenClosed
{
  return _f.is_released_when_closed;
}

/**
  Causes the window to miniaturize, that is the window is removed from
  the screen and it's counterpart (mini)window is displayed. Does
  nothing if the window can't be miniaturized (eg. because it's already
  miniaturized).  */
- (void) miniaturize: (id)sender
{
  GSDisplayServer *srv = GSServerForWindow(self);
  NSSize iconSize = [GSCurrentServer() iconSize];

  if (_f.is_miniaturized || (_styleMask & NSMiniWindowMask))
    {
      /* Can't miniaturize a miniwindow or a miniaturized window.
       */
      return;
    }

  if (self == [NSApp iconWindow])
    {
      if (NO == [[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSSuppressAppIcon"])
	{
	  return;
	}
    }
  else if ((!(_styleMask & (NSIconWindowMask | NSMiniaturizableWindowMask)))
    || (_styleMask & NSMiniWindowMask)
    || (![self isVisible]))
    {
      return;
    }

  [nc postNotificationName: NSWindowWillMiniaturizeNotification
                    object: self];

  _f.is_miniaturized = YES;
  /* Make sure we're not defered */
  if (_windowNum == 0)
    {
      [self _initBackendWindow];
    }
  /*
   * Ensure that we have a miniwindow counterpart.
   */
  if (_counterpart == 0 && [srv appOwnsMiniwindow])
    {
      NSWindow *mini;
      NSMiniWindowView *v;
      NSRect rect = NSMakeRect(0, 0, iconSize.height, iconSize.width);

      mini = [[NSMiniWindow alloc] initWithContentRect: rect
                                             styleMask: NSMiniWindowMask
                                               backing: NSBackingStoreBuffered
                                                 defer: NO];
      mini->_counterpart = [self windowNumber];
      _counterpart = [mini windowNumber];
      v = [[NSMiniWindowView alloc] initWithFrame: rect];
      [v setImage: [self miniwindowImage]];
      [v setTitle: [self miniwindowTitle]];
      [mini setContentView: v];
      RELEASE(v);
    }
  [self _lossOfKeyOrMainWindow];
  [srv miniwindow: _windowNum];
  _f.visible = NO;

  /*
   * We must order the miniwindow in so that we will start sending
   * it messages to tell it to display itsself when neccessary.
   */
  if (_counterpart != 0)
    {
      NSRect iconRect;
      NSWindow *mini = GSWindowWithNumber(_counterpart);
      iconRect = GSGetIconFrame(mini);
      [mini setFrame: iconRect display: YES];
      [mini orderFront: self];
    }
  [nc postNotificationName: NSWindowDidMiniaturizeNotification
                    object: self];
}

/**
   Causes the window to close.  Calls the windowShouldClose: method
   on the delegate to determine if it should close and calls
   shouldCloseWindowController on the controller for the receiver.
*/
- (void) performClose: (id)sender
{
  /* Don't close if a modal session is running and we are not the
     modal window */
  if ([NSApp modalWindow] && self != [NSApp modalWindow])
    {
      /* Panel that work in modal session can be closed nevertheless */
      if (![self worksWhenModal])
	return;
    }

  /* self must have a close button in order to be closed */
  if (!(_styleMask & NSClosableWindowMask))
    {
      NSBeep();
      return;
    }

  if (_windowController)
    {
      NSDocument *document = [_windowController document];

      if (document && ![document shouldCloseWindowController: _windowController])
        {
          NSBeep();
          return;
        }
    }
  if ([_delegate respondsToSelector: @selector(windowShouldClose:)])
    {
      /*
       * if delegate responds to windowShouldClose query it to see if
       * it's ok to close the window
       */
      if (![_delegate windowShouldClose: self])
        {
          NSBeep();
          return;
        }
    }
  else
    {
      /*
       * else if self responds to windowShouldClose query
       * self to see if it's ok to close self
       */
      if ([self respondsToSelector: @selector(windowShouldClose:)])
        {
          if (![self windowShouldClose: self])
            {
              NSBeep();
              return;
            }
        }
    }

  // FIXME: The button should be highlighted
  [self close];
}

/**
   Performs the key equivalent represented by theEvent.
 */
- (BOOL) performKeyEquivalent: (NSEvent*)theEvent
{
  if (_contentView)
    {
      return [_contentView performKeyEquivalent: theEvent];
    }
  return NO;
}

/**
 * Miniaturize the receiver ... as long as its style mask includes
 * NSMiniaturizableWindowMask (and as long as the receiver is not an
 * icon or mini window itsself). Calls -miniaturize: to do this.<br />
 * Beeps if the window can't be miniaturised.<br />
 * Should ideally provide visual feedback (highlighting the miniaturize
 * button as if it had been clicked) first ... but that's not yet implemented.
 */
- (void) performMiniaturize: (id)sender
{
  if ((!(_styleMask & NSMiniaturizableWindowMask))
    || (_styleMask & (NSIconWindowMask | NSMiniWindowMask)))
    {
      NSBeep();
      return;
    }

  // FIXME: The button should be highlighted
  [self miniaturize: sender];
}

+ (NSButton *) standardWindowButton: (NSWindowButton)button
                       forStyleMask: (NSUInteger) mask
{
  return [[GSTheme theme] standardWindowButton: button
				  forStyleMask: mask];
}

- (NSButton *) standardWindowButton: (NSWindowButton)button
{
  return [_wv viewWithTag: button];
}

- (BOOL) showsToolbarButton
{
  return _f.shows_toolbar_button;
}

- (void) setShowsToolbarButton: (BOOL)flag
{
  _f.shows_toolbar_button = flag;
}

- (NSInteger) resizeFlags
{
  // FIXME: The implementation is missing
  return 0;
}

/**
   Set document edit status.   If YES, then, if the receiver has a close
   button, the close button will show a broken X.  If NO, then, if the reciever
   has a close button, the close button will show a solid X.
 */
- (void) setDocumentEdited: (BOOL)flag
{
  if (_f.is_edited != flag)
    {
      _f.is_edited = flag;
      if (_f.menu_exclude == NO && _f.has_opened == YES)
        {
          [NSApp updateWindowsItem: self];
        }
      [_wv setDocumentEdited: flag];
    }
}

/**
   Get an undo manager from the delegate or create one.
 */
- (NSUndoManager*) undoManager
{
  NSUndoManager *undo = nil;

  if ([_delegate respondsToSelector: @selector(windowWillReturnUndoManager:)])
    {
      undo = [_delegate windowWillReturnUndoManager: self];
    }
  else if (_windowController)
    {
      NSDocument *document = [_windowController document];

      if (document)
        {
          undo = [document undoManager];
        }
    }

  if (undo == nil)
    {
      if (windowUndoManagers == NULL)
        {
          windowUndoManagers =
	    NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,
			     NSObjectMapValueCallBacks, 0);
        }
      else
        {
          undo = NSMapGet(windowUndoManagers, self);
        }
      if (undo == nil)
        {
          undo = [[NSUndoManager alloc] init];
          NSMapInsertKnownAbsent(windowUndoManagers, self, undo);
          RELEASE(undo);
        }
    }
  return undo;
}

- (void) undo: (id)sender
{
  [[_firstResponder undoManager] undo];
}

- (void) redo: (id)sender
{
  [[_firstResponder undoManager] redo];
}

/**
   If YES, then the window is released when the close method is called.
 */
- (void) setReleasedWhenClosed: (BOOL)flag
{
  _f.is_released_when_closed = flag;
}

/*
 * Aiding event handling
 */
- (BOOL) acceptsMouseMovedEvents
{
  return _f.accepts_mouse_moved;
}

- (void) setAcceptsMouseMovedEvents: (BOOL)flag
{
  _f.accepts_mouse_moved = flag;
}

- (BOOL) ignoresMouseEvents
{
  return _f.ignores_mouse_events;
}

- (void) setIgnoresMouseEvents: (BOOL)flag
{
  _f.ignores_mouse_events = flag;
  [GSServerForWindow(self) setIgnoreMouse: flag : _windowNum];
}

- (NSEvent*) currentEvent
{
  return [NSApp currentEvent];
}

- (void) discardEventsMatchingMask: (NSUInteger)mask
                       beforeEvent: (NSEvent*)lastEvent
{
  [NSApp discardEventsMatchingMask: mask beforeEvent: lastEvent];
}

/**
   Returns the first responder of the window.
 */
- (NSResponder*) firstResponder
{
  return _firstResponder;
}

/**
   Returns YES, if the window can accept first responder.  The default
   implementation of this method returns YES.
 */
- (BOOL) acceptsFirstResponder
{
  return YES;
}

/**
   Makes aResponder the first responder within the receiver.
 */
- (BOOL) makeFirstResponder: (NSResponder*)aResponder
{
  if (_firstResponder == aResponder)
    {
      return YES;
    }

  if (aResponder != nil)
    {
      if (![aResponder isKindOfClass: responderClass])
        {
          return NO;
        }

      if (![aResponder acceptsFirstResponder])
        {
          return NO;
        }
    }

  /* So that the implementation of -resignFirstResponder in
     _firstResponder might ask for what will be the new first
     responder by calling our method _futureFirstResponder */
  _futureFirstResponder = aResponder;

  /*
   * If there is a first responder tell it to resign.
   * Change only if it replies YES.
   */
  if ((_firstResponder) && (![_firstResponder resignFirstResponder]))
    {
      return NO;
    }

  _firstResponder = aResponder;
  if ((aResponder == nil) || ![_firstResponder becomeFirstResponder])
    {
      _firstResponder = self;
      [_firstResponder becomeFirstResponder];
    }

  return YES;
}

/**
   Sets the initial first responder of the receiver.
 */
- (void) setInitialFirstResponder: (NSView*)aView
{
  if ([aView isKindOfClass: viewClass])
    {
      ASSIGN(_initialFirstResponder, aView);
    }
}

/**
   returns the initial first responder of the receiver.
 */
- (NSView*) initialFirstResponder
{
  return _initialFirstResponder;
}

/**
   Processes theEvent when a key is pressed while within
   the window.
 */
- (void) keyDown: (NSEvent*)theEvent
{
  NSString *characters = [theEvent characters];
  unichar character = 0;

  if ([characters length] > 0)
    {
      character = [characters characterAtIndex: 0];
    }

  if (character == NSHelpFunctionKey)
    {
      [NSHelpManager setContextHelpModeActive: YES];
      return;
    }

  // If this is a BACKTAB event, move to the previous key view
  if (character == NSBackTabCharacter)
    {
      [self selectPreviousKeyView: self];
      return;
    }

  // If this is a TAB or TAB+SHIFT event, move to the next key view
  if (character == NSTabCharacter)
    {
      if ([theEvent modifierFlags] & NSShiftKeyMask)
        [self selectPreviousKeyView: self];
      else
        [self selectNextKeyView: self];
      return;
    }

  // If this is an ESC event, abort modal loop
  if (character == 0x001b)
    {
      if ([NSApp modalWindow] == self)
        {
          // NB: The following *never* returns.
          [NSApp abortModal];
        }
      return;
    }

  if (character == NSEnterCharacter
    || character == NSFormFeedCharacter
    || character == NSCarriageReturnCharacter)
    {
      if (_defaultButtonCell && _f.default_button_cell_key_disabled == NO)
        {
          [_defaultButtonCell performClick: self];
          return;
        }
    }

  // Discard null character events such as a Shift event after a tab key
  if ([characters length] == 0)
    {
      return;
    }

  // FIXME: Why is this here, is the code still needed or a left over hack?
  // Try to process the event as a key equivalent
  // without Command having being pressed
  {
    NSEvent *new_event
      =  [NSEvent keyEventWithType: [theEvent type]
                  location: NSZeroPoint
                  modifierFlags: ([theEvent modifierFlags] | NSCommandKeyMask)
                  timestamp: [theEvent timestamp]
                  windowNumber: [theEvent windowNumber]
                  context: [theEvent context]
                  characters: characters
                  charactersIgnoringModifiers: [theEvent
                                                 charactersIgnoringModifiers]
                  isARepeat: [theEvent isARepeat]
                  keyCode: [theEvent keyCode]];
    if ([self performKeyEquivalent: new_event])
      {
        return;
      }
  }

  // Otherwise, pass the event up
  [super keyDown: theEvent];
}

- (void) keyUp: (NSEvent*)theEvent
{
  if ([NSHelpManager isContextHelpModeActive])
    {
      NSString *characters = [theEvent characters];
      unichar character = 0;

      if ([characters length] > 0)
        {
          character = [characters characterAtIndex: 0];
        }
      if (character == NSHelpFunctionKey)
        {
          [NSHelpManager setContextHelpModeActive: NO];
          return;
        }
    }

  [super keyUp: theEvent];
}

/* Return mouse location in reciever's base coord system, ignores event
 * loop status */
- (NSPoint) mouseLocationOutsideOfEventStream
{
  int screen;
  NSPoint p;

  screen = [_screen screenNumber];
  p = [GSServerForWindow(self) mouseLocationOnScreen: screen window: NULL];
  if (p.x != -1)
    {
      p = [self convertScreenToBase: p];
    }
  return p;
}

- (NSEvent*) nextEventMatchingMask: (NSUInteger)mask
{
  return [NSApp nextEventMatchingMask: mask
                            untilDate: [NSDate distantFuture]
                               inMode: NSEventTrackingRunLoopMode
                              dequeue: YES];
}

- (NSEvent*) nextEventMatchingMask: (NSUInteger)mask
                         untilDate: (NSDate*)expiration
                            inMode: (NSString*)mode
                           dequeue: (BOOL)deqFlag
{
  return [NSApp nextEventMatchingMask: mask
                            untilDate: expiration
                               inMode: mode
                              dequeue: deqFlag];
}

- (void) postEvent: (NSEvent*)event atStart: (BOOL)flag
{
  [NSApp postEvent: event atStart: flag];
}

- (void) _checkTrackingRectangles: (NSView*)theView
                         forEvent: (NSEvent*)theEvent
{
  if (theView == nil)
    {
      return;
    }
  if (theView->_rFlags.has_trkrects)
    {
      BOOL isFlipped = [theView isFlipped];
      NSArray *tr = theView->_tracking_rects;
      NSUInteger count = [tr count];

      /*
       * Loop through the tracking rectangles
       */
      if (count > 0)
        {
          GSTrackingRect *rects[count];
          NSPoint loc = [theEvent locationInWindow];
	  NSPoint lastPoint = _lastPoint;
	  NSRect vr = [theView visibleRect];
          NSUInteger i;

	  lastPoint = [theView convertPoint: lastPoint fromView: nil];
	  loc = [theView convertPoint: loc fromView: nil];
          [tr getObjects: rects];

          for (i = 0; i < count; ++i)
            {
              BOOL last;
              BOOL now;
              GSTrackingRect *r = rects[i];
	      NSRect tr = NSIntersectionRect(vr, r->rectangle);

              if ([r isValid] == NO)
                {
                  continue;
                }
              /* Check mouse at last point */
              last = NSMouseInRect(lastPoint, tr, isFlipped);
              /* Check mouse at current point */
              now = NSMouseInRect(loc, tr, isFlipped);

              if ((!last) && (now))                // Mouse entered event
                {
                  if (r->flags.checked == NO)
                    {
                      if ([r->owner respondsToSelector:
                        @selector(mouseEntered:)])
                        {
                          r->flags.ownerRespondsToMouseEntered = YES;
                        }
                      if ([r->owner respondsToSelector:
                        @selector(mouseExited:)])
                        {
                          r->flags.ownerRespondsToMouseExited = YES;
                        }
                      r->flags.checked = YES;
                    }
                  if (r->flags.ownerRespondsToMouseEntered)
                    {
                      NSEvent        *e;

                      e = [NSEvent enterExitEventWithType: NSMouseEntered
                        location: loc
                        modifierFlags: [theEvent modifierFlags]
                        timestamp: 0
                        windowNumber: [theEvent windowNumber]
                        context: NULL
                        eventNumber: 0
                        trackingNumber: r->tag
                        userData: r->user_data];
                      [r->owner mouseEntered: e];
                    }
                }

              if ((last) && (!now))                // Mouse exited event
                {
                  if (r->flags.checked == NO)
                    {
                      if ([r->owner respondsToSelector:
                        @selector(mouseEntered:)])
                        {
                          r->flags.ownerRespondsToMouseEntered = YES;
                        }
                      if ([r->owner respondsToSelector:
                        @selector(mouseExited:)])
                        {
                          r->flags.ownerRespondsToMouseExited = YES;
                        }
                      r->flags.checked = YES;
                    }
                  if (r->flags.ownerRespondsToMouseExited)
                    {
                      NSEvent        *e;

                      e = [NSEvent enterExitEventWithType: NSMouseExited
                        location: loc
                        modifierFlags: [theEvent modifierFlags]
                        timestamp: 0
                        windowNumber: [theEvent windowNumber]
                        context: NULL
                        eventNumber: 0
                        trackingNumber: r->tag
                        userData: r->user_data];
                      [r->owner mouseExited: e];
                    }
                }
            }
        }
    }

  /*
   * Check tracking rectangles for the subviews
   */
  if (theView->_rFlags.has_subviews)
    {
      NSArray *sb = theView->_sub_views;
      NSUInteger count = [sb count];

      if (count > 0)
        {
          NSView *subs[count];
          NSUInteger i;

          [sb getObjects: subs];
          for (i = 0; i < count; ++i)
            {
              if (![subs[i] isHidden])
                {
                  (*ctImp)(self, ctSel, subs[i], theEvent);
                }
            }
        }
    }
}

- (void) _checkCursorRectangles: (NSView*)theView forEvent: (NSEvent*)theEvent
{
  // As we add the events to the front of the queue, we need to add the last
  // events first. That is, first the enter events from inner to outer and
  // then the exit events
  checkCursorRectanglesEntered(theView, theEvent, _lastPoint);
  checkCursorRectanglesExited(theView, theEvent, _lastPoint);
  //[GSServerForWindow(self) _printEventQueue];
}

- (void) _processResizeEvent
{
  if (_windowNum && _gstate)
    {
      [GSServerForWindow(self) setWindowdevice: _windowNum
                        forContext: _context];
      GSReplaceGState(_context, _gstate);
    }

  [self update];
}

- (void) mouseDown: (NSEvent*)theEvent
{
  // Quietly discard an unused mouse down.
}

- (BOOL) becomesKeyOnlyIfNeeded
{
  return NO;
}

/** Handles mouse and other events sent to the receiver by NSApplication.
    Do not invoke this method directly.
*/
- (void) sendEvent: (NSEvent*)theEvent
{
  NSView *v;
  NSEventType type;

  /*
  If the backend reacts slowly, events (eg. mouse down) might arrive for a
  window that has been ordered out (and thus is logically invisible). We
  need to ignore those events. Otherwise, eg. clicking twice on a button
  that ends a modal session and closes the window with the button might
  cause the button to be pressed twice, which causes Bad Things to happen
  when it tries to stop a modal session twice.

  We let NSAppKitDefined events through since they deal with window ordering.
  */
  if (!_f.visible && [theEvent type] != NSAppKitDefined)
    {
      NSDebugLLog(@"NSEvent", @"Discard (window not visible) %@", theEvent);
      return;
    }

  if (!_f.cursor_rects_valid)
    {
      [self resetCursorRects];
    }

  type = [theEvent type];
  if ([self ignoresMouseEvents]
    && GSMouseEventMask == NSEventMaskFromType(type))
    {
      NSDebugLLog(@"NSEvent", @"Discard (window ignoring mouse) %@", theEvent);
      return;
    }

  switch (type)
    {
      case NSLeftMouseDown:
        {
          BOOL wasKey = _f.is_key;

          if (_f.has_closed == NO)
            {
              v = [_wv hitTest: [theEvent locationInWindow]];
              if (_f.is_key == NO && _windowLevel != NSDesktopWindowLevel)
                {
                  /* NSPanel modification: check becomesKeyOnlyIfNeeded. */
                  if (![self becomesKeyOnlyIfNeeded]
                      || [v needsPanelToBecomeKey])
                    {
                      v = nil;
                      [self makeKeyAndOrderFront: self];
                    }
                }
              /* Activate the app *after* making the receiver key, as app
                 activation tries to make the previous key window key.
		 However, don't activate the app after a single click into
		 the app icon or a miniwindow. This allows dragging app
		 icons and miniwindows without unnecessarily switching
		 applications (cf. Sect. 4 of the OpenStep UI Guidelines).
	      */
              if ((_styleMask & (NSIconWindowMask | NSMiniWindowMask)) == 0
	          && ![NSApp isActive])
                {
                  v = nil;
                  [NSApp activateIgnoringOtherApps: YES];
                }
              // Activating the app may change the window layout.
              if (v == nil)
                {
                  v = [_wv hitTest: [theEvent locationInWindow]];
                }
              if (_lastLeftMouseDownView)
                {
                  DESTROY(_lastLeftMouseDownView);
                }
              if (wasKey == YES || [v acceptsFirstMouse: theEvent])
                {
                  // Don't make buttons first responder otherwise they cannot
                  // send actions to the current first responder.
                  // TODO: First responder status update would more cleanly
                  // handled by -mouseDown in each control subclass (Mac OS X
                  // seems to do that).
                  if (_firstResponder != v && ![v isKindOfClass: [NSButton class]])
                    {
                      // Only try to set first responder, when the view wants it.
                      if ([v acceptsFirstResponder] && ![self makeFirstResponder: v])
                        {
                          return;
                        }
                    }
                  if ([NSHelpManager isContextHelpModeActive])
                    {
                      [v helpRequested: theEvent];
                    }
                  else
                    {
                      ASSIGN(_lastLeftMouseDownView, v);
                      if (toolTipVisible != nil)
                        {
                          /* Inform the tooltips system that we have had
                           * a mouse down so it should stop displaying.
                           */
                          [toolTipVisible mouseDown: theEvent];
                        }
                      [v mouseDown: theEvent];
                    }
                }
              else
                {
                  [self mouseDown: theEvent];
                }
            }
	  else
	    {
              NSDebugLLog(@"NSEvent", @"Discard (window closed) %@", theEvent);
	    }
          _lastPoint = [theEvent locationInWindow];
          break;
        }

      case NSLeftMouseUp:
        v = AUTORELEASE(RETAIN(_lastLeftMouseDownView));
        DESTROY(_lastLeftMouseDownView);
        if (v == nil)
          {
            break;
          }
        [v mouseUp: theEvent];
        _lastPoint = [theEvent locationInWindow];
        break;

      case NSOtherMouseDown:
          v = [_wv hitTest: [theEvent locationInWindow]];
	  ASSIGN(_lastOtherMouseDownView, v);
          [v otherMouseDown: theEvent];
          _lastPoint = [theEvent locationInWindow];
          break;

      case NSOtherMouseUp:
        v = AUTORELEASE(RETAIN(_lastOtherMouseDownView));
        DESTROY(_lastOtherMouseDownView);
        if (v == nil)
          {
            break;
          }
        [v otherMouseUp: theEvent];
        _lastPoint = [theEvent locationInWindow];
        break;

      case NSRightMouseDown:
        v = [_wv hitTest: [theEvent locationInWindow]];
	ASSIGN(_lastRightMouseDownView, v);
        [v rightMouseDown: theEvent];
        _lastPoint = [theEvent locationInWindow];
        break;

      case NSRightMouseUp:
        v = AUTORELEASE(RETAIN(_lastRightMouseDownView));
        DESTROY(_lastRightMouseDownView);
        if (v == nil)
          {
            break;
          }
        [v rightMouseUp: theEvent];
        _lastPoint = [theEvent locationInWindow];
        break;

      case NSLeftMouseDragged:
      case NSOtherMouseDragged:
      case NSRightMouseDragged:
      case NSMouseMoved:
        switch (type)
          {
            case NSLeftMouseDragged:
              [_lastLeftMouseDownView mouseDragged: theEvent];
              break;
            case NSOtherMouseDragged:
              [_lastOtherMouseDownView otherMouseDragged: theEvent];
              break;
            case NSRightMouseDragged:
              [_lastRightMouseDownView rightMouseDragged: theEvent];
              break;
            default:
              if (_f.accepts_mouse_moved)
                {
                  /*
                   * If the window is set to accept mouse movements, we need to
                   * forward the mouse movement to the correct view.
                   */
                  v = [_wv hitTest: [theEvent locationInWindow]];

                  /* If the view is displaying a tooltip, we should
                   * send mouse movements to the tooltip system so
                   * that the window can track the mouse.
                   */
                  if (toolTipVisible != nil)
                    {
                      [toolTipVisible mouseMoved: theEvent];
                    }
                  else
                    {
                      [v mouseMoved: theEvent];
                    }
                }
              break;
          }

        /*
         * We need to go through all of the views, and if there is any with
         * a tracking rectangle then we need to determine if we should send
         * a NSMouseEntered or NSMouseExited event.
         */
        (*ctImp)(self, ctSel, _wv, theEvent);

        if (_f.is_key)
          {
            /*
             * We need to go through all of the views, and if there is any with
             * a cursor rectangle then we need to determine if we should send a
             * cursor update event.
             */
            if (_f.cursor_rects_enabled)
              {
                (*ccImp)(self, ccSel, _wv, theEvent);
              }
          }

        _lastPoint = [theEvent locationInWindow];
        break;

      case NSMouseEntered:
      case NSMouseExited:
        break;

      case NSKeyDown:
	/* Always shift keyboard focus to the next and previous key view,
	 * respectively, upon receiving Ctrl-Tab and Ctrl-Shift-Tab keyboard
	 * events. This means that the key view loop won't get stuck in views
	 * that interpret the Tab key differently, e.g., NSTextView. (cf. the
	 * Keyboard Interface Control section in Apple's Cocoa Event-Handling
	 * Guide).
	 */
	if (([theEvent modifierFlags] & NSControlKeyMask)
	    && [[theEvent charactersIgnoringModifiers] isEqualToString: @"\t"])
	  {
	    if ([theEvent modifierFlags] & NSShiftKeyMask)
              {
                [self selectPreviousKeyView: self];
              }
	    else
              {
                [self selectNextKeyView: self];
              }
	  }
	else
	  [_firstResponder keyDown: theEvent];
        break;

      case NSKeyUp:
        [_firstResponder keyUp: theEvent];
        break;

      case NSFlagsChanged:
        [_firstResponder flagsChanged: theEvent];
        break;

      case NSCursorUpdate:
        {
          GSTrackingRect *r =(GSTrackingRect*)[theEvent userData];
          NSCursor *c = (NSCursor*)[r owner];

	  // Don't update the cursor if the window isn't the key window.
	  if (!_f.is_key)
            {
              break;
            }

          if ([theEvent trackingNumber]) // It's a mouse entered
            {
	      /* Only send the event mouse entered if the
	       * cursor rectangle is valid. */
	      if ([r isValid])
		{
		  [c mouseEntered: theEvent];

		  /* This could seems redundant, but ensure the correct
		   * value to use in events mouse moved. And avoids strange
		   * issues with cursor. */
		  _lastPoint = [theEvent locationInWindow];
		}
            }
          else                           // it is a mouse exited
            {
              [c mouseExited: theEvent];
            }
        }
        break;

      case NSScrollWheel:
        v = [_wv hitTest: [theEvent locationInWindow]];
        [v scrollWheel: theEvent];
        break;

      case NSAppKitDefined:
        {
          id dragInfo;
          int action;
          NSEvent *e;
          GSAppKitSubtype sub = [theEvent subtype];

          switch (sub)
            {
            case GSAppKitWindowMoved:
              {
                NSScreen *oldScreen = _screen;

                _frame.origin.x = (CGFloat)[theEvent data1];
                _frame.origin.y = (CGFloat)[theEvent data2];
                NSDebugLLog(@"Moving", @"Move event: %d %@",
                            (int)_windowNum, NSStringFromPoint(_frame.origin));
                if (_autosaveName != nil)
                  {
                    [self saveFrameUsingName: _autosaveName];
                  }
                [nc postNotificationName: NSWindowDidMoveNotification
                                  object: self];
                if ([self screen] != oldScreen)
                  {
                    [nc postNotificationName: NSWindowDidChangeScreenNotification
                                      object: self];
                  }
              }
              break;

            case GSAppKitWindowResized:
              {
                NSRect newFrame;

                newFrame.size.width = [theEvent data1];
                newFrame.size.height = [theEvent data2];
                /* Resize events always move the frame origin. The new origin
                   is stored in the event location field. */
                newFrame.origin = [theEvent locationInWindow];

                /* FIXME: For a user resize we should call windowWillResize:toSize:
                   on the delegate.
                 */
                _frame = newFrame;
                newFrame.origin = NSZeroPoint;
                [_wv setFrame: newFrame];
                [_wv setNeedsDisplay: YES];

                if (_autosaveName != nil)
                  {
                    [self saveFrameUsingName: _autosaveName];
                  }

                [self _processResizeEvent];
                [nc postNotificationName: NSWindowDidResizeNotification
                                  object: self];
                break;
              }

            case GSAppKitRegionExposed:
              {
                NSRect region;

                region.size.width = [theEvent data1];
                region.size.height = [theEvent data2];
                region.origin = [theEvent locationInWindow];
                switch (_backingType)
                  {
                    case NSBackingStoreBuffered:
                    case NSBackingStoreRetained:
                      /*
                       * The backend may have the region buffered ...
                       * so we add it to the rectangle to be flushed
                       * and set the flag to say that a flush is required.
                       */
                      _rectNeedingFlush
                        = NSUnionRect(_rectNeedingFlush, region);
                      _f.needs_flush = YES;
                      /* Some or all of the window has not been drawn,
                       * so we must at least make sure that the exposed
                       * region gets drawn before its backing store is
                       * flushed ... otherwise we might actually flush
                       * bogus data from an out of date buffer.
                       * Maybe we should call
                       * [_wv displayIfNeededInRect: region]
                       * but why not do all drawing at this point so
                       * that if we get another expose event immediately
                       * (eg. something is dragged over the window and
                       * we get a series of expose events) we can just
                       * flush without having to draw again.
                       */
                      [self displayIfNeeded];
                      [self flushWindowIfNeeded];
                      break;

                    default:
                      /* non-retained ... so we need to redraw the exposed
                       * region here.
                       */
                      [_wv setNeedsDisplayInRect: region];
                      break;
                  }
                }
              break;

            case GSAppKitWindowClose:
              [self performClose: NSApp];
              break;

            case GSAppKitWindowDeminiaturize:
              [self _didDeminiaturize: NSApp];
              break;

            case GSAppKitWindowMiniaturize:
              [self performMiniaturize: NSApp];
              break;

            case GSAppKitAppHide:
              [NSApp hide: self];
              break;

            case GSAppKitWindowFocusIn:
              if (_f.is_miniaturized)
		{
		  /* Window Manager just deminiaturized us */
		  [self deminiaturize: self];
		}
              if ([NSApp modalWindow]
		&& self != [NSApp modalWindow]
		&& ![self worksWhenModal])
		{
		  /* Ignore this request. We're in a modal loop and the
		     user pressed on the title bar of another window. */
		  break;
		}
              if ([self canBecomeKeyWindow] == YES)
		{
		  NSDebugLLog(@"Focus", @"Making %d key", (int)_windowNum);
		  [self makeKeyWindow];
		  [self makeMainWindow];
		  [NSApp activateIgnoringOtherApps: YES];
		}
              if (self == [[NSApp mainMenu] window])
		{
		  /* We should really find another window that can become
		     key (if possible)
		  */
		  [self _lossOfKeyOrMainWindow];
		}
              break;

            case GSAppKitWindowFocusOut:
              break;

            case GSAppKitWindowLeave:
	      /* we ignore this event for a window that is already closed */
	      if (_f.has_closed == YES)
                {
                  break;
                }

              /*
               * We need to go through all of the views, and if there
               * is any with a tracking rectangle then we need to
               * determine if we should send a NSMouseExited event.  */
              (*ctImp)(self, ctSel, _wv, theEvent);

              if (_f.is_key)
                {
                  /*
                   * We need to go through all of the views, and if
                   * there is any with a cursor rectangle then we need
                   * to determine if we should send a cursor update
                   * event.  */
                  if (_f.cursor_rects_enabled)
                    {
                      checkCursorRectanglesExited(_wv, theEvent, _lastPoint);
                    }
                }

              _lastPoint = NSMakePoint(-1, -1);
              break;

            case GSAppKitWindowEnter:
              break;


#define     GSPerformDragSelector(view, sel, info, action) \
              if ([view window] == self) \
                { \
                  id target = view; \
                  \
                  if (target == _wv) \
                    { \
                      if (_delegate != nil \
                        && [_delegate respondsToSelector: sel] == YES) \
                        { \
                          target = _delegate; \
                        } \
                      else \
                        { \
                          target = self; \
                        } \
                    } \
                  \
                  if ([target respondsToSelector: sel]) \
                    { \
                      action = (intptr_t)[target performSelector: sel \
                                                      withObject: info]; \
                    } \
                }

#define     GSPerformVoidDragSelector(view, sel, info) \
              if ([view window] == self) \
                {  \
                  id target = view; \
                  \
                  if (target == _wv) \
                    { \
                      if (_delegate != nil \
                        && [_delegate respondsToSelector: sel] == YES) \
                        { \
                          target = _delegate; \
                        } \
                      else \
                        { \
                          target = self; \
                        } \
                    } \
                  \
                  if ([target respondsToSelector: sel]) \
                    { \
                      [target performSelector: sel withObject: info];   \
                    } \
                }

            case GSAppKitDraggingEnter:
            case GSAppKitDraggingUpdate:
            {
              BOOL        isEntry;

              dragInfo = [GSServerForWindow(self) dragInfo];
              v = [_wv hitTest: [theEvent locationInWindow]];

              while (v != nil)
                {
                  if (v->_rFlags.has_draginfo != 0
                      && GSViewAcceptsDrag(v, dragInfo))
                    {
                      break;
                    }
                  v = [v superview];
                }
              if (v == nil)
                {
                  v = _wv;
                }
              if (_lastDragView == v)
                {
                  isEntry = NO;
                }
              else
                {
                  isEntry = YES;
                  if (_lastDragView != nil && _f.accepts_drag)
                    {
                      NSDebugLLog(@"NSDragging", @"Dragging exit");
                      GSPerformVoidDragSelector(_lastDragView,
                        @selector(draggingExited:), dragInfo);
                    }
                  ASSIGN(_lastDragView, v);
                  _f.accepts_drag = GSViewAcceptsDrag(v, dragInfo);
                }
              if (_f.accepts_drag)
                {
                  if (isEntry == YES)
                    {
                      action = NSDragOperationNone;
                      NSDebugLLog(@"NSDragging", @"Dragging entered");
                      GSPerformDragSelector(v, @selector(draggingEntered:),
                        dragInfo, action);
                    }
                  else
                    {
                      action = _lastDragOperationMask;
                      NSDebugLLog(@"NSDragging", @"Dragging updated");
                      GSPerformDragSelector(v, @selector(draggingUpdated:),
                                            dragInfo, action);
                    }
                }
              else
                {
                  action = NSDragOperationNone;
                }

              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: [theEvent locationInWindow]
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: _windowNum
                           context: GSCurrentContext()
                           subtype: GSAppKitDraggingStatus
                           data1: [theEvent data1]
                           data2: action];

              _lastDragOperationMask = action;
              [dragInfo postDragEvent: e];
              break;
            }

            case GSAppKitDraggingStatus:
              NSDebugLLog(@"NSDragging",
                @"Internal: dropped GSAppKitDraggingStatus event");
              break;

            case GSAppKitDraggingExit:
              NSDebugLLog(@"NSDragging", @"GSAppKitDraggingExit");
              dragInfo = [GSServerForWindow(self) dragInfo];
              if (_lastDragView && _f.accepts_drag)
                {
                  NSDebugLLog(@"NSDragging", @"Dragging exit");
                  GSPerformVoidDragSelector(_lastDragView,
                    @selector(draggingExited:), dragInfo);
                }
              _lastDragOperationMask = NSDragOperationNone;
              DESTROY(_lastDragView);
              break;

            case GSAppKitDraggingDrop:
              NSDebugLLog(@"NSDragging", @"GSAppKitDraggingDrop");
              dragInfo = [GSServerForWindow(self) dragInfo];
              if (_lastDragView && _f.accepts_drag
		  && _lastDragOperationMask != NSDragOperationNone)
                {
                  action = YES;
                  GSPerformDragSelector(_lastDragView,
                    @selector(prepareForDragOperation:), dragInfo, action);
                  if (action)
                    {
                      action = NO;
                      GSPerformDragSelector(_lastDragView,
                        @selector(performDragOperation:), dragInfo, action);
                    }
                  if (action)
                    {
                      GSPerformVoidDragSelector(_lastDragView,
                        @selector(concludeDragOperation:), dragInfo);
                    }
                }
              _lastDragOperationMask = NSDragOperationNone;
              DESTROY(_lastDragView);
              e = [NSEvent otherEventWithType: NSAppKitDefined
                           location: [theEvent locationInWindow]
                           modifierFlags: 0
                           timestamp: 0
                           windowNumber: _windowNum
                           context: GSCurrentContext()
                           subtype: GSAppKitDraggingFinished
                           data1: [theEvent data1]
                           data2: 0];
              [dragInfo postDragEvent: e];
              break;

            case GSAppKitDraggingFinished:
              _lastDragOperationMask = NSDragOperationNone;
              DESTROY(_lastDragView);
              NSDebugLLog(@"NSDragging",
                @"Internal: dropped GSAppKitDraggingFinished event");
              break;

            default:
              break;
            }
        }
        break;

      case NSPeriodic:
      case NSSystemDefined:
      case NSApplicationDefined:
        break;

      case NSTabletPoint:
      case NSTabletProximity:
        // FIXME: Tablet events
        break;
    }
}

- (BOOL) shouldBeTreatedAsInkEvent: (NSEvent *)theEvent
{
  NSView *v;

  v = [_wv hitTest: [theEvent locationInWindow]];
  if (![self isMainWindow])
    {
      return (v != _wv);
    }
  else
    {
      return [v shouldBeTreatedAsInkEvent: theEvent];
    }
}

- (BOOL) tryToPerform: (SEL)anAction with: (id)anObject
{
  if ([super tryToPerform: anAction with: anObject])
    {
      return YES;
    }
  else if (_delegate && [_delegate respondsToSelector: anAction])
    {
      [_delegate performSelector: anAction withObject: anObject];
      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) worksWhenModal
{
  return NO;
}

/** If aView responds to -nextValidKeyView with a new NSView, call
  -makeFirstResponder: for the returned view.
*/
- (void) selectKeyViewFollowingView: (NSView*)aView
{
  NSView *theView = nil;

  if ([aView isKindOfClass: viewClass])
    {
      theView = [aView nextValidKeyView];
    }
  if (theView)
    {
      if (![self makeFirstResponder: theView])
        {
          return;
        }
      if ([theView respondsToSelector:@selector(selectText:)])
        {
          _f.selectionDirection =  NSSelectingNext;
          [(id)theView selectText: self];
          _f.selectionDirection =  NSDirectSelection;
        }
    }
}

/** If aView responds to -previousValidKeyView with a new NSView, call
  -makeFirstResponder: for this view.
*/
- (void) selectKeyViewPrecedingView: (NSView*)aView
{
  NSView *theView = nil;

  if ([aView isKindOfClass: viewClass])
    {
      theView = [aView previousValidKeyView];
    }
  if (theView)
    {
      if (![self makeFirstResponder: theView])
        {
          return;
        }
      if ([theView respondsToSelector:@selector(selectText:)])
        {
          _f.selectionDirection =  NSSelectingPrevious;
          [(id)theView selectText: self];
          _f.selectionDirection =  NSDirectSelection;
        }
    }
}

/** This method checks if:
  <list>
   <item>_firstResponder answers to -nextValidKeyView</item>
   <item>_initialFirstResponder answers to -acceptsFirstResponder</item>
   <item>_initialFirstResponder answers to -previousValidKeyView</item>
  </list>
  If any of these checks return a NSView, call -makeFirstResponder: on
  this NSView.
*/
- (void) selectNextKeyView: (id)sender
{
  NSView *theView = nil;

  if ([_firstResponder isKindOfClass: viewClass])
    {
      theView = [_firstResponder nextValidKeyView];
    }

  if ((theView == nil) && (_initialFirstResponder))
    {
      if ([_initialFirstResponder acceptsFirstResponder])
        {
          theView = _initialFirstResponder;
        }
      else
        {
          theView = [_initialFirstResponder nextValidKeyView];
        }
    }

  if (theView)
    {
      if (![self makeFirstResponder: theView])
        {
          return;
        }
      if ([theView respondsToSelector:@selector(selectText:)])
        {
          _f.selectionDirection =  NSSelectingNext;
          [(id)theView selectText: self];
          _f.selectionDirection =  NSDirectSelection;
        }
    }
}

/** This method checks if:
  <list>
   <item>_firstResponder answers to -previousValidKeyView</item>
   <item>_initialFirstResponder answers to -acceptsFirstResponder</item>
   <item>_initialFirstResponder answers to -previousValidKeyView</item>
  </list>
  If any of these checks return a NSView, call -makeFirstResponder: on
  this NSView.
*/
- (void) selectPreviousKeyView: (id)sender
{
  NSView *theView = nil;

  if ([_firstResponder isKindOfClass: viewClass])
    {
      theView = [_firstResponder previousValidKeyView];
    }

  if ((theView == nil) && (_initialFirstResponder))
    {
      if ([_initialFirstResponder acceptsFirstResponder])
        {
          theView = _initialFirstResponder;
        }
      else
        {
          theView = [_initialFirstResponder previousValidKeyView];
        }
    }

  if (theView)
    {
      if (![self makeFirstResponder: theView])
        {
          return;
        }
      if ([theView respondsToSelector:@selector(selectText:)])
        {
          _f.selectionDirection =  NSSelectingPrevious;
          [(id)theView selectText: self];
          _f.selectionDirection =  NSDirectSelection;
        }
    }
}

// This is invoked by selectText: of some views (eg matrixes),
// to know whether they have received it from the window, and
// if so, in which direction is the selection moving (so that they know
// if they should select the last or the first editable cell).
/** Returns the value of _selectionDirection, the direction of the
current key view.<br />
  See Also:
  <list>
   <item>-selectKeyViewFollowingView:</item>
   <item>-selectKeyViewPrecedingView:</item>
   <item>-selectNextKeyView:</item>
   <item>-selectPreviousKeyView:</item>
  </list>
*/
- (NSSelectionDirection) keyViewSelectionDirection
{
  return _f.selectionDirection;
}

- (BOOL) autorecalculatesKeyViewLoop
{
  return _f.autorecalculates_keyview_loop;
}

- (void) setAutorecalculatesKeyViewLoop: (BOOL)flag
{
  _f.autorecalculates_keyview_loop = flag;
}

- (void) recalculateKeyViewLoop
{
// Should be called from NSView viewWillMoveToWindow (but only if
// -autorecalculatesKeyViewLoop returns YES)
  [_contentView _setUpKeyViewLoopWithNextKeyView: _contentView];
  [self setInitialFirstResponder: [_contentView nextValidKeyView]];
}


/*
 * Dragging
 */
- (void) dragImage: (NSImage*)anImage
                at: (NSPoint)baseLocation
            offset: (NSSize)initialOffset
             event: (NSEvent*)event
        pasteboard: (NSPasteboard*)pboard
            source: (id)sourceObject
         slideBack: (BOOL)slideFlag
{
  id dragView = [GSServerForWindow(self) dragInfo];

  [NSApp preventWindowOrdering];
  [dragView dragImage: anImage
                   at: [self convertBaseToScreen: baseLocation]
               offset: initialOffset
                event: event
           pasteboard: pboard
               source: sourceObject
            slideBack: slideFlag];
}

- (void) registerForDraggedTypes: (NSArray*)newTypes
{
  [_wv registerForDraggedTypes: newTypes];
}

- (void) unregisterDraggedTypes
{
  [_wv unregisterDraggedTypes];
}

/*
 * Services and windows menu support
 */
- (BOOL) isExcludedFromWindowsMenu
{
  return _f.menu_exclude;
}

- (void) setExcludedFromWindowsMenu: (BOOL)flag
{
  if (_f.menu_exclude != flag)
    {
      _f.menu_exclude = flag;
      if (flag == YES)
        {
          [NSApp removeWindowsItem: self];
        }
      else if (_f.has_opened == YES && flag == NO)
        {
          [NSApp addWindowsItem: self
                          title: _windowTitle
                       filename: [self _hasTitleWithRepresentedFilename]];
        }
    }
}

- (id) validRequestorForSendType: (NSString*)sendType
                      returnType: (NSString*)returnType
{
  id result = nil;

  if (_delegate && [_delegate respondsToSelector: _cmd]
      && ![_delegate isKindOfClass: [NSResponder class]])
    {
      result = [_delegate validRequestorForSendType: sendType
                                         returnType: returnType];
    }

  if (result == nil)
    {
      result = [NSApp validRequestorForSendType: sendType
                                     returnType: returnType];
    }

  return result;
}

/*
 * Saving and restoring the frame
 */
- (NSString*) frameAutosaveName
{
  return _autosaveName;
}

- (void) saveFrameUsingName: (NSString*)name
{
  NSUserDefaults *defs;
  NSString *key;
  id obj;

  defs = [NSUserDefaults standardUserDefaults];
  obj = [self stringWithSavedFrame];
  key = [NSString stringWithFormat: @"NSWindow Frame %@", name];
  [defs setObject: obj forKey: key];
}

- (BOOL) setFrameAutosaveName: (NSString*)name
{
  if ([name isEqual: _autosaveName])
    {
      return YES;                /* That's our name already.        */
    }

  if ([autosaveNames member: name] != nil)
    {
      return NO;                /* Name in use elsewhere.        */
    }
  if (_autosaveName != nil)
    {
      [[self class] removeFrameUsingName: _autosaveName];
      [autosaveNames removeObject: _autosaveName];
      _autosaveName = nil;
    }
  if (name != nil && [name isEqual: @""] == NO)
    {
      name = [name copy];
      [autosaveNames addObject: name];
      _autosaveName = name;
      RELEASE(name);
      if (![self setFrameUsingName: _autosaveName])
        {
          [self saveFrameUsingName: _autosaveName];
        }
    }
  return YES;
}

- (void) setFrameFromString: (NSString*)string
{
  NSScanner *scanner = [NSScanner scannerWithString: string];
  NSRect nRect;
  NSRect sRect;
  NSRect fRect;
  int value;
  NSScreen *screen;

  /*
   * Scan in the window frame (flipped coordinate system).
   */
  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad window frame format - x-coord missing");
      return;
    }
  fRect.origin.x = value;

  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad window frame format - y-coord missing");
      return;
    }
  fRect.origin.y = value;

  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad window frame format - width missing");
      return;
    }
  fRect.size.width = value;

  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad window frame format - height missing");
      return;
    }
  fRect.size.height = value;

  /*
   * Check that the window will come up on screen
   */
#if 0 // Not valid since screen frame x/y values can be negative...
  if (fRect.origin.x + fRect.size.width < 0)
  {
    NSLog(@"Bad screen frame  - window is off screen");
    return;
  }
#endif

  // if toolbar is showing, adjust saved frame to add the toolbar back in
  if ([_toolbar isVisible])
    {
      CGFloat toolbarHeight = [[_toolbar _toolbarView] frame].size.height;
      fRect.size.height += toolbarHeight;
      fRect.origin.y -= toolbarHeight;
    }
  // if window has a menu, adjust saved frame to add the menu back in
  if ([_wv hasMenu])
    {
      CGFloat menuBarHeight = [[GSTheme theme] menuHeightForWindow: self];
      fRect.size.height += menuBarHeight;
      fRect.origin.y -= menuBarHeight;
    }

  /*
   * Scan in the frame for the area the window was placed in in screen.
   */
  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad screen frame format - x-coord missing");
      return;
    }
  sRect.origin.x = value;

  if ([scanner scanInt: &value] == NO)
    {
      NSLog(@"Bad screen frame format - y-coord missing");
      return;
    }
  sRect.origin.y = value;

  if (![scanner scanInt: &value])
    {
      NSLog(@"Bad screen frame format - width missing");
      return;
    }
  sRect.size.width = value;

  if (![scanner scanInt: &value])
    {
      NSLog(@"Bad screen frame format - height missing");
      return;
    }
  sRect.size.height = value;

  /*
   * The screen rectangle gives the area of the screen in which
   * the window could be placed (ie a rectangle excluding the dock).
   */
  screen = [self _screenForFrame: fRect];
  nRect = [screen visibleFrame];

  /*
   * If the new screen drawable area has moved relative to the one in
   * which the window was saved, adjust the window position accordingly.
   */
  if (!NSEqualPoints(nRect.origin, sRect.origin))
    {
      fRect.origin.x += nRect.origin.x - sRect.origin.x;
      fRect.origin.y += nRect.origin.y - sRect.origin.y;
    }

  /*
   * If the stored screen area is not the same as that currently
   * available, we adjust the window frame (position) to try to
   * make layout sensible.
   */
  if (nRect.size.width != sRect.size.width)
    {
      fRect.origin.x = nRect.origin.x + (fRect.origin.x - nRect.origin.x)
        * (nRect.size.width / sRect.size.width);
    }
  if (nRect.size.height != sRect.size.height)
    {
      fRect.origin.y = nRect.origin.y + (fRect.origin.y - nRect.origin.y)
        * (nRect.size.height / sRect.size.height);

      /*
       * If height of the window goes above the screen height, then adjust the window down.
       */
      if ((fRect.size.height + fRect.origin.y) > nRect.size.height)
      {
        fRect.origin.y = nRect.size.height - fRect.size.height;
      }
    }

  // FIXME: Is this check needed?
  /* If we aren't resizable (ie. if we don't have a resize bar), make sure
     we don't change the size. */
  if (!(_styleMask & NSResizableWindowMask))
    {
      fRect.size = _frame.size;
    }

  if (NSEqualSizes(fRect.size, _frame.size) == NO)
    {
      if ([_delegate respondsToSelector: @selector(windowWillResize:toSize:)])
        {
          fRect.size = [_delegate windowWillResize: self
                                  toSize: fRect.size];
        }
    }

  /*
   * Set frame.
   */
  [self setFrame: fRect display: (_f.visible) ? YES : NO];
}

- (BOOL) setFrameUsingName: (NSString*)name
{
  NSUserDefaults *defs;
  id obj;
  NSString *key;

  defs = [NSUserDefaults standardUserDefaults];
  key = [NSString stringWithFormat: @"NSWindow Frame %@", name];
  obj = [defs objectForKey: key];
  if (obj == nil)
    {
      return NO;
    }

  [self setFrameFromString: obj];
  return YES;
}

- (BOOL) setFrameUsingName: (NSString *)name
                     force: (BOOL)force
{
  // FIXME
  if ((_styleMask & NSResizableWindowMask) || force)
    {
      return [self setFrameUsingName: name];
    }
  else
    {
      return NO;
    }
}

- (NSString *) stringWithSavedFrame
{
  NSRect fRect;
  NSRect sRect;
  NSString *autosaveString;

  fRect = _frame;

  // if toolbar is showing, adjust saved frame to not include the toolbar
  if ([_toolbar isVisible])
    {
      CGFloat toolbarHeight = [[_toolbar _toolbarView] frame].size.height;
      fRect.size.height -= toolbarHeight;
      fRect.origin.y += toolbarHeight;
    }
  // if window has a menu, adjust saved frame to not include the menu
  if ([_wv hasMenu])
    {
      CGFloat menuBarHeight = [[GSTheme theme] menuHeightForWindow: self];
      fRect.size.height -= menuBarHeight;
      fRect.origin.y += menuBarHeight;
    }

  /*
   * The screen rectangle should give the area of the screen in which
   * the window could be placed (ie a rectangle excluding the dock).
   */
  sRect = [[self screen] visibleFrame];
  autosaveString = [NSString stringWithFormat: @"%d %d %d %d %d %d % d %d ",
                               (int)fRect.origin.x, (int)fRect.origin.y,
                               (int)fRect.size.width, (int)fRect.size.height,
                               (int)sRect.origin.x, (int)sRect.origin.y,
                               (int)sRect.size.width, (int)sRect.size.height];
  NSDebugLLog(@"NSWindow", @"%s:autosaveName: %@ frame string: %@", __PRETTY_FUNCTION__,
              _autosaveName, autosaveString);

  return autosaveString;
}

/*
 * Printing and postscript
 */
- (NSData *) dataWithEPSInsideRect: (NSRect)rect
{
  return [_wv dataWithEPSInsideRect:
                           [_wv convertRect: rect fromView: nil]];
}

- (NSData *) dataWithPDFInsideRect:(NSRect)aRect
{
  return [_wv dataWithPDFInsideRect:
                           [_wv convertRect: aRect fromView: nil]];
}

/**
   Opens the fax panel to allow the user to fax the contents of
   the window view.
*/
- (void) fax: (id)sender
{
  [_wv fax: sender];
}

/**
   Opens the print panel to allow the user to print the contents of
   the window view.
*/
- (void) print: (id)sender
{
  [_wv print: sender];
}

/*
 * Zooming
 */

#define DIST 3

/**
   Returns yes, if the receiver is zoomed.
 */
- (BOOL) isZoomed
{
  NSRect maxRect = [[self screen] visibleFrame];

  if ([_delegate respondsToSelector: @selector(windowWillUseStandardFrame:defaultFrame:)])
    {
      maxRect = [_delegate windowWillUseStandardFrame: self defaultFrame: maxRect];
    }
  else if ([self respondsToSelector: @selector(windowWillUseStandardFrame:defaultFrame:)])
    {
      maxRect = [self windowWillUseStandardFrame: self defaultFrame: maxRect];
    }
  else if ([_delegate respondsToSelector: @selector(windowWillResize:toSize:)])
    {
      maxRect.size = [_delegate windowWillResize: self toSize: maxRect.size];
    }
  else if ([self respondsToSelector: @selector(windowWillResize:toSize:)])
    {
      maxRect.size = [self windowWillResize: self toSize: maxRect.size];
    }

  // Compare the new frame with the current one
  return NSEqualRects(maxRect, _frame);
}

/**
   Performs the zoom method on the receiver.
*/
- (void) performZoom: (id)sender
{
  // FIXME: We should check for the style and highlight the button
  [self zoom: sender];
}

/**
   Zooms the receiver.   This method calls the delegate method
   windowShouldZoom:toFrame: to determine if the window should
   be allowed to zoom to full screen.
*/
- (void) zoom: (id)sender
{
  NSRect maxRect = [[self screen] visibleFrame];

  if ([_delegate respondsToSelector: @selector(windowWillUseStandardFrame:defaultFrame:)])
    {
      maxRect = [_delegate windowWillUseStandardFrame: self defaultFrame: maxRect];
    }
  else if ([self respondsToSelector: @selector(windowWillUseStandardFrame:defaultFrame:)])
    {
      maxRect = [self windowWillUseStandardFrame: self defaultFrame: maxRect];
    }

  maxRect = [self constrainFrameRect: maxRect toScreen: [self screen]];

  // Compare the new frame with the current one
  if ((fabs(NSMaxX(maxRect) - NSMaxX(_frame)) < DIST)
    && (fabs(NSMaxY(maxRect) - NSMaxY(_frame)) < DIST)
    && (fabs(NSMinX(maxRect) - NSMinX(_frame)) < DIST)
    && (fabs(NSMinY(maxRect) - NSMinY(_frame)) < DIST))
    {
      // Already in zoomed mode, reset user frame, if stored
      if (_autosaveName != nil)
        {
          [self setFrameUsingName: _autosaveName];
        }
      return;
    }

  if ([_delegate respondsToSelector: @selector(windowShouldZoom:toFrame:)])
    {
      if (![_delegate windowShouldZoom: self toFrame: maxRect])
        {
          return;
        }
    }
  else if ([self respondsToSelector: @selector(windowShouldZoom:toFrame:)])
    {
      if (![self windowShouldZoom: self toFrame: maxRect])
        {
          return;
        }
    }

  if (_autosaveName != nil)
    {
      [self saveFrameUsingName: _autosaveName];
    }

  [self setFrame: maxRect display: YES];
}


/*
 * Default botton
 */

- (NSButtonCell *) defaultButtonCell
{
  return _defaultButtonCell;
}

- (void) setDefaultButtonCell: (NSButtonCell *)aCell
{
  ASSIGN(_defaultButtonCell, aCell);
  _f.default_button_cell_key_disabled = NO;

  [aCell setKeyEquivalent: @"\r"];
  [aCell setKeyEquivalentModifierMask: 0];
  [[GSTheme theme] didSetDefaultButtonCell: aCell];
}

- (void) disableKeyEquivalentForDefaultButtonCell
{
  _f.default_button_cell_key_disabled = YES;
}

- (void) enableKeyEquivalentForDefaultButtonCell
{
  _f.default_button_cell_key_disabled = NO;
}

- (NSArray *) childWindows
{
  return _children;
}

- (void) addChildWindow: (NSWindow *)child
                ordered: (NSWindowOrderingMode)place
{
  if (_children == nil)
    {
      _children = [[NSMutableArray alloc] init];
    }
  [_children addObject: child];
  [child setParentWindow: self];
}

- (void) removeChildWindow: (NSWindow *)child
{
  [_children removeObject: child];
  [child setParentWindow: nil];
}

- (NSWindow *) parentWindow
{
  return _parent;
}

- (void) setParentWindow: (NSWindow *)window
{
  _parent = window;

  if (_windowNum)
    {
      [GSServerForWindow(self) setParentWindow: [_parent windowNumber]
				forChildWindow: _windowNum];
    }
}

- (BOOL) allowsToolTipsWhenApplicationIsInactive
{
  return _f.allows_tooltips_when_inactive;
}

- (void) setAllowsToolTipsWhenApplicationIsInactive: (BOOL)flag
{
  _f.allows_tooltips_when_inactive = flag;
}

- (BOOL) isMovableByWindowBackground
{
  return _f.is_movable_by_window_background;
}

- (void) setMovableByWindowBackground: (BOOL)flag
{
  _f.is_movable_by_window_background = flag;
}

- (BOOL) displaysWhenScreenProfileChanges
{
  return _f.displays_when_screen_profile_changes;
}

- (void) setDisplaysWhenScreenProfileChanges: (BOOL)flag
{
  _f.displays_when_screen_profile_changes = flag;
}

/*
 * Menu item validation
 */

- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem
{
  BOOL result = YES;
  SEL  action = [anItem action];

  if (sel_isEqual(action, @selector(performClose:)))
    {
      result = ([self styleMask] & NSClosableWindowMask) ? YES : NO;
    }
  else if (sel_isEqual(action, @selector(performMiniaturize:)))
    {
      result = ([self styleMask] & NSMiniaturizableWindowMask) ? YES : NO;
    }
  else if (sel_isEqual(action, @selector(performZoom:)))
    {
      result = ([self styleMask] & NSResizableWindowMask) ? YES : NO;
    }
  else if (sel_isEqual(action, @selector(undo:)))
    {
      NSUndoManager *undo = [_firstResponder undoManager];
      if (undo == nil)
        {
          result = NO;
        }
      else
        {
          result = [undo canUndo];
          if ([(id)anItem respondsToSelector: @selector(setTitle:)])
            {
              if (result)
                {
                  [(id)anItem setTitle: [undo undoMenuItemTitle]];
                }
              else
                {
                  [(id)anItem setTitle:
                         [undo undoMenuTitleForUndoActionName: @""]];
                }
            }
        }
    }
  else if (sel_isEqual(action, @selector(redo:)))
    {
      NSUndoManager *undo = [_firstResponder undoManager];
      if (undo == nil)
        {
          result = NO;
        }
      else
        {
          result = [undo canRedo];
          if ([(id)anItem respondsToSelector: @selector(setTitle:)])
            {
              if (result)
                {
                  [(id)anItem setTitle: [undo redoMenuItemTitle]];
                }
              else
                {
                  [(id)anItem setTitle:
                         [undo redoMenuTitleForUndoActionName: @""]];
                }
            }
        }
    }
  else if (sel_isEqual(action, @selector(toggleToolbarShown:)))
    {
      NSToolbar *toolbar = [self toolbar];

      if (toolbar == nil)
        {
          result = NO;
        }
      else
        {
          result = YES;
          if ([(id)anItem respondsToSelector: @selector(setTitle:)])
            {
              if ([toolbar isVisible])
                {
                  [(id)anItem setTitle: _(@"Hide Toolbar")];
                }
              else
                {
                  [(id)anItem setTitle: _(@"Show Toolbar")];
                }
            }
        }
    }
  else if (sel_isEqual(action, @selector(runToolbarCustomizationPalette:)))
    {
      NSToolbar *toolbar = [self toolbar];

      if (toolbar == nil)
	{
	  result = NO;
	}
      else
	{
	  result = [toolbar allowsUserCustomization];
	}
    }

  return result;
}

- (BOOL) validateMenuItem: (NSMenuItem *)anItem
{
  return [self validateUserInterfaceItem: anItem];
}

/*
 * Assigning a delegate
 */

/**
   Returns the delegate.
 */
- (id) delegate
{
  return _delegate;
}

/**
   Sets the delegate to anObject.
*/
- (void) setDelegate: (id)anObject
{
  if (anObject == _delegate)
    {
      // don't remove previously registered notifications if delegate is unchanged!
      return;
    }

  if (_delegate)
    {
      [nc removeObserver: _delegate name: nil object: self];
    }
  _delegate = anObject;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(window##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(window##notif_name:) \
      name: NSWindow##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(DidBecomeKey);
  SET_DELEGATE_NOTIFICATION(DidBecomeMain);
  SET_DELEGATE_NOTIFICATION(DidChangeScreen);
  SET_DELEGATE_NOTIFICATION(DidDeminiaturize);
  SET_DELEGATE_NOTIFICATION(DidExpose);
  SET_DELEGATE_NOTIFICATION(DidMiniaturize);
  SET_DELEGATE_NOTIFICATION(DidMove);
  SET_DELEGATE_NOTIFICATION(DidResignKey);
  SET_DELEGATE_NOTIFICATION(DidResignMain);
  SET_DELEGATE_NOTIFICATION(DidResize);
  SET_DELEGATE_NOTIFICATION(DidUpdate);
  SET_DELEGATE_NOTIFICATION(WillClose);
  SET_DELEGATE_NOTIFICATION(WillMiniaturize);
  SET_DELEGATE_NOTIFICATION(WillMove);
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  BOOL flag;

  // If were're being initialized from a keyed coder...
  if ([aCoder allowsKeyedCoding])
    {
      // The docs indicate that there should be an error when directly encoding with
      // a keyed coding archiver. We should only encode NSWindow and subclasses
      // using NSWindowTemplate.
      [NSException raise: NSInvalidArgumentException
                   format: @"Keyed coding not implemented for %@.",
                   NSStringFromClass([self class])];
    }

  [super encodeWithCoder: aCoder];

  [aCoder encodeRect: [[self contentView] frame]];
  [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_styleMask];
  // This used to be int, we need to stay compatible
  [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_backingType];

  [aCoder encodePoint: NSMakePoint(NSMinX([self frame]), NSMaxY([self frame]))];
  [aCoder encodeObject: _contentView];
  [aCoder encodeObject: _backgroundColor];
  [aCoder encodeObject: _representedFilename];
  [aCoder encodeObject: _miniaturizedTitle];
  [aCoder encodeObject: _windowTitle];

  //  [aCoder encodeSize: _minimumSize];
  //  [aCoder encodeSize: _maximumSize];
  [aCoder encodeSize: [self contentMinSize]];
  [aCoder encodeSize: [self contentMaxSize]];

  [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_windowLevel];

  flag = _f.menu_exclude;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.is_one_shot;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.is_autodisplay;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.optimize_drawing;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.dynamic_depth_limit;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.cursor_rects_enabled;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.is_released_when_closed;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.hides_on_deactivate;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
  flag = _f.accepts_mouse_moved;
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];

  [aCoder encodeObject: _miniaturizedImage];
  [aCoder encodeConditionalObject: _initialFirstResponder];
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  id oldself = self;
  BOOL flag;

  // If were're being initialized from a keyed coder...
  if ([aDecoder allowsKeyedCoding])
    {
      // The docs indicate that there should be an error when directly encoding with
      // a keyed coding archiver.  We should only encode NSWindow and subclasses
      // using NSWindowTemplate.
      [NSException raise: NSInvalidArgumentException
                   format: @"Keyed coding not implemented for %@.",
                   NSStringFromClass([self class])];
    }


  if ((self = [super initWithCoder: aDecoder]) == oldself)
    {
      NSSize aSize;
      NSRect aRect;
      NSPoint p;
      NSUInteger aStyle;
      NSBackingStoreType aBacking;
      NSInteger level;
      id obj;
      int version = [aDecoder versionForClassName: @"NSWindow"];

      aRect = [aDecoder decodeRect];
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger)
                                   at: &aStyle];
      // This used to be int, we need to stay compatible
      [aDecoder decodeValueOfObjCType: @encode(NSInteger)
                                   at: &aBacking];

      // call the designated initializer....
      self = [self initWithContentRect: aRect
                             styleMask: aStyle
                               backing: aBacking
                                 defer: NO];

      p = [aDecoder decodePoint];
      obj = [aDecoder decodeObject];
      [self setContentView: obj];
      obj = [aDecoder decodeObject];
      [self setBackgroundColor: obj];
      obj = [aDecoder decodeObject];
      [self setRepresentedFilename: obj];
      obj = [aDecoder decodeObject];
      [self setMiniwindowTitle: obj];
      obj = [aDecoder decodeObject];
      [self setTitle: obj];

      if (version < 3)
        {
          aSize = [aDecoder decodeSize];
          [self setMinSize: aSize];
          aSize = [aDecoder decodeSize];
          [self setMaxSize: aSize];
        }
      else
        {
          aSize = [aDecoder decodeSize];
          [self setContentMinSize: aSize];
          aSize = [aDecoder decodeSize];
          [self setContentMaxSize: aSize];
        }

      [aDecoder decodeValueOfObjCType: @encode(NSInteger)
                                   at: &level];
      [self setLevel: level];

      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setExcludedFromWindowsMenu: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setOneShot: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setAutodisplay: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self useOptimizedDrawing: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setDynamicDepthLimit: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      if (flag)
        [self enableCursorRects];
      else
        [self disableCursorRects];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setReleasedWhenClosed: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setHidesOnDeactivate: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setAcceptsMouseMovedEvents: flag];

      /* If the image has been specified, use it, if not use the default. */
      obj = [aDecoder decodeObject];
      if (obj != nil)
        {
          ASSIGN(_miniaturizedImage, obj);
        }

      [aDecoder decodeValueOfObjCType: @encode(id)
                                   at: &_initialFirstResponder];

      [self setFrameTopLeftPoint: p];
    }

  return self;
}

- (void) bind: (NSString *)binding
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding isEqual: NSTitleBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueBinding alloc] initWithBinding: @"title"
                                              withName: NSTitleBinding
                                              toObject: anObject
                                           withKeyPath: keyPath
                                               options: options
                                            fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding toObject: anObject withKeyPath: keyPath
        options: options];
    }
}

/**
   Returns all drawers associated with this window.
*/
- (NSArray *) drawers
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
        "drawers", "NSWindow");
  return nil;
}

- (void *)windowRef
{
  GSDisplayServer *srv = GSServerForWindow(self);

  return [srv windowDevice: _windowNum];
}

- (void *) windowHandle
{
  // Should only be defined on MS Windows
  return (void *)(intptr_t)_windowNum;
}

- (NSWindow *) attachedSheet
{
  return _attachedSheet;
}

- (NSWindow *) sheetParent
{
  return nil;
}

- (CGFloat) backingScaleFactor
{
  return 1.0;
}

+ (NSInteger)windowNumberAtPoint:(NSPoint)point
     belowWindowWithWindowNumber:(NSInteger)windowNumber
{
  return 0;
}

@end

@implementation NSWindow (Toolbar)

- (void) runToolbarCustomizationPalette: (id)sender
{
  [[self toolbar] runCustomizationPalette: sender];
}

- (void) toggleToolbarShown: (id)sender
{
  NSToolbar *toolbar = [self toolbar];
  BOOL isVisible = [toolbar isVisible];

  if (!toolbar)
    {
      return;
    }

  // We do this again on a lower level, but doing it here is faster.
  if (isVisible)
    {
      [_wv removeToolbarView: [toolbar _toolbarView]];
    }
  else
    {
      [_wv addToolbarView: [toolbar _toolbarView]];
    }

  [toolbar setVisible: !isVisible];

  [self display];
}

// Accessors

- (NSToolbar *) toolbar
{
  return _toolbar;
}

- (void) setToolbar: (NSToolbar*)toolbar
{
  if (toolbar == _toolbar)
    {
      return;
    }

  if (_toolbar != nil)
    {
      if ([_toolbar isVisible])
        {
          // Throw the last toolbar view out
          [_wv removeToolbarView: [_toolbar _toolbarView]];
        }
    }

  ASSIGN(_toolbar, toolbar);

  if (_toolbar != nil)
    {
      if ([_toolbar isVisible])
        {
          // Make the new toolbar view visible
          [_wv addToolbarView: [_toolbar _toolbarView]];
        }
    }

  // To show the changed toolbar
  [self displayIfNeeded];
}

@end

@implementation NSWindow (Menu)

- (void) setMenu: (NSMenu *)menu
{
  // Do theme specific logic...
  [[GSTheme theme] setMenu: menu forWindow: self];

  if ([self menu] != menu)
    {
      [super setMenu: menu];
    }
}

@end

/*
 * GNUstep backend methods
 */
@implementation NSWindow (GNUstepBackend)

/*
 * Mouse capture/release
 */
- (void) _captureMouse: sender
{
  NSDebugLLog(@"CaptureMouse", @"Capturing the mouse");
  [GSCurrentServer() capturemouse: _windowNum];
}

- (void) _releaseMouse: sender
{
  NSDebugLLog(@"CaptureMouse", @"Releasing the mouse");
  [GSCurrentServer() releasemouse];
}

- (void) _setVisible: (BOOL)flag
{
  _f.visible = flag;
}

- (void) performDeminiaturize: sender
{
  [self deminiaturize: sender];
}

/*
 * Allow subclasses to init without the backend
 * class attempting to create an actual window
 */
- (void) _initDefaults
{
  _firstResponder = self;
//  _initialFirstResponder = nil;
//  _delegate = nil;
//  _windowNum = 0;
//  _gstate = 0;
  _backgroundColor = RETAIN([NSColor windowBackgroundColor]);
  _representedFilename = @"Window";
  _miniaturizedTitle = @"Window";
  _miniaturizedImage = RETAIN([NSApp applicationIconImage]);
  _windowTitle = @"Window";
  _lastPoint = NSZeroPoint;
  _windowLevel = NSNormalWindowLevel;

  _depthLimit = NSDefaultDepth;
  _disableFlushWindow = 0;
  _alphaValue = 1.0;

//  _f.accepts_drag = NO;
//  _f.is_one_shot = NO;
//  _f.needs_flush = NO;
  _f.is_autodisplay = YES;
//  _f.optimize_drawing = NO;
  _f.dynamic_depth_limit = YES;
//  _f.cursor_rects_enabled = NO;
//  _f.cursor_rects_valid = NO;
//  _f.visible = NO;
//  _f.is_key = NO;
//  _f.is_main = NO;
//  _f.is_edited = NO;
  _f.is_released_when_closed = YES;
//  _f.is_miniaturized = NO;
//  _f.menu_exclude = NO;
//  _f.hides_on_deactivate = NO;
//  _f.accepts_mouse_moved = NO;
//  _f.has_opened = NO;
//  _f.has_closed = NO;
//  _f.default_button_cell_key_disabled = NO;
  _f.can_hide = YES;
//  _f.has_shadow = NO;
  _f.is_opaque = YES;
  _f.views_need_display = YES;
  _f.selectionDirection = NSDirectSelection;
}

@end

@implementation NSWindow (GNUstepTextView)
- (id) _futureFirstResponder
{
  return _futureFirstResponder;
}
@end

@implementation NSApplication(Inactive)
- (BOOL) _isWindowInactive: (NSWindow *)window
{
  return [_inactive containsObject: window];
}

- (void) _setWindow: (NSWindow *)window inactive: (BOOL)inactive
{
  if (inactive)
    {
      [_inactive addObject: window];
    }
  else
    {
      [_inactive removeObject: window];
    }
}
@end

BOOL GSViewAcceptsDrag(NSView *v, id<NSDraggingInfo> dragInfo)
{
  NSPasteboard *pb = [dragInfo draggingPasteboard];
  if ([pb availableTypeFromArray: GSGetDragTypes(v)])
    {
    return YES;
    }
  else
    {
      return NO;
    }
}

void NSCountWindows(NSInteger *count)
{
  *count = [[GSCurrentServer() windowlist] count];
}

void NSWindowList(NSInteger size, NSInteger list[])
{
  NSArray *windowList = [GSCurrentServer() windowlist];
  NSUInteger i, c;

  for (i = 0, c = [windowList count]; i < size && i < c; i++)
    {
      list[i] = [[windowList objectAtIndex: i] integerValue];
    }
}

NSArray *GSOrderedWindows(void)
{
  NSArray *window_list = [GSCurrentServer() windowlist];
  NSMutableArray *ret = [NSMutableArray array];
  NSUInteger i, c;

  for (i = 0, c = [window_list count]; i < c; i++)
    {
      NSInteger windowNumber = [[window_list objectAtIndex: i] integerValue];
      NSWindow *win = GSWindowWithNumber(windowNumber);

      if (win != nil)
        {
          [ret addObject: win];
        }
    }

  return ret;
}


NSArray* GSAllWindows(void)
{
  if (windowmaps)
    {
      return NSAllMapTableValues(windowmaps);
    }
  else
    {
      return nil;
    }
}

NSWindow* GSWindowWithNumber(NSInteger num)
{
  if (windowmaps)
    {
      return (NSWindow*)NSMapGet(windowmaps, (void*)(intptr_t)num);
    }
  else
    {
      return nil;
    }
}
