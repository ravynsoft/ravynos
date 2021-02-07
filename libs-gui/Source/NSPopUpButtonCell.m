/** <title>NSPopUpButtonCell</title>

   Copyright (C) 1999 Free Software Foundation, Inc.
   
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Jul 2003
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: 1999

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
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSPopUpButtonCell.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSBindingHelpers.h"
#import "GSGuiPrivate.h"

/* The image to use in a specific popupbutton depends on type and
 * preferred edge; that is, _pbc_image[0] if it is a
 * popup menu, _pbc_image[1] if it is a pulls down list pointing down, 
 * and so on.  */
static NSImage *_pbc_image[5];

@interface NSButtonCell (Private)
- (GSThemeControlState) themeControlState;
@end

@interface NSPopUpButtonCell (CocoaExtensions)
- (void) _popUpItemAction: (id)sender;
@end

@implementation NSPopUpButtonCell
+ (void) initialize
{
  if (self == [NSPopUpButtonCell class])
    {
      [self setVersion: 3];
      ASSIGN(_pbc_image[0], [NSImage imageNamed: @"common_Nibble"]);
      ASSIGN(_pbc_image[1], [NSImage imageNamed: @"common_3DArrowDown"]);
      ASSIGN(_pbc_image[2], [NSImage imageNamed: @"common_3DArrowRight"]);
      ASSIGN(_pbc_image[3], [NSImage imageNamed: @"common_3DArrowUp"]);
      ASSIGN(_pbc_image[4], [NSImage imageNamed: @"common_3DArrowLeft"]);
    }
}

+ (BOOL) prefersTrackingUntilMouseUp
{
  return YES;
}

+ (NSFocusRingType) defaultFocusRingType
{
  return NSFocusRingTypeDefault;
}

// Initialization
/**
 * Initialize a blank cell.
 */
- (id) init
{
  return [self initTextCell: @"" pullsDown: NO];
}

/**
 * Initialize with stringValue.
 */
- (id) initTextCell: (NSString *)stringValue
{
  return [self initTextCell: stringValue pullsDown: NO];
}

/*
 * Helper method should be overriden by Gorm
 */
- (void)_initMenu
{
  NSMenu *menu;

  menu = [[NSMenu alloc] initWithTitle: @""];
  [self setMenu: menu];
  RELEASE(menu);
}

/**
 * Initialize with stringValue and pullDown.  If pullDown is YES, the
 * reciever will be a pulldown button.
 */
- (id) initTextCell: (NSString *)stringValue
          pullsDown: (BOOL)flag
{
  self = [super initTextCell: stringValue];
  if (!self)
    return nil;

  [self _initMenu];
  [self setPullsDown: flag];
  _pbcFlags.usesItemFromMenu = YES;
  [self setPreferredEdge: NSMaxYEdge];
  [self setArrowPosition: NSPopUpArrowAtCenter];

  if ([stringValue length] > 0)
    {
      [self addItemWithTitle: stringValue]; 
    }

  return self;
}

- (void) dealloc
{
  /* 
   * The popup must be closed here, just in case the cell goes away 
   * while the popup is still displayed. In that case the notification
   * center would still send notifications to the deallocated cell.
   */
  if ([[_menu window] isVisible])
    {
      [self dismissPopUp];
    }

  if (_menu != nil)
    {
      [self setMenu:nil];
    }
  _selectedItem = nil;
  [super dealloc];
}

// Getters and setters.
/**
 * Set the menu for the popup.
 */ 
- (void) setMenu: (NSMenu *)menu
{
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

  if (_menu == menu)
    {
      return;
    }

  if (_menu != nil)
    {
      [_menu _setOwnedByPopUp: nil];
      [nc removeObserver: self
                    name: nil
                  object: _menu];
    }
  ASSIGN(_menu, menu);
  if (_menu != nil)
    {
      [_menu _setOwnedByPopUp: self];
      /* We need to set the menu view so we trigger the special case 
       * popupbutton code in super class NSMenuItemCell
       */
      [self setMenuView: [_menu menuRepresentation]];
      [nc addObserver: self
             selector: @selector(synchronizeTitleAndSelectedItem)
                 name: NSMenuDidAddItemNotification
               object: _menu];
      [nc addObserver: self
             selector: @selector(synchronizeTitleAndSelectedItem)
                 name: NSMenuDidRemoveItemNotification
               object: _menu];
      [nc addObserver: self
             selector: @selector(synchronizeTitleAndSelectedItem)
                 name: NSMenuDidChangeItemNotification
               object: _menu];
    }
  else 
    {
      [self setMenuView: nil];
    }
  
  // FIXME: Select the first or last item?
  [self selectItemAtIndex: [_menu numberOfItems] - 1];
  [self synchronizeTitleAndSelectedItem];
}

