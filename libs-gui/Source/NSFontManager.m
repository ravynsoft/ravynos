/** <title>NSFontManager</title>

   <abstract>Manages system and user fonts</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: January 2000
   Almost complete rewrite.
   
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

#include "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDebug.h>
#import "AppKit/NSFontDescriptor.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSFontPanel.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItem.h"
#import "GNUstepGUI/GSFontInfo.h"


/*
 * Class variables
 */
static NSFontManager *sharedFontManager = nil;
static NSFontPanel   *fontPanel = nil;
static Class         fontManagerClass = Nil;
static Class         fontPanelClass = Nil;


@implementation NSFontManager

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSFontManager class])
    {
      // Initial version
      [self setVersion: 1];

      // Set the factories
      [self setFontManagerFactory: [NSFontManager class]];
      [self setFontPanelFactory: [NSFontPanel class]];
    }
}

/**<p>Sets the class used to create the NSFontManager to <var>aClass</var>.
   By default it is a NSFontManager class. You can change this behavour by
   implementing your own class ( a subclass of NSFontManager ) </p>
   <p>This class is init into +sharedFontManager</p>
   <p>See Also: +sharedFontManager</p>
 */
+ (void) setFontManagerFactory: (Class)aClass
{
  fontManagerClass = aClass;
}

/**<p>Sets the class used to create a NSFontPanel. If you want to use
   a custom class it should be NSFontPanel subclass</p>
   <p>See Also: -fontPanel:</p>
 */
+ (void) setFontPanelFactory: (Class)aClass
{
  fontPanelClass = aClass;
}

/**<p>Creates ( if needed ) and returns the NSFontManager shared instance.
   This method init the font manager class defined in +setFontManagerFactory:
   ( it is usally a NSFontManager class ) </p>
 */
+ (NSFontManager*) sharedFontManager
{
  if (!sharedFontManager)
    {
      sharedFontManager = [[fontManagerClass alloc] init];
    }
  return sharedFontManager;
}

/*
 * Instance methods
 */
- (id) init
{
  if (sharedFontManager && self != sharedFontManager)
    {
      RELEASE(self);
      return sharedFontManager;
    }
  self = [super init];

  _action = @selector(changeFont:);
  _storedTag = NSNoFontChangeAction;
  _fontEnumerator = RETAIN([GSFontEnumerator sharedEnumerator]);
  _collections = [[NSMutableDictionary alloc] initWithCapacity: 3];

  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_selectedFont);
  TEST_RELEASE(_selectedAttributes);
  TEST_RELEASE(_fontMenu);
  TEST_RELEASE(_fontEnumerator);
  RELEASE(_collections);
  [super dealloc];
}

/**<p>Returns an array of available fonts.</p>
 */
- (NSArray*) availableFonts
{
  return [_fontEnumerator availableFonts];
}

- (NSArray*) availableFontFamilies
{
  return [_fontEnumerator availableFontFamilies];
}

- (NSArray*) availableFontNamesWithTraits: (NSFontTraitMask)fontTraitMask
{
  unsigned int i, j;
  NSArray *fontFamilies = [self availableFontFamilies];
  NSMutableArray *fontNames = [NSMutableArray array];
  NSFontTraitMask traits;

  if (fontTraitMask == (NSUnitalicFontMask | NSUnboldFontMask))
    {
      fontTraitMask = 0;
    }

  for (i = 0; i < [fontFamilies count]; i++)
    {
      NSArray *fontDefs = [self availableMembersOfFontFamily: 
                                 [fontFamilies objectAtIndex: i]];
      
      for (j = 0; j < [fontDefs count]; j++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: j];

          traits = [[fontDef objectAtIndex: 3] unsignedIntValue];
          // Check if the font has exactly the given mask
          if (traits == fontTraitMask)
            [fontNames addObject: [fontDef objectAtIndex: 0]];
        }
    }

  return fontNames;
}

- (NSArray*) availableMembersOfFontFamily: (NSString*)family
{
  return [_fontEnumerator availableMembersOfFontFamily: family];
}

