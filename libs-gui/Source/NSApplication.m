/** <title>NSApplication</title>

   <abstract>The one and only application class.</abstract>

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
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
#include <stdio.h>
#include <stdlib.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSErrorRecoveryAttempting.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>

#ifndef LIB_FOUNDATION_LIBRARY
#import <Foundation/NSConnection.h>
#endif

#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSAlert.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSDocumentController.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSPageLayout.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSToolbarItem.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSScreen.h"
#import "AppKit/PSOperators.h"

#import "GSIconManager.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSServicesManager.h"
#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSInfoPanel.h"
#import "GNUstepGUI/GSVersion.h"
#import "NSDocumentFrameworkPrivate.h"
#import "NSToolbarFrameworkPrivate.h"

// minimize icon when suppressed?
#define	MINI_ICON	0

/* The -gui thread. See the comment in initialize_gnustep_backend. */
NSThread *GSAppKitThread;

/* Notifications used to implement hide and unhide functionality. */
static NSString	*GSHideOtherApplicationsNotification
  = @"GSHideOtherApplicationsNotification";
static NSString	*GSUnhideAllApplicationsNotification
  = @"GSUnhideAllApplicationsNotification";

/*
 * Base library exception handler
 */
static NSUncaughtExceptionHandler *defaultUncaughtExceptionHandler;

/*
 * Gui library user friendly exception handler 
 */
static void
_NSAppKitUncaughtExceptionHandler (NSException *exception)
{
  NSInteger retVal;

  /* Reset the exception handler to the Base library's one, to prevent
     recursive calls to the gui one. */
  NSSetUncaughtExceptionHandler(defaultUncaughtExceptionHandler);  

  /*
   * If there is no graphics context to run the alert panel in or
   * its a severe error, use a non-graphical exception handler
   */
  if (GSCurrentContext() == nil
    || [[exception name] isEqual: NSWindowServerCommunicationException]
    || [[exception name] isEqual: GSWindowServerInternalException])
    {
      /* The following will raise again the exception using the base 
	 library exception handler */
      [exception raise];
    }

  retVal = GSRunExceptionPanel 
    ([NSString stringWithFormat: _(@"Critical Error in %@"),
	       [[NSProcessInfo processInfo] processName]],
     exception,
     _(@"Abort"), 
     nil,
#ifdef DEBUG
     _(@"Debug"));
#else
     nil);
#endif

  /* The user wants to abort */
  if (retVal == NSAlertDefault)
    {
      /* The following will raise again the exception using the base 
	 library exception handler */
      [exception raise];
    }
  else
    {
      /* Debug button: abort so we can trace the error in gdb */
      abort();
    }
}

/* Get the bundle.  */
NSBundle *
GSGuiBundle(void)
{
  /* This is the bundle from where we load localization of messages.  */
  static NSBundle *guiBundle = nil;

  if (!guiBundle)
    {
      /* Create the gui bundle we use to localize messages.  */
      guiBundle = [NSBundle bundleForLibrary: @"gnustep-gui"
			    version: OBJC_STRINGIFY(GNUSTEP_GUI_MAJOR_VERSION.GNUSTEP_GUI_MINOR_VERSION)];
      RETAIN(guiBundle);
    }
  return guiBundle;
}

@interface GSBackend : NSObject
{}
+ (void) initializeBackend;
@end

static NSString *
gnustep_backend_path(NSString *dir, NSString *name)
{
  NSString *path;
  NSEnumerator *benum;

  NSDebugFLLog(@"BackendBundle", @"Looking for %@", name);
  
  /* Find the backend framework */
  benum = [NSStandardLibraryPaths() objectEnumerator];
  while ((path = [benum nextObject]))
    {
      path = [path stringByAppendingPathComponent: dir];
      path = [path stringByAppendingPathComponent: name];
      if ([[NSFileManager defaultManager] fileExistsAtPath: path])
	{
	  break;
	}
    }
  return path;
}

/* Find and load the backend framework, if there is one. The name is
   taken from a user default containing the name of the backend framework,
   such as 'GNUstep-back', or simply 'back', or for historical reasons,
   'libgnustep-back'.  */
static NSString *
gnustep_backend_framework(NSString *bundleName)
{
  if (bundleName == nil)
    bundleName = @"GNUstep_back.framework";
  else
    {
      if ([bundleName hasPrefix: @"GNUstep-"])
	bundleName = [bundleName stringByAppendingString: @".framework"];
      else 
	{
	if  ([bundleName hasPrefix: @"libgnustep-"])
	  {
	    bundleName = [bundleName substringFromIndex: [@"libgnustep-" length]];
	  }
	bundleName = [NSString stringWithFormat: @"GNUstep-%@.framework",
			       bundleName];
	} 
    }

  return gnustep_backend_path(@"Frameworks", bundleName);
}

/* Find and load the backend bundle, if there is one. The name is
   taken from a user default containing the name of the backend bundle,
   such as 'back', or for historical reasons, 'libgnustep-back'. New
   versions may also have a version number associated with it.  */
static NSString *
gnustep_backend_bundle(NSString *bundleName)
{
  NSString *path, *bundleWithVersion;
  int version = GNUSTEP_GUI_MAJOR_VERSION * 100 + GNUSTEP_GUI_MINOR_VERSION;
  
  if (bundleName == nil)
    {
      bundleName = @"libgnustep-back";
    }
  else
    {
      if ([bundleName hasPrefix: @"libgnustep-"] == NO)
	{
	  bundleName = [NSString stringWithFormat: @"libgnustep-%@",
				 bundleName];
	} 
    }
  bundleWithVersion = [NSString stringWithFormat: @"%@-%03d.bundle",
				bundleName, version];
  bundleName = [bundleName stringByAppendingString: @".bundle"];
  path = gnustep_backend_path(@"Bundles", bundleWithVersion);
  if (path == nil)
    {
      NSLog(@"Did not find correct version of backend (%@), "
	@"falling back to std (%@).", bundleWithVersion, bundleName);
      path = gnustep_backend_path(@"Bundles", bundleName);
    }
  return path;
}

BOOL
initialize_gnustep_backend(void)
{
  static int first = 1;

  if (first)
    {
      Class backend;

      /*
      Remember which thread we are running in. This thread will be the
      -gui thread, ie. the only thread that may do any rendering. With
      the exception of a few methods explicitly marked as thread-safe,
      other threads should not call any methods in -gui.
      */
      GSAppKitThread = [NSThread currentThread];

      first = 0;
#ifdef BACKEND_BUNDLE
      {      
	NSBundle *theBundle;
	NSString *path, *bundleName;
	NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];

	/* What backend ? */
	bundleName = [defs stringForKey: @"GSBackend"];
	path = gnustep_backend_framework (bundleName);
	if (path == nil)
	  {
	    NSDebugLLog(@"BackendBundle", @"Did not find backend framework.");
	    path = gnustep_backend_bundle (bundleName);
	  }

	/* FIXME/TODO - update localized error messages.  */

	/* Backend found ? */
	if (bundleName == nil)
	  bundleName = @"back";
	NSCAssert1(path != nil, _(@"Unable to find backend %@"), bundleName);
	NSDebugLog(@"Loading Backend from %@", path);
	NSDebugFLLog(@"BackendBundle", @"Loading Backend from %@", path);

	/* Create a bundle object.  (Should normally succeed).  */
	theBundle = [NSBundle bundleWithPath: path];
	NSCAssert1(theBundle != nil, 
		   _(@"Can't create NSBundle object for backend at path %@"),
		   path);

	/* Now load the object file from the bundle.  */
	NSCAssert1 ([theBundle load],
		    _(@"Can't load object file from backend at path %@"),
		    path);
	
	/* Now get the GSBackend class, which should have just been loaded
	 * from the bundle.  */
	backend = NSClassFromString (@"GSBackend");
	NSCAssert1 (backend != Nil, 
	  _(@"Backend at path %@ doesn't contain the GSBackend class"), path);
	[backend initializeBackend];
      }

#else
      /* GSBackend will be in a separate library linked in with the app.
       This would be cleaner with ...classNamed: @"GSBackend", but that 
       doesn't work in some cases (Mac OS X for instance).  */
      [GSBackend initializeBackend];
#endif
    }
  return YES;
}

void
gsapp_user_bundles(void)
{
  NSUserDefaults *defs=[NSUserDefaults standardUserDefaults];
  NSArray *a=[defs arrayForKey: @"GSAppKitUserBundles"];
  NSUInteger i, c;
  c = [a count];
  if (a == nil || c == 0)
    return;
  NSLog(@"Loading %d user defined AppKit bundles", (int)c);
  for (i = 0; i < c; i++)
    {
      NSBundle *b = [NSBundle bundleWithPath: [a objectAtIndex: i]];
      if (!b)
	{
	  NSLog(@"* Unable to load '%@'", [a objectAtIndex: i]);
	  continue;
	}
      NSLog(@"Loaded '%@'\n", [a objectAtIndex: i]);
      [[[b principalClass] alloc] init];
    }
}

/*
 * Types
 */
struct _NSModalSession {
  NSInteger		runState;
  NSInteger		entryLevel;
  NSWindow		*window;
  NSModalSession	previous;
};
 
@interface NSApplication (Private)
- (void) _appIconInit;
- (void) _loadAppIconImage;
- (NSDictionary*) _notificationUserInfo;
- (void) _openDocument: (NSString*)name;
- (id) _targetForAction: (SEL)aSelector
	      keyWindow: (NSWindow *)keyWindow
	     mainWindow: (NSWindow *)mainWindow;
- (void) _windowDidBecomeKey: (NSNotification*) notification;
- (void) _windowDidBecomeMain: (NSNotification*) notification;
- (void) _windowDidResignKey: (NSNotification*) notification;
- (void) _windowWillClose: (NSNotification*) notification;
- (void) _workspaceNotification: (NSNotification*) notification;
- (NSArray *) _openFiles;
- (NSMenu *) _dockMenu;
@end

@interface NSWindow (TitleWithRepresentedFilename)
- (BOOL) _hasTitleWithRepresentedFilename;
@end

@interface NSWindow (ApplicationPrivate)
- (void) setAttachedSheet: (id) sheet;
@end

@implementation NSWindow (ApplicationPrivate)
/**
 * Associate sheet with the window it's attached to.  The window is not retained.
 */ 
- (void) setAttachedSheet: (id) sheet
{
  _attachedSheet = sheet;
}
@end

@interface NSIconWindow : NSWindow
@end

@interface NSAppIconView : NSView
- (void) setImage: (NSImage *)anImage;
@end

@interface NSMenu (HorizontalPrivate)
- (void) _organizeMenu;
@end

/*
 * Class variables
 */
static NSEvent *null_event;
static Class arpClass;
static NSNotificationCenter *nc;

NSApplication	*NSApp = nil;

@implementation	NSIconWindow

- (BOOL) canBecomeMainWindow
{
  return NO;
}

- (BOOL) canBecomeKeyWindow
{
  return NO;
}

- (BOOL) becomesKeyOnlyIfNeeded
{
  return YES;
}

- (BOOL) worksWhenModal
{
  return YES;
}

