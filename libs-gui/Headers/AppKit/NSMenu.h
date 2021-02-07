/* 
   NSMenu.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Ovidiu Predescu <ovidiu@net-community.com>
   Date: May 1997
   A completely rewritten version of the original source by Scott Christley.
   
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

#ifndef _GNUstep_H_NSMenu
#define _GNUstep_H_NSMenu
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSMenuItem.h>
#import <AppKit/AppKitDefines.h>

@class NSString;
@class NSEvent;
@class NSFont;
@class NSMatrix;
@class NSPopUpButton;
@class NSPopUpButtonCell;
@class NSView;
@class NSWindow;
@class NSMutableArray;
@class NSScreen;

/* ******************* */
/* NSMenuView Protocol */
/* ******************* */
@protocol NSMenuView 
/** Set the menu that this view object will be drawing.
 *  This method will NOT retain the menu.
 *  In normal usage an instance of NSMenu will
 *  use this method to supply the NSMenuView with reference
 *  to itself.  The NSMenu will retain the NSMenuView.
 */
- (void) setMenu: (NSMenu *)menu;

/** Set the currently highlighted item.
 *  This is used by the NSMenu class to restore
 *  the selected item when it is temporary set to
 *  another item.  This happens when both the regular
 *  version and the transient version are on the screen.
 *  A value of -1 means that no item will be highlighted.
 */
- (void) setHighlightedItemIndex: (NSInteger)index;

/** Returns the currently highlighted item.  Returns -1
 *  if no item is highlighted.
 */
- (NSInteger) highlightedItemIndex;

/** This should ensure that if there is an attached
 *  submenu this submenu will be detached.
 *  Detaching means that this particular menu representation
 *  should be removed from the screen.
 *  It should implement a deep detach, that is, all
 *  attached submenus of this menu should also be detached.
 */
- (void) detachSubmenu;

/** This will relayout the NSMenuView. 
 *  It should be called when the menu changes.  Changes include
 *  becoming detached, adding or removing submenu items etcetera.
 *  However, normally it does not need to be called directly because
 *  Because the NSMenuView is supposed to listen to the NSMenu notifications
 *  for the item added, removed and change notifications.
 *  It should be called explicitly when other changes occur, such as
 *  becoming detached or changing the title.
 */
- (void) update;

/** Hm, why is this method needed?  Shouldn't this be done by
 *  the update method?
 */
- (void) sizeToFit;  //!!!

/** Method used by NSMenuItemCell to draw itself correctly and nicely
 *  lined up with the other menu items.
 */
- (float) stateImageWidth; 
/** Method used by NSMenuItemCell to draw itself correctly and nicely
 *  lined up with the other menu items
 */
- (float) imageAndTitleOffset;
/** Methos used by NSMenuItemCell to draw itself correctly and nicely
 *  lined up with the other menu items.
 */
- (float) imageAndTitleWidth;
/** Methos used by NSMenuItemCell to draw itself correctly and nicely
 *  lined up with the other menu items.
 */
- (float) keyEquivalentOffset;
/** Used by NSItemCell to ...
 */
- (float) keyEquivalentWidth;

/** Used by the NSMenu to determine where to position a
 *  submenu.
 */
- (NSPoint) locationForSubmenu: (NSMenu *)aSubmenu;

- (void) performActionWithHighlightingForItemAtIndex: (NSInteger)index; //????

