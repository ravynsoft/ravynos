/* NSInputManager                              -*-objc-*-

   Copyright (C) 2001, 2002 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 2001, January 2002, February 2002

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

#import <Foundation/NSException.h>
#import "AppKit/NSEvent.h"
#import "AppKit/NSInputManager.h"
#import "AppKit/NSInputServer.h"
#import "AppKit/NSText.h"
#import "AppKit/NSHelpManager.h"

/* For NSBeep () */
#import "AppKit/NSGraphics.h"

#import "GSKeyBindingAction.h"
#import "GSKeyBindingTable.h"

/* A table mapping character names to characters, used to interpret
   the character names found in KeyBindings dictionaries.  */

static struct 
{
  NSString *name;
  unichar character;
} 
character_table[] =
{
  /* Function keys.  */
  { @"UpArrow", NSUpArrowFunctionKey },
  { @"DownArrow", NSDownArrowFunctionKey },
  { @"LeftArrow", NSLeftArrowFunctionKey },
  { @"RightArrow", NSRightArrowFunctionKey },
  { @"F1", NSF1FunctionKey },
  { @"F2", NSF2FunctionKey },
  { @"F3", NSF3FunctionKey },
  { @"F4", NSF4FunctionKey },
  { @"F5", NSF5FunctionKey },
  { @"F6", NSF6FunctionKey },
  { @"F7", NSF7FunctionKey },
  { @"F8", NSF8FunctionKey },
  { @"F9", NSF9FunctionKey },
  { @"F10", NSF10FunctionKey },
  { @"F11", NSF11FunctionKey },
  { @"F12", NSF12FunctionKey },
  { @"F13", NSF13FunctionKey },
  { @"F14", NSF14FunctionKey },
  { @"F15", NSF15FunctionKey },
  { @"F16", NSF16FunctionKey },
  { @"F17", NSF17FunctionKey },
  { @"F18", NSF18FunctionKey },
  { @"F19", NSF19FunctionKey },
  { @"F20", NSF20FunctionKey },
  { @"F21", NSF21FunctionKey },
  { @"F22", NSF22FunctionKey },
  { @"F23", NSF23FunctionKey },
  { @"F24", NSF24FunctionKey },
  { @"F25", NSF25FunctionKey },
  { @"F26", NSF26FunctionKey },
  { @"F27", NSF27FunctionKey },
  { @"F28", NSF28FunctionKey },
  { @"F29", NSF29FunctionKey },
  { @"F30", NSF30FunctionKey },
  { @"F31", NSF31FunctionKey },
  { @"F32", NSF32FunctionKey },
  { @"F33", NSF33FunctionKey },
  { @"F34", NSF34FunctionKey },
  { @"F35", NSF35FunctionKey },
  { @"Insert", NSInsertFunctionKey },
  { @"Delete", NSDeleteFunctionKey },
  { @"Home", NSHomeFunctionKey },
  { @"Begin", NSBeginFunctionKey },
  { @"End", NSEndFunctionKey },
  { @"PageUp", NSPageUpFunctionKey },
  { @"PageDown", NSPageDownFunctionKey },
  { @"PrintScreen", NSPrintScreenFunctionKey },
  { @"ScrollLock", NSScrollLockFunctionKey },
  { @"Pause", NSPauseFunctionKey },
  { @"SysReq", NSSysReqFunctionKey },
  { @"Break", NSBreakFunctionKey },
  { @"Reset", NSResetFunctionKey },
  { @"Stop", NSStopFunctionKey },
  { @"Menu", NSMenuFunctionKey },
  { @"User", NSUserFunctionKey },
  { @"System", NSSystemFunctionKey },
  { @"Print", NSPrintFunctionKey },
  { @"ClearLine", NSClearLineFunctionKey },
  { @"ClearDisplay", NSClearDisplayFunctionKey },
  { @"InsertLine", NSInsertLineFunctionKey },
  { @"DeleteLine", NSDeleteLineFunctionKey },
  { @"InsertChar", NSInsertCharFunctionKey },
  { @"DeleteChar", NSDeleteCharFunctionKey },
  { @"Prev", NSPrevFunctionKey },
  { @"Next", NSNextFunctionKey },
  { @"Select", NSSelectFunctionKey },
  { @"Execute", NSExecuteFunctionKey },
  { @"Undo", NSUndoFunctionKey },
  { @"Redo", NSRedoFunctionKey },
  { @"Find", NSFindFunctionKey },
  { @"Help", NSHelpFunctionKey },
  { @"ModeSwitch", NSModeSwitchFunctionKey },

  /* Special characters by name.  Useful if you want, for example,
     to associate some special action to C-Tab or similar evils.  */
  { @"Backspace", NSDeleteCharacter },
  { @"BackTab", NSBackTabCharacter },
  { @"Tab", NSTabCharacter },
  { @"Enter", NSEnterCharacter },
  { @"FormFeed", NSFormFeedCharacter },
  { @"CarriageReturn", NSCarriageReturnCharacter },
  { @"Escape", 0x1b }
};

