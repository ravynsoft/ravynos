/*
   GSTypesetter.m

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

#import <Foundation/NSDictionary.h>
#import "AppKit/NSAttributedString.h"
#import "GNUstepGUI/GSTypesetter.h"
#import "GNUstepGUI/GSLayoutManager.h"
#import "GNUstepGUI/GSHorizontalTypesetter.h"

@implementation GSTypesetter

+ (NSSize) printingAdjustmentsInLayoutManager: (GSLayoutManager *)layoutManager
		 forNominallySpacedGlyphRange: (NSRange)glyphRange
				 packedGlyphs: (const unsigned char *)glyphs
					count: (unsigned)packedGlyphCount
{
  return NSMakeSize(0,0);
}

+(GSTypesetter *) sharedSystemTypesetter
{
  return [GSHorizontalTypesetter sharedInstance];
}


-(NSFont *) fontForCharactersWithAttributes: (NSDictionary *)attributes
{
  NSFont *f = [attributes valueForKey: NSFontAttributeName];
  if (!f)
    f = [NSFont userFontOfSize: 0];
  return f;
}


-(int) layoutGlyphsInLayoutManager: (GSLayoutManager *)layoutManager
		   inTextContainer: (NSTextContainer *)textContainer
	      startingAtGlyphIndex: (unsigned int)glyphIndex
	  previousLineFragmentRect: (NSRect)previousLineFragRect
		    nextGlyphIndex: (unsigned int *)nextGlyphIndex
	     numberOfLineFragments: (unsigned int)howMany
{
  [self subclassResponsibility: _cmd];
  return 0;
}

@end