- (void)setMenuView: (NSMenuView *)aMenuView
{
  // NB Don't call NSMenuItemCell's implementation here because it also
  //    changes various items of the receiver, which is reasonable for a
  //    menu item cell that is part of a menu, but not for a pop up button
  //    cell.
  _menuView = aMenuView;
}

/**
 * Return the menu for the popup.
 */
- (NSMenu *) menu
{
  return _menu;
}

/**
 * Set the pull-down state
 */
- (void) setPullsDown: (BOOL)flag
{
  NSMenuItem *item = _menuItem;

  [self setMenuItem: nil];
  if (flag && !_pbcFlags.pullsDown
      && _pbcFlags.altersStateOfSelectedItem)
    {
      [[self selectedItem] setState: NSOffState];
    }

  _pbcFlags.pullsDown = flag;

  [self setMenuItem: item];
  [self synchronizeTitleAndSelectedItem];
}

/**
 * Returns YES, if this is a pull-down 
 */
- (BOOL) pullsDown
{
  return _pbcFlags.pullsDown;
}

/**
 * Set to YES, if the items are to be autoenabled.
 */
- (void) setAutoenablesItems: (BOOL)flag
{
  [_menu setAutoenablesItems: flag];  
}

/**
 * Returns YES, if the items are autoenabled.
 */
- (BOOL) autoenablesItems
{
  return [_menu autoenablesItems];
}

/**
 * Set the preferred edge as described by edge.  This is used
 * to determine the edge which will open the popup when the screen
 * is small.
 */
- (void) setPreferredEdge: (NSRectEdge)preferredEdge
{
  _pbcFlags.preferredEdge = preferredEdge;
}

/**
 * Return the preferred edge.
 */
- (NSRectEdge) preferredEdge
{
  return _pbcFlags.preferredEdge;
}

/**
 * Set to YES, if the reciever should use a menu item for its title. YES
 * is the default.
 */ 
- (void) setUsesItemFromMenu: (BOOL)flag
{
  if (_pbcFlags.usesItemFromMenu != flag)
    {
      _pbcFlags.usesItemFromMenu = flag;
      if (!flag)
        {
          NSMenuItem *anItem;

          anItem = [[NSMenuItem alloc] initWithTitle: [self title]
                                       action: NULL
                                       keyEquivalent: nil];
          [self setMenuItem: anItem];
          RELEASE(anItem);
        }
    }
}

/**
 * Returns YES, if the reciever uses a menu item for its title.
 */
- (BOOL) usesItemFromMenu
{
  return _pbcFlags.usesItemFromMenu;
}

/**
 * Return YES, if the reciever changes the state of the item chosen by
 * the user.
 */
- (void) setAltersStateOfSelectedItem: (BOOL)flag
{
  id <NSMenuItem> selectedItem = [self selectedItem];

  if (flag)
    {
      [selectedItem setState: NSOnState];
    }
  else
    {
      [selectedItem setState: NSOffState];
    }

  _pbcFlags.altersStateOfSelectedItem = flag;
}


/**
 * Return YES, if the reciever changes the state of the item chosen by
 * the user.
 */
- (BOOL) altersStateOfSelectedItem
{
  return _pbcFlags.altersStateOfSelectedItem;
}

/**
 * Returns the current arrow position of the reciever.
 */
- (NSPopUpArrowPosition) arrowPosition
{
  return _pbcFlags.arrowPosition;
}

/**
 * Sets the current arrow position of the reciever.
 */
- (void) setArrowPosition: (NSPopUpArrowPosition)pos
{
  _pbcFlags.arrowPosition = pos;
}

// Item management
/**
 * Add an item to the popup with title.
 */
- (void) addItemWithTitle: (NSString *)title
{
  [self insertItemWithTitle: title
        atIndex: [_menu numberOfItems]];
}

/**
 * Add a number of items to the reciever using the provided itemTitles array.
 */
- (void) addItemsWithTitles: (NSArray *)titles
{
  unsigned c = [titles count];
  unsigned i;

  for (i = 0; i < c; i++)
    {
      [self addItemWithTitle: [titles objectAtIndex: i]];
    }
}

