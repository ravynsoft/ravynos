/** <title>NSMenuView</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Sep 2001
   Author: David Lazaro Saz <khelekir@encomix.es>
   Date: Oct 1999
   Author: Michael Hanni <mhanni@sprintmail.com>
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

#import <Foundation/NSArray.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSPopUpButtonCell.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"

#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSTitleView.h"


typedef struct _GSCellRect {
  NSRect rect;
} GSCellRect;

#define GSI_ARRAY_TYPES         0
#define GSI_ARRAY_TYPE          GSCellRect

#define GSI_ARRAY_NO_RETAIN
#define GSI_ARRAY_NO_RELEASE

#ifdef GSIArray
#undef GSIArray
#endif
#include <GNUstepBase/GSIArray.h>

static NSMapTable *viewInfo = 0;

#define cellRects ((GSIArray)NSMapGet(viewInfo, self))

#define HORIZONTAL_MENU_LEFT_PADDING 8

/*
  NSMenuView contains:

  a) Title, if needed, this is a subview
  b) menu items
*/

/* A menu's title is an instance of this class */
@class NSButton;

@interface NSMenu (Private)
- (void) _attachMenu: (NSMenu*)aMenu;
@end

@implementation NSMenu (Private)
- (void) _attachMenu: (NSMenu*)aMenu
{
  _attachedMenu = aMenu;
}
@end

@interface NSMenuView (Private)
- (BOOL) _rootIsHorizontal: (BOOL*)isAppMenu;
@end

@implementation NSMenuView (Private)
- (BOOL) _rootIsHorizontal: (BOOL*)isAppMenu
{
  NSMenu *m = _attachedMenu;

  /* Determine root menu of this menu hierarchy */
  while ([m supermenu] != nil)
    {
      m = [m supermenu];
    }
  if (isAppMenu != 0)
    {
      if (m == [NSApp mainMenu])
        {
          *isAppMenu = YES;
        }
      else
        {
          *isAppMenu = NO;
        }
    }
  return [[m menuRepresentation] isHorizontal];
}
@end

@implementation NSMenuView

/*
 * Class methods.
 */

static float menuBarHeight = 0.0;

+ (void) _themeWillDeactivate: (NSNotification*)n
{
  /* Clear cached information from the old theme ... will get info from
   * the new theme as required.
   */
  menuBarHeight = 0;
}

+ (void) initialize
{
  if (viewInfo == 0)
    {
      viewInfo = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
                                  NSNonOwnedPointerMapValueCallBacks, 20);

      [[NSNotificationCenter defaultCenter] addObserver: self
	selector: @selector(_themeWillDeactivate:)
	name: GSThemeWillDeactivateNotification
	object: nil];
    }
}

+ (float) menuBarHeight
{
  if (menuBarHeight == 0.0)
    {
      const CGFloat themeHeight = [[GSTheme theme] menuBarHeight];

      NSFont *font = [NSFont menuBarFontOfSize: 0.0];

      menuBarHeight = [font boundingRectForFont].size.height;
      if (menuBarHeight < themeHeight)
        menuBarHeight = themeHeight;
    }

  return menuBarHeight;
}

/*
 * NSView overrides
 */
- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent
{
  return YES;
}

// We do not want to popup menus in this menu view.
- (NSMenu *) menuForEvent: (NSEvent*) theEvent
{
  NSDebugLLog (@"NSMenu", @"Query for menu in view");
  return nil;
}

/*
 * Init methods.
 */
- (id) initWithFrame: (NSRect)aFrame
{
  self = [super initWithFrame: aFrame];
  if (!self)
    return nil;

  [self setFont: [NSFont menuFontOfSize: 0.0]];

  _highlightedItemIndex = -1;
  _horizontalEdgePad = 4.;

  /* Set the necessary offset for the menuView. That is, how many pixels 
   * do we need for our left side border line.
   */
  _leftBorderOffset = 1;

  // Create an array to store our menu item cells.
  _itemCells = [NSMutableArray new];

  // FIXME: Should this go in NSMenu instead of here?
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];

  return self;
}

- (id) initAsTearOff
{
  self = [self initWithFrame: NSZeroRect];
  if (nil == self)
    return nil;
        
  if (_attachedMenu)
    [_attachedMenu setTornOff: YES];
  
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];

  // We must remove the menu view from the menu list of observers.
  if (_attachedMenu != nil)
    {
      [[NSNotificationCenter defaultCenter] removeObserver: self  
                                            name: nil
                                            object: _attachedMenu];
    }

  /* Clean the pointer to us stored into the _itemCells.  */
  [_itemCells makeObjectsPerformSelector: @selector(setMenuView:)
              withObject: nil];

  RELEASE(_itemCells);
  RELEASE(_font);

  /*
   * Get rid of any cached cell rects.
   */
  {
  GSIArray a = NSMapGet(viewInfo, self);

  if (a != 0)
    {
      GSIArrayEmpty(a);
      NSZoneFree(NSDefaultMallocZone(), a);
      NSMapRemove(viewInfo, self);
    }
  }

  [super dealloc];
}

/*
 * Getting and Setting Menu View Attributes
 */
- (void) setMenu: (NSMenu*)menu
{
  NSNotificationCenter *theCenter = [NSNotificationCenter defaultCenter];
  unsigned count;
  unsigned i;

  if (_attachedMenu != nil)
    {
      // Remove this menu view from the old menu list of observers.
      [theCenter removeObserver: self  name: nil  object: _attachedMenu];
    }

  /* menu is retaining us, so we should not be retaining menu.  */
  _attachedMenu = menu;
  _items_link = [_attachedMenu itemArray];

  if (_attachedMenu != nil)
    {
      // Add this menu view to the menu's list of observers.
      [theCenter addObserver: self
                    selector: @selector(itemChanged:)
                        name: NSMenuDidChangeItemNotification
                      object: _attachedMenu];

      [theCenter addObserver: self
                    selector: @selector(itemAdded:)
                        name: NSMenuDidAddItemNotification
                      object: _attachedMenu];

      [theCenter addObserver: self
                    selector: @selector(itemRemoved:)
                        name: NSMenuDidRemoveItemNotification
                      object: _attachedMenu];
    }

  count = [[[self menu] itemArray] count];
  for (i = 0; i < count; i++)
    {
      NSNumber *n = [NSNumber numberWithInt: i];
      NSDictionary *d;

      d = [NSDictionary dictionaryWithObject: n forKey: @"NSMenuItemIndex"];

      [self itemAdded: [NSNotification
        notificationWithName: NSMenuDidAddItemNotification
        object: self
        userInfo: d]];
    }

  // Force menu view's layout to be recalculated.
  [self setNeedsSizing: YES];
  [self update];
}

- (NSMenu*) menu
{
  return _attachedMenu;
}

- (void) setHorizontal: (BOOL)flag
{
  if (flag == YES && _horizontal == NO)
    {
      NSRect scRect = [[NSScreen mainScreen] frame];
      GSIArray a = NSZoneMalloc(NSDefaultMallocZone(), sizeof(GSIArray_t));

      GSIArrayInitWithZoneAndCapacity(a, NSDefaultMallocZone(), 8);
      NSMapInsert(viewInfo, self, a);

      scRect.size.height = [NSMenuView menuBarHeight];
      [self setFrameSize: scRect.size];
      [self setNeedsSizing: YES];
    }
  else if (flag == NO && _horizontal == YES)
    {
      GSIArray a = NSMapGet(viewInfo, self);

      if (a != 0)
        {
          GSIArrayEmpty(a);
          NSZoneFree(NSDefaultMallocZone(), a);
          NSMapRemove(viewInfo, self);
        }
      [self setNeedsSizing: YES];
    }

  _horizontal = flag;
}