- (NSArray *) availableFontNamesMatchingFontDescriptor: (NSFontDescriptor *)descriptor
{
  return [_fontEnumerator availableFontNamesMatchingFontDescriptor: descriptor];
}

// GNUstep extension
- (NSArray *) matchingFontDescriptorsFor: (NSDictionary *)attributes
{
  return [_fontEnumerator matchingFontDescriptorsFor: attributes];
}

- (NSString*) localizedNameForFamily: (NSString*)family 
                                face: (NSString*)face
{
  // TODO
  return [NSString stringWithFormat: @"%@-%@", family, face];
}

/**
 */
- (void) setSelectedFont: (NSFont*)fontObject
              isMultiple: (BOOL)flag
{
  if (_selectedFont == fontObject)
    {
      if (flag != _multiple)
        {
          _multiple = flag;
          // The panel should also know if multiple changed
          if (fontPanel != nil)
            {
              [fontPanel setPanelFont: fontObject isMultiple: flag];
            }
        }
      return;
    }

  _multiple = flag;
  ASSIGN(_selectedFont, fontObject);
  DESTROY(_selectedAttributes);

  if (fontPanel != nil)
    {
      [fontPanel setPanelFont: fontObject isMultiple: flag];
    }
  
  if (_fontMenu != nil)
    {
      id <NSMenuItem> menuItem;
      NSFontTraitMask trait = [self traitsOfFont: fontObject];

      /*
       * FIXME: We should check if that trait is available
       * We keep the tag, to mark the item
       */
      if (trait & NSItalicFontMask)
        {
          menuItem = [_fontMenu itemWithTag: NSItalicFontMask];
          if (menuItem != nil)
            {
              [menuItem setTitle: @"Unitalic"];
              [menuItem setAction: @selector(removeFontTrait:)];
            }
        }
      else
        {
          menuItem = [_fontMenu itemWithTag: NSItalicFontMask];
          if (menuItem != nil)
            {
              [menuItem setTitle: @"Italic"];
              [menuItem setAction: @selector(addFontTrait:)];
            }
        }

      if (trait & NSBoldFontMask)
        {
          menuItem = [_fontMenu itemWithTag: NSBoldFontMask];
          if (menuItem != nil)
            {
              [menuItem setTitle: @"Unbold"];
              [menuItem setAction: @selector(removeFontTrait:)];
            }
        }
      else
        {
          menuItem = [_fontMenu itemWithTag: NSBoldFontMask];
          if (menuItem != nil)
            {
              [menuItem setTitle: @"Bold"];
              [menuItem setAction: @selector(addFontTrait:)];
            }
        }

      // TODO Update the rest of the font menu to reflect this font
    }
}

/**<p>Returns the selected font</p>
   <p>See Also: -setSelectedFont:isMultiple:</p>
 */
- (NSFont*) selectedFont
{
  return _selectedFont;
}

/**<p>Returns whether the current selection contains multiple fonts</p>
 */
- (BOOL) isMultiple
{
  return _multiple;
}

/*
 * Action methods
 */
- (void) addFontTrait: (id)sender
{
  _storedTag = NSAddTraitFontAction;
  _trait = [sender tag];
  [self sendAction];

  // We update our own selected font
  if (_selectedFont != nil)
    {
      NSFont *newFont = [self convertFont: _selectedFont];

      if (newFont != nil)
        {
          [self setSelectedFont: newFont isMultiple: _multiple];
        }
    }
}

- (void) removeFontTrait: (id)sender
{
  _storedTag = NSRemoveTraitFontAction;
  _trait = [sender tag];
  [self sendAction];

  // We update our own selected font
  if (_selectedFont != nil)
    {
      NSFont *newFont = [self convertFont: _selectedFont];

      if (newFont != nil)
        {
          [self setSelectedFont: newFont isMultiple: _multiple];
        }
    }
}

- (void) modifyFont: (id)sender
{
  _storedTag = [sender tag];
  [self sendAction];

  // We update our own selected font
  if (_selectedFont != nil)
    {
      NSFont *newFont = [self convertFont: _selectedFont];

      if (newFont != nil)
        {
          [self setSelectedFont: newFont isMultiple: _multiple];
        }
    }
}