- (void) orderWindow: (NSWindowOrderingMode)place relativeTo: (NSInteger)otherWin
{     
  if ([[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSSuppressAppIcon"] == NO)
    {
      [super orderWindow: place relativeTo: otherWin];
    }
}

- (void) _initDefaults
{
  [super _initDefaults];
  /* Set the title of the window to the process name. Even as the
     window shows no title bar, the window manager may show it.  */
  [self setTitle: [[NSProcessInfo processInfo] processName]];
  [self setExcludedFromWindowsMenu: YES];
  [self setReleasedWhenClosed: NO];

#if	MINI_ICON
  /* Hack ... 
   * At least one window manager won't miniaturize a window unless
   * it's at the standard level.  If the app icon is suppressed, we
   * may still want a miniaturised version while the app is hidden.
   */
  if (YES == [[NSUserDefaults standardUserDefaults]
    boolForKey: @"GSSuppressAppIcon"])
    {
      return;
    }
#endif
  /* App icons and mini windows are displayed at dock level by default. Yet,
     with the current window level mapping in -back, some window managers
     will order pop up and context menus behind app icons and mini windows.
     Therefore, it is possible to have app icons and mini windows displayed
     at normal window level under control of a user preference. */
  // See also NSMiniWindow -_initDefaults in NSWindow.m
  if ([[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSAllowWindowsOverIcons"] == YES)
    _windowLevel = NSDockWindowLevel;
}

- (void) rightMouseDown: (NSEvent *)theEvent
{
  NSMenu *menu = nil;
  NSInterfaceStyle style = NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil);

  if (style == NSMacintoshInterfaceStyle || style == NSWindows95InterfaceStyle)
    {
      menu = [NSApp _dockMenu];
    }
  if (menu)
    {
      [NSMenu popUpContextMenu: menu
		     withEvent: theEvent
		       forView: [self contentView]];
    }
  else
    {
      [super rightMouseDown: theEvent];
    }
}

@end

@implementation NSAppIconView

// Class variables
static NSCell* dragCell = nil;
static NSCell* tileCell = nil;

static NSSize scaledIconSizeForSize(NSSize imageSize)
{
  NSSize iconSize, retSize;
  
  iconSize = GSGetIconSize();
  retSize.width = imageSize.width * iconSize.width / 64;
  retSize.height = imageSize.height * iconSize.height / 64;
  return retSize;
}

+ (void) initialize
{
  NSImage	*tileImage;
  NSSize	iconSize;

  iconSize = GSGetIconSize();
  /* _appIconInit will set our image */
  dragCell = [[NSCell alloc] initImageCell: nil];
  [dragCell setBordered: NO];
  
  tileImage = [[GSCurrentServer() iconTileImage] copy];
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

- (void) concludeDragOperation: (id<NSDraggingInfo>)sender
{
}

- (NSDragOperation) draggingEntered: (id<NSDraggingInfo>)sender
{
  return NSDragOperationGeneric;
}

- (void) draggingExited: (id<NSDraggingInfo>)sender
{
}

- (NSDragOperation) draggingUpdated: (id<NSDraggingInfo>)sender
{
  return NSDragOperationGeneric;
}

- (void) drawRect: (NSRect)rect
{
  NSSize iconSize = GSGetIconSize();
  
  [tileCell drawWithFrame: NSMakeRect(0, 0, iconSize.width, iconSize.height)
  		   inView: self];
  [dragCell drawWithFrame: NSMakeRect(0, 0, iconSize.width, iconSize.height)
		   inView: self];
  
  if ([NSApp isHidden])
    {
      NSRectEdge mySides[] = {NSMinXEdge, NSMinYEdge, NSMaxXEdge, NSMaxYEdge};
      CGFloat myGrays[] = {NSBlack, NSWhite, NSWhite, NSBlack};
      NSDrawTiledRects(NSMakeRect(4, 4, 3, 2), rect, mySides, myGrays, 4);
    }
}

- (id) initWithFrame: (NSRect)frame
{
  self = [super initWithFrame: frame];
  [self registerForDraggedTypes: [NSArray arrayWithObjects:
    NSFilenamesPboardType, nil]];
  return self;
}

- (void) mouseDown: (NSEvent*)theEvent
{
  if ([theEvent clickCount] >= 2)
    {
      /* if not hidden raise windows which are possibly obscured. */
      if ([NSApp isHidden] == NO)
        {
          NSArray *windows = RETAIN(GSOrderedWindows());
          NSWindow *aWin;
          NSEnumerator *iter = [windows reverseObjectEnumerator];
          
          while ((aWin = [iter nextObject]))
            { 
              if ([aWin isVisible] == YES && [aWin isMiniaturized] == NO
                && aWin != [NSApp keyWindow] && aWin != [NSApp mainWindow]
                && aWin != [self window] 
                && ([aWin styleMask] & NSMiniWindowMask) == 0)
                {
                  [aWin orderFrontRegardless];
                }
            }
	
          if ([NSApp isActive] == YES)
            {
              if ([NSApp keyWindow] != nil)
                {
                  [[NSApp keyWindow] orderFront: self];
                }
              else if ([NSApp mainWindow] != nil)
                {
                  [[NSApp mainWindow] makeKeyAndOrderFront: self];
                }
              else
                {
                  /* We need give input focus to some window otherwise we'll 
                     never get keyboard events. FIXME: doesn't work. */
                    NSWindow *menu_window= [[NSApp mainMenu] window];
                    NSDebugLLog(@"Focus",
		      @"No key on activation - make menu key");
                    [GSServerForWindow(menu_window) setinputfocus:
		      [menu_window windowNumber]];
                }
            }
	  
          RELEASE(windows);
        }
      [NSApp unhide: self]; // or activate or do nothing.
    }
  else
    {
      NSPoint	lastLocation;
      NSPoint	location;
      NSUInteger eventMask = NSLeftMouseDownMask | NSLeftMouseUpMask
	| NSPeriodicMask | NSOtherMouseUpMask | NSRightMouseUpMask;
      NSDate	*theDistantFuture = [NSDate distantFuture];
      BOOL	done = NO;

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
	      /* any mouse up means we're done */
		done = YES;
		break;
	      case NSPeriodic:
		location = [_window mouseLocationOutsideOfEventStream];
		if (NSEqualPoints(location, lastLocation) == NO)
		  {
		    NSPoint	origin = [_window frame].origin;

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

- (BOOL) prepareForDragOperation: (id<NSDraggingInfo>)sender
{
  return YES;
}

- (BOOL) performDragOperation: (id<NSDraggingInfo>)sender
{
  NSArray	*types;
  NSPasteboard	*dragPb;

  dragPb = [sender draggingPasteboard];
  types = [dragPb types];
  if ([types containsObject: NSFilenamesPboardType] == YES)
    {
      NSArray	*names = [dragPb propertyListForType: NSFilenamesPboardType];
      NSUInteger index;

      [NSApp activateIgnoringOtherApps: YES];
      for (index = 0; index < [names count]; index++)
	{
	  [NSApp _openDocument: [names objectAtIndex: index]];
	}
      return YES;
    }
  return NO;
}

- (void) setImage: (NSImage *)anImage
{
  NSImage *imgCopy = [anImage copy];

  if (imgCopy)
    {
      NSSize imageSize = [imgCopy size];

      [imgCopy setScalesWhenResized: YES];
      [imgCopy setSize: scaledIconSizeForSize(imageSize)];
    }
  [dragCell setImage: imgCopy];
  RELEASE(imgCopy);
  [self setNeedsDisplay: YES];
}

@end

/**
 * <p>Every graphical GNUstep application has exactly one instance of
 * <code>NSApplication</code> (or a subclass) instantiated.  Usually this is
 * created through the +sharedApplication method.  Once created, this instance
 * is always accessible through the global variable '<code>NSApp</code>'.</p>
 * 
 * <p>The NSApplication instance manages the main run loop, dispatches
 * events, and manages resources.  It sets up the connection to the window
 * server and provides special methods for putting up "modal" (always on top)
 * windows.</p>
 *
 * <p>Typically, -run is called by an application's <code>main</code> method
 * after the NSApplication instance is created, which never returns.  However,
 * applications needing to integrate other event loops may strategically call
 * the -stop: method, followed by -run later on.</p>
 *
 * <p>To avoid most common needs for subclassing, NSApplication allows you to
 * specify a <em>delegate</em> that is messaged in particular situations.
 * See -delegate , -setDelegate: , and [(NSApplicationDelegate)].</p>
 *
 * <p><strong>Subclassing</strong> should be a last resort, and delegate
 * methods should be used in most cases.  However, subclassing is most
 * frequently done to implement custom event loop management by overriding
 * -run when the method described above is not sufficient, or to intercept
 * events by overriding -sendEvent: .</p>
 */
@implementation NSApplication

static BOOL _isAutolaunchChecked = NO;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSApplication class])
    {
      CREATE_AUTORELEASE_POOL(pool);
      /*
       * Dummy functions to fool linker into linking files that contain
       * only catagories - static libraries seem to have problems here.
       */
      extern void	GSStringDrawingDummyFunction(void);

      GSStringDrawingDummyFunction();

      [self setVersion: 1];
     
      /* Cache the NSAutoreleasePool class */
      arpClass = [NSAutoreleasePool class];
      nc = [NSNotificationCenter defaultCenter];
      [pool drain];
    }
}

// Helper method
+ (void) _invokeWithAutoreleasePool: (NSInvocation*) inv
{
  CREATE_AUTORELEASE_POOL(pool);

  [inv invoke];
  [pool drain];
}

/**
 * Calls [NSThread+detachNewThreadSelector:toTarget:withObject:] with the
 * invocation wrapped by an autorelease pool.
 */
+ (void) detachDrawingThread: (SEL)selector
		    toTarget: (id)target
		  withObject: (id)argument
{
  NSInvocation *inv;

  inv = [NSInvocation
            invocationWithMethodSignature: 
                [target methodSignatureForSelector: selector]]; 
  [inv setTarget: target];
  [inv setSelector: selector];
  [inv setArgument: argument atIndex: 2];
  [NSThread detachNewThreadSelector: @selector(_invokeWithAutoreleasePool:) 
	    toTarget: self 
	    withObject: inv];
}

/**
 * <p>Return the shared application instance, creating one (of the
 * receiver class) if needed.  There is (and must always be) only a
 * single shared application instance for each application.  After the
 * shared application instance has been created, you can access it
 * directly via the global variable <code>NSApp</code> (but not before!). When
 * the shared application instance is created, it is also automatically
 * initialized (that is, its <code>init</code> method is called), which
 * connects to the window server and prepares the gui library for actual
 * operation.  For this reason, you must always call <code>[NSApplication
 * sharedApplication]</code> before using any functionality of the gui
 * library - so, normally, this should be one of the first commands in
 * your program (if you use <code>NSApplicationMain()</code>, this is
 * automatically done).</p>
 *
 * <p>The shared application instance is normally an instance of
 * NSApplication; but you can subclass NSApplication, and have an
 * instance of your own subclass be created and used as the shared
 * application instance.  If you want to get this result, you need to
 * make sure the first time you call +sharedApplication is on your
 * custom NSApplication subclass (rather than on NSApplication).
 * Putting <code>[MyApplicationClass sharedApplication]</code>; as the first
 * command in your program is the recommended way. :-) If you use
 * <code>NSApplicationMain()</code>, it automatically creates the appropriate
 * instance (which you can control by editing the info dictionary of
 * the application).</p>
 *
 * <p>It is not safe to call this method from multiple threads - it would
 * be useless anyway since the whole library is not thread safe: there
 * must always be at most one thread using the gui library at a time.
 * (If you absolutely need to have multiple threads in your
 * application, make sure only one of them uses the gui [the 'drawing'
 * thread], and the other ones do not).</p>
 */
+ (NSApplication *) sharedApplication
{
  /* If the global application does not yet exist then create it.  */
  if (NSApp == nil)
    {
      /* -init sets NSApp.  */
      [[self alloc] init];
    }
  return NSApp;
}

/*
 * Instance methods
 */

/**
 * The real gui initialisation ... called from -init
 */
- (void) _init
{
  GSDisplayServer *srv;
  NSDictionary *attributes;
  /* Initialization must be enclosed in an autorelease pool.  */
  CREATE_AUTORELEASE_POOL(_app_init_pool);
 
  /* 
   * Set NSApp as soon as possible, since other gui classes (which
   * we refer or use in this method) might be calling [NSApplication
   * sharedApplication] during their initialization, and we want
   * those calls to succeed.  
   */
  NSApp = self;

  /* Initialize the backend here.  */
  initialize_gnustep_backend();

  /* Load user-defined bundles */
  gsapp_user_bundles();

  /* Connect to our window server.  */
  srv = [GSDisplayServer serverWithAttributes: nil];
  RETAIN(srv);
  [GSDisplayServer setCurrentServer: srv];

  /* Create a default context with the attributes of the main screen.  */
  attributes = [[NSScreen mainScreen] deviceDescription];
  _default_context = [NSGraphicsContext graphicsContextWithAttributes: attributes];
  RETAIN(_default_context);
  [NSGraphicsContext setCurrentContext: _default_context];

  /* Initialize font manager.  */
  [NSFontManager sharedFontManager];

  _hidden = [[NSMutableArray alloc] init];
  _inactive = [[NSMutableArray alloc] init];
  _unhide_on_activation = YES;
  _app_is_hidden = NO;
  /* Ivar already automatically initialized to NO when the app is
     created.  */
  //_app_is_active = NO;
  //_main_menu = nil;
  _windows_need_update = YES;

  /* Save the base library exception handler */
  defaultUncaughtExceptionHandler = NSGetUncaughtExceptionHandler();
  /* Set a new exception handler for the gui library.  */
  NSSetUncaughtExceptionHandler(_NSAppKitUncaughtExceptionHandler);

  _listener = [GSServicesManager newWithApplication: self];

  /* NSEvent doesn't use -init so we use +alloc instead of +new.  */
  _current_event = [NSEvent alloc]; // no current event
  null_event = [NSEvent alloc];    // create dummy event

  /* We are the end of responder chain.  */
  [self setNextResponder: nil];

  /* Create our app icon.
     NB We are doing this here because WindowMaker will not map the app icon
     window unless it is the very first window being mapped. */
  [self _appIconInit];

  [_app_init_pool drain];
}


/** 
 * This method initializes an NSApplication instance.  It sets the
 * shared application instance to be the receiver, and then connects
 * to the window server and performs the actual gui library
 * initialization.
 *
 * If there is a already a shared application instance, calling this
 * method results in an assertion (and normally program abortion/crash).
 *
 * It is recommended that you /never/ call this method directly from
 * your code!  It's called automatically (and only once) by
 * [NSApplication sharedApplication].  You might override this method
 * in subclasses (make sure to call super's :-), then your overridden
 * method will automatically be called (guaranteed once in the
 * lifetime of the application) when you call [MyApplicationClass
 * sharedApplication].
 *
 * If you call this method from your code (which we discourage you
 * from doing), it is <em>your</em> responsibility to make sure it is called
 * only once (this is according to the openstep specification).  Since
 * +sharedApplication automatically calls this method, making also
 * sure it calls it only once, you definitely want to use
 * +sharedApplication instead of calling -init directly.  
 */
- (id) init
{
  /*
   * As per openstep specification, calling -init twice is a bug in
   * the program.  +sharedApplication automatically makes sure it
   * never calls -init more than once, and programmers should normally
   * use +sharedApplication in programs.
   *
   * Please refrain from trying to have this method work with multiple
   * calls (such as returning NSApp instead of raising an assertion).
   * No matter what you do, you can't protect subclass -init custom
   * code from multiple executions by changing the implementation here
   * - so it's just simpler and cleaner that multiple -init executions
   * are always forbidden, and subclasses inherit exactly the same
   * kind of multiple execution protection as the superclass has, and
   * initialization code behaves always in the same way for this class
   * and for subclasses.
   */
  NSAssert (NSApp == nil, _(@"[NSApplication -init] called more than once"));

  /*
   * The appkit should run in the main thread ... so to be sure we perform
   * all the initialisation there.
   */
  [self performSelectorOnMainThread: @selector(_init)
			 withObject: self
		      waitUntilDone: YES];
  return NSApp;
}

/**
 * <p>Activates the application, sets the application icon, loads the main
 * Nib file if <code>NSMainNibFile</code> is set in the application
 * property list, posts an
 * <code>NSApplicationWillFinishLaunchingNotification</code>, and takes care
 * of a few other startup tasks.
 * If you override this method, be sure to call <em>super</em>.</p>
 *
 * <p>The -run method calls this the first time it is called, before starting
 * the event loop for the first time.</p>
 */
- (void) finishLaunching
{
  NSDocumentController	*sdc;
  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
  NSString		*filePath;
  NSArray		*windows_list;
  NSUInteger		count;
  NSUInteger		i;
  BOOL			hadDuplicates = NO;
  BOOL			didAutoreopen = NO;
  NSArray               *files = nil;

  /* post notification that launch will finish */
  [nc postNotificationName: NSApplicationWillFinishLaunchingNotification
      object: self];

  /* Register our listener to incoming services requests etc. */
  [_listener registerAsServiceProvider];

  /*
   * Establish the current key and main windows.  We need to do this in case
   * the windows were created and set to be key/main earlier - before the
   * app was active.
   */
  windows_list = [self windows];
  count = [windows_list count];
  for (i = 0; i < count; i++)
    {
      NSWindow	*win = [windows_list objectAtIndex: i];

      if ([win isKeyWindow] == YES)
	{
	  if (_key_window == nil)
	    {
	      _key_window = win;
	    }
	  else
	    {
	      hadDuplicates = YES;
	      NSDebugLog(@"Duplicate keyWindow ignored");
	      [win resignKeyWindow];
	    }
	}
      if ([win isMainWindow] == YES)
	{
	  if (_main_window == nil)
	    {
	      _main_window = win;
	    }
	  else
	    {
	      hadDuplicates = YES;
	      NSDebugLog(@"Duplicate mainWindow ignored");
	      [win resignMainWindow];
	    }
	}
    }

  /*
   * If there was more than one window set as key or main, we must make sure
   * that the one we have recorded is the real one by making it become key/main
   * again.
   */
  if (hadDuplicates)
    {
      [_main_window resignMainWindow];
      [_main_window becomeMainWindow];
      [_main_window orderFrontRegardless];
      [_key_window resignKeyWindow];
      [_key_window becomeKeyWindow];
      [_key_window orderFrontRegardless];
    }

  // Make sure there is one designated main window
  if (_main_window == nil)
    {
      for (i = 0; i < count; i++)
        {
          NSWindow	*win = [windows_list objectAtIndex: i];

          if ([win isVisible] && [win canBecomeMainWindow])
            {
              _main_window = win;
              break;
            }
        }
    }

  /* Register self as observer to window events. */
  [nc addObserver: self selector: @selector(_windowWillClose:)
      name: NSWindowWillCloseNotification object: nil];
  [nc addObserver: self selector: @selector(_windowDidBecomeKey:)
      name: NSWindowDidBecomeKeyNotification object: nil];
  [nc addObserver: self selector: @selector(_windowDidBecomeMain:)
      name: NSWindowDidBecomeMainNotification object: nil];
  [nc addObserver: self selector: @selector(_windowDidResignKey:)
      name: NSWindowDidResignKeyNotification object: nil];
  [nc addObserver: self selector: @selector(_windowDidResignMain:)
      name: NSWindowDidResignMainNotification object: nil];

  /* register as observer for hide/unhide notifications */
  [[[NSWorkspace sharedWorkspace] notificationCenter]
    addObserver: self selector: @selector(_workspaceNotification:)
      name: GSHideOtherApplicationsNotification object: nil];
  [[[NSWorkspace sharedWorkspace] notificationCenter]
    addObserver: self selector: @selector(_workspaceNotification:)
      name: GSUnhideAllApplicationsNotification object: nil];

  // Don't activate the application, when the delegate hid it
  if (![self isHidden])
    {
      [self activateIgnoringOtherApps: YES];
    }

  /*
   * Instantiate the NSDocumentController if we are a doc-based app
   * and eventually reopen all autosaved documents
   */
  sdc = [NSDocumentController sharedDocumentController];
  if ([[sdc documentClassNames] count] > 0)
    {
      didAutoreopen = [sdc _reopenAutosavedDocuments];
    }

  /*
   *	Now check to see if we were launched with arguments asking to
   *	open a file.  We permit some variations on the default name.
   */

  if ((files = [self _openFiles]) != nil)
    {
      [_listener application: self openFiles: files];
    } 
  else if ((filePath = [defs stringForKey: @"GSFilePath"]) != nil
    || (filePath = [defs stringForKey: @"NSOpen"]) != nil)
    {
      [_listener application: self openFile: filePath];
    }
  else if ((filePath = [defs stringForKey: @"GSTempPath"]) != nil)
    {
      [_listener application: self openTempFile: filePath];
    }
  else if ((filePath = [defs stringForKey: @"NSPrint"]) != nil)
    {
      [_listener application: self printFile: filePath];
      [self terminate: self];
    }
  else if (!didAutoreopen && ![defs boolForKey: @"autolaunch"])
    {
      // For document based applications we automatically open a fresh document
      // unless denied by the delegate. For non-document based applications we
      // open a fresh document only when requested by the delegate.
      // Note: We consider an application document based if the shared document
      // controller reports at least one editable type.
      BOOL docBased =
	[[sdc documentClassNames] count] > 0 && [sdc defaultType] != nil;
      BOOL shouldOpen = docBased ? YES : NO;

      if ([_delegate respondsToSelector:
                       @selector(applicationShouldOpenUntitledFile:)])
        {
	  shouldOpen = [_delegate applicationShouldOpenUntitledFile: self];
	}
      if (shouldOpen)
	{
	  if (docBased)
	    {
	      NSError *err = nil;
	      if ([sdc openUntitledDocumentAndDisplay: YES error: &err] == nil)
		{
		  [sdc presentError: err];
		}
	    }
	  else if ([_delegate respondsToSelector:
                                @selector(applicationOpenUntitledFile:)])
	    {
	      [_delegate applicationOpenUntitledFile: self];
	    }
        }
    }
}

/*
 * Posts <code>NSApplicationDidFinishLaunchingNotification</code>.
 *
 * <p>The -run method calls this the first time it is called, before starting
 * the event loop for the first time and after calling finishLaunching.</p>
 */  
- (void) _didFinishLaunching
{
  /* finish the launching post notification that launching has finished */
  [nc postNotificationName: NSApplicationDidFinishLaunchingNotification
		    object: self];

  NS_DURING
    {
      [[[NSWorkspace sharedWorkspace] notificationCenter]
          postNotificationName: NSWorkspaceDidLaunchApplicationNotification
          object: [NSWorkspace sharedWorkspace]
          userInfo: [self _notificationUserInfo]];
    }
  NS_HANDLER
    {
      NSLog (_(@"Problem during launch app notification: %@"),
             [localException reason]);
      [localException raise];
    }
  NS_ENDHANDLER
}

- (void) dealloc
{
  GSDisplayServer *srv = GSServerForWindow(_app_icon_window);

  if (srv == nil)
    {
      srv = GSCurrentServer();
    }
  [[[NSWorkspace sharedWorkspace] notificationCenter]
    removeObserver: self];
  [nc removeObserver: self];

  RELEASE(_hidden);
  RELEASE(_inactive);
  RELEASE(_listener);
  RELEASE(null_event);
  RELEASE(_current_event);

  /* We may need to tidy up nested modal session structures. */
  while (_session != 0)
    {
      NSModalSession tmp = _session;

      _session = tmp->previous;
      NSZoneFree(NSDefaultMallocZone(), tmp);
    }

  /* Release the menus, then set them to nil so we don't try updating
     them after they have been deallocated.  */
  DESTROY(_main_menu);
  DESTROY(_windows_menu);

  TEST_RELEASE(_app_icon);
  TEST_RELEASE(_app_icon_window);
  TEST_RELEASE(_infoPanel);

  /* Destroy the default context */
  [NSGraphicsContext setCurrentContext: nil];
  DESTROY(_default_context);

  /* Close the server */
  [srv closeServer];
  DESTROY(srv);

  [super dealloc];
}

/**
 * Activate app unconditionally if flag is YES, otherwise only if no other app
 * is active.  (<em><strong>Note:</strong> this is currently not implemented
 * under GNUstep.  The app is always activated unconditionally.</em>)  Usually
 * it is not necessary to manually call this method, except in some
 * circumstances of interapplication communication.
 */
- (void) activateIgnoringOtherApps: (BOOL)flag
{
  if (_isAutolaunchChecked == NO)
    {
      NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
      NSString       *autolaunch = [defaults objectForKey: @"autolaunch"];

      _isAutolaunchChecked = YES;
      
      /* Application was executed with an argument '-autolaunch YES'.
         Do not activate application on first call. */
      if (autolaunch && [autolaunch isEqualToString: @"YES"])
        {
          return;
        }
    }
  
  // TODO: Currently the flag is ignored
  if (_app_is_active == NO)
    {
      NSUInteger	count;
      NSUInteger	i;
      NSDictionary	*info;

     /*
       * Menus should observe this notification in order to make themselves
       * visible when the application is active.
       */
      [nc postNotificationName: NSApplicationWillBecomeActiveNotification
          object: self];

      _app_is_active = YES;

      if ([[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSSuppressAppIcon"])
	{
	  [_app_icon_window orderOut: self];
	}

      /* Make sure to calculate count after the notification, since
         inactive status might be changed by a notifiee.  */
      count = [_inactive count];
      for (i = 0; i < count; i++)
        {
          [[_inactive objectAtIndex: i] orderFrontRegardless];
        }
      [_inactive removeAllObjects];

      if (_unhide_on_activation)
        {
          [self unhide: nil];
        }
     
      if ([self keyWindow] == nil && _hidden_key != nil
          && [[self windows] indexOfObjectIdenticalTo: _hidden_key] != NSNotFound)
        {
          [_hidden_key makeKeyWindow];
          _hidden_key = nil;
        }

      if ([self mainWindow] == nil && _hidden_main != nil
          && [[self windows] indexOfObjectIdenticalTo: _hidden_main] != NSNotFound)
        {
          [_hidden_main makeMainWindow];
          _hidden_main = nil;
        }
      
      if ([self keyWindow] != nil)
        {
          [[self keyWindow] orderFront: self];
        }
      else if ([self mainWindow] != nil)
        {
          [[self mainWindow] makeKeyAndOrderFront: self];
        }
      else
        {
          /* We need give input focus to some window otherwise we'll never get
             keyboard events. FIXME: doesn't work. */
          NSWindow *menu_window= [[self mainMenu] window];
          NSDebugLLog(@"Focus", @"No key on activation - make menu key");
          [GSServerForWindow(menu_window) setinputfocus: 
                                [menu_window windowNumber]];
        }

      info = [self _notificationUserInfo];
      [nc postNotificationName: NSApplicationDidBecomeActiveNotification
          object: self
		      userInfo: info];
      [[[NSWorkspace sharedWorkspace] notificationCenter]
          postNotificationName: NSApplicationDidBecomeActiveNotification
		      object: [NSWorkspace sharedWorkspace]
          userInfo: info];
    }
}

/**
 * Forcefully deactivate the app, without activating another.  It is rarely
 * necessary to use this method.
 */
- (void) deactivate
{
  if (_app_is_active == YES)
    {
      NSArray		*windows_list;
      NSDictionary	*info;
      NSWindow		*win;
      NSEnumerator	*iter;

      [nc postNotificationName: NSApplicationWillResignActiveNotification
          object: self];
      
      _app_is_active = NO;

      if ([self keyWindow] != nil)
        {
          _hidden_key = [self keyWindow];
          [_hidden_key resignKeyWindow];
        }
      // The main window is saved for when the app is activated again.
      // This is necessary for menu in window.
      if ([self mainWindow] != nil)
        {
          _hidden_main = [self mainWindow];
          [_hidden_main resignMainWindow];
        }
      
      windows_list = GSOrderedWindows();
      iter = [windows_list reverseObjectEnumerator];

      while ((win = [iter nextObject]))
        {
          NSModalSession theSession;
	   
          if ([win isVisible] == NO)
            {
              continue;		/* Already invisible	*/
            }
          if ([win canHide] == NO)
            {
              continue;		/* Can't be hidden	*/
            }
          if (win == _app_icon_window)
            {
              continue;		/* can't hide the app icon.	*/
            }
          /* Don't order out modal windows */
          theSession = _session;
          while (theSession != 0)
            {
              if (win == theSession->window)
                break;
              theSession = theSession->previous;
            }
          if (theSession)
            continue;
          
          if ([win hidesOnDeactivate] == YES)
            {
	      /* NB Order is important here. When a hide on deactivate window
		 is ordered out while the application is inactive, it gets
		 removed from the _inactive list. Therefore, we must first
		 order out the window and then add it to the _inactive list. */
              [win orderOut: self];
              [_inactive addObject: win];
            }
        }
      
      if (YES == [[NSUserDefaults standardUserDefaults]
	boolForKey: @"GSSuppressAppIcon"])
	{
#if	MINI_ICON
	  NSRect	f = [[[self mainMenu] window] frame];
	  NSPoint	p = f.origin;

	  p.y += f.size.height;
          [_app_icon_window setFrameTopLeftPoint: p];
	  [_app_icon_window orderFrontRegardless];
          [_app_icon_window miniaturize: self];
#else
	  [_app_icon_window orderFrontRegardless];
#endif
	}

      info = [self _notificationUserInfo];
      [nc postNotificationName: NSApplicationDidResignActiveNotification
          object: self
		      userInfo: info];
      [[[NSWorkspace sharedWorkspace] notificationCenter]
          postNotificationName: NSApplicationDidResignActiveNotification
		      object: [NSWorkspace sharedWorkspace]
          userInfo: info];
    }
}

/**
 * Returns whether this app is the currently active GNUstep application.
 * Note that on a GNUstep system, unlike OS X, it is possible for NO GNUstep
 * app to be active.
 */
- (BOOL) isActive
{
  return _app_is_active;
}

/**
 * Cause all other apps to hide themselves.
 */
- (void) hideOtherApplications: (id)sender
{
  [[[NSWorkspace sharedWorkspace] notificationCenter]
    postNotificationName: GSHideOtherApplicationsNotification
		  object: [NSWorkspace sharedWorkspace]
		userInfo: [self _notificationUserInfo]];
}

/**
 * Cause all apps including this one to unhide themselves.
 */
- (void) unhideAllApplications: (id)sender
{
  [[[NSWorkspace sharedWorkspace] notificationCenter]
    postNotificationName: GSUnhideAllApplicationsNotification
		  object: [NSWorkspace sharedWorkspace]
		userInfo: [self _notificationUserInfo]];
}

#define NSLogUncaughtExceptionMask 1
#define NSHandleUncaughtExceptionMask 2
#define NSLogUncaughtSystemExceptionMask 4
#define NSHandleUncaughtSystemExceptionMask 8
#define NSLogRuntimeErrorMask 16
#define NSLogUncaughtRuntimeErrorMask 32

/**
 * Private method to handle an exception which occurs in the application runloop.
 */
- (void) _handleException: (NSException *)exception
{
  NSInteger mask = [[NSUserDefaults standardUserDefaults] integerForKey: @"NSExceptionHandlerMask"];

  /**
   * If we are in debug mode, then rethrow the exception so that
   * the application can be stopped.
   **/

  // log the exception.
  if (mask & NSLogUncaughtExceptionMask) 
    {
      [self reportException: exception];
    }

  // allow the default handler to handle the exception.
  if (mask & NSHandleUncaughtExceptionMask) 
    {
      [exception raise];
    }
}

/*
 * Running the main event loop
 */

/**
 * <p>This method first calls -finishLaunching (if this is the first time -run)
 * has been called, then starts the main event loop of the application which
 * continues until -terminate: or -stop: is called.</p>
 *
 * <p>At each iteration, at most one event is dispatched, the main and services
 * menus are sent [NSMenu-update] messages, and -updateWindows is possibly
 * called if this has been flagged as necessary.</p>
 */
- (void) run
{
  NSEvent *e;
  id distantFuture = [NSDate distantFuture];     /* Cache this, safe */

  if (_runLoopPool != nil)
    {
      [NSException raise: NSInternalInconsistencyException
		   format: @"NSApp's run called recursively"];
    }

  /*
   *  Set this flag here in case the application is actually terminated
   *  inside -finishLaunching.
   */
  _app_is_running = YES;

  if (_app_is_launched == NO)
    {
      _app_is_launched = YES;
      IF_NO_GC(_runLoopPool = [arpClass new]);

      [self finishLaunching];
      [self _didFinishLaunching];

      [_listener updateServicesMenu];
      [_main_menu update];
      DESTROY(_runLoopPool);
    }
 
  while (_app_is_running)
    {
      IF_NO_GC(_runLoopPool = [arpClass new]);

      // Catch and report any uncaught exceptions.
      NS_DURING
	{	  	  
	  e = [self nextEventMatchingMask: NSAnyEventMask
				untilDate: distantFuture
				   inMode: NSDefaultRunLoopMode
				  dequeue: YES];

	  if (e != nil)
	    {
	      NSEventType	type = [e type];

	      [self sendEvent: e];

	      // update (en/disable) the services menu's items
	      if (type != NSPeriodic && type != NSMouseMoved)
		{
		  [_listener updateServicesMenu];
		  [_main_menu update];
		}
	    }
	}
      NS_HANDLER
	{
	  [self _handleException: localException];
	}
      NS_ENDHANDLER;

      DESTROY (_runLoopPool);
    }

  /* Every single non trivial line of code must be enclosed into an
     autorelease pool.  Create an autorelease pool here to wrap
     synchronize and the NSDebugLog.  */
  IF_NO_GC(_runLoopPool = [arpClass new]);

  [[NSUserDefaults standardUserDefaults] synchronize];
  DESTROY (_runLoopPool);
}

/**
 *  Returns whether the event loop managed by -run is currently active.
 */
- (BOOL) isRunning
{
  return _app_is_running;
}

/*
 * Running modal event loops
 */

/**
 * Halts a currently running modal event loop started by -runModalForWindow:
 * or -runModalSession: .  If you wish to halt the session in response to
 * user interaction with the modal window, use -stopModalWithCode: or
 * -stopModal instead; only use this to halt the loop from elsewhere, such as
 * another thread.
 */
- (void) abortModal
{
/* FIXME: The previous, now commented out, code here did only work from within the modal loop, 
   which is contrary to the purpose of this method. Calling stopModalWithCode: works a bit better,
   but still relies on the modal loop to cooperate. Calling that method via performSelectorOnMainThread:...
   and moving the exception into stopModalWithCode:, looks like another option. Still this would 
   rely on the loop getting executed.

  if (_session == 0)
    {
      [NSException raise: NSAbortModalException
		  format: @"abortModal called while not in a modal session"];
    }
  [NSException raise: NSAbortModalException format: @"abortModal"];
*/
  [self stopModalWithCode: NSRunAbortedResponse];
}

/**
 * Set up modal session for theWindow, and, if it is not visible already,
 * puts it up on screen, centering it if it is an NSPanel.  It is then
 * ordered front and made key or main window.
 */
- (NSModalSession) beginModalSessionForWindow: (NSWindow*)theWindow
{
  NSModalSession theSession;

  theSession = (NSModalSession)NSZoneMalloc(NSDefaultMallocZone(),
		    sizeof(struct _NSModalSession));
  theSession->runState = NSRunContinuesResponse;
  theSession->entryLevel = [theWindow level];
  theSession->window = theWindow;
  theSession->previous = _session;
  _session = theSession;

  /*
   * Displaying / raising window but centering panel only if not up
   * seems to match the behavior on OS X (Panther).
   */
  if ([theWindow isKindOfClass: [NSPanel class]])
    {
      if ([theWindow isVisible] == NO)
          [theWindow center];
      [theWindow setLevel: NSModalPanelWindowLevel];
    }
  [theWindow orderFrontRegardless];
  if ([self isActive] == YES)
    {
      if ([theWindow canBecomeKeyWindow] == YES)
	{
	  [theWindow makeKeyWindow];
	}
      else if ([theWindow canBecomeMainWindow] == YES)
	{
	  [theWindow makeMainWindow];
	}
    }

  return theSession;
}

/**
 * Clean up after a modal session has been run.  Called with theSession
 * returned from a previous call to beginModalSessionForWindow: .
 */
- (void) endModalSession: (NSModalSession)theSession
{
  NSModalSession	tmp = _session;
  NSArray		*windows = [self windows];

  if (theSession == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"null pointer passed to endModalSession:"];
    }
  /* Remove this session from linked list of sessions. */
  while (tmp != 0 && tmp != theSession)
    {
      tmp = tmp->previous;
    }
  if (tmp == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"unknown session passed to endModalSession:"];
    }
  while (_session != theSession)
    {
      tmp = _session;
      _session = tmp->previous;
      if ([windows indexOfObjectIdenticalTo: tmp->window] != NSNotFound)
	{
	  [tmp->window setLevel: tmp->entryLevel];
	}
      NSZoneFree(NSDefaultMallocZone(), tmp);
    }
  _session = _session->previous;
  if ([windows indexOfObjectIdenticalTo: theSession->window] != NSNotFound)
    {
      [theSession->window setLevel: theSession->entryLevel];
    }
  NSZoneFree(NSDefaultMallocZone(), theSession);

  // Bring the next modal window to front
  if ((_session != 0) && 
      ([windows indexOfObjectIdenticalTo: _session->window] != NSNotFound))
    {
      NSWindow *theWindow = _session->window;

      // Same code as in beginModalSessionForWindow:
      [theWindow orderFrontRegardless];      
      if ([self isActive] == YES)
        {
          if ([theWindow canBecomeKeyWindow] == YES)
            {
              [theWindow makeKeyWindow];
            }
          else if ([theWindow canBecomeMainWindow] == YES)
            {
              [theWindow makeMainWindow];
            }
        }
    }
  else
    {
      [_main_menu update];
    }
}

/**
 * Starts modal event loop for given window, after calling
 * -beginModalSessionForWindow:.  Loop is broken only by -stopModal ,
 * -stopModalWithCode: , or -abortModal , at which time -endModalSession:
 * is called automatically.
 */
- (NSInteger) runModalForWindow: (NSWindow*)theWindow
{
  NSModalSession theSession = 0;
  NSInteger code = NSRunContinuesResponse;

  NS_DURING
    {
      NSDate		*limit;
      GSDisplayServer	*srv;

      theSession = [self beginModalSessionForWindow: theWindow];
      limit = [NSDate distantFuture];
      srv = GSCurrentServer();

      while (code == NSRunContinuesResponse)
	{
	  /*
	   * Try to handle events for this session, discarding others.
	   */
	  code = [self runModalSession: theSession];
	  if (code == NSRunContinuesResponse)
	    {
	      /*
	       * Wait until there are more events to handle.
	       */
	      DPSPeekEvent(srv, NSAnyEventMask, limit, NSModalPanelRunLoopMode);
	    }
	}

      [self endModalSession: theSession];
    }
  NS_HANDLER
    {
      if (theSession != 0)
	{
	  NSWindow *win_to_close = theSession->window;
	  
	  [self endModalSession: theSession];
	  [win_to_close close];
	}
      if ([[localException name] isEqual: NSAbortModalException] == NO)
	{
	  [localException raise];
     	} 
      code = NSRunAbortedResponse;
    }
  NS_ENDHANDLER

  return code;
}

/** 
<p>
Processes any events for a modal session described by the theSession
variable. When finished, it returns the state of the session (i.e.
whether it is still running or has been stopped, etc) 
</p>
<p>If there are no pending events for the session, this method returns
immediately.
</p>
<p>
 Although Apple's docs state that, before processing the events, it makes the
 session window key and orders the window front, this method does not attempt
 to do this, because: 1) we don't want to interfere with use of other apps
 during modal session for this app; 2) occasionally other windows are active
 and should be usable during modal sessions (e.g., a popup dialog from a modal
 window); 3) most of the time -beginModalSessionForWindow: will have been
 called in advance.  If the latter is not the case, you may need to order the
 window front yourself in advance.
</p>
<p>
See Also: -runModalForWindow:
</p>
*/
- (NSInteger) runModalSession: (NSModalSession)theSession
{
  NSAutoreleasePool	*pool;
  GSDisplayServer	*srv;
  BOOL		done = NO;
  NSEvent	*event;
  NSDate	*limit;
  
  if (theSession != _session)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"runModalSession: with wrong session"];
    }

  // Use the default context for all events.
  srv = GSCurrentServer();

  // Only handle input which is already available.
  limit = [NSDate distantPast];

  /*
   *	Deal with the events in the queue.
   */
  while (done == NO && theSession->runState == NSRunContinuesResponse)
    {
      IF_NO_GC(pool = [arpClass new]);

      event = DPSGetEvent(srv, NSAnyEventMask, limit, NSModalPanelRunLoopMode);
      if (event != nil)
	{
	  NSWindow	*eventWindow = [event window];

	  /*
	   * We handle events for the session window, events for any
	   * window which works when modal, and any window management
	   * events.  All others are ignored/discarded.
	   */
	  if (eventWindow == theSession->window
	    || [eventWindow worksWhenModal] == YES
	    || [event type] == NSAppKitDefined)
	    {
	      ASSIGN(_current_event, event);
	    }
	  else
	    {
	      event = nil;	// Ignore/discard this event.
	    }
	}
      else
	{
	  done = YES;		// No more events pending.
	}

      if (event != nil)
	{
	  NSEventType	type = [_current_event type];

	  [self sendEvent: _current_event];

	  // update (en/disable) the services menu's items
	  if (type != NSPeriodic && type != NSMouseMoved)
	    {
	      [_listener updateServicesMenu];
	      [_main_menu update];
	    }

	  /*
	   *	Check to see if the window has gone away - if so, end session.
	   */
	  if ([[self windows] indexOfObjectIdenticalTo: _session->window]
	    == NSNotFound
            || ![_session->window isVisible])
	    {
	      [self stopModal];
	    }
	}
      [pool drain];
    }

  NSAssert(_session == theSession, @"Session was changed while running");

  return theSession->runState;
}

/**
<p>
   Returns the window that is part of the current modal session, if any.
</p>
<p>
See -runModalForWindow:
</p>
*/
- (NSWindow *) modalWindow
{
  if (_session != 0) 
    return (_session->window);
  else
    return nil;
}

/**
 * Stops the main run loop, as well as a modal session if it is running.
 */
- (void) stop: (id)sender
{
  if (_session != 0)
    [self stopModal];
  else
    {
      _app_is_running = NO;
      /*
       * add dummy event to queue to assure loop cycles
       * at least one more time
       */
      DPSPostEvent(GSCurrentServer(), null_event, NO);
    }
}

/**<p> Stops a running modal session causing -runModalForWindow: or
 * -runModalSession: to return <code>NSRunStoppedResponse</code>.  Use this
 * or -stopModalWithCode: to end a modal session in response to user input.</p>
 * <p>See Also: -stopModalWithCode:</p>
 */
- (void) stopModal
{
  [self stopModalWithCode: NSRunStoppedResponse];
}

/**
 * Stops a running modal session causing -runModalForWindow: or
 * -runModalSession: to return the specified integer code.  Use this
 * or -stopModal to end a modal session in response to user input.
 */
- (void) stopModalWithCode: (NSInteger)returnCode
{
  /* According to the spec, there is no exception which is thrown
   * if we are not currently in a modal session.   While it is not
   * good practice to call this when we're not, we shouldn't throw
   * an exception.
   */
  if (_session != 0 && _session->runState == NSRunContinuesResponse)
    {
      if (returnCode == NSRunContinuesResponse)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"stopModalWithCode: with NSRunContinuesResponse"];
	}
      _session->runState = returnCode;
    }
}

/**
 * Put up a modal window centered relative to docWindow.  On OS X this is
 * deprecated in favor of
 * -beginSheet:modalForWindow:modalDelegate:didEndSelector:contextInfo: .
 */
- (NSInteger) runModalForWindow: (NSWindow *)theWindow
               relativeToWindow: (NSWindow *)docWindow
{
  if ((docWindow != nil) && (theWindow != nil))
    {
      NSRect  docFrame = [docWindow frame];
      NSPoint point = docFrame.origin;
      NSRect  theFrame = [theWindow frame];
      NSSize  size  = theFrame.size;
  
      // Calculate window position...
      point.x += (docFrame.size.width - size.width) / 2;
      point.y += (docFrame.size.height - size.height) / 2;

      NSDebugLLog(@"NSWindow", @"Positioning window %@ relative to %@ at %@", 
            NSStringFromRect(theFrame), NSStringFromRect(docFrame), NSStringFromPoint(point));
      // Position window...
      [theWindow setFrameOrigin: point];
    }
  [theWindow orderWindow: NSWindowAbove
	     relativeTo: [docWindow windowNumber]];
  return [self runModalForWindow: theWindow];
}

/**
 * Put up a modal sheet sliding down from top of docWindow.  If modalDelegate
 * is non-nil and responds to didEndSelector (this is optional), it is invoked
 * after the session ends.  The selector should take three arguments:
 * NSWindow *, int, void *.  It is passed the sheet window, the return code,
 * and the contextInfo passed in here.

 * <em>Under GNUstep, the sheet aspect is not implemented (just centers
 * window on the screen), but modalDelegate didEndSelector is called if
 * both non-nil.</em>
 */
- (void) beginSheet: (NSWindow *)sheet
     modalForWindow: (NSWindow *)docWindow
      modalDelegate: (id)modalDelegate
     didEndSelector: (SEL)didEndSelector
	contextInfo: (void *)contextInfo
{
  // FIXME
  NSInteger ret;

  [sheet setParentWindow: docWindow];
  [docWindow setAttachedSheet: sheet];

  [[NSNotificationCenter defaultCenter] 
          postNotificationName: NSWindowWillBeginSheetNotification
                        object: docWindow];
  ret = [self runModalForWindow: sheet 
	      relativeToWindow: docWindow];

  if (modalDelegate && [modalDelegate respondsToSelector: didEndSelector])
    {
      void (*didEnd)(id, SEL, id, NSInteger, void*);

      didEnd = (void (*)(id, SEL, id, NSInteger, void*))[modalDelegate methodForSelector: 
								 didEndSelector];
      didEnd(modalDelegate, didEndSelector, sheet, ret, contextInfo);
    }

  [sheet close];
  [docWindow setAttachedSheet: nil];
  [sheet setParentWindow: nil];
  [[NSNotificationCenter defaultCenter] 
          postNotificationName: NSWindowDidEndSheetNotification
                        object: docWindow];
}

/**
 *  Analogous to -stopModal for sheets.
 */
- (void) endSheet: (NSWindow *)sheet
{
  // FIXME
  [self stopModal];
}

/**
 *  Analogous to -stopModalWithCode: for sheets.
 */
- (void) endSheet: (NSWindow *)sheet
       returnCode: (NSInteger)returnCode
{
  // FIXME
  [self stopModalWithCode: returnCode];
}


/*
 * Getting, removing, and posting events
 */

/* Private method used by GSDragView to dispatch drag events as Cocoa does. */
- (void) _postAndSendEvent: (NSEvent *)anEvent
{
  ASSIGN(_current_event, anEvent);
  [self sendEvent: anEvent];
}

/**
 * Called by -run to dispatch events that are received according to AppKit's
 * forwarding conventions.  You rarely need to invoke this directly.  If you
 * want to synthesize an event for processing, call -postEvent:atStart: .
 */
- (void) sendEvent: (NSEvent *)theEvent
{
  NSEventType type;

  type = [theEvent type];
  switch (type)
    {
      case NSPeriodic:	/* NSApplication traps the periodic events	*/
	break;

      case NSKeyDown:
	{
	  NSDebugLLog(@"NSEvent", @"send key down event\n");
	  /* Key equivalents must be looked up explicitly in the Services menu
	     after checking the main menu, as NSMenu's -performKeyEquivalent:
	     ignores the Services menu. See the comment in that method for a
	     rationale. */
	  if ([[self keyWindow] performKeyEquivalent: theEvent] == NO
	    && [[self mainMenu] performKeyEquivalent: theEvent] == NO
	    && [[self servicesMenu] performKeyEquivalent: theEvent] == NO)
	    {
	      [[theEvent window] sendEvent: theEvent];
	    }
	  break;
	}

      case NSKeyUp:
	{
	  NSDebugLLog(@"NSEvent", @"send key up event\n");
	  [[theEvent window] sendEvent: theEvent];
	  break;
	}

      default:	/* pass all other events to the event's window	*/
	{
	  NSWindow *window = [theEvent window];

	  if (!theEvent)
	    NSDebugLLog(@"NSEvent", @"NSEvent is nil!\n");
	  if (type == NSMouseMoved)
	    NSDebugLLog(@"NSMotionEvent", @"Send move (%d) to %@", 
			(int)type, window);
	  else
	    NSDebugLLog(@"NSEvent", @"Send NSEvent type: %@ to %@", 
			theEvent, window);
	  if (window)
	    [window sendEvent: theEvent];
	  else if (type == NSRightMouseDown)
	    [self rightMouseDown: theEvent];
	}
    }
}

/**
 * Returns the most recent event -run pulled off the event queue.
 */
- (NSEvent*) currentEvent
{
  return _current_event;
}

/* Utility for pen-device input.  See NSResponder. */
- (BOOL) shouldBeTreatedAsInkEvent: (NSEvent *)theEvent
{
  return [[theEvent window] shouldBeTreatedAsInkEvent: theEvent];
}

/**
 * Drop events matching mask from the queue, before but not including
 * lastEvent.  The mask is a bitwise AND of event mask constants.
 * See (EventType) .  Use <code>NSAnyEventMask</code> to discard everything
 * up to lastEvent.
 */
- (void) discardEventsMatchingMask: (NSUInteger)mask
		       beforeEvent: (NSEvent *)lastEvent
{
  DPSDiscardEvents(GSCurrentServer(), mask, lastEvent);
}

/**
 * Return the next event matching mask from the queue, dequeuing if flag is
 * YES.  Intervening events are NOT dequeued.  If no matching event is on the
 * queue, will wait for one until expiration, returning nil if none found.
 * See (EventType) for the list of masks.
 */
- (NSEvent*) nextEventMatchingMask: (NSUInteger)mask
			 untilDate: (NSDate*)expiration
			    inMode: (NSString*)mode
			   dequeue: (BOOL)flag
{
  NSEvent	*event;

  if (_windows_need_update)
    {
      [self updateWindows];
    }
  if (!expiration)
    expiration = [NSDate distantPast];

  if (flag)
    event = DPSGetEvent(GSCurrentServer(), mask, expiration, mode);
  else
    event = DPSPeekEvent(GSCurrentServer(), mask, expiration, mode);

  if (event == null_event)
    {
      // Never return the null_event
      event = nil;
    }

  if (event)
    {
IF_NO_GC(NSAssert([event retainCount] > 0, NSInternalInconsistencyException));
      /*
       * If we are not in a tracking loop, we may want to unhide a hidden
       * because the mouse has been moved.
       */
      if (mode != NSEventTrackingRunLoopMode)
	{
	  _windows_need_update = YES;
	  if ([NSCursor isHiddenUntilMouseMoves])
	    {
	      NSEventType type = [event type];

	      if ((type == NSLeftMouseDown) || (type == NSLeftMouseUp)
		|| (type == NSOtherMouseDown) || (type == NSOtherMouseUp)
		|| (type == NSRightMouseDown) || (type == NSRightMouseUp)
		|| (type == NSMouseMoved))
		{
		  [NSCursor setHiddenUntilMouseMoves: NO];
		}
	    }
	}

      if (flag)
        ASSIGN(_current_event, event);
    }
  return event;
}

/**
 * Add an event to be processed by the app, either at end or at beginning
 * (next up) if flag is YES.  This provides a way to provide synthetic input
 * to an application, or, for whatever reason, to allow a real event to be
 * re-dispatched.
 */
- (void) postEvent: (NSEvent *)event atStart: (BOOL)flag
{
  DPSPostEvent(GSCurrentServer(), event, flag);
}

/**
 * Sends the aSelector message to the receiver returned by the
 * -targetForAction:to:from: method (to which the aTarget and sender
 * arguments are passed).<br />
 * The method in the receiver must expect a single argument ...
 * the sender.<br />
 * Any value returned by the method in the receiver is ignored.<br />
 * This method returns YES on success, NO on failure (when no receiver
 * can be found for aSelector).
 */
- (BOOL) sendAction: (SEL)aSelector to: (id)aTarget from: (id)sender
{
  id resp = [self targetForAction: aSelector to: aTarget from: sender];

  if (resp != nil)
    {
      IMP actionIMP = [resp methodForSelector: aSelector];

      if (0 != actionIMP)
        {
          actionIMP(resp, aSelector, sender);
          return YES;
        }
    }

  return NO;
}

/**
 * If theTarget responds to theAction it is returned, otherwise
 * the application searches for an object which will handle
 * theAction and returns the first object found.<br />
 * Returns nil on failure.
 */
- (id) targetForAction: (SEL)theAction to: (id)theTarget from: (id)sender
{
  /*
   * If target responds to the selector then have it perform it.
   */
  if (theTarget)
    {
      if ([theTarget respondsToSelector: theAction])
        {
          return theTarget;
        }
      else
        {
          NSDebugLog(@"Target %@ does not respont to action %@", theTarget, NSStringFromSelector(theAction));
          return nil;
        }
    }
  else if ([sender isKindOfClass: [NSToolbarItem class]])
    {
      /* Special case for toolbar items which must look up the target in the
         responder chain of the window containing their toolbar not in the key
         or main window.
         Note: If (and only if) the toolbar's window is key window we must
         pass it as such to _targetForAction:... so that toolbar items in a
         modal dialog panel work.
       */
      NSWindow *toolbarWindow =
	[[[(NSToolbarItem *)sender toolbar] _toolbarView] window];
      NSWindow *keyWindow = [self keyWindow];
      if (keyWindow != toolbarWindow)
        keyWindow = nil;
      return [self _targetForAction: theAction
			  keyWindow: keyWindow
			 mainWindow: toolbarWindow];
    }
  else
    {
      return [self targetForAction: theAction];
    }
}

/** 
 * <p>
 *   Returns the target object that will respond to aSelector, if any. The
 *   method first checks if any of the key window's first responders, the
 *   key window or its delegate responds. Next it checks the main window in
 *   the same way. Finally it checks the receiver (NSApplication) and its
 *   delegate.
 * </p>
 */
- (id) targetForAction: (SEL)aSelector
{
  if (!aSelector)
    {
      return nil;
    }
  
  /* During a modal session actions must not be sent to the main window of
   * the application, but rather to the dialog window of the modal session.
   * Note that the modal session window is not necessarily the key window,
   * as a panel with worksWhenModal = YES, e.g., the font panel, can still
   * become key window during a modal session.
   */
  NSWindow *mainWindow = [self mainWindow];
  if (_session != 0)
    mainWindow = _session->window;
  return [self _targetForAction: aSelector
		      keyWindow: [self keyWindow]
		     mainWindow: mainWindow];
}


/**
 * Attempts to perform aSelector using [NSResponder-tryToPerform:with:]
 * and if that is not possible, attempts to get the application
 * delegate to perform the aSelector.<br />
 * Returns YES if an object was found to perform aSelector, NO otherwise.
 */
- (BOOL) tryToPerform: (SEL)aSelector with: (id)anObject
{
  if ([super tryToPerform: aSelector with: anObject] == YES)
    {
      return YES;
    }
  if (_delegate != nil && [_delegate respondsToSelector: aSelector])
    {
      IMP actionIMP = [_delegate methodForSelector: aSelector];
      if (0 != actionIMP)
        {
          actionIMP(_delegate, aSelector, anObject);
          return YES;
        }
    }
  return NO;
}

/**<p>Sets the application's icon. Any windows that use the old application
icon image as their mini window image will be updated to use the new
image.</p><p>See Also: -applicationIconImage</p>
*/
- (void) setApplicationIconImage: (NSImage*)anImage
{
  NSEnumerator *iterator;
  NSWindow *current;
  NSImage *old_app_icon = _app_icon;
  NSSize miniWindowSize;
  NSSize imageSize;
  GSDisplayServer *server;

  // Ignore attempts to set nil as the icon image.
  if (nil == anImage)
    return;

  RETAIN(old_app_icon);

  // Use a copy as we change the name and size
  ASSIGNCOPY(_app_icon, anImage);

  server = GSCurrentServer();
  miniWindowSize = server != 0 ? [server iconSize] : NSZeroSize;
  if (miniWindowSize.width <= 0 || miniWindowSize.height <= 0) 
    {
      miniWindowSize = NSMakeSize(48, 48);
    }

  // restrict size when the icon is larger than the mini window.
  imageSize = [_app_icon size];
  if (imageSize.width > miniWindowSize.width
    || imageSize.height > miniWindowSize.height)
    {
      [_app_icon setSize: miniWindowSize];
    }

  // Let horizontal menu change icon
  [_main_menu _organizeMenu];

  if (_app_icon_window != nil)
    {
      [(NSAppIconView *)[_app_icon_window contentView] setImage: _app_icon];
    }

  // Swap the old image for the new one wherever it's used
  iterator = [[self windows] objectEnumerator];
  while ((current = [iterator nextObject]) != nil)
    {
      if ([current miniwindowImage] == old_app_icon)
        [current setMiniwindowImage: _app_icon];
    }

  DESTROY(old_app_icon);
}

/**<p>Returns the current icon be used for the application.</p>
   <p>See Also: -setApplicationIconImage:</p>
 */
- (NSImage*) applicationIconImage
{
  if (!_app_icon)
    {
      /* load the application icon */
      [self _loadAppIconImage];
    }
  return _app_icon;
}

/**
 * Returns the actual window object being used to display the application
 * icon (usually in the dock).
 */
- (NSWindow*) iconWindow
{
  return _app_icon_window;
}

/*
 * Hiding and arranging windows
 */

/**<p> Request this application to "hide" (unmap all windows from the screen
 * except the icon window).  Posts 
 * <code>NSApplicationWillHideNotification</code> and
 * <code>NSApplicationDidHideNotification</code>.  On OS X this activates
 * the next app that is running, however on GNUstep this is up to the window
 * manager.</p><p>See Also: -unhide: -isHidden</p>
 */
- (void) hide: (id)sender
{
#ifdef __MINGW32__
  [self miniaturizeAll: sender];
#else
  if (_app_is_hidden == NO)
    {
      if (![[NSUserDefaults standardUserDefaults]
	     boolForKey: @"GSSuppressAppIcon"])
	{
	  NSArray		*windows_list;
	  NSDictionary  	*info;
	  NSWindow		*win;
	  NSEnumerator  	*iter;

	  [nc postNotificationName: NSApplicationWillHideNotification
	                    object: self];

	  if ([self keyWindow] != nil)
	    {
	      _hidden_key = [self keyWindow];
	      [_hidden_key resignKeyWindow];
	    }
	  
	  // The main window is saved for when the app is activated again.
	  // This is necessary for menu in window.
	  if ([self mainWindow] != nil)
	    {
	      _hidden_main = [self mainWindow];
	      [_hidden_main resignMainWindow];
	    }
	  
          /** Ask the window manager to hide all the application windows for us. 
              Return whether they have been hidden. */
          win = [[self mainMenu] window];
          if ([GSServerForWindow(win) hideApplication: [win windowNumber]] == NO)
            {
              windows_list = GSOrderedWindows();
              iter = [windows_list reverseObjectEnumerator];
	  
              while ((win = [iter nextObject]))
                {
                  if ([win isVisible] == NO && ![win isMiniaturized])
                    {
                      continue;		/* Already invisible	*/
                    }
                  if ([win canHide] == NO)
                    {
                      continue;		/* Not hideable	*/
                    }
                  if (win == _app_icon_window)
                    {
                      continue;		/* can't hide the app icon.	*/
                    }
                  if (_app_is_active == YES && [win hidesOnDeactivate] == YES)
                    {
                      continue;		/* Will be hidden by deactivation	*/
                    }
                  [_hidden addObject: win];
                  [win orderOut: self];
                }
            }
	  _app_is_hidden = YES;
	  
	  if (YES == [[NSUserDefaults standardUserDefaults]
		       boolForKey: @"GSSuppressAppIcon"])
	    {
#if	MINI_ICON
	      NSRect	f = [[[self mainMenu] window] frame];
	      NSPoint	p = f.origin;
	      
	      p.y += f.size.height;
	      [_app_icon_window setFrameTopLeftPoint: p];
	      [_app_icon_window orderFrontRegardless];
	      [_app_icon_window miniaturize: self];
#else
	      [_app_icon_window orderFrontRegardless];
#endif
	    }
	  else
	    {
	      [[_app_icon_window contentView] setNeedsDisplay: YES];
	    }
	  
	  /*
	   * On hiding we also deactivate the application which will make the menus
	   * go away too.
	   */
	  [self deactivate];
	  _unhide_on_activation = YES;
	  
	  info = [self _notificationUserInfo];
	  [nc postNotificationName: NSApplicationDidHideNotification
			    object: self
		          userInfo: info];
	  [[[NSWorkspace sharedWorkspace] notificationCenter]
	    postNotificationName: NSApplicationDidHideNotification
		          object: [NSWorkspace sharedWorkspace]
		        userInfo: info];
	}
      else
	{
	  /*Minimize all windows if there isn't an AppIcon. This isn't the
	    most elegant solution, but avoids to loss the app if the user
	    hide it. */
	  [self miniaturizeAll: sender];
	}
    }
#endif
}

/**<p>Returns whether app is currently in hidden state.</p>
   <p>See Also: -hide: -unhide:</p>
 */
- (BOOL) isHidden
{
  return _app_is_hidden;
}

/**<p>Unhides and activates this application.</p>
   <p>See Also: -unhideWithoutActivation -hide: -isHidden</p>
 */
- (void) unhide: (id)sender
{
  if (_app_is_hidden)
    {
      [self unhideWithoutActivation];
      _unhide_on_activation = NO;
    }
  if (_app_is_active == NO)
    {
      /*
       * Activation should make the applications menus visible.
       */
      [self activateIgnoringOtherApps: YES];
    }
}

/**
 * Unhides this app (displays its windows) but does not activate it.
 */
- (void) unhideWithoutActivation
{
  if (_app_is_hidden == YES)
    {
      NSDictionary	*info;
      NSUInteger	count;
      NSUInteger	i;

      [nc postNotificationName: NSApplicationWillUnhideNotification
			object: self];

      /* Make sure we set this before ordering windows to avoid possible
	 recursive loops (some methods window/backend methods check if
	 the app is hidden before ordering a window).  */
      _app_is_hidden = NO;

      count = [_hidden count];
      for (i = 0; i < count; i++)
        {
          NSWindow *win = [_hidden objectAtIndex: i];
          [win orderFrontRegardless];
          if ([win isMiniaturized])
            {
              [GSServerForWindow(win) miniwindow: [win windowNumber]];
            }
        }
      [_hidden removeAllObjects];
      [[_app_icon_window contentView] setNeedsDisplay: YES];

      info = [self _notificationUserInfo];
      [nc postNotificationName: NSApplicationDidUnhideNotification
			object: self
		      userInfo: info];
      [[[NSWorkspace sharedWorkspace] notificationCenter]
        postNotificationName: NSApplicationDidUnhideNotification
		      object: [NSWorkspace sharedWorkspace]
		    userInfo: info];
    }
}

/**
 * Arranges all non-miniaturized app's windows in front by successively calling
 * [NSWindow-orderFront:] on each window in the app's Windows menu.
 */
- (void) arrangeInFront: (id)sender
{
  NSMenu	*menu;

  menu = [self windowsMenu];
  if (menu)
    {
      NSArray	*itemArray;
      NSUInteger count;
      NSUInteger i;

      itemArray = [menu itemArray];
      count = [itemArray count];
      for (i = 0; i < count; i++)
	{
	  id win = [(NSMenuItem*)[itemArray objectAtIndex: i] target];

	  if ([win isKindOfClass: [NSWindow class]] &&
	      [win isVisible] && ![win isMiniaturized])
	    {
	      [win orderFront: sender];
	    }
	}
    }
}

/*
 * User interface validation
 */
- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem
{
  // FIXME
  return YES;
}

/*
 * Managing windows
 */

/**
 * Returns current key window.  If this app is active, one window should be
 * key.  If this app is not active, nil should be returned.  The key window
 * is the one that will receive keyboard input.
 */
- (NSWindow*) keyWindow
{
  return _key_window;
}

/**
 * Returns current main window of this application. There need not necessarily
 * be a main window, even if app is active, and this should return nil if the
 * app is inactive.
 */
- (NSWindow*) mainWindow
{
  return _main_window;
}

/**
 * Sends aSelector to each window, either in the app's internal window list
 * (flag = NO) or their stacking order from front to back on the screen
 * (flag = YES, <em>currently not implemented under GNUstep</em>).
 */
- (NSWindow*) makeWindowsPerform: (SEL)aSelector inOrder: (BOOL)flag
{
  NSArray *window_list;
  NSUInteger i, c;

  // so i suppose when flag is YES it only runs on visible windows
  if (flag)
    {
      window_list = GSOrderedWindows();
    }
  else
    {
      window_list = [self windows];
    }
  
  for (i = 0, c = [window_list count]; i < c; i++)
    {
      NSWindow *window = [window_list objectAtIndex: i];

      if ([window performSelector: aSelector] != nil)
        {
          return window;
        }
    }
  return nil;
}

/**
 * Iconify all windows in the app.
 */
- (void) miniaturizeAll: sender
{
  NSArray *window_list = [self windows];
  NSUInteger i, count;

  for (i = 0, count = [window_list count]; i < count; i++)
    [[window_list objectAtIndex: i] miniaturize: sender];
}

/**
 * Prevent window reordering in response to most recent mouse down (useful to
 * prevent raise-on-mouse-click).  <em>Not implemented under GNUstep.</em>
 */
- (void) preventWindowOrdering
{
  //TODO
}

/**
 * Set whether the main run loop will request all visible windows update
 * themselves after the current or next event is processed.  (Update occurs
 * after event dispatch in the loop.)
 * This is needed when in NSEventTrackingRunLoopMode.  When the application
 * is using NSDefaultRunLoopMode or NSModalPanelRunLoopMode windows are updated
 * after each loop iteration irrespective of this setting.
 */
- (void) setWindowsNeedUpdate: (BOOL)flag
{
  _windows_need_update = flag;
}

/**
 * Sends each of the app's visible windows an [NSWindow-update] message.
 * This method is called automatically for every iteration of the run loop
 * in NSDefaultRunLoopMode or NSModalPanelRunLoopMode, but is only called during
 * NSEventTrackingRunLoopMode if -setWindowsNeedUpdate: is set to YES.
 */
- (void) updateWindows
{
  NSArray	*window_list = [self windows];
  NSUInteger	count = [window_list count];
  NSUInteger	i;

  _windows_need_update = NO;
  [nc postNotificationName: NSApplicationWillUpdateNotification object: self];

  for (i = 0; i < count; i++)
    {
      NSWindow *win = [window_list objectAtIndex: i];
      if ([win isVisible])
	[win update];
    }
  [nc postNotificationName: NSApplicationDidUpdateNotification object: self];
}

/**
 *  Returns array of app's visible and invisible windows.
 */
- (NSArray*) windows
{
  return GSAllWindows();
}

/**
 * Returns window for windowNum.  Note the window number can be obtained for
 * a window from [NSWindow-windowNumber].
 */
- (NSWindow *) windowWithWindowNumber: (NSInteger)windowNum
{
  return GSWindowWithNumber(windowNum);
}

/*
 * Showing Standard Panels
 */

/** Calls -orderFrontStandardAboutPanelWithOptions: with nil passed as
    the options dictionary.
*/
- (void) orderFrontStandardAboutPanel: sender
{
  [self orderFrontStandardAboutPanelWithOptions: nil];
}

/** OS X compatibility: Calls -orderFrontStandardInfoPanelWithOptions: .
*/
- (void) orderFrontStandardAboutPanelWithOptions: (NSDictionary *)dictionary
{
  [self orderFrontStandardInfoPanelWithOptions: dictionary];
}

/* infoPanel, GNUstep API */
/** Calls -orderFrontStandardInfoPanelWithOptions: with nil passed as
    the options dictionary.
*/
- (void) orderFrontStandardInfoPanel: sender
{
  [self orderFrontStandardInfoPanelWithOptions: nil];
}

/** 
   <p> 
   Orders front the standard info panel for the application,
   taking the needed information from the <code>dictionary</code>
   argument.  There is a single standard info panel per application;
   it is created the first time that this method is invoked, and then
   reused in all subsequent calls.  The application standard info
   panel is immutable and can not be changed after creation.  Useful
   keys for the <code>dictionary</code> are:
   </p>

   <deflist>
   <term>ApplicationName</term>
   <desc>A string with the name of the
   application (eg, <var>"Gorm"</var>).  If not available, the
   <file>Info-gnustep.plist</file> file is searched for the value of
   <var>ApplicationName</var> followed by
   <var>NSHumanReadableShortName</var>. If this also fails, the
   string returned by [NSProcessInfo-processName] is used.
   </desc>

   <term>ApplicationDescription</term> 
   <desc> A string with a very short
   description of the application (eg, <var>"GNUstep Graphics
   Objects Relationship Modeller"</var>).  If not available,
   <file>Info-gnustep.plist</file> is searched for that key; if this
   fails, no application description is shown.
   </desc>

   <term>ApplicationIcon</term> 
   <desc> An image to be shown near the title.
   If not available, <file>Info-gnustep.plist</file> is searched for
   <var>ApplicationIcon</var>; if this fails, NSApp's -applicationIconImage
   method is used instead.  
   </desc>

   <term>ApplicationRelease</term> 
   <desc> A string with the name of the
   application, release included (eg, <var>"Gorm 0.1"</var>).  If
   not available, the value for <var>ApplicationVersion</var> is
   used instead.  If this fails, <file>Info-gnustep.plist</file> is
   searched for <var>ApplicationRelease</var> or
   <var>NSAppVersion</var>, otherwise, <var>"Unknown"</var> is
   used.
   </desc>

   <term>FullVersionID</term> 
   <desc> A string with the full version of the
   application (eg, <var>"0.1.2b"</var> or
   <var>"snap011100"</var>).  If not available,
   <var>Version</var> is used instead.  If this fails,
   <file>Info-gnustep.plist</file> is looked for
   <var>NSBuildVersion</var>.  If all fails, no full version is
   shown.
   </desc>
 
   <term>Authors</term> 
   <desc> An array of strings, each one with the name
   of an author (eg, <var>[NSArray arrayWithObject: "Nicola Pero
   &lt;n.pero\@mi.flashnet.it&gt;"]</var>).  If not found,
   <file>Info-gnustep.plist</file> is searched for <var>Authors</var>,
   if this fails, <var>"Unknown"</var> is displayed.
   </desc>

   <term>URL</term> 
   <desc> [This field is still under work, so it might be
   changed] A string with an URL (eg, <var>"See
   http://www.gnustep.org"</var>).
   </desc>

   <term>Copyright</term> 
   <desc> A string with copyright owners (eg,
   <var>"Copyright (C) 2000 The Free Software Foundation,
   Inc."</var>).  Support for multiple line strings is planned but
   not yet available.  If not found, <file>Info-gnustep.plist</file>
   is searched for <var>Copyright</var> and then (failing this) for
   <var>NSHumanReadableCopyright</var>.  If all fails,
   <var>"Copyright Information Not Available"</var> is used.
   </desc>

   <term>CopyrightDescription</term>
   <desc>A string describing the kind of
   copyright (eg, <var>"Released under the GNU General Public
   License 2.0"</var>).  If not available,
   <file>Info-gnustep.plist</file> is searched for
   <var>CopyrightDescription</var>.  If this fails, no copyright
   description is shown.
   </desc>
   </deflist>
 */
- (void) orderFrontStandardInfoPanelWithOptions: (NSDictionary *)dictionary
{
  if (_infoPanel == nil)
    _infoPanel = [[GSInfoPanel alloc] initWithDictionary: dictionary];
  
  [_infoPanel setTitle: NSLocalizedString (@"Info", 
					   @"Title of the Info Panel")];
  [_infoPanel orderFront: self];
}

/*
 * Getting the main menu
 */

/**
 * Returns the main menu of the receiver.
 */
- (NSMenu*) mainMenu
{
  return _main_menu;
}

/**
 * Sets the main menu of the receiver.  This is sent update messages by the
 * main event loop.
 */ 
- (void) setMainMenu: (NSMenu*)aMenu
{
  if (_main_menu == aMenu)
    {
      return;
    }

  if (_main_menu != nil)
    {
      [_main_menu setMain: NO];
    }

  ASSIGN(_main_menu, aMenu);

  if (_main_menu != nil)
    {
      [_main_menu setMain: YES];
    }
}

/*
   Overrides to show transient version of main menu as in NeXTstep.
*/
- (void) rightMouseDown: (NSEvent*)theEvent
{
  // On right mouse down display the main menu transient
  if (_main_menu != nil)
    [NSMenu popUpContextMenu: _main_menu
	    withEvent: theEvent
	    forView: nil];
  else
    [super rightMouseDown: theEvent];
}

/**
 *  Here for compatibility with OS X, but currently a no-op.
 */
- (void) setAppleMenu: (NSMenu*)aMenu
{
    //TODO: Unclear, what this should do.
}

/*
 * Managing the Windows menu
 */

/**
 * Adds an item to the app's Windows menu.  This is usually done automatically
 * so you don't need to call this method.
 */
- (void) addWindowsItem: (NSWindow*)aWindow
		  title: (NSString*)aString
	       filename: (BOOL)isFilename
{
  [self changeWindowsItem: aWindow  title: aString  filename: isFilename];
}

/**
 * Removes an item from the app's Windows menu.  This is usually done
 * automatically so you don't need to call this method.
 */
- (void) removeWindowsItem: (NSWindow*)aWindow
{
  if (_windows_menu)
    {
      NSArray	*itemArray;
      NSUInteger count;

      itemArray = [_windows_menu itemArray];
      count = [itemArray count];
      while (count-- > 0)
	{
	  NSMenuItem *item = [itemArray objectAtIndex: count];

	  if ([item target] == aWindow)
	    {
	      [_windows_menu removeItemAtIndex: count];
	      return;
	    }
	}
    }
}

// this is an internal helper method for changeWindowsItem:title:filename
// when window represents an editable document
- (void) setImageForWindowsItem: (NSMenuItem *)item
{
  NSImage *oldImage = [item image];
  NSImage *newImage;

  if (!([[item target] styleMask] & NSClosableWindowMask))
    return;

  if ([[item target] isDocumentEdited])
    {
      newImage = [NSImage imageNamed: @"common_CloseBroken"];
    }
  else
    {
      newImage = [NSImage imageNamed: @"common_Close"];
    }

  if (newImage != oldImage)
    {
      [item setImage: newImage];
    }
}

/** Changes the Window menu item associated with aWindow to aString. If no
    associated window item exists, one is created. If isFilename is YES, then
    aString is assumed to be a filename representation the way
    [NSWindow-setTitleWithRepresentedFilename:] would format it, otherwise
    the string is displayed literally in the menu item.
 */
- (void) changeWindowsItem: (NSWindow*)aWindow
		     title: (NSString*)aString
		  filename: (BOOL)isFilename
{
  NSArray	*itemArray;
  NSUInteger	count;
  NSUInteger	i;
  id		item;

  if (![aWindow isKindOfClass: [NSWindow class]])
    [NSException raise: NSInvalidArgumentException
		 format: @"Object of bad type passed as window"];

  if (isFilename)
    {
      NSRange	r = [aString rangeOfString: @"  --  "];

      if (r.length > 0)
	{
	  aString = [aString substringToIndex: r.location];
	}
    }

  /*
   * If there is no menu and nowhere to put one, we can't do anything.
   */
  if (_windows_menu == nil)
    return;

  /*
   * Check if the window is already in the menu.  
   */
  itemArray = [_windows_menu itemArray];
  count = [itemArray count];
  for (i = 0; i < count; i++)
    {
      NSMenuItem *item = [itemArray objectAtIndex: i];
      
      if ([item target] == aWindow)
	{
	  /*
	   * If our menu item already exists and with the correct
	   * title, we need not continue.  
	   */
	  if ([[item title] isEqualToString: aString])
	    {
	      return;
	    }
	  else
	    {
	      /* 
	       * Else, we need to remove the old item and add it again
	       * with the new title.  Then new item might be located
	       * somewhere else in the menu than the old one (because
	       * items in the menu are sorted by title) ... this is
	       * why we remove the old one and then insert it again.
	       */
	      [_windows_menu removeItem: item];
	      break;
	    }
	}
    }

  /*
   * Can't permit an untitled window in the window menu ... so if the 
   * window has not title, we don't add it to the menu.
   */
  if (aString == nil || [aString isEqualToString: @""])
    return;
  
  /*
   * Now we insert a menu item for the window in the correct order.
   * Window menu items are inserted in alphabetic order at the bottom
   * of the windows menu except for two special items with actions
   * 'performMiniaturize: ' and 'performClose: '.  If these exist the
   * they are kept at the below all window entries in the menu.
   */
  itemArray = [_windows_menu itemArray];
  count = [itemArray count];

  if (count > 0 && sel_isEqual([[itemArray objectAtIndex: count-1] action],
		@selector(performClose:)))
    count--;
  if (count > 0 && sel_isEqual([[itemArray objectAtIndex: count-1] action],
		@selector(performMiniaturize:)))
    count--;
  for (i = 0; i < count; i++)
    {
      NSMenuItem *item = [itemArray objectAtIndex: i];
      if ([[item target] isKindOfClass: [NSWindow class]] &&
	  sel_isEqual([item action], @selector(makeKeyAndOrderFront:)))
	break;
    }

  while (i < count)
    {
      item = [itemArray objectAtIndex: i];

      if ([[item title] compare: aString] == NSOrderedDescending)
	break;
      i++;
    }

  item = [_windows_menu insertItemWithTitle: aString
			action: @selector(makeKeyAndOrderFront:)
			keyEquivalent: @""
			atIndex: i];
  [item setTarget: aWindow];

  // When changing for a window with a file, we should also set the image.
  [self setImageForWindowsItem: item];
}

/**
 * Update Windows menu item for aWindow, to reflect its edited status.  This
 * is usually done automatically so you don't need to call this.
 */
- (void) updateWindowsItem: (NSWindow*)aWindow
{
  NSMenu *menu;

  menu = [self windowsMenu];
  if (menu != nil)
    {
      NSArray	*itemArray;
      NSUInteger count;
      NSUInteger i;
      BOOL	found = NO;

      itemArray = [menu itemArray];
      count = [itemArray count];
      for (i = 0; i < count; i++)
	{
	  NSMenuItem *item = [itemArray objectAtIndex: i];

	  if ([item target] == aWindow)
	    {
	      [self setImageForWindowsItem: item];
	      break;
	    }
	}

      if (found == NO)
	{
	  [self changeWindowsItem: aWindow
			    title: [aWindow title]
			 filename: [aWindow _hasTitleWithRepresentedFilename]];
	}
    }
}

/**
 * Sets the windows menu of the receiver.  The windows menu keeps track of all
 * windows open in the application.
 */
- (void) setWindowsMenu: (NSMenu*)aMenu
{
  if (_windows_menu == aMenu)
    {
      return;
    }

  /*
   * Remove all the windows from the old windows menu.
   */
  if (_windows_menu != nil)
    {
      NSArray *itemArray = [_windows_menu itemArray];
      NSUInteger i, count = [itemArray count];
      
      for (i = 0; i < count; i++)
	{
	  NSMenuItem *anItem = [itemArray objectAtIndex: i];
	  id win = [anItem target];

	  if ([win isKindOfClass: [NSWindow class]])
	    {
	      [_windows_menu removeItem: anItem];
	    }
	}
    }

  /* Set the new _windows_menu.  */
  ASSIGN (_windows_menu, aMenu);
  
  {
    /*
     * Now use [-changeWindowsItem:title:filename:] to build the new menu.
     */
    NSArray * windows = [self windows];
    NSUInteger i, count = [windows count];
    for (i = 0; i < count; i++)
      {
	NSWindow	*win = [windows objectAtIndex: i];
	
	if (([win isExcludedFromWindowsMenu] == NO)
	    && ([win isVisible] || [win isMiniaturized]))
	  {
	    [self changeWindowsItem: win
			      title: [win title]
			   filename: [win _hasTitleWithRepresentedFilename]];
	  }
      }
  }
}

/**
 * Returns current Windows menu for the application (whose window contents are
 * managed automatically by this class and NSWindow).
 */
- (NSMenu*) windowsMenu
{
  return _windows_menu;
}

/*
 * Managing the Service menu
 */

/**
 * Registers the types this application can send and receive over Services.
 * sendTypes represents the pasteboard types this app can receive in Service
 * invocation.  This is used to set up the services menu of this app -- an
 * item is added for each registered service that can accept one of sendTypes
 * or return one of returnTypes
 */
- (void) registerServicesMenuSendTypes: (NSArray *)sendTypes
			   returnTypes: (NSArray *)returnTypes
{
  [_listener registerSendTypes: sendTypes
		  returnTypes: returnTypes];
}

/** <p>Returns the services menu of the receiver.</p>
    <p>See Also: -setServicesMenu:</p>
 */
- (NSMenu *) servicesMenu
{
  return [_listener servicesMenu];
}

/**<p> Returns the services provided previously registered using the
 * -setServicesProvider: method.</p><p>See Also: -setServicesProvider:</p>
 */
- (id) servicesProvider
{
  return [_listener servicesProvider];
}

/**<p>ets the services menu for the receiver.  This should be called, otherwise
 * warnings will be generated by the main event loop, which is in charge of
 * updating all menus, including the Services menu.</p>
 * <p>See Also: -servicesMenu</p>
 */
- (void) setServicesMenu: (NSMenu *)aMenu
{
  [_listener setServicesMenu: aMenu];
}

/**
 * Sets the object which provides services to other applications.<br />
 * Passing a nil value for anObject will result in the provision of
 * services to other applications by this application being disabled.<br />
 * See [NSPasteboard] for information about providing services.
 */
- (void) setServicesProvider: (id)anObject
{
  [_listener setServicesProvider: anObject];
}

/**
 * Return an object capable of sending and receiving the specified sendType
 * and returnType in a services interaction.  NSApp is generally the last
 * responder to get this request, and the implementation passes it on to the
 * delegate if it handles it and is not itself an [NSResponder], or returns
 * nil otherwise.
 */
- (id) validRequestorForSendType: (NSString *)sendType
		      returnType: (NSString *)returnType
{
  if (_delegate != nil && ![_delegate isKindOfClass: [NSResponder class]]
    && [_delegate respondsToSelector:
                    @selector(validRequestorForSendType:returnType:)])
    return [_delegate validRequestorForSendType: sendType
				     returnType: returnType];

  return nil;
}

/**
 *  Returns the default drawing context for the app.
 */
- (NSGraphicsContext *) context
{
  return _default_context;
}

/**
 *  NSLogs an exception without raising it.
 */
- (void) reportException: (NSException *)anException
{
  if (anException)
    NSLog (_(@"reported exception - %@"), anException);
}

- (BOOL) presentError: (NSError *)error
{
  NSAlert *alert;
  NSInteger result;

  error = [self willPresentError: error];
  alert = [NSAlert alertWithError: error];
  result = [alert runModal];

  if (result != NSAlertErrorReturn)
    {
      // Convert result (1, 0, -1) into index (0, 1, 2)
      result = 1 - result;
      return [[error recoveryAttempter] attemptRecoveryFromError: error
                 optionIndex: result];
    }
  else
    {
      return NO;
    }
}

struct _DelegateWrapper
{
  id delegate;
  SEL selector;
  NSError *error;
  void *context;
};

- (void) presentError: (NSError*)error
       modalForWindow: (NSWindow*)window
             delegate: (id)delegate 
   didPresentSelector: (SEL)sel
          contextInfo: (void*)context
{
  NSAlert *alert;
  struct _DelegateWrapper *wrapper;

  error = [self willPresentError: error];
  alert = [NSAlert alertWithError: error];
  // FIXME: Who is trying to recover the error?
  wrapper = malloc(sizeof(struct _DelegateWrapper));
  wrapper->delegate = delegate;
  wrapper->selector = sel;
  wrapper->error = error;
  wrapper->context = context;

  [alert beginSheetModalForWindow: window
         modalDelegate: self
         didEndSelector: @selector(_didPresentError:returnCode:contextInfo:)
         contextInfo: wrapper];
}

- (void) _didPresentError: (NSWindow*)sheet 
               returnCode: (NSInteger)result
              contextInfo: (void*)context
{
  struct _DelegateWrapper *wrapper;
  id delegate;
  SEL sel;
  NSError *error;
  void *orgContext;
  BOOL recover;
  
  wrapper = (struct _DelegateWrapper*)context;
  delegate = wrapper->delegate;
  sel = wrapper->selector;
  error = wrapper->error;
  orgContext = wrapper->context;
  free(wrapper);

  if (result != NSAlertErrorReturn)
    {
      // Convert result (1, 0, -1) into index (0, 1, 2)
      result = 1 - result;
      recover = [[error recoveryAttempter] attemptRecoveryFromError: error
                 optionIndex: result];
    }
  else
    {
      recover = NO;
    }
  
  if ([delegate respondsToSelector: sel])
    {
      void (*didEnd)(id, SEL, BOOL, void*);

      didEnd = (void (*)(id, SEL, BOOL, void*))[delegate methodForSelector: sel];
      didEnd(delegate, sel, recover, orgContext);
    }
}

- (NSError*) willPresentError: (NSError*)error
{
  if ([_delegate respondsToSelector: @selector(application:willPresentError:)])
    {
      return [_delegate application: self willPresentError: error];
    }
  else
    {
      return error;
    }
}

/**
 * Requests the application terminates the application.  First an
 * -applicationShouldTerminate: message is sent to the delegate, and only if
 * it returns <code>NSTerminateNow</code> will termination be
 * carried out.<br />
 * The old version of -applicationShouldTerminate: returned a BOOL, and this
 * should still work as YES is
 * equivalent to <code>NSTerminateNow</code> and NO is
 * equivalent to <code>NSTerminateCancel</code>.
 */
- (void) terminate: (id)sender
{
  NSDocumentController		*sdc;
  NSApplicationTerminateReply	termination;

  /* First ask the shared document controller to save any unsaved changes */
  sdc = [NSDocumentController sharedDocumentController];
  if ([[sdc documentClassNames] count] > 0)
    {
      if ([sdc reviewUnsavedDocumentsWithAlertTitle: _(@"Quit")
					cancellable: YES] == NO)
	{
	  return;
	}
    }

  /* Now ask the application delegate whether its okay to terminate */
  termination = NSTerminateNow;
  if ([_delegate respondsToSelector: @selector(applicationShouldTerminate:)])
    {
      /* The old API has applicationShouldTerminate: return a BOOL,
       * so if we are linked in to an application which used that
       * API, the delegate might return a BOOL rather than an
       * NSTerminateNow.  That's fine as both NSTerminateNow
       * and BOOL are integral types (though potentially of different sizes),
       * and NSTerminateNow is defined as YES and NSTerminateCancel as NO.
       * So all we need to do is mask the low byte of the return value in
       * case there is uninitialised random data in the higher bytes.
       */
      termination = ([_delegate applicationShouldTerminate: self] & 0xff);
    }

  if (termination == NSTerminateNow)
    {
      [self replyToApplicationShouldTerminate: YES];
    }
  /*
     Don't need to do anything on NSTerminateLater, as long as user code
     follows the contract (to call replyTo... later).
  */
}

/**
 * Terminates the app if shouldTerminate is YES, otherwise does nothing.
 * This should be called by user code if a delegate has returned
 * <code>NSTerminateLater</code> to an -applicationShouldTerminate: message.
 */
- (void) replyToApplicationShouldTerminate: (BOOL)shouldTerminate
{
  // Prevent cycles in terminate: call.
  if (shouldTerminate && _app_is_running)
    {
      [nc postNotificationName: NSApplicationWillTerminateNotification
	  object: self];
      
      _app_is_running = NO;

      GSRemoveIcon(_app_icon_window);
      [[self windows] makeObjectsPerformSelector: @selector(close)];
      [NSCursor setHiddenUntilMouseMoves: NO];
      
      /* Store our user information.  */
      [[NSUserDefaults standardUserDefaults] synchronize];

      /* Tell the Workspace that we really did terminate.  */
      [[[NSWorkspace sharedWorkspace] notificationCenter]
        postNotificationName: NSWorkspaceDidTerminateApplicationNotification
		      object: [NSWorkspace sharedWorkspace]
		    userInfo: [self _notificationUserInfo]];

      /* Destroy the main run loop pool (this also destroys any nested
         pools which might have been created inside this one).  */
      DESTROY (_runLoopPool);

      /* Now free the NSApplication object.  Enclose the operation
         into an autorelease pool, in case some -dealloc method needs
         to use any temporary object.  */
      {
        NSAutoreleasePool *pool;
	
        IF_NO_GC(pool = [arpClass new]);

        DESTROY(NSApp);

        DESTROY(pool);
      }

      /* And finally, stop the program.  */
      exit(0);
    }
}

- (void) replyToOpenOrPrint: (NSApplicationDelegateReply)reply
{
  // FIXME
}

/**
 * Returns the application's delegate, as set by the -setDelegate: method.<br/>
 * <p>The application delegate will automatically be sent various
 * notifications (as long as it implements the appropriate methods)
 * when application events occur.  The method to handle each of these
 * notifications has name mirroring the notification name, so for instance
 * an <em>NSApplicationDidBecomeActiveNotification</em> is handled by an
 * <code>applicationDidBecomeActive:</code> method.
 * </p> 
 * <list>
 *   <item>NSApplicationDidBecomeActiveNotification</item>
 *   <item>NSApplicationDidFinishLaunchingNotification</item>
 *   <item>NSApplicationDidHideNotification</item>
 *   <item>NSApplicationDidResignActiveNotification</item>
 *   <item>NSApplicationDidUnhideNotification</item>
 *   <item>NSApplicationDidUpdateNotification</item>
 *   <item>NSApplicationWillBecomeActiveNotification</item>
 *   <item>NSApplicationWillFinishLaunchingNotification</item>
 *   <item>NSApplicationWillHideNotification</item>
 *   <item>NSApplicationWillResignActiveNotification</item>
 *   <item>NSApplicationWillTerminateNotification</item>
 *   <item>NSApplicationWillUnhideNotification</item>
 *   <item>NSApplicationWillUpdateNotification</item>
 * </list>
 * <p>The delegate is also sent various messages to ask for authorisation
 * to perform actions, or to ask it to perform actions (again, as long
 * as it implements the appropriate methods).
 * </p>
 * <list>
 *   <item>application:shouldTerminateAfterLastWindowClosed:</item>
 *   <item>application:shouldOpenUntitledFile:</item>
 *   <item>application:openFile:</item>
 *   <item>application:openFiles:</item>
 *   <item>application:openFileWithoutUI:</item>
 *   <item>application:openTempFile:</item>
 *   <item>application:openUntitledFile:</item>
 *   <item>application:shouldOpenUntitledFile:</item>
 *   <item>application:printFile:</item>
 *   <item>application:shouldTerminate:</item>
 *   <item>application:shouldTerminateAfterLastWindowClosed:</item>
 * </list>
 * <p>The delegate is also called upon to respond to any actions which
 *   are not handled by a window, a window delgate, or by the application
 *   object itself.  This is controlled by the -targetForAction: method. 
 * </p>
 * <p>Finally, the application delegate is responsible for handling
 *   messages sent to the application from remote processes (see the
 *   section documenting distributed objects for [NSPasteboard]).
 * </p>
 * <p>See -setDelegate: and [(NSApplicationDelegate)] for more information.</p>
 */
- (id) delegate
{
  return _delegate;
}

/**
 * Sets the delegate of the application to anObject.<br />
 * <p><em>Beware</em>, this does not retain anObject, so you must be sure
 * that, in the event of anObject being deallocated, you
 * stop it being the application delagate by calling this
 * method again with another object (or nil) as the argument.
 * </p>
 * <p>See -delegate and [(NSApplicationDelegate)] for more information.</p>
 */
- (void) setDelegate: (id)anObject
{
  if (_delegate)
    [nc removeObserver: _delegate name: nil object: self];
  _delegate = anObject;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(application##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(application##notif_name:) \
      name: NSApplication##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(DidBecomeActive);
  SET_DELEGATE_NOTIFICATION(DidFinishLaunching);
  SET_DELEGATE_NOTIFICATION(DidHide);
  SET_DELEGATE_NOTIFICATION(DidResignActive);
  SET_DELEGATE_NOTIFICATION(DidUnhide);
  SET_DELEGATE_NOTIFICATION(DidUpdate);
  SET_DELEGATE_NOTIFICATION(WillBecomeActive);
  SET_DELEGATE_NOTIFICATION(WillFinishLaunching);
  SET_DELEGATE_NOTIFICATION(WillHide);
  SET_DELEGATE_NOTIFICATION(WillResignActive);
  SET_DELEGATE_NOTIFICATION(WillTerminate);
  SET_DELEGATE_NOTIFICATION(WillUnhide);
  SET_DELEGATE_NOTIFICATION(WillUpdate);
}

/*
 * Methods for scripting
 */

/**
 * OS X scripting method to return document objects being handled by
 * application.  <em>Not implemented yet under GNUstep.</em>
 */
- (NSArray *) orderedDocuments
{
  // FIXME
  return nil;
}

/**
 * OS X scripting method to return windows in front-to-back on-screen order
 * for scriptable windows.
 * <em>The GNUstep implementation returns all the windows excluding NSPanels.
 * some backends may return an array in an unspecified order.</em>
 */
- (NSArray *) orderedWindows
{
  NSArray *arr = GSOrderedWindows();
  NSMutableArray *ret = [[NSMutableArray alloc] initWithCapacity:[arr count]];
  NSEnumerator *iter = [arr objectEnumerator];
  id win;
  while ((win = [iter nextObject]))
    {
      if (![win isKindOfClass:[NSPanel class]])
        [ret addObject:win];
    }
  
  return AUTORELEASE(ret);
}

/*
 * Methods for user attention requests
 */

/**
 * Cancels a request previously made through calling -requestUserAttention: .
 * Note that request is cancelled automatically if user activates the app.
 */
- (void) cancelUserAttentionRequest: (NSInteger)request
{
  // FIXME
}

/**
 * Method that on OS X makes the icon jump in the doc.  Mercifully, this is
 * unimplemented under GNUstep.  If it <em>were</em> implemented, requestType
 * could be either <code>NSCriticalRequest</code> (bounce endlessly) or
 * <code>NSInformationalRequest</code> (bounce for one second).
 */
- (NSInteger) requestUserAttention: (NSRequestUserAttentionType)requestType
{
  // FIXME
  return 0;
}

- (NSApplicationPresentationOptions) currentPresentationOptions
{
  return _presentationOptions;
}

- (NSApplicationPresentationOptions) presentationOptions
{
  return _presentationOptions;
}

/**
 * Currently unimplemented and unused in GNUstep, it could be extended to handle
 * special GNUstep needs too
 */
- (void)setPresentationOptions: (NSApplicationPresentationOptions)options
{
  _presentationOptions = options;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      /*
      if (_delegate != nil)
        {
	  [aCoder encodeObject: _delegate forKey: @"NSDelegate"];
	}
      [aCoder encodeObject: _main_menu forKey: @"NSMainMenu"]; // ???
      [aCoder encodeObject: _windows_menu forKey: @"NSWindowsMenu"]; // ???
      */
    }
  else
    {
      [aCoder encodeConditionalObject: _delegate];
      [aCoder encodeObject: _main_menu];
      [aCoder encodeConditionalObject: _windows_menu];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  id	obj;

  [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      /*
      if ([aDecoder containsValueForKey: @"NSDelegate"])
	{
	  obj = [aDecoder decodeObjectForKey: @"NSDelegate"];
	  [self setDelegate: obj];
	}
      obj = [aDecoder decodeObjectForKey: @"NSMainMenu"];
      [self setMainMenu: obj];
      obj = [aDecoder decodeObjectForKey: @"NSWindowsMenu"];
      [self setWindowsMenu: obj];
      */
    }
  else
    {
      obj = [aDecoder decodeObject];
      [self setDelegate: obj];
      obj = [aDecoder decodeObject];
      [self setMainMenu: obj];
      obj = [aDecoder decodeObject];
      [self setWindowsMenu: obj];
    }
  return self;
}

@end /* NSApplication */


@implementation	NSApplication (Private)

- (void) _loadAppIconImage
{
  NSDictionary	*infoDict;
  NSString	*appIconFile;
  NSImage	*image = nil;

  infoDict = [[NSBundle mainBundle] infoDictionary];
  appIconFile = [infoDict objectForKey: @"NSIcon"];
  if (appIconFile && ![appIconFile isEqual: @""])
    {
      image = [NSImage imageNamed: appIconFile];
    }

  // Try to look up the icon file.
  appIconFile = [infoDict objectForKey: @"CFBundleIconFile"];
  if (appIconFile && ![appIconFile isEqual: @""])
    {
      image = [NSImage imageNamed: appIconFile];
    }

  if (image == nil)
    {
      image = [NSImage imageNamed: @"GNUstep"];
    }
  else
    {
      /* Set the new image to be named 'NSApplicationIcon' ... to do that we
       * must first check that any existing image of the same name has its
       * name removed.
       */
      [(NSImage*)[NSImage imageNamed: @"NSApplicationIcon"] setName: nil];
      // We need to copy the image as we may have a proxy here
      image = AUTORELEASE([image copy]);
      [image setName: @"NSApplicationIcon"];
    }
  [self setApplicationIconImage: image];
}

- (void) _appIconInit
{
  NSAppIconView	*iv;
  NSUInteger	mask = NSIconWindowMask;
  BOOL  	suppress;
  
  suppress = [[NSUserDefaults standardUserDefaults]
    boolForKey: @"GSSuppressAppIcon"];
#if	MINI_ICON
  if (suppress)
    {
      mask = NSMiniaturizableWindowMask;
    }
#endif
  
  _app_icon_window = [[NSIconWindow alloc] initWithContentRect: NSZeroRect 
				styleMask: mask
				  backing: NSBackingStoreRetained
				    defer: NO
				   screen: nil];



  {
    NSRect iconContentRect;
    NSRect iconFrame;
    NSRect iconViewFrame;

    iconContentRect = GSGetIconFrame(_app_icon_window);
    iconFrame = [_app_icon_window frameRectForContentRect: iconContentRect];
    iconFrame.origin = [[NSScreen mainScreen] frame].origin;
    iconViewFrame = NSMakeRect(0, 0,
      iconContentRect.size.width, iconContentRect.size.height);
    [_app_icon_window setFrame: iconFrame display: YES];

    iv = [[NSAppIconView alloc] initWithFrame: iconViewFrame]; 
    [iv setImage: [self applicationIconImage]];
    [_app_icon_window setContentView: iv];
    RELEASE(iv);
  }

  if (NO == suppress)
    {
      /* The icon window is not suppressed ... display it.
       */
      [_app_icon_window orderFrontRegardless];
    }
}

- (NSDictionary*) _notificationUserInfo
{
  NSString	*path;
  NSString	*port;
  NSNumber	*processIdentifier;
  NSDictionary	*userInfo;

  processIdentifier = [NSNumber numberWithInt:
    [[NSProcessInfo processInfo] processIdentifier]];
  port = [(GSServicesManager*)_listener port];
  path = [[NSBundle mainBundle] bundlePath];
  if (port == nil)
    {
      if (path == nil)
	{
	  userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
	    processIdentifier, @"NSApplicationProcessIdentifier",
	    nil];
	}
      else
	{
	  userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
	    path, @"NSApplicationPath",
	    processIdentifier, @"NSApplicationProcessIdentifier",
	    nil];
	}
    }
  else if (path == nil)
    {
      userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
	port, @"NSApplicationName",
	processIdentifier, @"NSApplicationProcessIdentifier",
	nil];
    }
  else
    {
      userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
	port, @"NSApplicationName",
	path, @"NSApplicationPath",
	processIdentifier, @"NSApplicationProcessIdentifier",
	nil];
    }
  return userInfo;
}