- (BOOL) isHorizontal
{
  return _horizontal;
}

- (void) setFont: (NSFont*)font
{
  ASSIGN(_font, font);
  if (_font != nil)
    {
      const CGFloat themeHeight = [[GSTheme theme] menuItemHeight];

      NSRect r;
  
      r = [_font boundingRectForFont];
      /* Should make up 110, 20 for default font */
      _cellSize = NSMakeSize (r.size.width * 10., r.size.height + 3.);

      if (_cellSize.height < themeHeight)
        _cellSize.height = themeHeight;

      [self setNeedsSizing: YES];
    }
}

- (NSFont*) font
{
  return _font;
}

- (void) setHighlightedItemIndex: (NSInteger)index
{
  NSMenuItemCell *aCell;

  if (index == _highlightedItemIndex)
    return;

  // Unhighlight old
  if (_highlightedItemIndex != -1)
    {
      aCell  = [self menuItemCellForItemAtIndex: _highlightedItemIndex];
      [aCell setHighlighted: NO];
      [self setNeedsDisplayForItemAtIndex: _highlightedItemIndex];
    }

  // Set ivar to new index.
  _highlightedItemIndex = index;

  // Highlight new
  if (_highlightedItemIndex != -1) 
    {
      aCell  = [self menuItemCellForItemAtIndex: _highlightedItemIndex];
      [aCell setHighlighted: YES];
      [self setNeedsDisplayForItemAtIndex: _highlightedItemIndex];
    } 
}

- (NSInteger) highlightedItemIndex
{
  return _highlightedItemIndex;
}

- (void) setMenuItemCell: (NSMenuItemCell *)cell
          forItemAtIndex: (NSInteger)index
{
  NSMenuItem *anItem = [_items_link objectAtIndex: index];
  
  [_itemCells replaceObjectAtIndex: index withObject: cell];

  [cell setMenuItem: anItem];
  [cell setMenuView: self];

  if ([self highlightedItemIndex] == index)
    [cell setHighlighted: YES];
  else
    [cell setHighlighted: NO];

  // Mark the new cell and the menu view as needing resizing.
  [cell setNeedsSizing: YES];
  [self setNeedsSizing: YES];
  [self setNeedsDisplayForItemAtIndex: index];
}

- (NSMenuItemCell*) menuItemCellForItemAtIndex: (NSInteger)index
{
  if ((index >= 0) && (index < [_itemCells count]))
    return [_itemCells objectAtIndex: index];
  else
    return nil;
}

- (NSMenuView*) attachedMenuView
{
  return [[_attachedMenu attachedMenu] menuRepresentation];
}

- (NSMenu*) attachedMenu
{
  return [_attachedMenu attachedMenu];
}

- (BOOL) isAttached
{
  return [_attachedMenu isAttached];
}

- (BOOL) isTornOff
{
  return [_attachedMenu isTornOff];
}

- (void) setHorizontalEdgePadding: (float)pad
{
  _horizontalEdgePad = pad;
  [self setNeedsSizing: YES];
}

- (float) horizontalEdgePadding
{
  return _horizontalEdgePad;
}

/*
 * Notification Methods
 */
- (void) itemChanged: (NSNotification*)notification
{
  int index = [[[notification userInfo] objectForKey: @"NSMenuItemIndex"]
                intValue];
  NSMenuItemCell *aCell = [self menuItemCellForItemAtIndex: index];

  // Enabling of the item may have changed
  [aCell setEnabled: [[aCell menuItem] isEnabled]];
  // Mark the cell associated with the item as needing resizing.
  [aCell setNeedsSizing: YES];

  // Mark the menu view as needing to be resized.
  [self setNeedsSizing: YES];
  [self setNeedsDisplayForItemAtIndex: index];
}

- (void) itemAdded: (NSNotification*)notification
{
  int index  = [[[notification userInfo]
                    objectForKey: @"NSMenuItemIndex"] intValue];
  NSMenuItem *anItem = [_items_link objectAtIndex: index];
  id aCell  = [NSMenuItemCell new];
  int wasHighlighted = _highlightedItemIndex;

  // FIXME do we need to differentiate between popups and non popups
  [aCell setMenuItem: anItem];
  [aCell setMenuView: self];
  [aCell setFont: _font];

  /* Unlight the previous highlighted cell if the index of the highlighted
   * cell will be ruined up by the insertion of the new cell.  */
  if (wasHighlighted >= index)
    {
      [self setHighlightedItemIndex: -1];
    }
  
  [_itemCells insertObject: aCell atIndex: index];
  
  /* Restore the highlighted cell, with the new index for it.  */
  if (wasHighlighted >= index)
    {
      /* Please note that if wasHighlighted == -1, it shouldn't be possible
       * to be here.  */
      [self setHighlightedItemIndex: ++wasHighlighted];
    }

  [aCell setNeedsSizing: YES];
  RELEASE(aCell);

  // Mark the menu view as needing to be resized.
  [self setNeedsSizing: YES];
  [self setNeedsDisplay: YES];
}

- (void) itemRemoved: (NSNotification*)notification
{
  int wasHighlighted = [self highlightedItemIndex];
  int index = [[[notification userInfo] objectForKey: @"NSMenuItemIndex"]
                intValue];

  if (index <= wasHighlighted)
    {
      [self setHighlightedItemIndex: -1];
    }
  [_itemCells removeObjectAtIndex: index];

  if (wasHighlighted > index)
    {
      [self setHighlightedItemIndex: --wasHighlighted];
    }
  // Mark the menu view as needing to be resized.
  [self setNeedsSizing: YES];
  [self setNeedsDisplay: YES];
}

/*
 * Working with Submenus.
 */

- (void) detachSubmenu
{
  NSMenu     *attachedMenu = [_attachedMenu attachedMenu];
  NSMenuView *attachedMenuView;

  if (!attachedMenu)
    return;

  attachedMenuView = [attachedMenu menuRepresentation];

  [attachedMenuView detachSubmenu];

  NSDebugLLog (@"NSMenu", @"detach submenu: %@ from: %@",
               attachedMenu, _attachedMenu);
  
  if ([attachedMenu isTransient])
    {
      [attachedMenu closeTransient];
    }
  else
    {
      [attachedMenu close];
      // Unselect the active item
      [self setHighlightedItemIndex: -1];
    }
}

- (void) attachSubmenuForItemAtIndex: (NSInteger)index
{
  /*
   * Transient menus are used for torn-off menus, which are already on the
   * screen and for sons of transient menus.  As transients disappear as
   * soon as we release the mouse the user will be able to leave submenus
   * open on the screen and interact with other menus at the same time.
   */
  NSMenu *attachableMenu;

  if (index < 0)
    {
      return;
    }
  
  attachableMenu = [[_items_link objectAtIndex: index] submenu];

  if ([attachableMenu isTornOff] || [_attachedMenu isTransient])
    {
      NSDebugLLog (@"NSMenu",  @"Will open transient: %@", attachableMenu);
      [attachableMenu displayTransient];
      [[attachableMenu menuRepresentation] setHighlightedItemIndex: -1]; 
    }
  else
    {
      NSDebugLLog (@"NSMenu",  @"Will open normal: %@", attachableMenu);
      // Check for the main menu of NSWindows95InterfaceStyle case.
      // There we have a separate NSMenuView embedded in the window.
      if ([_attachedMenu menuRepresentation] == self)
        {
          [attachableMenu display];
        }
      else
        {
          [attachableMenu update];
          [attachableMenu sizeToFit];
          [[attachableMenu window] setFrameOrigin: [self locationForSubmenu: attachableMenu]];
          [_attachedMenu _attachMenu: attachableMenu];
          [[attachableMenu window] orderFrontRegardless];
        }
    }
}