/**
 * Adds an item with the given title at index.  If an item already exists at
 * index, it, and all items after it are advanced one position.  Index needs
 * to be within the valid range for the array of items in the popup button.
 */
- (void) insertItemWithTitle: (NSString *)title atIndex: (NSInteger)index
{
  id <NSMenuItem> anItem;
  NSInteger i, count;
  
  i = [self indexOfItemWithTitle: title];

  if (-1 != i)
    {
      [self removeItemAtIndex: i];
    }

  count = [_menu numberOfItems];

  if (index < 0)
    index = 0;
  if (index > count)
    index = count;

  anItem = [_menu insertItemWithTitle: title
                  action: NULL
                  keyEquivalent: @""
                  atIndex: index];
  /* Disable showing the On/Off/Mixed state.  We change the state of
     menu items when selected, according to the doc, but we don't want
     it to appear on the screen.  */
  [anItem setOnStateImage: nil];
  [anItem setMixedStateImage: nil];

  // FIXME: The documentation is unclear what to set here.
  //[anItem setAction: [self action]];
  //[anItem setTarget: [self target]];
  // Or
  [anItem setAction: @selector(_popUpItemAction:)];
  [anItem setTarget: self];
  
  // Select the new item if there isn't any selection.
  if (_selectedItem == nil)
    {
      [self selectItem: anItem];
    }
}

/**
 * Remove a given item based on its title.
 */    
- (void) removeItemWithTitle: (NSString *)title
{
  [self removeItemAtIndex: [self indexOfItemWithTitle: title]];
}

/**
 * Remove a given item based on its index, must be a valid index within the
 * range for the item array of this popup.
 */
- (void) removeItemAtIndex: (NSInteger)index
{
  if (index == [self indexOfSelectedItem])
    {
      [self selectItem: nil];
    }

  [_menu removeItemAtIndex: index];
}

/**
 * Purges all items from the popup.
 */
- (void) removeAllItems
{
  [self selectItem: nil];

  while ([_menu numberOfItems] > 0)
    {
      [_menu removeItemAtIndex: 0];
    }
}

// Referencing items...
/**
 * Item array of the reciever.
 */
- (NSArray *) itemArray
{
  return [_menu itemArray];
}

/**
 * Number of items in the reciever.
 */
- (NSInteger) numberOfItems
{
  return [_menu numberOfItems];
}

/**
 * Return the index of item in the item array of the reciever.
 */
- (NSInteger) indexOfItem: (id <NSMenuItem>)item
{
  return [_menu indexOfItem: item];
}

/**
 * Return index of the item with the given title.
 */
- (NSInteger) indexOfItemWithTitle: (NSString *)title
{
  return [_menu indexOfItemWithTitle: title];
}

/**
 * Return index of the item with a tag equal to aTag.
 */
- (NSInteger) indexOfItemWithTag: (NSInteger)tag
{
  return [_menu indexOfItemWithTag: tag];
}

/**
 * Index of the item whose menu item's representedObject is equal to obj.
 */
- (NSInteger) indexOfItemWithRepresentedObject: (id)obj
{
  return [_menu indexOfItemWithRepresentedObject: obj];
}

/**
 * Index of the item in the reciever whose target and action
 * are equal to aTarget and actionSelector.
 */
- (NSInteger) indexOfItemWithTarget: (id)aTarget andAction: (SEL)actionSelector
{
  return [_menu indexOfItemWithTarget: aTarget andAction: actionSelector];
}

/**
 * Return the item at index.
 */ 
- (id <NSMenuItem>) itemAtIndex: (NSInteger)index
{
  if ((index >= 0) && (index < [_menu numberOfItems]))
    {
      return [_menu itemAtIndex: index];
    }
  else 
    {
      return nil;
    }
}

/**
 * Return the item with title.
 */
- (id <NSMenuItem>) itemWithTitle: (NSString *)title
{
  return [_menu itemWithTitle: title];
}

/**
 * Return the item listed last in the reciever.
 */
- (id <NSMenuItem>) lastItem
{
  NSInteger end = [_menu numberOfItems] - 1;

  if (end < 0)
    return nil;
  return [_menu itemAtIndex: end];
}

/*
  The Cocoa specification has this method as: 
  - (id <NSCopying>) objectValue

  But this gives a conflict with the NSCell declaration of this method, 
  which is: 
  - (id) objectValue
 */
- (id <NSCopying>) objectValue
{
  return [NSNumber numberWithInt: [self indexOfSelectedItem]];
}

