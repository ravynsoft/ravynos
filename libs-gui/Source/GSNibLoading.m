/** <title>GSNibLoading</title>

   <abstract>
   These are templates for use with OSX Nib files.  These classes are the
   templates and other things which are needed for reading/writing nib files.
   </abstract>

   Copyright (C) 1997, 1999 Free Software Foundation, Inc.

   Author: Gregory John Casamento
   Date: 2003, 2005
   Author: Fred Kiefer
   Date: 2003, 2010

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

#define EXPOSE_NSKeyedUnarchiver_IVARS

#import <Foundation/NSArchiver.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSByteOrder.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDecimalNumber.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>

#import "GNUstepGUI/GSNibLoading.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSSound.h"
#import "AppKit/NSToolbar.h"
#import "GNUstepGUI/GSInstantiator.h"
#import "GSGuiPrivate.h"

static BOOL _isInInterfaceBuilder = NO;

@interface NSKeyedUnarchiver (NSClassSwapperPrivate)
- (Class) replacementClassForClassName: (NSString *)className;
@end

@interface NSApplication (NibCompatibility)
- (void) _setMainMenu: (NSMenu*)aMenu;
@end

@interface NSView (NibCompatibility)
- (void) _fixSubviews;
@end

/* Correct some instances where the ":" is missing from the method name in the label */
@interface NSNibControlConnector (NibCompatibility)
- (void) instantiateWithInstantiator: (id<GSInstantiator>)instantiator;
@end

@interface NSNibConnector (NibCompatibility)
- (void) instantiateWithInstantiator: (id<GSInstantiator>)instantiator;
@end

@interface NSDecimalNumberPlaceholder : NSObject
@end

@interface _NSCornerView : NSView
@end

@interface NSMenu (NibCompatibility)
- (void) _setMain: (BOOL)isMain;
@end
@interface NSMenu (GNUstepPrivate)
- (void) _setGeometry;
- (BOOL) _isMainMenu;
@end

@implementation NSMenu (NibCompatibility)
// FIXME: Why can't this be merged with setMain: ?
- (void) _setMain: (BOOL)isMain
{
  if (isMain)
    {
      NSMenuView	*oldRep;
      NSInterfaceStyle	oldStyle;
      NSInterfaceStyle	newStyle;
      NSString          *processName;

      if ([self numberOfItems] == 0)
        return;

      oldRep = [self menuRepresentation];
      oldStyle = [oldRep interfaceStyle];
      newStyle = NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil);
      processName = [[NSProcessInfo processInfo] processName];

      /*
       * If necessary, rebuild menu for (different) style
       */
      if (oldStyle != newStyle)
        {
	  NSMenuView	*newRep;

	  newRep = [[NSMenuView alloc] initWithFrame: NSZeroRect];
	  if (newStyle == NSMacintoshInterfaceStyle
	    || newStyle == NSWindows95InterfaceStyle)
	    {
	      [newRep setHorizontal: YES];
	    }
	  else
	    {
	      [newRep setHorizontal: NO];
	    }
	  [newRep setInterfaceStyle: newStyle];
	  [self setMenuRepresentation: newRep];
	  RELEASE(newRep);
	}

      [[self window] setTitle: processName];
      [[self window] setLevel: NSMainMenuWindowLevel];

      // if it's a standard menu, transform it to be more NeXT'ish/GNUstep-like
      if (_menu.horizontal == NO)
        {
          NSMenuItem *appItem;
	  NSMenu *sub;
	  SEL	sel = @selector(terminate:);
          
	  /* The title of the main menu should be the process name.
	   */
          [self setTitle: processName];

	  /* If there is no 'quite' item (one which sends a -terminate:
	   * actions) we add one.
	   */
	  if ([self indexOfItemWithTarget: nil andAction: sel] < 0
	    && [self indexOfItemWithTarget: NSApp andAction: sel] < 0)
	    {
	      NSString *quitString;
	      NSMenuItem *quitItem;

	      quitString = [NSString stringWithFormat: @"%@ %@", 
		NSLocalizedString (@"Quit", @"Quit"), processName];
	      quitItem = [[NSMenuItem alloc] initWithTitle: quitString
		action: @selector(terminate:)
		keyEquivalent: @"q"];
              [self addItem: quitItem];
	    }

	  /* An OSX main menu has the first item pointing to a submenu
	   * whose contents are much the same as a GNUstep info menu.
	   */
          appItem = (NSMenuItem*)[self itemAtIndex: 0]; // Info item.
	  sub = [appItem submenu];
	  if (sub != nil)
	    {
              NSString	*infoString;
	      NSInteger	index;

	      infoString = NSLocalizedString (@"Info", @"Info");
	      [appItem setTitle: infoString];
	      [sub setTitle: infoString];
	      /* The submenu may contain a 'quit' item ... if so we need to
	       * remove it as we already added one to the main menu.
	       */
	      index = [sub indexOfItemWithTarget: nil andAction: sel];
	      if (index < 0)
		{
	          index = [sub indexOfItemWithTarget: NSApp andAction: sel];
		}
	      if (index >= 0)
		{
		  [sub removeItemAtIndex: index];
		}
	    }
        }

      [self _setGeometry];
      [self sizeToFit];

      if ([NSApp isActive])
        {
	  [self display];
	}
    }
  else 
    {
      [self close];
      [[self window] setLevel: NSSubmenuWindowLevel];
    }
}
@end

@implementation NSApplication (NibCompatibility)
- (void) _setMainMenu: (NSMenu*)aMenu
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
      [_main_menu _setMain: YES];
    }
}
@end

@implementation NSView (NibCompatibility)
- (void) _setWindow: (id) w
{
  _window = w;
}

- (void) _fixSubviews
{
  NSEnumerator *en = [[self subviews] objectEnumerator];
  id v = nil;
  while ((v = [en nextObject]) != nil)
    {
      if ([v window] != [self window] ||
         [v superview] != self)
        {
          [v _setWindow: [self window]];
          RETAIN(v);
          [_sub_views removeObject: v];
          [self addSubview: v];
          RELEASE(v);
        }
      [v _fixSubviews];
    }
}
@end

/**
 * NSWindowTemplate
 *
 * Instances of this class take the place of all windows in the nib file.
 */
@implementation NSWindowTemplate
+ (void) initialize
{
  if (self == [NSWindowTemplate class]) 
    { 
      [self setVersion: 0];
    }
}

- (void) dealloc
{
  RELEASE(_title);
  RELEASE(_viewClass);
  RELEASE(_windowClass);
  RELEASE(_view);
  RELEASE(_autosaveName);
  RELEASE(_realObject);
  [super dealloc];
}

/**
 * Designated initializer for NSWindowTemplate.
 */
