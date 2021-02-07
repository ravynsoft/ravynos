/*
   <title>NSToolbar.m</title>

   <abstract>The toolbar class.</abstract>

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>,
            Fabien Vallon <fabien.vallon@fr.alcove.com>,
            Quentin Mathe <qmathe@club-internet.fr>
   Date: May 2002

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

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSDecimalNumber.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSToolbar.h"
#import "AppKit/NSToolbarItem.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSToolbarView.h"

#import "NSToolbarFrameworkPrivate.h"
#import "GSToolbarCustomizationPalette.h"

// internal
static NSNotificationCenter *nc = nil;
static NSMutableArray *toolbars = nil;
static const int current_version = 1;

// Validation stuff
static const unsigned int ValidationInterval = 4;
@class GSValidationCenter; // Mandatory because the interface is declared later
static GSValidationCenter *vc = nil;

// Extensions
@interface NSArray (ObjectsWithValueForKey)
- (NSArray *) objectsWithValue: (id)value forKey: (NSString *)key;
@end

@implementation NSArray (ObjectsWithValueForKey)

- (NSArray *) objectsWithValue: (id)value forKey: (NSString *)key
{
  NSMutableArray *result = [NSMutableArray array];
  int i = 0;
  int n = [self count];

  for (i = 0; i < n; i++)
    {
      id obj = [self objectAtIndex: i];
      if ([[obj valueForKey: key] isEqual: value])
        {
          [result addObject: obj];
        }
    }

  if ([result count] == 0)
    return nil;

  return result;
}
@end

/*
 * Validation support
 *
 * Validation support is architectured around a shared validation center, which
 * is our public interface to handle the validation, behind the scene each
 * window has an associated validation object created when an observer is added
 * to the validation center.
 * A validation object calls the _validate: method on the observer when the
 * mouse is inside the observed window and only in the case this window is
 * updated or in the case the mouse stays inside more than four seconds, then
 * the action will be reiterated every four seconds until the mouse exits.
 * A validation object owns a window to observe, a tracking rect attached to
 * the window root view to know when the mouse is inside, a timer to be able to
 * send the _validate: message periodically, and one ore more observers, then it
 * is necessary to supply with each registered observer an associated window to
 * observe.
 * In the case, an object would observe several windows, the _validate: has a
 * parameter observedWindow to let us know where the message is coming from.
 * Because we cannot know surely when a validation object is deallocated, a
 * method named clean has been added which permits to invalidate a validation
 * object which must not be used anymore, not calling it would let segmentation
 * faults occurs.
 */

@interface GSValidationObject : NSObject
{
  NSWindow *_window;
  NSView *_trackingRectView;
  NSTrackingRectTag _trackingRect;
  NSMutableArray *_observers;
  NSTimer *_validationTimer;
  BOOL _inside;
  BOOL _validating;
}

- (NSMutableArray *) observers;
- (void) setObservers: (NSMutableArray *)observers;
- (NSWindow *) window;
- (void) setWindow: (NSWindow *)window;
- (void) validate;
- (void) scheduledValidate;
- (void) clean;

@end

@interface GSValidationCenter : NSObject
{
  NSMutableArray *_vobjs;
}

+ (GSValidationCenter *) sharedValidationCenter;

- (NSArray *) observersWindow: (NSWindow *)window;
- (void) addObserver: (id)observer window: (NSWindow *)window;
- (void) removeObserver: (id)observer window: (NSWindow *)window;

@end

// Validation mechanism

@interface NSWindow (GNUstepPrivate)
- (NSView *) _windowView;
@end

@implementation GSValidationObject

- (id) initWithWindow: (NSWindow *)window
{
  if ((self = [super init]) != nil)
    {
      _observers = [[NSMutableArray alloc] init];

      [nc addObserver: self selector: @selector(windowDidUpdate:)
                 name: NSWindowDidUpdateNotification
               object: window];
      [nc addObserver: vc
                   selector: @selector(windowWillClose:)
                 name: NSWindowWillCloseNotification
               object: window];

       _trackingRectView = [window _windowView];
       _trackingRect
         = [_trackingRectView addTrackingRect: [_trackingRectView bounds]
                                        owner: self
                                     userData: nil
                                 assumeInside: NO];
       _window = window;
    }
  return self;
}

- (void) dealloc
{
  // NSLog(@"vobj dealloc");

  // [_trackingRectView removeTrackingRect: _trackingRect];
  // Not here because the tracking rect retains us, then when the tracking rect
  // would be deallocated that would create a loop and a segmentation fault.
  // See next method.

  RELEASE(_observers);

  [super dealloc];
}