/** <p>This is method is responsible for handling all events while
 *  the user is interacting with this menu.  It should pass on this
 *  call to another menurepresentation when the user moves the
 *  mouse cursor over either a submenu or over the supermenu.
 *  </p>
 *  <p>The method returns when the interaction from the user with the
 *  menu system is over.
 *  </p>
 *  <p>The method returns NO when the user releases the mouse button
 *  above a submenu item and  YES in all other cases.
 *  </p>
 *  <p>This return value can be used to determine if submenus should
 *  be removed from the screen or that they are supposed to stay.
 *  </p>
 *  <p>The implementation should roughly follow the following logic:
 *  </p>
 *  <code>
 *  {
 *    while (have not released mouse button)
 *      {
 *        if (mouse hovers over submenu, or supermenu)
 *          {
 *             if ([(menurepresentation under mouse)
 *                          trackWithEvent: the event])
 *                {
 *                   [self detachSubmenu];
 *                   return YES;
 *                }
 *             return NO;
 *          }
 *          //highlight item under  mouse
 *
 *          if (highlighting submenu item)
 *            {
 *              [self attachSubmenuAtIndex:..];
 *            }
 *          else
 *            {
 *              [self detachSubmenu];
 *            }
 *          get next event.
 *      }
 *
 *    execute the menu action if applicable;
 *
 *    return YES | NO depending on the situation;
 *  }
 *  </code>
 *
 *  Note that actual implementations tend to be more complicated because
 *  because of all kind of useability issues.   Useabilities issues to
 *  look out for are:
 *  <list>
 *   <item>Menus that are only partly on the screen.  Those need to be 
 *         moved while navigation the menu.</item>
 *   <item>Submenus that are hard to reach.  If the natural route to 
 *         the content of a submenu travels through other menu items 
 *         you do not want to remove the submenu immediately.</item>
 *   <item>Transient menus require slightly different behaviour from 
 *         the normal menus.  For example, when selecting a action from 
 *         a transient menu that brings up a modal panel you would 
 *         expect the transient menu to dissappear.  However in the 
 *         normal menu system you want it to stay, so you still have 
 *         feedback on which menu action triggered the modal panel.</item>
 *  </list>
 */
- (BOOL) trackWithEvent: (NSEvent *)event;

@end

/**
 * The NSMenuDelegate protocol defines optional methods implemented
 * by delegates of NSMenu objects.
 */
@protocol NSMenuDelegate <NSObject>

/**
 * Allows the delegate to return the target and action for a key-down event.
 */
- (BOOL) menuHasKeyEquivalent: (NSMenu *)menu
                     forEvent: (NSEvent *)event
                       target: (id *)target
                       action: (SEL *)action;

/**
 * Invoked on the delegate to allow changes before the menu opens.
 */
- (void) menuWillOpen: (NSMenu *)menu;

/**
 * Invoked when the menu is about to be displayed.
 */
- (NSInteger) numberOfItemsInMenu: (NSMenu *)menu;

/**
 * Invoked to indicate that the menu is about to be updated.
 */
- (void) menuNeedsUpdate: (NSMenu *)menu;

/**
 * Invoked to inform the delegate that the menu did close.
 */
- (void) menuDidClose: (NSMenu *)menu;

/**
 * Invoked too notify the delegate that the item will be highlighted.
 */
- (void) menu: (NSMenu *)menu
         willHighlightItem: (NSMenuItem *)item;

/**
 * Invoked to allow the delegate to update an item before
 * it is displayed.
 */ 
- (BOOL) menu: (NSMenu *)menu
   updateItem: (NSMenuItem *)item
      atIndex: (NSInteger)index
 shouldCancel: (BOOL)shouldCancel;

/**
 * Specify a display location for the menu
 */
- (NSRect)confinementRectForMenu: (NSMenu *)menu
                        onScreen: (NSScreen *)screen;

@end

