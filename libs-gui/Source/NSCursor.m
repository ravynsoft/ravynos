/** <title>NSCursor</title>

   <abstract>Holds an image to use as a cursor</abstract>

   Copyright (C) 1996,1999,2001 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Adam Fedor <fedor@gnu.org>
   Date: Dec 2001
   
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

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyedArchiver.h>

#import "AppKit/NSColor.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSBitmapImageRep.h"

#import "GNUstepGUI/GSDisplayServer.h"

// Class variables
static NSMutableArray *gnustep_gui_cursor_stack;
static NSCursor *gnustep_gui_current_cursor;
static BOOL gnustep_gui_hidden_until_move;
static Class NSCursor_class;

static NSMutableDictionary *cursorDict = nil;

@implementation NSCursor

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSCursor class])
    {
      // Initial version
      [self setVersion: 1];

      // Initialize class variables
      NSCursor_class = self;
      gnustep_gui_cursor_stack = [[NSMutableArray alloc] initWithCapacity: 1];
      gnustep_gui_hidden_until_move = NO;
      cursorDict = [NSMutableDictionary new]; 
      [[self arrowCursor] set];
    }
}

#ifdef WIN32
+ (NSUInteger) count
{
  return [gnustep_gui_cursor_stack count];
}
#endif

- (void *) _cid
{
  return _cid;
}

- (void) _setCid: (void *)val
{
  _cid = val;
}

- (void) _computeCid
{
  void *c;
  
  if (_cursor_image == nil)
    {
      _cid = NULL;
      return;
    }

  [GSCurrentServer() imagecursor: _hot_spot : _cursor_image : &c];
  _cid = c;
}

/**<p>Hides the current cursor.</p>
 */
+ (void) hide
{
  [GSCurrentServer() hidecursor];
}

/**<p>Pops the cursor off the top of the stack and makes the previous one
   as the current cursor.</p>
 */
+ (void) pop
{
  /*
   * The object we pop is the current cursor
   */
  if ([gnustep_gui_cursor_stack count] > 0)
    {
      NSCursor *c = RETAIN([gnustep_gui_cursor_stack lastObject]);
      [gnustep_gui_cursor_stack removeLastObject];

      NSDebugLLog(@"NSCursor", @"Cursor pop");
      [c set];
      RELEASE(c);
    }
}

/**<p>Hides the cursor if <var>flag</var> is YES. Unhides it if NO</p>
 */
+ (void) setHiddenUntilMouseMoves: (BOOL)flag
{
  if (flag)
    {
      [self hide];
    } 
  else 
    {
      [self unhide];
    } 
  gnustep_gui_hidden_until_move = flag;
}

/**<p>Returns whther the cursor is hidden until mouse moves</p>
 */
+ (BOOL) isHiddenUntilMouseMoves
{
  return gnustep_gui_hidden_until_move;
}

/**<p>Shows the NSCursor.</p>
   <p>See Also: +hide</p>
 */
+ (void) unhide
{
  [GSCurrentServer() showcursor];
}

/*
 * Getting the Cursor
 */
static
NSCursor *getStandardCursor(NSString *name, int style)
{
  NSCursor *cursor = [cursorDict objectForKey: name];

  if (cursor == nil)
    {
      void *c = NULL;
    
      cursor = [[NSCursor_class alloc] initWithImage: nil
                                             hotSpot: NSZeroPoint];
      if (nil != cursor)
        {
          [GSCurrentServer() standardcursor: style : &c];
          if (c == NULL)
            {
              /* 
                 There is no standard cursor with this name defined in the 
                 backend, so try an image with this name.
              */
              [cursor setImage: [NSImage imageNamed: name]];
            }
          else
            {
              [cursor _setCid: c];
            }
          cursor->_cursor_flags.type = style;
          [cursorDict setObject: cursor forKey: name];
          RELEASE(cursor);
        }
    }
  return cursor;
}

/**<p>Returns an arrow cursor.</p>
 */
+ (NSCursor*) arrowCursor
{
  return getStandardCursor(@"GSArrowCursor", GSArrowCursor);
}

/**<p>Returns an I-beam cursor.</p>
 */
+ (NSCursor*) IBeamCursor
{
  return getStandardCursor(@"GSIBeamCursor", GSIBeamCursor);
}