static int CHARACTER_TABLE_SIZE = (sizeof(character_table) / sizeof(character_table[0]));

static NSInputManager *currentInputManager = nil;

@implementation NSInputManager

+ (NSInputManager *) currentInputManager
{
  if (currentInputManager == nil)
    {
      currentInputManager = [[self alloc] initWithName: nil  host: nil];
    }
  
  return currentInputManager;
}

+ (BOOL) parseKey: (NSString *)key 
    intoCharacter: (unichar *)character
     andModifiers: (unsigned int *)modifiers
{
  int flags = 0;
  unichar c = 0;

  /* Parse the key: first break it into segments separated by - */
  NSArray *components = [key componentsSeparatedByString: @"-"];
  NSString *name;

  /* Then, parse the modifiers.  The modifiers are the components 
     - all of them except the last one!  */
  int i, count = [components count];

  for (i = 0; i < count - 1; i++)
    {
      NSString *modifier = [components objectAtIndex: i];
      
      if ([modifier isEqualToString: @"Control"] 
          || [modifier isEqualToString: @"Ctrl"] 
          || [modifier isEqualToString: @"C"])
        {
          flags |= NSControlKeyMask;
        }
      else if ([modifier isEqualToString: @"Alternate"] 
               || [modifier isEqualToString: @"Alt"] 
               || [modifier isEqualToString: @"A"]
               || [modifier isEqualToString: @"Meta"]
               || [modifier isEqualToString: @"M"])
        {
          flags |= NSAlternateKeyMask;
        }
      /* The Shift modifier is only meaningful when used in
       * conjunction with function keys.  'Shift-LeftArrow' is
       * meaningful; 'Control-Shift-g' is not - you should use
       * 'Control-G' instead.  */
      else if ([modifier isEqualToString: @"Shift"] 
               || [modifier isEqualToString: @"S"])
        {
          flags |= NSShiftKeyMask;
        }
      else if ([modifier isEqualToString: @"NumericPad"]
               ||  [modifier isEqualToString: @"Numeric"]
               ||  [modifier isEqualToString: @"N"])
        {
          flags |= NSNumericPadKeyMask;
        }
      else
        {
          NSLog (@"NSInputManager - unknown modifier '%@' ignored", modifier);
          return NO;
        }
    }

  /* Now, parse the actual key.  */
  name = [components objectAtIndex: (count - 1)];
  
  if ([name isEqualToString: @""])
    {
      /* This happens if '-' was the character.  */
      c = '-';
    }
  else if ([name length] == 1)
    {
      /* A single character, such as 'a'.  */
      c = [name characterAtIndex: 0];
    }
  else
    {
      /* A descriptive string, such as Tab or Home.  */
      for (i = 0; i < CHARACTER_TABLE_SIZE; i++)
        {
          if ([name isEqualToString: (character_table[i]).name])
            {
              c = (character_table[i]).character;
              flags |= NSFunctionKeyMask;
              break;
            }
        }
      if (i == CHARACTER_TABLE_SIZE)
        {
          NSLog (@"NSInputManager - unknown character '%@' ignored", name);
          return NO;
        }
    }  
  if (character != NULL)
    {
      *character = c;
    }

  if (modifiers != NULL)
    {
      *modifiers = flags;
    }

  return YES;
}

