/** <title>NSPopUpButton</title>

   <abstract>Popup list class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Michael Hanni <mhanni@sprintmail.com>
   Date: June 1999
   
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

#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSPopUpButtonCell.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSWindow.h"

/*
 * class variables
 */
Class _nspopupbuttonCellClass = 0;

/*
 * NSPopUpButton implementation
 */

@implementation NSPopUpButton

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSPopUpButton class])
    {
      // Initial version
      [self setVersion: 1];
      [self setCellClass: [NSPopUpButtonCell class]];

      [self exposeBinding: NSSelectedIndexBinding];
      [self exposeBinding: NSSelectedObjectBinding];
      [self exposeBinding: NSSelectedTagBinding];
      [self exposeBinding: NSSelectedValueBinding];
      [self exposeBinding: NSContentValuesBinding];
   } 
}

+ (Class) cellClass
{
  return _nspopupbuttonCellClass;
}

+ (void) setCellClass: (Class)classId
{
  _nspopupbuttonCellClass = classId;
}

/*
 * Initializing an NSPopUpButton 
 */
- (id) init
{
  return [self initWithFrame: NSZeroRect pullsDown: NO];
}

- (id) initWithFrame: (NSRect)frameRect
{
  return [self initWithFrame: frameRect pullsDown: NO];
}

/** <p>Initialize and returns a new NSPopUpButton into the frame frameRect
    and specified by flag if the NSPopUpButton is a pull-down list</p>
    <p>See Also: -setPullsDown: [NSView-initWithFrame:]</p>
 */
- (id) initWithFrame: (NSRect)frameRect
	   pullsDown: (BOOL)flag
{
  if ( ! ( self = [super initWithFrame: frameRect] ) )
    return nil;
  
  [self setPullsDown: flag];

  return self;
}


/*
In NSView, -menuForEvent: returns [self menu] as the context menu of the
view. Since our -menu returns the menu for our pop-up, we need to override
this to return nil to indicate that we have no context menu.
*/
- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
  return nil;
}


- (void) setMenu: (NSMenu*)menu
{
  [_cell setMenu: menu];
}

- (NSMenu*) menu
{
  return [_cell menu];
}

/**<p>Sets whether the NSPopUpButton's cell has a pulls-down list ( YES ) 
   or a pop-up list (NO)  </p> <p>See Also: -pullsDown 
   [NSPopUpButtonCell-setPullsDown:]</p>
*/

- (void) setPullsDown: (BOOL)flag
{
  [_cell setPullsDown: flag];
}
/** <p>Returns whether the NSPopUpButton's cell has a pulls-down list ( YES ) 
    or a pop-up list (NO) </p> 
    <p>See Also: -setPullsDown: [NSPopUpButtonCell-pullsDown]</p>
 */
- (BOOL) pullsDown
{
  return [_cell pullsDown];
}

- (void) setAutoenablesItems: (BOOL)flag
{
  [_cell setAutoenablesItems: flag];
}

- (BOOL) autoenablesItems
{
  return [_cell autoenablesItems];
}

/** <p>Inserts a new item with title as its title at the end of the list and
    synchronizes the NSPopUpButton's title with the title of the selected item.
    </p><p>See Also: [NSPopUpButtonCell-addItemWithTitle:]
    -synchronizeTitleAndSelectedItem</p>
 */
