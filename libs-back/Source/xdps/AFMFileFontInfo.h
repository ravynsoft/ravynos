/*
   AFMFileFontInfo.h

   Private data of PXKFont class.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   
   This file is part of the GNUstep GUI X/DPS Library.

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

#ifndef __AFMFileFontInfo_h__
#define __AFMFileFontInfo_h__

#include <Foundation/NSMapTable.h>
#include <GNUstepGUI/GSFontInfo.h>
#include "parseAFM.h"

typedef struct {
  NSGlyph glyph;
  NSSize advancement;
} tPairKerningInfo;

@interface PXKFontEnumerator : GSFontEnumerator
{
}
@end

@interface AFMGlyphInfo : NSObject <NSCopying>
{
  NSString* name;
  NSGlyph code;
  NSRect bbox;
  NSSize advancement;
  int lastKernPair;
  int numOfPairs;
  tPairKerningInfo* kerning;
}

+ (AFMGlyphInfo*) glyphFromAFMCharMetricInfo: (AFMCharMetricInfo*)metricInfo;

- (AFMGlyphInfo*)mutableCopyWithZone: (NSZone*)zone;

- (NSString*) name;
- (NSGlyph) code;
- (NSRect) boundingRect;
- (NSSize) advancement;
- (BOOL) isEncoded;

- (void) transformUsingMatrix: (const float*)matrix;

- (void) incrementNumberOfKernPairs;
- (void) addPairKerningForGlyph:(NSGlyph)glyph advancement:(NSSize)advancement;
- (NSSize) advancementIfFollowedByGlyph: (NSGlyph)glyph
                               isNominal: (BOOL*)nominal;

@end


@interface AFMFileFontInfo : GSFontInfo
{
  NSString *afmFileName;
  NSMapTable *glyphsByName;
  AFMGlyphInfo *glyphs[256];
}

- (AFMFileFontInfo*)mutableCopyWithZone: (NSZone*)zone;
- (AFMFileFontInfo*)initUnscaledWithFontName: (NSString*)name;
@end

#endif /* __AFMFileFontInfo_h__ */

