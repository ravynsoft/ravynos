/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSFont, NSFontDescriptor, NSFontPanel;

typedef unsigned NSFontTraitMask;

typedef enum {
    NSNoFontChangeAction = 0,
    NSViaPanelFontAction = 1,
    NSAddTraitFontAction = 2,
    NSSizeUpFontAction = 3,
    NSSizeDownFontAction = 4,
    NSHeavierFontAction = 5,
    NSLighterFontAction = 6,
    NSRemoveTraitFontAction = 7,
} NSFontAction;

enum {
    NSItalicFontMask = 0x00000001,
    NSBoldFontMask = 0x00000002,
    NSUnboldFontMask = 0x00000004,
    NSNonStandardCharacterSetFontMask = 0x00000008,
    NSNarrowFontMask = 0x00000010,
    NSExpandedFontMask = 0x00000020,
    NSCondensedFontMask = 0x00000040,
    NSSmallCapsFontMask = 0x00000080,
    NSPosterFontMask = 0x00000100,
    NSCompressedFontMask = 0x00000200,
    NSFixedPitchFontMask = 0x00000400,
    NSUnitalicFontMask = 0x01000000,
};

@interface NSFontManager : NSObject {
    NSFontPanel *_panel;
    id _delegate;
    SEL _action;

    NSFont *_selectedFont;
    BOOL _isMultiple;
    NSFontAction _currentFontAction;
    int _currentTrait;
}

+ (NSFontManager *)sharedFontManager;

+ (void)setFontManagerFactory:(Class)value;
+ (void)setFontPanelFactory:(Class)value;

- delegate;
- (SEL)action;

- (void)setDelegate:delegate;
- (void)setAction:(SEL)value;

- (NSFontAction)currentFontAction;

- (NSArray *)collectionNames;
- (BOOL)addCollection:(NSString *)name options:(int)options;
- (void)addFontDescriptors:(NSArray *)descriptors toCollection:(NSString *)name;
- (BOOL)removeCollection:(NSString *)name;

- (NSArray *)fontDescriptorsInCollection:(NSString *)name;

- (NSArray *)availableFonts;
- (NSArray *)availableFontFamilies;
- (NSArray *)availableMembersOfFontFamily:(NSString *)name;
- (NSArray *)availableFontNamesMatchingFontDescriptor:(NSFontDescriptor *)descriptor;
- (NSArray *)availableFontNamesWithTraits:(NSFontTraitMask)traits;

- (BOOL)fontNamed:(NSString *)name hasTraits:(NSFontTraitMask)traits;
- (NSFont *)fontWithFamily:(NSString *)family traits:(NSFontTraitMask)traits weight:(int)weight size:(float)size;
- (int)weightOfFont:(NSFont *)font;
- (NSFontTraitMask)traitsOfFont:(NSFont *)font;

- (NSString *)localizedNameForFamily:(NSString *)family face:(NSString *)face;

- (NSFontPanel *)fontPanel:(BOOL)create;

- (BOOL)sendAction;

- (BOOL)isEnabled;
- (BOOL)isMultiple;
- (NSFont *)selectedFont;

- (void)setSelectedFont:(NSFont *)font isMultiple:(BOOL)isMultiple;

- (NSFont *)convertFont:(NSFont *)font;
- (NSFont *)convertFont:(NSFont *)font toSize:(float)size;
- (NSFont *)convertFont:(NSFont *)font toHaveTrait:(NSFontTraitMask)trait;
- (NSFont *)convertFont:(NSFont *)font toNotHaveTrait:(NSFontTraitMask)trait;

- (NSFont *)convertFont:(NSFont *)font toFace:(NSString *)typeface;
- (NSFont *)convertFont:(NSFont *)font toFamily:(NSString *)family;
- (NSFont *)convertWeight:(BOOL)heavierNotLighter ofFont:(NSFont *)font;

- (NSDictionary *)convertAttributes:(NSDictionary *)attributes;

- (void)addFontTrait:sender;
- (void)modifyFont:sender;
- (void)modifyFontViaPanel:sender;
- (void)removeFontTrait:sender;

- (void)orderFrontFontPanel:sender;
- (void)orderFrontStylesPanel:sender;

@end

@interface NSObject (NSFontManager_delegate)
- (BOOL)fontManager:sender willIncludeFont:(NSString *)fontName;
@end