/*
 * Calculating Menu Geometry
 */
- (void) update
{
  BOOL needTitleView;
  BOOL rootIsAppMenu;
  NSInterfaceStyle style;

  NSDebugLLog (@"NSMenu", @"update called on menu view");

  /*
   * Ensure that a title view exists only if needed.
   */
  style = NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil);
  if (style == NSWindows95InterfaceStyle || style == NSMacintoshInterfaceStyle)
    {
      needTitleView = NO;
    }
  else if (_attachedMenu == nil)
    {
      needTitleView = NO;
    }
  else if ([self _rootIsHorizontal: &rootIsAppMenu] == YES)
    {
      needTitleView = NO;
    }
  else if (rootIsAppMenu == YES)
    {
      needTitleView = YES;
    } 
  else
    {
      // Popup menu doesn't need title bar
      needTitleView = ([_attachedMenu _ownedByPopUp] == YES) ? NO : YES;
    }

  if (needTitleView == YES && _titleView == nil)
    {
      Class titleViewClass = [[GSTheme theme] titleViewClassForMenuView: self];
      _titleView = [[titleViewClass alloc] initWithOwner: _attachedMenu];
      [self addSubview: _titleView];
      RELEASE(_titleView);
    }
  if (needTitleView == NO && _titleView != nil)
    {
      [_titleView removeFromSuperview];
      _titleView = nil;
    }

  if (_titleView != nil)
    {
      if ([_attachedMenu isTornOff] && ![_attachedMenu isTransient])
        {
          [_titleView
              addCloseButtonWithAction: @selector(_performMenuClose:)];
        }
      else
        {
          [_titleView removeCloseButton];
        }
    }

  // Ask the menu to update itself. This will call sizeToFit if needed.
  [_attachedMenu update];
}

- (void) setNeedsSizing: (BOOL)flag
{
  _needsSizing = flag;
}

- (BOOL) needsSizing
{
  return _needsSizing;
}

- (CGFloat) heightForItem: (NSInteger)idx
{
  NSMenuItemCell *cell = [self menuItemCellForItemAtIndex: idx];

  if (cell != nil)
    {
      NSMenuItem *item = [cell menuItem];
      
      if ([item isSeparatorItem])
	{
	  return [[GSTheme theme] menuSeparatorHeight];
	}
    }
  return _cellSize.height;
}

- (CGFloat) yOriginForItem: (NSInteger)item
{  
  const NSInteger count = [_itemCells count];
  CGFloat total = 0;

  if (item >= 0)
    {
      NSInteger i = 0;
      for (i = (count - 1); i > item; i--)
	{
	  total += [self heightForItem: i];
	}
    }
  return total;
}

- (CGFloat) totalHeight
{
  CGFloat total = 0;
  NSUInteger i = 0;

  for (i = 0; i < [_itemCells count]; i++)
    {
      total += [self heightForItem: i];
    }
  return total;
}

- (void) sizeToFit
{
  BOOL isPullDown =
    [_attachedMenu _ownedByPopUp] && [[_attachedMenu _owningPopUp] pullsDown];

  if (_horizontal == YES)
    {
      unsigned i;
      unsigned howMany = [_itemCells count];
      float currentX = HORIZONTAL_MENU_LEFT_PADDING;
//      NSRect scRect = [[NSScreen mainScreen] frame];

      GSIArrayRemoveAllItems(cellRects);

/*
      scRect.size.height = [NSMenuView menuBarHeight];
      [self setFrameSize: scRect.size];
      _cellSize.height = scRect.size.height;
*/
      _cellSize.height = [NSMenuView menuBarHeight];

      if (howMany && isPullDown)
        {
          GSCellRect elem;
          elem.rect = NSMakeRect (currentX,
                                  0,
                                  (2 * _horizontalEdgePad),
                                  [self heightForItem: 0]);
          GSIArrayAddItem(cellRects, (GSIArrayItem)elem);
          currentX += 2 * _horizontalEdgePad;
        }
      for (i = isPullDown ? 1 : 0; i < howMany; i++)
        {
          GSCellRect elem;
          NSMenuItemCell *aCell = [self menuItemCellForItemAtIndex: i];
          float titleWidth = [aCell titleWidth];

          if ([aCell imageWidth])
            {
              titleWidth += [aCell imageWidth] + GSCellTextImageXDist;
            }

          elem.rect = NSMakeRect (currentX,
                                  0,
                                  (titleWidth + (2 * _horizontalEdgePad)),
                                  [self heightForItem: i]);
          GSIArrayAddItem(cellRects, (GSIArrayItem)elem);

          currentX += titleWidth + (2 * _horizontalEdgePad);
        }
    }
  else
    {
      unsigned i;
      unsigned howMany = [_itemCells count];
      unsigned wideTitleView = 1;
      float    neededImageAndTitleWidth = 0.0;
      float    neededKeyEquivalentWidth = 0.0;
      float    neededStateImageWidth = 0.0;
      float    accumulatedOffset = 0.0;
      float    popupImageWidth = 0.0;
      float    menuBarHeight = 0.0;

      if (_titleView)
        {
          NSMenu *m = [_attachedMenu supermenu];
          NSMenuView *r = [m menuRepresentation];

          neededImageAndTitleWidth = [_titleView titleSize].width;
          if (r != nil && [r isHorizontal] == YES)
            {
              NSMenuItemCell *msr;

              msr = [r menuItemCellForItemAtIndex:
                [m indexOfItemWithTitle: [_attachedMenu title]]];
              neededImageAndTitleWidth
                = [msr titleWidth] + GSCellTextImageXDist;
            }

          if (_titleView)
            menuBarHeight = [[self class] menuBarHeight];
          else
            menuBarHeight += _leftBorderOffset;
        }
      else
        {
          menuBarHeight += _leftBorderOffset;
        }

      for (i = isPullDown ? 1 : 0; i < howMany; i++)
        {
          float aStateImageWidth;
          float aTitleWidth;
          float anImageWidth;
          float anImageAndTitleWidth;
          float aKeyEquivalentWidth;
          NSMenuItemCell *aCell = [self menuItemCellForItemAtIndex: i];
          
          // State image area.
          aStateImageWidth = [aCell stateImageWidth];
          
          // Title and Image area.
          aTitleWidth = [aCell titleWidth];
          anImageWidth = [aCell imageWidth];
          
          // Key equivalent area.
          aKeyEquivalentWidth = [aCell keyEquivalentWidth];
          
          switch ([aCell imagePosition])
            {
              case NSNoImage: 
                anImageAndTitleWidth = aTitleWidth;
                break;
                
              case NSImageOnly: 
                anImageAndTitleWidth = anImageWidth;
                break;
                
              case NSImageLeft: 
              case NSImageRight: 
                anImageAndTitleWidth
                  = anImageWidth + aTitleWidth + GSCellTextImageXDist;
                break;
                
              case NSImageBelow: 
              case NSImageAbove: 
              case NSImageOverlaps: 
              default: 
                if (aTitleWidth > anImageWidth)
                  anImageAndTitleWidth = aTitleWidth;
                else
                  anImageAndTitleWidth = anImageWidth;
                break;
            }
          
          if (aStateImageWidth > neededStateImageWidth)
            neededStateImageWidth = aStateImageWidth;
          
          if (anImageAndTitleWidth > neededImageAndTitleWidth)
            neededImageAndTitleWidth = anImageAndTitleWidth;
                    
          if (aKeyEquivalentWidth > neededKeyEquivalentWidth)
            neededKeyEquivalentWidth = aKeyEquivalentWidth;
          
          // Title view width less than item's left part width
          if ((anImageAndTitleWidth + aStateImageWidth) 
              > neededImageAndTitleWidth)
            wideTitleView = 0;
          
          // Popup menu has only one item with nibble or arrow image
          if (anImageWidth)
            popupImageWidth = anImageWidth;
        }
      if (isPullDown && howMany)
        howMany -= 1;
      
      // Cache the needed widths.
      _stateImageWidth = neededStateImageWidth;
      _imageAndTitleWidth = neededImageAndTitleWidth;
      _keyEqWidth = neededKeyEquivalentWidth;
      
      accumulatedOffset = _horizontalEdgePad;
      if (howMany)
        {
          // Calculate the offsets and cache them.
          if (neededStateImageWidth)
            {
              _stateImageOffset = accumulatedOffset;
              accumulatedOffset += neededStateImageWidth += _horizontalEdgePad;
            }
          
          if (neededImageAndTitleWidth)
            {
              _imageAndTitleOffset = accumulatedOffset;
              accumulatedOffset += neededImageAndTitleWidth;
            }
          
          if (wideTitleView)
            {
              _keyEqOffset = accumulatedOffset = neededImageAndTitleWidth
                + (3 * _horizontalEdgePad);
            }
          else
            {
              _keyEqOffset = accumulatedOffset += (2 * _horizontalEdgePad);
            }
          accumulatedOffset += neededKeyEquivalentWidth + _horizontalEdgePad; 
          
          if ([_attachedMenu supermenu] != nil && neededKeyEquivalentWidth < 8)
            {
              accumulatedOffset += 8 - neededKeyEquivalentWidth;
            }
        }
      else
        {
          accumulatedOffset += neededImageAndTitleWidth + 3 + 2;
          if ([_attachedMenu supermenu] != nil)
            accumulatedOffset += 15;
        }
      
      // Calculate frame size.
      if (_needsSizing)
        {
          // Add the border width: 1 for left, 2 for right sides
          _cellSize.width = accumulatedOffset + 3;
        }

      if ([_attachedMenu _ownedByPopUp])
        {
          _keyEqOffset = _cellSize.width - _keyEqWidth - popupImageWidth;
        }

      [self setFrameSize: NSMakeSize(_cellSize.width + _leftBorderOffset, 
                                     [self totalHeight] 
                                     + menuBarHeight)];
      [_titleView setFrame: NSMakeRect (0, [self totalHeight],
                                        NSWidth (_bounds), menuBarHeight)];
    }
  _needsSizing = NO;
}

