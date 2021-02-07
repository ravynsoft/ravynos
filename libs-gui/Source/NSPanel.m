/** <title>NSPanel</title>

   <abstract>Panel window class and related functions</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

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

#import <Foundation/NSCoder.h>
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSTextField.h"

@implementation	NSPanel

/*
 * Class methods
 */
+ (void)initialize
{
  if (self  ==  [NSPanel class])
    {
      [self setVersion: 1];
    }
}

/*
 * Instance methods
 */
- (id) init
{
  int style =  NSTitledWindowMask | NSClosableWindowMask;

  return [self initWithContentRect: NSZeroRect
			 styleMask: style
			   backing: NSBackingStoreBuffered
			     defer: NO];
}

- (id) initWithContentRect: (NSRect)contentRect
                 styleMask: (NSUInteger)aStyle
                   backing: (NSBackingStoreType)bufferingType
                     defer: (BOOL)flag
{
  self = [super initWithContentRect: contentRect
			  styleMask: aStyle
			    backing: bufferingType
			      defer: flag];
  if (nil == self)
    {
      return nil;
    }

  if ((_styleMask & NSUtilityWindowMask) == NSUtilityWindowMask)
    {
      [self setFloatingPanel: YES];
    }

  return self;
}

- (void) _initDefaults
{
  [super _initDefaults];
  [self setReleasedWhenClosed: NO];
  [self setHidesOnDeactivate: YES];
  [self setExcludedFromWindowsMenu: YES];
}

- (BOOL) canBecomeKeyWindow
{
  return YES;
}

- (BOOL) canBecomeMainWindow
{
  return NO;
}

/*
 * If we receive an escape, close.
 */
- (void) keyDown: (NSEvent*)theEvent
{
  if ([@"\e" isEqual: [theEvent charactersIgnoringModifiers]]
     &&  ([self styleMask] & NSClosableWindowMask)  ==  NSClosableWindowMask)
    [self close];
  else
    [super keyDown: theEvent];
}

/**<p>Returns whether the NSPanel is a floating panel, e.g. the window level
   is NSFloatingWindowLevel instead of NSNormalWindowLevel.</p>
   <p>See Also:  -setFloatingPanel: </p>
 */
- (BOOL) isFloatingPanel
{
  return _isFloatingPanel;
}

/**<p>Sets whether the NSPanel is a floating panel, e.g. the window level
   is NSFloatingWindowLevel instead of NSNormalWindowLevel.</p>
   <p>See Also:  -isFloatingPanel [NSWindow-setLevel:]</p>
 */
- (void) setFloatingPanel: (BOOL)flag
{
  if (_isFloatingPanel != flag)
    {
      _isFloatingPanel = flag;
      if (flag == YES)
	{
	  [self setLevel: NSFloatingWindowLevel];
	}
      else
	{
	  [self setLevel: NSNormalWindowLevel];
	}
    }
}

/**<p>Returns whether the NSPanel can receive events when another window/panel
   runs modally.</p><p>See Also: -setWorksWhenModal:
   [NSApplication-runModalSession:]</p>
 */
- (BOOL) worksWhenModal
{
  return _worksWhenModal;
}

/**<p>Sets whether the NSPanel can receive events when another window/panel
   runs modally.</p>See Also: -worksWhenModal [NSApplication-runModalSession:]
 */
- (void) setWorksWhenModal: (BOOL)flag
{
  _worksWhenModal = flag;
}

/**<p>Returns whether if the NSPanel becomes key window only when a view
   require to be the first responder.</p>
   <p>See Also: -setBecomesKeyOnlyIfNeeded: [NSView-needsPanelToBecomeKey]
   [NSWindow-sendEvent:]</p>
 */
- (BOOL) becomesKeyOnlyIfNeeded
{
  return _becomesKeyOnlyIfNeeded;
}

/**<p>Sets whether if the NSPanel becomes key window only when a view
   require to be the first responder.</p>
   <p>See Also: -setBecomesKeyOnlyIfNeeded: [NSView-needsPanelToBecomeKey]
   [NSWindow-sendEvent:]</p>
 */
- (void) setBecomesKeyOnlyIfNeeded: (BOOL)flag
{
  _becomesKeyOnlyIfNeeded = flag;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  BOOL	flag;

  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      // Nothing to do here, for keyed coding this is handled by NSWindowTemplate.
      // Calling the above method should throw an NSInvalidArgumentException.
    }
  else
    {
       flag = _becomesKeyOnlyIfNeeded;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _isFloatingPanel;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _worksWhenModal;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  BOOL	flag;

  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      // Nothing to do here, for keyed coding this is handled by NSWindowTemplate.
      // Calling the above method should throw an NSInvalidArgumentException.
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setBecomesKeyOnlyIfNeeded: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setFloatingPanel: flag];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      [self setWorksWhenModal: flag];
    }

  return self;
}

@end /* NSPanel */

