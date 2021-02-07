/** <title>NSScreen</title>

   Copyright (C) 1996, 2000 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Major modifications and updates
   Author: Gregory John Casamento <borgheron@yahoo.com>
   Date: 2000
   
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
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSInterfaceStyle.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSDisplayServer.h"

@interface NSScreen (Private)
- (id) _initWithScreenNumber: (int)screen;
@end

@implementation NSScreen

/*
 * Class methods
 */

+ (void) initialize
{
  if (self == [NSScreen class])
    {
      [self setVersion: 1];
    }
}

static NSMutableArray *screenArray = nil;

/**
 * Resets the cached list of screens.
 */
+ (void) resetScreens
{
  DESTROY(screenArray);
}

/**
 * Returns an NSArray containing NSScreen instances representing all of the
 * screen devices attached to the computer.
 */
+ (NSArray*) screens
{
  int count = 0, index = 0;
  NSArray *screens;
  GSDisplayServer *srv;

  if (screenArray != nil)
    return screenArray;

  srv = GSCurrentServer();
  screens = [srv screenList];
  count = [screens count];
  if (count == 0)
    {
      // something is wrong. This shouldn't happen.
      [NSException raise: NSWindowServerCommunicationException
                   format: @"Unable to retrieve list of screens from window server."];
      return nil;
    }

  screenArray = [NSMutableArray new];

  // Iterate over the list
  for (index = 0; index < count; index++)
    {
      NSScreen *screen = nil;
      
      screen = [[NSScreen alloc] _initWithScreenNumber: 
                                     [[screens objectAtIndex: index] intValue]];
      [screenArray addObject: screen];
      RELEASE(screen);
    }

  return [NSArray arrayWithArray: screenArray];
}

// Creating NSScreen Instances
/**
 * Gets information about the main screen.
 */
+ (NSScreen*) mainScreen
{
  NSWindow *keyWindow;

  keyWindow = [NSApp keyWindow];
  if (keyWindow == nil)
    {
      keyWindow = [[NSApp mainMenu] window];
    }
  
  if (keyWindow != nil)
    {
      return [keyWindow screen];
    }
  else
    {
      NSArray *screenArray = [self screens];

      if (screenArray != nil)
        {
          return [screenArray objectAtIndex: 0];
        }
      else
        {
          return nil;
        }
    }
}

/**
 * Gets information about the screen with the highest depth (i.e. bits per pixel).
 */
+ (NSScreen*) deepestScreen
{
  NSArray *screenArray = [self screens];
  NSEnumerator *screenEnumerator = nil;
  NSScreen *deepestScreen = nil, *screen = nil;  
  int maxBits = 0;

  // Iterate over the list of screens and find the
  // one with the most depth.
  screenEnumerator = [screenArray objectEnumerator];
  while ((screen = [screenEnumerator nextObject]) != nil)
    {
      int bits = 0;
      
      bits = [screen depth];
      
      if (bits > maxBits)
        {
          maxBits = bits;
          deepestScreen = screen;
        }
    }

  return deepestScreen;
}


/*
 * Instance methods
 */

/**
 * NSScreen does not respond to the init method.
 */
- (id) init
{
  [self doesNotRecognizeSelector: _cmd];
  return nil;
}

/**
 * Get all of the infomation for a given screen.
 */
- (id) _initWithScreenNumber: (int)screen
{
  GSDisplayServer *srv;

  if (!(self = [super init]))
    {
      return nil;
    }

  // Check for problems
  if (screen < 0)
    {
      NSLog(@"Internal error: Invalid screen number %d\n", screen);
      RELEASE(self);
      return nil;
    }

  srv = GSCurrentServer();
  if (srv == nil)
    {
      NSLog(@"Internal error: No current context\n");
      RELEASE(self);
      return nil;
    }

  // Fill in all of the i-vars with appropriate values.
  _screenNumber = screen;
  _frame = [srv boundsForScreen: _screenNumber];
  _depth = [srv windowDepthForScreen: _screenNumber];
  _supportedWindowDepths = NULL;

  return self;
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    return YES;
  if ([anObject isKindOfClass: [self class]] == NO)
    return NO;
  if (_screenNumber == ((NSScreen *)anObject)->_screenNumber)
    return YES;
  return NO;
}

/**
 * Returns the depth of the screen in bits.
 */
- (NSWindowDepth) depth
{
  return _depth;
}

/**
 * The full frame of the screen.
 */
- (NSRect) frame
{
  return _frame;
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ number: %ld frame: %@", 
                   [super description], (long)_screenNumber,
                   NSStringFromRect(_frame)];
}


/**
 * <p>
 * This method generates a dictionary containing information
 * about the screen device.  The resulting dictionary will have the following
 * entires: NSScreenNumber, NSDeviceSize, NSDeviceResolution, NSDeviceBitsPerSample,
 * NSDeviceColorSpaceName.
 * </p>
 */