- (void) setObjectValue: (id)object
{
  if ([object respondsToSelector: @selector(intValue)])
    {
      int i = [object intValue];
        
      [self selectItemAtIndex: i];
      [self synchronizeTitleAndSelectedItem];
    }
}

- (NSImage *) _currentArrowImage
{
  if (_pbcFlags.pullsDown)
    {
      if (_pbcFlags.arrowPosition == NSPopUpNoArrow)
        {
          return nil;
        }

      if (_pbcFlags.preferredEdge == NSMaxYEdge)
        {
          return _pbc_image[1];
        }
      else if (_pbcFlags.preferredEdge == NSMaxXEdge)
        {
          return _pbc_image[2];
        }
      else if (_pbcFlags.preferredEdge == NSMinYEdge)
        {
          return _pbc_image[3];
        }
      else if (_pbcFlags.preferredEdge == NSMinXEdge)
        {
          return _pbc_image[4];
        }
      else
        {
          return _pbc_image[1];
        }
    }
  else
    {
      return _pbc_image[0];
    }
}

- (void) setImage: (NSImage *)anImage
{
  // Do nothing as the image is determined by the current item
}

- (void) setMenuItem: (NSMenuItem *)item
{
  NSImage *image;

  if (_menuItem == item)
    return;

  image = [self _currentArrowImage];

  if ([_menuItem image] == image)
    {
      [_menuItem setImage: nil];
    }

  //[super setMenuItem: item];
  ASSIGN(_menuItem, item);

  if ([_menuItem image] == nil)
    {
      [_menuItem setImage: image];
    }
}

- (void) selectItem: (id <NSMenuItem>)item
{
  id<NSMenuItem> oldSelectedItem = _selectedItem;

  if (_selectedItem == item)
    {
      // pull-down should set highlighted item even when selection is unchanged
      if (_pbcFlags.pullsDown)
        [[_menu menuRepresentation] setHighlightedItemIndex: 
                   [_menu indexOfItem: _selectedItem]];
      return;
    }

  if (_selectedItem != nil)
    {
      if (_pbcFlags.altersStateOfSelectedItem)
        {
          [_selectedItem setState: NSOffState];
        }
    }

  _selectedItem = item;

  /* Set the item in the menu */
  /* Note: Must do this before changing the state of the selected item, since
   * the change will recursively invoke synchronizeTitleAndSelectedItem, which
   * otherwise would select the old item again */
  [[_menu menuRepresentation] setHighlightedItemIndex: 
                   [_menu indexOfItem: _selectedItem]];

  if (_selectedItem != nil)
    {
      if (_pbcFlags.altersStateOfSelectedItem)
        {
          [_selectedItem setState: NSOnState];
        }
    }

  if (oldSelectedItem)
    {
      [[_menu menuRepresentation] setNeedsDisplayForItemAtIndex:
                       [_menu indexOfItem: oldSelectedItem]];
    }
}

- (void) selectItemAtIndex: (NSInteger)index
{
  id <NSMenuItem> anItem;

  if (index < 0) 
    anItem = nil;
  else
    anItem = [self itemAtIndex: index];

  [self selectItem: anItem];
}

- (void) selectItemWithTitle: (NSString *)title
{
  id <NSMenuItem> anItem = [self itemWithTitle: title];

  [self selectItem: anItem];
}

- (NSString *)title
{
  id <NSMenuItem> selectedItem;

  if (!_pbcFlags.usesItemFromMenu)
    {
      selectedItem = _menuItem;
    }
  else if (_pbcFlags.pullsDown)
    {
      if ([_menu numberOfItems] == 0)
	{
	  selectedItem = _menuItem;
	}
      else
	{
	  selectedItem = [_menu itemAtIndex: 0];
	}
    }
  else
    {
      selectedItem = [self selectedItem];
    }
  return [selectedItem title]; 
}

- (void) setTitle: (NSString *)aString
{
  id <NSMenuItem> anItem;

  if (!_pbcFlags.usesItemFromMenu)
    {
      [_menuItem setTitle: aString];

      return; 
    }
  else if (_pbcFlags.pullsDown)
    {
      if ([_menu numberOfItems] == 0)
        {
          anItem = nil;
        }
      else
        {
          anItem = [_menu itemAtIndex: 0];
          [anItem setTitle: aString];
        }
    }
  else
    {
      anItem = [_menu itemWithTitle: aString];
      if (anItem == nil)
        {
          [self addItemWithTitle: aString];
          anItem = [_menu itemWithTitle: aString];
        }
    }
  [self selectItem: anItem];
}