- (id) initWithWindow: (NSWindow *)window
            className: (NSString *)windowClass
           isDeferred: (BOOL) deferred
            isOneShot: (BOOL) oneShot
            isVisible: (BOOL) visible
       wantsToBeColor: (BOOL) wantsToBeColor
     autoPositionMask: (int) autoPositionMask
{
  if ((self = [super init]) != nil)
    {
      if (window != nil)
        {
          // object members
          ASSIGN(_title, [window title]);
          ASSIGN(_viewClass, NSStringFromClass([[window contentView] class]));
          ASSIGN(_windowClass, windowClass);
          ASSIGN(_view, [window contentView]);
          ASSIGN(_autosaveName, [window frameAutosaveName]);
          
          // style & size
          _windowStyle = [window styleMask];
          _backingStoreType = [window backingType];
          //_maxSize = [window maxSize];
          //_minSize = [window minSize];
          _maxSize = [window contentMaxSize];
          _minSize = [window contentMinSize];
          _windowRect = [window frame];
          _screenRect = [[NSScreen mainScreen] frame];
          
          // flags
          _flags.isHiddenOnDeactivate = [window hidesOnDeactivate];
          _flags.isNotReleasedOnClose = (![window isReleasedWhenClosed]);
          _flags.isDeferred = deferred;
          _flags.isOneShot = oneShot;
          _flags.isVisible = visible;
          _flags.isNotShadowed = ![window hasShadow];
          _flags.wantsToBeColor = wantsToBeColor;
          _flags.dynamicDepthLimit = [window hasDynamicDepthLimit];
          _flags.autoPositionMask = autoPositionMask;
          _flags.savePosition = YES; // not yet implemented.
          _flags.autorecalculatesKeyViewLoop = [window autorecalculatesKeyViewLoop];
        }
    }
  return self;
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSViewClass"])
        {
          ASSIGN(_viewClass, [coder decodeObjectForKey: @"NSViewClass"]);
        }
      else
        {
          ASSIGN(_viewClass, @"NSView");
        }
      if ([coder containsValueForKey: @"NSWindowClass"])
        {
          ASSIGN(_windowClass, [coder decodeObjectForKey: @"NSWindowClass"]);
        }
      else
        {
          ASSIGN(_windowClass, @"NSWindow");
        }
      if ([coder containsValueForKey: @"NSWindowStyleMask"])
        {
          _windowStyle = [coder decodeIntForKey: @"NSWindowStyleMask"];
        }
      else
        {
          _windowStyle = 0;
        }
      if ([coder containsValueForKey: @"NSWindowBacking"])
        {
          _backingStoreType = [coder decodeIntForKey: @"NSWindowBacking"];
        }
      if ([coder containsValueForKey: @"NSWindowView"])
        {
          ASSIGN(_view, [coder decodeObjectForKey: @"NSWindowView"]);
        }
      if ([coder containsValueForKey: @"NSWTFlags"])
        {
          unsigned long flags = [coder decodeIntForKey: @"NSWTFlags"];
          memcpy((void *)&_flags,(void *)&flags,sizeof(struct _GSWindowTemplateFlags));
        }

      if ([coder containsValueForKey: @"NSWindowContentMinSize"])
        {
          _minSize = [coder decodeSizeForKey: @"NSWindowContentMinSize"];
        }
      else if ([coder containsValueForKey: @"NSMinSize"])
        {
          NSRect rect = NSZeroRect;
          rect.size = [coder decodeSizeForKey: @"NSMinSize"];
          rect = [NSWindow contentRectForFrameRect: rect
                                         styleMask: _windowStyle];
          _minSize = rect.size;
        }

      if ([coder containsValueForKey: @"NSWindowContentMaxSize"])
        {
          _maxSize = [coder decodeSizeForKey: @"NSWindowContentMaxSize"];
        }
      else if ([coder containsValueForKey: @"NSMaxSize"])
        {
          NSRect rect = NSZeroRect;
          rect.size = [coder decodeSizeForKey: @"NSMaxSize"];
          rect = [NSWindow contentRectForFrameRect: rect
                                         styleMask: _windowStyle];
          _maxSize = rect.size;
        }
      else
        {
          _maxSize = NSMakeSize (10e4, 10e4);
        }

      if ([coder containsValueForKey: @"NSWindowRect"])
        {
          _windowRect = [coder decodeRectForKey: @"NSWindowRect"];
        }
      if ([coder containsValueForKey: @"NSFrameAutosaveName"])
        {
	  ASSIGN(_autosaveName, [coder decodeObjectForKey: @"NSFrameAutosaveName"]);
        }
      if ([coder containsValueForKey: @"NSWindowTitle"])
        {
          ASSIGN(_title, [coder decodeObjectForKey: @"NSWindowTitle"]);
          _windowStyle |= NSTitledWindowMask;
        }

      if ([coder containsValueForKey: @"NSToolbar"])
        {
          _toolbar = [coder decodeObjectForKey: @"NSToolbar"];
        }

      _baseWindowClass = [NSWindow class];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      unsigned long flags = 0; 
      NSRect rect = [NSWindow contentRectForFrameRect: _windowRect
                              styleMask: _windowStyle];
      memcpy((void *)&flags,(void *)&_flags,sizeof(unsigned long));

      [aCoder encodeObject: _viewClass forKey: @"NSViewClass"];
      [aCoder encodeObject: _windowClass forKey: @"NSWindowClass"];
      [aCoder encodeInt: _windowStyle forKey: @"NSWindowStyleMask"];
      [aCoder encodeInt: _backingStoreType forKey: @"NSWindowBacking"];
      [aCoder encodeObject: _view forKey: @"NSWindowView"];
      [aCoder encodeInt: flags forKey: @"NSWTFlags"];
      [aCoder encodeSize: _minSize forKey: @"NSWindowContentMinSize"];
      [aCoder encodeSize: _maxSize forKey: @"NSWindowContentMaxSize"];
      [aCoder encodeRect: rect forKey: @"NSWindowRect"];
      [aCoder encodeObject: _title forKey: @"NSWindowTitle"];
      [aCoder encodeObject: _autosaveName forKey: @"NSFrameAutosaveName"];
      [aCoder encodeObject: _toolbar forKey: @"NSToolbar"];
    }
}

/**
 * This method is used to get the real object when connections are established.
 */
- (id) nibInstantiate
{
  if (_realObject == nil)
    {
      Class aClass;

      if ([NSClassSwapper isInInterfaceBuilder])
        {
          aClass = [self baseWindowClass];
        }
      else
        {
          aClass = NSClassFromString(_windowClass);      
        }

      if (aClass == nil)
        {
          [NSException raise: NSInternalInconsistencyException
                       format: @"Unable to find class '%@'", _windowClass];
        }
      
      _realObject = [[aClass allocWithZone: NSDefaultMallocZone()]
                      initWithContentRect: _windowRect
                      styleMask: _windowStyle
                      backing: _backingStoreType
                      defer: _flags.isDeferred];
      
      // set flags...
      [_realObject setHidesOnDeactivate: _flags.isHiddenOnDeactivate];
      [_realObject setReleasedWhenClosed: !(_flags.isNotReleasedOnClose)];
      [_realObject setOneShot: _flags.isOneShot];
      // [_realObject setVisible: _flags.isVisible]; // this is determined by whether it's in the visible windows array...
      // [_realObject setWantsToBeColor: _flags.wantsToBeColor]; // not applicable on GNUstep.
      [_realObject setAutodisplay: YES];
      [_realObject setDynamicDepthLimit: _flags.dynamicDepthLimit];
      // [_realObject setAutoPositionMask: _flags.autoPositionMask]; // currently not implemented for nibs
      // [_realObject setAutoPosition: _flags.autoPosition];
      [_realObject setDynamicDepthLimit: _flags.dynamicDepthLimit];
      // [_realObject setFrameAutosaveName: _autosaveName]; // done after setting the min/max sizes
      [_realObject setHasShadow: !_flags.isNotShadowed];
      [_realObject setAutorecalculatesKeyViewLoop: _flags.autorecalculatesKeyViewLoop];

      // reset attributes...
      [_realObject setContentView: _view];
      //[_realObject setMinSize: _minSize];
      //[_realObject setMaxSize: _maxSize];
      [_realObject setTitle: _title];

      if ([_viewClass isKindOfClass: [NSToolbar class]])
	{
          // FIXME: No idea what is going on here
	  [_realObject setToolbar: (NSToolbar*)_viewClass];
	}
      if (_toolbar)
        {
          [_realObject setToolbar: _toolbar];
        }

      [_realObject setContentMinSize: _minSize];
      [_realObject setContentMaxSize: _maxSize];
	  
      [_view _fixSubviews];

      // FIXME What is the point of calling -setFrame:display: here? It looks
      // like an effective no op to me.
      // resize the window...
      [_realObject setFrame: [NSWindow frameRectForContentRect: [self windowRect] 
                                       styleMask: [self windowStyle]]
                   display: NO];
      [_realObject setFrameAutosaveName: _autosaveName];
    } 
  return _realObject;
}