+ (NSCursor*) closedHandCursor
{
  return getStandardCursor(@"GSClosedHandCursor", GSClosedHandCursor);
}

+ (NSCursor*) contextualMenuCursor
{
  return getStandardCursor(@"GSContextualMenuCursor", GSContextualMenuCursor);
}

+ (NSCursor*) crosshairCursor
{
  return getStandardCursor(@"GSCrosshairCursor", GSCrosshairCursor);
}

+ (NSCursor*) dragCopyCursor
{
  return getStandardCursor(@"GSDragCopyCursor", GSDragCopyCursor);
}

+ (NSCursor*) dragLinkCursor
{
  return getStandardCursor(@"GSDragLinkCursor", GSDragLinkCursor);
}

+ (NSCursor*) disappearingItemCursor
{
  return getStandardCursor(@"GSDisappearingItemCursor", GSDisappearingItemCursor);
}

+ (NSCursor*) openHandCursor
{
  return getStandardCursor(@"GSOpenHandCursor", GSOpenHandCursor);
}

+ (NSCursor*) operationNotAllowedCursor
{
  return getStandardCursor(@"GSOperationNotAllowedCursor", 
                           GSOperationNotAllowedCursor);
}

+ (NSCursor*) pointingHandCursor
{
  return getStandardCursor(@"GSPointingHandCursor", GSPointingHandCursor);
}

+ (NSCursor*) resizeDownCursor
{
  return getStandardCursor(@"GSResizeDownCursor", GSResizeDownCursor);
}

+ (NSCursor*) resizeLeftCursor
{
  return getStandardCursor(@"GSResizeLeftCursor", GSResizeLeftCursor);
}

+ (NSCursor*) resizeLeftRightCursor
{
  return getStandardCursor(@"GSResizeLeftRightCursor", GSResizeLeftRightCursor);
}

+ (NSCursor*) resizeRightCursor
{
  return getStandardCursor(@"GSResizeRightCursor", GSResizeRightCursor);
}

+ (NSCursor*) resizeUpCursor
{
  return getStandardCursor(@"GSResizeUpCursor", GSResizeUpCursor);
}

+ (NSCursor*) resizeUpDownCursor
{
  return getStandardCursor(@"GSResizeUpDownCursor", GSResizeUpDownCursor);
}

/**<p>Returns the current cursor.</p>
 */
+ (NSCursor*) currentCursor
{
  return gnustep_gui_current_cursor;
}

+ (NSCursor*) currentSystemCursor
{
  // FIXME
  return nil;
}

+ (NSCursor*) greenArrowCursor
{
  return getStandardCursor(@"GSGreenArrowCursor", GSGreenArrowCursor);
}

/*
 * Initializing a New NSCursor Object
 */
- (id) init
{
  return [self initWithImage: nil hotSpot: NSZeroPoint];
}

/**<p>Initializes and returns a new NSCursor with a NSImage <var>newImage</var>
   and a hot spot point with x=0 and y=0 (top left corner).</p>
   <p>See Also: -initWithImage:hotSpot:</p>
 */
- (id) initWithImage: (NSImage *)newImage
{
  return [self initWithImage: newImage
		     hotSpot: NSZeroPoint];
}

/**<p>Initializes and returns a new NSCursor with a NSImage <var>newImage</var>
   and the hot spot to <var>hotSpot</var>.</p>
   <p>NB. The coordinate system of an NSCursor is flipped, so a hotSpot at
   0,0 is in the top left corner of the cursor.</p>
   <p>See Also: -initWithImage: -setImage:</p>
 */
- (id) initWithImage: (NSImage *)newImage hotSpot: (NSPoint)hotSpot
{
  //_cursor_flags.is_set_on_mouse_entered = NO;
  //_cursor_flags.is_set_on_mouse_exited = NO;
  _hot_spot = hotSpot;
  [self setImage: newImage];

  return self;
}

- (id)initWithImage:(NSImage *)newImage 
foregroundColorHint:(NSColor *)fg 
backgroundColorHint:(NSColor *)bg
	    hotSpot:(NSPoint)hotSpot
{
  self = [self initWithImage: newImage hotSpot: hotSpot];
  if (self == nil)
    return nil;

  if (fg || bg)
    {
      if (bg == nil)
	bg = [NSColor whiteColor];
      if (fg == nil)
	fg = [NSColor blackColor];
      bg = [bg colorUsingColorSpaceName: NSDeviceRGBColorSpace];
      fg = [fg colorUsingColorSpaceName: NSDeviceRGBColorSpace];
      [GSCurrentServer() recolorcursor: fg : bg : _cid];
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_cursor_image);
  if (_cid)
    {
      [GSCurrentServer() freecursor: _cid];
    }
  [super dealloc];
}

