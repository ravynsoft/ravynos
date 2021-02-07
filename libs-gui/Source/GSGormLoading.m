/** <title>GSGormLoading</title>

   <abstract>Contains all of the private classes used in .gorm files.</abstract>

   Copyright (C) 2003 Free Software Foundation, Inc.

   Author: Gregory John Casamento
   Date: July 2003.
   
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

#import <Foundation/NSCoder.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSArchiver.h>
#import <Foundation/NSSet.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSNibConnector.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSGormLoading.h"
#import "NSDocumentFrameworkPrivate.h"

static const int currentVersion = 1; // GSNibItem version number...

@interface NSObject (GSNibPrivateMethods)
- (BOOL) isInInterfaceBuilder;
@end

/*
 * This private class is used to collect the nib items while the 
 * .gorm file is being unarchived.  This is done to allow only
 * the top level items to be retained in a clean way.  The reason it's
 * being done this way is because old .gorm files don't have any
 * array within the nameTable which indicates the objects which are
 * considered top level, so there is no clean and generic way to determine
 * this.   Basically the top level items are any instances of or instances
 * of subclasses of NSMenu, NSWindow, or any controller class.
 * It's the last one that's hairy.  Controller classes are
 * represented in .gorm files by the GSNibItem class, but once they transform
 * into the actual class instance it's not easy to tell if it should be 
 * retained or not since there are a lot of other things stored in the nameTable
 * as well.  GJC
 */

static NSString *GSInternalNibItemAddedNotification = @"_GSInternalNibItemAddedNotification";

@interface GSNibItemCollector : NSObject
{
  NSMutableArray *items;
}
- (void) handleNotification: (NSNotification *)notification;
- (NSMutableArray *)items;
@end

@implementation GSNibItemCollector
- (void) handleNotification: (NSNotification *)notification;
{
  id obj = [notification object];
  [items addObject: obj];
}

- init
{
  if ((self = [super init]) != nil)
    {
      NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

      // add myself as an observer and initialize the items array.
      [nc addObserver: self
	  selector: @selector(handleNotification:)
	  name: GSInternalNibItemAddedNotification 
	  object: nil];
      items = [[NSMutableArray alloc] init];
    }
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  RELEASE(items);
  [super dealloc];
}

- (NSMutableArray *)items
{
  return items;
}
@end

/*
 *	The GSNibContainer class manages the internals of a nib file.
 */
@implementation GSNibContainer

+ (void) initialize
{
  if (self == [GSNibContainer class])
    {
      [self setVersion: GNUSTEP_NIB_VERSION];
    }
}