// setters and getters
/**
 * sets the type of backing store the window uses.
 */
- (void) setBackingStoreType: (NSBackingStoreType)type
{
  _backingStoreType = type;
}

/**
 * Returns the type of backing store which is used.
 */
- (NSBackingStoreType) backingStoreType
{
  return _backingStoreType;
}

/**
 * Sets whether or not the window is deferred.
 */
- (void) setDeferred: (BOOL)flag
{
  _flags.isDeferred = flag;
}

/**
 * Returns YES, if the window is deferred, NO otherwise.
 */
- (BOOL) isDeferred
{
  return _flags.isDeferred;
}

/**
 * Sets the maximum size of the window.
 */
- (void) setMaxSize: (NSSize)maxSize
{
  _maxSize = maxSize;
}

/**
 * Returns the maximum size of the window.
 */
- (NSSize) maxSize
{
  return _maxSize;
}

/**
 * Sets the minimum size of the window.
 */
- (void) setMinSize: (NSSize)minSize
{
  _minSize = minSize;
}

/**
 * Returns the maximum size of the window.
 */
- (NSSize) minSize
{
  return _minSize;
}

/**
 * Sets the window style.
 */
- (void) setWindowStyle: (unsigned)style
{
  _windowStyle = style;
}

/** 
 * Returns the window style.
 */
- (unsigned) windowStyle
{
  return _windowStyle;
}

/**
 * Sets the window title.
 */
- (void) setTitle: (NSString *) title
{
  ASSIGN(_title, title);
}

/**
 * Returns the window style.
 */
- (NSString *)title;
{
  return _title;
}

/**
 * Sets the class used for the content view.
 */
- (void) setViewClass: (NSString *)viewClass
{
  ASSIGN(_viewClass,viewClass);
}

/**
 * Returns the name of the class used for the content view.
 */
- (NSString *)viewClass
{
  return _viewClass;
}

/**
 * Sets the window rect.
 */
- (void) setWindowRect: (NSRect)rect
{
  _windowRect = rect;
}

/**
 * Returns the window rect.
 */
- (NSRect)windowRect
{
  return _windowRect;
}

/**
 * Sets the screen rect.
 */
- (void) setScreenRect: (NSRect)rect
{
  _screenRect = rect;
}

/**
 * Returns the screen rect.
 */
- (NSRect) screenRect
{
  return _screenRect;
}

/**
 * Sets the instantiated object/real object.
 */
- (void) setRealObject: (id)o
{
  ASSIGN(_realObject,o);
}

/**
 * Returns the real object represented by this template.
 */
- (id) realObject
{
  return _realObject;
}

/**
 * Sets the view instance.
 */ 
- (void) setView: (id)view
{
  ASSIGN(_view,view);
}

/** 
 * Gets the view instance.
 */
- (id) view
{
  return _view;
}

/**
 * sets the class name to be used when unarchiving the window.
 */
- (void) setClassName: (NSString *)name
{
  ASSIGN(_windowClass, name);
}

/**
 * Returns the class instance.
 */
- (NSString *)className
{
  return _windowClass;
}

/**
 * Returns the base window class.   This is usually NSWindow, but this method
 * is overriden in the editor so that a different class may be used to take the
 * place of the window.   In the case of Gorm, this is GormNSWindow.
 */
- (Class) baseWindowClass
{
  return _baseWindowClass;
}
@end

/*
 * NSViewTemplate
 *
 * Template for any classes which derive from NSView
 */
@implementation NSViewTemplate
+ (void) initialize
{
  if (self == [NSViewTemplate class]) 
    {
      [self setVersion: 0];
    }
}

- (void) dealloc
{
  RELEASE(_className);
  RELEASE(_realObject);
  [super dealloc];
}

/**
 * Designated initializer for NSViewTemplate.
 */
- (id) initWithObject: (id)o
	    className: (NSString *)name
{
  if ((self = [super init]) != nil)
    {
      [self setRealObject: o];
      [self setClassName: name];
    }
  return self;
}

- (id) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self != nil)
    {
      if ([coder allowsKeyedCoding])
        {
          _className = [coder decodeObjectForKey: @"NSClassName"];
        }

      if (_realObject == nil)
	{
	  Class aClass = NSClassFromString(_className);
	  if (aClass == nil)
	    {
	      [NSException raise: NSInternalInconsistencyException
			   format: @"Unable to find class '%@'", _className];
	    }
	  else
	    {
	      ASSIGN(_realObject, [[aClass allocWithZone: NSDefaultMallocZone()] initWithCoder: coder]);
	      [[self superview] replaceSubview: self with: _realObject]; // replace the old view...
	    }
	}

      AUTORELEASE(self);
      return _realObject;
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return nil;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: (id)_className forKey: @"NSClassName"];
      [_realObject encodeWithCoder: coder];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't encode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
}

// setters and getters
/**
 * Set the class name to be used by the NSView subclass.
 */
- (void) setClassName: (NSString *)name
{
  ASSIGN(_className, name);
}

/**
 * Returns the classname.
 */
- (NSString *)className
{
  return _className;
}

/**
 * Set the real object of the template.
 */
- (void) setRealObject: (id)o
{
  ASSIGN(_realObject, o);
}

/**
 * Get the real object represented by the template.
 */
- (id) realObject
{
  return _realObject;
}
@end

// Template for any classes which derive from NSText
@implementation NSTextTemplate
+ (void) initialize
{
  if (self == [NSTextTemplate class]) 
    {
      [self setVersion: 0];
    }
}
@end

/**
 * NSTextViewTemplate 
 *
 * Template for any classes which derive from NSTextView
 */
@implementation NSTextViewTemplate
+ (void) initialize
{
  if (self == [NSTextViewTemplate class]) 
    {
      [self setVersion: 0];
    }
}
@end

// Template for any classes which derive from NSMenu.
@implementation NSMenuTemplate
+ (void) initialize
{
  if (self == [NSMenuTemplate class]) 
    {
      [self setVersion: 0];
    }
}

- (void) dealloc
{
  RELEASE(_menuClass);
  RELEASE(_realObject);
  [super dealloc];
}

- (id) initWithCoder: (NSCoder *)aCoder
{
  RELEASE(self);
  return nil;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
}

- (void) setClassName: (NSString *)className
{
  ASSIGN(_menuClass, className);
}

- (NSString *)className
{
  return _menuClass;
}

- (void) setRealObject: (id)o
{
  ASSIGN(_realObject,o);
}

- (id) realObject
{
  return _realObject;
}
@end

@implementation NSCustomObject
- (void) setClassName: (NSString *)name
{
  ASSIGNCOPY(_className, name);
}

- (NSString *)className
{
  return _className;
}

- (void) setExtension: (NSString *)name
{
  ASSIGNCOPY(_extension, name);
}

- (NSString *)extension
{
  return _extension;
}

- (void) setRealObject: (id)obj
{
  ASSIGN(_object, obj);
}

- (id) realObject
{
  return _object;
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      ASSIGN(_className, [coder decodeObjectForKey: @"NSClassName"]);
      ASSIGN(_extension, [coder decodeObjectForKey: @"NSExtension"]);
      ASSIGN(_object, [coder decodeObjectForKey: @"NSObject"]);
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: (id)_className forKey: @"NSClassName"];
      [coder encodeConditionalObject: (id)_extension forKey: @"NSExtension"];
      [coder encodeConditionalObject: (id)_object forKey: @"NSObject"];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Keyed coding not implemented for %@.", 
                   NSStringFromClass([self class])];
    }
  
}

