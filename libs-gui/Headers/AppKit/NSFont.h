/* 
   NSFont.h

   The font class

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Author:  Ovidiu Predescu <ovidiu@net-community.com>
   Date: 1996, 1997
   
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

#ifndef _GNUstep_H_NSFont
#define _GNUstep_H_NSFont
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
// For NSControlSize
#import <AppKit/NSColor.h>

@class NSAffineTransform;
@class NSCharacterSet;
@class NSDictionary;
@class NSFontDescriptor;
@class NSGraphicsContext;

typedef unsigned int NSGlyph;

enum {
  NSControlGlyph = 0x00ffffff,
  GSAttachmentGlyph = 0x00fffffe,
  NSNullGlyph = 0x0
};

typedef enum _NSGlyphRelation {
  NSGlyphBelow,
  NSGlyphAbove,
} NSGlyphRelation;

typedef enum _NSMultibyteGlyphPacking {
  NSOneByteGlyphPacking,
  NSJapaneseEUCGlyphPacking, 
  NSAsciiWithDoubleByteEUCGlyphPacking,
  NSTwoByteGlyphPacking, 
  NSFourByteGlyphPacking
} NSMultibyteGlyphPacking;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
typedef enum _NSFontRenderingMode
{
  NSFontDefaultRenderingMode = 0,
  NSFontAntialiasedRenderingMode,
  NSFontIntegerAdvancementsRenderingMode,
  NSFontAntialiasedIntegerAdvancementsRenderingMode
} NSFontRenderingMode;
#endif

APPKIT_EXPORT const CGFloat *NSFontIdentityMatrix;

@interface NSFont : NSObject <NSCoding, NSCopying>
{
  NSString *fontName;
  CGFloat matrix[6];
  BOOL matrixExplicitlySet; // unused
  BOOL screenFont;

  id fontInfo;
  void *_fontRef;

  /*
  If this font was created with a specific "role", like user font, or
  message font, and not a specific postscript name, the role will be
  stored here.
  */
  int role;

  /*
  For printer fonts, this is a cache of the corresponding screen font.
  It is initialized to placeHolder, and is created for real on demand in
  -screenFont (and retained). For screen fonts, it's nil.
  */
  NSFont *cachedScreenFont;

  /*
  In the GNUstep implementation, fonts may encapsulate some rendering state
  relating to view flipped state, therefore we generate a separate font for
  this case.  We don't create it by default, unless -set is called in a
  flipped context.
  */
  NSFont *cachedFlippedFont;
}

//
// Creating a Font Object
//
+ (NSFont*) boldSystemFontOfSize: (CGFloat)fontSize;
+ (NSFont*) fontWithName: (NSString*)aFontName 
		  matrix: (const CGFloat*)fontMatrix;
+ (NSFont*) fontWithName: (NSString*)aFontName
		    size: (CGFloat)fontSize;
+ (NSFont*) systemFontOfSize: (CGFloat)fontSize;
+ (NSFont*) userFixedPitchFontOfSize: (CGFloat)fontSize;
+ (NSFont*) userFontOfSize: (CGFloat)fontSize;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (NSFont*) titleBarFontOfSize: (CGFloat)fontSize;
+ (NSFont*) menuFontOfSize: (CGFloat)fontSize;
+ (NSFont*) messageFontOfSize: (CGFloat)fontSize;
+ (NSFont*) paletteFontOfSize: (CGFloat)fontSize;
+ (NSFont*) toolTipsFontOfSize: (CGFloat)fontSize;
+ (NSFont*) controlContentFontOfSize: (CGFloat)fontSize;
+ (NSFont*) labelFontOfSize: (CGFloat)fontSize;
+ (NSFont*) menuBarFontOfSize: (CGFloat)fontSize;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
+ (NSFont*) fontWithDescriptor: (NSFontDescriptor*)descriptor size: (CGFloat)size;
+ (NSFont*) fontWithDescriptor: (NSFontDescriptor*)descriptor 
                 textTransform: (NSAffineTransform*)transform;
// This method was a mistake in the 10.4 documentation
+ (NSFont*) fontWithDescriptor: (NSFontDescriptor*)descriptor 
                          size: (CGFloat)size
                 textTransform: (NSAffineTransform*)transform;
#endif

//
// Font Sizes
//
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (CGFloat) labelFontSize;
+ (CGFloat) smallSystemFontSize;
+ (CGFloat) systemFontSize;
+ (CGFloat) systemFontSizeForControlSize: (NSControlSize)controlSize;
#endif

//
// Preferred Fonts
//
+ (NSArray*) preferredFontNames;
+ (void) setPreferredFontNames: (NSArray*)fontNames;

//
// Setting the Font
//
+ (void) setUserFixedPitchFont: (NSFont*)aFont;
+ (void) setUserFont: (NSFont*)aFont;
+ (void) useFont: (NSString*)aFontName;
- (void) set;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) setInContext: (NSGraphicsContext*)context;
- (NSAffineTransform*) textTransform;
#endif

