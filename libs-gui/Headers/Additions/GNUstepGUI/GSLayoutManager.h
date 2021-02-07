/*
   GSLayoutManager.h

   Copyright (C) 2002, 2003 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: November 2002 - February 2003

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

#ifndef _GNUstep_H_GSLayoutManager
#define _GNUstep_H_GSLayoutManager

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSGlyphGenerator.h>

@class GSTypesetter;
@class NSTextStorage,NSTextContainer;

typedef enum
{
  NSGlyphInscribeBase = 0,
  NSGlyphInscribeBelow = 1,
  NSGlyphInscribeAbove = 2,
  NSGlyphInscribeOverstrike = 3,
  NSGlyphInscribeOverBelow = 4
} NSGlyphInscription;

enum {
  NSGlyphAttributeSoft = 0,
  NSGlyphAttributeElastic = 1,
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  NSGlyphAttributeBidiLevel = 2,
#endif
  NSGlyphAttributeInscribe = 5
};

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
@interface GSLayoutManager : NSObject <NSGlyphStorage, NSCoding>
#else
@interface GSLayoutManager : NSObject
#endif
{
@protected
  NSTextStorage *_textStorage;
  NSGlyphGenerator *_glyphGenerator;

  id _delegate;

  BOOL usesScreenFonts;
  BOOL backgroundLayoutEnabled;
  BOOL showsInvisibleCharacters;
  BOOL showsControlCharacters;

  GSTypesetter *typesetter;


/* Glyph storage */

  /* Skip list of runs */
  struct GSLayoutManager_glyph_run_head_s *glyphs;

  /* number of runs created from existing text (ie. not as a result of
  stuff being invalidated) */
  int glyph_num_end_runs;


/* Layout storage */
/* OPT: This is just a simple implementation that should let me figure out
how it's supposed to work. It's functional and correct, but it isn't fast. */

  unsigned int layout_glyph, layout_char;

  struct GSLayoutManager_textcontainer_s *textcontainers;
  int num_textcontainers;

  NSRect extra_rect, extra_used_rect;
  NSTextContainer *extra_textcontainer;


  /* For -rectArrayForGlyphRange:... */
  NSRect *rect_array;
  int rect_array_size;


  /*
  Cached run. GSHorizontalTypesetter (and other typesetters, presumably)
  often walk linearly through the glyphs. Thus, for many methods, we cache
  the last run so we can quickly get more information for the next glyph.
  */
  struct GSLayoutManager_glyph_run_s *cached_run;
  unsigned int cached_pos, cached_cpos;
}


- (NSTextStorage *) textStorage;
- (void) setTextStorage: (NSTextStorage *)aTextStorage;
- (void) replaceTextStorage: (NSTextStorage *)newTextStorage;

- (NSGlyphGenerator *) glyphGenerator;
- (void) setGlyphGenerator: (NSGlyphGenerator *)glyphGenerator;

- (id) delegate;
- (void) setDelegate: (id)aDelegate;


-(GSTypesetter *) typesetter;
-(void) setTypesetter: (GSTypesetter *)typesetter;


- (void) setBackgroundLayoutEnabled: (BOOL)flag;
- (BOOL) backgroundLayoutEnabled;

- (void) setShowsInvisibleCharacters: (BOOL)flag;
- (BOOL) showsInvisibleCharacters;

- (void) setShowsControlCharacters: (BOOL)flag;
- (BOOL) showsControlCharacters;


/** Font handling **/

- (BOOL) usesScreenFonts;
- (void) setUsesScreenFonts: (BOOL)flag;

- (NSFont *) substituteFontForFont: (NSFont *)originalFont;


/*
(?)
Sent by the NSTextStorage. mask tells us if attributes or characters (or
both) have been changed. range is the range of directly modified
characters. invalidatedRange is the range of characters affected by the
changes (contains range but may be larger due to eg. attribute fixing).
If characters have been edited, lengthChange has the text length delta.
*/
- (void) textStorage: (NSTextStorage *)aTextStorage
	edited: (unsigned int)mask
	range: (NSRange)range
	changeInLength: (int)lengthChange
	invalidatedRange: (NSRange)invalidatedRange;

/**
 * GNUstep extension
 */
- (void) insertGlyphs: (const NSGlyph*)glyph_list
     withAdvancements: (const NSSize*)advancements
               length: (NSUInteger)length
forStartingGlyphAtIndex: (NSUInteger)glyph
       characterIndex: (NSUInteger)index;

- (NSDictionary *) typingAttributes;

@end


@interface GSLayoutManager (glyphs)

/** Glyph handling **/