- (NSString *)stringValue
{
  return [self titleOfSelectedItem];
}

- (id <NSMenuItem>) selectedItem
{
  return _selectedItem;
}

- (id) representedObject
{
  return [[self selectedItem] representedObject];
}

- (void) setRepresentedObject: (id)object
{
  [[self selectedItem] setRepresentedObject: object];
}

- (NSInteger) indexOfSelectedItem
{
  return [_menu indexOfItem: [self selectedItem]];
}

- (void) synchronizeTitleAndSelectedItem
{
  NSInteger index;

  if (!_pbcFlags.usesItemFromMenu)
    return;

  if ([_menu numberOfItems] == 0)
    {
      index = -1;
    }
  else if (_pbcFlags.pullsDown)
    {
      index = 0;
    }
  else
    {
      index = [[_menu menuRepresentation] highlightedItemIndex];

      if (index < 0) 
        {
          // If no item is highighted, display the selected one, if there is one.
          index = [self indexOfSelectedItem];
        }
      else 
        {
          // Selected the highlighted item
          [self selectItemAtIndex: index];
        }
    }

  if ((index >= 0)  && ([_menu numberOfItems] > index))
    {
      NSMenuItem *anItem;

      // This conversion is needed as [setMenuItem:] expects an NSMenuItem
      anItem = (NSMenuItem *)[_menu itemAtIndex: index];
      [self setMenuItem: anItem];
    }
  else
    {
      [self setMenuItem: nil];
    }

  if (_control_view)
    if ([_control_view isKindOfClass: [NSControl class]])
      [(NSControl *)_control_view updateCell: self];
}


// Title management
/**
 * Set item title at the given index in the reciever.
 */
- (NSString *) itemTitleAtIndex: (NSInteger)index
{
  return [[self itemAtIndex: index] title];
}

/**
 * Returns an array containing all of the current item titles.
 */ 
- (NSArray *) itemTitles
{
  unsigned count = [_menu numberOfItems];
  id items[count];
  unsigned i;

  [[_menu itemArray] getObjects: items];
  for (i = 0; i < count; i++)
    {
      items[i] = [items[i] title];
    }

  return [NSArray arrayWithObjects: items count: count];
}

/**
 * Returns the title of the currently selected item in the reciever.
 */
- (NSString *) titleOfSelectedItem
{
  id <NSMenuItem> item = [self selectedItem];

  if (item != nil)
    return [item title];
  else
    return @"";
}

/**
 * Attach popup
 */
- (void) attachPopUpWithFrame: (NSRect)cellFrame
                       inView: (NSView *)controlView
{
  NSRectEdge            preferredEdge = _pbcFlags.preferredEdge;
  NSNotificationCenter  *nc = [NSNotificationCenter defaultCenter];
  NSWindow              *cvWin = [controlView window];
  NSMenuView            *mr = [_menu menuRepresentation];
  NSInteger                   selectedItem;

  [nc postNotificationName: NSPopUpButtonCellWillPopUpNotification
                    object: self];

  [nc postNotificationName: NSPopUpButtonWillPopUpNotification
                    object: controlView];

  // Convert to Screen Coordinates
  cellFrame = [controlView convertRect: cellFrame toView: nil];
  cellFrame.origin = [cvWin convertBaseToScreen: cellFrame.origin];

  if (_pbcFlags.pullsDown)
    selectedItem = -1;
  else
    {
      selectedItem = [self indexOfSelectedItem];
      if (selectedItem == -1)
	{
	  selectedItem = 0;
	}
    }

  if (selectedItem > 0)
    {
      [mr setHighlightedItemIndex: selectedItem];
    }

  if ([controlView isFlipped])
    {
      if (preferredEdge == NSMinYEdge)
	{
	  preferredEdge = NSMaxYEdge;
	}
      else if (preferredEdge == NSMaxYEdge)
	{
	  preferredEdge = NSMinYEdge;
	}
    }

  // display the menu item...
  [[GSTheme theme] displayPopUpMenu: mr
		      withCellFrame: cellFrame
		  controlViewWindow: cvWin
		      preferredEdge: preferredEdge
		       selectedItem: selectedItem];

  [nc addObserver: self
      selector: @selector(_handleNotification:)
      name: NSMenuDidSendActionNotification
      object: _menu];
}