- (float) stateImageOffset
{
  if (_needsSizing)
    [self sizeToFit];

  return _stateImageOffset;
}

- (float) stateImageWidth
{
  if (_needsSizing)
    [self sizeToFit];

  return _stateImageWidth;
}

- (float) imageAndTitleOffset
{
  if (_needsSizing)
    [self sizeToFit];

  return _imageAndTitleOffset;
}

- (float) imageAndTitleWidth
{
  if (_needsSizing)
    [self sizeToFit];

  return _imageAndTitleWidth;
}

- (float) keyEquivalentOffset
{
  if (_needsSizing)
    [self sizeToFit];

  return _keyEqOffset;
}

- (float) keyEquivalentWidth
{
  if (_needsSizing)
    [self sizeToFit];

  return _keyEqWidth;
}

- (NSRect) innerRect
{
  if (_horizontal == NO)
    {
      return NSMakeRect (_bounds.origin.x + _leftBorderOffset, 
                         _bounds.origin.y,
                         _bounds.size.width - _leftBorderOffset, 
                         _bounds.size.height);
    }
  else
    {
      return NSMakeRect (_bounds.origin.x, 
                         _bounds.origin.y + _leftBorderOffset,
                         _bounds.size.width, 
                         _bounds.size.height - _leftBorderOffset);
    }
}

- (NSRect) rectOfItemAtIndex: (NSInteger)index
{
  if (_needsSizing == YES)
    {
      [self sizeToFit];
    } 

  // The first item of a pull down menu holds its title and isn't displayed
  if (index == 0 && [_attachedMenu _ownedByPopUp] &&
      [[_attachedMenu _owningPopUp] pullsDown])
    {
      return NSZeroRect;
    }

  if (_horizontal == YES)
    {
      GSCellRect aRect;

      aRect = GSIArrayItemAtIndex(cellRects, index).ext;

      return aRect.rect;
    }
  else
    {
      NSRect theRect;

      theRect.origin.y	= [self yOriginForItem: index];
      theRect.origin.x = _leftBorderOffset;
      theRect.size = _cellSize;
      theRect.size.height = [self heightForItem: index];

      /* NOTE: This returns the correct NSRect for drawing cells, but nothing 
       * else (unless we are a popup). This rect will have to be modified for 
       * event calculation, etc..
       */
      return theRect;
    }
}

- (NSInteger) indexOfItemAtPoint: (NSPoint)point
{
  unsigned howMany = [_itemCells count];
  unsigned i;

  for (i = 0; i < howMany; i++)
    {
      NSRect aRect = [self rectOfItemAtIndex: i];
      
      //NSLog(@"indexOfItemAtPoint called for %@ %@ %d %@", self, NSStringFromPoint(point), i, NSStringFromRect(aRect));
      aRect.origin.x -= _leftBorderOffset;
      aRect.size.width +=  _leftBorderOffset;

      // For horizontal menus, clicking in the left padding should be treated
      // as hitting the first menu item.
      if (_horizontal == YES && i == 0)
        {
          aRect.origin.x -= HORIZONTAL_MENU_LEFT_PADDING;
          aRect.size.width += HORIZONTAL_MENU_LEFT_PADDING;
        }

      if (NSMouseInRect(point, aRect, NO))
        return (int)i;
    }

  return -1;
}

- (void) setNeedsDisplayForItemAtIndex: (NSInteger)index
{
  NSRect aRect;

  aRect = [self rectOfItemAtIndex: index];
  aRect.origin.x -= _leftBorderOffset;
  aRect.size.width +=  _leftBorderOffset;
  [self setNeedsDisplayInRect: aRect];
}

