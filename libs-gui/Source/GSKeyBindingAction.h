/* GSKeyBindingAction.h                    -*-objc-*-

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

#ifndef _GS_KEYBINDING_ACTION_H
#define _GS_KEYBINDING_ACTION_H

#import <Foundation/Foundation.h>

@class NSInputManager;

/* A GSKeyBindingAction represents a special action to perform.  This
 * action can be bound to any sequence of keystrokes.  When the
 * approrpriate sequence of keystrokes is read from the keyboard, the
 * GSKeyBindingAction object is retrieved, and it is executed.  Once
 * the action object is performed, all the keystrokes which caused the
 * action to be performed are forgotten.  Normally, we store into
 * action objects only actions which we want to be able to store in
 * our complicated keybinding tables ...  so the default action of
 * inserting a keystroke into the text is not stored into an action
 * object.
 */
@interface GSKeyBindingAction: NSObject
{}
/* To execute the action, call the following method.  The
 * implementation of the GSKeyBindingAction class does nothing -
 * actually, it raises an exception.  Subclasses should implement this
 * to do what they need - in the implementation they can call any
 * methods of the input manager, to execute selectors (by calling
 * doCommandBySelector: one or multiple times), or to influence the
 * interpretation of further keystrokes (by modifying the input
 * manager context) - for example to cause the next keystroke to be
 * quoted (inserted literally into the text), or the next action to be
 * repeated N times [or, conceptually, also stuff like starting/ending
 * the recording of a keyboard macro, even if we don't need that].
 */
- (void) performActionWithInputManager: (NSInputManager *)manager;
@end


/* This subclass represents a keybinding where the keystroke is bound
 * to a single selector.  */
@interface GSKeyBindingActionSelector : GSKeyBindingAction
{
  SEL _selector;
}
- (id) initWithSelectorName: (NSString *)sel;
@end


/* This subclass represents a keybinding where the keystroke is bound
 * to an array of selectors.  */
@interface GSKeyBindingActionSelectorArray : GSKeyBindingAction
{
  /* Array of selectors.  */
  SEL *_selectors;

  /* Lenght of the array of selectors.  */
  int _selectorsCount;
}
- (id) initWithSelectorNames: (NSArray *)sels;
@end


/* This subclass represents the action of quoting literally the next
 * keystroke.  It is normally bound to Control-q.  */
@interface GSKeyBindingActionQuoteNextKeyStroke : GSKeyBindingAction
{}
@end

#endif /* _GS_KEYBINDING_ACTION_H */