+ (NSString *) describeKeyStroke: (unichar)character
                   withModifiers: (unsigned int)modifiers
{
  NSMutableString *description = [NSMutableString new];
  int i;

  if (modifiers & NSCommandKeyMask)
    {
      [description appendString: @"Command-"];
    }

  if (modifiers & NSControlKeyMask)
    {
      [description appendString: @"Control-"];
    }

  if (modifiers & NSAlternateKeyMask)
    {
      [description appendString: @"Alternate-"];
    }

  if (modifiers & NSShiftKeyMask)
    {
      [description appendString: @"Shift-"];
    }

  if (modifiers & NSNumericPadKeyMask)
    {
      [description appendString: @"NumericPad-"];
    }

  for (i = 0; i < CHARACTER_TABLE_SIZE; i++)
    {
      if (character == ((character_table[i]).character))
        {
          [description appendString: character_table[i].name];
          break;
        }
    }  

  if (i == CHARACTER_TABLE_SIZE)
    {
      NSString *c = [NSString stringWithCharacters: &character  length: 1];
      [description appendString: c];
    }
  return AUTORELEASE(description);
}

- (void) loadBindingsFromFile: (NSString *)fullPath
{
  NSDictionary *bindings;
      
  bindings = [NSDictionary dictionaryWithContentsOfFile: fullPath];
  if (bindings == nil)
    {
      NSLog (@"Unable to load KeyBindings from file %@", fullPath);
    }
  else
    {
      [_rootBindingTable loadBindingsFromDictionary: bindings];
    }
}

- (void) loadBindingsWithName: (NSString *)fileName
{
  NSArray *paths;
  NSEnumerator *enumerator;
  NSString *libraryPath;
  NSFileManager *fileManager = [NSFileManager defaultManager];

  paths = NSSearchPathForDirectoriesInDomains (NSLibraryDirectory,
                                               NSAllDomainsMask, YES);
  /* paths are in the order - user, network, local, root. Instead we
     want to load keybindings in the order root, local, network, user
     - so that user can override root - for this reason we use a
     reverseObjectEnumerator.  */
  enumerator = [paths reverseObjectEnumerator];
  while ((libraryPath = [enumerator nextObject]) != nil)
    {
      NSString *fullPath;
      fullPath =
        [[[libraryPath stringByAppendingPathComponent: @"KeyBindings"]
          stringByAppendingPathComponent: fileName]
          stringByAppendingPathExtension: @"dict"];
      if ([fileManager fileExistsAtPath: fullPath])
        {
          [self loadBindingsFromFile: fullPath];
        }
    }
}

