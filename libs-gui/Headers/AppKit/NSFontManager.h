/* 
   NSFontManager.h

   Manages system and user fonts

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Modified:  Fred Kiefer <FredKiefer@gmx.de>
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

#ifndef _GNUstep_H_NSFontManager
#define _GNUstep_H_NSFontManager
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSString;
@class NSArray;
@class NSDictionary;
@class NSMutableDictionary;

@class NSFont;
@class NSFontDescriptor;
@class NSMenu;
@class NSFontPanel;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum _NSFontManagerAddCollectionOptions
{
	NSFontCollectionApplicationOnlyMask = 1 << 0
};
#endif

typedef unsigned int NSFontTraitMask;

enum {
  NSItalicFontMask = 1,
  NSBoldFontMask = 2,
  NSUnboldFontMask = 4,
  NSNonStandardCharacterSetFontMask = 8,
  NSNarrowFontMask = 16,
  NSExpandedFontMask = 32,
  NSCondensedFontMask = 64,
  NSSmallCapsFontMask = 128,
  NSPosterFontMask = 256,
  NSCompressedFontMask = 512,
  NSFixedPitchFontMask = 1024,
  NSUnitalicFontMask = 1 << 24
};

typedef enum {
  NSNoFontChangeAction,
  NSViaPanelFontAction,
  NSAddTraitFontAction,
  NSRemoveTraitFontAction,
  NSSizeUpFontAction,
  NSSizeDownFontAction,
  NSHeavierFontAction,
  NSLighterFontAction,
} NSFontTag;

@interface NSFontManager : NSObject
{
  // Attributes
  id _delegate;
  SEL _action;
  NSFont *_selectedFont;
  BOOL _multiple;
  NSMenu *_fontMenu;
  NSFontTag _storedTag;
  NSFontTraitMask _trait;
  id _fontEnumerator;
  NSDictionary *_selectedAttributes;
  NSMutableDictionary *_collections;
}

//
// Managing the FontManager
//
+ (void) setFontManagerFactory: (Class)aClass;
+ (void) setFontPanelFactory: (Class)aClass;
+ (NSFontManager *)sharedFontManager;

//
// information on available fonts
//
- (NSArray *)availableFonts;
- (NSArray *)availableFontFamilies;
- (NSArray *)availableFontNamesWithTraits:(NSFontTraitMask)fontTraitMask;
- (NSArray *)availableMembersOfFontFamily:(NSString *)family;
- (NSString *) localizedNameForFamily:(NSString *)family face:(NSString *)face;

//
// Selecting fonts
//
- (void)setSelectedFont:(NSFont *)fontObject
	     isMultiple:(BOOL)flag;
- (NSFont *)selectedFont;
- (BOOL)isMultiple;
- (BOOL)sendAction;

//
// Action methods
//
- (void)addFontTrait:(id)sender;
- (void)removeFontTrait:(id)sender;
- (void)modifyFont:(id)sender;
- (void)modifyFontViaPanel:(id)sender;

//
//Automatic font conversion
//
- (NSFont *)convertFont:(NSFont *)fontObject;

//
// Converting Fonts
//
- (NSFont *)convertFont:(NSFont *)fontObject
	       toFamily:(NSString *)family;
- (NSFont *)convertFont:(NSFont *)fontObject
		 toFace:(NSString *)typeface;
- (NSFont *)convertFont:(NSFont *)fontObject
	    toHaveTrait:(NSFontTraitMask)trait;
- (NSFont *)convertFont:(NSFont *)fontObject
	 toNotHaveTrait:(NSFontTraitMask)trait;
- (NSFont *)convertFont:(NSFont *)fontObject
		 toSize:(float)size;
- (NSFont *)convertWeight:(BOOL)upFlag
		   ofFont:(NSFont *)fontObject;

//
// Getting a font
//
- (NSFont *)fontWithFamily:(NSString *)family
		    traits:(NSFontTraitMask)traits
		    weight:(int)weight
		      size:(float)size;
//
// Examining a font
//
- (NSFontTraitMask)traitsOfFont:(NSFont *)aFont;
- (int)weightOfFont:(NSFont *)fontObject;
- (BOOL)fontNamed:(NSString *)typeface 
        hasTraits:(NSFontTraitMask)fontTraitMask;

//
// Enabling
//
- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)flag;

//
// Font menu
//
- (NSMenu *)fontMenu:(BOOL)create;
- (void)setFontMenu:(NSMenu *)newMenu;

//
// Font panel
//
- (NSFontPanel *)fontPanel:(BOOL)create;
- (void)orderFrontFontPanel:(id)sender;

//
// Assigning a Delegate
//
- (id)delegate;
- (void)setDelegate:(id)anObject;

//
// Action Methods
//
- (SEL)action;
- (void)setAction:(SEL)aSelector;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL)addCollection:(NSString *)name options:(int)options;
- (BOOL)removeCollection:(NSString *)collection;
- (NSArray *)collectionNames;

- (void)addFontDescriptors:(NSArray *)descriptors 
              toCollection:(NSString *)collection;
- (void)removeFontDescriptor:(NSFontDescriptor *)descriptor 
              fromCollection:(NSString *)collection;
- (NSArray *)fontDescriptorsInCollection:(NSString *)collection;

- (NSArray *)availableFontNamesMatchingFontDescriptor:(NSFontDescriptor *)descriptor;
- (NSDictionary *)convertAttributes:(NSDictionary *)attributes;
- (void)setSelectedAttributes:(NSDictionary *)attributes 
                   isMultiple:(BOOL)flag;
#endif
@end

@interface NSObject (NSFontManagerDelegate)
//
// Methods Implemented by the Delegate
//
- (BOOL)fontManager:(id)sender willIncludeFont:(NSString *)fontName;
@end

#endif // _GNUstep_H_NSFontManager


