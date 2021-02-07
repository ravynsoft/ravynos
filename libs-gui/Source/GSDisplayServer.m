/** <title>GSDisplayServer</title>

   <abstract>Abstract display server class.</abstract>

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Date: Mar 2002
   
   This file is part of the GNU Objective C User interface library.

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

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSData.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSGeometry.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSDragView.h"

#import "GSSlideView.h"

/* Display attributes */
NSString * GSDisplayName = @"DisplayName";
NSString * GSDisplayNumber = @"DisplayNumber";
NSString * GSScreenNumber = @"ScreenNumber";

/* The memory zone where all server objects are allocated from (Contexts
   are also allocated from this zone) */
static NSZone *_globalGSZone = NULL;

/* The current concrete class */
static Class defaultServerClass = NULL;

/* Maps windows to a server */
static NSMapTable *windowmaps = NULL;

/* Lock for use when creating contexts */
static NSRecursiveLock  *serverLock = nil;

static NSString *NSCurrentServerThreadKey;
static GSDisplayServer *currentServer = nil;

/** Returns the GSDisplayServer that created the interal
    representation for window. If the internal representation has not
    yet been created (for instance, if the window is deferred), it
    returns the current server */
GSDisplayServer *
GSServerForWindow(NSWindow *window)
{
  int num;

  if (windowmaps == NULL)
    {
      NSLog(@"GSServerForWindow: No window server");
      return nil;
    }

  num = [window windowNumber];
  if (num == 0)
    {
      /* Backend window hasn't been initialized yet, assume current server. */
      return GSCurrentServer();
    }
  return NSMapGet(windowmaps, (void *)(intptr_t)num);
}

/** Returns the current GSDisplayServer */
GSDisplayServer *
GSCurrentServer(void)
{
  return currentServer;
}

/**
  <unit>
  <heading>GSDisplayServer</heading>

  <p>This is an abstract class which provides a framework for a device
  independant window server. A window server handles the very basic control
  of the computer display and input. This includes basic window
  creation and handling, event handling, cursors, and providing
  miscellaneous information about the display.
  </p>
  
  <p>Typically a backend library will provide a concrete subclass
  which implements the device specific methods described below.
  </p>

  <p>In almost all cases, you should not call these methods directly
  in an application. You should use the equivalent methods available
  elsewhere in the library (e.g. NSWindow, NSScreen, etc).
  </p>

  </unit> */
  
@implementation GSDisplayServer

+ (void) initialize
{
  if (serverLock == nil)
    {
      [gnustep_global_lock lock];
      if (serverLock == nil)
        {
          serverLock = [NSRecursiveLock new];
          _globalGSZone = NSDefaultMallocZone();
          defaultServerClass = [GSDisplayServer class];
          NSCurrentServerThreadKey  = @"NSCurrentServerThreadKey";
          windowmaps = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
                                        NSNonOwnedPointerMapValueCallBacks, 20);
        }
      [gnustep_global_lock unlock];
    }
}

/** Set the concrete subclass that will provide the device dependant
    implementation.
*/
+ (void) setDefaultServerClass: (Class)aClass
{
  defaultServerClass = aClass;
}

/** 
    <p>Create a window server with attributes, which contains key/value
    pairs which describe the specifics of how the window server is to
    be initialized. Typically these values are specific to the
    concrete implementation. The current set of attributes that can be
    used with GSDisplayServer is.
   </p>
   <list>
     <item>GSDisplayName</item>
     <item>GSDisplayNumber</item>
     <item>GSScreenNumber</item>
   </list>
   <p>
   GSDisplayName is window server specific and shouldn't be used when
   creating a GSDisplayServer (although you can retrieve the value with
   the -attributes method). On X-Windows the value might be set to something
   like "host:d.s" where host is the host name, d is the display number and
   s is the screen number. GSDisplayNumber indicates the number of the
   display to open. GSScreenNumber indicates the number of the screen to
   display on. If not explicitly set, these attributes may be taked from
   environment variables or from other operating specific information.
   </p>
    <p>In almost all applications one would only create a
    single instance of a window server. Although it is possible, it is
    unlikely that you would need more than one window server (and you
    would have to be very careful how you handled window creation and
    events in this case).</p>
*/
+ (GSDisplayServer *) serverWithAttributes: (NSDictionary *)attributes
{
  GSDisplayServer *server;

  if (self == [GSDisplayServer class])
    {
      server = [[defaultServerClass allocWithZone: _globalGSZone]
	       initWithAttributes: attributes];
    }
  else
    server = [[self allocWithZone: _globalGSZone] 
	       initWithAttributes: attributes];
 
  return AUTORELEASE(server);
}