- (id) nibInstantiate
{
  if (_object == nil)
    {
      Class aClass;
      
      if ([NSClassSwapper isInInterfaceBuilder])
        {
          aClass = [self class];
        }
      else
        {
          aClass = NSClassFromString(_className);
        }

      if (aClass == nil)
        {
          [NSException raise: NSInternalInconsistencyException
                       format: @"Unable to find class '%@'", _className];
        }

      if (GSObjCIsKindOf(aClass, [NSApplication class]) || 
	 [_className isEqual: @"NSApplication"])
        {
	  _object = RETAIN([aClass sharedApplication]);
        }
      else if ((GSObjCIsKindOf(aClass, [NSFontManager class])) ||
               ([_className isEqual: @"NSFontManager"]))
        {
          _object = RETAIN([aClass sharedFontManager]);
        }
      else
	{
	  _object = [[aClass allocWithZone: NSDefaultMallocZone()] init];
	}      
    }
  return _object;
}

- (void) awakeFromNib
{
  NSDebugLog(@"Called awakeFromNib on an NSCustomObject instance: %@", self);
  if ([_object respondsToSelector: @selector(awakeFromNib)])
    {
      [_object awakeFromNib];
    }
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"<%s: %lx> = <<className: %@, object: %@>>",
		   GSClassNameFromObject(self), 
		   (unsigned long)self,
		   _className,_object];
}

- (void) dealloc
{
  RELEASE(_className);
  RELEASE(_extension);
  RELEASE(_object);
  [super dealloc];
}
@end

@implementation NSCustomView
- (void) setClassName: (NSString *)name
{
  ASSIGNCOPY(_className, name);
}

- (NSString *)className
{
  return _className;
}
- (void) setExtension: (NSString *)ext;
{
  ASSIGNCOPY(_extension, ext);
}

- (NSString *)extension
{
  return _extension;
}

- (id) nibInstantiate
{
  if ([NSClassSwapper isInInterfaceBuilder])
    {
      _view = self;
      return self;
    }

  if (_view == nil)
    {
      Class aClass;
  
      // If the class name is nil, assume NSView.
      if (_className == nil)
        {
          aClass = [NSView class];
        }
      else
        {
          aClass = NSClassFromString(_className);
        }
  
      if (aClass == nil)
        {
          [NSException raise: NSInternalInconsistencyException
                      format: @"Unable to find class '%@'", _className];
        }
      else
        {
          _view = [[aClass allocWithZone: NSDefaultMallocZone()] initWithFrame: [self frame]];
        }
    }

  return _view;
}

- (id) nibInstantiateWithCoder: (NSCoder *)coder
{
  if ([NSClassSwapper isInInterfaceBuilder])
    {
      return _view;
    }
  else if ([coder allowsKeyedCoding])
    {
      NSArray *subs = nil;
      id nextKeyView = nil;
      id prevKeyView = nil;
      NSEnumerator *en = nil;
      id v = nil;

      // Tell the decoder that the object gets replaced before decoding subviews
      [(NSKeyedUnarchiver *)coder replaceObject: self withObject: _view];

      prevKeyView = [coder decodeObjectForKey: @"NSPreviousKeyView"];
      nextKeyView = [coder decodeObjectForKey: @"NSNextKeyView"];
      if (nextKeyView != nil)
        {
          [_view setNextKeyView: nextKeyView];
        }
      if (prevKeyView != nil)
        {
          [_view setPreviousKeyView: prevKeyView];
        }      
      if ([coder containsValueForKey: @"NSvFlags"])
	{
	  int vFlags = [coder decodeIntForKey: @"NSvFlags"];
	  [_view setAutoresizingMask: vFlags & 0x3F];
	  [_view setAutoresizesSubviews: ((vFlags & 0x100) == 0x100)];
	  [_view setHidden: ((vFlags & 0x80000000) == 0x80000000)];
	}
      /*
      if ([coder containsValueForKey: @"NSNextResponder"])
	{
	  [_view setNextResponder: [coder decodeObjectForKey: @"NSNextResponder"]];
	}      
      */

      // reset the bounds...
      // [_view setBounds: [_view frame]];

      subs = [coder decodeObjectForKey: @"NSSubviews"];
      en = [subs objectEnumerator];
      while((v = [en nextObject]) != nil)
	{
	  [_view addSubview: v];
	}
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		   format: @"Called NSCustomView awakeAfterUsingCoder with non-keyed archiver."];
    }

  return _view;
}

- (id) initWithCoder: (NSCoder *)coder
{
  // if in interface builder, then initialize as normal.
  if ([NSClassSwapper isInInterfaceBuilder])
    {
      self = [super initWithCoder: coder];
      if (self == nil)
        {
          return nil;
        }
    }

  if ([coder allowsKeyedCoding])
    {
      // get the super stuff without calling super...
      if ([coder containsValueForKey: @"NSFrame"])
        {
          _frame = [coder decodeRectForKey: @"NSFrame"];
        }
      else
        {
          _frame = NSZeroRect;
          if ([coder containsValueForKey: @"NSFrameSize"])
            {
              _frame.size = [coder decodeSizeForKey: @"NSFrameSize"];
            }
        }
      
      ASSIGN(_className, [coder decodeObjectForKey: @"NSClassName"]);
      ASSIGN(_extension, [coder decodeObjectForKey: @"NSExtension"]);
      
      if ([self nibInstantiate] != nil)
        {
          [self nibInstantiateWithCoder: coder];
        }
      
      if (self != _view)
        {
          AUTORELEASE(self);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                  format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }

  return (id)_view;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _className forKey: @"NSClassName"];
      [coder encodeObject: _extension forKey: @"NSExtension"];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't encode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
}
@end

/**
 * This class represents an image or a sound which is referenced by the nib file.
 */
@implementation NSCustomResource
- (void) setClassName: (NSString *)className
{
  ASSIGNCOPY(_className, className);
}

- (NSString *)className
{
  return _className;
}

- (void) setResourceName: (NSString *)resourceName
{
  ASSIGNCOPY(_resourceName, resourceName);
}

- (NSString *)resourceName
{
  return _resourceName;
}

- (id) initWithCoder: (NSCoder *)coder
{
  id realObject = nil;
  if ([coder allowsKeyedCoding])
    {
      ASSIGN(_className, [coder decodeObjectForKey: @"NSClassName"]);
      ASSIGN(_resourceName, [coder decodeObjectForKey: @"NSResourceName"]);

      // FIXME: this is a hack, but for now it should do.
      if ([_className isEqual: @"NSSound"])
        {
          realObject = RETAIN([NSSound soundNamed: _resourceName]);
        }
      else if ([_className isEqual: @"NSImage"])
        {
          realObject = RETAIN([NSImage imageNamed: _resourceName]);
        }

      if (realObject == nil)
        {
          NSLog(@"Could not load NSCustomResource %@ for class %@", _resourceName, _className);
          // Use a default instead of the missing object
          if ([_className isEqual: @"NSSound"])
            {
              realObject = RETAIN([NSSound soundNamed: @"Ping"]);
            }
          else if ([_className isEqual: @"NSImage"])
            {
              realObject  = RETAIN([NSImage imageNamed: @"GNUstep"]);
            }
        }
      // The object has been substituted, release the placeholder.
      RELEASE(self);
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }

  return realObject;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: (id)_className forKey: @"NSClassName"];
      [coder encodeObject: (id)_resourceName forKey: @"NSResourceName"];
    }
}
@end

/**
 * Category to add methods to NSKeyedUnarchiver which are needed during
 * nib reading.
 */