- (void) modifyFontViaPanel: (id)sender
{
  _storedTag = NSViaPanelFontAction;
  [self sendAction];

  // We update our own selected font
  if (_selectedFont != nil)
    {
      NSFont *newFont = [self convertFont: _selectedFont];

      if (newFont != nil)
        {
          [self setSelectedFont: newFont isMultiple: _multiple];
        }
    }
}

/**<p>Converts the NSFont <var>fontObject</var> according to user changes
   in the Font panel or the font menu</p>
   <p>See Also: -addFontTrait: -removeFontTrait: -modifyFont: 
   -modifyFontViaPanel: -convertFont:toHaveTrait: -convertFont:toNotHaveTrait:
   -convertFont:toSize: -convertFont:toFamily: -convertWeight:ofFont:</p>
 */
- (NSFont*) convertFont: (NSFont*)fontObject
{
  NSFont *newFont = fontObject;
  int i;
  float size;
  float sizes[] = {4.0, 6.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 
                   14.0, 16.0, 18.0, 24.0, 36.0, 48.0, 64.0};

  if (fontObject == nil)
    return nil;

  switch (_storedTag)
    {
      case NSNoFontChangeAction: 
        break;
      case NSViaPanelFontAction: 
        if (fontPanel != nil)
          {
            newFont = [fontPanel panelConvertFont: fontObject];
          }
        break;
      case NSAddTraitFontAction: 
        newFont = [self convertFont: fontObject toHaveTrait: _trait];
        break;
      case NSRemoveTraitFontAction: 
         newFont = [self convertFont: fontObject toNotHaveTrait: _trait];
        break;
      case NSSizeUpFontAction: 
        size = [fontObject pointSize];
        for (i = 0; i < sizeof(sizes)/sizeof(float); i++)
          {
            if (sizes[i] > size)
              {
                size = sizes[i];
                break;
              }
          }
        newFont = [self convertFont: fontObject 
                        toSize: size];
        break;
      case NSSizeDownFontAction: 
        size = [fontObject pointSize];
        for (i = sizeof(sizes)/sizeof(float) -1; i >= 0; i--)
          {
            if (sizes[i] < size)
              {
                size = sizes[i];
                break;
              }
          }
        newFont = [self convertFont: fontObject 
                        toSize: size];
        break;
      case NSHeavierFontAction: 
        newFont = [self convertWeight: YES ofFont: fontObject]; 
        break;
      case NSLighterFontAction: 
        newFont = [self convertWeight: NO ofFont: fontObject]; 
        break;
    }

  return newFont;
}


- (NSFont*) convertFont: (NSFont*)fontObject
               toFamily: (NSString*)family
{
  if ([family isEqualToString: [fontObject familyName]])
    {
      // If already of that family then just return it
      return fontObject;
    }
  else
    {
      // Else convert it
      NSFont *newFont;
      NSFontTraitMask trait = [self traitsOfFont: fontObject];
      int weight = [self weightOfFont: fontObject];
      float size = [fontObject pointSize];

      newFont = [self fontWithFamily: family 
                              traits: trait
                              weight: weight
                                size: size];
      if (newFont == nil)
        return fontObject;
      else 
        return newFont;
    }
}

- (NSFont*) convertFont: (NSFont*)fontObject
                 toFace: (NSString*)typeface
{
  NSFont *newFont;

  // This conversion just retains the point size
  if ([[fontObject fontName] isEqualToString: typeface])
    {
      return fontObject;
    }

  newFont = [NSFont fontWithName: typeface size: [fontObject pointSize]];
  if (newFont == nil)
    return fontObject;
  else 
    return newFont;
}