- (NSPoint) locationForSubmenu: (NSMenu *)aSubmenu
{
  NSRect frame = [_window frame];
  NSRect submenuFrame;
  const CGFloat submenuHorizOverlap = [[GSTheme theme] menuSubmenuHorizontalOverlap];
  const CGFloat submenuVertOverlap = [[GSTheme theme] menuSubmenuVerticalOverlap];

  if (_needsSizing)
    [self sizeToFit];

  if (aSubmenu)
    submenuFrame = [[[aSubmenu menuRepresentation] window] frame];
  else
    submenuFrame = NSZeroRect;

  if (_horizontal == NO)
    {
      if (NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", 
                                 [aSubmenu menuRepresentation])
          == GSWindowMakerInterfaceStyle)
        {
          NSRect aRect =  [self convertRect: [self rectOfItemAtIndex: 
            [_attachedMenu indexOfItemWithSubmenu: aSubmenu]] toView: nil];
          NSPoint subOrigin = [_window convertBaseToScreen: aRect.origin];

          return NSMakePoint (NSMaxX(frame) - submenuHorizOverlap,
            subOrigin.y - NSHeight(submenuFrame) - 2 +
            2*[NSMenuView menuBarHeight]);
        }
      else if ([self _rootIsHorizontal: 0] == YES)
        {
          NSRect aRect =  [self convertRect: [self rectOfItemAtIndex: 
            [_attachedMenu indexOfItemWithSubmenu: aSubmenu]] toView: nil];
          NSPoint subOrigin = [_window convertBaseToScreen: aRect.origin];

          // FIXME ... why is the offset +1 needed below? 
          return NSMakePoint (NSMaxX(frame) - submenuHorizOverlap,
            subOrigin.y - NSHeight(submenuFrame) + aRect.size.height + 1);
        }
      else
        {
          return NSMakePoint(NSMaxX(frame) - submenuHorizOverlap,
            NSMaxY(frame) - NSHeight(submenuFrame));
        }
    }
  else
    {
      NSRect aRect =  [self convertRect: [self rectOfItemAtIndex: 
	[_attachedMenu indexOfItemWithSubmenu: aSubmenu]] toView: nil];
      NSPoint subOrigin = [_window convertBaseToScreen: aRect.origin];

      /* If menu is in window, we add +1 for don't lose the track when
	 the user move the mouse from the horizontal menu to a submenu.*/
      if (NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil) ==
	  NSWindows95InterfaceStyle)
	{
	  return NSMakePoint(subOrigin.x,
			     subOrigin.y - NSHeight(submenuFrame) + 1 + submenuVertOverlap);
	}
      else
	{
	  return NSMakePoint(subOrigin.x,
			     subOrigin.y - NSHeight(submenuFrame) + submenuVertOverlap);
	}
    }
}

- (void) resizeWindowWithMaxHeight: (float)maxHeight
{
  // FIXME set the menuview's window to max height in order to keep on screen?
}

- (void) setWindowFrameForAttachingToRect: (NSRect)screenRect 
                                 onScreen: (NSScreen*)screen
                            preferredEdge: (NSRectEdge)edge
                        popUpSelectedItem: (NSInteger)selectedItemIndex
{
  NSRect r;
  NSRect cellFrame;
  NSRect popUpFrame;
  int items = [_itemCells count];
  BOOL growHeight = YES;
  BOOL resizeCell = NO;
  CGFloat borderOffsetInBaseCoords;

  // Our window needs to have a nonzero size for the
  // -convertRect:fromView: and relatead methods to work.
  [_window setFrame: NSMakeRect(0,0,1,1) display: NO];

  // Make sure the menu entries are up to date
  [self update];
  if (_needsSizing)
    [self sizeToFit];

  /* FIXME: Perhaps all of this belongs into NSPopupButtonCell and
     should be used to determine the proper screenRect to pass on into
     this method.
   */
  /* certain style of pulldowns don't want sizing on the _cellSize.height */
  if ([_attachedMenu _ownedByPopUp])
    {
      NSPopUpButtonCell *bcell;

      bcell = [_attachedMenu _owningPopUp];
      if ([bcell pullsDown])
        {
          if ([bcell isBordered] == NO)
            {
              growHeight = NO;
            }
          else
            {
              switch ([bcell bezelStyle])
                {
                  case NSRegularSquareBezelStyle:
                  case NSShadowlessSquareBezelStyle:
                    growHeight = NO;
                    break;
                  default:
                    break;
                }
            }
        }
    }

  cellFrame = screenRect;

  /*
    we should have the calculated cell size, grow the width
    if needed to match the screenRect and vice versa
  */
  if (cellFrame.size.width > [self convertSizeToBase: _cellSize].width) 
    {
      _cellSize.width = [self convertSizeFromBase: cellFrame.size].width;
      resizeCell = YES;
    }
  else
    {
      cellFrame.size.width = [self convertSizeToBase: _cellSize].width;
    }

  /* certain pop-ups don't want the height resized */
  if (growHeight && cellFrame.size.height > [self convertSizeToBase: _cellSize].height) 
    {
      _cellSize.height = [self convertSizeFromBase: cellFrame.size].height;
      resizeCell = YES;
    }
  else
    {
      cellFrame.size.height = [self convertSizeToBase: _cellSize].height;
    }

  /*
     now sizeToFit again with needs sizing = NO so it doesn't 
     overwrite _cellSize just recalculate the offsets.
   */
  if (resizeCell)
    [self sizeToFit];

  /*
   * Compute the frame. popUpFrame is in screen coordinates
   */
  popUpFrame = cellFrame;

  borderOffsetInBaseCoords = [self convertSizeToBase: NSMakeSize(_leftBorderOffset, 0)].width;
   
  if (items > 0)
    {
      float f;

      if (_horizontal == NO)
        {
          f = cellFrame.size.height * (items - 1);
          popUpFrame.size.height += f + borderOffsetInBaseCoords;
          popUpFrame.origin.y -= f;
          popUpFrame.size.width += borderOffsetInBaseCoords;
          popUpFrame.origin.x -= borderOffsetInBaseCoords;

	  // If the menu is a pull down menu the first item, which would
	  // appear at the top of the menu, holds the title and is omitted
	  if ([_attachedMenu _ownedByPopUp])
	    {
	      if ([[_attachedMenu _owningPopUp] pullsDown])
		{
		  popUpFrame.size.height -= cellFrame.size.height;
		  popUpFrame.origin.y += cellFrame.size.height;
		}
	    }

          // Compute position for popups, if needed
          if (selectedItemIndex != -1) 
            {
              popUpFrame.origin.y
                  += cellFrame.size.height * selectedItemIndex;
            }
        }
      else
        {
          f = cellFrame.size.width * (items - 1);
          popUpFrame.size.width += f;

	  // If the menu is a pull down menu the first item holds the
	  // title and is omitted
	  if ([_attachedMenu _ownedByPopUp])
	    {
	      if ([[_attachedMenu _owningPopUp] pullsDown])
		{
		  popUpFrame.size.width -= cellFrame.size.width;
		}
	    }

          // Compute position for popups, if needed
          if (selectedItemIndex != -1) 
            {
              popUpFrame.origin.x -= cellFrame.size.width * selectedItemIndex;
            }
        }
    }  
  
  // Update position, if needed, using the preferredEdge
  if (selectedItemIndex == -1)
    {
      NSRect screenFrame;

      if (screen == nil)
	screen = [NSScreen mainScreen];
      screenFrame = [screen frame];

      popUpFrame.origin.y -= cellFrame.size.height;
      if (edge == NSMinYEdge || edge == NSMaxYEdge)
	{
	  NSRect minYFrame = popUpFrame;
	  NSRect maxYFrame = popUpFrame;

	  // show menu above or below the cell depending on the preferred edge
	  // if the menu would be partially off screen on that edge use the
	  // opposite edge or at least the one where more space is left
	  maxYFrame.origin.y +=
	    maxYFrame.size.height + screenRect.size.height - _leftBorderOffset;

	  if (edge == NSMinYEdge)
	    {
	      if ((NSMinY(minYFrame) < NSMinY(screenFrame))
		  && ((NSMaxY(maxYFrame) <= NSMaxY(screenFrame))
		      || (NSMaxY(screenFrame) - NSMaxY(screenRect) >
			  NSMinY(screenRect) - NSMinY(screenFrame))))
		{
		  edge = NSMaxYEdge;
		}
	    }
	  else
	    {
	      if ((NSMaxY(maxYFrame) > NSMaxY(screenFrame))
		  && ((NSMinY(minYFrame) >= NSMinY(screenFrame))
		      || (NSMaxY(screenFrame) - NSMaxY(screenRect) <
			  NSMinY(screenRect) - NSMinY(screenFrame))))
		{
		  edge = NSMinYEdge;
		}
	    }
	  popUpFrame = edge == NSMinYEdge ? minYFrame : maxYFrame;
	}
      else
	{
	  NSRect minXFrame = popUpFrame;
	  NSRect maxXFrame = popUpFrame;

	  minXFrame.origin.y += screenRect.size.height;
	  minXFrame.origin.x -= minXFrame.size.width;

	  maxXFrame.origin.y += screenRect.size.height;
	  maxXFrame.origin.x += screenRect.size.width + _leftBorderOffset;

	  // show menu on the opposite edge if it does not fit on screen on
	  // the preferred edge
	  if (edge == NSMinXEdge)
	    {
	      if ((NSMinX(minXFrame) < NSMinX(screenFrame))
		  && ((NSMaxX(maxXFrame) <= NSMaxX(screenFrame))
		      || (NSMaxX(screenFrame) - NSMaxX(screenRect) >
			  NSMinX(screenRect) - NSMinX(screenFrame))))
		{
		  edge = NSMaxXEdge;
		}
	    }
	  else
	    {
	      if ((NSMaxX(maxXFrame) > NSMaxX(screenFrame))
		  && ((NSMinX(minXFrame) >= NSMinX(screenFrame))
		      || (NSMaxX(screenFrame) - NSMaxX(screenRect) <
			  NSMinX(screenRect) - NSMinX(screenFrame))))
		{
		  edge = NSMinXEdge;
		}
	    }
	  popUpFrame = edge == NSMinXEdge ? minXFrame : maxXFrame;
	}
    }

  // Get the frameRect
  {
    NSSize contentSize = [self convertSizeFromBase: popUpFrame.size];
    NSRect contentRect = NSMakeRect(popUpFrame.origin.x,
				    popUpFrame.origin.y,
				    contentSize.width,
				    contentSize.height);
    r = [_window frameRectForContentRect: contentRect];
   

    // Set the window frame. r should be identical to popUpFrame except with 
    // any borders the window wanted to add.
    [_window setFrame: r display: NO];
  }
}