- (void) awakeWithContext: (NSDictionary *)context
{
  if (isAwake == NO)
    {
      NSEnumerator	*enumerator;
      NSNibConnector	*connection;
      NSString		*key;
      NSMenu		*menu;
      NSMutableArray    *topObjects; 
      id                 obj;

      // Add these objects with there old names as the code expects them
      context = AUTORELEASE([context mutableCopyWithZone: [context zone]]);

      isAwake = YES;
      /*
       *	Add local entries into name table.
       */
      if ([context count] > 0)
	{
	  [nameTable addEntriesFromDictionary: context];
	}

      /*
       *	Now establish all connections by taking the names
       *	stored in the connection objects, and replaciong them
       *	with the corresponding values from the name table
       *	before telling the connections to establish themselves.
       */
      enumerator = [connections objectEnumerator];
      while ((connection = [enumerator nextObject]) != nil)
	{
	  id	val;

	  val = [nameTable objectForKey: [connection source]];
	  [connection setSource: val];
	  val = [nameTable objectForKey: [connection destination]];
	  [connection setDestination: val];
	  [connection establishConnection];
	}

      /*
       * See if there is a main menu to be set.  Report #4815, mainMenu 
       * should be initialized before awakeFromNib is called.
       */
      menu = [nameTable objectForKey: @"NSMenu"];
      if (menu != nil && [menu isKindOfClass: [NSMenu class]] == YES)
	{
	  [NSApp setMainMenu: menu];
	}

      /*
       * Set the Services menu.
       * Report #5205, Services/Window menu does not behave correctly.
       */
      menu = [nameTable objectForKey: @"NSServicesMenu"];
      if (menu != nil && [menu isKindOfClass: [NSMenu class]] == YES)
	{
	  [NSApp setServicesMenu: menu];
	}

      /*
       * Set the Services menu.
       * Report #5205, Services/Window menu does not behave correctly.
       */
      menu = [nameTable objectForKey: @"NSWindowsMenu"];
      if (menu != nil && [menu isKindOfClass: [NSMenu class]] == YES)
	{
	  [NSApp setWindowsMenu: menu];
	}

      /*
       * Set the Recent Documents menu.
       */
      menu = [nameTable objectForKey: @"NSRecentDocumentsMenu"];
      if (menu != nil && [menu isKindOfClass: [NSMenu class]] == YES)
	{
	  [[NSDocumentController sharedDocumentController]
	      _setRecentDocumentsMenu: menu];
	}


      /* 
       * See if the user has passed in the NSNibTopLevelObjects key.
       * This is an implementation of a commonly used feature to give access to
       * all top level objects of a nib file.
       */
      obj = [context objectForKey: NSNibTopLevelObjects];
      if ([obj isKindOfClass: [NSMutableArray class]])
	{
	  topObjects = obj;
	}
      else
	{
	  topObjects = nil; 
	}


      /*
       * Now tell all the objects that they have been loaded from
       * a nib.
       */
      enumerator = [nameTable keyEnumerator];
      while ((key = [enumerator nextObject]) != nil)
	{
	  if ([context objectForKey: key] == nil || 
	      [key isEqualToString: NSNibOwner]) // we want to send the message to the owner
	    {
	      // we don't want to send a message to these menus twice, if they're custom classes. 
	      if ([key isEqualToString: @"NSWindowsMenu"] == NO && 
		  [key isEqualToString: @"NSServicesMenu"] == NO && 
		  [key isEqualToString: NSNibTopLevelObjects] == NO)
		{
		  id o = [nameTable objectForKey: key];

		  // send the awake message, if it responds...
		  if ([o respondsToSelector: @selector(awakeFromNib)])
		    {
		      [o awakeFromNib];
		    }

		  /*
		   * Retain all "top level" items so that, when the container 
		   * is released, they will remain. The GSNibItems instantiated in the gorm need 
		   * to be retained, since we are deallocating the container.  
		   * We don't want to retain the owner.
		   *
		   * Please note: It is encumbent upon the developer of an application to 
		   * release these objects. Instantiating a window manually or loading in a .gorm 
		   * file are equivalent processes. These objects need to be released in their 
		   * respective controllers. If the developer has used the NSNibTopLevelObjects feature, 
		   * then she will get the objects back in an array. She will will have to first release 
                   * all the objects in the array and then the array itself in order to release the 
                   * objects held within.
		   */
		  if ([key isEqualToString: NSNibOwner] == NO)
		    {
		      if ([topLevelObjects containsObject: o]) // anything already designated a top level item..
			{
			  [topObjects addObject: o];
			  // All top level objects must be released by the
			  // caller to avoid leaking, unless they are going
			  // to be released by other nib objects on behalf
			  // of the owner.
			  RETAIN(o);
			}
		    }
		}
	    }
	}
      
      /*
       * See if there are objects that should be made visible.
       * This is the last thing we should do since changes might be made
       * in the awakeFromNib methods which are called on all of the objects.
       */
      if (visibleWindows != nil)
	{
	  unsigned	pos = [visibleWindows count];
	  while (pos-- > 0)
	    {
	      NSWindow *win = [visibleWindows objectAtIndex: pos];
	      [win orderFront: self];
	    }
	}

      /*
       * Now remove any objects added from the context dictionary.
       */
      if ([context count] > 0)
	{
	  [nameTable removeObjectsForKeys: [context allKeys]];
	}
    }
}

