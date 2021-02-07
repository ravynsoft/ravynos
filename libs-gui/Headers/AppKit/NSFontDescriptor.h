/* 
   NSFontDescriptor.h

   Holds an image to use as a cursor

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author:  Dr. H. Nikolaus Schaller <hns@computer.org>
   Date: 2006
   
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


#ifndef _GNUstep_H_NSFontDescriptor
#define _GNUstep_H_NSFontDescriptor
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

@class NSArray;
@class NSCoder;
@class NSDictionary;
@class NSSet;
@class NSString;
@class NSAffineTransform;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
typedef uint32_t NSFontSymbolicTraits;

typedef enum _NSFontFamilyClass
{
  NSFontUnknownClass = 0 << 28,
  NSFontOldStyleSerifsClass = 1U << 28,
  NSFontTransitionalSerifsClass = 2U << 28,
  NSFontModernSerifsClass = 3U << 28,
  NSFontClarendonSerifsClass = 4U << 28,
  NSFontSlabSerifsClass = 5U << 28,
  NSFontFreeformSerifsClass = 7U << 28,
  NSFontSansSerifClass = 8U << 28,
  NSFontOrnamentalsClass = 9U << 28,
  NSFontScriptsClass = 10U << 28,
  NSFontSymbolicClass = 12U << 28
} NSFontFamilyClass;

enum _NSFontFamilyClassMask {
    NSFontFamilyClassMask = 0xF0000000
};

enum _NSFontTrait
{
  NSFontItalicTrait = 0x0001,
  NSFontBoldTrait = 0x0002,
  NSFontExpandedTrait = 0x0020,
  NSFontCondensedTrait = 0x0040,
  NSFontMonoSpaceTrait = 0x0400,
  NSFontVerticalTrait = 0x0800,
  NSFontUIOptimizedTrait = 0x1000
};

#endif 

APPKIT_EXPORT NSString *NSFontFamilyAttribute;
APPKIT_EXPORT NSString *NSFontNameAttribute;
APPKIT_EXPORT NSString *NSFontFaceAttribute;
APPKIT_EXPORT NSString *NSFontSizeAttribute; 
APPKIT_EXPORT NSString *NSFontVisibleNameAttribute; 
APPKIT_EXPORT NSString *NSFontColorAttribute;
APPKIT_EXPORT NSString *NSFontMatrixAttribute;
APPKIT_EXPORT NSString *NSFontVariationAttribute;
APPKIT_EXPORT NSString *NSFontCharacterSetAttribute;
APPKIT_EXPORT NSString *NSFontCascadeListAttribute;
APPKIT_EXPORT NSString *NSFontTraitsAttribute;
APPKIT_EXPORT NSString *NSFontFixedAdvanceAttribute;

APPKIT_EXPORT NSString *NSFontSymbolicTrait;
APPKIT_EXPORT NSString *NSFontWeightTrait;
APPKIT_EXPORT NSString *NSFontWidthTrait;
APPKIT_EXPORT NSString *NSFontSlantTrait;

APPKIT_EXPORT NSString *NSFontVariationAxisIdentifierKey;
APPKIT_EXPORT NSString *NSFontVariationAxisMinimumValueKey;
APPKIT_EXPORT NSString *NSFontVariationAxisMaximumValueKey;
APPKIT_EXPORT NSString *NSFontVariationAxisDefaultValueKey;
APPKIT_EXPORT NSString *NSFontVariationAxisNameKey;

@interface NSFontDescriptor : NSObject <NSCoding, NSCopying>
{
  NSDictionary *_attributes;
}

+ (id) fontDescriptorWithFontAttributes: (NSDictionary *)attributes;
+ (id) fontDescriptorWithName: (NSString *)name
                         size: (CGFloat)size;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
+ (id) fontDescriptorWithName: (NSString *)name
                       matrix: (NSAffineTransform *)matrix;
#endif

- (NSDictionary *) fontAttributes;
- (id) initWithFontAttributes: (NSDictionary *)attributes;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSFontDescriptor *) fontDescriptorByAddingAttributes:
  (NSDictionary *)attributes;
- (NSFontDescriptor *) fontDescriptorWithFace: (NSString *)face;
- (NSFontDescriptor *) fontDescriptorWithFamily: (NSString *)family;
- (NSFontDescriptor *) fontDescriptorWithMatrix: (NSAffineTransform *)matrix;
- (NSFontDescriptor *) fontDescriptorWithSize: (CGFloat)size;
- (NSFontDescriptor *) fontDescriptorWithSymbolicTraits:
  (NSFontSymbolicTraits)traits;
- (NSArray *) matchingFontDescriptorsWithMandatoryKeys: (NSSet *)keys;

- (id) objectForKey: (NSString *)attribute;
- (NSAffineTransform *) matrix;
- (CGFloat) pointSize;
- (NSString *) postscriptName;
- (NSFontSymbolicTraits) symbolicTraits;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSFontDescriptor *) matchingFontDescriptorWithMandatoryKeys: (NSSet *)keys;
#endif

@end

#endif /* OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST) */

#endif /* _GNUstep_H_NSFontDescriptor */