- (void) _openDocument: (NSString*)filePath
{
  [_listener application: self openFile: filePath];
}

- (id) _targetForAction: (SEL)aSelector window: (NSWindow *)window
{
  id resp, delegate;
  NSDocumentController *sdc;
  
  if (window == nil)
    {
      return nil;
    }

  /* traverse the responder chain including the window's delegate */
  resp = [window firstResponder];
  while (resp != nil && resp != self)
    {
      if ([resp respondsToSelector: aSelector])
	{
	  return resp;
	}
      if (resp == window)
	{
	  delegate = [window delegate];
	  if ([delegate respondsToSelector: aSelector])
	    {
	      return delegate;
	    }
	}
      resp = [resp nextResponder];
    }

  /* in a document based app try the window's document */
  sdc = [NSDocumentController sharedDocumentController];
  if ([[sdc documentClassNames] count] > 0)
    {
      resp = [sdc documentForWindow: window];

      if (resp != nil && [resp respondsToSelector: aSelector])
	{
	  return resp;
	}
    }

  /* nothing found */
  return nil;
}

- (id) _targetForAction: (SEL)aSelector
	      keyWindow: (NSWindow *)keyWindow
	     mainWindow: (NSWindow *)mainWindow
{
  NSDocumentController *sdc;
  id resp;

  if (aSelector == NULL)
    return nil;

  /* start looking in the key window's responder chain */
  resp = [self _targetForAction: aSelector window: keyWindow];
  if (resp != nil)
    {
      return resp;
    }

  /* next check the main window's responder chain (provided it is not
   * the key window) */
  if (mainWindow != keyWindow)
    {
      resp = [self _targetForAction: aSelector window: mainWindow];
      if (resp != nil)
	{
	  return resp;
	}
    }

  /* try the shared application imstance and its delegate */
  if ([self respondsToSelector: aSelector])
    {
      return self;
    }

  if (_delegate != nil && [_delegate respondsToSelector: aSelector])
    {
      return _delegate;
    }

  /* Try the NSApplication's responder list to determine if any of them 
   * respond to the selector.
   */
  resp = [self nextResponder];
  while (resp != nil)
    {
      if ([resp respondsToSelector: aSelector])
	{
	  return resp;
	}
      resp = [resp nextResponder];
    }

  /* as last resort in a document based app, try the document controller */
  sdc = [NSDocumentController sharedDocumentController];
  if ([[sdc documentClassNames] count] > 0
    && [sdc respondsToSelector: aSelector])
    {
      return sdc;
    }

  /* give up */
  return nil;
}