- (NSFont*) convertFont: (NSFont*)fontObject
            toHaveTrait: (NSFontTraitMask)trait
{
  NSFontTraitMask t = [self traitsOfFont: fontObject];

  if (t & trait)
    {
      // If already have that trait then just return it
      return fontObject;
    }
  else
    {
      // Else convert it
      NSFont *newFont;

      int weight = [self weightOfFont: fontObject];
      float size = [fontObject pointSize];
      NSString *family = [fontObject familyName];

      if (trait & NSBoldFontMask)
        {
          // We cannot reuse the weight in a bold
          weight = 9;
          t = t & ~NSUnboldFontMask;
        }
      else if (trait & NSUnboldFontMask)
        {
          // We cannot reuse the weight in an unbold
          weight = 5;
          t = t & ~NSBoldFontMask;
        }
      if (trait == NSItalicFontMask)
        {
          t = t & ~NSUnitalicFontMask;
        }
      else if (trait & NSUnitalicFontMask)
        {
          t = t & ~NSItalicFontMask;
        }

      t = t | trait;

      newFont = [self fontWithFamily: family 
                              traits: t
                              weight: weight
                                size: size];

      if (newFont == nil)
        return fontObject;
      else 
        return newFont;
    }
}

- (NSFont*) convertFont: (NSFont*)fontObject
         toNotHaveTrait: (NSFontTraitMask)trait
{
  NSFontTraitMask t = [self traitsOfFont: fontObject];

  if (!(t & trait))
    {
      // If already do not have that trait then just return it
      return fontObject;
    }
  else
    {
      // Else convert it
      NSFont *newFont;

      int weight = [self weightOfFont: fontObject];
      float size = [fontObject pointSize];
      NSString *family = [fontObject familyName];

      if (trait & NSBoldFontMask)
        {
          // We cannot reuse the weight in an unbold
          weight = 5;
          t = (t | NSUnboldFontMask);
        }
      else if (trait & NSUnboldFontMask)
        {
          // We cannot reuse the weight in a bold
          weight = 9;
          t = (t | NSBoldFontMask);
        }
      if (trait & NSItalicFontMask)
        {
          t = (t | NSUnitalicFontMask);
        }
      else if (trait & NSUnitalicFontMask)
        {
          t = (t | NSItalicFontMask);
        }
 
      t &= ~trait;
      newFont = [self fontWithFamily: family 
                              traits: t
                              weight: weight
                                size: size];
      if (newFont == nil)
        return fontObject;
      else 
        return newFont;
    }
}

- (NSFont*) convertFont: (NSFont*)fontObject
                 toSize: (float)size
{
  if ([fontObject pointSize] == size)
    {
      // If already that size then just return it
      return fontObject;
    }
  else
    {
      // Else convert it
      NSFont *newFont;

      newFont = [NSFont fontWithName: [fontObject fontName] 
                        size: size];
      if (newFont == nil)
        return fontObject;
      else 
        return newFont;
    }
}