- (void) dealloc
{
  RELEASE(nameTable);
  RELEASE(connections);
  RELEASE(topLevelObjects);
  RELEASE(visibleWindows);
  RELEASE(deferredWindows);
  RELEASE(customClasses);
  [super dealloc];
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      nameTable = [[NSMutableDictionary alloc] initWithCapacity: 8];
      connections = [[NSMutableArray alloc] initWithCapacity: 8];
      topLevelObjects = [[NSMutableSet alloc] initWithCapacity: 8];
      customClasses = [[NSMutableDictionary alloc] initWithCapacity: 8];
      deferredWindows = [[NSMutableArray alloc] initWithCapacity: 8];
      visibleWindows = [[NSMutableArray alloc] initWithCapacity: 8];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  int version = [GSNibContainer version];
  if (version == GNUSTEP_NIB_VERSION)
    {
      [aCoder encodeObject: topLevelObjects];
      [aCoder encodeObject: visibleWindows];
      [aCoder encodeObject: deferredWindows];
      [aCoder encodeObject: nameTable];
      [aCoder encodeObject: connections];
      [aCoder encodeObject: customClasses];
    }
  else if (version == 1)
    {
      NSMutableDictionary *nt = [NSMutableDictionary dictionaryWithDictionary: nameTable];
      [nt setObject: [NSMutableArray arrayWithArray: visibleWindows] 
	  forKey: @"NSVisible"];
      [nt setObject: [NSMutableArray arrayWithArray: deferredWindows] 
	  forKey: @"NSDeferred"];
      [nt setObject: [NSMutableDictionary dictionaryWithDictionary: customClasses] 
	  forKey: @"GSCustomClassMap"];
      [aCoder encodeObject: nt];
      [aCoder encodeObject: connections];
      [aCoder encodeObject: topLevelObjects];
    }
  else if (version == 0)
    {
      NSMutableDictionary *nt = [NSMutableDictionary dictionaryWithDictionary: nameTable];
      [nt setObject: [NSMutableArray arrayWithArray: visibleWindows] 
	  forKey: @"NSVisible"];
      [nt setObject: [NSMutableArray arrayWithArray: deferredWindows] 
	  forKey: @"NSDeferred"];
      [nt setObject: [NSMutableDictionary dictionaryWithDictionary: customClasses] 
	  forKey: @"GSCustomClassMap"];
      [aCoder encodeObject: nt];
      [aCoder encodeObject: connections];
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		   format: @"Unable to write GSNibContainer version #%d.  GSNibContainer version for the installed gui lib is %d.", version, GNUSTEP_NIB_VERSION];
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  int version = [aCoder versionForClassName: @"GSNibContainer"]; 

  // save the version to the ivar, we need it later.
  if (version == GNUSTEP_NIB_VERSION)
    {
      [aCoder decodeValueOfObjCType: @encode(id) at: &topLevelObjects];
      [aCoder decodeValueOfObjCType: @encode(id) at: &visibleWindows];
      [aCoder decodeValueOfObjCType: @encode(id) at: &deferredWindows];
      [aCoder decodeValueOfObjCType: @encode(id) at: &nameTable];
      [aCoder decodeValueOfObjCType: @encode(id) at: &connections];
      [aCoder decodeValueOfObjCType: @encode(id) at: &customClasses];
    }
  else if (version == 1)
    {
      [aCoder decodeValueOfObjCType: @encode(id) at: &nameTable];
      [aCoder decodeValueOfObjCType: @encode(id) at: &connections];
      [aCoder decodeValueOfObjCType: @encode(id) at: &topLevelObjects];

      // initialize with special entries...
      ASSIGN(visibleWindows, [NSMutableArray arrayWithArray: 
					       [nameTable objectForKey: @"NSVisible"]]);
      ASSIGN(deferredWindows, [NSMutableArray arrayWithArray: 
						[nameTable objectForKey: @"NSDeferred"]]);
      ASSIGN(customClasses, [NSMutableDictionary dictionaryWithDictionary: 
						   [nameTable objectForKey: @"GSCustomClassMap"]]);

      // then remove them from the name table.
      [nameTable removeObjectForKey: @"NSVisible"];
      [nameTable removeObjectForKey: @"NSDeferred"];
      [nameTable removeObjectForKey: @"GSCustomClassMap"];
    }
  else if (version == 0)
    {
      GSNibItemCollector *nibitems = [[GSNibItemCollector alloc] init];
      NSEnumerator *en;
      NSString *key;
      
      // initialize the set of top level objects...
      topLevelObjects = [[NSMutableSet alloc] initWithCapacity: 8];

      // unarchive...
      [aCoder decodeValueOfObjCType: @encode(id) at: &nameTable];
      [aCoder decodeValueOfObjCType: @encode(id) at: &connections];
      [topLevelObjects addObjectsFromArray: [nibitems items]]; // get the top level items here...
      RELEASE(nibitems);

      // iterate through the objects returned
      en = [nameTable keyEnumerator];
      while ((key = [en nextObject]) != nil)
	{
	  id o = [nameTable objectForKey: key];
	  if (([o isKindOfClass: [NSMenu class]] && [key isEqual: @"NSMenu"]) ||
	     [o isKindOfClass: [NSWindow class]])
	    {
	      [topLevelObjects addObject: o]; // if it's a top level object, add it.
	    }
	}

      // initialize with special entries...
      ASSIGN(visibleWindows, [NSMutableArray arrayWithArray: 
					       [nameTable objectForKey: @"NSVisible"]]);
      ASSIGN(deferredWindows, [NSMutableArray arrayWithArray: 
						[nameTable objectForKey: @"NSDeferred"]]);
      ASSIGN(customClasses, [NSMutableDictionary dictionaryWithDictionary: 
						   [nameTable objectForKey: @"GSCustomClassMap"]]);


      // then remove them from the name table.
      [nameTable removeObjectForKey: @"NSVisible"];
      [nameTable removeObjectForKey: @"NSDeferred"];
      [nameTable removeObjectForKey: @"GSCustomClassMap"];
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		   format: @"Unable to read GSNibContainer version #%d.  GSNibContainer version for the installed gui lib is %d.  Please upgrade to a more recent version of the gui library.", version, GNUSTEP_NIB_VERSION];
    }

  return self;
}