@implementation NSKeyedUnarchiver (NSClassSwapperPrivate)
/**
 * This method returns the class which replaces the class named
 * by className.   It uses the classes map to do this.
 */
- (Class) replacementClassForClassName: (NSString *)className
{
  Class aClass;
  if ((aClass = [self classForClassName: className]) == nil)
    {
      if ((aClass = [[self class] classForClassName: className]) == nil)
        {
          aClass = NSClassFromString(className);
          if (aClass == nil)
            {
              [NSException raise: NSInternalInconsistencyException
                          format: @"NSClassSwapper unable to find class '%@'", className];
            }
        }
    }
  return aClass;
}
@end

/**
 * NSClassSwapper
 *
 * This class is used to stand-in for objects which need to be replaced by another object.  
 * When this class is loaded in the live application, it unarchives and immediately replaces
 * itself with the instance of the object requested.   This is necessary since IB/Gorm does 
 * have objects this is used for in palettes, so there is no "live" or actual instance saved
 * in the gorm file... only this object as a stand in.
 */
@implementation NSClassSwapper
- (id) initWithObject: (id)object 
        withClassName: (NSString *)className
    originalClassName: (NSString *)origClassName
{
  if ((self = [super init]) != nil)
    {
      [self setTemplate: object];
      [self setClassName: className];
      [self setOriginalClassName: origClassName];
    }
  return self;
}

/**
 * This class method keeps track of whether or not we are operating within IB/Gorm.   
 * When unarchiving in IB/Gorm some behavior may need to be surpressed for some objects
 * or it 
 */
+ (void) setIsInInterfaceBuilder: (BOOL)flag
{
  _isInInterfaceBuilder = flag;
}

/**
 * returns YES, if we are currently in IB/Gorm.
 */
+ (BOOL) isInInterfaceBuilder
{
  return _isInInterfaceBuilder;
}

/**
 * Sets the template represented by temp.
 */
- (void) setTemplate: (id)temp
{
  ASSIGN(_template, temp);
}

/**
 * Returns the template.
 */
- (id) template
{
  return _template;
}

/**
 * Sets the class name.
 */
- (void) setClassName: (NSString *)className
{
  ASSIGNCOPY(_className, className);
}

/**
 * Returns the class name.
 */
- (NSString *)className
{
  return _className;
}

/**
 * Sets the original class name.
 */
- (void) setOriginalClassName: (NSString *)className
{
  ASSIGNCOPY(_originalClassName, className);
}

/**
 * Returns the original class name.
 */
- (NSString *)originalClassName
{
  return _originalClassName;
}

/** 
 * Instantiates the real object using className.
 */
- (void) instantiateRealObject: (NSCoder *)coder withClassName: (NSString *)className
{
  Class newClass = nil;
  id object = nil;
  NSKeyedUnarchiver *decoder = (NSKeyedUnarchiver *)coder;

  if ([NSClassSwapper isInInterfaceBuilder] == YES)
    {
      newClass = [decoder replacementClassForClassName: _originalClassName];
    }
  else
    {
      newClass = [decoder replacementClassForClassName: className];
    }

  // swap the class...
  object = [newClass allocWithZone: NSDefaultMallocZone()];
  [decoder setDelegate: self]; // set the delegate...
  [decoder replaceObject: self withObject: object];
  [self setTemplate: [object initWithCoder: decoder]];
  if (object != _template)
    {
      [decoder replaceObject: object withObject: _template];
    }
  [decoder setDelegate: nil]; // unset the delegate...
}

/**
 * This delegate method makes the proper substitution for cellClass
 * when the object needs to have it's own cell.   An example of this
 * is NSSecureTextField/NSSecureTextFieldCell.
 */
- (id) unarchiver: (NSKeyedUnarchiver *)coder
  didDecodeObject: (id)obj
{
  Class newClass = nil; 
  id result = obj;

  // if we are in an interface builder, then return the original object.
  if ([NSClassSwapper isInInterfaceBuilder] == YES)
    {
      newClass = [coder replacementClassForClassName: _originalClassName];
    }
  else
    {
      newClass = [coder replacementClassForClassName: _className];
    }

  // if this is a class which uses cells, override with the new cellClass, if the 
  // subclass responds to cellClass.
  if ([obj isKindOfClass: [NSCell class]] &&
      [newClass respondsToSelector: @selector(cellClass)] && 
      [_className isEqualToString: _originalClassName] == NO)
    {
      Class newCellClass = [newClass cellClass];
      if (newCellClass != [NSCell class])
        {
          RELEASE(obj);
          result = [[newCellClass alloc] initWithCoder: coder];      
        }
    }

  return result;
}

/**
 * Decode NSClassSwapper.
 */
- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      ASSIGN(_className, [coder decodeObjectForKey: @"NSClassName"]);  
      ASSIGN(_originalClassName, [coder decodeObjectForKey: @"NSOriginalClassName"]);  

      // build the real object...
      if ([NSClassSwapper isInInterfaceBuilder] == YES)
        {
          [self instantiateRealObject: coder withClassName: _originalClassName];
        }
      else
        {
          [self instantiateRealObject: coder withClassName: _className];
        }
      
      {
        id object;

        object = RETAIN(_template);
        RELEASE(self);
        return AUTORELEASE(object);
      }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }

  return self;
}

/**
 * Encode NSClassSwapper.
 */
- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _originalClassName forKey: @"NSOriginalClassName"];
      [coder encodeObject: _className forKey: @"NSClassName"];
      [_template encodeWithCoder: coder]; // encode the actual object;
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't encode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
}

/**
 * Deallocate NSClassSwapper instance.
 */
- (void) dealloc
{
  RELEASE(_className);
  RELEASE(_originalClassName);
  RELEASE(_template);
  [super dealloc];
}
@end

@implementation NSNibConnector (NibCompatibility)
/**
 * This method causes the connection to instantiate the objects in it's source
 * and destination.   The instantiator is the object which holds any custom
 * class information which might be needed to do the proprer substitution of
 * objects based on the contents of the maps.
 */
- (void) instantiateWithInstantiator: (id<GSInstantiator>)instantiator
{
  [self setSource: [instantiator instantiateObject: _src]];
  [self setDestination: [instantiator instantiateObject: _dst]];
}

- (id) nibInstantiate
{
  if ([_src respondsToSelector: @selector(nibInstantiate)])
    {
      [self setSource: [_src nibInstantiate]];
    }
  if ([_dst respondsToSelector: @selector(nibInstantiate)])
    {
      [self setDestination: [_dst nibInstantiate]];
    }
  return self;
}

@end

@implementation NSNibControlConnector (NibCompatibility)
/**
 * This method overrides the default implementation of instantiate with
 * instantiator.   It also corrects a common issue in some nib files
 * by adding a colon to the end if none was given.   It then calls the
 * superclass with the corrected label.
 */
- (void) instantiateWithInstantiator: (id<GSInstantiator>)instantiator
{
  NSRange colonRange = [_tag rangeOfString: @":"];
  NSUInteger location = colonRange.location;
  
  if (location == NSNotFound)
    {
      NSString *newTag = [NSString stringWithFormat: @"%@:",_tag];
      [self setLabel: (id)newTag];
    }

  [super instantiateWithInstantiator: instantiator];
}
@end

/**
 * NSIBObjectData
 *
 * This class is the container for all of the nib data.  It contains several maps.
 * The maps are the following:
 * 
 *     name -> object (name table)
 *     object -> name (name table reverse lookup)
 *     classes -> object (for custom class storage)
 *     oids -> object (for relating the oid to each object)
 *     accessibilityOids -> object 
 *
 * The maps are stored in the nib itself as a set of synchronized 
 * arrays one array containing the keys and the other the values.  This is why, in the
 * initWithCoder: and encodeWithCoder: methods they are saved as arrays and then 
 * loaded into NSMapTables.   
 */