- (NSFont*) convertWeight: (BOOL)upFlag
                   ofFont: (NSFont*)fontObject
{
  NSFont *newFont = nil;
  NSString *fontName = nil;
  NSFontTraitMask trait = [self traitsOfFont: fontObject];
  float size = [fontObject pointSize];
  NSString *family = [fontObject familyName];
  int w = [self weightOfFont: fontObject];
  // We check what weights we have for this family. We must
  // also check to see if that font has the correct traits!
  NSArray *fontDefs = [self availableMembersOfFontFamily: family];

  if (upFlag)
    {
      unsigned int i;
      // The documentation is a bit unclear about the range of weights
      // sometimes it says 0 to 9 and sometimes 0 to 15
      int next_w = 15;

      for (i = 0; i < [fontDefs count]; i++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: i];
          int w1 = [[fontDef objectAtIndex: 2] intValue];

          if (w1 > w && w1 < next_w && 
              [[fontDef objectAtIndex: 3] unsignedIntValue] == trait)
            {
              next_w = w1;
              fontName = [fontDef objectAtIndex: 0];
            }
        }

      if (fontName == nil)
        {
          // Not found, try again with changed trait
          trait |= NSBoldFontMask;
          
          for (i = 0; i < [fontDefs count]; i++)
            { 
              NSArray *fontDef = [fontDefs objectAtIndex: i];
              int w1 = [[fontDef objectAtIndex: 2] intValue];

              if (w1 > w && w1 < next_w && 
                  [[fontDef objectAtIndex: 3] unsignedIntValue] == trait)
                {
                  next_w = w1;
                  fontName = [fontDef objectAtIndex: 0];
                }
            }
        }
    }
  else
    {
      unsigned int i;
      int next_w = 0;

      for (i = 0; i < [fontDefs count]; i++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: i];
          int w1 = [[fontDef objectAtIndex: 2] intValue];

          if (w1 < w && w1 > next_w
            && [[fontDef objectAtIndex: 3] unsignedIntValue] == trait)
            {
              next_w = w1;
              fontName = [fontDef objectAtIndex: 0];
            }
        }

      if (fontName == nil)
        {
          // Not found, try again with changed trait
          trait &= ~NSBoldFontMask;

          for (i = 0; i < [fontDefs count]; i++)
            {
              NSArray *fontDef = [fontDefs objectAtIndex: i];
              int w1 = [[fontDef objectAtIndex: 2] intValue];
              
              if (w1 < w && w1 > next_w
                && [[fontDef objectAtIndex: 3] unsignedIntValue] == trait)
                {
                  next_w = w1;
                  fontName = [fontDef objectAtIndex: 0];
                }
            }
        }
    }

  if (fontName != nil)
    {
      newFont = [NSFont fontWithName: fontName
                                size: size];
    }
  if (newFont == nil)
    return fontObject;
  else 
    return newFont;
}

/*
 * Getting a font
 */
- (NSFont*) fontWithFamily: (NSString*)family
                    traits: (NSFontTraitMask)traits
                    weight: (int)weight
                      size: (float)size
{
  NSArray *fontDefs = [self availableMembersOfFontFamily: family];
  unsigned int i;

  //NSLog(@"Searching font %@: %i: %i size %.0f", family, weight, traits, size);

  // First do an exact match search
  for (i = 0; i < [fontDefs count]; i++)
    {
      NSArray *fontDef = [fontDefs objectAtIndex: i];

      //NSLog(@"Testing font %@: %i: %i", [fontDef objectAtIndex: 0], 
      //          [[fontDef objectAtIndex: 2] intValue], 
      //          [[fontDef objectAtIndex: 3] unsignedIntValue]);  
      if (([[fontDef objectAtIndex: 2] intValue] == weight) &&
          ([[fontDef objectAtIndex: 3] unsignedIntValue] == traits))
        {
            //NSLog(@"Found font");
          return [NSFont fontWithName: [fontDef objectAtIndex: 0] 
                         size: size];
        }
    }

  // Try to find something close by ignoring some trait flags
  traits &= ~(NSNonStandardCharacterSetFontMask | NSFixedPitchFontMask
              | NSUnitalicFontMask | NSUnboldFontMask);
  for (i = 0; i < [fontDefs count]; i++)
    {
      NSArray *fontDef = [fontDefs objectAtIndex: i];
      NSFontTraitMask t = [[fontDef objectAtIndex: 3] unsignedIntValue];

      t &= ~(NSNonStandardCharacterSetFontMask | NSFixedPitchFontMask
             | NSUnitalicFontMask | NSUnboldFontMask);
      if (([[fontDef objectAtIndex: 2] intValue] == weight) &&
          (t == traits))
        {
            //NSLog(@"Found font");
          return [NSFont fontWithName: [fontDef objectAtIndex: 0] 
                         size: size];
        }
    }

  if (traits & NSBoldFontMask)
    {
      //NSLog(@"Trying ignore weights for bold font");
      for (i = 0; i < [fontDefs count]; i++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: i];
          NSFontTraitMask t = [[fontDef objectAtIndex: 3] unsignedIntValue];

          t &= ~(NSNonStandardCharacterSetFontMask | NSFixedPitchFontMask
                 | NSUnitalicFontMask | NSUnboldFontMask);
          if (t == traits)
            {
              //NSLog(@"Found font");
              return [NSFont fontWithName: [fontDef objectAtIndex: 0] 
                             size: size];
            }
        }
    }
  
  if (weight == 5 || weight == 6)
    {
      //NSLog(@"Trying alternate non-bold weights for non-bold font");
      for (i = 0; i < [fontDefs count]; i++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: i];
          NSFontTraitMask t = [[fontDef objectAtIndex: 3] unsignedIntValue];

          t &= ~(NSNonStandardCharacterSetFontMask | NSFixedPitchFontMask
                 | NSUnitalicFontMask | NSUnboldFontMask);
          if ((([[fontDef objectAtIndex: 2] intValue] == 5) ||
               ([[fontDef objectAtIndex: 2] intValue] == 6)) &&
              (t == traits))
            {
              //NSLog(@"Found font");
              return [NSFont fontWithName: [fontDef objectAtIndex: 0] 
                             size: size];
            }
        }
    }

  //NSLog(@"Didnt find font");  
  return nil;
}