/** <p>Menus provide the user with a list of actions and/or submenus.
 *  Submenus themselves are full fledged menus and so a heirarchical
 *  structure of appears.</p>
 *  <p>Every application has one special menu, the so called Application 
 *  menu.  This menu is always visible on the screen when the application 
 *  is active.  This menu normally contains items like, <em>info</em>, 
 *  <em>services</em>, <em>print</em>, <em>hide</em> and <em>quit</em>.</p>
 *  <p>After the <em>info</em> item normally some submenus follow containing
 *  the application specific actions.</p>
 *  <p>On GNUstep the content of the menu is stacked vertically as 
 *  oppossed to the Windows and Mac world, where they are stacked 
 *  horizontally.  Also because the menus are not part of any normal 
 *  window they can be dragged around opened and closed independend of 
 *  the application windows.</p>
 *  <p>This can lead to a few menus being open simultanuously.
 *  The collection of open menus is remembered,
 *  when the program is started again, all the torn off menus aka
 *  detached menus, are displayed at their last known position.</p>
 *  <p>The menu behaviour is richer than in most other environments and
 *  bear some explanation.  This explanation is aimed at users of Menus 
 *  but more so at the developer of custom menus.</p>
 *  <deflist>
 *   <term>Application menu</term>
 *    <desc>There alwasy at least one menu present and visible while the 
 *          application is active.  This is the application menu. This 
 *          window can never be closed.</desc>
 *   <term>Attached menu</term>
 *    <desc>Normally when you click in a menu on a submenu item, the 
 *          submenu is shown directly next to the menu you click in.
 *          The submenu is now called an <em>attached</em> menu.   It is 
 *          attached to the menu that was clicked in.</desc>
 *   <term>Detached menu</term>
 *    <desc>A menu is detached when it is not attached to its parent 
 *          menu.  A menu can become detached when the user drags a 
 *          submenu away from its parents. A detached window contains in 
 *          its title a close button.</desc>
 *   <term>Transient menu</term>
 *    <desc>A transient menu is a menu that dissappears as
 *          soon as the user stops interacting with the menus.
 *          Typically a transient menu is created when a right mouse 
 *          click appears in an application window.  The right mouse 
 *          click will bring up the Application menu at the place the 
 *          user clicks.  While keeping the mouse button down the
 *          user can select items by moving around.  When releasing the 
 *          button, all transient menus will be removed from the screen 
 *          and the action will be executed.
 *          <p>It is important to note that it is impossible to click 
 *          in transient menus.</p></desc>
 *   <term>Attached transient menu</term>
 *    <desc>This is a menu that is attached and transient at the same 
 *          time.</desc>
 *  </deflist>
 *
 *  A single NSMenu instance can be displayed zero or one times when the 
 *  user is not interaction with the menus.
 *  When the user is interaction with the menus it can occur that the 
 *  same NSMenu is displayed in two locations at the same time.  This is 
 *  only possible when one of the displayed instances is a transient 
 *  menu.
 *  <br/>
 *  To understand how the diffent kind of menus are created lets look 
 *  at some user actions:
 *
 *  <list>
 *   <item>The user clicks on an item which is not a submenu.<br/>
 *         The item is highlighted until the action corresponding with 
 *         the item is completed.  More precisely,  the application 
 *         highlights the menu item, performs the action, and 
 *         unhighlights the item.</item>
 *   <item>The user clicks on a submenu item which is not highlighted 
 *         already<br/> If the submenu is not a detached menu, the 
 *         submenu will become an attached menu to the menu that is 
 *         clicked in.  The item that is clicked in will
 *         become highlighted and stays highlighted.
 *         <p>If the submenu is a detached menu, the transient version
 *         of the submenu will be shown</p></item>
 *   <item>The user clicks on a submenu item which is highlighted<br/>
 *         This means that the submenu is an attached submenu for this 
 *         menu.  After clicking the submenu item will no be no longer 
 *         highlighted and the submenu will be removed from the screen.</item>
 *   <item>The user drags over a menu item<br/>
 *         The item will be highlighted, if the item is a submenu item, 
 *         the submenu will be shown as an attached submenu.  This can 
 *         be transient, or non transient.</item>
 *  </list>
 *
 *  <br/>
 *  <strong>Customizing the look of Menus</strong>
 *  <br/>
 *
 *  There are basically three ways of customizing the look of NSMenu
 *  <enum>
 *   <item>Using custom NSMenuItemCell's.  This you should do when you 
 *         want to influence the look of the items displayed in the 
 *         menu.</item>
 *   <item>Using custom NSMenuView.  This is the class to modify if 
 *         you want to change the way the menu is layout on the screen.  
 *         So if you want to stack the menu items horizontally, you 
 *         should change this class.  This should be rarely needed.</item>
 *   <item>Reimplement NSMenu.  This you should not do. But, if you 
 *         implement everything yourself you can achieve anything.</item>
 *  </enum>
 *
 *  <br/>
 *  <strong>Information for implementing custom NSMenuView class</strong>
 *  <br/>
 *  When implementing a custom NSMenuView class it is important
 *  to keep the following information in mind.
 *
 *  <list>
 *   <item>The menus (or the menu items) form a tree.  Navigating 
 *         through this tree is done with the methods 
 *         [NSMenu-supermenu], which returns the parent menu
 *         of the receiver, and with [NSMenu-itemAtIndex:] which returns 
 *         a NSMenuItem on which we can call [(NSMenuItem)-submenu] for 
 *         a child menu.</item>
 *   <item>The menus as displayed on the screen do NOT form a tree.
 *         This because detached and transient menus lead to duplicate 
 *         menus on the screen.</item>
 *  </list>
 *
 *  The displayed menus on the screen have the following structure:
 *  <enum>
 *   <item>The ordered graph of displayed menus (note, NOT the set of 
 *         NSMenus) form a collection of line graphs.</item>
 *   <item>The attached menus are precisely the non root vertices in 
 *         this graph.</item>
 *   <item>An attached menu of a transient menu is itself a transient 
 *         menu.</item>
 *   <item>The collection of transient menus form connect subgraph of 
 *         the menu graph.</item>
 *  </enum>
 *
 */