/** 
    Sets the current server that will be handling windows, events,
    etc. This method must be called after a window server is created
    in order to make it available to the rest of the GUI library
*/
+ (void) setCurrentServer: (GSDisplayServer *)server
{
  ASSIGN(currentServer, server);
}

/** <init />
    Initializes the server. This typically causes the receiver to 
    <em>connect</em> to the display (e.g. XOpenDisplay () on an X-Windows
    server). 
*/
- (id) initWithAttributes: (NSDictionary *)attributes
{
  self = [super init];
  if (nil == self)
    return nil;

  server_info = [attributes mutableCopy];
  event_queue = [[NSMutableArray allocWithZone: [self zone]]
			initWithCapacity: 32];
  drag_types = NSCreateMapTable(NSIntMapKeyCallBacks,
                NSObjectMapValueCallBacks, 0);

  return self;
}

/** Return information used to create the server */
- (NSDictionary *) attributes
{
  return AUTORELEASE([server_info copy]);
}

/**
   Causes the server to disconnect from the display. If the receiver
   is the current server, it removes itself and sets the current 
   server to nil. Sending any more messages to the receiver after this
   is likely to cause severe problems and probably crash the
   application. 
*/
- (void) closeServer
{
  if (self == GSCurrentServer())
    [GSDisplayServer setCurrentServer: nil];
}

- (void) dealloc
{
  NSMapEnumerator	enumerator;
  void			*key;
  void			*val;

  if (windowmaps != NULL)
    {
      /*
       * Remove the display server from the windows map.
       * This depends on a property of GNUstep map tables, that an
       * enumerated object can safely be removed from the map.
       */
      enumerator = NSEnumerateMapTable(windowmaps);
      while (NSNextMapEnumeratorPair(&enumerator, &key, &val))
        {
          if (val == (void*)self)
            {
              NSMapRemove(windowmaps, key);
            }
        }
      NSEndMapTableEnumeration(&enumerator);
    }

  DESTROY(server_info);
  DESTROY(event_queue);
  NSFreeMapTable(drag_types);
  [super dealloc];
}

- glContextClass
{
  return nil;
}

- glPixelFormatClass
{
  return nil;
}

/** Returns YES if the backend handles window decorations and NO
 * if the gui library must do that instead.
 */
- (BOOL) handlesWindowDecorations
{
  return YES;
}


/* Drag and drop support. */
/** Convienience method that calls -addDragTypes:toWindow: using the
    server that controls win.
*/
+ (BOOL) addDragTypes: (NSArray*)types toWindow: (NSWindow *)win
{
  return [GSServerForWindow(win) addDragTypes: types toWindow: win];
}

/** Convienience method that calls -removeDragTypes:fromWindow: using the
    server that controls win.
*/
+ (BOOL) removeDragTypes: (NSArray*)types fromWindow: (NSWindow *)win
{
  return [GSServerForWindow(win) removeDragTypes: types fromWindow: win];
}

/** Convienience method that calls -dragTypesForWindow: using the
    server that controls win.
*/
+ (NSCountedSet*) dragTypesForWindow: (NSWindow *)win
{
  return [GSServerForWindow(win) dragTypesForWindow: win];
}

/**
 * Add (increment count by 1) each drag type to those registered
 * for the window.  If this results in a change to the types registered
 * in the counted set, return YES, otherwise return NO.
 * Subclasses should override this method, call 'super' and take
 * appropriate action if the method returns 'YES'.
 */