//
// Examining a font
//
- (NSFontTraitMask) traitsOfFont: (NSFont*)aFont
{
  return [[aFont fontInfo] traits];
}

/**<p>Returns the weight of the NSFont <var>fontObject</var></p>
 */
- (int) weightOfFont: (NSFont*)fontObject
{
  return [[fontObject fontInfo] weight];
}

- (BOOL) fontNamed: (NSString*)typeface 
         hasTraits: (NSFontTraitMask)fontTraitMask
{
  // TODO: This method is implemented very slow, but I dont 
  // see any use for it, so why change it?
  unsigned int i, j;
  NSArray *fontFamilies = [self availableFontFamilies];
  NSFontTraitMask traits;
  
  for (i = 0; i < [fontFamilies count]; i++)
    {
      NSArray *fontDefs = [self availableMembersOfFontFamily: 
                                  [fontFamilies objectAtIndex: i]];
      
      for (j = 0; j < [fontDefs count]; j++)
        {
          NSArray *fontDef = [fontDefs objectAtIndex: j];
          
          if ([[fontDef objectAtIndex: 0] isEqualToString: typeface])
            {
              traits = [[fontDef objectAtIndex: 3] unsignedIntValue];
              // FIXME: This is not exactly the right condition
              if ((traits & fontTraitMask) == fontTraitMask)
                {
                  return YES;
                }
              else
                return NO;
            }
        }
    }
  
  return NO;
}

/**<p>Returns whether the NSFontPanel is enabled ( if exists )</p> 
 */
- (BOOL) isEnabled
{
  if (fontPanel != nil)
    {
      return [fontPanel isEnabled];
    }
  else
    return NO;
}

/**<p>Enables/disables the NSFontPanel and the font menu ( if they exist )</p> 
   <p>See Also: -isEnabled</p>
 */
- (void) setEnabled: (BOOL)flag
{
  int i;

  if (_fontMenu != nil)
    {
      for (i = 0; i < [_fontMenu numberOfItems]; i++)
        {
          [[_fontMenu itemAtIndex: i] setEnabled: flag];
        }
    }

  if (fontPanel != nil)
    [fontPanel setEnabled: flag];
}

/**<p>Returns the font menu, creates it (if needed ) if <var>create</var> 
   is YES.</p><p>See Also: -setFontMenu:</p>
 */