- (NSDictionary*) deviceDescription
{
  if (_reserved == 0)
    {
      NSMutableDictionary	*devDesc;
      int			bps = 0;
      NSSize			screenResolution;
      NSString			*colorSpaceName = nil;
      CGFloat                   scaleFactor;

      /*
       * This method generates a dictionary from the
       * information we have gathered from the screen.
       */

      // Set the screen number in the current object.
      devDesc = [[NSMutableDictionary alloc] initWithCapacity: 8];
      [devDesc setObject: [NSNumber numberWithInt: _screenNumber]
		  forKey: @"NSScreenNumber"];

      // This is assumed since we are in NSScreen.
      [devDesc setObject: @"YES"  forKey: NSDeviceIsScreen];

      // Add the NSDeviceSize dictionary item
      [devDesc setObject: [NSValue valueWithSize: _frame.size]
		  forKey: NSDeviceSize];

      // Add the NSDeviceResolution dictionary item
      scaleFactor = [self userSpaceScaleFactor];
      screenResolution = NSMakeSize(72.0 * scaleFactor, 72.0 * scaleFactor);
      [devDesc setObject: [NSValue valueWithSize: screenResolution]
		  forKey: NSDeviceResolution];

      // Add the bits per sample entry
      bps = NSBitsPerSampleFromDepth(_depth);
      [devDesc setObject: [NSNumber numberWithInt: bps]
		  forKey: NSDeviceBitsPerSample];

      // Add the color space entry.
      colorSpaceName = NSColorSpaceFromDepth(_depth);
      [devDesc setObject: colorSpaceName
		  forKey: NSDeviceColorSpaceName];
		    
      _reserved = (void*)[devDesc copy];
      RELEASE(devDesc);
    }
  return (NSDictionary*)_reserved;
}

// Mac OS X methods
/**
 * Returns the window depths this screen will support.
 */
- (const NSWindowDepth*) supportedWindowDepths
{
  // If the variable isn't initialized, get the info and
  // store it for the future.
  if (_supportedWindowDepths == NULL)
    {
      _supportedWindowDepths = 
        (NSWindowDepth*)[GSCurrentServer()
			       availableDepthsForScreen: _screenNumber];

      // Check the results
      if (_supportedWindowDepths == NULL)
        {
          NSLog(@"Internal error: no depth list returned from window server.");
          return NULL;
        }
    }

  return _supportedWindowDepths;
}

/**
 * Returns the NSRect representing the visible area of the screen.
 */
- (NSRect) visibleFrame
{
  NSRect visFrame = _frame;
  float menuHeight;

  switch (NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil))
    {
      case NSMacintoshInterfaceStyle:
        if ([NSApp mainMenu] == nil)
          {
            // No menu yet ... assume a standard height
            menuHeight = [NSMenuView menuBarHeight];
          }
        else
          {
            menuHeight = [[[NSApp mainMenu] window] frame].size.height;
          }
        
        visFrame.size.height -= menuHeight;
        break;

      case GSWindowMakerInterfaceStyle:
      case NSNextStepInterfaceStyle:
        /* FIXME: Menu width will vary from app to app and  there is no
         * fixed position for the menu ... should we be making room for
         * a menu top left, or something else?
         */
#if 0
        if ([NSApp mainMenu] != nil)
          {
            float menuWidth = [[[NSApp mainMenu] window] frame].size.width;
      
            visFrame.size.width -= menuWidth;
            visFrame.origin.x += menuWidth;
          }
#endif
        break;
      
      default:
        break;
    }
  return visFrame;
}

/** Returns the screen number */
- (int) screenNumber
{
  return _screenNumber;
}

// Release the memory for the depths array.
- (void) dealloc
{
  // _supportedWindowDepths can be NULL since it may or may not
  // be necessary to get this info.  The most common use of NSScreen
  // is to get the depth and frame attributes.
  if (_supportedWindowDepths != NULL)
    {
      NSZoneFree(NSDefaultMallocZone(), _supportedWindowDepths);
    }
  if (_reserved != 0)
    {
      [(id)_reserved release];
    }
  [super dealloc];
}

- (CGFloat) userSpaceScaleFactor
{
  NSNumber *factor = [[NSUserDefaults standardUserDefaults]
		       objectForKey: @"GSScaleFactor"];

  if (factor != nil)
    {
      return [factor floatValue];
    }
  else
    {    
      GSDisplayServer *srv = GSCurrentServer();
      if (srv != nil)
      {
        NSSize dpi = [GSCurrentServer() resolutionForScreen: _screenNumber];
        // average the width and height
        return (dpi.width + dpi.height) / 144.0;
      }
      else
      {
        return 1.0;
      }
    }
}

- (CGFloat) backingScaleFactor
{
  return 1.0;
}

@end