- (BOOL) addDragTypes: (NSArray*)types toWindow: (NSWindow *)win
{
  NSCountedSet	*old = (NSCountedSet*)NSMapGet(drag_types, (void*)win);
  NSEnumerator *drag_enum = [types objectEnumerator];
  id            type;
  NSUInteger	originalCount;

  /*
   * Make sure the set exists.
   */
  if (old == nil)
    {
      old = [NSCountedSet new];
      NSMapInsert(drag_types, (void*)win, (void*)(gsaddr)old);
      RELEASE(old);
    }
  originalCount = [old count];

  while ((type = [drag_enum nextObject]))
    {
      [old addObject: type];
    }
  if ([old count] == originalCount)
    return NO;
  return YES;
}

/**
 * Remove (decrement count by 1) each drag type from those registered
 * for the window.  If this results in a change to the types registered
 * in the counted set, return YES, otherwise return NO.
 * If given 'nil' as the array of types, remove ALL.
 * Subclasses should override this method, call 'super' and take
 * appropriate action if the method returns 'YES'.
 */
- (BOOL) removeDragTypes: (NSArray*)types fromWindow: (NSWindow *)win
{
  NSCountedSet	*old = (NSCountedSet*)NSMapGet(drag_types, (void*)win);
  NSEnumerator *drag_enum = [types objectEnumerator];

  if (types == nil)
    {
      if (old == nil)
	return NO;
      NSMapRemove(drag_types, (void*)win);
      return YES;
    }
  else if (old == nil)
    {
      return NO;
    }
  else
    {
      unsigned	originalCount = [old count];
      id o;

      while ((o = [drag_enum nextObject]))
	{
	  [old removeObject: o];
	}
      if ([old count] == originalCount)
	return NO;
      return YES;
    }
}

/** Returns the drag types set for the window win. */
- (NSCountedSet*) dragTypesForWindow: (NSWindow *)win
{
  return (NSCountedSet*)NSMapGet(drag_types, (void *)win);
}

/** Returns an instance of a class which implements the NSDraggingInfo
    protocol. */
- (id <NSDraggingInfo>) dragInfo
{
  return [GSDragView sharedDragView];
}

- (BOOL) slideImage: (NSImage*)image from: (NSPoint)from to: (NSPoint)to
{
  return [GSSlideView _slideImage: image from: from to: to];
}

- (void) restrictWindow: (int)win toImage: (NSImage*)image
{
  [self subclassResponsibility: _cmd];
}

- (int) findWindowAt: (NSPoint)screenLocation 
           windowRef: (int*)windowRef 
           excluding: (int)win
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/* Screen information */
/** Returns the resolution, in points, for the indicated screen of the
    display. */
- (NSSize) resolutionForScreen: (int)screen
{
  /*[self subclassResponsibility: _cmd];*/
  return NSMakeSize(72, 72);
}

/** Returns the bounds, in pixels, for the indicated screen of the
    display. */
- (NSRect) boundsForScreen: (int)screen
{
  [self subclassResponsibility: _cmd];
  return NSZeroRect;
}

/** Returns the default depth of windows that are created on screen. */
- (NSWindowDepth) windowDepthForScreen: (int)screen
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Returns a null terminated list of possible window depths for
    screen. */
- (const NSWindowDepth *) availableDepthsForScreen: (int)screen
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

/**
   Returns an array of NSNumbers, where each number describes a screen
   that is available on this display. The default screen is listed first.
 */
- (NSArray *) screenList
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
   Returns a display dependant pointer that describes the internal
   connection to the display. On X-Windows, for example, this is a
   pointer to the <code>Display</code> variable.  */
- (void *) serverDevice
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

/**
   Returns a display dependant pointer that describes the internal
   window representation for win. On X-Windows, for example, this is a
   pointer to the <code>Window</code> variable. */
- (void *) windowDevice: (int)win
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

/** Play the System Beep */
- (void) beep
{
  [self subclassResponsibility: _cmd];
}

/** 
   Returns a display dependent NSImage which will be used as the background
   image for AppIcons and MiniWindows.  Under Windowmaker, for example this 
   could be a user specified gradient. */
- (NSImage *) iconTileImage
{
  return [NSImage imageNamed: @"common_Tile"];
}


/** Returns the size of icons and miniwindows for screen. */
- (NSSize) iconSize
{
  return NSMakeSize(64.0, 64.0);
}