- (NSMutableDictionary*) nameTable
{
  return nameTable;
}

- (NSMutableSet*) topLevelObjects
{
  return topLevelObjects;
}

- (NSMutableArray*) connections
{
  return connections;
}

- (NSMutableArray*) visibleWindows
{
  return visibleWindows;
}

- (NSMutableArray*) deferredWindows
{
  return deferredWindows;
}

- (NSMutableDictionary *) customClasses
{
  return customClasses;
}
@end

// The first standin objects here are for views and normal objects like controllers
// or data sources.
@implementation	GSNibItem
+ (void) initialize
{
  if (self == [GSNibItem class])
    {
      [self setVersion: currentVersion];
    }
}

- (void) dealloc
{
  RELEASE(theClass);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [aCoder encodeObject: theClass];
  [aCoder encodeRect: theFrame];
  [aCoder encodeValueOfObjCType: @encode(unsigned int) 
	  at: &autoresizingMask];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  int version = [aCoder versionForClassName: 
			  NSStringFromClass([self class])];
  id obj = nil;

  if (version == 1)
    {
      Class		cls;
      unsigned int      mask;
      
      [aCoder decodeValueOfObjCType: @encode(id) at: &theClass];
      theFrame = [aCoder decodeRect];
      [aCoder decodeValueOfObjCType: @encode(unsigned int) 
	      at: &mask];
      
      cls = NSClassFromString(theClass);
      if (cls == nil)
	{
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Unable to find class '%@', it is not linked into the application.", theClass];
	}
      
      if (theFrame.size.height > 0 && theFrame.size.width > 0)
	{
	  obj = [[cls allocWithZone: [self zone]] initWithFrame: theFrame];
	}
      else
	{
	  if(GSObjCIsKindOf(cls, [NSApplication class]))
	    {
	      obj = RETAIN([cls sharedApplication]);
	    }
	  else
	    {
	      obj = [[cls allocWithZone: [self zone]] init];
	    }  
	}

      if ([obj respondsToSelector: @selector(setAutoresizingMask:)])
	{
	  [obj setAutoresizingMask: mask];
	}
    }
  else if (version == 0)
    {
      Class		cls;
      
      [aCoder decodeValueOfObjCType: @encode(id) at: &theClass];
      theFrame = [aCoder decodeRect];
      
      cls = NSClassFromString(theClass);
      if (cls == nil)
	{
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Unable to find class '%@', it is not linked into the application.", theClass];
	}
      
      obj = [cls allocWithZone: [self zone]];
      if (theFrame.size.height > 0 && theFrame.size.width > 0)
	{
	  obj = [obj initWithFrame: theFrame];
	}
      else
	{
	  obj = [obj init];
	}
    }
  else
    {
      NSLog(@"no initWithCoder for this version");
    }

  // If this is a nib item and not a custom view, then we need to add it to
  // the set of things to be retained.  Also, the initial version of the nib container
  // needed this code, but subsequent versions don't, so don't send the notification,
  // if the version isn't zero.
  if (obj != nil && [aCoder versionForClassName: NSStringFromClass([GSNibContainer class])] == 0)
    {
      if ([self isKindOfClass: [GSNibItem class]] == YES &&
	 [self isKindOfClass: [GSCustomView class]] == NO)
	{
	  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	  [nc postNotificationName: GSInternalNibItemAddedNotification
	      object: obj];
	}
    }

  // release self and return the object this represents...
  RELEASE(self);
  return obj;
}