/**
 * Dismiss the reciever.
 */ 
- (void) dismissPopUp
{
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
  NSMenuView *mr = [_menu menuRepresentation];
  NSWindow *mWin;

  [nc removeObserver: self
      name: NSMenuDidSendActionNotification
      object: _menu];
  [_menu close];

  // remove from main window
  mWin = [mr window];
  [[mWin parentWindow] removeChildWindow: mWin];
}

/* Private method handles all cases after doing a selection from 
   the popup menu. Especially the obscure case where the user uses the 
   keyboard to open a popup, but subsequently uses the mouse to select
   an item. We'll never know this was done (and thus cannot dismiss
   the popUp) without getting this notification */
- (void) _handleNotification: (NSNotification*)aNotification
{
  NSString *name = [aNotification name];

  if ([name isEqual: NSMenuDidSendActionNotification] == YES)
    {
      [self dismissPopUp];
      [self synchronizeTitleAndSelectedItem];
    }
}

- (BOOL) trackMouse: (NSEvent *)theEvent
             inRect: (NSRect)cellFrame
             ofView: (NSView *)controlView
       untilMouseUp: (BOOL)untilMouseUp
{
  NSMenuView *mr = [[self menu] menuRepresentation];
  NSWindow   *menuWindow = [mr window];
  NSEvent    *e;
  NSPoint    p;

  if ([self isEnabled] == NO)
    return NO;

  if ([[self menu] numberOfItems] == 0)
    {
      NSBeep ();
      return NO;
    }

  // Attach the popUp
  [self attachPopUpWithFrame: cellFrame
                      inView: controlView];

  p = [[controlView window] convertBaseToScreen: [theEvent locationInWindow]];
  p = [menuWindow convertScreenToBase: p];
  
  // Process events; we start menu events processing by converting 
  // this event to the menu window, and sending it there. 
  e = [NSEvent mouseEventWithType: [theEvent type]
               location: p
               modifierFlags: [theEvent modifierFlags]
               timestamp: [theEvent timestamp]
               windowNumber: [menuWindow windowNumber]
               context: [theEvent context]
               eventNumber: [theEvent eventNumber]
               clickCount: [theEvent clickCount] 
               pressure: [theEvent pressure]];

  // Send the event directly to the popup window, as it may not be located
  // at the event position.
  [mr mouseDown: e];
  
  // End of mouse tracking here -- dismiss popup
  // No synchronization needed here
  if ([[_menu window] isVisible])
    {
      [self dismissPopUp];
      return NO;
    }

  return YES;
}

/**
 * Perform the click operation with the given frame and controlView.
 */
- (void) performClickWithFrame: (NSRect)frame inView: (NSView *)controlView
{  
  [super performClickWithFrame: frame inView: controlView];
  
  [self attachPopUpWithFrame: frame inView: controlView];
}

/*
 * Override the implementation in NSMenuItemCell to behave the same
 * as superclass NSButtonCell's implementation, since our direct
 * superclass NSMenuItemCell has special menu-specific drawing.
 */
- (void) drawBorderAndBackgroundWithFrame: (NSRect)cellFrame
                                   inView: (NSView *)controlView
{
  [[GSTheme theme] drawButton: cellFrame
			   in: self
			 view: controlView
			style: _bezel_style
			state: [self themeControlState]];
}


/*
 * This drawing uses the same code that is used to draw cells in the menu.
 */
- (void) drawInteriorWithFrame: (NSRect)cellFrame
                        inView: (NSView*)controlView
{
  BOOL new = NO;

  if ([self menuItem] == nil)
    {
      NSMenuItem *anItem;

      /* 
       * Create a temporary NSMenuItemCell to at least draw our control,
       * if items array is empty.
       */
      anItem = [NSMenuItem new];
      [anItem setTitle: [self title]];
      /* We need this menu item because NSMenuItemCell gets its contents 
       * from the menuItem not from what is set in the cell */
      [self setMenuItem: anItem];
      RELEASE(anItem);
      new = YES;
    }      

  /* We need to calc our size to get images placed correctly */
  [self calcSize];
  [[GSTheme theme] drawPopUpButtonCellInteriorWithFrame: cellFrame
		   withCell: self
		   inView: controlView];
  [super drawInteriorWithFrame: cellFrame inView: controlView];

  /* Unset the item to restore balance if a new was created */
  if (new)
    {
      [self setMenuItem: nil];
    }
}