@implementation NSIBObjectData
/**
 * Get the values from the map in the same order as the keys.
 */
- (NSArray *) _valuesForKeys: (NSArray *)keys inMap: (NSMapTable *)map
{
  NSMutableArray *result = [NSMutableArray array];
  NSEnumerator *en = [keys objectEnumerator];
  id key = nil;
  while ((key = [en nextObject]) != nil)
    {
      id value = (id)NSMapGet(map,key);
      [result addObject: value];
    }
  return result;
}

/**
 * Build a map with two arrays of keys and values.
 */
- (void) _buildMap: (NSMapTable *)mapTable 
          withKeys: (NSArray *)keys 
         andValues: (NSArray *)values
{
  NSEnumerator *ken = [keys objectEnumerator];
  NSEnumerator *ven = [values objectEnumerator];
  id key = nil;
  id value = nil;
  
  while ((key = [ken nextObject]) != nil && (value = [ven nextObject]) != nil)
    {
      NSMapInsert(mapTable, key, value);
      if (value == nil)
	{
	  NSLog(@"==> WARNING: Value for key %@ is %@",key , value);
	}
    }
}

/**
 * Encode the NSIBObjectData container
 */
- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      NSArray *accessibilityOidsKeys = (NSArray *)NSAllMapTableKeys(_accessibilityOids);
      NSArray *accessibilityOidsValues = [self _valuesForKeys: accessibilityOidsKeys inMap: _accessibilityOids];
      NSArray *classKeys = (NSArray *)NSAllMapTableKeys(_classes);
      NSArray *classValues = [self _valuesForKeys: classKeys inMap: _classes];
      NSArray *nameKeys = (NSArray *)NSAllMapTableKeys(_names);
      NSArray *nameValues = [self _valuesForKeys: nameKeys inMap: _names];
      NSArray *objectsKeys = (NSArray *)NSAllMapTableKeys(_objects);
      NSArray *objectsValues = [self _valuesForKeys: objectsKeys inMap: _objects];
      NSArray *oidsKeys = (NSArray *)NSAllMapTableKeys(_oids);
      NSArray *oidsValues = [self _valuesForKeys: oidsKeys inMap: _oids];

      [(NSKeyedArchiver *)coder setClassName: @"_NSCornerView" forClass: NSClassFromString(@"GSTableCornerView")];

      [coder encodeObject: (id)_accessibilityConnectors forKey: @"NSAccessibilityConnectors"];
      [coder encodeObject: (id) accessibilityOidsKeys forKey: @"NSAccessibilityOidsKeys"];
      [coder encodeObject: (id) accessibilityOidsValues forKey: @"NSAccessibilityOidsValues"];
      [coder encodeObject: (id) classKeys forKey: @"NSClassesKeys"];
      [coder encodeObject: (id) classValues forKey: @"NSClassesValues"];
      [coder encodeObject: (id) nameKeys forKey: @"NSNamesKeys"];
      [coder encodeObject: (id) nameValues forKey: @"NSNamesValues"];
      [coder encodeObject: (id) objectsKeys forKey: @"NSObjectsKeys"];
      [coder encodeObject: (id) objectsValues forKey: @"NSObjectsValues"];
      [coder encodeObject: (id) oidsKeys forKey: @"NSOidsKeys"];
      [coder encodeObject: (id) oidsValues forKey: @"NSOidsValues"];
      [coder encodeObject: (id) _connections forKey: @"NSConnections"];
      [coder encodeObject: (id) _fontManager forKey: @"NSFontManager"];
      [coder encodeObject: (id) _framework forKey: @"NSFramework"];
      [coder encodeObject: (id) _visibleWindows forKey: @"NSVisibleWindows"];
      [coder encodeInt: _nextOid forKey: @"NSNextOid"];
      [coder encodeConditionalObject: (id) _root forKey: @"NSRoot"];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't encode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
}

/**
 * Decode the NSIBObjectData container.
 */
- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      NSArray *nameKeys = nil;
      NSArray *nameValues = nil;
      NSArray *classKeys = nil;
      NSArray *classValues = nil;
      NSArray *objectsKeys = nil;
      NSArray *objectsValues = nil;
      NSArray *oidsKeys = nil;
      NSArray *oidsValues = nil;
      NSArray *accessibilityOidsKeys = nil;
      NSArray *accessibilityOidsValues = nil;

      [(NSKeyedUnarchiver *)coder setClass: NSClassFromString(@"GSTableCornerView")
			      forClassName: @"_NSCornerView"];

      //
      // Get root, font, framwork and oid. 
      // Retain objects since NSKeyedUnarchiver autoreleases unarchived objects.
      //
      ASSIGN(_root, [coder decodeObjectForKey: @"NSRoot"]);
      ASSIGN(_fontManager, [coder decodeObjectForKey: @"NSFontManager"]);
      ASSIGN(_framework, [coder decodeObjectForKey: @"NSFramework"]);
      _nextOid = [coder decodeIntForKey: @"NSNextOid"];

      // get connections.
      ASSIGN(_connections, (NSMutableArray *)
	[coder decodeObjectForKey: @"NSConnections"]);
      ASSIGN(_accessibilityConnectors, (NSMutableArray *)
	[coder decodeObjectForKey: @"NSAccessibilityConnectors"]);

      // get visible windows
      ASSIGN(_visibleWindows, (NSMutableArray *)
	[coder decodeObjectForKey: @"NSVisibleWindows"]);

      // instantiate the maps..
      _classes = NSCreateMapTable(NSObjectMapKeyCallBacks,
				  NSObjectMapValueCallBacks, 2);
      _names = NSCreateMapTable(NSObjectMapKeyCallBacks,
				NSObjectMapValueCallBacks, 2);
      _objects = NSCreateMapTable(NSObjectMapKeyCallBacks,
				  NSObjectMapValueCallBacks, 2);
      _oids = NSCreateMapTable(NSObjectMapKeyCallBacks,
			       NSObjectMapValueCallBacks, 2);

      // 
      // Get the maps.  There is no need to retain these, 
      // since they are going to be placed into the NSMapTable
      // structures anyway.
      //
      nameKeys = (NSArray *)
	[coder decodeObjectForKey: @"NSNamesKeys"];
      nameValues = (NSArray *)
	[coder decodeObjectForKey: @"NSNamesValues"];
      classKeys = (NSArray *)
	[coder decodeObjectForKey: @"NSClassesKeys"];
      classValues = (NSArray *)
	[coder decodeObjectForKey: @"NSClassesValues"];
      objectsKeys = (NSArray *)
	[coder decodeObjectForKey: @"NSObjectsKeys"];
      objectsValues = (NSArray *)
	[coder decodeObjectForKey: @"NSObjectsValues"];
      oidsKeys = (NSArray *)
	[coder decodeObjectForKey: @"NSOidsKeys"];
      oidsValues = (NSArray *)
	[coder decodeObjectForKey: @"NSOidsValues"];

      // Fill in the maps...
      [self _buildMap: _classes 
	    withKeys: classKeys 
	    andValues: classValues];
      [self _buildMap: _names 
	    withKeys: nameKeys 
	    andValues: nameValues];
      [self _buildMap: _objects 
	    withKeys: objectsKeys 
	    andValues: objectsValues];
      [self _buildMap: _oids 
	    withKeys: oidsKeys 
	    andValues: oidsValues];
      
      //
      // Only get these maps when in the editor.  They
      // aren't useful outside of it and only waste memory if
      // unarchived in the live application.
      //
      if ([NSClassSwapper isInInterfaceBuilder])
	{
	  // Only get these when in the editor...
	  accessibilityOidsKeys = (NSArray *)
	    [coder decodeObjectForKey: @"NSAccessibilityOidsKeys"];
	  accessibilityOidsValues = (NSArray *)
	    [coder decodeObjectForKey: @"NSAccessibilityOidsValues"];      

	  _accessibilityOids = NSCreateMapTable(NSObjectMapKeyCallBacks,
						NSObjectMapValueCallBacks, 2);	  
	  [self _buildMap: _accessibilityOids 
		withKeys: accessibilityOidsKeys 
		andValues: accessibilityOidsValues];
	}

      // instantiate...
      _topLevelObjects = [[NSMutableSet alloc] init];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
 
  return self;
}