@end

@implementation	GSCustomView
+ (void) initialize
{
  if (self == [GSCustomView class])
    {
      [self setVersion: currentVersion];
    }
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  return [super initWithCoder: aCoder];
}
@end

/*
  These stand-ins are here for use by GUI elements within Gorm.   Since each gui element
  has it's own "designated initializer" it's important to provide a division between these
  so that when they are loaded, the application will call the correct initializer. 
  
  Some "tricks" are employed in this code.   For instance the use of initWithCoder and
  encodeWithCoder directly as opposed to using the encodeObjC..  methods is the obvious
  standout.  To understand this it's necessary to explain a little about how encoding itself
  works.

  When the model is saved by the Interface Builder (whether Gorm or another 
  IB equivalent) these classes should be used to substitute for the actual classes.  The actual
  classes are encoded as part of it, but since they are being replaced we can't use the normal
  encode methods to do it and must encode it directly.

  Also, the reason for encoding the superclass itself is that by doing so the unarchiver knows
  what version is referred to by the encoded object.  This way we can replace the object with
  a substitute class which will allow it to create itself as the custom class when read it by
  the application, and using the encoding system to do it in a clean way.
*/
@implementation GSClassSwapper
+ (void) initialize
{
  if (self == [GSClassSwapper class]) 
    { 
      [self setVersion: GSSWAPPER_VERSION];
    }
}

- (id) initWithObject: (id)object className: (NSString *)className superClassName: (NSString *)superClassName
{
  if ((self = [self init]) != nil)
    {
      NSDebugLog(@"Created template %@ -> %@",NSStringFromClass([self class]), className);
      ASSIGN(_object, object);
      ASSIGN(_className, className);
      _superClass = NSClassFromString(superClassName);
      if (_superClass == nil)
	{
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Unable to find class '%@', it is not linked into the application.", superClassName];
	}
    }
  return self;
}

- init
{
  if ((self = [super init]) != nil)
    {
      _className = nil;
      _superClass = nil;
      _object = nil;
    } 
  return self;
}

- (void) dealloc
{
  RELEASE(_object);
  RELEASE(_className);
  [super dealloc];
}

- (void) setClassName: (NSString *)name
{
  ASSIGN(_className, name);
}

- (NSString *)className
{
  return _className;
}

