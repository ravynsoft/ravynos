/* 
   NSMenuItem.h

   The menu cell protocol and the GNUstep menu cell class.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  David Lazaro Saz <khelekir@encomix.es>
   Date: Sep 1999

   Author:  Ovidiu Predescu <ovidiu@net-community.com>
   Date: May 1997
   
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

#ifndef _GNUstep_H_NSMenuItem
#define _GNUstep_H_NSMenuItem
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSAttributedString;
@class NSString;

@class NSMenu;
@class NSImage;

/**
 * Specifies the methods that an object must implement if it is to be
 * placed in a menu as a menu item.  The [NSMenuItem] class provides
 * a reference implementation suitable for most uses.
 */
@protocol NSMenuItem <NSValidatedUserInterfaceItem, NSCopying, NSCoding, NSObject>

/**
   <p> Returns a seperator.   This is just a blank menu item which serves 
   to divide the menu into seperate parts.
   </p>
 */
+ (id<NSMenuItem>) separatorItem;

/**
   <p> 
   Sets a flag that, when set to <code>YES</code>, objects of this class will use user defined key equivalents.
   </p>
 */
+ (void) setUsesUserKeyEquivalents: (BOOL)flag;

/**
   <p> Returns a flag which indicates if the receiver will use user defined 
   key equivalents.
   </p>
 */
+ (BOOL) usesUserKeyEquivalents;

/**
   <p>
   Returns the action of the receiver.
   </p>
 */
- (SEL) action;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
/**
   <p>
   Returns the menu item's title as an attributed string.
   </p>
 */
- (NSAttributedString *)attributedTitle;
#endif

/**
   <p>
   Returns a boolean indicating if the receiver has a sub menu.
   </p>
 */
- (BOOL) hasSubmenu;

/**
   <p>
   Returns the image to be displayed in the receiver.
   </p>
 */
- (NSImage*) image;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
/**
   <p>
   Returns the indentation level, a number between 0 and 15.
   </p>
 */
- (NSInteger)indentationLevel;
#endif

/**
   <p>
   Initializes the receiver with <var>aString</var> as the title.
   The method called with the menu is selected is represented by <var>aSelector</var>.
   The key equivalent which can be used to invoke this menu item is represented by
   <var>charCode</var>.
   </p>
 */
- (id) initWithTitle: (NSString*)aString
	      action: (SEL)aSelector
       keyEquivalent: (NSString*)charCode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL)isAlternate;
#endif

/**
   <p>
   Returns <code>YES</code> if the receiver is enabled.
   </p>
 */
- (BOOL) isEnabled;

/**
   <p>
   Returns a boolean indicating if the receiver is a separator.
   </p>
 */
- (BOOL) isSeparatorItem;

/**
   <p>
   Returns the key equivalent of the receiver.
   </p>
 */
- (NSString*) keyEquivalent;

/**
   <p>
   Returns the key equivalent mask.
   </p>
 */
- (NSUInteger) keyEquivalentModifierMask;

/**
   <p>
   Returns the menu to which this menu item is connected.
   </p>
 */
- (NSMenu*) menu;

/**
   <p>
   Returns the image to be displayed when the receiver is in the "Mixed" state.
   </p>
 */
- (NSImage*) mixedStateImage;
- (NSString*) mnemonic;
- (NSUInteger) mnemonicLocation;

/**
   <p>
   Returns the image to be displayed when the receiver is in the "Off" state.
   </p>
 */
- (NSImage*) offStateImage;

/**
   <p>
   Returns the image to be displayed when the receiver is in the "On" state.
   </p>
 */
- (NSImage*) onStateImage;

/**
   <p>
   Returns the object represented by the reciever.
   </p>
 */
- (id) representedObject;

/**
   <p>
   Sets the action as <var>aSelector</var> on the receiver.
   </p>
 */
- (void) setAction: (SEL)aSelector;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) setAlternate: (BOOL)isAlternate;

-(void) setAttributedTitle: (NSAttributedString *)title;
#endif

/**
   <p>
   Set the receiver to be enabled.
   </p>
 */
- (void) setEnabled: (BOOL)flag;