/*
Mark the glyphs for the characters in aRange as invalid. lengthChange
is the text length delta. If not NULL, the range of characters actually
affected (_after_ the change) will be returned in actualRange.

This method is used internally and should _not_ be called. (It interacts
in complex ways with layout invalidation.)
*/
- (void) invalidateGlyphsForCharacterRange: (NSRange)aRange 
	changeInLength: (int)lengthChange
	actualCharacterRange: (NSRange *)actualRange;

/*
These are internal methods and should _not_ be called.
*/
- (void) insertGlyph: (NSGlyph)aGlyph
	atGlyphIndex: (unsigned int)glyphIndex
	characterIndex: (unsigned int)charIndex;
- (void) replaceGlyphAtIndex: (unsigned int)glyphIndex
	withGlyph: (NSGlyph)newGlyph;
- (void) deleteGlyphsInRange: (NSRange)aRange;
- (void) setCharacterIndex: (unsigned int)charIndex
	forGlyphAtIndex: (unsigned int)glyphIndex;


/* Returns total number of glyphs. */
- (unsigned int) numberOfGlyphs;

/* Returns the glyph at glyphIndex or raises an NSRangeException if the
index is invalid (past the end of the glyphs). */
- (NSGlyph) glyphAtIndex: (unsigned int)glyphIndex;

/* Returns the glyph at glyphIndex and sets isValidIndex to YES if the index
is valid. Otherwise, the return value is arbitrary and isValidIndex is set
to NO. */
- (NSGlyph) glyphAtIndex: (unsigned int)glyphIndex
	isValidIndex: (BOOL *)isValidIndex;

/* Returns if the glyph at glyphIndex is valid or not */
- (BOOL) isValidGlyphIndex: (unsigned int)glyphIndex;

/* Copies displayed glyphs to glyphArray for glyphRange. Returns the number
of glyphs actually copied to the array. NSRangeException of the range is
invalid (extends beyond the end of glyphs). */
- (unsigned int) getGlyphs: (NSGlyph *)glyphArray
	range: (NSRange)glyphRange;

/* Return the first character for the glyph at glyphIndex.
(NSRangeException?) */
- (unsigned int) characterIndexForGlyphAtIndex: (unsigned int)glyphIndex;

/**
 * GNUstep extension
 */
- (NSSize) advancementForGlyphAtIndex: (unsigned int)glyphIndex;

/* Returns the range of glyphs for the characters in charRange. If
actualRange isn't NULL, the exact range of characters for the glyphs in the
returned range is returned there. */
- (NSRange) glyphRangeForCharacterRange: (NSRange)charRange 
	actualCharacterRange: (NSRange *)actualCharRange;

/* Returns the range of characters for the glyphs in glyphRange. Returns
the actual glyphs for the characters in the range in actualGlyphRange, if
it isn't NULL. */
- (NSRange) characterRangeForGlyphRange: (NSRange)glyphRange
	actualGlyphRange: (NSRange *)actualGlyphRange;


/* These can be used to set arbitrary tags on individual glyphs.
Non-negative tags are reserved. You must provide storage yourself (by
subclassing). */
#if !OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) setIntAttribute: (int)attributeTag 
	value: (int)anInt
	forGlyphAtIndex: (unsigned int)glyphIndex;
#endif
- (int) intAttribute: (int)attributeTag
	forGlyphAtIndex: (unsigned int)glyphIndex;


/* Returns the font actually used for a range of glyphs. This isn't
necessarily the font specified by NSFontAttributeName; both the typesetter
and the layout manager can substitute a different font (the typesetter might
eg. substitute a smaller font for sub-/super-scripted text, and the layout
manager might be substituting screen fonts. */
- (NSFont *) effectiveFontForGlyphAtIndex: (unsigned int)glyphIndex
	range: (NSRange *)range; /* GNUstep extension */


- (void) setDrawsOutsideLineFragment: (BOOL)flag
	forGlyphAtIndex: (unsigned int)glyphIndex;
- (BOOL) drawsOutsideLineFragmentForGlyphAtIndex: (unsigned int) glyphIndex;

- (void) setNotShownAttribute: (BOOL)flag 
	forGlyphAtIndex: (unsigned int)glyphIndex;
- (BOOL) notShownAttributeForGlyphAtIndex: (unsigned int) glyphIndex;

@end


@interface GSLayoutManager (layout)

/** Text containers **/

- (NSArray *) textContainers;

- (void) addTextContainer: (NSTextContainer *)container;
- (void) insertTextContainer: (NSTextContainer*)aTextContainer 
	atIndex: (unsigned int)index;
- (void) removeTextContainerAtIndex: (unsigned int)index;

