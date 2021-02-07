/** <title>NSInterfaceStyle</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date:	1999
   
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

#include "config.h"
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSMapTable.h>

#import "AppKit/NSResponder.h"
#import "AppKit/NSInterfaceStyle.h"
#import "GNUstepGUI/GSTheme.h"

NSString	*NSInterfaceStyleDefault = @"NSInterfaceStyleDefault";

static NSMapTable	*styleMap = 0;
static NSInterfaceStyle	defStyle;

static NSInterfaceStyle
styleFromString(NSString* str)
{
  if ([str isEqualToString: @"NSNextStepInterfaceStyle"])
    return NSNextStepInterfaceStyle;
  if ([str isEqualToString: @"NSMacintoshInterfaceStyle"])
    return NSMacintoshInterfaceStyle;
  if ([str isEqualToString: @"NSWindows95InterfaceStyle"])
    return NSWindows95InterfaceStyle;
  if ([str isEqualToString: @"GSWindowMakerInterfaceStyle"])
    return GSWindowMakerInterfaceStyle;
  return NSNoInterfaceStyle;
}

@interface	GSInterfaceStyle : NSObject
+ (void) defaultsDidChange: (NSNotification*)notification;
@end



/*
typedef struct {
  @defs(NSResponder)
} *accessToResponder;
*/
typedef NSResponder* accessToResponder;

/**
   <p>
   Returns the interface style the responder should use, which affects
   how a UI element (such as a button or menu) is displayed. If the 
   responder has an interface style set, the key is ignored and the
   responder's interface style is returned. Otherwise the style
   associated with the key is returned (if set), otherwise the default
   style is returned. In no case will the style <code>NSNoInterfaceStyle</code>
   be returned.
   </p>
   <p>
   Styles can be set using the user defaults system. Currently available
   styles are 
   </p>
   <list>
     <item>NSNextStepInterfaceStyle</item>
     <item>NSMacintoshInterfaceStyle</item> 
     <item>NSWindows95InterfaceStyle</item> 
     <item>GSWindowMakerInterfaceStyle</item>
   </list>
   <p>
   You can set a default style
   for all UI elements using the <code>NSInterfaceStyleDefault</code> key:
   </p>
   <example>
   defaults write NSGlobalDomain NSInterfaceStyleDefault GSWindowMakerInterfaceStyle 
   </example>
*/
extern  NSInterfaceStyle
NSInterfaceStyleForKey(NSString *key, NSResponder *responder)
{
  NSInterfaceStyle	style;

  /*
   *	If the specified responder has a style set, return it.
   */
  if (responder)
    {
      style
	= (NSInterfaceStyle)((accessToResponder)responder)->_interface_style;
      if (style != NSNoInterfaceStyle)
	{
	  return style;
	}
    }

  /*
   *	If there is no style map, the defaults/cache management class must be
   *	initialised.
   */
  if (styleMap == 0)
    [GSInterfaceStyle class];

  /*
   *	If there is a style for the given defaults key, return it - after
   *	caching it in a map table if necessary.
   */
  if (key)
    {
      /*
       *	First try the cache - then, if no style is found,  use the
       *	defaults system and add the results into the cache.
       */
      style = (NSInterfaceStyle)NSMapGet(styleMap, key);
      if (style == NSNoInterfaceStyle)
	{
	  NSUserDefaults	*defs;
	  NSString		*def;
      
	  defs = [NSUserDefaults standardUserDefaults];
	  def = [defs stringForKey: key];
	  if (def == nil
	    || (style = styleFromString(def)) == NSNoInterfaceStyle)
	    {
	      style = defStyle;
	    }
	  if (style != NSNoInterfaceStyle)
	    NSMapInsert(styleMap, (void*)key, (void*)style);
	}
      return style;
    }

  /*
   *	No responder and no key - return the default style.
   */
  return defStyle;
}



/*
 *	The GSInterfaceStyle class is used solely to maintain our map of
 *	know interface styles by updating when user defaults change.
 */
@implementation	GSInterfaceStyle

+ (void) initialize
{
  if (self == [GSInterfaceStyle class])
    {
      styleMap = NSCreateMapTable(NSObjectMapKeyCallBacks,
			     NSIntMapValueCallBacks, 8);

      [NSUserDefaults standardUserDefaults];
      [self defaultsDidChange: nil];
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	   selector: @selector(defaultsDidChange:)
	       name: NSUserDefaultsDidChangeNotification
	     object: nil];
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	   selector: @selector(defaultsDidChange:)
	       name: GSThemeDidActivateNotification
	     object: nil];
    }
}

+ (void) defaultsDidChange: (NSNotification*)notification
{
  NSUserDefaults	*defs;
  NSMapEnumerator	enumerator;
  NSString		*key;
  void                  *val;

  /*
   * We ignore the actual notification, which may be nil (when called at
   * initialization), or may contain a user defaults object (if a persistent
   * domain changed), or may contain a theme object (if a theme activated).
   * What we need to do is examine the current state of the standard defaults.
   */
  defs = [NSUserDefaults standardUserDefaults];

  /*
   *	Determine the default interface style for the application.
   */
  key = [defs stringForKey: NSInterfaceStyleDefault];
  if (key == nil || (defStyle = styleFromString(key)) == NSNoInterfaceStyle)
    defStyle = NSNextStepInterfaceStyle;
    
  /*
   *	Now check the interface styles for all the keys in use and adjust our
   *	map table for any changes.
   */
  enumerator = NSEnumerateMapTable(styleMap);
  while (NSNextMapEnumeratorPair(&enumerator, (void**)&key, (void**)&val))
    {
      NSInterfaceStyle	newStyle;
      NSString		*def = [defs stringForKey: key];

      if (def == nil)
	{
	  newStyle = defStyle;
	}
      else
	{
	  newStyle = styleFromString(def);
	  if (newStyle == NSNoInterfaceStyle)
	    {
	      newStyle = defStyle;
	    }
	}

      if (newStyle != ((NSInterfaceStyle)val))
	{
	  NSMapInsert(styleMap, (void*)key, (void*)newStyle);
	}
    }
}

@end