- (void) clean
{
  if ([_validationTimer isValid])
    {
      [_validationTimer invalidate];
      _validationTimer = nil;
    }

  [nc removeObserver: vc
                name: NSWindowWillCloseNotification
              object: _window];
  [nc removeObserver: self
                name: NSWindowDidUpdateNotification
              object: _window];

  [self setWindow: nil];
  // Needed because the validation timer can retain us and by this way retain also the toolbar which is
  // currently observing.

  [self setObservers: nil]; // To release observers

  [_trackingRectView removeTrackingRect: _trackingRect];
  // We can safely remove the tracking rect here, because it will never call
  // this method unlike dealloc.
}

- (id) valueForUndefinedKey: (NSString *)key
{
  if ([key isEqualToString: @"window"] || [key isEqualToString: @"_window"])
    {
      return nil;
    }

  return [super valueForUndefinedKey: key];
}

- (NSMutableArray *) observers
{
  return _observers;
}

- (void) setObservers: (NSMutableArray *)observers
{
  ASSIGN(_observers, observers);
}

- (NSWindow *) window
{
  return _window;
}

- (void) setWindow: (NSWindow *)window
{
  _window = window;
}

- (void) validate
{
  if (_validating == NO)
    {
      _validating = YES;
      NS_DURING
	{
          [_observers makeObjectsPerformSelector: @selector(_validate:)
                                      withObject: _window];
          _validating = NO;
	}
      NS_HANDLER
	{
          _validating = NO;
	  NSLog(@"Problem validating toolbar: %@", localException);
	}
      NS_ENDHANDLER
    }
}

- (void) mouseEntered: (NSEvent *)event
{
  _inside = YES;
  [self scheduledValidate];
}

- (void) mouseExited: (NSEvent *)event
{
  _inside = NO;
  if ([_validationTimer isValid])
    {
      [_validationTimer invalidate];
      _validationTimer = nil;
    }
}

- (void) windowDidUpdate: (NSNotification *)notification
{
  // Validate the toolbar for each update of the window.
  [self validate];
}

- (void) scheduledValidate
{
  if (!_inside)
    return;

  [self validate];

  _validationTimer =
    [NSTimer timerWithTimeInterval: ValidationInterval
                            target: self
                          selector: @selector(scheduledValidate)
                          userInfo: nil
                           repeats: NO];
  [[NSRunLoop currentRunLoop] addTimer: _validationTimer
                               forMode: NSDefaultRunLoopMode];
}

@end


@implementation GSValidationCenter

+ (GSValidationCenter *) sharedValidationCenter
{
  if (vc == nil)
    {
      if ((vc = [[GSValidationCenter alloc] init]) != nil)
        {
           // Nothing special
        }
    }

  return vc;
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      _vobjs = [[NSMutableArray alloc] init];
    }

  return self;
}

- (void) dealloc
{
  [nc removeObserver: self];

  RELEASE(_vobjs);

  [super dealloc];
}

- (GSValidationObject *) validationObjectForWindow: (NSWindow*)w
{
  return [[_vobjs objectsWithValue: w forKey: @"_window"] objectAtIndex: 0];
}

- (NSArray *) observersWindow: (NSWindow *)window
{
  int i;
  NSArray *observersArray;
  NSMutableArray *result;

  if (window == nil)
    {
      result = [NSMutableArray array];
      observersArray = [_vobjs valueForKey: @"_observers"];
      for (i = 0; i < [observersArray count]; i++)
        {
          [result addObjectsFromArray: [observersArray objectAtIndex: i]];
        }
      return result;
    }
  else
    {
      return [[self validationObjectForWindow: window] observers];
    }
}

- (void) addObserver: (id)observer window: (NSWindow *)window
{
  GSValidationObject *vobj = [self validationObjectForWindow: window];
  NSMutableArray *observersWindow = nil;

  if (window == nil)
    return;

  if (vobj != nil)
    {
      observersWindow = [vobj observers];
    }
  else
    {
      vobj = [[GSValidationObject alloc] initWithWindow: window];
      [_vobjs addObject: vobj];
      RELEASE(vobj);

      observersWindow = [NSMutableArray array];
      [vobj setObservers: observersWindow];
    }

  [observersWindow addObject: observer];
}

- (void) removeObserver: (id)observer window: (NSWindow *)window
{
  GSValidationObject *vobj;
  NSMutableArray *observersWindow;
  NSArray *windows;
  NSEnumerator *e;
  NSWindow *w;

  if (window == nil)
    {
      windows = [_vobjs valueForKey: @"_window"];
    }
  else
    {
      windows = [NSArray arrayWithObject: window];
    }

  e = [windows objectEnumerator];

  while ((w = [e nextObject]) != nil)
    {
      vobj = [self validationObjectForWindow: w];
      observersWindow = [vobj observers];

      if (observersWindow != nil && [observersWindow containsObject: observer])
        {
          [observersWindow removeObject: observer];
          if ([observersWindow count] == 0)
            {
              [vobj clean];
              [_vobjs removeObjectIdenticalTo: vobj];
            }
        }
    }

}