- (void) _windowDidBecomeKey: (NSNotification*) notification
{
  id	obj = [notification object];

  if (_key_window == nil && [obj isKindOfClass: [NSWindow class]])
    {
      _key_window = obj;
      [_main_menu update];
    }
  else if (_key_window != obj)
    {
      NSLog(@"Bogus attempt to set key window");
    }
}

- (void) _windowDidBecomeMain: (NSNotification*) notification
{
  id	obj = [notification object];

  if (_main_window == nil && [obj isKindOfClass: [NSWindow class]])
    {
      _main_window = obj;
      [_main_menu update];
    }
  else if (_main_window != obj)
    {
      NSLog(@"Bogus attempt to set main window");
    }
}

- (void) _windowDidResignKey: (NSNotification*) notification
{
  id	obj = [notification object];

  if (_key_window == obj)
    {
      _key_window = nil;
      [NSCursor setHiddenUntilMouseMoves: NO];
    }
  else
    {
      NSLog(@"Bogus attempt to resign key window");
    }
}

- (void) _windowDidResignMain: (NSNotification*) notification
{
  id	obj = [notification object];

  if (_main_window == obj)
    {
      _main_window = nil;
    }
  else
    {
      NSLog(@"Bogus attempt to resign key window");
    }
}

- (void) _lastWindowClosed
{
  if ([_delegate respondsToSelector:
    @selector(applicationShouldTerminateAfterLastWindowClosed:)])
    {
      if ([_delegate
	applicationShouldTerminateAfterLastWindowClosed: self])
        {
          [self terminate: self];
        }
    }
  /* wlux 2009-10-17: If we use MS Windows style menus, terminate
     the application by default when the last window is closed. */
  else if (NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil) == 
	   NSWindows95InterfaceStyle)
    {
      [self terminate: self];
    }
}

