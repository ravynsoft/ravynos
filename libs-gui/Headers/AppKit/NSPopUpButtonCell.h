/* 
   NSPopUpButtonCell.h

   Cell for Popup list class

   Copyright (C) 1999 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSPopUpButtonCell
#define _GNUstep_H_NSPopUpButtonCell

#import <AppKit/AppKitDefines.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSMenuItemCell.h>

@class NSString;
@class NSMenu;
@class NSView;

APPKIT_EXPORT NSString *NSPopUpButtonCellWillPopUpNotification;

typedef enum {
  NSPopUpNoArrow = 0,
  NSPopUpArrowAtCenter = 1,
  NSPopUpArrowAtBottom = 2
} NSPopUpArrowPosition;

@interface NSPopUpButtonCell : NSMenuItemCell
{
  id <NSMenuItem> _selectedItem;
  struct __pbcFlags {
      unsigned int pullsDown: 1;
      unsigned int preferredEdge: 3;
      unsigned int usesItemFromMenu: 1;
      unsigned int altersStateOfSelectedItem: 1;
      unsigned int arrowPosition: 2;
  } _pbcFlags;
}

// Initialization
/**
 * Initialize with stringValue and pullDown.  If pullDown is YES, the
 * reciever will be a pulldown button.
 */
- (id) initTextCell: (NSString*)stringValue pullsDown: (BOOL)flag;

// Selection processing
/**
 * The currently selected item in the reciever.
 */
- (id <NSMenuItem>) selectedItem;

/** 
 * Index of the currently selected item in the reciever.
 */
- (NSInteger) indexOfSelectedItem;

/**
 * Synchronizes the title and the selected item.  This sets
 * the selected item to the title of the reciever.
 */
- (void) synchronizeTitleAndSelectedItem;

/**
 * Select item in the reciever.
 */
- (void) selectItem: (id <NSMenuItem>)item;

/**
 * Select item at the given index.
 */
- (void) selectItemAtIndex: (NSInteger)index;

/**
 * Select the item with the given title.
 */
- (void) selectItemWithTitle: (NSString*)title;

/**
 * Set title to aString.
 */
- (void) setTitle: (NSString*)aString;

// Getters and setters.
/**
 * Set the menu for the popup.
 */ 
- (void) setMenu: (NSMenu*)menu;

/**
 * Return the menu for the popup.
 */
- (NSMenu*) menu;

/**
 * Set to YES to make the popup button a pull-down style control.
 */
- (void) setPullsDown: (BOOL)flag;

/**
 * Returns YES, if this is a pull-down 
 */
- (BOOL) pullsDown;

/**
 * Set to YES, if the items are to be autoenabled.
 */
- (void) setAutoenablesItems: (BOOL)flag;

/**
 * Returns YES, if the items are autoenabled.
 */
- (BOOL) autoenablesItems;

/**
 * Set the preferred edge as described by edge.  This is used
 * to determine the edge which will open the popup when the screen
 * is small.
 */
- (void) setPreferredEdge: (NSRectEdge)preferredEdge;

/**
 * Return the preferred edge.
 */
- (NSRectEdge) preferredEdge;

/**
 * Set to YES, if the reciever should use a menu item for its title. YES
 * is the default.
 */ 
- (void) setUsesItemFromMenu: (BOOL)flag;

/**
 * Returns YES, if the reciever uses a menu item for its title.
 */
- (BOOL) usesItemFromMenu;

/**
 * Set to YES, if the state of the menu item selected in the reciever
 * should be changed when it's selected.
 */
- (void) setAltersStateOfSelectedItem: (BOOL)flag;

/**
 * Return YES, if the reciever changes the state of the item chosen by
 * the user.
 */
- (BOOL) altersStateOfSelectedItem;

/**
 * Returns the current arrow position of the reciever.
 */ 
- (NSPopUpArrowPosition) arrowPosition;

/**
 * Sets the current arrow position of the reciever.
 */
- (void) setArrowPosition: (NSPopUpArrowPosition)pos;

// Item management
/**
 * Add an item to the popup with title.
 */
- (void) addItemWithTitle: (NSString*)title;

/**
 * Add a number of items to the reciever using the provided itemTitles array.
 */
- (void) addItemsWithTitles: (NSArray*)titles;

/**
 * Adds an item with the given title at index.  If an item already exists at
 * index, it, and all items after it are advanced one position.  Index needs
 * to be within the valid range for the array of items in the popup button.
 */
- (void) insertItemWithTitle: (NSString*)title atIndex: (NSInteger)index;

/**
 * Remove a given item based on its title.
 */        
- (void) removeItemWithTitle: (NSString*)title;

/**
 * Remove a given item based on its index, must be a valid index within the
 * range for the item array of this popup.
 */
- (void) removeItemAtIndex: (NSInteger)index; 

/**
 * Purges all items from the popup.
 */
- (void) removeAllItems;
    
// Referencing items...
/**
 * Item array of the reciever.
 */ 
- (NSArray*) itemArray;

/**
 * Number of items in the reciever.
 */
- (NSInteger) numberOfItems;
 
/**
 * Return the index of item in the item array of the reciever.
 */
- (NSInteger) indexOfItem: (id<NSMenuItem>)item;

/**
 * Return index of the item with the given title.
 */
- (NSInteger) indexOfItemWithTitle: (NSString*)title;

/**
 * Return index of the item with a tag equal to aTag.
 */
- (NSInteger) indexOfItemWithTag: (NSInteger)tag;

/**
 * Index of the item whose menu item's representedObject is equal to obj.
 */
- (NSInteger) indexOfItemWithRepresentedObject: (id)obj;

/**
 * Index of the item in the reciever whose target and action
 * are equal to aTarget and actionSelector.
 */
- (NSInteger) indexOfItemWithTarget: (id)aTarget andAction: (SEL)actionSelector;

/**
 * Return the item at index.
 */ 
- (id <NSMenuItem>) itemAtIndex: (NSInteger)index;

/**
 * Return the item with title.
 */
- (id <NSMenuItem>) itemWithTitle: (NSString*)title;

/**
 * Return the item listed last in the reciever.
 */
- (id <NSMenuItem>) lastItem;


// Title management
/**
 * Set item title at the given index in the reciever.
 */ 
- (NSString*) itemTitleAtIndex: (NSInteger)index;

/**
 * Returns an array containing all of the current item titles.
 */ 
- (NSArray*) itemTitles;

/**
 * Returns the title of the currently selected item in the reciever.
 */
- (NSString*) titleOfSelectedItem;

/**
 * Attach popup
 */
- (void) attachPopUpWithFrame: (NSRect)cellFrame inView: (NSView*)controlView;

/**
 * Dismiss the reciever.
 */ 
- (void) dismissPopUp;

/**
 * Perform the click operation with the given frame and controlView.
 */
- (void) performClickWithFrame: (NSRect)frame inView: (NSView*)controlView;
@end    

#endif // _GNUstep_H_NSPopUpButtonCell