- (void) addItemWithTitle: (NSString *)title
{
  [_cell addItemWithTitle: title];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Inserts a new list of items with titles as titles at the end of the list
    and synchronizes the NSPopUpButton's title with the title of the selected
    item.</p><p>See Also: [NSPopUpButtonCell-addItemsWithTitles:] 
    -synchronizeTitleAndSelectedItem</p>
 */
- (void) addItemsWithTitles: (NSArray*)itemTitles
{
  [_cell addItemsWithTitles: itemTitles];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Inserts a new item with title as its title at the specified index 
    and synchronizes the NSPopUpButton's title with the title of the selected
    item.</p><p>See Also: [NSPopUpButtonCell-insertItemWithTitle:atIndex:] 
    -synchronizeTitleAndSelectedItem</p>
 */
- (void) insertItemWithTitle: (NSString*)title
		     atIndex: (NSInteger)index
{
  [_cell insertItemWithTitle: title 
		     atIndex: index];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Removes all items from the item list and synchronizes the 
    NSPopUpButton's title with the title of the selected</p>
    <p>See Also: [NSPopUpButtonCell-removeAllItems] -removeItemWithTitle:
    -synchronizeTitleAndSelectedItem</p>
*/
- (void) removeAllItems
{
  [_cell removeAllItems];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Removes the item specified with title as its title from the item list
    and synchronizes the NSPopUpButton's title with the title of the selected
    </p><p>See Also: [NSPopUpButtonCell-removeItemWithTitle:] 
    -removeAllItems -removeItemAtIndex: -synchronizeTitleAndSelectedItem</p>
*/
- (void) removeItemWithTitle: (NSString*)title
{
  [_cell removeItemWithTitle: title];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Removes the item at the specified index index from the item list
    and synchronizes the NSPopUpButton's title with the title of the selected
    </p><p>See Also: [NSPopUpButtonCell-removeItemAtIndex:] 
    -removeAllItems -removeItemWithTitle: -synchronizeTitleAndSelectedItem</p>
*/
- (void) removeItemAtIndex: (NSInteger)index
{
  [_cell removeItemAtIndex: index];

  [self synchronizeTitleAndSelectedItem];
}

/** <p>Returns the selected item</p>
    <p>See Also: [NSPopUpButtonCell-selectedItem]</p>    
 */
- (id <NSMenuItem>) selectedItem
{
  return [_cell selectedItem];
}

/** <p>Returns the title of the selected item</p>
    <p>See Also: [NSPopUpButtonCell-titleOfSelectedItem]</p>    
 */
- (NSString*) titleOfSelectedItem
{
  return [_cell titleOfSelectedItem];
}

/**<p>Returns the index of the selected item</p>
   <p>See Also: [NSPopUpButtonCell-indexOfSelectedItem]</p>
 */
- (NSInteger) indexOfSelectedItem
{
  return [_cell indexOfSelectedItem];
}

/**<p>Returns the tag of the selected item</p>
   <p>See Also: [NSPopUpButtonCell-indexOfSelectedItem]</p>
*/
- (NSInteger) selectedTag
{
  return [[_cell selectedItem] tag];
}

- (void) selectItem: (id <NSMenuItem>)anObject
{
  [_cell selectItem: anObject];
  [self synchronizeTitleAndSelectedItem];
}

/**<p>Select the item at index <var>index</var> and synchronizes the
   NSPopUpButton's title with the title of the selected</p><p>See Also: 
   [NSPopUpButtonCell-selectItemAtIndex:] -synchronizeTitleAndSelectedItem</p>
 */
- (void) selectItemAtIndex: (NSInteger)index
{
  [_cell selectItemAtIndex: index];
  [self synchronizeTitleAndSelectedItem];
}

/**<p>Select the item with title <var>title</var> and synchronizes the
   NSPopUpButton's title with the title of the selected</p><p>See Also: 
   [NSPopUpButtonCell-selectItemWithTitle:]
   -synchronizeTitleAndSelectedItem</p>
 */
- (void) selectItemWithTitle: (NSString*)title
{
  [_cell selectItemWithTitle: title];
  [self synchronizeTitleAndSelectedItem];
}

- (BOOL) selectItemWithTag: (NSInteger)tag
{
   NSInteger index = [self indexOfItemWithTag: tag];

   if (index >= 0)
     {
       [self selectItemAtIndex: index];
       return YES;
     }
   else
     {
       return NO;
     }
}


/** <p>Returns the number of items in the item list</p>
    <p>See Also: [NSPopUpButtonCell-numberOfItems]</p>
 */
- (NSInteger) numberOfItems
{
  return [_cell numberOfItems];
}

- (NSArray*) itemArray 
{
  return [_cell itemArray];
}

/**<p>Returns the NSMenuItem at index index or nil if index is out of
   range</p><p>See Also: [NSPopUpButtonCell-itemAtIndex:] </p>
 */
- (id <NSMenuItem>) itemAtIndex: (NSInteger)index
{
  return [_cell itemAtIndex: index];
}

/** <p>Returns the item's title at index <var>index</var></p>
 */
- (NSString*) itemTitleAtIndex: (NSInteger)index
{
  return [_cell itemTitleAtIndex: index];
}

/**<p>Returns an array containing the items's titles</p>
 */
- (NSArray*) itemTitles
{
  return [_cell itemTitles];
}

/**<p>Returns the NSMenuItem with title as its title</p>
 */
- (id <NSMenuItem>) itemWithTitle: (NSString*)title
{
  return [_cell itemWithTitle: title];
}

/**<p> Returns the last NSMenuItem of the list</p>
 */
- (id <NSMenuItem>) lastItem
{
  return [_cell lastItem];
}

- (NSInteger) indexOfItem: (id <NSMenuItem>)anObject
{
  return [_cell indexOfItem: anObject];
}

/**<p>Returns the index of the item with tag as its tag. Returns -1
   if the cell is not found</p><p>See Also: 
   [NSPopUpButtonCell-indexOfItemWithTag:] -indexOfItemWithTitle:
   -indexOfItemWithRepresentedObject:</p>
*/
- (NSInteger) indexOfItemWithTag: (NSInteger)tag
{
  return [_cell indexOfItemWithTag: tag];
}

/**<p>Returns the index of the item with title as its title. Returns -1
   if the cell is not found</p><p>See Also: 
   [NSPopUpButtonCell-indexOfItemWithTitle:] -indexOfItemWithTag:
   -indexOfItemWithRepresentedObject:</p>
*/
- (NSInteger) indexOfItemWithTitle: (NSString*)title
{
  return [_cell indexOfItemWithTitle: title];
}

- (NSInteger) indexOfItemWithRepresentedObject: (id)anObject
{
  return [_cell indexOfItemWithRepresentedObject: anObject];
}

- (NSInteger) indexOfItemWithTarget: (id)target
		    andAction: (SEL)actionSelector
{
  return [_cell indexOfItemWithTarget: target andAction: actionSelector];
}

- (void) setPreferredEdge: (NSRectEdge)edge
{
  [_cell setPreferredEdge: edge];
}

- (NSRectEdge) preferredEdge
{
  return [_cell preferredEdge];
}

- (void) setTitle: (NSString*)aString
{
  [_cell setTitle: aString];
  [self synchronizeTitleAndSelectedItem];
}

- (void) synchronizeTitleAndSelectedItem
{
  [_cell synchronizeTitleAndSelectedItem];
  [self setNeedsDisplay: YES];
}

- (BOOL) resignFirstResponder
{
  [_cell dismissPopUp];

  return [super resignFirstResponder];
}

- (BOOL) performKeyEquivalent: (NSEvent*)theEvent
{
  NSMenu     *m = [self menu];
  NSMenuItem *oldSelectedItem = (NSMenuItem *)[_cell selectedItem];

  if (m != nil)
    {
      if ([m performKeyEquivalent: theEvent])
	{
	  // pullsDown does not change selected item
	  if ([_cell pullsDown])
	    {
	      [self selectItem: oldSelectedItem];
	    }
	  else
	    {
	      /* If the key equivalent was performed, redisplay ourselves
	       * to account for potential changes in the selected item.
	       */
	      [self setNeedsDisplay: YES];
	    }
	  return YES;
	}
    }
  return NO;
}

- (void) mouseDown: (NSEvent*)theEvent
{ 
  [_cell trackMouse: theEvent 
	     inRect: [self bounds] 
	     ofView: self 
       untilMouseUp: YES];
}

- (void) keyDown: (NSEvent*)theEvent
{
  // FIXME: This method also handles the key events for the popup menu window,
  // as menu windows cannot become key window.
  if ([self isEnabled])
    {
      NSString *characters = [theEvent characters];
      unichar character = 0;

      if ([characters length] > 0)
	{
	  character = [characters characterAtIndex: 0];
	}

      switch (character)
	{
	case NSNewlineCharacter:
	case NSEnterCharacter: 
	case NSCarriageReturnCharacter:
	  /* Handle Enter and Return keys only when the menu is visible.
	     The button's action to pop up the menu is initiated only by
	     the Space key similar to other buttons. */
	  {
	    NSMenuView *menuView = [[_cell menu] menuRepresentation];
	    if ([[menuView window] isVisible] == NO)
	      break;
	  }
	case ' ':
	  {
	    NSInteger selectedIndex;
	    NSMenuView *menuView;

	    // Beep, as on OS, and then return.
	    if ([[_cell menu] numberOfItems] == 0)
	      {
		NSBeep();
		return;
	      }

	    menuView = [[_cell menu] menuRepresentation];
	    if ([[menuView window] isVisible] == NO)
	      {
		// Attach the popUp
		[_cell attachPopUpWithFrame: _bounds
		       inView: self];
	      }
	    else
	      {
		selectedIndex = [menuView highlightedItemIndex];
		if (selectedIndex >= 0)
		  {
		    [[_cell menu] performActionForItemAtIndex: selectedIndex];
		  }
	      }
	  }
	  return;
	case '\e':
	  [_cell dismissPopUp];
	  return;
	case NSUpArrowFunctionKey:
	  {
	    NSMenuView *menuView;
	    NSInteger selectedIndex, numberOfItems;

	    menuView = [[_cell menu] menuRepresentation];
	    selectedIndex = [menuView highlightedItemIndex];
	    numberOfItems = [self numberOfItems];

	    switch (selectedIndex)
	      {
	      case -1:
		selectedIndex = numberOfItems - 1;
		break;
	      case 0:
		return;
	      default:
		selectedIndex--;
		break;
	      }

	    [menuView setHighlightedItemIndex: selectedIndex];
	  }
	  return;
	case NSDownArrowFunctionKey:
	  {
	    NSMenuView *menuView;
	    NSInteger selectedIndex, numberOfItems;

	    menuView = [[_cell menu] menuRepresentation];
	    selectedIndex = [menuView highlightedItemIndex];
	    numberOfItems = [self numberOfItems];

	    if (selectedIndex < numberOfItems-1)
	      [menuView setHighlightedItemIndex: selectedIndex + 1];
	  }
	  return;
	}
    }
  
  [super keyDown: theEvent];
}

- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  if ([aKey isEqual: NSSelectedIndexBinding])
    {
      [self selectItemAtIndex: [anObject intValue]];
    }
  else if ([aKey isEqual: NSSelectedTagBinding])
    {
      [self selectItemWithTag: [anObject integerValue]];
    }
  else if ([aKey isEqual: NSSelectedObjectBinding])
    {
      // FIXME: This looks wrong to me
      [self selectItemWithTag: [anObject intValue]];
    }
  else if ([aKey isEqual: NSSelectedValueBinding])
    {
      [self selectItemWithTitle: anObject];
    }
  else if ([aKey isEqual: NSContentValuesBinding])
    {
      [self removeAllItems];
      [self addItemsWithTitles: (NSArray*)anObject];
    }
  else
    {
      [super setValue: anObject forKey: aKey];
    }
}

- (id) valueForKey: (NSString*)aKey
{
  if ([aKey isEqual: NSSelectedIndexBinding])
    {
      return [NSNumber numberWithInt: [self indexOfSelectedItem]];
    }
  else if ([aKey isEqual: NSSelectedTagBinding])
    {
      return [NSNumber numberWithInteger: [self selectedTag]];
    }
  else if ([aKey isEqual: NSSelectedObjectBinding])
    {
      // FIXME
      return [NSNumber numberWithInt: [self selectedTag]];
    }
  else if ([aKey isEqual: NSSelectedValueBinding])
    {
      return [self titleOfSelectedItem];
    }
  else if ([aKey isEqual: NSContentValuesBinding])
    {
      return [self itemTitles];
    }
  else
    {
      return [super valueForKey: aKey];
    }
}

@end