- (NSMenu*) fontMenu: (BOOL)create
{
  if (create && _fontMenu == nil)
    {
      id <NSMenuItem> menuItem;
      
      // As the font menu is stored in a instance variable we 
      // dont autorelease it
      _fontMenu = [NSMenu new];
      [_fontMenu setTitle: @"Font Menu"];

      // First an entry to start the font panel
      menuItem = [_fontMenu addItemWithTitle: @"Font Panel"
                            action: @selector(orderFrontFontPanel:)
                            keyEquivalent: @"t"];
      [menuItem setTarget: self];

      // Entry for italic
      menuItem = [_fontMenu addItemWithTitle: @"Italic"
                            action: @selector(addFontTrait:)
                            keyEquivalent: @"i"];
      [menuItem setTag: NSItalicFontMask];
      [menuItem setTarget: self];

      // Entry for bold
      menuItem = [_fontMenu addItemWithTitle: @"Bold"
                            action: @selector(addFontTrait:)
                            keyEquivalent: @"b"];
      [menuItem setTag: NSBoldFontMask];
      [menuItem setTarget: self];

      // Entry to increase weight
      menuItem = [_fontMenu addItemWithTitle: @"Heavier"
                            action: @selector(modifyFont:)
                            keyEquivalent: @""];
      [menuItem setTag: NSHeavierFontAction];
      [menuItem setTarget: self];
 
      // Entry to decrease weight
      menuItem = [_fontMenu addItemWithTitle: @"Lighter"
                            action: @selector(modifyFont:)
                            keyEquivalent: @""];
      [menuItem setTag: NSLighterFontAction];
      [menuItem setTarget: self];
 
      // Entry to increase size
      menuItem = [_fontMenu addItemWithTitle: @"Larger"
                            action: @selector(modifyFont:)
                            keyEquivalent: @"+"];
      [menuItem setTag: NSSizeUpFontAction];
      [menuItem setTarget: self];

      // Entry to decrease size
      menuItem = [_fontMenu addItemWithTitle: @"Smaller"
                            action: @selector(modifyFont:)
                            keyEquivalent: @"-"];
      [menuItem setTag: NSSizeDownFontAction];
      [menuItem setTarget: self];
    }
  return _fontMenu;
}

/**<p>Sets the font menu to <var>newMenu</var></p>
   <p>See Also: -fontMenu:</p>
 */
- (void) setFontMenu: (NSMenu*)newMenu
{
  ASSIGN(_fontMenu, newMenu); 
}

/**<p>Returns the NSFontPanel, creates it ( if needed ) if <var>create</var>
   is YES.</p>
   <p>See Also: +setFontPanelFactory:</p>
 */
- (NSFontPanel*) fontPanel: (BOOL)create
{
  if ((fontPanel == nil) && (create))
    {
      fontPanel = [[fontPanelClass alloc] init];
    }
  return fontPanel;
}

- (void) orderFrontFontPanel: (id)sender
{
  if (fontPanel == nil)
    fontPanel = [self fontPanel: YES];
  [fontPanel orderFront: sender];
}

/**<p>Returns the NSFontManager's delegate</p>
 */
- (id) delegate
{
  return _delegate;
}

/**<p>Sets the NSFontManager's delegate to <var>anObject</var></p>
 * FIXME: This is extremely unclear.  At the moment, the
 * NSFontManager's delegate is never used.  This can't be right.
 */
- (void) setDelegate: (id)anObject
{
  _delegate = anObject;
}

/** <p>Returns the action sents by the NSFontManager.</p>
    <p>See Also: -setAction:</p>
 */
- (SEL) action
{
  return _action;
}

/** <p>Sents the action sents by the NSFontManager to <var>aSelector</var>.</p>
    <p>See Also: -action</p>
 */
- (void) setAction: (SEL)aSelector
{
  _action = aSelector;
}

- (BOOL) sendAction
{
  NSApplication *theApp = [NSApplication sharedApplication];

  if (_action)
    {
      /* FIXME - shouldn't we try our own delegate first ??  It seems
       * what every programmer would expect, but it looks like Apple
       * doesn't do it!  Or maybe they fixed it in recent releases ?
       */
      return [theApp sendAction: _action to: nil from: self];
    }
  else
    {
      return NO;
    }
}

- (BOOL) addCollection: (NSString *)name options: (int)options
{
  [_collections setObject: [NSMutableArray arrayWithCapacity: 10] forKey: name];
  return YES;
}

- (BOOL) removeCollection:(NSString *) collection
{
  if ([_collections objectForKey: collection])
    {
      [_collections removeObjectForKey: collection];
      return YES;
    }
  else
    {
      return NO;
    }
}

- (NSArray *) collectionNames
{
  return [_collections allKeys];
}