@interface NSMenu : NSObject <NSCoding, NSCopying>
{
  NSString *_title;
  NSMutableArray *_items;
  NSView<NSMenuView>* _view;
  NSMenu *_superMenu;
  NSMenu *_attachedMenu;
  NSMutableArray *_notifications;
  id _delegate;

  // GNUstepExtra category
  NSPopUpButtonCell *_popUpButtonCell;
  struct GSMenuFlags {
    unsigned int changedMessagesEnabled: 1;
    unsigned int autoenable: 1;
    unsigned int needsSizing: 1;
    unsigned int is_tornoff: 1;
    unsigned int transient: 1;
    unsigned int horizontal: 1;
    unsigned int mainMenuChanged: 1;
		unsigned int unused: 25;
  } _menu;

@private
  NSWindow *_aWindow;
  NSWindow *_bWindow;
  NSMenu *_oldAttachedMenu;
  int     _oldHiglightedIndex;
  NSString *_name;
}

/** Returns the memory allocation zone used to create instances of this class.
 */
+ (NSZone*) menuZone;
/** Specifies the memory allocation zone used to create instances of this class.
 */
+ (void) setMenuZone: (NSZone*)zone;

+ (void) popUpContextMenu: (NSMenu*)menu
		withEvent: (NSEvent*)event
		  forView: (NSView*)view;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
+ (void) popUpContextMenu: (NSMenu *)menu 
                withEvent: (NSEvent *)event 
                  forView: (NSView *)view 
                 withFont: (NSFont *)font;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (void) popUpMenuPositionItem: (NSMenuItem *)item
                    atLocation: (NSPoint) point
                        inView: (NSView *) view;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
+ (BOOL) menuBarVisible;
+ (void) setMenuBarVisible: (BOOL)flag;
#endif

/** Add newItem to the menu.
 */
- (void) addItem: (id <NSMenuItem>)newItem;

/** Prefered method for inserting a menu item.  This method calls 
 *  [NSMenu-insertItemWithTitle:-action:-keyEquivalent:-atIndex:]
 *  <deflist>
 *   <term>aString</term>
 *    <desc>The title of the specific menu item.</desc>
 *   <term>aSelector</term>
 *    <desc>The action taken by selecting this menu item, or NULL.</desc>
 *   <term>keyEquiv</term>
 *    <desc>The shortcut key for this menu item.  If none is needed, 
 *          specify and empty NSString, ie: @"".</desc>
 *  </deflist>
 *  <p>See Also: -insertItemWithTitle:-action:-keyEquivalent:-atIndex</p>
 */