/** 
 * Returns a screenshot of the specified rectangle of the specified screen.
 * The mouse cursor should be ommitted from the returned image.
 */
- (NSImage *) contentsOfScreen: (int)screen inRect: (NSRect)rect
{
  return nil;
}

@end

/* ----------------------------------------------------------------------- */
/* GNUstep Window operations */
/* ----------------------------------------------------------------------- */
@implementation GSDisplayServer (WindowOps)

/** Tells the receiver that it owns the window described by
    win. Concrete subclasses must call this function when creating a
    window. Do not call this method in any other case, particularly
    for a window that has already been created */
- (void) _setWindowOwnedByServer: (int)win
{
  if (windowmaps != NULL)
    {
      NSMapInsert(windowmaps, (void*)(intptr_t)win,  self);
    }
}

/** Creates a window whose location and size is described by frame and
    whose backing store is described by type. This window is not
    mapped to the screen by this call.<br />

    Note that frame is the frame of the entire GNUstep window including
    borders, titlebar and other standard decorations.<br />
    If -handlesWindowDecorations returns YES, the backend will produce
    (and return the identifier of) a smaller drawable window inside this
    decorated area.<br />
    Use -styleoffsets::::: to determine the extent of the decorations
    and determine the size of the drawable area inside them.
*/
- (int) window: (NSRect)frame : (NSBackingStoreType)type : (unsigned int)style
{
  int sn = [[server_info objectForKey: GSScreenNumber] intValue];

  return [self window: frame : type : style : sn];
}

/** Like -window::: only there is an additional argument to specify which
    screen the window will display on */
- (int) window: (NSRect)frame : (NSBackingStoreType)type : (unsigned int)style
	      : (int)screen
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Destroys the representation of the window and frees and memory
    associated with it. */