- (NSInputManager *) initWithName: (NSString *)inputServerName
                             host: (NSString *)hostName
{
  NSUserDefaults *defaults;
  CREATE_AUTORELEASE_POOL (pool);

  defaults = [NSUserDefaults standardUserDefaults];

  self = [super init];

  _rootBindingTable = [GSKeyBindingTable new];
  
  /* Read the abort key from the user defaults.  */
  {
    NSString *abortKey = [defaults stringForKey: @"GSAbortKey"];

    if (abortKey == nil)
      {
        _abortCharacter = 'g';
        _abortFlags = NSControlKeyMask;                
      }
    else if (![NSInputManager parseKey: abortKey  
                              intoCharacter: &_abortCharacter
                              andModifiers: &_abortFlags])
      {
        NSLog (@"Could not parse GSAbortKey - using Control-g");
        _abortCharacter = 'g';
        _abortFlags = NSControlKeyMask;                
      }
  }

  /* Read if we should insert Control- keystrokes into the text.  
     This defaults to NO.  */
  _insertControlKeystrokes = [defaults boolForKey: 
                                         @"GSInsertControlKeystrokes"];

  /* Read the quote key from the user defaults.  */
  {
    NSString *quoteKey = [defaults stringForKey: @"GSQuoteKey"];
    GSKeyBindingActionQuoteNextKeyStroke *quoteAction;
    
    quoteAction = [[GSKeyBindingActionQuoteNextKeyStroke alloc] init];

    if (quoteKey == nil)
      {
        quoteKey = @"Control-q";
      }

    [_rootBindingTable bindKey: quoteKey  toAction: quoteAction];
    RELEASE (quoteAction);
  }
  

  /* Normally, when we start up, we load all the keybindings we find
     in the following files, in this order:

     $GNUSTEP_SYSTEM_ROOT/Library/KeyBindings/DefaultKeyBindings.dict
     $GNUSTEP_LOCAL_ROOT/Library/KeyBindings/DefaultKeyBindings.dict
     $GNUSTEP_NETWORK_ROOT/Library/KeyBindings/DefaultKeyBindings.dict
     $GNUSTEP_USER_ROOT/Library/KeyBindings/DefaultKeyBindings.dict
     
     This gives you a first way of adding your customized keybindings
     - adding a DefaultKeyBindings.dict to your GNUSTEP_USER_ROOT, and
     putting additional keybindings in there.  This allows you to add
     new keybindings to the standard ones, or override standard ones
     with your own.  These keybindings are normally used by all your
     applications (this is why they are in 'DefaultKeyBindings').

     You can change this behaviour, by setting the GSKeyBindingsFiles
     default to something else.  The GSKeyBindingsFiles default
     contains an array of files which is loaded, in that order.  Each
     file is searched first in GNUSTEP_SYSTEM_ROOT, then
     GNUSTEP_LOCAL_ROOT, the GNUSTEP_NETWORK_ROOT, then
     GNUSTEP_USER_ROOT.

     Examples - 

       GSKeyBindingsFiles = (DefaultKeyBindings, NicolaKeyBindings);

     will first load DefaultKeyBindings.dict (as by default), then
     NicolaKeyBindings.dict.

       GSKeyBindingsFiles = (NicolaKeyBindings);

     will not load DefaultKeyBindings.dict but only
     NicolaKeyBindings.dict.

     The default of course is 

       GSKeyBindingsFiles = (DefaultKeyBindings);

  */

  /* First, load the DefaultKeyBindings.  */
  {
    NSArray *keyBindingsFiles = [defaults arrayForKey: @"GSKeyBindingsFiles"];
    
    if (keyBindingsFiles == nil)
      {
        keyBindingsFiles = [NSArray arrayWithObject: @"DefaultKeyBindings"];
      }
    
    {
      Class string = [NSString class];
      unsigned int i;
    
      for (i = 0; i < [keyBindingsFiles count]; i++)
        {
          NSString *filename = [keyBindingsFiles objectAtIndex: i];
          
          if ([filename isKindOfClass: string])
            {
              [self loadBindingsWithName: filename];
            }
        }
    }
  }

  /* Then, load any manually specified keybinding.  */
  {
     NSDictionary *keyBindings = [defaults dictionaryForKey: @"GSKeyBindings"];

     if ([keyBindings isKindOfClass: [NSDictionary class]])
       {
         [_rootBindingTable loadBindingsFromDictionary: keyBindings];
       }
  }

  [pool drain];
  
  return self;
}

- (void) dealloc
{
  TEST_RELEASE (_pendingKeyEvents);
  RELEASE (_rootBindingTable);
  [super dealloc];
}