- (id <NSMenuItem>) addItemWithTitle: (NSString *)aString
                              action: (SEL)aSelector
                       keyEquivalent: (NSString *)keyEquiv;

/** Returns the menu that is attached to this menu.  
 *  <p>
 *  If two instances of this menu are visible,
 *  return the attached window of the transient version
 *  of this menu.</p>
 *  <p>
 *  If no menu is attached return nil.</p>
 */
- (NSMenu*) attachedMenu;

/** Returns YES if item does autoenable (default value) and NO otherwise.
 *  <p>See Also:
 *  </p>
 *  <list>
 *   <item>-setAutoenablesItems:</item>
 *  </list>
 */
- (BOOL) autoenablesItems;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, MAC_OS_X_VERSION_10_1)
- (id) contextMenuRepresentation;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (id) delegate;
#endif

/* Displaying Context-Sensitive Help */
- (void) helpRequested: (NSEvent*)event;

/** Returns the index of item anObject.
 */
- (NSInteger) indexOfItem: (id <NSMenuItem>)anObject;

/** Returns the index of an item with the tag aTag.
 */
- (NSInteger) indexOfItemWithTag: (NSInteger)aTag;

/** Returns the index of an item with the target anObject
 * and the actionSelector.
 */
- (NSInteger) indexOfItemWithTarget: (id)anObject
                   andAction: (SEL)actionSelector;

/** Returns the index of an item with the represented object anObject.
 */
- (NSInteger) indexOfItemWithRepresentedObject: (id)anObject;

/** Returns the index of an item with the submenu anObject.
 */
- (NSInteger) indexOfItemWithSubmenu: (NSMenu *)anObject;

/** Returns the index of an item with the title aTitle.
 */
- (NSInteger) indexOfItemWithTitle: (NSString *)aTitle;

/** <init/>
 */
- (id) initWithTitle: (NSString*)aTitle;

/** Insert newItem at position index.
 */
- (void) insertItem: (id <NSMenuItem>)newItem
            atIndex: (NSInteger)index;

/** Inserts a new menu item at position index.
 *  <p>See Also: 
 *  </p>
 *  <list>
 *   <item>-addItemWithTitle:-action:-keyEquivalent-atIndex:</item>
 *  </list>
 */
- (id <NSMenuItem>) insertItemWithTitle: (NSString *)aString
                                 action: (SEL)aSelector
                          keyEquivalent: (NSString *)charCode
                                atIndex: (NSInteger)index;

/** Returns if this menu is attached to its supermenu,
 *  return nil if it does not have a parent menu.
 *  <p>
 *  If two instances of this menu are visible, return
 *  the outcome of the check for the transient version
 *  of the menu.</p>
 */
- (BOOL) isAttached;

/** If there are two instances of this menu visible, return NO.
 *  Otherwise, return YES if we are a detached menu and visible.
 */
- (BOOL) isTornOff;

/**
 * Returns an array containing all menu items in this menu.
 */
- (NSArray*) itemArray;

/** Returns an item located at index.
 */
- (NSMenuItem *) itemAtIndex: (NSInteger)index;

/** Informs the menu that the specified item has changed.
 */
- (void) itemChanged: (id <NSMenuItem>)anObject;

/** Retuns an item referenced by aTag.
 *  <p>See Also:
 *  </p>
 *  <list>
 *   <item>-indexOfItemWithTag:</item>
 *   <item>[(NSMenuItem)-tag]</item>
 *  </list>
 */
- (id <NSMenuItem>) itemWithTag: (NSInteger)aTag;

/** Returns an item with aString as its title.
 */
- (id <NSMenuItem>) itemWithTitle: (NSString*)aString;