/* FIXME: this method needs to be rewritten to be something like 
 * NSMenuView's sizeToFit. That way if you call [NSPopUpButton sizeToFit]; 
 * you will get the absolutely correct cellSize.
 * Not sure if this is true. Maybe the popup should only use the size 
 * of the current title, at least, when usesItemFromMenu is false.
 */
- (NSSize) cellSize
{
  NSSize s;
  NSSize imageSize;
  NSSize titleSize;
  NSInteger i, count;
  NSString *title;
  NSImage *image;

  count = [_menu numberOfItems];

  image = [self _currentArrowImage];
  if (image)
    imageSize = [image size];
  else
    imageSize = NSZeroSize;

  s = NSMakeSize(0, imageSize.height);
  
  if (count == 0)
    {
      title = [self title];
      titleSize = [self _sizeText: title];

      if (titleSize.width > s.width)
        s.width = titleSize.width;
      if (titleSize.height > s.height)
        s.height = titleSize.height;
    }

  for (i = 0; i < count; i++)
    {
      title = [[_menu itemAtIndex: i] title];
      titleSize = [self _sizeText: title];

      if (titleSize.width > s.width)
        s.width = titleSize.width;
      if (titleSize.height > s.height)
        s.height = titleSize.height;
    }

  s.width += imageSize.width; 
  s.width += 5; /* Left border to text (border included) */
  s.width += 3; /* Text to Image */
  s.width += 4; /* Right border to image (border included) */

  /* (vertical) border: */
  s.height += 2 * [[GSTheme theme] sizeForBorderType: NSBezelBorder].height;

  /* Spacing between border and inside: */
  s.height += 2 * 1;
  s.width  += 2 * 3;
  
  return s;
}

- (void) setAction: (SEL)aSelector
{
  [super setAction: aSelector];
  [_menu update];
}