- (void) windowWillClose: (NSNotification *)notification
{
  GSValidationObject *vobj;

  // NSLog(@"Window will close");

  vobj = [self validationObjectForWindow: [notification object]];
  if (vobj != nil)
    {
      [vobj clean];
      [_vobjs removeObjectIdenticalTo: vobj];
    }
}

@end

// ---

@implementation NSToolbar

// Class methods

// Initialize the class when it is loaded
+ (void) initialize
{
  if (self == [NSToolbar class])
    {
      [self setVersion: current_version];
      nc = [NSNotificationCenter defaultCenter];
      vc = [GSValidationCenter sharedValidationCenter];
      toolbars = [[NSMutableArray alloc] init];
    }
}

// Private class method to access static variable toolbars in subclasses

+ (NSArray *) _toolbarsWithIdentifier: (NSString *)identifier
{
  return [toolbars objectsWithValue: identifier
                             forKey: @"_identifier"];
}

// Instance methods

- (id) initWithIdentifier: (NSString *)identifier
{
  NSToolbar *toolbarModel = nil;

  if ((self = [super init]) == nil)
    {
      return nil;
    }

  ASSIGN(_identifier, identifier);

  _items = [[NSMutableArray alloc] init];

  // Only set when loaded from a nib
  _interfaceBuilderItemsByIdentifier = nil;
  _interfaceBuilderAllowedItemIdentifiers = nil;
  _interfaceBuilderDefaultItemIdentifiers = nil;
  _interfaceBuilderSelectableItemIdentifiers = nil;

  toolbarModel = [self _toolbarModel];

  if (toolbarModel != nil)
    {
      _customizationPaletteIsRunning = NO;
      _allowsUserCustomization = [toolbarModel allowsUserCustomization];
      _autosavesConfiguration = [toolbarModel autosavesConfiguration];
      ASSIGN(_configurationDictionary, [toolbarModel configurationDictionary]);
      _displayMode = [toolbarModel displayMode];
      _sizeMode = [toolbarModel sizeMode];
      _visible = [toolbarModel isVisible];
    }
  else
    {
      _customizationPaletteIsRunning = NO;
      _allowsUserCustomization = NO;
      _autosavesConfiguration = NO;
      _configurationDictionary = nil;
      _displayMode = NSToolbarDisplayModeIconAndLabel;
      _sizeMode = NSToolbarSizeModeRegular;
      _visible = YES;
    }

  _delegate = nil;
  [self setShowsBaselineSeparator: YES];

  // Store in list of toolbars
  [toolbars addObject: self];

  return self;
}

- (NSArray *) _identifiersForItems: (NSArray*)items
{
  NSMutableArray *result = [NSMutableArray arrayWithCapacity: [items count]];
  NSEnumerator *e = [items objectEnumerator];
  NSToolbarItem *item;

  if (items == nil)
    return nil;

  while ((item = [e nextObject]) != nil)
    {
      [result addObject: [item itemIdentifier]];
    }
  return result;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      // FIXME
    }
}

- (id) initWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      ASSIGN(_identifier, [aCoder decodeObjectForKey: @"NSToolbarIdentifier"]);
      _items = [[NSMutableArray alloc] init];

      ASSIGN(_interfaceBuilderItemsByIdentifier, [aCoder decodeObjectForKey: @"NSToolbarIBIdentifiedItems"]);
      ASSIGN(_interfaceBuilderAllowedItemIdentifiers, [self _identifiersForItems: [aCoder decodeObjectForKey: @"NSToolbarIBAllowedItems"]]);
      ASSIGN(_interfaceBuilderDefaultItemIdentifiers, [self _identifiersForItems: [aCoder decodeObjectForKey: @"NSToolbarIBDefaultItems"]]);
      ASSIGN(_interfaceBuilderSelectableItemIdentifiers, [self _identifiersForItems: [aCoder decodeObjectForKey: @"NSToolbarIBSelectableItems"]]);

      _customizationPaletteIsRunning = NO;
      _configurationDictionary = nil;

      [self setAllowsUserCustomization: [aCoder decodeBoolForKey: @"NSToolbarAllowsUserCustomization"]];
      [self setAutosavesConfiguration: [aCoder decodeBoolForKey: @"NSToolbarAutosavesConfiguration"]];
      [self setDisplayMode: [aCoder decodeIntForKey: @"NSToolbarDisplayMode"]];
      [self setShowsBaselineSeparator: [aCoder decodeBoolForKey: @"NSToolbarShowsBaselineSeparator"]];
      [self setSizeMode: [aCoder decodeIntForKey: @"NSToolbarSizeMode"]];
      [self setVisible: [aCoder decodeBoolForKey: @"NSToolbarPrefersToBeShown"]];
      [self setDelegate: [aCoder decodeObjectForKey: @"NSToolbarDelegate"]];
    }
  else
    {
      // FIXME
    }

  [self _build];

  // Store in list of toolbars
  [toolbars addObject: self];

   return self;
}

