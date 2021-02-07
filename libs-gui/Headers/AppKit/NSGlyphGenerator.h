/* -*-objc-*-
   NSGlyphGenerator.h

   Interfaces for glyph generation and storage.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  H. N. Schaller <hns@computer.org>
   Date: Jun 2006 - aligned with 10.4
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSGlyphGenerator
#define _GNUstep_H_NSGlyphGenerator
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
// define NSGlyph
#import <AppKit/NSFont.h>

@class NSAttributedString;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

enum
{
  NSShowControlGlyphs = 1,
  NSShowInvisibleGlyphs = 2,
  NSWantsBidiLevels = 4
};

@protocol NSGlyphStorage

- (NSAttributedString*) attributedString;
- (void) insertGlyphs: (const NSGlyph*)glyphs
               length: (NSUInteger)length
forStartingGlyphAtIndex: (NSUInteger)glyph
       characterIndex: (NSUInteger)index;
- (NSUInteger) layoutOptions;
- (void) setIntAttribute: (NSInteger)tag
                   value: (NSInteger)value 
         forGlyphAtIndex: (NSUInteger)index;

@end


@interface NSGlyphGenerator : NSObject

+ (id) sharedGlyphGenerator;
- (void) generateGlyphsForGlyphStorage: (id <NSGlyphStorage>)storage
             desiredNumberOfCharacters: (NSUInteger)num
              glyphIndex: (NSUInteger*)glyph
            characterIndex: (NSUInteger*)index;

@end

#endif // MAC_OS_X_VERSION_10_3

#endif // _GNUstep_H_NSGlyphGenerator