/**
   <p>
   Sets the image to be displayed in the receiver.
   </p>
 */
- (void) setImage: (NSImage*)menuImage;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void)setIndentationLevel: (NSInteger)level;
#endif

/**
   <p>
   Sets the key equivalent of the receiver.
   </p>
 */
- (void) setKeyEquivalent: (NSString*)aKeyEquivalent;

/**
   <p>
   Sets the modfier for the key equivalent.   These masks indicate if the
   key equivalent requires ALT, Control or other key modifiers.
   </p>
 */
- (void) setKeyEquivalentModifierMask: (NSUInteger)mask;

/**
   <p> Sets the menu which this item belongs to.   This method does not retain the
   object represented by <var>menu</var>.
   </p>
 */
- (void) setMenu: (NSMenu*)menu;

/**
   <p>
   Sets the image to be displayed when the receiver is in the "Mixed" state.
   </p>
 */
- (void) setMixedStateImage: (NSImage*)image;
- (void) setMnemonicLocation: (NSUInteger) location;

/**
   <p>
   Sets the image to be displayed when the receiver is in the "Off" state.
   </p>
 */
- (void) setOffStateImage: (NSImage*)image;

/**
   <p>
   Sets the image to be displayed when the receiver is in the "On" state.
   </p>
 */
- (void) setOnStateImage: (NSImage*)image;

/**
   <p>
   Sets the object represented by the reciever to <var>anObject</var>.
   </p>
 */
- (void) setRepresentedObject: (id)anObject;

/**
   <p>
   Sets the state of the the receiver.
   </p>
 */
- (void) setState: (NSInteger)state;

/**
   <p>
   Sets the submenu of the receiver.  This method does retain the 
   <var>submenu</var> object.
   </p>
 */
- (void) setSubmenu: (NSMenu*)submenu;

/**
   <p>
   Sets the tag of the reciever as <var>anInt</var>.
   </p>
 */
- (void) setTag: (NSInteger)anInt;

/**
   <p>
   Sets the target as <var>anObject</var> on the receiver.
   </p>
 */
- (void) setTarget: (id)anObject;

/**
   <p>
   Sets the title of the menu, represented by <var>aString</var>.
   </p>
 */
- (void) setTitle: (NSString*)aString;
- (void) setTitleWithMnemonic: (NSString*)stringWithAmpersand;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) setToolTip: (NSString *)toolTip;
#endif

/**
   <p>
   Returns the state of the receiver.
   </p>
 */
- (NSInteger) state;
/**
   <p>
   Returns the attached submenu.
   </p>
 */
- (NSMenu*) submenu;

/**
   <p>
   Returns the tag of the receiver.
   </p>
 */
- (NSInteger) tag;

/**
   <p>
   Returns the target of the receiver.
   </p>
 */
- (id) target;

/**
   <p>
   Returns the menu's title.
   </p>
 */
- (NSString*) title;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSString *) toolTip;
#endif

/**
   <p>
   Returns the user defined key equivalent modifier.
   </p>
 */
- (NSUInteger) userKeyEquivalentModifierMask;

/**
   <p>
   Returns the key equivalent defined by the users defaults.
   </p>
 */
- (NSString*) userKeyEquivalent;

@end

@interface NSMenuItem : NSObject <NSMenuItem, NSValidatedUserInterfaceItem>
{
  NSMenu *_menu;
  NSString *_title;
  NSString *_keyEquivalent;
  NSUInteger _keyEquivalentModifierMask;
  NSUInteger _mnemonicLocation;
  NSInteger _state;
  NSImage *_image;
  NSImage *_onStateImage;
  NSImage *_offStateImage;
  NSImage *_mixedStateImage;
  id _target;
  SEL _action;
  NSInteger _tag;
  id _representedObject;
  NSMenu *_submenu;
  BOOL _enabled;
  BOOL _changesState;
  BOOL _isAlternate;
  char _indentation; // 0..15
  NSString *_toolTip;
}

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@interface NSMenuItem (GNUstepExtra)
- (void) setChangesState: (BOOL)flag;
- (BOOL) changesState;
@end
#endif

#endif // _GNUstep_H_NSMenuItem

