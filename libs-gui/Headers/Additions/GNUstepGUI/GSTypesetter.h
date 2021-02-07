/*
   GSTypesetter.h

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: 2002

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

#ifndef _GNUstep_H_GSTypesetter
#define _GNUstep_H_GSTypesetter

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSGeometry.h>

@class GSLayoutManager;
@class NSTextContainer;
@class NSFont;


/* This class is abstract. */
/* This isn't final. It will probably change. If you want to sub-class,
you'll have to update your code when things change (or wait until it's
done). */
@interface GSTypesetter : NSObject

/*
???
GSTypesetter's implementation returns NSMakeSize(0,0).
*/
+ (NSSize) printingAdjustmentsInLayoutManager: (GSLayoutManager *)layoutManager
		 forNominallySpacedGlyphRange: (NSRange)glyphRange
				 packedGlyphs: (const unsigned char *)glyphs
					count: (unsigned)packedGlyphCount;

/*
Returns a thread-safe shared GSTypesetter (a GSHorizontalTypesetter
instance in practice, at least when this is done).
*/
+(GSTypesetter *) sharedSystemTypesetter;


/*
Returns the font that should be used for the given attributes.

GSTypesetter's implementation returns the value of NSFontAttribute, or
[NSFont userFontOfSize: 0] if there is no such value.

Subclasses can use this to make the font picked depend on other attributes,
eg. to automatically use a smaller size for subscripts and superscripts.

{Provide context?}
*/
-(NSFont *) fontForCharactersWithAttributes: (NSDictionary *)attributes;


/*

Lay out glyphs means that, for each glyph laid out in one call, the following
things need to be done:

 -setTextContainer:forGlyphRange:
 1.  Set the text container for the glyph.

 -setLineFragmentRect:forGlyphRange:usedRect:
 2.  Set the line fragment rectangle for the glyph.

 -setLocation:forStartOfGlyphRange:
 3.  Set the position for the glyph (directly, or indirectly by setting the
    starting position for a range of nominally spaced glyphs containing the
    glyph).

It should also set the drawsOutsideLineFragment and notShownAttribute if
they are YES (they are set to NO by -setTextContainer:forGlyphRange:).
(TODO: good?)

*/

/*
Lay out glyphs, starting at glyphIndex, in textContainer.

The line fragment rectangle for the previous line fragment in this
textContainer, or NSZeroRect if there are none, is in previousLineFragRect.

The index of the first glyph not laid out should be returned in
nextGlyphIndex (which may _not_ be NULL). If all glyphs have been laid out,
set it to [layoutManager numberOfGlyphs].

howMany is the number of requested line fragment rectangles. The typesetter
should try to create approximately this many and then stop. If it is 0, the
typesetter should fill the entire text container.

Returns:
  0 did some layout, nothing special happened
  1 text container is full
  2 all glyphs have been laid out

Subclasses need to implement this method.

{Too much context?}
*/
-(int) layoutGlyphsInLayoutManager: (GSLayoutManager *)layoutManager
		   inTextContainer: (NSTextContainer *)textContainer
	      startingAtGlyphIndex: (unsigned int)glyphIndex
	  previousLineFragmentRect: (NSRect)previousLineFragRect
		    nextGlyphIndex: (unsigned int *)nextGlyphIndex
	     numberOfLineFragments: (unsigned int)howMany;

@end

#endif