/*
 * Drawing.
 */
- (BOOL) isOpaque
{
  return NO;
}

- (void) drawRect: (NSRect)rect
{
  [[GSTheme theme] drawMenuRect: rect
		   inView: self
		   isHorizontal: _horizontal
		   itemCells: _itemCells];
}

/*
 * Event Handling
 */
- (void) performActionWithHighlightingForItemAtIndex: (NSInteger)index
{
  NSMenu     *candidateMenu = _attachedMenu;
  NSMenuView *targetMenuView;
  int        indexToHighlight = index;
  int        oldHighlightedIndex;

  for (;;)
    {
      NSMenu *superMenu = [candidateMenu supermenu];

      if (superMenu == nil
          || [candidateMenu isAttached]
          || [candidateMenu isTornOff])
        {
          targetMenuView = [candidateMenu menuRepresentation];

          break;
        }
      else
        {
          indexToHighlight = [superMenu indexOfItemWithSubmenu: candidateMenu];
          candidateMenu = superMenu;
        }
    }
        
  oldHighlightedIndex = [targetMenuView highlightedItemIndex];
  [targetMenuView setHighlightedItemIndex: indexToHighlight];

  /* We need to let the run loop run a little so that the fact that
   * the item is highlighted gets displayed on screen.
   */
  [[NSRunLoop currentRunLoop] 
    runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];

  [_attachedMenu performActionForItemAtIndex: index];

  if (![_attachedMenu _ownedByPopUp])
    {
      [targetMenuView setHighlightedItemIndex: oldHighlightedIndex];
    }
}

#define MOVE_THRESHOLD_DELTA 2.0
#define DELAY_MULTIPLIER     10

- (BOOL) _executeItemAtIndex: (int)indexOfActionToExecute
	       removeSubmenu: (BOOL)subMenusNeedRemoving
{
  NSInterfaceStyle style =
    NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", self);
  
  if (indexOfActionToExecute >= 0
      && [_attachedMenu attachedMenu] != nil && [_attachedMenu attachedMenu] ==
      [[_items_link objectAtIndex: indexOfActionToExecute] submenu])
    {
      if (style == NSMacintoshInterfaceStyle)
        {
	  // On Macintosh, clicking on or releasing the mouse over a
	  // submenu item always closes the menu (if it is open) and
	  // ends menu tracking. We do the same here, too.
          [self detachSubmenu];
	  return YES;
        }

      if (style == NSWindows95InterfaceStyle)
	{
	  return YES;
	}

      if (subMenusNeedRemoving)
        {
          [self detachSubmenu];
        }
      // Clicked on a submenu.
      return NO;
    }

  return YES;
}