- (void) _windowWillClose: (NSNotification*) notification
{
  NSWindow *win = [notification object];

  if (_app_is_running)
    {
      /* FIXME Don't use GSOrderedWindows(), since it may not return all
	 windows when the application is hidden. E.g., given a hidden
	 application with a single window with canHide == NO, if the user
	 closes this window it would appear as if the last window was closed. */
      NSArray *windows_list = GSOrderedWindows();
      NSEnumerator *iter = [windows_list objectEnumerator];
      BOOL wasLast = YES;
      NSWindow *tmp;

      while ((tmp = [iter nextObject]))
        {
          if (tmp != win && [tmp canBecomeMainWindow] == YES)
            {
              wasLast = NO;
	      break;
            }
        }

      /* Perform _lastWindowDidClose at the end of the event loop cycle so
	 that all interested objects are notified before we terminate the
	 application. In particular, this means that a modified document
	 associated with the closed window has been closed before -terminate:
	 is called and therefore the user isn't asked twice whether she wants
	 to save that document's unsaved changes. */
      if (wasLast)
	{
	  [self performSelector: @selector(_lastWindowClosed)
		     withObject: nil
		     afterDelay: 0.1];
	}
    }
}

- (void) _workspaceNotification: (NSNotification*) notification
{
  NSString	*name = [notification name];
  NSDictionary	*info = [notification userInfo];

  /*
   * Handle hiding and unhiding of this app if necessary.
   */
  if ([name isEqualToString: GSUnhideAllApplicationsNotification] == YES)
    {
      [self unhideWithoutActivation];
    }
  else if ([name isEqualToString: GSHideOtherApplicationsNotification] == YES)
    {
      NSString	*port = [info objectForKey: @"NSApplicationName"];

      if ([port isEqual: [[GSServicesManager manager] port]] == NO)
	{
	  [self hide: self];
	}
    }
}