//
// Querying the Font
//
- (NSDictionary*) afmDictionary;
- (NSString*) afmFileContents;
- (NSRect) boundingRectForFont;
- (NSString*) displayName;
- (NSString*) familyName;
- (NSString*) fontName;
- (NSString*) encodingScheme;
- (BOOL) isFixedPitch;
- (BOOL) isBaseFont;
- (const CGFloat*) matrix;
- (CGFloat) pointSize;
- (NSFont*) printerFont;
- (NSFont*) screenFont;
- (CGFloat) ascender;
- (CGFloat) descender;
- (CGFloat) capHeight;
- (CGFloat) italicAngle;
- (NSSize) maximumAdvancement;
- (NSSize) minimumAdvancement;
- (CGFloat) underlinePosition;
- (CGFloat) underlineThickness;
- (CGFloat) xHeight;
- (CGFloat) widthOfString: (NSString*)string;
- (CGFloat) defaultLineHeightForFont;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (CGFloat) leading;

#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSUInteger) numberOfGlyphs;
- (NSCharacterSet*) coveredCharacterSet;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSFontDescriptor*) fontDescriptor;
- (NSFontRenderingMode) renderingMode;
- (NSFont*) screenFontWithRenderingMode: (NSFontRenderingMode)mode;
#endif

//
// Manipulating Glyphs
//
- (NSSize) advancementForGlyph: (NSGlyph)aGlyph;
- (NSRect) boundingRectForGlyph: (NSGlyph)aGlyph;
- (BOOL) glyphIsEncoded: (NSGlyph)aGlyph;
- (NSMultibyteGlyphPacking) glyphPacking;
- (NSGlyph) glyphWithName: (NSString*)glyphName;
- (NSPoint) positionOfGlyph: (NSGlyph)curGlyph
	    precededByGlyph: (NSGlyph)prevGlyph
		  isNominal: (BOOL*)nominal;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
	       forCharacter: (unichar)aChar 
	     struckOverRect: (NSRect)aRect;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
	    struckOverGlyph: (NSGlyph)baseGlyph 
	metricsExist: (BOOL*)flag;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
             struckOverRect: (NSRect)aRect 
               metricsExist: (BOOL*)flag;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
               withRelation: (NSGlyphRelation)relation 
                toBaseGlyph: (NSGlyph)baseGlyph
           totalAdvancement: (NSSize*)offset 
               metricsExist: (BOOL*)flag;
- (int) positionsForCompositeSequence: (NSGlyph*)glyphs 
                       numberOfGlyphs: (int)numGlyphs 
                           pointArray: (NSPoint*)points;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) getAdvancements: (NSSizeArray)advancements
               forGlyphs: (const NSGlyph*)glyphs
                   count: (NSUInteger)count;
- (void) getAdvancements: (NSSizeArray)advancements
         forPackedGlyphs: (const void*)glyphs
                   count: (NSUInteger)count;
- (void) getBoundingRects: (NSRectArray)bounds
                forGlyphs: (const NSGlyph*)glyphs
                    count: (NSUInteger)count;
#endif

- (NSStringEncoding) mostCompatibleStringEncoding;

@end

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
@class GSFontInfo;

@interface NSFont (GNUstep)
- (GSFontInfo*) fontInfo;
- (void *) fontRef;
@end

int NSConvertGlyphsToPackedGlyphs(NSGlyph*glBuf, 
				  int count, 
				  NSMultibyteGlyphPacking packing, 
				  char*packedGlyphs);
#endif

APPKIT_EXPORT NSString *NSAFMAscender;
APPKIT_EXPORT NSString *NSAFMCapHeight;
APPKIT_EXPORT NSString *NSAFMCharacterSet;
APPKIT_EXPORT NSString *NSAFMDescender;
APPKIT_EXPORT NSString *NSAFMEncodingScheme;
APPKIT_EXPORT NSString *NSAFMFamilyName;
APPKIT_EXPORT NSString *NSAFMFontName;
APPKIT_EXPORT NSString *NSAFMFormatVersion;
APPKIT_EXPORT NSString *NSAFMFullName;
APPKIT_EXPORT NSString *NSAFMItalicAngle;
APPKIT_EXPORT NSString *NSAFMMappingScheme;
APPKIT_EXPORT NSString *NSAFMNotice;
APPKIT_EXPORT NSString *NSAFMUnderlinePosition;
APPKIT_EXPORT NSString *NSAFMUnderlineThickness;
APPKIT_EXPORT NSString *NSAFMVersion;
APPKIT_EXPORT NSString *NSAFMWeight;
APPKIT_EXPORT NSString *NSAFMXHeight;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11, GS_API_LATEST)
typedef CGFloat NSFontWeight;
APPKIT_EXPORT const CGFloat NSFontWeightUltraLight;
APPKIT_EXPORT const CGFloat NSFontWeightThin;
APPKIT_EXPORT const CGFloat NSFontWeightLight;
APPKIT_EXPORT const CGFloat NSFontWeightRegular;
APPKIT_EXPORT const CGFloat NSFontWeightMedium;
APPKIT_EXPORT const CGFloat NSFontWeightSemibold;
APPKIT_EXPORT const CGFloat NSFontWeightBold;
APPKIT_EXPORT const CGFloat NSFontWeightHeavy;
APPKIT_EXPORT const CGFloat NSFontWeightBlack;
#endif

#endif // _GNUstep_H_NSFont