- (void) termwindow: (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Create all the backend structures for a reference to a native window and 
    return the extend, backing type, style and screen for that window. */ 
- (int) nativeWindow: (void *)winref
		    : (NSRect*)frame
		    : (NSBackingStoreType*)type 
		    : (unsigned int*)style
		    : (int*)screen
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Sets the style of the window. See [NSWindow-styleMask] for a
    description of the available styles */
- (void) stylewindow: (unsigned int) style : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Changes window's the backing store to type */
- (void) windowbacking: (NSBackingStoreType)type : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Sets the window title */
- (void) titlewindow: (NSString *) window_title : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Miniaturizes the window */
- (void) miniwindow: (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Ask the window manager to hide all the application windows for us. 
    Return whether they have been hidden. */
- (BOOL) hideApplication: (int) win
{
  return NO;
}

/** Returns YES if the application should create the miniwindow counterpart
    to the full size window and own it. Some display systems handle the
    miniwindow themselves. In this case the backend subclass should
    override this method to return NO. */
- (BOOL) appOwnsMiniwindow
{
  return YES;
}


/** Sets the window device information for the current NSGraphicsContext,
    typically by calling [NSGraphicsContext-GSSetDevice:::],
    although depending on the concrete implmentation, more information
    than this may need to be exchanged. */
- (void) windowdevice: (int)winNum
{
  [self setWindowdevice: winNum forContext: GSCurrentContext()];
}

/** Sets the window device information for the NSGraphicsContext,
    typically by calling [NSGraphicsContext-GSSetDevice:::],
    although depending on the concrete implmentation, more information
    than this may need to be exchanged. */
- (void) setWindowdevice: (int)win forContext: (NSGraphicsContext *)ctxt
{
  [self subclassResponsibility: _cmd];
}

/**
 * <p>Causes the window to be ordered onto or off the screen depending
 * on the value of op. The window is ordered relative to otherWin.
 * </p>
 * <p>The effect of the various combinations of op and otherWin are:
 * </p>
 * <deflist>
 *   <term>op is NSWindowOut</term>
 *   <desc>
 *     The window is removed from the display and otherWinm is ignored.
 *   </desc>
 *   <term>op is NSWindowAbove and otherWin is zero</term>
 *   <desc>
 *     The window is placed above all other windows at the same level
 *     unless doing the current key window is at this level (in which
 *     case the window will be placed immediately below that).
 *   </desc>
 *   <term>op is NSWindowAbove and otherWin is minus one</term>
 *   <desc>
 *     The window is placed above all other windows at the same level
 *     even if doing that would place it above the current key window.<br />
 *     This is a special feature that [NSWindow-orderWindow:relativeTo:] uses
 *     to place the window correctly.
 *   </desc>
 *   <term>op is NSWindowBelow and otherWin is zero</term>
 *   <desc>
 *     The window is placed above all other windows at the same level.
 *   </desc>
 *   <term>op is NSWindowAbove and otherWin is a window on the display</term>
 *   <desc>
 *     The level of the window is set to be the same as that of
 *     otherWin and the window is placed immediately above otherWin.
 *   </desc>
 *   <term>op is NSWindowBelow and otherWin is a window on the display</term>
 *   <desc>
 *     The level of the window is set to be the same as that of
 *     otherWin and the window is placed immediately below otherWin.
 *   </desc>
 * </deflist>
 */
- (void) orderwindow: (int) op : (int) otherWin : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Moves the bottom left corner of the window (including any border)
 * to loc.<br />
 * The position is expressed as an offset from the bottom left
 * corner of the screen.
 */ 
- (void) movewindow: (NSPoint)loc : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Moves and resizes the window on the screen as described by frame.
 * The value of frame is a rectangle containing the entire window, including
 * any border/decorations.  Its position is expressed as an offset from
 * the bottom left corner of the screen.
 */
- (void) placewindow: (NSRect)frame : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Returns the frame of the window on the screen.<br />
 * The value of frame is a rectangle containing the entire window, including
 * any border/decorations.  Its position is expressed as an offset from
 * the bottom left corner of the screen.
 */
- (NSRect) windowbounds: (int) win
{
  [self subclassResponsibility: _cmd];
  return NSZeroRect;
}

/** Set the level of the window as in the [NSWindow -setLevel] method.<br />
 * The use of window levels organises the window hierarchy into groups
 * of windows at each level.  It effects the operation of the
 * -orderwindow::: method in the case where the position is 'above' or
 * 'below' and the other window number is zero.
 */
- (void) setwindowlevel: (int) level : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Returns the window level as in [NSWindow -level] */
- (int) windowlevel: (int) win
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Backends can override this method to return an array of window numbers 
    ordered front to back.  The front most window being the first object
    in the array.  
    The default implementation returns the visible windows in an
    unspecified order.
 */
- (NSArray *) windowlist
{
  NSMutableArray *list = [NSMutableArray arrayWithArray:[NSApp windows]];
  int c = [list count];

  while (c-- > 0)
    {
       if (![[list objectAtIndex:c] isVisible])
         {
	   [list removeObjectAtIndex:c];
         }
    }
  return [list valueForKey:@"windowNumber"];
}

/** Returns the depth of the window */
- (int) windowdepth: (int) win
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/** Set the maximum size (pixels) of the window */
- (void) setmaxsize: (NSSize)size : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Set the minimum size (pixels) of the window */
- (void) setminsize: (NSSize)size : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Set the resize incremenet of the window */
- (void) setresizeincrements: (NSSize)size : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Causes buffered graphics to be flushed to the screen.
 * The value of rect is expressed in OpenStep window coordinates.
 */
- (void) flushwindowrect: (NSRect)rect : (int) win
{
  [self subclassResponsibility: _cmd];
}

/**
 * Returns the dimensions of window decorations added outside the drawable
 * window frame by a window manager or equivalent. For instance, t
 * gives the height of the title bar for the window.<br />
 * If -handlesWindowDecorations returns NO, there
 * are no decorations outside the drawable window frame and this method
 * shouldn't be called.
 * */
- (void) styleoffsets: (float*) l : (float*) r : (float*) t : (float*) b 
		     : (unsigned int) style
{
  [self subclassResponsibility: _cmd];
}

/** Sets the document edited flag for the window */
- (void) docedited: (int) edited : (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Sets the input state for the window given by the
    GSWindowInputState constant.  Instructs the window manager that the
    specified window is 'key', 'main', or just a normal window.  */
- (void) setinputstate: (int)state : (int)win
{
  [self subclassResponsibility: _cmd];
}

/** Forces focus to the window so that all key events are sent to this
    window */
- (void) setinputfocus: (int) win
{
  [self subclassResponsibility: _cmd];
}

/** Sets the transparancy value for the whole window */
- (void) setalpha: (float)alpha : (int) win
{
  //[self subclassResponsibility: _cmd];
}

/** Sets the window shadow */
- (void) setShadow: (BOOL)hasShadow : (int)win
{
  //[self subclassResponsibility: _cmd];
}

/** Returns the current mouse location on the default screen. If the
 * pointer is not on the default screen, an invalid point (-1,-1} is
 * returned.<br />
 * The location is expressed as an offset from the bottom left corner
 * of the screen.
 */
- (NSPoint) mouselocation
{
  [self subclassResponsibility: _cmd];
  return NSZeroPoint;
}

/** Returns the current mouse location on aScreen. If the pointer is
 * not on aScreen, this method acts like -mouselocation. If aScreen is -1,
 * then the location of the mouse on any screen is returned. The
 * win pointer returns the window number of the GNUstep window
 * that the mouse is in or 0 if it is not in a window.<br />
 * The location is expressed as an offset from the bottom left corner
 * of the screen.
 */
- (NSPoint) mouseLocationOnScreen: (int)aScreen window: (int *)win
{
  [self subclassResponsibility: _cmd];
  return NSZeroPoint;
}

/** Grabs the pointer device so that all future mouse events will be
    directed only to the window win. If successful, the return value
    is YES and this message must be balanced by a -releasemouse
    message.  */
- (BOOL) capturemouse: (int) win
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/** Release a previous captured mouse from -capturemouse: */
- (void) releasemouse
{
  [self subclassResponsibility: _cmd];
}

/** Set mouse cursor position. */
- (void) setMouseLocation: (NSPoint)mouseLocation onScreen: (int)aScreen
{
  [self subclassResponsibility: _cmd];
}

/** Hides the cursor */
- (void) hidecursor
{
  [self subclassResponsibility: _cmd];
}

/** Show a previously hidden cursor */
- (void) showcursor
{
  [self subclassResponsibility: _cmd];
}

/** Create a standard cursor (such as an arror or IBeam). Returns a
    pointer to the internal device representation that can be used
    later to make this cursor the current one
*/
- (void) standardcursor: (int) style : (void**) cid
{
  [self subclassResponsibility: _cmd];
}

/** Create a cursor from an image. Returns a pointer to the internal
    device representation that can be used later to make this cursor
    the current one */
- (void) imagecursor: (NSPoint)hotp : (NSImage *) image : (void**) cid
{
  [self subclassResponsibility: _cmd];
}

/** Set the cursor given by the cid representation as being the
    current cursor. The cursor has a foreground color fg and a
    background color bg. To keep the default color for the cursor, pass
    nil for fg and bg. */
- (void) setcursorcolor: (NSColor *)fg : (NSColor *)bg : (void*) cid
{
  NSLog(@"Call to obsolete method -setcursorcolor:::");
  [self recolorcursor: fg : bg : cid];
  [self setcursor: cid];
}

/** Recolour the cursor given by the cid representation into having
    a foreground color fg and a background color bg. */
- (void) recolorcursor: (NSColor *)fg : (NSColor *)bg : (void*) cid
{
  [self subclassResponsibility: _cmd];
}

/** Set the cursor given by the cid representation as being the
    current cursor. */
- (void) setcursor: (void*) cid
{
  [self subclassResponsibility: _cmd];
}

/** Free the cursor given by the cid representation. */
- (void) freecursor: (void*) cid
{
  [self subclassResponsibility: _cmd];
}

- (void) setParentWindow: (int)parentWin 
          forChildWindow: (int)childWin
{
  [self subclassResponsibility: _cmd];
}

- (void) setIgnoreMouse: (BOOL)ignoreMouse : (int)win
{
  // Do nothing if not overridden by subclass
}

@end

/* ----------------------------------------------------------------------- */
/* GNUstep Event Operations */
/* ----------------------------------------------------------------------- */
@implementation GSDisplayServer (EventOps)

/**
 * Scans through the event queue to find the first event whose type matches
 * mask.  If no event is found, then the current run loop is run in the
 * specified mode to allow more events to arrive.<br />
 * If a matching event is found, it is returned and either removed from or
 * left in the queue according to flag.<br />
 * If no matching event is found and the limit date is reached, this method
 * returns nil.
 */
- (NSEvent*) getEventMatchingMask: (unsigned)mask
		       beforeDate: (NSDate*)limit
			   inMode: (NSString*)mode
			  dequeue: (BOOL)flag
{
  NSUInteger pos = 0;	/* Position in queue scanned so far	*/
  NSRunLoop *loop = nil;

  do
    {
      NSUInteger count = [event_queue count];
      NSEvent *event;
      NSUInteger i = 0;

      if (count == 0)
	{
	  event = nil;
	}
      else if (mask == NSAnyEventMask)
	{
	  /*
	   * Special case - if the mask matches any event, we just get the
	   * first event on the queue.
	   */
	  event = [event_queue objectAtIndex: 0];
	}
      else
	{
	  event = nil;
	  /*
	   * Scan the queue from the last position we have seen, up to the end.
	   */
	  if (count > pos)
	    {
	      NSUInteger end = count - pos;
	      NSRange	r = NSMakeRange(pos, end);
	      NSEvent	*events[end];

	      [event_queue getObjects: events range: r];
	      for (i = 0; i < end; i++)
		{
		  if (mask & NSEventMaskFromType([events[i] type]))
		    {
		      event = events[i];
		      break;
		    }
		}
	    }
	}

      /*
       * Note the position we have read up to.
       */
      pos += i;

      /*
       * If we found a matching event, we (depending on the flag) de-queue it.
       * We return the event RETAINED - the caller must release it.
       */
      if (event)
	{
	  RETAIN(event);
	  if (flag)
	    {
	      [event_queue removeObjectAtIndex: pos];
	    }
	  return AUTORELEASE(event);
	}
      if (loop == nil)
	{
	  loop = [NSRunLoop currentRunLoop];
	}
      if ([loop runMode: mode beforeDate: limit] == NO)
	{
	  break;	// Nothing we can do ... no input handlers.
	}
    }
  while ([limit timeIntervalSinceNow] > 0.0);

  return nil;	/* No events in specified time	*/
}

/**
 * Steps through the event queue and removes all events whose timestamp
 * is earlier than that of limit wand which match the supplied mask
 * of event types.
 */
- (void) discardEventsMatchingMask: (unsigned)mask
		       beforeEvent: (NSEvent*)limit
{
  NSUInteger index = [event_queue count];

  /*
   *	If there is a range to use - remove all the matching events in it
   *    which were created before the specified event.
   */
  if (index > 0)
    {
      NSTimeInterval when = [limit timestamp];
      NSEvent *events[index];

      [event_queue getObjects: events];

      while (index-- > 0)
	{
	  NSEvent *event = events[index];

	  if ([event timestamp] < when)
	    {	
	      if ((mask == NSAnyEventMask)
		|| (mask & NSEventMaskFromType([event type])))
		{
		  [event_queue removeObjectAtIndex: index];
		}
	    }
	}
    }
}

/** Posts an event to the event queue.  The value of flag determines
 * whether the event is inserted at the start of the queue or appended
 * at the end.
 */
- (void) postEvent: (NSEvent*)anEvent atStart: (BOOL)flag
{
  if (flag)
    [event_queue insertObject: anEvent atIndex: 0];
  else
    [event_queue addObject: anEvent];
}

- (void) _printEventQueue
{
  NSUInteger index = [event_queue count];

  if (index > 0)
    {
      NSEvent *events[index];
      NSUInteger i;

      NSLog(@"Dumping events from queue");
      [event_queue getObjects: events];

      for (i = 0; i < index; i++)
	{
	  NSEvent *event = events[i];

          NSLog(@"index %lu %@", (unsigned long) i, event);
        }
    }
  else
    {
      NSLog(@"Event queue is empty");
    }
}

@end