/**
 * Initialize a new NSIBObjectData.
 */
- (id) init
{
  if ((self = [super init]) != nil)
    {
      // instantiate the maps..
      _objects = NSCreateMapTable(NSObjectMapKeyCallBacks,
                                  NSObjectMapValueCallBacks, 2);
      _names = NSCreateMapTable(NSObjectMapKeyCallBacks,
                                NSObjectMapValueCallBacks, 2);
      _oids = NSCreateMapTable(NSObjectMapKeyCallBacks,
                               NSObjectMapValueCallBacks, 2);
      _classes = NSCreateMapTable(NSObjectMapKeyCallBacks,
                                  NSObjectMapValueCallBacks, 2);
      _accessibilityOids = NSCreateMapTable(NSObjectMapKeyCallBacks,
                                            NSObjectMapValueCallBacks, 2);  

      // initialize the objects...
      _accessibilityConnectors = [[NSMutableArray alloc] init];
      _connections = [[NSMutableArray alloc] init];
      _visibleWindows = [[NSMutableArray alloc] init];
      _framework = nil;
      _fontManager = nil;
      _root = nil;
      _nextOid = 0;
    }
  return self;
}

/**
 * Deallocate NSIBObjectData.
 */
- (void) dealloc
{
  // free the maps.
  NSFreeMapTable(_objects);
  NSFreeMapTable(_names);
  NSFreeMapTable(_classes);
  NSFreeMapTable(_oids);
  // these are not allocated when not in interface builder.
  if ([NSClassSwapper isInInterfaceBuilder])
    {
      NSFreeMapTable(_accessibilityOids);
    }

  // free other objects.
  RELEASE(_accessibilityConnectors);
  RELEASE(_connections);
  RELEASE(_fontManager);
  RELEASE(_framework);
  RELEASE(_visibleWindows);
  RELEASE(_root);
  RELEASE(_topLevelObjects);
  [super dealloc];
}

/**
 * Call nibInstantiate on an object, if it responds to the nibInstantiate selector.
 */
- (id)instantiateObject: (id)obj
{
  id newObject = obj;
  if ([obj respondsToSelector: @selector(nibInstantiate)])
    {
      newObject = [obj nibInstantiate];
    }
  return newObject;
}

/**
 * Instantiate all of the objects in the nib file.
 */
- (void) nibInstantiateWithOwner: (id)owner topLevelObjects: (NSMutableArray *)topLevelObjects
{
  NSEnumerator *en;
  NSArray *objs;
  id obj = nil;
  id menu = nil;

  // set the new root object.
  [_root setRealObject: owner];

  // iterate over all objects, instantiate them and fill in top level array.
  /* Note: We instantiate all objects before establishing any connections
     between them, so that any shared instances defined in the nib are
     initialized before being used. This sequence is important when, e.g.,
     the nib defines a shared document controller that is an instance of a
     subclass of NSDocumentController. */
  objs = NSAllMapTableKeys(_objects);
  en = [objs objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      id v = NSMapGet(_objects, obj);
      NSInteger oid = [(id)NSMapGet(_oids, obj) intValue];

      obj = [self instantiateObject: obj];
      // Object is top level if it isn't the owner but points to it.
      /* Don't record proxy objects in the top level array. The only
	 reliable way to identify proxy objects seems to look at their
	 object ID. Apparently, Apple is using fixed negative IDs for
	 proxy objects (-1 = File's Owner, -2 = First Responder,
	 -3 = NSApplication). */
      if (oid >= 0)
	{
	  if ((v == owner || v == _root) && (obj != owner) && (obj != _root))
	    {
	      [topLevelObjects addObject: obj];
	      // All top level objects must be released by the caller to avoid
	      // leaking, unless they are going to be released by other nib
	      // objects on behalf of the owner.
	      RETAIN(obj);
	    }
          if ([obj isKindOfClass: [NSMenu class]] &&
              [obj _isMainMenu])
            {
              [NSApp _setMainMenu: obj];
            }
        }
    }

  // iterate over connections, instantiate and then establish them.
  en = [_connections objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      if ([obj respondsToSelector: @selector(instantiateWithInstantiator:)])
        {
          [obj instantiateWithInstantiator: self];          
          [obj establishConnection];
        }
      else
        {
          if ([obj respondsToSelector: @selector(instantiateWithObjectInstantiator:)])
            {
              [obj instantiateWithObjectInstantiator: self];          
              [obj establishConnection];
            }
        }
    }

  // awaken all objects except proxy objects.
  objs = NSAllMapTableKeys(_objects);
  en = [objs objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      NSInteger oid = [(id)NSMapGet(_oids, obj) intValue];
      if (oid >= 0)
        {
          if ([obj respondsToSelector: @selector(realObject)])
            {
              obj = [obj realObject];
            }
          if ([obj respondsToSelector: @selector(awakeFromNib)])
            {
              [obj awakeFromNib];
            }
        }
    }

  // awaken the owner
  if ([owner respondsToSelector: @selector(awakeFromNib)])
    {
      [owner awakeFromNib];
    }

  // bring visible windows to front...
  en = [_visibleWindows objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      id w = [obj realObject];
      [w orderFront: self];
    }

  // add the menu...
  menu = [self objectForName: @"MainMenu"];
  if (menu != nil)
    {
      menu = [self instantiateObject: menu];
      [NSApp _setMainMenu: menu];
    }
}

/**
 * Awake after loading the nib and extract the top level and owner for nib instantiation,
 * then call nibInstantateWithOwner:topLevelObjects:
 */
- (void) awakeWithContext: (NSDictionary *)context
{
  NSMutableArray *tlo = [context objectForKey: NSNibTopLevelObjects];
  id owner = [context objectForKey: NSNibOwner];

  // instantiate...
  [self nibInstantiateWithOwner: owner topLevelObjects: tlo];
}

/**
 * Retrieve an object by name from the map.
 */
- (id) objectForName: (NSString *)name
{
  NSArray *nameKeys = (NSArray *)NSAllMapTableKeys(_names);
  NSArray *nameValues = (NSArray *)NSAllMapTableValues(_names);
  NSUInteger i = [nameValues indexOfObject: name];
  id result = nil;

  if (i != NSNotFound)
    {
      result = [nameKeys objectAtIndex: i];
    }

  return result;
}

/**
 * Get the name for an object.
 */
- (NSString *) nameForObject: (id)obj
{
  NSArray *nameKeys = (NSArray *)NSAllMapTableKeys(_names);
  NSArray *nameValues = (NSArray *)NSAllMapTableValues(_names);
  int i = [nameKeys indexOfObject: obj];
  NSString *result = [nameValues objectAtIndex: i];
  return result;
}

/**
 * Set the root object.
 */
- (void) setRoot: (id) root
{
  ASSIGN(_root, root);
}

/**
 * Return the root object.
 */
- (id) root
{
  return _root;
}

/**
 * Set the value of the next available oid.
 */
- (void) setNextOid: (int)noid
{
  _nextOid = noid;
}

/**
 * Get the value of the next available oid.
 */
- (int) nextOid
{
  return _nextOid;
}