- (void) dealloc
{
  //NSLog(@"Toolbar dealloc %@", self);
  [self _setToolbarView: nil];

  DESTROY(_identifier);
  DESTROY(_selectedItemIdentifier);
  DESTROY(_configurationDictionary);
  DESTROY(_items);

  DESTROY(_interfaceBuilderItemsByIdentifier);
  DESTROY(_interfaceBuilderAllowedItemIdentifiers);
  DESTROY(_interfaceBuilderDefaultItemIdentifiers);
  DESTROY(_interfaceBuilderSelectableItemIdentifiers);

  if (_delegate != nil)
    {
      [nc removeObserver: _delegate  name: nil  object: self];
      _delegate = nil;
    }

  [super dealloc];
}

// FIXME: Hack
- (oneway void) release
{
  // When a toolbar has no external references any more, it's necessary
  // to remove the toolbar from the master list, so that it
  // doesn't cause a memory leak.
  if ([self retainCount] == 2)
    [toolbars removeObjectIdenticalTo: self];

  [super release];
}

- (id) valueForUndefinedKey: (NSString *)key
{
  if ([key isEqualToString: @"window"] || [key isEqualToString: @"_window"])
    {
      return nil;
    }

  return [super valueForUndefinedKey: key];
}

- (void) insertItemWithItemIdentifier: (NSString *)itemIdentifier
                              atIndex: (NSInteger)index
{
  [self _insertItemWithItemIdentifier: itemIdentifier
                              atIndex: index
                            broadcast: YES];
}

- (void) removeItemAtIndex: (NSInteger)index
{
  [self _removeItemAtIndex: index broadcast: YES];
}

- (void) runCustomizationPalette: (id)sender
{
  GSToolbarCustomizationPalette *palette;

  if (![self allowsUserCustomization])
    {
      return;
    }

  if (_customizationPaletteIsRunning)
    {
      NSLog(@"Customization palette is already running for toolbar: %@", self);
      return;
    }

  if (!_visible)
    {
      [self setVisible: YES];
    }

  palette = [GSToolbarCustomizationPalette palette];

  if (palette != nil)
    {
      _customizationPaletteIsRunning = YES;

      [palette showForToolbar: self];
    }
}

- (void) validateVisibleItems
{
  NSEnumerator *e = [[self visibleItems] objectEnumerator];
  NSToolbarItem *item = nil;

  while ((item = [e nextObject]) != nil)
    {
      [item validate];
    }
}

// Accessors

- (BOOL) allowsUserCustomization
{
  return _allowsUserCustomization;
}

- (BOOL) autosavesConfiguration
{
  return _autosavesConfiguration;
}

- (NSDictionary *) configurationDictionary
{
  return _configurationDictionary;
}

- (BOOL) customizationPaletteIsRunning
{
  return _customizationPaletteIsRunning;
}

- (void) _setCustomizationPaletteIsRunning: (BOOL)isRunning
{
  _customizationPaletteIsRunning = isRunning;
}

- (id) delegate
{
  return _delegate;
}

- (NSToolbarDisplayMode) displayMode
{
  return _displayMode;
}

- (NSString *) identifier
{
  return _identifier;
}

- (NSArray *) items
{
  return _items;
}

- (NSString *) selectedItemIdentifier
{
  return _selectedItemIdentifier;
}

- (BOOL) showsBaselineSeparator
{
  return _showsBaselineSeparator;
}

- (NSArray *) visibleItems
{
  if ([_toolbarView superview] == nil)
    {
      return nil;
    }
  else
    {
      return [[_toolbarView _visibleBackViews] valueForKey: @"toolbarItem"];
    }
}

- (void) setAllowsUserCustomization: (BOOL)flag
{
  [self _setAllowsUserCustomization: flag broadcast: YES];
}

- (void) setAutosavesConfiguration: (BOOL)flag
{
  [self _setAutosavesConfiguration: flag broadcast: YES];
}

- (void) setConfigurationFromDictionary: (NSDictionary *)configDict
{
  int i = 0;
  id item = nil;
  NSArray *items = nil;
  NSEnumerator *en = nil;

  ASSIGN(_configurationDictionary, configDict);

  // set
  _visible = [[_configurationDictionary objectForKey: @"isVisible"] boolValue];
  _displayMode = [[_configurationDictionary objectForKey: @"displayMode"] intValue];

  // remove all items...
  for (i = 0; i < [_items count]; i++)
    {
      [self _removeItemAtIndex: 0 broadcast: YES];
    }

  // add all of the items...
  i = 0;
  items = [_configurationDictionary objectForKey: @"items"];
  en = [items objectEnumerator];
  while ((item = [en nextObject]) != nil)
    {
      [self _insertItemWithItemIdentifier: item
                                  atIndex: i++
                                broadcast: YES];
    }
}