- (id) initWithCoder: (NSCoder *)coder
{
  id obj = nil;
  int version = [coder versionForClassName: @"GSClassSwapper"];
  if (version == 0)
    {
      if ((self = [super init]) != nil)
	{
	  NSUnarchiver *unarchiver = (NSUnarchiver *)coder;

	  // decode class/superclass...
	  [coder decodeValueOfObjCType: @encode(id) at: &_className];  
	  [coder decodeValueOfObjCType: @encode(Class) at: &_superClass];

	  // if we are living within the interface builder app, then don't try to 
	  // morph into the subclass.
	  if ([self shouldSwapClass])
	    {
	      Class aClass = NSClassFromString(_className);
	      if (aClass == nil)
		{
		  [NSException raise: NSInternalInconsistencyException
			       format: @"Unable to find class '%@', it is not linked into the application.", _className];
		}
	  
	      // Initialize the object...  dont call decode, since this wont 
	      // allow us to instantiate the class we want. 
	      obj = [aClass alloc];
	    }
	  else
	    {
	      obj = [_superClass alloc];
	    }

	  // inform the coder that this object is to replace the template in all cases.
	  [unarchiver replaceObject: self withObject: obj];
	  obj = [obj initWithCoder: coder]; // unarchive the object... 
	}
    }

  // change the class of the instance to the one we want to see...
  return obj;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  [aCoder encodeValueOfObjCType: @encode(id) at: &_className];  
  [aCoder encodeValueOfObjCType: @encode(Class) at: &_superClass];

  if (_object != nil)
    {
      // Don't call encodeValue, the way templates are used will prevent
      // it from being saved correctly.  Just call encodeWithCoder directly.
      [_object encodeWithCoder: aCoder]; 
    }
}

- (BOOL) shouldSwapClass
{
  BOOL result = YES;
  if ([self respondsToSelector: @selector(isInInterfaceBuilder)])
    {
      result = !([self isInInterfaceBuilder]);
    }
  return result;
}
@end

@implementation GSWindowTemplate
+ (void) initialize
{
  if (self == [GSWindowTemplate class]) 
    { 
      [self setVersion: GSWINDOWT_VERSION];
    }
}

- (unsigned int) autoPositionMask
{
  return _autoPositionMask;
}

- (void) setAutoPositionMask: (unsigned int)flag
{
  _autoPositionMask = flag;
}

- (BOOL) deferFlag
{
  return _deferFlag;
}

- (void) setDeferFlag: (BOOL)flag
{
  _deferFlag = flag;
}

- (void) autoPositionWindow: (NSWindow *)window
{
  int		options = 0;
  NSRect        currentScreenFrame = [[window screen] frame];
  NSRect        windowFrame = [window frame];
  NSPoint       origin  = windowFrame.origin;
  NSSize	newSize = currentScreenFrame.size;
  NSSize        oldSize = _screenRect.size;
  BOOL		changedOrigin = NO;

  // reposition the window on the screen.
  if (_autoPositionMask == GSWindowAutoPositionNone)
    return;

  /*
   * determine if and how the X axis can be resized
   */
  if (_autoPositionMask & GSWindowMinXMargin)
    options++;

  if (_autoPositionMask & GSWindowMaxXMargin)
    options++;

  /*
   * adjust the X axis if any X options are set in the mask
   */
  if (options > 0)
    {
      float change = newSize.width - oldSize.width;
      float changePerOption = change / options;

      if (_autoPositionMask & GSWindowMinXMargin)
	{
	  origin.x += changePerOption;
	  changedOrigin = YES;
	}
    }

  /*
   * determine if and how the Y axis can be resized
   */
  options = 0;
  if (_autoPositionMask & GSWindowMinYMargin)
    options++;

  if (_autoPositionMask & GSWindowMaxYMargin)
    options++;

  /*
   * adjust the Y axis if any Y options are set in the mask
   */
  if (options > 0)
    {
      float change = newSize.height - oldSize.height;
      float changePerOption = change / options;
      
      if (_autoPositionMask & (GSWindowMaxYMargin | GSWindowMinYMargin))
	{
	  if (_autoPositionMask & GSWindowMinYMargin)
	    {
	      origin.y += changePerOption;
	      changedOrigin = YES;
	    }
	}
    }
  
  // change the origin of the window.
  if (changedOrigin)
    {
      [window setFrameOrigin: origin];
    }
}