- (NSArray *) _openFiles
{
  NSMutableArray *files = nil;
  NSArray *args = [[NSProcessInfo processInfo] arguments];
  NSEnumerator *en = [args objectEnumerator];
  NSString *file = nil;

  [en nextObject]; // skip the first element, which is always empty...
  while ((file = [en nextObject]) != nil)
    {
      if ([file length] == 0)
        {
          continue;
        }

      if ([file characterAtIndex: 0] != '-')
	{
	  if (files == nil)
	    {
	      files = [NSMutableArray array];
	    }
	  [files addObject: file];
	}
      else
	{
	  break;
	}
    }

  return files;
}

- (NSMenu *) _dockMenu
{
  NSUInteger i, j, n; 
  NSMenu *dockMenu, *windowsMenu;

  // ask delegate for a dock menu, if none create a new one
  dockMenu = nil;
  if ([_delegate respondsToSelector: @selector(applicationDockMenu:)])
    {
      // NB we make a copy of the menu since we are going to modify it
      dockMenu = [[_delegate applicationDockMenu: self] copy];
    }
  if (dockMenu == nil)
    {
      dockMenu = [[NSMenu alloc] initWithTitle:@""];
    }

  // copy window menu entries to the top of the menu
  windowsMenu = [NSApp windowsMenu];
  for (i = j = 0, n = [windowsMenu numberOfItems]; i < n; i++)
    {
      NSMenuItem *item = [windowsMenu itemAtIndex: i];
      if ([[item target] isKindOfClass:[NSWindow class]] &&
          sel_isEqual([item action], @selector(makeKeyAndOrderFront:)))
        {
          [[dockMenu insertItemWithTitle: [item title]
                                  action: @selector(makeKeyAndOrderFront:)
                           keyEquivalent: @""
                                 atIndex: j++]
                    setTarget: [item target]];
        }
    }
  if (j > 0)
    {
      [dockMenu insertItem: [NSMenuItem separatorItem] atIndex: j++];
    }

  // insert standard entries to show or hide and to quit the application at
  // the bottom
  if (j < [dockMenu numberOfItems])
    {
      [dockMenu addItem: [NSMenuItem separatorItem]];
    }
  if ([self isHidden])
    {
      [dockMenu addItemWithTitle:_(@"Show")
                          action:@selector(unhide:)
                   keyEquivalent:@""];
    }
  else
    {
      [dockMenu addItemWithTitle:_(@"Hide")
                          action:@selector(hide:)
                   keyEquivalent:@""];
    }
  [dockMenu addItemWithTitle:_(@"Quit")
                      action:@selector(terminate:)
               keyEquivalent:@""];

  // return the menu
  return dockMenu;
}

@end // NSApplication (Private)


@implementation NSApplication (GSGUIInternal)

- (void) _windowWillDealloc: (NSWindow *)window
{
  if (window == _key_window)
    _key_window = nil;
  if (window == _main_window)
    _main_window = nil;
}

@end