/**<p>Returns the hot spot point of the NSCursor.  This is in the
 * cursor's coordinate system which is a flipped on (origin at the
 * top left of the cursor).</p>
 */
- (NSPoint) hotSpot
{
  // FIXME: This wont work for the standard cursor
  return _hot_spot;
}

/**<p>Returns the image of the NSCursor</p>
 */
- (NSImage*) image
{
  // FIXME: This wont work for the standard cursor
  return _cursor_image;
}

/**<p>Sets the hot spot point of the NSCursor to <var>spot</var></p>
 */
- (void) setHotSpot: (NSPoint)spot
{
  _hot_spot = spot;
  [self _computeCid];
}

/**<p>Sets <var>newImage</var> the image of the NSCursor</p>
 */
- (void) setImage: (NSImage *)newImage
{
  ASSIGN(_cursor_image, newImage);
  [self _computeCid];
}

/** <p>Returns whether if the cursor is set on -mouseEntered:.
    </p><p>See Also: -setOnMouseEntered: -mouseEntered: -set 
    -isSetOnMouseExited</p> 
 */
- (BOOL) isSetOnMouseEntered
{
  return _cursor_flags.is_set_on_mouse_entered;
}

/** <p>Returns whether if the cursor is push on -mouseExited:.
    </p><p>See Also: -setOnMouseEntered: -mouseExited: -set 
    -isSetOnMouseEntered</p>
 */
- (BOOL) isSetOnMouseExited
{
  return _cursor_flags.is_set_on_mouse_exited;
}

/**<p>Sets the cursor if -isSetOnMouseEntered is YES or pushs the cursor
   if -isSetOnMouseExited is NO</p>
   <p>See Also: -isSetOnMouseEntered -isSetOnMouseExited</p>
 */
- (void) mouseEntered: (NSEvent*)theEvent
{
  NSDebugLLog(@"NSCursor", @"Cursor mouseEntered:enter %d exit %d",
              _cursor_flags.is_set_on_mouse_entered, _cursor_flags.is_set_on_mouse_exited);
  if (_cursor_flags.is_set_on_mouse_entered == YES)
    {
      [self set];
    }
  else if (_cursor_flags.is_set_on_mouse_exited == NO)
    {
      /*
       * Undocumented behavior - if a cursor is not set on exit or entry,
       * we assume a push-pop situation instead.
       */
      [self push];
    }
}

/**<p>Sets the cursor if -isSetOnMouseExited is YES or pops the cursor
   if -isSetOnMouseEntered is NO</p>
   <p>See Also: -isSetOnMouseExited -isSetOnMouseEntered </p>
 */
- (void) mouseExited: (NSEvent*)theEvent
{
  NSDebugLLog(@"NSCursor", @"Cursor mouseExited: enter %d exit %d",
              _cursor_flags.is_set_on_mouse_entered, _cursor_flags.is_set_on_mouse_exited);
  if (_cursor_flags.is_set_on_mouse_exited == YES)
    {
      [self set];
    }
  else if (_cursor_flags.is_set_on_mouse_entered == NO)
    {
      /*
       * Undocumented behavior - if a cursor is not set on exit or entry,
       * we assume a push-pop situation instead.
       */
      [self pop];
    }
  else
    {
      /*
       * The cursor was set on entry, we reset it to the default cursor on exit. 
       * Using push and pop all the time would seem to be a clearer way.
       */
      [[NSCursor arrowCursor] set];
    }
}

/**<p>Pops the cursor off the top of the stack and makes the previous
   the current cursor.</p>
 */
- (void) pop
{
  [NSCursor_class pop];
}

/**<p>Adds the NSCursor into the cursor stack and makes it the current 
   cursor.</p><p>See Also: -pop -set</p>
 */
- (void) push
{
  [gnustep_gui_cursor_stack addObject: gnustep_gui_current_cursor];
  [self set];
  NSDebugLLog(@"NSCursor", @"Cursor push %p", _cid);
}

