/*
   GSFontInfo.h

   Private class for handling font info

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Date: Mar 2000
   
   This file is part of the GNUstep.

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

#ifndef __GSFontInfo_h_INCLUDE_
#define __GSFontInfo_h_INCLUDE_

#import <AppKit/NSFont.h>
#import <AppKit/NSFontManager.h>

@class NSMutableDictionary;
@class NSArray;
@class NSBezierPath;
@class NSFontDescriptor;

@interface GSFontEnumerator : NSObject
{
  NSArray *allFontNames;
  NSMutableDictionary *allFontFamilies;
  NSArray *allFontDescriptors;
}

+ (void) setDefaultClass: (Class)defaultClass;
+ (GSFontEnumerator*) sharedEnumerator;
- (void) enumerateFontsAndFamilies;
- (NSArray*) availableFonts;
- (NSArray*) availableFontFamilies;
- (NSArray*) availableMembersOfFontFamily: (NSString*)family;
- (NSArray*) availableFontDescriptors;
- (NSArray *) availableFontNamesMatchingFontDescriptor: (NSFontDescriptor *)descriptor;
- (NSArray *) matchingFontDescriptorsFor: (NSDictionary *)attributes;
- (NSArray *) matchingDescriptorsForFamily: (NSString *)family
                                   options: (NSDictionary *)options
                                 inclusion: (NSArray *)queryDescriptors
                                 exculsion: (NSArray *)exclusionDescriptors;

/* Note that these are only called once. NSFont will remember the returned
values. Backends may override these. */
- (NSString *) defaultSystemFontName;
- (NSString *) defaultBoldSystemFontName;
- (NSString *) defaultFixedPitchFontName;
@end

@interface GSFontInfo : NSObject <NSCopying, NSMutableCopying>
{
  NSMutableDictionary* fontDictionary;

  // metrics of the font
  NSString *fontName;
  NSString *familyName;
  CGFloat matrix[6];
  CGFloat italicAngle;
  CGFloat underlinePosition;
  CGFloat underlineThickness;
  CGFloat capHeight;
  CGFloat xHeight;
  CGFloat descender;
  CGFloat ascender;
  NSSize maximumAdvancement;
  NSSize minimumAdvancement;
  NSString *encodingScheme;
  NSStringEncoding mostCompatibleStringEncoding;
  NSRect fontBBox;
  BOOL isFixedPitch;
  BOOL isBaseFont;
  int weight;
  NSFontTraitMask traits;
  unsigned numberOfGlyphs;
  NSCharacterSet *coveredCharacterSet;
  NSFontDescriptor *fontDescriptor;
}

+ (GSFontInfo*) fontInfoForFontName: (NSString*)fontName 
                             matrix: (const CGFloat*)fmatrix
			 screenFont: (BOOL)screenFont;
+ (void) setDefaultClass: (Class)defaultClass;
+ (NSString*) stringForWeight: (int)weight;
+ (int) weightForString: (NSString*)weightString;

- (NSSize) advancementForGlyph: (NSGlyph)aGlyph;
- (NSDictionary*) afmDictionary;
- (NSString*) afmFileContents;
- (void) appendBezierPathWithGlyphs: (NSGlyph*)glyphs
			      count: (int)count
		       toBezierPath: (NSBezierPath*)path;
- (CGFloat) ascender;
- (NSRect) boundingRectForGlyph: (NSGlyph)aGlyph;
- (NSRect) boundingRectForFont;
- (CGFloat) capHeight;
- (NSCharacterSet*) coveredCharacterSet;
- (CGFloat) defaultLineHeightForFont;
- (CGFloat) descender;
- (NSString *) displayName;
- (NSString *) encodingScheme;
- (NSString *) familyName;
- (NSString *) fontName;
- (BOOL) glyphIsEncoded: (NSGlyph)aGlyph;
- (NSMultibyteGlyphPacking) glyphPacking;
- (NSGlyph) glyphWithName: (NSString*)glyphName;
- (BOOL) isFixedPitch;
- (BOOL) isBaseFont;
- (CGFloat) italicAngle;
- (const CGFloat*) matrix;
- (NSSize) maximumAdvancement;
- (NSSize) minimumAdvancement;
- (NSStringEncoding) mostCompatibleStringEncoding;
- (NSUInteger) numberOfGlyphs;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
               forCharacter: (unichar)aChar 
             struckOverRect: (NSRect)aRect;
- (NSPoint) positionOfGlyph: (NSGlyph)curGlyph
	    precededByGlyph: (NSGlyph)prevGlyph
		  isNominal: (BOOL*)nominal;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
	    struckOverGlyph: (NSGlyph)baseGlyph 
	       metricsExist: (BOOL *)flag;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
	     struckOverRect: (NSRect)aRect 
	       metricsExist: (BOOL *)flag;
- (NSPoint) positionOfGlyph: (NSGlyph)aGlyph 
	       withRelation: (NSGlyphRelation)relation 
		toBaseGlyph: (NSGlyph)baseGlyph
	   totalAdvancement: (NSSize *)offset 
	       metricsExist: (BOOL *)flag;
- (NSFontTraitMask) traits;
- (CGFloat) underlinePosition;
- (CGFloat) underlineThickness;
- (int) weight;
- (CGFloat) widthOfString: (NSString*)string;
- (CGFloat) xHeight;
- (NSGlyph) glyphForCharacter: (unichar)theChar;
- (NSFontDescriptor*) fontDescriptor;

@end

#endif /* __GSFontInfo_h_INCLUDE_ */
