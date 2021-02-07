/* 
   NSMenuView.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Michael Hanni <mhanni@sprintmail.com>
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

#ifndef _GNUstep_H_NSMenuView
#define _GNUstep_H_NSMenuView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
//#import <AppKit/NSMenuItemCell.h>
#import <AppKit/NSView.h>

@class NSArray;
@class NSMutableArray;
@class NSNotification;
@class NSString;
@class NSColor;
@class NSMenuItemCell;
@class NSPopUpButton;
@class NSEvent;
@class NSFont;
@class NSScreen;
@class NSWindow;


/**
   The NSMenu class uses an object implementing the NSMenuView protocol to
   do the actual drawing.<br/><br/>

   Normally there is no good reason to write your own class implementing
   this protocol.  However if you want to customize your menus you should
   implement this protocol to ensure that it works nicely together
   with sub/super menus not using your custom menu representation.

   <br/><br/>
   <strong>How menus are drawn</strong>
   <br/><br/>

   This class implements several menu look and feels at the same time.
   The looks and feels implemented are:
   <list>
   <item>
   Ordinary vertically stacked menus with the NeXT submenu positioning 
   behavour.
   </item>
   <item>
   Vertically stacked menus with the WindowMaker submenu placement. This
   behaviour is selected by choosing the 
   <strong>GSWindowMakerInterfaceStyle</strong>.
   </item>
   <item>
   PopupButtons are actually menus. This class implements also the 
   behaviour for the NSPopButtons. See for the the class NSPopButton.
   </item>
   </list>
*/
@interface NSMenuView : NSView <NSCoding, NSMenuView>
{
  NSMutableArray *_itemCells;
  BOOL           _horizontal;
  char		 _pad1[3];
  NSFont         *_font;
  int            _highlightedItemIndex;
  float          _horizontalEdgePad;
  float          _stateImageOffset;
  float          _stateImageWidth;
  float          _imageAndTitleOffset;
  float          _imageAndTitleWidth;
  float          _keyEqOffset;
  float          _keyEqWidth;
  BOOL           _needsSizing;
  char		 _pad2[3];
  NSSize         _cellSize;

@private
  id             _items_link;
  int            _leftBorderOffset;
  id             _titleView;

  /*
  Private and not named '_menu' to avoid confusion and further problems
  with NSResponder's menu.
  */
  NSMenu        *_attachedMenu;
}

/***********************************************************************
 * Initializing a menu view
 ***********************************************************************/
/**
   Creates new instance and sets menu to torn off state with 
   NSMenu's setTornOff:.
 */
- (id) initAsTearOff;

/***********************************************************************
 * Getting and setting menu view attributes
 ***********************************************************************/
/**
   Returns the height of the menu bar.
 */
+ (float) menuBarHeight;

/**
   Sets the menu to be displayed in to menu. Also this method adds this 
   menu view to the menu's list of observers, mark view to force 
   recalculation of layout with setNeedsSizing:YES, and updates itself 
   with update method.
 */
- (void) setMenu: (NSMenu *)menu;

/**
   Returns the NSMenu associated with this menu view.
 */
- (NSMenu *) menu;

/**
   Sets menu orientation. If YES menu items are displayed from left
   to right, if NO from top to bottom (vertically). By default, menu 
   items are displayed vertically.
 */
- (void) setHorizontal: (BOOL)flag;

/**
   Returns YES if menu items are displayed horizontally, NO if 
   vertically.
 */
- (BOOL) isHorizontal;

/**
   Sets the default font to use when drawing the menu text.
 */
- (void) setFont: (NSFont *)font;

/**
   Returns the default font used to draw the menu text.
 */
- (NSFont *) font;

/**
   Highlights item with at index. If index is -1 all highlighing is removed.
 */
- (void) setHighlightedItemIndex: (NSInteger)index;

/**
   Returns the index of the highlighted item. Returns -1 if there is no
   highlighted item.
 */
- (NSInteger) highlightedItemIndex;

/**
   Replaces item cell at index with cell. Highlighting of item is preserved.
 */
- (void) setMenuItemCell: (NSMenuItemCell *)cell
          forItemAtIndex: (NSInteger)index;

/**
   Returns cell associated with item at index.
 */
- (NSMenuItemCell *) menuItemCellForItemAtIndex: (NSInteger)index;

/**
   Returns menu view associated with visible attached submenu.
 */
- (NSMenuView *) attachedMenuView;

/**
   Returns visible attached submenu.
 */
- (NSMenu *) attachedMenu;

/**
   Returns YES, if this object is an visivle attached submenu's view. 
   Returns NO otherwise.
 */
- (BOOL) isAttached;

/**
   Returns YES, if this object is associated with torn off menu 
   (menu with a close button on title bar).
 */
- (BOOL) isTornOff;

/**
   Returns horizontal space used for padding between menu item elements 
   (state image, title image, title, key equivalent, submenu arrow image).
 */
- (float) horizontalEdgePadding;

/**
   Sets amount of pixels added between menu item elements to pad.
 */
- (void) setHorizontalEdgePadding: (float)pad;

/***********************************************************************
 * Notification methods
 ***********************************************************************/
/**
   Marks menu item cell associated with the menu item and menu view as 
   needing to be resized. This method is invoked when 
   NSMenuDidChangeItemNotification received. The notification parameter 
   contains index of changed menu item and can be accessed with 
   NSMenuItemIndex key.
 */
- (void) itemChanged: (NSNotification *)notification;

/**
   Creates new item cell for the newly created menu item, marks cell and
   menu view as needing to be resized. This method is invoked when 
   NSMenuDidAddItemNotification received. The notification parameter 
   contains index of changed menu item and can be accessed with 
   NSMenuItemIndex key.
   
 */
