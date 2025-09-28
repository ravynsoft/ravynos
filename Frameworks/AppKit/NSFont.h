/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSCell.h>

@class NSFontDescriptor;

// Provides an extendable translation scheme for apps that use non-standard fonts in UI elements
@interface NSNibFontNameTranslator : NSObject {
}

- (NSString *)translateToNibFontName:(NSString *)fontName;
- (NSString *)translateFromNibFontName:(NSString *)fontName;

@end

typedef NSUInteger NSGlyph;

enum {
    NSNullGlyph = 0,
    NSControlGlyph = 0xFFFFFF,
};

typedef enum {
    NSNativeShortGlyphPacking,
} NSMultibyteGlyphPacking;

typedef enum {
    NSFontDefaultRenderingMode,
    NSFontAntialiasedRenderingMode,
    NSFontIntegerAdvancementsRenderingMode,
    NSFontAntialiasedIntegerAdvancementsRenderingMode,
} NSFontRenderingMode;

@interface NSFont : NSObject <NSCopying> {
    NSString *_name;
    float _pointSize;
    float _matrix[6];
    NSStringEncoding _encoding;

    CGFontRef _cgFont;
    CTFontRef _ctFont;
}

+ (float)systemFontSize;
+ (float)smallSystemFontSize;
+ (float)labelFontSize;
+ (float)systemFontSizeForControlSize:(NSControlSize)size;

+ (NSFont *)boldSystemFontOfSize:(float)size;
+ (NSFont *)controlContentFontOfSize:(float)size;

+ (NSFont *)labelFontOfSize:(float)size;
+ (NSFont *)menuFontOfSize:(float)size;
+ (NSFont *)menuBarFontOfSize:(float)size;

+ (NSFont *)messageFontOfSize:(float)size;
+ (NSFont *)paletteFontOfSize:(float)size;
+ (NSFont *)systemFontOfSize:(float)size;
+ (NSFont *)titleBarFontOfSize:(float)size;
+ (NSFont *)toolTipsFontOfSize:(float)size;
+ (NSFont *)userFontOfSize:(float)size;
+ (NSFont *)userFixedPitchFontOfSize:(float)size;

+ (void)setUserFont:(NSFont *)value;
+ (void)setUserFixedPitchFont:(NSFont *)value;

+ (NSFont *)fontWithName:(NSString *)name size:(float)size;
+ (NSFont *)fontWithName:(NSString *)name matrix:(const float *)matrix;
+ (NSFont *)fontWithDescriptor:(NSFontDescriptor *)descriptor size:(float)size;
+ (NSFont *)fontWithDescriptor:(NSFontDescriptor *)descriptor size:(float)size textTransform:(NSAffineTransform *)transform;

+ (NSArray *)preferredFontNames;
+ (void)setPreferredFontNames:(NSArray *)fontNames;

- (float)pointSize;
- (NSString *)fontName;
- (const float *)matrix;
- (NSAffineTransform *)textTransform;
- (NSFontRenderingMode)renderingMode;
- (NSCharacterSet *)coveredCharacterSet;
- (NSStringEncoding)mostCompatibleStringEncoding;
- (NSString *)familyName;
- (NSString *)displayName;
- (NSFontDescriptor *)fontDescriptor;

- (NSFont *)printerFont;
- (NSFont *)screenFont;
- (NSFont *)screenFontWithRenderingMode:(NSFontRenderingMode)mode;

- (NSRect)boundingRectForFont;
- (NSRect)boundingRectForGlyph:(NSGlyph)glyph;

- (NSMultibyteGlyphPacking)glyphPacking;
- (unsigned)numberOfGlyphs;
- (NSGlyph)glyphWithName:(NSString *)name;
- (BOOL)glyphIsEncoded:(NSGlyph)glyph;
- (NSSize)advancementForGlyph:(NSGlyph)glyph;

- (NSSize)maximumAdvancement;
- (float)underlinePosition;
- (float)underlineThickness;
- (CGFloat)ascender;
- (CGFloat)descender;
- (CGFloat)leading;
- (CGFloat)defaultLineHeightForFont;
- (BOOL)isFixedPitch;
- (float)italicAngle;
- (float)xHeight;
- (float)capHeight;

- (void)setInContext:(NSGraphicsContext *)context;
- (void)set;

- (NSPoint)positionOfGlyph:(NSGlyph)current precededByGlyph:(NSGlyph)previous isNominal:(BOOL *)isNominalp;

- (void)getAdvancements:(NSSize *)advancements forGlyphs:(const NSGlyph *)glyphs count:(unsigned)count;
- (void)getAdvancements:(NSSize *)advancements forPackedGlyphs:(const void *)packed length:(unsigned)length;
- (void)getBoundingRects:(NSRect *)rects forGlyphs:(const NSGlyph *)glyphs count:(unsigned)count;

// private

- (unsigned)getGlyphs:(NSGlyph *)glyphs forCharacters:(unichar *)characters length:(unsigned)length;

@end

@interface NSFont (PortatibilityAdditions)

+ (void)setNibFontTranslator:(NSNibFontNameTranslator *)fontTranslator;
+ (NSNibFontNameTranslator *)nibFontTranslator;

@end

int NSConvertGlyphsToPackedGlyphs(NSGlyph *glyphs, int length, NSMultibyteGlyphPacking packing, char *output);