- (void) handleKeyboardEvents: (NSArray *)eventArray
                       client: (id)client
{
  NSEvent *theEvent;
  NSEnumerator *eventEnum = [eventArray objectEnumerator];

  /* If the client has changed, reset our internal state before going
     on.  */
  if (client != _currentClient)
    {
      [self resetInternalState];
    }

  _currentClient = client;

  while ((theEvent = [eventEnum nextObject]) != nil)
    {
      NSString *characters = [theEvent characters];
      NSString *unmodifiedCharacters = [theEvent charactersIgnoringModifiers];
      unichar character = 0;
      NSUInteger flags = [theEvent modifierFlags] & (NSShiftKeyMask 
                                                   | NSAlternateKeyMask 
                                                   | NSControlKeyMask
                                                   | NSNumericPadKeyMask);
      BOOL isFunctionKey = ([theEvent modifierFlags] & NSFunctionKeyMask) 
        == NSFunctionKeyMask;

      if ([unmodifiedCharacters length] > 0)
        {
          character = [unmodifiedCharacters characterAtIndex: 0];
        }

      if (!_interpretNextKeyStrokeLiterally)
        {
          GSKeyBindingAction *action;
          GSKeyBindingTable *table;
          BOOL found;
          unsigned adaptedFlags;

          /* If the keystroke is a function key, then we need to use
           * the full modifier flags to compare it against stored
           * keybindings, so that we can make a difference for example
           * between Shift-LeftArrow and LeftArrow.  But if it's not a
           * function key, then we should ignore the shift modifier -
           * for example Control-g is a keystroke, and Control-G is
           * another one.  The shift modifier flag is not used to
           * match these keystrokes - the fact that it's 'G' rather
           * than 'g' already contains the fact that it's typed in
           * with Shift.  */
          if (!isFunctionKey)
            {
              adaptedFlags = flags & (~NSShiftKeyMask); 
            }
          else
            {
              adaptedFlags = flags;
            }

          /* Special keybinding recognized in all contexts - abort -
             normally bound to Control-g.  The user is confused and
             wants to go home.  Abort whatever train of thoughts we
             were following, discarding whatever pending keystrokes we
             have, and return into default state.  */
          if (character == _abortCharacter  &&  adaptedFlags == _abortFlags)
            {
              [self resetInternalState];
              break;
            }

          /* Look up the character in the current keybindings table.  */
          found = [_currentBindingTable lookupKeyStroke: character
                                        modifiers: adaptedFlags
                                        returningActionIn: &action
                                        tableIn: &table];
          
          if (found)
            {
              if (action != nil)
                {
                  /* First reset our internal state - we are done
                     interpreting this keystroke sequence.  */
                  [self resetInternalState];
                  
                  /* Then perform the action.  The action might actually
                     modify our internal state, which is why we reset it
                     before calling the action! (for example, performing
                     the action might cause us to interpret the next
                     keystroke literally).  */
                  [action performActionWithInputManager: self];
                  break;
                }
              else if (table != nil)
                {
                  /* It's part of a composite multi-stroke
                     keybinding.  */
                  _currentBindingTable = table;
                  [_pendingKeyEvents addObject: theEvent];
                  break;
                }
              /* Else it is as if we didn't find it! */
            }
          
          /* Ok - the keybinding wasn't found.  If we were tracking a
             multi-stroke keybinding, it means we were on a false
             track.  */
          if ([_pendingKeyEvents count] > 0)
            {
              NSEvent *e;

              /* Save the pending events locally in this stack
                 frame.  */
              NSMutableArray *a = _pendingKeyEvents;
              RETAIN (a);
              
              /* Reset our internal state.  */
              [self resetInternalState];
              
              /* Take the very first event we received and which we
                 tried to interpret as a key binding, which now we
                 know was the wrong thing to do.  */              
              e = [a objectAtIndex: 0];

              /* Interpret it literally, since interpreting it as a
                 keybinding failed.  */
              _interpretNextKeyStrokeLiterally = YES;
              [self handleKeyboardEvents: [NSArray arrayWithObject: e]
                    client: client];

              /* Now feed the remaining pending key events to
                 ourselves for interpretation - again from
                 scratch.  */
              [a removeObjectAtIndex: 0];
              [a addObject: theEvent];
              
              [self handleKeyboardEvents: a
                    client: client];

              RELEASE (a);
              break;
            }          
        }
      
      /* We couldn't (or shouldn't) find the keybinding ... perform
         the default action - literally interpreting the
         keystroke.  */

      /* If this was a forced literal interpretation, make sure the
         next one is interpreted normally.  */
      _interpretNextKeyStrokeLiterally = NO;

      /* During literal interpretation, function keys are ignored.
         Trying to insert 'PageUp' literally makes simply no sense.  */
      if (isFunctionKey)
        {
          NSBeep ();
          break;
        }

      /* During literal interpretation, control characters are ignored
         if GSInsertControlKeystrokes was NO.  */
      if (_insertControlKeystrokes == NO)
        {
          if (flags & NSControlKeyMask)
            {
              NSBeep ();
              break;
            }
        }

      switch (character)
        {
        case NSDeleteCharacter:
        case NSBackspaceCharacter:
          [self doCommandBySelector: @selector (deleteBackward:)];
          break;
          
        case '\e': // escape
          [self doCommandBySelector: @selector (cancelOperation:)];
          break;

        case NSBackTabCharacter:
          [self doCommandBySelector: @selector (insertBacktab:)];
          break;

        case NSTabCharacter:
          if (flags & NSShiftKeyMask)
            {
              [self doCommandBySelector: @selector (insertBacktab:)];
            }
          else
            {
              [self doCommandBySelector: @selector (insertTab:)];
            }
          break;
          
        case NSEnterCharacter:
        case NSFormFeedCharacter:
        case NSCarriageReturnCharacter:
          if (flags & NSAlternateKeyMask)
            {
              [self doCommandBySelector: @selector (insertNewlineIgnoringFieldEditor:)];
            }
          else
            {
              [self doCommandBySelector: @selector (insertNewline:)];
            }
          break;
          
        case NSHelpFunctionKey:
          [NSHelpManager setContextHelpModeActive: YES];
          break;

        default:
          [self insertText: characters];
          break;
        }
    }
}