/** Returns the position where submenu will be displayed
 *  when it will be displayed as an attached menu of this menu.
 *  The result is undefined when aSubmenu is not actually a submenu
 *  of this menu.
 */
- (NSPoint) locationForSubmenu: (NSMenu*)aSubmenu;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (float)menuBarHeight;
#endif

- (BOOL) menuChangedMessagesEnabled;

/** Return the NSView that is used for drawing
 *  the menu.
 *  It is the view set with [NSMenu-setMenuRepresentation:] and
 *  therefore it should be safe to assume it is an NSView
 *  implementing the NSMenuView protocol.
 */
- (id) menuRepresentation;

/** Returns the numbers of items on the menu
 */
- (NSInteger) numberOfItems;

/** Simulates a mouse click on item located at index.
 *  <p>See Also:
 *  </p>
 *  <list>
 *   <item>-indexOfItem:</item>
 *   <item>-indexOfItemWithTitle:</item>
 *  </list>
 */
- (void) performActionForItemAtIndex: (NSInteger)index;

/** Looks for a menu item that responds to theEvent on the receiver.  If 
 *  the receiver is a submenu, the method is performed on it.
 */
- (BOOL) performKeyEquivalent: (NSEvent*)theEvent;

/** Calls -removeItemAtIndex: for anItem.
 */
- (void) removeItem: (id <NSMenuItem>)anItem;

/** Removes item at position index.
 */
- (void) removeItemAtIndex: (NSInteger)index;

/** Sets if a menu does autoenable.
 */
- (void) setAutoenablesItems: (BOOL)flag;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, MAC_OS_X_VERSION_10_1)
- (void) setContextMenuRepresentation: (id)representation;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) setDelegate:(id) delegate;
#endif

- (void) setMenuChangedMessagesEnabled: (BOOL)flag;

/** Set the View that should be used to display the menu.
 *  <p>
 *  The default is NSMenuView, but a user can supply its
 *  own NSView object as long as it
 *  </p>
 *  <list>
 *   <item>Inherits from NSView</item>
 *   <item>Implements NSMenuView protocol</item>
 *  </list>
 */
- (void) setMenuRepresentation: (id) menuRep;

/** Set a submenu of a menu.
 *  <deflist>
 *   <term>aMenu</term>
 *    <desc>The submenu to be inserted.</desc>
 *   <term>anItem</term>
 *    <desc>Item to be turned into a submenu.</desc>
 *  </deflist>
 *  <p>See Also:
 *  </p>
 *  <list>
 *   <item>[(NSMenuItem)-setSubmenu:]</item>
 *  </list>
 */
- (void) setSubmenu: (NSMenu*)aMenu forItem: (id <NSMenuItem>)anItem;

/** Set the supermenu of this menu.
 *  TODO:  add explanation if this will change remove this menu
 *  from the old supermenu or if it does not.
 */
- (void) setSupermenu: (NSMenu *)supermenu;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, MAC_OS_X_VERSION_10_1)
- (void) setTearOffMenuRepresentation: (id)representation;
#endif

/** Change the title of the menu.
 */
- (void) setTitle: (NSString*)aTitle;

- (void) sizeToFit;

- (void) submenuAction: (id)sender;

/** Returns the supermenu of this menu.  Return nil
 *  if this is the application menu.  
 */
- (NSMenu*) supermenu;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, MAC_OS_X_VERSION_10_1)
- (id) tearOffMenuRepresentation;
#endif

/** Returns the current title.
 */
- (NSString*) title;

- (void) update;

@end


/**
 * Specifies the protocol to which an object must confirm if it is to be
 * used to validate menu items (in order to implement automatic enabling
 * and disabling of menu items).
 */

@protocol	NSMenuValidation
/** <p>The receiver should return YES if the menuItem is valid ... and should
 *  be enabled in the menu, NO if it is invalid and the user should not be
 *  able to select it.</p>
 *  <p>This method is invoked automatically to determine whether menu items
 *  should be enabled or disabled automatically whenever [NSMenu-update] is
 *  invoked (usually by the applications event loop).</p>
 */
