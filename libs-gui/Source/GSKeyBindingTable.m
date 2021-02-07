/* GSKeyBindingTable.m                    -*-objc-*-

   Copyright (C) 2002-2011 Free Software Foundation, Inc.

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

#import "AppKit/NSEvent.h"
#import "AppKit/NSInputManager.h"
#import "GSKeyBindingAction.h"
#import "GSKeyBindingTable.h"

@implementation GSKeyBindingTable : NSObject

- (void) loadBindingsFromDictionary: (NSDictionary *)dict
{
  NSEnumerator *e;
  id key;
  
  e = [dict keyEnumerator];
  while ((key = [e nextObject]) != nil)
    {
      [self bindKey: key  toAction: [dict objectForKey: key]];
    }
}

- (void) bindKey: (id)key  toAction: (id)action
{
  unichar character;
  unsigned int modifiers;
  GSKeyBindingAction *a = nil;
  GSKeyBindingTable *t = nil;
  BOOL isTable = NO;
  int i;

  /* First, try to determine what exactly is key :-) ... it might
     either be a simple string, "Control-f", or an array,
     ("Control-x", "Control-s").  We implement the case of arrays in
     terms of the case of strings.  */
  if ([key isKindOfClass: [NSArray class]])
    {
      if ([(NSArray *)key count] == 0)
	{
	  /* Ignore them.  */
	  return;
	}
      else if ([(NSArray *)key count] == 1)
	{
	  key = [key objectAtIndex: 0];
	}
      else
	{
	  /* Ok - simply convert that into a temporary dictionary
	     representation and store that.  Eg, key ("Control-x",
	     "Control-s", "Control-k") action "moveUp:" gets converted
	     into: key "Control-x" action { "Control-s" = {
	     "Control-k" = "moveUp:"; }; }.  */

	  /* Now start from the end of the array, and start building
	     the temporary dictionary structure going backwards.  */
	  id value = action;
	  int j;

	  for (j = [key count] - 1; j > 0; j--)
	    {
	      NSMutableDictionary *tmp = [NSMutableDictionary dictionary];
	      [tmp setObject: value  forKey: [key objectAtIndex: j]];
	      value = tmp;
	    }
	  key = [key objectAtIndex: 0];
	  action = value;
	}
    }
  

  if (![key isKindOfClass: [NSString class]])
    {
      NSLog (@"GSKeyBindingTable - key %@ is not a NSString!", key);
      return;
    }
  
  if (![NSInputManager parseKey: (NSString *)key 
		       intoCharacter: &character
		       andModifiers: &modifiers])
    {
      NSLog (@"GSKeyBindingTable - Could not bind key %@", key);
      return;
    }

  /* If it is not a function key, we automatically ignore the Shift
   * modifier.  You shouldn't use it unless you are describing a modification
   * of a function key.  The NSInputManager will ignore Shift modifiers
   * as well for non-function keys.  */
  if (modifiers & NSFunctionKeyMask)
    {
      /* Ignore all other modifiers when storing the keystroke modifiers.  */
      modifiers = modifiers & (NSShiftKeyMask 
			       | NSAlternateKeyMask 
			       | NSControlKeyMask 
			       | NSNumericPadKeyMask);
    }
  else
    {
      modifiers = modifiers & (NSAlternateKeyMask 
			       | NSControlKeyMask 
			       | NSNumericPadKeyMask);
    }


  /* Now build the associated action/table.  */
  if ([action isKindOfClass: [NSString class]])
    {
      /* "" as action means disable the keybinding.  It can be used to
	 override a previous keybinding.  */
      if ([(NSString *)action isEqualToString: @""])
	{
	  a = nil;
	}
      else
	{
	  a = [[GSKeyBindingActionSelector alloc] 
		initWithSelectorName: (NSString *)action];
	  AUTORELEASE (a);
	}
    }
  else if ([action isKindOfClass: [NSArray class]])
    {
      a = [[GSKeyBindingActionSelectorArray alloc]
	    initWithSelectorNames: (NSArray *)action];
      AUTORELEASE (a);
    }
  else if ([action isKindOfClass: [NSDictionary class]])
    {
      /* Don't load the keybindings from action yet ... load them
	 later on when we know if we need to create a new
	 GSKeyBindingTable or if we need to merge them into an
	 existing one.  */
      isTable = YES;
    }
  else if ([action isKindOfClass: [GSKeyBindingAction class]])
    {
      a = action;
    }

  /* Ok - at this point, we have all the elements ready, we just need
     to insert into the table.  */
    
  /* Check if there are already some bindings for this keystroke.  */
  for (i = 0; i < _bindingsCount; i++)
    {
      if ((_bindings[i].character == character)  
	  &&  (_bindings[i].modifiers == modifiers))
	{
	  /* Replace/override the existing action with the new one if
	     it's an action, or load the bindings into a (new or
	     existing) table if it's a table.  */
	  if (isTable)
	    {
	      /* If there was already a table, add keybindings to that
		 table.  */
	      if (_bindings[i].table != nil)
		{
		  t = _bindings[i].table;
		}
	      else
		{
		  /* Else, create a new one.  */
		  t = [[GSKeyBindingTable alloc] init];
		  AUTORELEASE (t);
		}
	      [t loadBindingsFromDictionary: (NSDictionary *)action];
	    }

	  ASSIGN (_bindings[i].action, a);
	  ASSIGN (_bindings[i].table, t);
	  return;
	}
    }

  /* Ok - new keystroke.  Create the table if needed.  */
  if (isTable)
    {
      t = [[GSKeyBindingTable alloc] init];
      AUTORELEASE (t);
      [t loadBindingsFromDictionary: (NSDictionary *)action];
    }

  /* Allocate memory for the new binding.  */
  if (_bindingsCount == 0)
    {
      _bindingsCount = 1;
      _bindings = malloc (sizeof (struct _GSKeyBinding));
    }
  else
    {
      _bindingsCount++;
      _bindings = realloc (_bindings, sizeof (struct _GSKeyBinding) 
				* _bindingsCount);
    }
  _bindings[_bindingsCount - 1].character = character;
  _bindings[_bindingsCount - 1].modifiers = modifiers;

  /* Don't use ASSIGN here because that uses the previous value of
     _bindings[_bindingsCount - 1] ... which is undefined.  */
  _bindings[_bindingsCount - 1].action = a;
  RETAIN (a);
  _bindings[_bindingsCount - 1].table = t;
  RETAIN (t);
}

- (BOOL) lookupKeyStroke: (unichar)character
	       modifiers: (int)flags
       returningActionIn: (GSKeyBindingAction **)action
		 tableIn: (GSKeyBindingTable **)table
{
  int i;
  
  for (i = 0; i < _bindingsCount; i++)
    {
      if (_bindings[i].character == character)
	{
	  if (_bindings[i].modifiers == flags)
	    {
	      if (_bindings[i].action == nil  &&  _bindings[i].table == nil)
		{
		  /* Found the keybinding, but it is disabled!  */
		  return NO;
		}
	      else
		{
		  *action = _bindings[i].action;
		  *table = _bindings[i].table;
		  return YES;
		}
	    }
	}
    }
  return NO;
}

- (void) dealloc
{
  int i;

  for (i = 0; i < _bindingsCount; i++)
    {
      RELEASE (_bindings[i].action);
      RELEASE (_bindings[i].table);
    }
  free (_bindings);
  [super dealloc];
}

@end