/**
 * Sets the receivers delegate ... this is the object which will receive
 * -toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar:
 * -toolbarAllowedItemIdentifiers: and -toolbarDefaultItemIdentifiers:
 * messages.
 */
- (void) setDelegate: (id)delegate
{
  if (_delegate == delegate)
    return;

  if (_delegate != nil)
    [nc removeObserver: _delegate name: nil object: self];

  // Assign the delegate...
  _delegate = delegate;

  if (_delegate != nil)
    {
      #define CHECK_REQUIRED_METHOD(selector_name) \
      if (![_delegate respondsToSelector: @selector(selector_name)]) \
	  [NSException raise: NSInternalInconsistencyException		\
		      format: @"delegate does not respond to %@",@#selector_name]

      if (_interfaceBuilderItemsByIdentifier == nil)
	{
	  CHECK_REQUIRED_METHOD(toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar:);
	}
      if (_interfaceBuilderAllowedItemIdentifiers == nil)
	{
	  CHECK_REQUIRED_METHOD(toolbarAllowedItemIdentifiers:);
	}
      if (_interfaceBuilderDefaultItemIdentifiers == nil)
	{
	  CHECK_REQUIRED_METHOD(toolbarDefaultItemIdentifiers:);
	}

      #define SET_DELEGATE_NOTIFICATION(notif_name) \
      if ([_delegate respondsToSelector: @selector(toolbar##notif_name:)]) \
        [nc addObserver: _delegate \
               selector: @selector(toolbar##notif_name:) \
                   name: NSToolbar##notif_name##Notification object: self]

      SET_DELEGATE_NOTIFICATION(DidRemoveItem);
      SET_DELEGATE_NOTIFICATION(WillAddItem);
    }

  [self _build];
}

- (NSArray *) _selectableItemIdentifiers
{
  NSArray *selectableIdentifiers = nil;

  if (_delegate != nil &&
      [_delegate respondsToSelector: @selector(toolbarSelectableItemIdentifiers:)])
    {
      selectableIdentifiers = [_delegate toolbarSelectableItemIdentifiers: self];
      if (selectableIdentifiers == nil)
	{
	  NSLog(@"Toolbar delegate returns no such selectable item identifiers");
	}
    }

  if (selectableIdentifiers == nil)
    {
      selectableIdentifiers = _interfaceBuilderSelectableItemIdentifiers;
    }

  return selectableIdentifiers;
}

- (NSArray *) _itemsWithIdentifier: (NSString *)identifier
{
  return [[self items] objectsWithValue: identifier
                                 forKey: @"_itemIdentifier"];
}

- (void) setSelectedItemIdentifier: (NSString *)identifier
{
  NSArray *selectedItems;
  NSArray *itemsToSelect;
  NSEnumerator *e;
  NSToolbarItem *item;
  NSArray *selectableIdentifiers;
  BOOL updated = NO;

  //  First, we have to deselect the previous selected toolbar items
  selectedItems = [self _itemsWithIdentifier: [self selectedItemIdentifier]];
  e = [selectedItems objectEnumerator];
  while ((item = [e nextObject]) != nil)
    {
      [item _setSelected: NO];
    }

  selectableIdentifiers = [self _selectableItemIdentifiers];
  if ((selectableIdentifiers == nil)
      || ![selectableIdentifiers containsObject: identifier])
    {
      return;
    }

  itemsToSelect = [self _itemsWithIdentifier: identifier];
  e = [itemsToSelect objectEnumerator];
  while ((item = [e nextObject]) != nil)
    {
      if (![item _selected])
        {
          [item _setSelected: YES];
        }
      updated = YES;
    }

  if (updated)
    {
      ASSIGN(_selectedItemIdentifier, identifier);
    }
  else
    {
      NSLog(@"Toolbar delegate returns no such selectable item identifiers");
    }
}

- (NSToolbarSizeMode) sizeMode
{
  return _sizeMode;
}

- (BOOL) isVisible
{
  return _visible;
}

- (void) setDisplayMode: (NSToolbarDisplayMode)displayMode
{
  [self _setDisplayMode: displayMode broadcast: YES];
}

- (void) setSizeMode: (NSToolbarSizeMode)sizeMode
{
  [self _setSizeMode: sizeMode broadcast: YES];
}

- (void) setVisible: (BOOL)shown
{
  [self _setVisible: shown broadcast: NO];
}

- (void) setShowsBaselineSeparator: (BOOL)flag
{
  _showsBaselineSeparator = flag;

  if (_showsBaselineSeparator)
    [_toolbarView setBorderMask: GSToolbarViewBottomBorder];
  else
    [_toolbarView setBorderMask: 0];
}

// Private methods

- (NSArray *) _defaultItemIdentifiers
{
  if (_delegate != nil)
    {
      return [_delegate toolbarDefaultItemIdentifiers: self];
    }
  else
    {
      return _interfaceBuilderDefaultItemIdentifiers;
    }
}

/*
 * Toolbar build :
 * will use the delegate when there is no toolbar model
 */
- (void) _build
{
  NSArray *wantedItemIdentifiers = nil;
  NSEnumerator *e;
  id itemIdentifier;
  NSDictionary *config = [self _config];

  // Switch off toolbar view reload
  _build = YES;

  if (config)
    {
      NSToolbarDisplayMode displayMode = 0;
      NSToolbarSizeMode sizeMode = 0;

      displayMode = (NSToolbarDisplayMode)[[config objectForKey: @"displayMode"] intValue];
      [self setDisplayMode: displayMode];
      sizeMode = (NSToolbarSizeMode)[[config objectForKey: @"sizeMode"] intValue];
      [self setSizeMode: sizeMode];
      wantedItemIdentifiers = [config objectForKey: @"items"];
    }

  RELEASE(_items);
  _items = [[NSMutableArray alloc] init];

  if (wantedItemIdentifiers == nil)
    {
      NSToolbar *toolbarModel = [self _toolbarModel];

      if (toolbarModel != nil)
	{
	  wantedItemIdentifiers =
	    [[toolbarModel items] valueForKey: @"_itemIdentifier"];
	}
      else
	{
	  wantedItemIdentifiers = [self _defaultItemIdentifiers];
	}
    }

  if (wantedItemIdentifiers == nil)
    {
      _build = NO;
      return;
    }

  e = [wantedItemIdentifiers objectEnumerator];
  while ((itemIdentifier = [e nextObject]) != nil)
    {
      [self _insertItemWithItemIdentifier: itemIdentifier
                                  atIndex: [_items count]
                                broadcast: NO];
    }

  _build = NO;

  // Now do the toolbar view reload
  if (_toolbarView != nil)
    [_toolbarView _reload];
}

- (BOOL) usesStandardBackgroundColor
{
  return [_toolbarView _usesStandardBackgroundColor];
}

- (void) setUsesStandardBackgroundColor: (BOOL)standard
{
  [_toolbarView _setUsesStandardBackgroundColor: standard];
}

- (int) _indexOfItem: (NSToolbarItem *)item
{
  return [_items indexOfObjectIdenticalTo: item];
}

- (NSDictionary *) _config
{
  if (_identifier != nil)
    {
      NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
      NSString       *tableKey =
        [NSString stringWithFormat: @"NSToolbar Config %@", _identifier];

      return [defaults objectForKey: tableKey];
    }
  else
    {
      return nil;
    }
}

- (void) _resetConfig
{
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *tableKey =
    [NSString stringWithFormat: @"NSToolbar Config %@", _identifier];
  [defaults removeObjectForKey: tableKey];
  [self _build];
}

- (void) _saveConfig
{
  if (_identifier != nil)
    {
      NSUserDefaults     *defaults;
      NSString           *tableKey;
      id                  config;
      NSMutableArray     *items = [NSMutableArray array];
      id                  item;
      NSEnumerator       *en = [_items objectEnumerator];

      defaults  = [NSUserDefaults standardUserDefaults];
      tableKey =
        [NSString stringWithFormat: @"NSToolbar Config %@", _identifier];

      config = [defaults objectForKey: tableKey];

      if (config == nil)
        {
          config = [NSMutableDictionary dictionary];
        }
      else
	{
	  config = [config mutableCopy];
	}

      [config setObject: [NSNumber numberWithBool: _visible] forKey: @"isVisible"];
      [config setObject: [NSNumber numberWithInt: _displayMode] forKey: @"displayMode"];
      [config setObject: [NSNumber numberWithInt: _sizeMode] forKey: @"sizeMode"];
      while((item = [en nextObject]) != nil)
	{
	  [items addObject: [item itemIdentifier]];
	}
      [config setObject: items forKey: @"items"];

      // write to defaults
      [defaults setObject: config forKey: tableKey];
      ASSIGN(_configurationDictionary, config);
    }
}

- (BOOL) _containsItemWithIdentifier: (NSString *)identifier
{
  NSEnumerator *en = [_items objectEnumerator];
  id item = nil;

  while((item = [en nextObject]) != nil)
    {
      if([identifier isEqual: [item itemIdentifier]])
	{
	  return YES;
	}
    }
  return NO;
}

- (NSToolbar *) _toolbarModel
{
  NSArray *linked;
  id toolbar;

  linked = [[self class] _toolbarsWithIdentifier: [self identifier]];

  if (linked != nil && [linked count] > 0)
    {
      toolbar = [linked objectAtIndex: 0];

      // Toolbar model class must be identical to self class
      if ([toolbar isMemberOfClass: [self class]] && toolbar != self)
        {
          return toolbar;
        }
    }

  return nil;
}

- (NSToolbarItem *) _toolbarItemForIdentifier: (NSString *)itemIdent willBeInsertedIntoToolbar: (BOOL)insert
{
  NSToolbarItem *item = nil;

  if ([itemIdent isEqual: NSToolbarSeparatorItemIdentifier] ||
     [itemIdent isEqual: NSToolbarSpaceItemIdentifier] ||
     [itemIdent isEqual: NSToolbarFlexibleSpaceItemIdentifier] ||
     [itemIdent isEqual: NSToolbarShowColorsItemIdentifier] ||
     [itemIdent isEqual: NSToolbarShowFontsItemIdentifier] ||
     [itemIdent isEqual: NSToolbarCustomizeToolbarItemIdentifier] ||
     [itemIdent isEqual: NSToolbarPrintItemIdentifier])
    {
      item = [[[NSToolbarItem alloc] initWithItemIdentifier: itemIdent] autorelease];
    }

  if (item == nil && _delegate != nil)
    {
      item = [_delegate toolbar: self itemForItemIdentifier: itemIdent
				  willBeInsertedIntoToolbar: insert];
    }

  if (item == nil && _interfaceBuilderItemsByIdentifier)
    {
      item = [_interfaceBuilderItemsByIdentifier objectForKey: itemIdent];
    }

  return item;
}


/*
 *
 * The methods below handles the toolbar edition and broadcasts each associated
 * event to the other toolbars with identical identifiers.
 * Warning : broadcast process only happens between instances based on the same
 * class.
 */

#define TRANSMIT(signature) \
  NSEnumerator *e = [[NSToolbar _toolbarsWithIdentifier: _identifier] objectEnumerator]; \
  NSToolbar *toolbar; \
  \
  while ((toolbar = [e nextObject]) != nil) \
    { \
      if (toolbar != self && [toolbar isMemberOfClass: [self class]]) \
        [toolbar signature]; \
    } \

- (NSArray *) _allowedItemIdentifiers
{
  if (_delegate)
    {
      return [_delegate toolbarAllowedItemIdentifiers: self];
    }
  else
    {
      return _interfaceBuilderAllowedItemIdentifiers;
    }
}

- (void) _insertItemWithItemIdentifier: (NSString *)itemIdentifier
                               atIndex: (int)index
                             broadcast: (BOOL)broadcast
{
  NSToolbarItem *item = nil;
  NSArray *allowedItems = [self _allowedItemIdentifiers];

  if ([allowedItems containsObject: itemIdentifier])
    {
      item = [self _toolbarItemForIdentifier: itemIdentifier willBeInsertedIntoToolbar: YES];

      if (item != nil)
        {
          NSArray *selectableItems = [self _selectableItemIdentifiers];

          if ([selectableItems containsObject: itemIdentifier])
	    [item _setSelectable: YES];

          [nc postNotificationName: NSToolbarWillAddItemNotification
                            object: self
                          userInfo: [NSDictionary dictionaryWithObject: item  forKey: @"item"]];
          [item _setToolbar: self];
		  [item _layout];
          [_items insertObject: item atIndex: index];

          // We reload the toolbarView each time a new item is added except when
          // we build/create the toolbar
          if (!_build)
            [_toolbarView _reload];

          if (broadcast)
            {
              TRANSMIT(_insertItemWithItemIdentifier: itemIdentifier
                                             atIndex: index
                                           broadcast: NO);
            }
        }
    }

}

- (void) _removeItemAtIndex: (int)index broadcast: (BOOL)broadcast
{
  id item = [_items objectAtIndex: index];

  RETAIN(item);
  [_items removeObject: item];
  [_toolbarView _reload];
  [self _saveConfig];

  [nc postNotificationName: NSToolbarDidRemoveItemNotification
                    object: self
                  userInfo: [NSDictionary dictionaryWithObject: item  forKey: @"item"]];
  RELEASE(item);

  if (broadcast)
    {
      TRANSMIT(_removeItemAtIndex: index broadcast: NO);
    }
}

- (void) _setAllowsUserCustomization: (BOOL)flag broadcast: (BOOL)broadcast
{
  _allowsUserCustomization = flag;

  if (broadcast)
    {
      TRANSMIT(_setAllowsUserCustomization: _allowsUserCustomization
                                 broadcast: NO);
    }
}

- (void) _setAutosavesConfiguration: (BOOL)flag broadcast: (BOOL)broadcast
{
  _autosavesConfiguration = flag;

  if (broadcast)
    {
      TRANSMIT(_setAutosavesConfiguration: _autosavesConfiguration
                                broadcast: NO);
    }
}

- (void) _setConfigurationFromDictionary: (NSDictionary *)configDict
                               broadcast: (BOOL)broadcast
{
  ASSIGN(_configurationDictionary, configDict);

  if (broadcast)
    {
      TRANSMIT(_setConfigurationFromDictionary: _configurationDictionary
                                       broadcast: NO);
    }
}

- (void) _moveItemFromIndex: (int)index toIndex: (int)newIndex broadcast: (BOOL)broadcast
{
  id item;

  item = RETAIN([_items objectAtIndex: index]);
  [_items removeObjectAtIndex: index];
  if (newIndex > [_items count] - 1)
    {
      [_items addObject: item];
    }
  else
    {
      [_items insertObject: item atIndex: newIndex];
    }
  [_toolbarView _reload];

  RELEASE(item);

  if (broadcast)
    {
      TRANSMIT(_moveItemFromIndex: index toIndex: newIndex broadcast: NO);
    }
}

- (void) _setDisplayMode: (NSToolbarDisplayMode)displayMode
               broadcast: (BOOL)broadcast
{
  if (_displayMode != displayMode)
    {
      _displayMode = displayMode;

      if ([self isVisible])
        {
          [_toolbarView _reload];
          [(id)[[_toolbarView window] _windowView] adjustToolbarView: _toolbarView];
        }

      if (broadcast)
        {
          TRANSMIT(_setDisplayMode: _displayMode broadcast: NO);
        }
    }
}

- (void) _setSizeMode: (NSToolbarSizeMode)sizeMode
            broadcast: (BOOL)broadcast
{
  if (_sizeMode != sizeMode)
    {
      _sizeMode = sizeMode;

      if ([self isVisible])
        {
          [_toolbarView _reload];
          [(id)[[_toolbarView window] _windowView] adjustToolbarView: _toolbarView];
        }

      if (broadcast)
        {
          TRANSMIT(_setSizeMode: _sizeMode broadcast: NO);
        }
    }
}

- (NSWindow*) _window
{
  NSWindow *window = [_toolbarView window];
  NSEnumerator *wenum;

  if (window)
    return window;

  wenum = [GSAllWindows() objectEnumerator];
  while ((window = [wenum nextObject]))
    {
      if ([window toolbar] == self)
        return window;
    }

  return nil;
}

- (void) _setVisible: (BOOL)shown broadcast: (BOOL)broadcast
{
  if (_visible != shown)
    {
       _visible = shown;

       if (shown)
         [self _build];

       if (shown)
         {
           if ((_toolbarView == nil) || ([_toolbarView superview] == nil))
             {
               NSWindow *w = [self _window];

               [(id)[w _windowView] addToolbarView: [self _toolbarView]];
             }
         }
       else
         {
           if ((_toolbarView != nil) && ([_toolbarView superview] != nil))
             {
               NSWindow *w = [self _window];

               [(id)[w _windowView] removeToolbarView: [self _toolbarView]];
             }
         }

       if (broadcast)
         {
           TRANSMIT(_setVisible: _visible broadcast: NO);
         }
    }
}

// Private Accessors

- (void) _setToolbarView: (GSToolbarView *)toolbarView
{
  if (_toolbarView == toolbarView)
    return;

  if (_toolbarView != nil)
    {
      // We unset the toolbar from the previous toolbar view
      [_toolbarView setToolbar: nil];
      [vc removeObserver: self window: nil];
    }

  ASSIGN(_toolbarView, toolbarView);

  if (toolbarView != nil)
    {
      // In the case the window parameter is a nil value, nothing happens.
      [vc addObserver: self window: [toolbarView window]];
      // We set the toolbar on the new toolbar view
      [_toolbarView setToolbar: self];
    }
}

- (GSToolbarView *) _toolbarView
{
  if (_toolbarView == nil)
    {
      // Instantiate the toolbar view
      // addToolbarView: method will set the toolbar view to the right
      // frame
      GSToolbarView *toolbarView = [[GSToolbarView alloc]
                                       initWithFrame:
                                           NSMakeRect(0, 0, 100, 100)];

      [toolbarView setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];
      if (_showsBaselineSeparator)
          [toolbarView setBorderMask: GSToolbarViewBottomBorder];
      else
          [toolbarView setBorderMask: 0];

      // Load the toolbar view inside the toolbar
      _toolbarView = toolbarView;
      [_toolbarView setToolbar: self];
    }

  return _toolbarView;
}

- (void) _toolbarViewWillMoveToSuperview: (NSView *)newSuperview
{
  // Must synchronize the validation system
  // _toolbarView should never be nil here
  // We don't handle synchronization when the toolbar view is added to a superview not
  // binded to a window, such superview being later moved to a window. (FIX ME ?)

  // NSLog(@"Moving to window %@", [newSuperview window]);

  [vc removeObserver: self window: nil];
  if (newSuperview != nil)
    [vc addObserver: self window: [newSuperview window]];
}

- (void) _validate: (NSWindow *)observedWindow
{
  // We observe only one window, then we ignore observedWindow.

  [self validateVisibleItems];
}

@end