- (BOOL) _trackWithEvent: (NSEvent*)event
        startingMenuView: (NSMenuView*)mainWindowMenuView
{
  NSUInteger eventMask = NSPeriodicMask;
  NSDate *theDistantFuture = [NSDate distantFuture];
  NSPoint lastLocation = {0,0};
  BOOL justAttachedNewSubmenu = NO;
  BOOL subMenusNeedRemoving = YES;
  BOOL shouldFinish = YES;
  BOOL popUpProcessEvents = [[GSTheme theme] doesProcessEventsForPopUpMenu];
  int delayCount = 0;
  int indexOfActionToExecute = -1;
  int firstIndex = -1;
  NSInterfaceStyle style =
    NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", self);
  NSEvent *original;
  NSEventType type;

  /*
   * The original event is unused except to determine whether the method
   * was invoked in response to a right or left mouse down.
   * We pass the same event on when we want tracking to move into a
   * submenu.
   */
  original = AUTORELEASE(RETAIN(event));

  type = [event type];

  /**
   * The following event tracking run loop can run for a long time, during
   * which new windows (for submenus) are created and put on screen. The 
   * window manager might not place the menus where we ask them to be placed 
   * (e.g. Metacity will move menus so they don't overlap the GNOME 
   * top-of-screen panel).
   *
   * The mechanism which updates an NSWindow's frame when an external force 
   * (such as the window manager) moves a window is an NSAppKitDefined event.
   * Since we need the frames of the menu windows to be accurate to know which 
   * menu item the cursor is actually over, and since we are running our own 
   * event loop and the NSAppKitDefined events aren't handled for us, we need
   * to request them ourselves and forward them to the relevant window.
   *
   * NOTE: While it seems messy to have to handle these events, Cocoa doesn't
   * handle them automatically either for code which goes in to an event
   * tracking loop.
   */
  eventMask |= NSAppKitDefinedMask;

  eventMask |= NSRightMouseUpMask | NSRightMouseDraggedMask;
  eventMask |= NSRightMouseDownMask;
  eventMask |= NSOtherMouseUpMask | NSOtherMouseDraggedMask;
  eventMask |= NSOtherMouseDownMask;
  eventMask |= NSLeftMouseUpMask | NSLeftMouseDraggedMask;
  eventMask |= NSLeftMouseDownMask;
  
  /* We need know if the user press a modifier key to close the menu
     when the menu is in a window or when is owned by a popup and theme
     process events. */
  if (style == NSWindows95InterfaceStyle || popUpProcessEvents)
    {
      eventMask |= NSFlagsChangedMask;
    }
  
  // Ignore the first mouse up if menu is horizontal.
  if ([self isHorizontal] == YES ||
      // Or if menu is transient and style is NSWindows95InterfaceStyle.
      ([[self menu] isTransient] && style == NSWindows95InterfaceStyle) ||
      /* Or to mimic Mac OS X behavior for pop up menus. If the user
	 presses the mouse button over a pop up button and then drags the mouse
	 over the menu, the menu is closed when the user releases the mouse. On
	 the other hand, when the user clicks on the button and then moves the
	 mouse the menu is closed upon the next mouse click. */
      ([[self menu] _ownedByPopUp] && (style == NSMacintoshInterfaceStyle ||
				       popUpProcessEvents)))
    {
      /*
       * Ignore the first mouse up if nothing interesting has happened.
       */
      shouldFinish = NO;
    }
  do
    {
      if (type == NSFlagsChanged)
	{
	  /* Close the menu if the user press a modifier key and menu
	     is in a window */
	  if (mainWindowMenuView != nil)
	    {
	      [self setHighlightedItemIndex: -1];
	      [[[mainWindowMenuView menu] attachedMenu] close];
	      return NO;
	    }

	  /* Close the menu if is owned by a popup and theme process events */
	  if ([[self menu] _ownedByPopUp] && popUpProcessEvents)
	    {
	      [[[self menu] _owningPopUp] dismissPopUp];
	      return NO;
	    }
	}

      if (type == NSLeftMouseUp ||
	  type == NSRightMouseUp ||
	  type == NSOtherMouseUp)
        {
          shouldFinish = YES;
        }

      if (type == NSPeriodic || event == original)
        {
          NSPoint location;
          int index;

          location = [_window mouseLocationOutsideOfEventStream];
          index = [self indexOfItemAtPoint: 
            [self convertPoint: location fromView: nil]];

          if (event == original)
            {
              firstIndex = index;
            }
          if (index != firstIndex)
            {
              shouldFinish = YES;
            }

          /*
           * 1 - if menus is only partly visible and the mouse is at the
           *     edge of the screen we move the menu so it will be visible.
           */ 
          if ([_attachedMenu isPartlyOffScreen])
            {
              NSPoint pointerLoc = [_window convertBaseToScreen: location];
              NSRect screenFrame = [[_window screen] visibleFrame];
              /*
               * The +/-1 in the y - direction is because the flipping
               * between X-coordinates and GNUstep coordinates let the
               * GNUstep screen coordinates start with 1.
               */
              if (pointerLoc.x == 0 || pointerLoc.y == 1
                || pointerLoc.x == screenFrame.size.width - 1
                || pointerLoc.y == screenFrame.size.height)
                [_attachedMenu shiftOnScreen];
            }

          /*
           * 2 - Check if we have to reset the justAttachedNewSubmenu
           * flag to NO.
           */
          if (justAttachedNewSubmenu && index != -1
            && index != _highlightedItemIndex)
            { 
              if (location.x - lastLocation.x > MOVE_THRESHOLD_DELTA)
                {
                  delayCount ++;
                  if (delayCount >= DELAY_MULTIPLIER)
                    {
                      justAttachedNewSubmenu = NO;
                    }
                }
              else
                {
                  justAttachedNewSubmenu = NO;
                }
            }


          // 3 - If we have moved outside this menu, take appropriate action
          if (index == -1)
            {
              NSPoint locationInScreenCoordinates;
              NSWindow *windowUnderMouse;
              NSMenu *candidateMenu;

              subMenusNeedRemoving = NO;

              locationInScreenCoordinates
                = [_window convertBaseToScreen: location];

              /*
               * 3a - Check if moved into one of the ancestor menus.
               *      This is tricky, there are a few possibilities:
               *          We are a transient attached menu of a
               *          non-transient menu
               *          We are a non-transient attached menu
               *          We are a root: isTornOff of AppMenu
               */
              candidateMenu = [_attachedMenu supermenu];
              while (candidateMenu  
                && !NSMouseInRect (locationInScreenCoordinates, 
                  [[candidateMenu window] frame], NO) // not found yet
                && (! ([candidateMenu isTornOff] 
                  && ![candidateMenu isTransient]))  // no root of display tree
                && [candidateMenu isAttached]) // has displayed parent
                {
                  candidateMenu = [candidateMenu supermenu];
                }

              if (candidateMenu != nil
                && NSMouseInRect (locationInScreenCoordinates,
                  [[candidateMenu window] frame], NO))
                {
                  BOOL candidateMenuResult;
                  NSMenuView *subMenuView = [[candidateMenu attachedMenu] menuRepresentation];

                  // The call to fetch attachedMenu is not needed. But putting
                  // it here avoids flicker when we go back to an ancestor 
                  // menu and the attached menu is already correct.
                  [subMenuView detachSubmenu];
                  
                  // Reset highlighted index for this menu.
                  // This way if we return to this submenu later there 
                  // won't be a highlighted item.
                  [subMenuView setHighlightedItemIndex: -1];
                  
                  candidateMenuResult = [[candidateMenu menuRepresentation]
                                          _trackWithEvent: original
                                          startingMenuView: mainWindowMenuView];
                  return candidateMenuResult;
                }

              // 3b - Check if we enter the attached submenu
              windowUnderMouse = [[_attachedMenu attachedMenu] window];
              if (windowUnderMouse != nil
                && NSMouseInRect (locationInScreenCoordinates,
                  [windowUnderMouse frame], NO))
                {
                  BOOL wasTransient = [_attachedMenu isTransient];
                  BOOL subMenuResult;

                  subMenuResult
                    = [[self attachedMenuView] _trackWithEvent: original
                                              startingMenuView: mainWindowMenuView];
                  if (subMenuResult
                    && wasTransient == [_attachedMenu isTransient])
                    {
                      [self detachSubmenu];
                    }
                  return subMenuResult;
                }
	      
	      /* We track the menu correctly when this is located
		in a window */
	      if (mainWindowMenuView != nil)
		{
                  // If the user moves the mouse into the main window
                  // horizontal menu, start tracking again.
                  NSWindow *mainWindow = [mainWindowMenuView window];
                  NSPoint locationInMainWindow = [mainWindow 
                    convertScreenToBase: locationInScreenCoordinates];
		  if ([mainWindowMenuView 
                        hitTest: locationInMainWindow] != nil)
		    {
                      int index = [mainWindowMenuView indexOfItemAtPoint: 
                        [mainWindowMenuView 
                          convertPoint: locationInMainWindow
                              fromView: nil]];
                      if (index != -1 &&
                          index != [mainWindowMenuView highlightedItemIndex])
                        {
		          [self setHighlightedItemIndex: -1];
		          return [mainWindowMenuView
                                   _trackWithEvent: original
                                   startingMenuView: mainWindowMenuView];
                        }
		    }
		}
            }

          // 4 - We changed the selected item and should update.
          if (!justAttachedNewSubmenu && index != _highlightedItemIndex)
            {
              subMenusNeedRemoving = NO;
              [self detachSubmenu];
              [self setHighlightedItemIndex: index];

              // WO: Question?  Why the ivar _items_link
              if (index >= 0 && [[_items_link objectAtIndex: index] submenu])
                {
                  [self attachSubmenuForItemAtIndex: index];
                  justAttachedNewSubmenu = YES;
                  delayCount = 0;
                }
            }

          // Update last seen location for the justAttachedNewSubmenu logic.
          lastLocation = location;
        }

      do
	{
	  event = [NSApp nextEventMatchingMask: eventMask
				     untilDate: theDistantFuture
					inMode: NSEventTrackingRunLoopMode
				       dequeue: YES];
	  type = [event type];
	  if (type == NSAppKitDefined)
	    {
	      [[event window] sendEvent: event];
	    }
	}
      while (type == NSAppKitDefined);
    }
  while ((type != NSLeftMouseUp &&
	  type != NSRightMouseUp &&
	  type != NSOtherMouseUp) || shouldFinish == NO);

  /*
   * Ok, we released the mouse
   * There are now a few possibilities:
   * A - We released the mouse outside the menu.
   *     Then we want the situation as it was before
   *     we entered everything.
   * B - We released the mouse on a submenu item
   *     (i) - this was highlighted before we started clicking:
   *           Remove attached menus
   *     (ii) - this was not highlighted before pressed the mouse button;
   *            Keep attached menus.
   * C - We released the mouse above an ordinary action:
   *     Execute the action.
   *
   *  In case A, B and C we want the transient menus to be removed
   *  In case A and C we want to remove the menus that were created
   *  during the dragging.
   *
   *  So we should do the following things:
   * 
   * 1 - Stop periodic events,
   * 2 - Determine the action.
   * 3 - Remove the Transient menus from the screen.
   * 4 - Perform the action if there is one.
   */

  // FIXME
  [NSEvent stopPeriodicEvents];

  /*
   * We need to store this, because _highlightedItemIndex
   * will not be valid after we removed this menu from the screen.
   */
  indexOfActionToExecute = _highlightedItemIndex;

  // remove transient menus. --------------------------------------------
  {
    NSMenu *currentMenu = _attachedMenu;
    
    while (currentMenu && ![currentMenu isTransient])
      {
	currentMenu = [currentMenu attachedMenu];
      }
    
    while ([currentMenu isTransient] && [currentMenu supermenu])
      {
	currentMenu = [currentMenu supermenu];
      }
    
    [[currentMenu menuRepresentation] detachSubmenu];
    
    if ([currentMenu isTransient])
      {
	[currentMenu closeTransient];
      }
  }
  
  if (indexOfActionToExecute == -1)
    {
      return YES;
    }

  // Before executing the action, uncapture the mouse
  [_window _releaseMouse: self];

  /* If we have menu in window, close the menu after select
    an option */
  if (mainWindowMenuView != nil)
    {
      if (self != mainWindowMenuView)
        {
          [mainWindowMenuView setHighlightedItemIndex: -1];
        }
      [[[mainWindowMenuView menu] attachedMenu] close];
    }

  if ([self _executeItemAtIndex: indexOfActionToExecute
                  removeSubmenu: subMenusNeedRemoving] == NO)
    {
      return NO;
    }

  [_attachedMenu performActionForItemAtIndex: indexOfActionToExecute];
  
  /*
   * Remove highlighting.
   * We first check if it still highlighted because it could be the
   * case that we choose an action in a transient window which
   * has already dissappeared.  
   */
  if (_highlightedItemIndex >= 0)
    {
      [self setHighlightedItemIndex: -1];
    }
  return YES;
}