/**
 * Connections between objects.
 */
- (NSMutableArray *) connections
{
  return _connections;
}

/**
 * Set of top level objects.
 */
- (NSMutableSet *) topLevelObjects
{
  return _topLevelObjects;
}

/**
 * Names to objects
 */
- (NSMutableDictionary *) nameTable
{
  return nil;
}

/**
 * Set of all visible windows.
 */
- (NSMutableArray *) visibleWindows
{
  return _visibleWindows;
}

/**
 * Objects to names table.
 */
- (NSMapTable *) objects
{
  return _objects;
}

/**
 * Names to objects table.
 */
- (NSMapTable *) names
{
  return _names;
}

/**
 * Classes to objects table.
 */
- (NSMapTable *) classes
{
  return _classes;
}

/**
 * Oids to objects table.
 */ 
- (NSMapTable *) oids
{
  return _oids;
}
@end

/**
 * NSButtonImageSource
 * 
 * This class is used by buttons to pull the correct image based on a given state.
 */
@implementation NSButtonImageSource
- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      ASSIGN(imageName, [coder decodeObjectForKey: @"NSImageName"]);
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }

  AUTORELEASE(self);
  return RETAIN([NSImage imageNamed: imageName]);
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: imageName forKey: @"NSImageName"];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Can't encode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
}

/**
 * Initializes with image name.
 */
- (id) initWithImageNamed: (NSString *)name
{
  if ((self = [super init]) != nil)
    {
      ASSIGN(imageName,name);
    }
  return self;
}

/**
 * Returns imageName.
 */
- (NSString *)imageName
{
  return imageName;
}

- (void) dealloc
{
  RELEASE(imageName);
  [super dealloc];
}
@end

@implementation NSIBHelpConnector
- (id) init
{
  if ((self = [super init]) != nil)
    {
      _file = nil;
      ASSIGN(_marker, @"NSToolTipHelpKey");
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_file);
  RELEASE(_marker);
  [super dealloc];
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ((self = [super initWithCoder: coder]) != nil)
    {
      if ([coder allowsKeyedCoding])
        {
          if ([coder containsValueForKey: @"NSFile"])
            {
              ASSIGN(_file, [coder decodeObjectForKey: @"NSFile"]);
            }
          if ([coder containsValueForKey: @"NSMarker"])
            {
              ASSIGN(_marker, [coder decodeObjectForKey: @"NSMarker"]);
            }
        }
      else
        {
          ASSIGN(_file, [coder decodeObject]);
          ASSIGN(_marker, [coder decodeObject]);
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      if (_file != nil)
        {
          [coder encodeObject: _file forKey: @"NSFile"];
        }
      if (_marker != nil)
        {
          [coder encodeObject: _marker forKey: @"NSMarker"];
        }      
    }
  else
    {
      [coder encodeObject: _file];
      [coder encodeObject: _marker];
    }
}

- (void) establishConnection
{
  if ([_dst respondsToSelector: @selector(setToolTip:)])
    {
      [_dst setToolTip: _marker];
    }
}

- (void) setFile: (id)file
{
  ASSIGN(_file, file);
}

- (id) file
{
  return _file;
}

- (void) setMarker: (id)marker
{
  ASSIGN(_marker, marker);
}

- (id) marker
{
  return _marker;
}
@end

@implementation NSDecimalNumberPlaceholder
- (id) initWithCoder: (NSCoder *)coder
{
  NSDecimalNumber *dn = nil;
  if ([coder allowsKeyedCoding])
    {
      NSUInteger len = 0;
      short exponent = (short)[coder decodeIntForKey: @"NS.exponent"];
      NSByteOrder bo = [coder decodeIntForKey: @"NS.mantissa.bo"];
      BOOL negative = [coder decodeBoolForKey: @"NS.negative"];
      void *mantissaBytes = (void *)[coder decodeBytesForKey: @"NS.mantissa" returnedLength: &len];
      unsigned long long unswapped = 0; 
      unsigned long long mantissa = 0;

      // BOOL compact = [coder decodeBoolForKey: @"NS.compact"];
      // int length = [coder decodeIntForKey: @"NS.length"];

      memcpy((void *)&unswapped, (void *)mantissaBytes, sizeof(unsigned long long));

      switch(bo)
        {
        case NS_BigEndian:
          mantissa = NSSwapBigLongLongToHost(unswapped);
          break;
        case NS_LittleEndian:
          mantissa = NSSwapLittleLongLongToHost(unswapped);
          break;
        default:
          break;
        }

      dn = [[NSDecimalNumber alloc] initWithMantissa: mantissa
                                    exponent: exponent
                                    isNegative: negative];
    }

  RELEASE(self);
  return (id)dn;
}

@end

/**
 * NSCornerView
 *
 * Overridden in NSTableView to be GSTableCornerView, 
 * but the class needs to be present to be overridden.
 * 
 * Currently this is a place-holder class.
 */
@implementation _NSCornerView
@end

/**
 * NSPSMatrix.
 *
 * This class is needed for nib encoding/decoding by transforms.  
 * Currently it's only referenced in the NSProgressIndicator,
 * as far as I can tell.
 *
 * Place holder class.
 */
@implementation NSPSMatrix
- (void) encodeWithCoder: (NSCoder *)coder
{
  // do nothing... just encoding the presence of the class.
}

- (id) initWithCoder: (NSCoder *)coder
{
  return self;
}
@end

@implementation NSIBUserDefinedRuntimeAttributesConnector
- (void) setObject: (id)object
{
  ASSIGN(_object, object);
}

- (id) object
{
  return _object;
}

- (void) setValues: (id)values
{
  ASSIGN(_values, values);
}

- (id) values
{
  return _values;
}

- (void) setKeyPaths: (id)keyPaths
{
  ASSIGN(_keyPaths, keyPaths);
}

- (id) keyPaths
{
  return _keyPaths;
}

- (void) dealloc
{
  RELEASE(_object);
  RELEASE(_keyPaths);
  RELEASE(_values);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      if (_object != nil)
        {
          [coder encodeObject: _object forKey: @"NSObject"];
        }
      if (_keyPaths != nil)
        {
          [coder encodeObject: _keyPaths forKey: @"NSKeyPaths"];
        }      
      if (_values != nil)
        {
          [coder encodeObject: _values forKey: @"NSValues"];
        }      
    }
  else
    {
      [coder encodeObject: _object];
      [coder encodeObject: _keyPaths];
      [coder encodeObject: _values];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSObject"])
        {
          ASSIGN(_object, [coder decodeObjectForKey: @"NSObject"]);
        }
      if ([coder containsValueForKey: @"NSKeyPaths"])
        {
          ASSIGN(_keyPaths, [coder decodeObjectForKey: @"NSKeyPaths"]);
        }
      if ([coder containsValueForKey: @"NSValues"])
        {
          ASSIGN(_values, [coder decodeObjectForKey: @"NSValues"]);
        }
    }
  else
    {
      ASSIGN(_object, [coder decodeObject]);
      ASSIGN(_keyPaths, [coder decodeObject]);
      ASSIGN(_values, [coder decodeObject]);
    }
  
  return self;
}

- (void) establishConnection
{
  // Loop over key paths and values and use KVC on object
  NSEnumerator *keyEn = [_keyPaths objectEnumerator];
  NSEnumerator *valEn = [_values objectEnumerator];
  id key;

  while ((key = [keyEn nextObject]) != nil)
    {
      id val = [valEn nextObject];

      [_object setValue: val forKeyPath: key];
    }
}

- (void) instantiateWithObjectInstantiator: (id)instantiator
{
  [self setObject: [(id<GSInstantiator>)instantiator instantiateObject: _object]];
  // FIXME Should handle values too
}

@end
