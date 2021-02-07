/* GSKeyBindingTable                    -*-objc-*-

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: February 2002

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

#ifndef _GS_KEYBINDING_TABLE_H
#define _GS_KEYBINDING_TABLE_H

#import "GSKeyBindingAction.h"

@class GSKeyBindingTable;

/* A struct _GSKeyBinding describes how to interpret a single
 * keystroke.  It binds a keystroke to an action (when that keystroke
 * is read, the action is performed), or to a further table of
 * bindings of keystrokes to actions, to be used to interpret the next
 * keystroke.
 *
 * The table is used in the case of multi-stroke bindings, such as
 * Control-x followed by Control-s.  In this case, Control-x will
 * point to a secondary table of keystrokes, which contains Control-s,
 * which binds it to the required action.
 */
struct _GSKeyBinding
{
  /* The character this binding is about.  */
  unichar character;

  /* The modifier for the binding.  Might be a combination of
   * NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask,
   * NSNumericPadKeyMask.
   */
  int modifiers;  

  /* The action, or NULL if there's no action associated.  NB - the action
   * is retained here.  */
  GSKeyBindingAction *action;
  
  /* A further table of _GSKeyBinding to be used to interpret the next
     keystroke, or NULL if this is a 'leaf' keybinding.  NB - the
     table is retained here.  */
  GSKeyBindingTable *table;

  /* Both action and table might be NULL if this is a keybinding which
     has been disabled.  In that case, it's ignored.  */
};


/* This class can manage a table of keybindings (that is, a table of
 * _GSKeyBinding objects).  It can add and remove them, and it can
 * look up a keystroke into the table, and return the associated action,
 * or a GSKeyBindingTable object which can be used to process further
 * keystrokes, for multi-stroke keybindings.
 */
@interface GSKeyBindingTable : NSObject
{
  /* The array of bindings.  */
  struct _GSKeyBinding *_bindings;
  
  /* The length of the array of bindings.  */
  int _bindingsCount;
}
/* Load all the bindings from this dictionary.  The dictionary binds
   keys to actions, as described below under bindKey:toAction:.  The
   DefaultKeyBindings.dict file is an example of such a
   dictionary.  */
- (void) loadBindingsFromDictionary: (NSDictionary *)dict;

/* Bind a specific key [must be a string, representing a keystroke] to
 * a specific action, which can be a string for a selector; an array
 * of strings for an array of selectors; a dictionary if the key is
 * just the prefix to a further array of keybindings; a
 * GSKeyBindingAction for a specific prebuilt action object [this is
 * normally used by the NSInputManager to bind very special actions
 * such as Control-q (interpret literally next keystroke)].
 */
- (void) bindKey: (id)key  toAction: (id)action;

/* The input manager calls this when it wants to look up a keybinding
 * in the table.  The method returns YES if the keybinding is in the
 * table, or NO if it's not.  If it is in the table, it returns the
 * action (or nil if none), and the further keybinding table (or nil
 * if none) to use for interpreting the next keystrokes.  */
- (BOOL) lookupKeyStroke: (unichar)character
	       modifiers: (int)flags
       returningActionIn: (GSKeyBindingAction **)action
		 tableIn: (GSKeyBindingTable **)table;

@end

#endif /* _GS_KEYBINDING_TABLE_H */