- (BOOL) validateMenuItem: (id<NSMenuItem>)menuItem;
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
@interface NSObject (NSMenuDelegate)
- (void) menuNeedsUpdate: (NSMenu *)menu;
- (NSInteger) numberOfItemsInMenu: (NSMenu *)menu;
- (BOOL) menu: (NSMenu *)menu
   updateItem: (NSMenuItem *)item
      atIndex: (NSInteger)index
 shouldCancel: (BOOL)shouldCancel;
- (BOOL) menuHasKeyEquivalent: (NSMenu *)menu
                     forEvent: (NSEvent *)event
                       target: (id *)target
                      action: (SEL *)action;
@end
#endif

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@interface NSObject (NSMenuActionResponder)
- (BOOL) validateMenuItem: (id<NSMenuItem>)aMenuItem;
@end

/** This interface exist contains methods that are meant
 *  for the NSMenuView.  If you write your own implementation
 *  of the NSMenuView interface you can use these methods
 *  to popup other menus or close them.  
 */
@interface NSMenu (GNUstepExtra)

/** Remove the window from the screen.  This method can/should be
 *  used by the menurepresentation to remove a submenu from the screen.
 */
- (void) close;

/** Remove the transient version of the window from the screen.
 *  This method is used by NSMenuView implementations that need
 *  to open/close transient menus.
 */
- (void) closeTransient;   

/** Show menu on the screen.  This method can/should be used by
 *  the menurepresentation to display a submenu on the screen.
 */
- (void) display;

/** Display the transient version of the menu.  
 */
- (void) displayTransient;

- (BOOL) isPartlyOffScreen; 

/** Returns YES if there is a transient version
 *  of this menu displayed on the screen.
 */
- (BOOL) isTransient;

/* Moving menus */
- (void) nestedSetFrameOrigin: (NSPoint)aPoint;

/** Flag this menu to be the main menu of the application, 
 *  when isMain is YES. Flag it as no longer being the main
 *  menu when NO is handed in.
 *  <p>This method also checks the user defaults to determine how
 *  the menu is to be displayed (eg vertical or horizontal) and can
 *  therefore be used to change window geometry.</p>
 */
- (void) setMain: (BOOL)isMain;

/** When the flag is YES 
 *  this method will detach the receiver from its parent and
 *  update the menurepresentation so it will display a close
 *  button if appropriate.
 *  <p>
 *  If the flag is NO this method will update the menurepresentation
 *  so it will be able to remove the close button if needed.
 *  Note that it will not reattach to its parent menu.</p>
 */
- (void) setTornOff: (BOOL) flag;

/* Shift partly off-screen menus */
- (void) shiftOnScreen;

/** Returns the window in which this menu is displayed.
 *  If there is a transient version it will return the
 *  window in which the transient version is displayed.
 *  If the Menu is not displayed at all the result
 *  is meaningless.
 */
- (NSWindow*) window;

/* Popup behaviour */
- (BOOL) _ownedByPopUp;
- (NSPopUpButtonCell *)_owningPopUp;
- (void) _setOwnedByPopUp: (NSPopUpButtonCell*)popUp;
@end
#endif

APPKIT_EXPORT NSString* const NSMenuDidSendActionNotification;
APPKIT_EXPORT NSString* const NSMenuWillSendActionNotification;
APPKIT_EXPORT NSString* const NSMenuDidAddItemNotification;
APPKIT_EXPORT NSString* const NSMenuDidRemoveItemNotification;
APPKIT_EXPORT NSString* const NSMenuDidChangeItemNotification;
APPKIT_EXPORT NSString* const NSMenuDidBeginTrackingNotification;
APPKIT_EXPORT NSString* const NSMenuDidEndTrackingNotification;

#endif // _GNUstep_H_NSMenu