- (void) itemAdded: (NSNotification *)notification;

/**
   Removes cell associated with removed menu item, removes highlighting 
   if removed menu item was highlighted, marks cell and menu view as 
   needing to be resized. This method is invoked when 
   NSMenuDidRemoveItemNotification received. The notification parameter 
   contains index of changed menu item and can be accessed with 
   NSMenuItemIndex key.
   
 */
- (void) itemRemoved: (NSNotification *)notification;

/***********************************************************************
 * Working with submenus
 ***********************************************************************/
/**
 * Detaches currently visible submenu window from main menu.
 */
- (void) detachSubmenu;

/**
   Attach submenu if the item at index is a submenu. It will figure out 
   if the new submenu should be transient or not.
*/
- (void) attachSubmenuForItemAtIndex: (NSInteger)index;

/***********************************************************************
 * Calculating menu geometry
 ***********************************************************************/
/**
   Adds title view for application menu and submenus, removes title view
   if menu is owned by NSPopUpButton, adds close button to title view for
   torn off menus and removes it for other type of menu. 
 */
- (void) update;

/**
   Sets the flag whether layout needs to be resized. Set it to YES if 
   menu contents changed and layout needs to be recalculated. This method
   is used internally. Yout should not invoke it directly from applications.
 */
- (void) setNeedsSizing: (BOOL)flag;

/**
   Return YES if menu view contents changed and needs to be resized.
 */
- (BOOL) needsSizing;

/**
   
 */
- (void) sizeToFit;

/**
   Returns the starting horizontal position for drawing the state image.
 */
- (float) stateImageOffset;

/**
   Returns the width of the state image.
 */
- (float) stateImageWidth;

/**
   Returns the starting horizontal position for drawing the image and title.
 */
- (float) imageAndTitleOffset;

/**
   Returns the width of the image and title section. Tis section contains 
   image and text of menu item.
 */
- (float) imageAndTitleWidth;

/**
   Returns the starting position for drawing the key equivalent. Key 
   equivalent can be submenu arrow if menu item has submenu.
 */
- (float) keyEquivalentOffset;

/**
   Returns the width of key equivalent text. Key equivalent can be submenu 
   arrow if menu item has submenu
 */
- (float) keyEquivalentWidth;

/**
   Returns bounds rectangle of the menu view. It is smaller by 1 pixel 
   in width than menu window (dark gray border at left).
 */
- (NSRect) innerRect;              

/**
   Returns frame rectangle of menu item cell. It is smaller by 1 pixel 
   in width than menu window (dark gray border).
 */
- (NSRect) rectOfItemAtIndex: (NSInteger)index;

/**
   Returns the index of the item below point. Returns -1 if mouse is 
   not above a menu item.
*/
- (NSInteger) indexOfItemAtPoint: (NSPoint)point;

/**
   Calls setNeedsDisplayInRect: for rectangle occupied by item at index.
 */
- (void) setNeedsDisplayForItemAtIndex: (NSInteger)index;

/**
   Returns the correct frame origin for aSubmenu based on the location
   of the receiver. This location may depend on the current NSInterfaceStyle.
*/
- (NSPoint) locationForSubmenu:(NSMenu *) aSubmenu;

/**
   Resize menu view frame to be appropriate in size to attach to screenRect 
   at preferredEdge. For popup's menu, if selectedItemIndex is other than 
   -1, position view so selected item covers the NSPopUpButton.
   
   <br/><strong>NOTE: preffered edge positioning doesn't implemented 
   yet!</strong>
 */
- (void) setWindowFrameForAttachingToRect: (NSRect)screenRect
                                 onScreen: (NSScreen *)screen
                            preferredEdge: (NSRectEdge)edge
                        popUpSelectedItem: (NSInteger)selectedItemIndex;

/***********************************************************************
 * Event handling
 ***********************************************************************/
- (void) performActionWithHighlightingForItemAtIndex: (NSInteger)index;

/**
   This method is responsible for tracking the mouse while this menu
   is on the screen and the user is busy navigating the menu or one
   of it submenus.  Responsible does not mean that this method does it
   all.  For submenus for example it will call, indirectly, itself for
   submenu under consideration.

   It will return YES if user released mouse, not above a submenu item.
   NO in all other circumstances.

   Implementation detail:

   <list>
   <item> It use periodic events to update the highlight state and 
	   attach / detach submenus.
   </item>
   <item> The flag justAttachedNewSubmenu is set to YES when a new 
          submenu is attached. The effect is that the 
          highlighting / attaching / detaching is supressed
          for this menu.  This is done so the user is given
          a change to move the mouse pointer into the newly
          attached submenu.  Otherwise it would immediately
          be removed as the mouse pointer move over another
          item.

          The logic for resetting the flag is rather adhoc.
   </item>

   <item> the flag subMenusNeedRemoving means that we
          will remove all the submenus after we are done.

          This flag is used to clean up the submenus
          when the user has opened a submenu by clicking
          and wants to close it again by clicking on the
          hihglighted item.
   </item>  
   <item> When the user released the mouse this method
          will cleanup all the transient menus.

          Not only its own, but also its attached menu
          and all its transient super menus.
   </item>
   <item> The clean up is done BEFORE the action is executed.
          This is needed otherwise `hiding' the application
          leaves a dangling menu.   If this is not acceptable,
          there should be another mechanism of handling
          the hiding.  BTW besides the `hiding' the application,
          model panels are also a problem when the menu
          is not cleared before executing the action.
    </item>
    </list>
*/
- (BOOL) trackWithEvent: (NSEvent *)event;

@end

#endif