- (void) textContainerChangedGeometry: (NSTextContainer *)aContainer;


/** Layout **/

/*
This method is used internally and should _not_ be called. (It interacts
in complex ways with glyph invalidation, and with itself when doing soft
invalidation.)
*/
- (void) invalidateLayoutForCharacterRange: (NSRange)aRange 
	isSoft: (BOOL)flag
	actualCharacterRange: (NSRange *)actualRange;


- (void) setTextContainer: (NSTextContainer *)aTextContainer 
	forGlyphRange: (NSRange)glyphRange;

- (void) setLineFragmentRect: (NSRect)fragmentRect 
	forGlyphRange: (NSRange)glyphRange
	usedRect: (NSRect)usedRect;

- (void) setLocation: (NSPoint)location 
	forStartOfGlyphRange: (NSRange)glyphRange;

- (void) setAttachmentSize: (NSSize)attachmentSize 
	forGlyphRange: (NSRange)glyphRange; /* not OPENSTEP */


- (NSTextContainer *) textContainerForGlyphAtIndex: (NSUInteger)glyphIndex
	effectiveRange: (NSRange *)effectiveRange;
- (NSRect) lineFragmentRectForGlyphAtIndex: (NSUInteger)glyphIndex
	effectiveRange: (NSRange *)effectiveGlyphRange;
- (NSRect) lineFragmentUsedRectForGlyphAtIndex: (NSUInteger)glyphIndex
	effectiveRange: (NSRange *)effectiveGlyphRange;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSTextContainer *) textContainerForGlyphAtIndex: (NSUInteger)glyphIndex
                      effectiveRange: (NSRange *)effectiveRange
                      withoutAdditionalLayout: (BOOL)flag;
- (NSRect) lineFragmentRectForGlyphAtIndex: (NSUInteger)glyphIndex
           effectiveRange: (NSRange *)effectiveGlyphRange
           withoutAdditionalLayout: (BOOL)flag;
- (NSRect) lineFragmentUsedRectForGlyphAtIndex: (NSUInteger)glyphIndex
           effectiveRange: (NSRange *)effectiveGlyphRange
           withoutAdditionalLayout: (BOOL)flag;
#endif


/*
The typesetter may set this to mark where the rectangle the insertion point
is to be placed if the insertion point is beyond the last character of the
text. The extra text container is reset to nil any time layout is
invalidated.
*/
-(void) setExtraLineFragmentRect: (NSRect)linefrag
			usedRect: (NSRect)used
		   textContainer: (NSTextContainer *)tc;

-(NSRect) extraLineFragmentRect;
-(NSRect) extraLineFragmentUsedRect;
-(NSTextContainer *) extraLineFragmentTextContainer;


/* Extension, but without this, there's no way to get the starting locations
of the nominally spaced glyphs. */
- (NSRange) rangeOfNominallySpacedGlyphsContainingIndex:(unsigned int)glyphIndex
	startLocation: (NSPoint *)p;

- (NSRange) rangeOfNominallySpacedGlyphsContainingIndex:(unsigned int)glyphIndex;

/* The union of all line frag rects' used rects. (TODO: shouldn't this be
just the union of all the line frag rects?) */
- (NSRect) usedRectForTextContainer: (NSTextContainer *)container;

- (NSRange) glyphRangeForTextContainer: (NSTextContainer *)container;


- (unsigned int) firstUnlaidCharacterIndex;
- (unsigned int) firstUnlaidGlyphIndex;
- (void) getFirstUnlaidCharacterIndex: (unsigned int *)charIndex
	glyphIndex: (unsigned int *)glyphIndex;


/*
Basic (and experimental) methods that let the typesetter use soft-invalidated
layout information.
*/
-(void) _softInvalidateUseLineFrags: (int)num
			  withShift: (NSSize)shift
		    inTextContainer: (NSTextContainer *)textContainer;
-(NSRect) _softInvalidateLineFragRect: (int)index
			   firstGlyph: (unsigned int *)first_glyph
			    nextGlyph: (unsigned int *)next_glyph
		      inTextContainer: (NSTextContainer *)textContainer;
-(unsigned int) _softInvalidateFirstGlyphInTextContainer: (NSTextContainer *)textContainer;
-(unsigned int) _softInvalidateNumberOfLineFragsInTextContainer: (NSTextContainer *)textContainer;

@end


@interface NSObject (GSLayoutManagerDelegate)
-(void) layoutManager: (GSLayoutManager *)layoutManager
	didCompleteLayoutForTextContainer: (NSTextContainer *)textContainer
	atEnd: (BOOL)atEnd;
@end


#endif