- (void) resetInternalState
{
  _currentBindingTable = _rootBindingTable;
  ASSIGN (_pendingKeyEvents, [NSMutableArray array]);
  _interpretNextKeyStrokeLiterally = NO;
}

- (void) quoteNextKeyStroke
{
  _interpretNextKeyStrokeLiterally = YES;
}

- (BOOL) handleMouseEvent: (NSEvent *)theMouseEvent
{
  return NO;
}

- (NSString *) language
{
  return @"English";
}


- (NSString *) localizedInputManagerName
{
  return nil;
}

- (void) markedTextAbandoned: (id)client
{}

- (void) markedTextSelectionChanged: (NSRange)newSel
                             client: (id)client
{}

- (BOOL) wantsToDelayTextChangeNotifications
{
  return NO;
}

- (BOOL) wantsToHandleMouseEvents
{
  return NO;
}

- (BOOL) wantsToInterpretAllKeystrokes
{
  return NO;
}

- (void) setMarkedText: (id)aString 
         selectedRange: (NSRange)selRange
{}

- (BOOL) hasMarkedText
{
  return NO;
}

- (NSRange) markedRange
{
  return NSMakeRange (NSNotFound, 0);
}

- (NSRange) selectedRange
{
  return NSMakeRange (NSNotFound, 0);
}

- (void) unmarkText
{}

- (NSArray*) validAttributesForMarkedText
{
  return nil;
}

- (NSAttributedString *) attributedSubstringFromRange: (NSRange)theRange
{
  return nil;
}

- (NSUInteger) characterIndexForPoint: (NSPoint)thePoint
{
  return 0;
}

- (NSInteger) conversationIdentifier
{
  return 0;
}

- (void) doCommandBySelector: (SEL)aSelector
{
  [_currentClient doCommandBySelector: aSelector];
}

- (NSRect) firstRectForCharacterRange: (NSRange)theRange
{
  return NSZeroRect;
}

- (void) insertText: (id)aString
{
  [_currentClient insertText: aString];
}

@end