- (void) setTarget: (id)anObject
{
  [super setTarget: anObject];
  [_menu update];
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeBool: [self altersStateOfSelectedItem] 
	      forKey: @"NSAltersState"];
      [aCoder encodeBool: [self usesItemFromMenu] 
	      forKey: @"NSUsesItemFromMenu"];
      [aCoder encodeInt:  [self arrowPosition] 
	      forKey: @"NSArrowPosition"];
      [aCoder encodeInt:  [self preferredEdge] 
	      forKey: @"NSPreferredEdge"];
      [aCoder encodeInt:  [self indexOfSelectedItem] 
	      forKey: @"NSSelectedIndex"];
      [aCoder encodeBool: [self pullsDown]
	      forKey: @"NSPullDown"];

      // encode the menu, if present.
      if (_menu != nil)
        {
          [aCoder encodeObject: _menu 
		  forKey: @"NSMenu"];
        }
    }
  else
    {    
      NSInteger flag;

      [aCoder encodeObject: _menu];
      [aCoder encodeConditionalObject: [self selectedItem]];
      flag = _pbcFlags.pullsDown;
      encode_NSInteger(aCoder, &flag);
      flag = _pbcFlags.preferredEdge;
      encode_NSInteger(aCoder, &flag);
      flag = _pbcFlags.usesItemFromMenu;
      encode_NSInteger(aCoder, &flag);
      flag = _pbcFlags.altersStateOfSelectedItem;
      encode_NSInteger(aCoder, &flag);
      flag = _pbcFlags.arrowPosition;
      encode_NSInteger(aCoder, &flag);
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  NSMenu *menu;

  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      /* First decode menu, menu items must be available to set the selection */
      menu = [aDecoder decodeObjectForKey: @"NSMenu"];
      if (menu)
        {
          NSEnumerator *menuItemEnumerator;
          NSMenuItem *menuItem;

          [self setMenu: nil];
          [self setMenu: menu];

          // FIXME: This special handling is needed bacause the NSClassSwapper
          // might have replaced the cell, but the items still refere to it.
          menuItemEnumerator = [[menu itemArray] objectEnumerator];

          while ((menuItem = [menuItemEnumerator nextObject]) != nil)
            {
              if (sel_isEqual([menuItem action], @selector(_popUpItemAction:))
                  && ([menuItem target] != self))
                {
                  [menuItem setTarget: self];
                }
            }
        }

      if ([aDecoder containsValueForKey: @"NSAltersState"])
        {
          BOOL alters = [aDecoder decodeBoolForKey: @"NSAltersState"];
          
          [self setAltersStateOfSelectedItem: alters];
        }
      if ([aDecoder containsValueForKey: @"NSPullDown"])
        {
	  BOOL pullDown = [aDecoder decodeBoolForKey: @"NSPullDown"];
	  [self setPullsDown: pullDown];
	}
      if ([aDecoder containsValueForKey: @"NSUsesItemFromMenu"])
        {
          BOOL usesItem = [aDecoder decodeBoolForKey: @"NSUsesItemFromMenu"];
          
          [self setUsesItemFromMenu: usesItem];
        }
      if ([aDecoder containsValueForKey: @"NSArrowPosition"])
        {
          NSPopUpArrowPosition position = [aDecoder decodeIntForKey: 
						      @"NSArrowPosition"];
          
          [self setArrowPosition: position];
        }
      if ([aDecoder containsValueForKey: @"NSPreferredEdge"])
        {
          NSRectEdge edge = [aDecoder decodeIntForKey: @"NSPreferredEdge"];
          
          [self setPreferredEdge: edge];
        }
      if ([aDecoder containsValueForKey: @"NSSelectedIndex"])
        {
	  int selectedIdx = [aDecoder decodeIntForKey: 
					@"NSSelectedIndex"];
	  [self selectItemAtIndex: selectedIdx];
	}
      else
        {
	  [self selectItemAtIndex: 0];
        }
      if ([aDecoder containsValueForKey: @"NSMenuItem"])
        {
          NSMenuItem *item = [aDecoder decodeObjectForKey: @"NSMenuItem"];
          [self setMenuItem: item];
        }
    }
  else
    {
      NSInteger flag;
      id<NSMenuItem> selectedItem;
      int version = [aDecoder versionForClassName: 
                                  @"NSPopUpButtonCell"];

      menu = [aDecoder decodeObject];
      /* 
         FIXME: This same ivar already gets set in NSCell initWithCoder, 
         but there it is used directly not via a method call. So here we first 
         unset it and than set it again as our setMenu: method tries to optimize 
         duplicate calls.
      */
      [self setMenu: nil];
      [self setMenu: menu];
      selectedItem = [aDecoder decodeObject];
      decode_NSInteger(aDecoder, &flag);
      _pbcFlags.pullsDown = flag;
      decode_NSInteger(aDecoder, &flag);
      _pbcFlags.preferredEdge = flag;
      decode_NSInteger(aDecoder, &flag);
      _pbcFlags.usesItemFromMenu = flag;
      decode_NSInteger(aDecoder, &flag);
      _pbcFlags.altersStateOfSelectedItem = flag;
      decode_NSInteger(aDecoder, &flag);
      _pbcFlags.arrowPosition = flag;
      
      if (version < 2)
        {
          int i;
          
          // Not the stored format did change but the interpretation of it.
          // in version 1 most of the ivars were not used, so their values may
          // be arbitray. We overwrite them with valid settings.
          [self setPullsDown: _pbcFlags.pullsDown];
          _pbcFlags.usesItemFromMenu = YES;
          
          for (i = 0; i < [_menu numberOfItems]; i++)
            {
              id <NSMenuItem> anItem = [menu itemAtIndex: i];
              
              [anItem setOnStateImage: nil];
              [anItem setMixedStateImage: nil];
            }
          [self setEnabled: YES];
        }
      if (version < 3)
        {
          [self setPreferredEdge: NSMaxYEdge];
          [self setArrowPosition: NSPopUpArrowAtCenter];
        }
      [self selectItem: selectedItem];
    }

  return self;
}

@end

@implementation NSPopUpButtonCell (CocoaExtensions)

/*
 * The selector for this method gets used by menu items in Apple NIB files.
 */
- (void) _popUpItemAction: (id)sender
{
  // first, if sender is one of our items, set it as our selected item
  NSUInteger index = [_menu indexOfItem: sender];
  if (index != NSNotFound)
    [self selectItemAtIndex: index];

  if (_control_view)
    {
      NSString *bindings[] = {NSSelectedIndexBinding, NSSelectedTagBinding,
                              NSSelectedObjectBinding, NSSelectedValueBinding};
      int i;

      for (i = 0; i < 4; i++)
        {
          NSString *binding = bindings[i];
          GSKeyValueBinding *theBinding;

          theBinding = [GSKeyValueBinding getBinding: binding 
                                           forObject: _control_view];
          if (theBinding != nil)
            [theBinding reverseSetValueFor: binding];
        }
    }

  [NSApp sendAction: [self action] to: [self target] from: _control_view];
}

@end