- (void) addFontDescriptors: (NSArray *)descriptors 
               toCollection: (NSString *)collection
{
  NSMutableArray *a = [_collections objectForKey: collection];

  if (a)
    {
      [a addObjectsFromArray: descriptors];
    }
}

- (void) removeFontDescriptor: (NSFontDescriptor *)descriptor 
               fromCollection: (NSString *)collection
{
  NSMutableArray *a = [_collections objectForKey: collection];

  if (a)
    {
      [a removeObject: descriptor];
    }
}

- (NSArray *) fontDescriptorsInCollection: (NSString *)collection
{
  return [_collections objectForKey: collection];
}

- (NSDictionary *) convertAttributes: (NSDictionary *)attributes
{
  NSMutableDictionary *newAttributes;
  int i;
  float size;
  float sizes[] = {4.0, 6.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 
                   14.0, 16.0, 18.0, 24.0, 36.0, 48.0, 64.0};
  NSFontTraitMask t;

  if (attributes == nil)
    return nil;

  newAttributes = AUTORELEASE([attributes mutableCopy]);
  switch (_storedTag)
    {
      case NSNoFontChangeAction: 
        break;
      case NSViaPanelFontAction: 
        // FIXME
        break;
      case NSAddTraitFontAction: 
        t = [[attributes objectForKey: NSFontSymbolicTrait] unsignedIntValue];
        
        if (t & _trait)
          {
            return newAttributes;
          }
        else if (_trait == NSUnboldFontMask)
          {
            t &= ~NSBoldFontMask;
          }
        else if (_trait == NSUnitalicFontMask)
          {
            t &= ~NSItalicFontMask;
          }
        else
          {
            t &= _trait;
            // FIXME: What about weight for NSBoldFontMask?
          }
        [newAttributes setObject: [NSNumber numberWithUnsignedInt: t]
                       forKey: NSFontSymbolicTrait];
        break;
      case NSRemoveTraitFontAction: 
        t = [[attributes objectForKey: NSFontSymbolicTrait] unsignedIntValue];
        
        if (!(t & _trait))
          {
            return newAttributes;
          }
        else if (_trait == NSUnboldFontMask)
          {
            t = (t | NSBoldFontMask) & ~NSUnboldFontMask;
          }
        else if (_trait == NSUnitalicFontMask)
          {
            t = (t | NSItalicFontMask) & ~NSUnitalicFontMask;
          }
        else
          {
            t &= ~_trait;
            // FIXME: What about weight for NSBoldFontMask?
          }
        [newAttributes setObject: [NSNumber numberWithUnsignedInt: t]
                       forKey: NSFontSymbolicTrait];
        break;
      case NSSizeUpFontAction: 
        size = [[attributes objectForKey: NSFontSizeAttribute] floatValue];
        for (i = 0; i < sizeof(sizes)/sizeof(float); i++)
          {
            if (sizes[i] > size)
              {
                size = sizes[i];
                break;
              }
          }
        [newAttributes setObject: [NSString stringWithFormat: @"%f", size]
                       forKey: NSFontSizeAttribute];
        break;
      case NSSizeDownFontAction: 
        size = [[attributes objectForKey: NSFontSizeAttribute] floatValue];
        for (i = sizeof(sizes)/sizeof(float) -1; i >= 0; i--)
          {
            if (sizes[i] < size)
              {
                size = sizes[i];
                break;
              }
          }
        [newAttributes setObject: [NSString stringWithFormat: @"%f", size]
                       forKey: NSFontSizeAttribute];
        break;
      case NSHeavierFontAction: 
        // FIXME
        break;
      case NSLighterFontAction: 
        // FIXME
        break;
    }

  return newAttributes;
}

- (void) setSelectedAttributes: (NSDictionary *)attributes 
                    isMultiple: (BOOL)flag
{
  ASSIGN(_selectedAttributes, attributes);
  _multiple = flag;
  DESTROY(_selectedFont);
}

@end

@implementation NSApplication(NSFontPanel)

- (void) orderFrontFontPanel: (id)sender
{
  [[NSFontManager sharedFontManager] orderFrontFontPanel: sender];
}

@end