// NSCoding...
- (id) initWithCoder: (NSCoder *)coder
{
  id obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      int version = [coder versionForClassName: @"GSWindowTemplate"];

      if (version == GSWINDOWT_VERSION)
	{
	  // decode the defer flag...
	  [coder decodeValueOfObjCType: @encode(BOOL) at: &_deferFlag];      
	  [coder decodeValueOfObjCType: @encode(unsigned int) at: &_autoPositionMask];
	  _screenRect = [coder decodeRect];
	}
      else if (version == 0)
	{
	  // decode the defer flag...
	  [coder decodeValueOfObjCType: @encode(BOOL) at: &_deferFlag];      
	  _autoPositionMask = GSWindowAutoPositionNone;
	  _screenRect = [[obj screen] frame];
	}

      // FIXME: The designated initializer logic for NSWindow is in the initWithCoder: method of
      // NSWindow.   Unfortunately, this means that the "defer" flag for NSWindows and NSWindow
      // subclasses in gorm files will be ignored.   This shouldn't have a great impact, 
      // but it is not the correct behavior.  

      //
      // Set all of the attributes into the object, if it 
      // responds to any of these methods.
      //
      if ([obj respondsToSelector: @selector(setAutoPositionMask:)])
	{
	  [obj setAutoPositionMask: [self autoPositionMask]];
	}

      RELEASE(self);
    }
  return obj;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  int version = [[self class] version];

  [super encodeWithCoder: coder];

  if (version == GSWINDOWT_VERSION)
    {
      _screenRect = [[_object screen] frame];  
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_deferFlag];
      [coder encodeValueOfObjCType: @encode(unsigned int) at: &_autoPositionMask];
      [coder encodeRect: _screenRect]; 
    }
  else if (version == 0)
    {
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_deferFlag];
    }
}
@end

@implementation GSViewTemplate
+ (void) initialize
{
  if (self == [GSViewTemplate class]) 
    {
      [self setVersion: GSVIEWT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end

// Template for any classes which derive from NSText
@implementation GSTextTemplate
+ (void) initialize
{
  if (self == [GSTextTemplate class]) 
    {
      [self setVersion: GSTEXTT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id     obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end

// Template for any classes which derive from GSTextView
@implementation GSTextViewTemplate
+ (void) initialize
{
  if (self == [GSTextViewTemplate class]) 
    {
      [self setVersion: GSTEXTVIEWT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id     obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end

// Template for any classes which derive from NSMenu.
@implementation GSMenuTemplate
+ (void) initialize
{
  if (self == [GSMenuTemplate class]) 
    {
      [self setVersion: GSMENUT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id     obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end


// Template for any classes which derive from NSControl
@implementation GSControlTemplate
+ (void) initialize
{
  if (self == [GSControlTemplate class]) 
    {
      [self setVersion: GSCONTROLT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id     obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end

@implementation GSObjectTemplate
+ (void) initialize
{
  if (self == [GSObjectTemplate class]) 
    {
      [self setVersion: GSOBJECTT_VERSION];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  id     obj = [super initWithCoder: coder];
  if (obj != nil)
    {
      RELEASE(self);
    }
  return obj;
}
@end

// Order in this factory method is very important.  
// Which template to create must be determined
// in sequence because of the class hierarchy.
@implementation GSTemplateFactory
+ (id) templateForObject: (id) object 
	   withClassName: (NSString *)className
      withSuperClassName: (NSString *)superClassName
{
  id template = nil;
  if (object != nil)
    {
      if ([object isKindOfClass: [NSWindow class]])
	{
	  template = [[GSWindowTemplate alloc] initWithObject: object
					       className: className 
					       superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSTextView class]])
	{
	  template = [[GSTextViewTemplate alloc] initWithObject: object
						 className: className 
						 superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSText class]])
	{
	  template = [[GSTextTemplate alloc] initWithObject: object
					     className: className 
					     superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSControl class]])
	{
	  template = [[GSControlTemplate alloc] initWithObject: object
						className: className 
						superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSView class]])
	{
	  template = [[GSViewTemplate alloc] initWithObject: object
					     className: className 
					     superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSMenu class]])
	{
	  template = [[GSMenuTemplate alloc] initWithObject: object
					     className: className 
					     superClassName: superClassName];
	}
      else if ([object isKindOfClass: [NSObject class]]) 
	{
	  // for gui elements derived from NSObject
	  template = [[GSObjectTemplate alloc] initWithObject: object
					       className: className 
					       superClassName: superClassName];
	}
    }
  return AUTORELEASE(template);
}
@end