- (BOOL) trackWithEvent: (NSEvent*)event
{
  BOOL result = NO;
  NSMenuView *mainWindowMenuView = nil;
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

  [nc postNotificationName: NSMenuDidBeginTrackingNotification
                    object: [self menu]];
 
  if (NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", self) ==
      NSWindows95InterfaceStyle &&
      ![[self menu] isTransient] &&
      ![[self menu] _ownedByPopUp])
    {
      mainWindowMenuView = self;
    }

  // Capture the mouse so we get clicks outside the menus and
  // GNUstep windows.
  [_window _captureMouse: self];
  NS_DURING
    result = [self _trackWithEvent: event
                  startingMenuView: mainWindowMenuView];
  NS_HANDLER
    [_window _releaseMouse: self];
    [localException raise];
  NS_ENDHANDLER
  [_window _releaseMouse: self];
  [nc postNotificationName: NSMenuDidEndTrackingNotification
                    object: [self menu]];
  return result;
}

/**
   This method is called when the user clicks on a button in the menu.
         Or, if a right click happens and the app menu is brought up.

   The original position is stored, so we can restore the position of menu.
         The position of the menu can change during the event tracking because
   the menu will automatillay move when parts are outside the screen and 
         the user move the mouse pointer to the edge of the screen.
*/
- (void) mouseDown: (NSEvent*)theEvent
{
  NSPoint originalTopLeft = NSZeroPoint; /* Silence compiler.  */
  BOOL restorePosition;
  /*
   * Only for non transient menus do we want
   * to remember the position.
   */ 
  restorePosition = ![_attachedMenu isTransient];

  if (restorePosition && (nil != _window))
    {
      // store old position;
      NSRect originalFrame = [_window frame];
      originalTopLeft = originalFrame.origin;
      originalTopLeft.y += originalFrame.size.height;
    }
  
  [NSEvent startPeriodicEventsAfterDelay: 0.1 withPeriod: 0.01];
  [self trackWithEvent: theEvent];
  [NSEvent stopPeriodicEvents];

  if (restorePosition && (nil != _window))
    {
      NSRect currentFrame = [_window frame];
      NSPoint currentTopLeft = currentFrame.origin;
      currentTopLeft.y += currentFrame.size.height;

      if (NSEqualPoints(currentTopLeft, originalTopLeft) == NO)
        {
          NSPoint origin = currentFrame.origin;
          
          origin.x += (originalTopLeft.x - currentTopLeft.x);
          origin.y += (originalTopLeft.y - currentTopLeft.y);
          [_attachedMenu nestedSetFrameOrigin: origin];
        }
    }
}

- (void) rightMouseDown: (NSEvent*) theEvent
{
  [self mouseDown: theEvent];
}

- (void) otherMouseDown: (NSEvent*) theEvent
{
  [self mouseDown: theEvent];
}

- (BOOL) performKeyEquivalent: (NSEvent *)theEvent
{
  return [_attachedMenu performKeyEquivalent: theEvent];
}

- (void) _themeDidActivate: (NSNotification*)notification
{
  // The new theme may have different menu item sizes, 
  // so the window size for the menu needs to be recalculated.
  [[self menu] sizeToFit];
}

/*
 * NSCoding Protocol
 *
 * Normally unused because NSMenu does not encode its NSMenuView since
 * NSMenuView is considered a platform specific way of rendering the menu.
 */
- (void) encodeWithCoder: (NSCoder*)encoder
{
  [super encodeWithCoder: encoder];
  if ([encoder allowsKeyedCoding] == NO)
    {
      [encoder encodeObject: _itemCells];
      [encoder encodeObject: _font];
      [encoder encodeValueOfObjCType: @encode(BOOL) at: &_horizontal];
      [encoder encodeValueOfObjCType: @encode(float) at: &_horizontalEdgePad];
      [encoder encodeValueOfObjCType: @encode(NSSize) at: &_cellSize];
    }
}

- (id) initWithCoder: (NSCoder*)decoder
{
  self = [super initWithCoder: decoder];
  if (!self)
    return nil;

  if ([decoder allowsKeyedCoding] == NO)
    {
      [decoder decodeValueOfObjCType: @encode(id) at: &_itemCells];
      
      [_itemCells makeObjectsPerformSelector: @selector(setMenuView:)
                  withObject: self];
      
      [decoder decodeValueOfObjCType: @encode(id) at: &_font];
      [decoder decodeValueOfObjCType: @encode(BOOL) at: &_horizontal];
      [decoder decodeValueOfObjCType: @encode(float) at: &_horizontalEdgePad];
      [decoder decodeValueOfObjCType: @encode(NSSize) at: &_cellSize];
      
      _highlightedItemIndex = -1;
      _needsSizing = YES;
    }
  return self;
}

@end

@implementation NSMenuView (GNUstepPrivate)

- (NSArray *)_itemCells
{
  return _itemCells;
}

@end