/**<p>Sets the NSCursor as the current cursor.</p>
 */
- (void) set
{
  ASSIGN(gnustep_gui_current_cursor, self);
  if (_cid)
    {
      [GSCurrentServer() setcursor: _cid];
    }
  else
    {
      /*
       * No image? This is odd, so we set an standard
       * cursor image (GSArrowCursor).
       */
      void *c = NULL;
      [GSCurrentServer() standardcursor: GSArrowCursor : &c];
      if (c != NULL)
	{
	  [self _setCid: c];
	  [GSCurrentServer() setcursor: _cid];
	}
    }
}

/** <p>Sets whether if the cursor is set on -mouseEntered:.
    </p><p>See Also: -isSetOnMouseEntered -mouseEntered: -set</p> 
 */
- (void) setOnMouseEntered: (BOOL)flag
{
  _cursor_flags.is_set_on_mouse_entered = flag;
}

/** <p>Sets whether if the cursor is push on -mouseExited:.
    </p><p>See Also: -isSetOnMouseExited -mouseExited: -set</p>
 */
- (void) setOnMouseExited: (BOOL)flag
{
  _cursor_flags.is_set_on_mouse_exited = flag;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      if (nil != _cursor_image)
        {
          [aCoder encodeObject: _cursor_image forKey: @"NSImage"];
        }
      else
        {
          int type = _cursor_flags.type;
          [aCoder encodeInt: type forKey: @"NSCursorType"];
        }
      [aCoder encodePoint: _hot_spot forKey: @"NSHotSpot"];
    }
  else
    {
      BOOL flag;

      // FIXME: This wont work for the standard cursor
      flag = _cursor_flags.is_set_on_mouse_entered;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cursor_flags.is_set_on_mouse_exited;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      [aCoder encodeObject: _cursor_image];
      [aCoder encodePoint: _hot_spot];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      DESTROY(self);
      if ([aDecoder containsValueForKey: @"NSCursorType"])
        {
	  int type = [aDecoder decodeIntForKey: @"NSCursorType"];

          switch (type)
            {
            case 0:
              self = [NSCursor arrowCursor];
              break;
            case 1:
              self = [NSCursor IBeamCursor];
              break;
            case 2:
              self = [NSCursor dragLinkCursor];
              break;
            case 3:
              self = [NSCursor operationNotAllowedCursor];
              break;
            case 5:
              self = [NSCursor dragCopyCursor];
              break;
            case 11:
              self = [NSCursor closedHandCursor];
              break;
            case 12:
              self = [NSCursor openHandCursor];
              break;
            case 13:
              self = [NSCursor pointingHandCursor];
              break;
            case 17:
              self = [NSCursor resizeLeftCursor];
              break;
            case 18:
              self = [NSCursor resizeRightCursor];
              break;
            case 19:
              self = [NSCursor resizeLeftRightCursor];
              break;
            case 20:
              self = [NSCursor crosshairCursor];
              break;
            case 21:
              self = [NSCursor resizeUpCursor];
              break;
            case 22:
              self = [NSCursor resizeDownCursor];
              break;
            case 24:
              self = [NSCursor contextualMenuCursor];
              break;
            case 25:
              self = [NSCursor disappearingItemCursor];
              break;
            default:
              // FIXME
              self = [NSCursor arrowCursor];
              break;
            }
          RETAIN(self);
	}
      else
        {
          NSPoint hotSpot = NSMakePoint(0, 0);
          NSImage *image = nil;

          if ([aDecoder containsValueForKey: @"NSHotSpot"])
            {
              hotSpot = [aDecoder decodePointForKey: @"NSHotSpot"];
            }
          if ([aDecoder containsValueForKey: @"NSImage"])
            {
              image = [aDecoder decodeObjectForKey: @"NSImage"];
            }
          
          self = [[NSCursor alloc] initWithImage: image
                                         hotSpot: hotSpot];
        }
    }
  else
    {
      BOOL flag;

      [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                   at: &flag];
      _cursor_flags.is_set_on_mouse_entered = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                   at: &flag];
      _cursor_flags.is_set_on_mouse_exited = flag;
      _cursor_image = [aDecoder decodeObject];
      _hot_spot = [aDecoder decodePoint];
      [self _computeCid];
    }
  return self;
}

@end
