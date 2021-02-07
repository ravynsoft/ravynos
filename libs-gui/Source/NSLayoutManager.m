/*
   NSLayoutManager.m

   Copyright (C) 1999, 2002, 2003 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: November 2002 - February 2003

   Parts based on the old NSLayoutManager.m:
   Author: Jonathan Gapen <jagapen@smithlab.chem.wisc.edu>
   Date: July 1999
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: August 1999
   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: January 2001

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

/*
TODO: document exact requirements on typesetting for this class

Roughly:

Line frag rects arranged in lines in which all line frag rects have same
y-origin and height. Lines must not overlap vertically, and must be arranged
with strictly increasing y-origin. Line frag rects go left->right, as do
points inside line frag rects.

"Nominally spaced", to this layout manager, is described at:
http://wiki.gnustep.org/index.php/NominallySpacedGlyphs

Lines are laid out as one unit. Ie. we never do layout for only a part of a
line, or invalidate only some line frags in a line.

Also, we assume that the limit of context on layout is the previous line.
Thus, when we invalidate layout, we invalidate all lines with invalidated
characters, and the line before the first invalidated line, and
soft-invalidate everything after the last invalidated line.

Consider:

|...            |
|foo bar zot    |
|abcdefghij     |
|...            |

If we insert a space between the 'a' and the 'b' in "abcd...", the correct
result is:

|...            |
|foo bar zot a  |
|bcdefghij      |
|...            |

and to get this, we must invalidate the previous line.

TODO: This is an important assumption, and the typesetter needs to make
sure that it holds. I'm not entirely convinced that it holds for standard
latin-text layout, but I haven't been able to come up with any
counter-examples. If it turns out not to hold, we'll have to fix
invalidation here (invalidate the entire paragraph? not good for
performance, but correctness is more important), or change the typesetter
behavior.

Another assumption is that each text container will contain at least one
line frag (unless there are no more glyphs to typeset).
TODO: this doesn't hold for containers with 0 height or 0 width. need to
test. rare case, though



TODO: We often need to deal with the case where a glyph can't be typeset
(because there's nowhere to typeset it, eg. all text containers are full).
Need to figure out how to handle it.


TODO: Don't do linear searches through the line frags if avoidable. Some
cases are considerably more important than others, and should be fixed
first. Remaining cases, highest priority first:

-glyphIndexForPoint:inTextContainer:fractionOfDistanceThroughGlyph:
	Used when selecting with the mouse, and called for every event.

-characterIndexMoving:fromCharacterIndex:originalCharacterIndex:distance:
	Keyboard insertion point movement. Performance isn't important.

*/

#include <math.h>

#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSRulerMarker.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSWindow.h"
#import "AppKit/DPSOperators.h"

#import "GNUstepGUI/GSLayoutManager_internal.h"
#import "GSBindingHelpers.h"


@interface NSLayoutManager (spelling)

-(void) _drawSpellingState: (NSInteger)spellingState
	     forGylphRange: (NSRange)range
	  lineFragmentRect: (NSRect)fragmentRect
    lineFragmentGlyphRange: (NSRange)fragmentGlyphRange
	   containerOrigin: (NSPoint)containerOrigin;

@end

@interface NSLayoutManager (LayoutHelpers)
-(void) _doLayoutToContainer: (int)cindex  point: (NSPoint)p;
@end

@implementation NSLayoutManager (LayoutHelpers)
-(void) _doLayoutToContainer: (int)cindex  point: (NSPoint)p
{
  [self _doLayout];
}
@end

/**
 * Temporary attributes are implemented using an NSMutableAttributedString
 * stored in the _temporaryAttributes ivar. We only care about this attributed
 * string's attributes, not its characters, so to save space,
 * _temporaryAttributes is initialized with an instance of the following
 * NSMutableString subclass, which doesn't store any character data.
 *
 * WARNING ... it's an unofficial implementation detail that the
 * GSMutableAttributedString class creates it's internal storage by taking
 * a mutable copy of its initialisation argument, and we have a comment in
 * the source to warn about it.  If we change the behavior here, we should
 * remove the warning comment from the source in base.
 */
@interface GSDummyMutableString : NSMutableString
{
  NSUInteger _length;
}
- (id)initWithLength: (NSUInteger)aLength;
@end

/* Helper for searching for the line frag of a glyph. */
#define LINEFRAG_FOR_GLYPH(glyph) \
  do { \
    int lo, hi, mid; \
\
    lf = tc->linefrags; \
    for (lo = 0, hi = tc->num_linefrags - 1; lo < hi;) \
      { \
	mid = (lo + hi) / 2; \
	if (lf[mid].pos > glyph) \
	  hi = mid - 1; \
	else if (lf[mid].pos + lf[mid].length <= glyph) \
	  lo = mid + 1; \
	else \
	  lo = hi = mid; \
      } \
    lf = &lf[lo]; \
    i = lo; \
  } while (0)


@implementation NSLayoutManager (layout)

-(NSPoint) locationForGlyphAtIndex: (NSUInteger)glyphIndex
{
  NSRange r;
  NSPoint p;
  NSUInteger i;

  r = [self rangeOfNominallySpacedGlyphsContainingIndex: glyphIndex
	    startLocation: &p];
  if (r.location == NSNotFound)
    {
      /* The glyph hasn't been typeset yet, probably because there isn't
      enough space in the text containers to fit it. */
      return NSMakePoint(0, 0);
    }

  for (i = r.location; i < glyphIndex; i++)
    {
      p.x += [self advancementForGlyphAtIndex: i].width;
    }
  return p;
}


-(void) textContainerChangedTextView: (NSTextContainer *)aContainer
{
/* TODO: what do we do here? invalidate the displayed range for that
container? necessary? */
  int i;

  /* NSTextContainer will send the necessary messages to update the text
  view that was disconnected from us. */
  for (i = 0; i < num_textcontainers; i++)
    {
      [[textcontainers[i].textContainer textView] _updateMultipleTextViews];
      if (textcontainers[i].textContainer == aContainer)
	{
	  [[aContainer textView] setNeedsDisplay: YES];
	}
    }
}


-(NSRect *) rectArrayForGlyphRange: (NSRange)glyphRange
	  withinSelectedGlyphRange: (NSRange)selGlyphRange
		   inTextContainer: (NSTextContainer *)container
			 rectCount: (NSUInteger *)rectCount
{
  NSUInteger last;
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  int num_rects;
  float x0, x1;
  NSRect r;

  *rectCount = 0;

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
//printf("container %i %@, %i+%i\n",i,tc->textContainer,tc->pos,tc->length);
  [self _doLayoutToGlyph: NSMaxRange(glyphRange) - 1];
//printf("   now %i+%i\n",tc->pos,tc->length);
  if (i == num_textcontainers)
    {
      if (i == num_textcontainers)
	NSLog(@"%s: invalid text container", __PRETTY_FUNCTION__);
      return NULL;
    }

  /* Silently clamp range to the text container.
  TODO: is this good? */
  if (tc->pos > glyphRange.location)
    {
      if (tc->pos > NSMaxRange(glyphRange))
	return NULL;
      glyphRange.length = NSMaxRange(glyphRange) - tc->pos;
      glyphRange.location = tc->pos;
    }

  if (tc->pos + tc->length < NSMaxRange(glyphRange))
    {
      if (tc->pos + tc->length < glyphRange.location)
        return NULL;
      glyphRange.length = tc->pos + tc->length - glyphRange.location;
    }

  if (!glyphRange.length)
    {
      return NULL;
    }

  last = NSMaxRange(glyphRange);

  num_rects = 0;

  LINEFRAG_FOR_GLYPH(glyphRange.location);

  /* Main loop. Work through all line frag rects and build the array of
  rects. */
  while (1)
    {
      /* Determine the starting x-coordinate for this line frag rect. */
      if (lf->pos <= glyphRange.location)
	{
	  /*
	  The start index is inside the line frag rect, so we need to
	  search through it to find the exact starting location.
	  */
	  unsigned int i;
	  int j;
	  linefrag_point_t *lp;
	  glyph_run_t *r;
	  unsigned int gpos, cpos;

	  for (j = 0, lp = lf->points; j < lf->num_points; j++, lp++)
	    if (lp->pos + lp->length > glyphRange.location)
	      break;

	  NSAssert(j < lf->num_points, @"can't find starting point of glyph");

	  x0 = lp->p.x + lf->rect.origin.x;
	  r = run_for_glyph_index(lp->pos, glyphs, &gpos, &cpos);
	  i = lp->pos - gpos;

	  while (i + gpos < glyphRange.location)
	    {
	      if (!r->glyphs[i].isNotShown && r->glyphs[i].g &&
		  r->glyphs[i].g != NSControlGlyph)
		{
		  x0 += r->glyphs[i].advancement.width;
		}
	      GLYPH_STEP_FORWARD(r, i, gpos, cpos)
	    }
	}
      else
	{
	  /*
	  The start index was before the this line frag, so the starting
	  x-coordinate is the left edge of the line frag.
	  */
	  x0 = NSMinX(lf->rect);
	}

      /* Determine the end x-coordinate for this line frag. */
      if (lf->pos + lf->length > last)
	{
	  /*
	  The end index is inside the line frag, so we need to find the
	  exact end location.
	  */
	  NSUInteger i;
	  int j;
	  linefrag_point_t *lp;
	  glyph_run_t *r;
	  unsigned int gpos, cpos;

	  /*
	  At this point there is a glyph in our range that is in this
	  line frag rect. If we're on the first line frag rect, it's
	  trivially true. If not, the check before the lf++; ensures it.
	  */
	  for (j = 0, lp = lf->points; j < lf->num_points; j++, lp++)
	    if (lp->pos + lp->length > last)
	      break;

	  NSAssert(j < lf->num_points, @"can't find starting point of glyph");

	  x1 = lp->p.x + lf->rect.origin.x;
	  r = run_for_glyph_index(lp->pos, glyphs, &gpos, &cpos);
	  i = lp->pos - gpos;

	  while (i + gpos < last)
	    {
	      if (!r->glyphs[i].isNotShown && r->glyphs[i].g &&
		  r->glyphs[i].g != NSControlGlyph)
		{
		  x1 += r->glyphs[i].advancement.width;
		}
	      GLYPH_STEP_FORWARD(r, i, gpos, cpos)
	    }
	}
      else if (lf->pos + lf->length == last)
	{
	  /*
	  The range ends in the last glyph of the line frag, so the end
	  x-coordinate is the right edge of this glyph. This egde is
	  equal to the right edge of the line fragment's rectangle if
	  the glyph is invisible or a control glyph, e.g., a newline,
	  and to the right edge of the line fragment's used rectangle
	  otherwise.
	  */
	  NSUInteger i;
	  glyph_run_t *r;
	  unsigned int gpos, cpos;

	  r = run_for_glyph_index(last - 1, glyphs, &gpos, &cpos);
	  i = (last - 1) - gpos;
	  if (!r->glyphs[i].isNotShown && r->glyphs[i].g &&
	      r->glyphs[i].g != NSControlGlyph)
	    x1 = NSMaxX(lf->used_rect);
	  else
	    x1 = NSMaxX(lf->rect);
	}
      else
	{
	  /*
	  The range continues beyond the end of the line frag, so the end
	  x-coordinate is the right edge of the line frag.
	  */
	  x1 = NSMaxX(lf->rect);
	}

      /*
      We have the start and end x-coordinates, and use the height of the
      line frag for the y-coordinates.
      */
      r = NSMakeRect(x0, lf->rect.origin.y, x1 - x0, lf->rect.size.height);

      /*
      As an optimization of the rectangle array, we check if the previous
      rectangle had the same x-coordinates as the new rectangle and touches
      it vertically. If so, we combine them.
      */
      if (num_rects &&
	  r.origin.x == rect_array[num_rects - 1].origin.x &&
	  r.size.width == rect_array[num_rects - 1].size.width &&
	  r.origin.y == NSMaxY(rect_array[num_rects - 1]))
	{
	  rect_array[num_rects - 1].size.height += r.size.height;
	}
      else
	{
	  if (num_rects == rect_array_size)
	    {
	      rect_array_size += 4;
	      rect_array = realloc(rect_array, sizeof(NSRect) * rect_array_size);
	    }
	  rect_array[num_rects++] = r;
	}

      if (lf->pos + lf->length >= last)
	break;
      lf++;
    }

  *rectCount = num_rects;
  return rect_array;
}

-(NSRect *) rectArrayForCharacterRange: (NSRange)charRange
	  withinSelectedCharacterRange: (NSRange)selCharRange
		       inTextContainer: (NSTextContainer *)container
			     rectCount: (NSUInteger *)rectCount
{
  NSRange r1, r2;

  // FIXME: should accept {NSNotFound, 0} for selCharRange to ignore that parameter

  /* TODO: we can actually do better than this by using the insertion point
  positioning behavior */
  r1 = [self glyphRangeForCharacterRange: charRange
	     actualCharacterRange: NULL];
  r2 = [self glyphRangeForCharacterRange: selCharRange
	     actualCharacterRange: NULL];

  return [self rectArrayForGlyphRange: r1
	       withinSelectedGlyphRange: r2
	       inTextContainer: container
	       rectCount: rectCount];
}

-(NSRect) boundingRectForGlyphRange: (NSRange)glyphRange
		    inTextContainer: (NSTextContainer *)aTextContainer
{
  NSRect *r;
  NSRect result;
  NSUInteger i, c;

/* TODO: This isn't correct. Need to handle glyphs that extend outside the
line frag rect. */
  r = [self rectArrayForGlyphRange: glyphRange
	    withinSelectedGlyphRange: NSMakeRange(NSNotFound, 0)
	    inTextContainer: aTextContainer
	    rectCount: &c];

  if (!c)
    return NSZeroRect;

  result = r[0];
  for (r++, i = 1; i < c; i++, r++)
    result = NSUnionRect(result, *r);

  return result;
}


-(NSRange) glyphRangeForBoundingRect: (NSRect)bounds
		     inTextContainer: (NSTextContainer *)container
{
  int i;
  unsigned int j;
  int low, high, mid;
  textcontainer_t *tc;
  linefrag_t *lf;

  NSRange range;


  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: invalid text container", __PRETTY_FUNCTION__);
      return NSMakeRange(0, 0);
    }

  [self _doLayoutToContainer: i
    point: NSMakePoint(NSMaxX(bounds), NSMaxY(bounds))];

  tc = textcontainers + i;

  if (!tc->num_linefrags)
    return NSMakeRange(0, 0);


  /* Find first glyph in bounds. */

  /* Find right "line", ie. the first "line" not above bounds. */
  for (low = 0, high = tc->num_linefrags - 1; low < high;)
    {
      mid = (low + high) / 2;
      lf = &tc->linefrags[mid];
      if (NSMaxY(lf->rect) > NSMinY(bounds))
	{
	  high = mid;
	}
      else
	{
	  low = mid + 1;
	}
    }

  i = low;
  lf = &tc->linefrags[i];

  if (NSMaxY(lf->rect) < NSMinY(bounds))
    {
      return NSMakeRange(0, 0);
    }

  /* Scan to first line frag intersecting bounds horizontally. */
  while (i < tc->num_linefrags - 1 &&
	 NSMinY(lf[0].rect) == NSMinY(lf[1].rect) &&
	 NSMaxX(lf[0].rect) < NSMinX(bounds))
    i++, lf++;

  /* TODO: find proper position in line frag rect */
  range.location = lf->pos;


  /* Find last glyph in bounds. */

  /* Find right "line", ie. last "line" not below bounds. */
  for (low = 0, high = tc->num_linefrags - 1; low < high;)
    {
      mid = (low + high) / 2;
      lf = &tc->linefrags[mid];
      if (NSMinY(lf->rect) > NSMaxY(bounds))
	{
	  high = mid;
	}
      else
	{
	  low = mid + 1;
	}
    }
  i = low;
  lf = &tc->linefrags[i];

  if (i && NSMinY(lf->rect) > NSMaxY(bounds))
    i--, lf--;

  if (NSMinY(lf->rect) > NSMaxY(bounds))
    {
      return NSMakeRange(0, 0);
    }

  /* Scan to last line frag intersecting bounds horizontally. */
  while (i > 0 &&
	 NSMinY(lf[0].rect) == NSMinY(lf[-1].rect) &&
	 NSMinX(lf[-1].rect) > NSMaxX(bounds))
    i--, lf--;

  /* TODO: find proper position in line frag rect */

  j = lf->pos + lf->length;
  if (j <= range.location)
    {
      return NSMakeRange(0, 0);
    }

  range.length = j - range.location;
  return range;
}

-(NSRange) glyphRangeForBoundingRectWithoutAdditionalLayout: (NSRect)bounds
					    inTextContainer: (NSTextContainer *)container
{
  /* TODO: this should be the same as
  -glyphRangeForBoundingRect:inTextContainer: but without the _doLayout...
  call.

  In other words, it returns the range of glyphs in the rect that have
  already been laid out.
  */

  return [self glyphRangeForBoundingRect: bounds
	       inTextContainer: container];
}


-(unsigned int) glyphIndexForPoint: (NSPoint)aPoint
		   inTextContainer: (NSTextContainer *)aTextContainer
{
  return [self glyphIndexForPoint: aPoint
	       inTextContainer: aTextContainer
	       fractionOfDistanceThroughGlyph: NULL];
}

/*
TODO: decide on behavior wrt. invisible glyphs and pointer far away from
anything visible
*/
-(NSUInteger) glyphIndexForPoint: (NSPoint)point
                 inTextContainer: (NSTextContainer *)container
  fractionOfDistanceThroughGlyph: (CGFloat *)partialFraction
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  linefrag_point_t *lp;
  CGFloat dummy;

  if (!partialFraction)
    partialFraction = &dummy;

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: invalid text container", __PRETTY_FUNCTION__);
      return NSNotFound;
    }

  [self _doLayoutToContainer: i  point: point];

  tc = textcontainers + i;

  /* Find the line frag rect that contains the point, and handle the case
  where the point isn't inside a line frag rect. */
  for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
    {
      /* The point is inside a rect; we're done. */
      if (NSPointInRect(point, lf->rect))
        break;

      /* If the current line frag rect is below the point, the point must
      be between the line with the current line frag rect and the line
      with the previous line frag rect. */
      if (NSMinY(lf->rect) > point.y)
        {
          /* If this is not the first line frag rect in the text container,
             we consider the point to be after the last glyph on the previous
             line. Otherwise, we consider it to be before the first glyph on
             the current line. */
          if (i > 0)
            {
              *partialFraction = 1.0;
              if (lf->pos == 0)
                {
                  return NSNotFound;
                }
              else
                {
                  return lf->pos - 1;
                }
            }
          else
            {
              *partialFraction = 0.0;
              return lf->pos;
            }
        }
      /* We know that NSMinY(lf->rect) <= point.y. If the point is on the
      current line and to the left of the current line frag rect, we
      consider the point to be before the first glyph in the current line
      frag rect.

      (This will happen if the point is between two line frag rects, or
      before the first line frag rect. If the point is to the right of the
      current line frag rect, it will be inside a subsequent line frag rect
      on this line, or to the left of one, which will be handled by the here
      or by the first check in the loop, or it will be after all line frag
      rects on the line, which will be detected and handled as a 'between
      two lines' case, or by the 'after all line frags' code below.)
      */
      if (NSMaxY(lf->rect) >= point.y && NSMinX(lf->rect) > point.x)
        {
          *partialFraction = 0.0;
          return lf->pos;
        }
    }

  /* Point is after all line frags. */
  if (i == tc->num_linefrags)
    {
      *partialFraction = 1.0;
      /* TODO: this should return the correct thing even if the container is empty */
      if (tc->pos + tc->length == 0)
        {
          return NSNotFound;
        }
      else
        {
          return tc->pos + tc->length - 1;
        }
    }

  /* only interested in x from here on */
  point.x -= lf->rect.origin.x;

  /* scan to the first point beyond the target */
  for (i = 0, lp = lf->points; i < lf->num_points; i++, lp++)
    if (lp->p.x > point.x)
      break;

  if (!i)
    {
      /* Before the first glyph on the line. */
      /* TODO: what if it isn't shown? */
      *partialFraction = 0;
      return lp->pos;
    }
  else
    {
      /* There are points in this line frag before the point we're looking
      for. */
      float cur, prev, next;
      glyph_run_t *r;
      unsigned int glyph_pos, char_pos, last_visible;
      unsigned j;

      if (i < lf->num_points)
        next = lp->p.x;
      else
        next = NSMinX(lf->rect);

      lp--; /* Valid since we checked for !i above. */
      r = run_for_glyph_index(lp->pos, glyphs, &glyph_pos, &char_pos);

      prev = lp->p.x;

      last_visible = lf->pos;
      for (j = lp->pos - glyph_pos; j + glyph_pos < lp->pos + lp->length;)
        {
          // Don't ignore invisble glyphs.
          // if (r->glyphs[j].isNotShown || r->glyphs[j].g == NSControlGlyph ||
          if (!r->glyphs[j].g)
            {
              GLYPH_STEP_FORWARD(r, j, glyph_pos, char_pos)
              continue;
            }
          last_visible = j + glyph_pos;

          cur = prev + r->glyphs[j].advancement.width;
          if (j + glyph_pos + 1 == lp->pos + lp->length && next > cur)
            cur = next;

          if (cur >= point.x)
            {
              *partialFraction = (point.x - prev) / (cur - prev);
              if (*partialFraction < 0)
                *partialFraction = 0;
              return j + glyph_pos;
            }
          prev = cur;
          GLYPH_STEP_FORWARD(r, j, glyph_pos, char_pos)
        }
      *partialFraction = 1;
      return last_visible;
    }
}


/*** Insertion point positioning and movement. ***/

/*
Determines at which glyph, and how far through it, the insertion point
should be placed for a certain character index.
*/
-(unsigned int) _glyphIndexForCharacterIndex: (unsigned int)cindex
			     fractionThrough: (float *)fraction
{
  if (cindex == [[_textStorage string] length])
    {
      *fraction = 0.0;
      return (unsigned int)-1;
    }
  else
    {
      NSRange glyphRange, charRange;
      unsigned int glyph_index;
      float fraction_through;

      glyphRange = [self glyphRangeForCharacterRange: NSMakeRange(cindex, 1)
				actualCharacterRange: &charRange];

      /*
      Deal with composite characters and ligatures.

      We determine how far through the character range this character is a
      part of the character is, and then determine the glyph index and
      fraction that is the same distance through the glyph range it is
      mapped to.

      (This gives good behavior when dealing with ligatures, at least.)

      Eg. if the character index is at character 3 in a 5 character range,
      we are 3/5=0.6 through the entire character range. If this range was
      mapped to 4 glyphs, we get 0.6*4=2.4, so the glyph index is 2 and
      the fraction is 0.4.
      */
      fraction_through = (cindex - charRange.location) / (float)charRange.length;
      fraction_through *= glyphRange.length;

      glyph_index = glyphRange.location + floor(fraction_through);
      fraction_through -= floor(fraction_through);

      *fraction = fraction_through;
      return glyph_index;
    }
}

/*
Note: other methods rely a lot on the fact that the rectangle returned here
has the same y origin and height as the line frag rect it is in.
*/
-(NSRect) _insertionPointRectForCharacterIndex: (unsigned int)cindex
				 textContainer: (int *)textContainer
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  float x0, x1;
  NSRect r;

  unsigned int glyph_index;
  float fraction_through;


  glyph_index = [self _glyphIndexForCharacterIndex: cindex
				   fractionThrough: &fraction_through];
  if (glyph_index == (unsigned int)-1)
    {
      /* Need complete layout information. */
      [self _doLayout];
      if (extra_textcontainer)
	{
	  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
	    if (tc->textContainer == extra_textcontainer)
	      break;
	  NSAssert(i < num_textcontainers, @"invalid extraTextContainer");
	  *textContainer = i;
	  r = extra_rect;
	  r.size.width = 1;
	  return r;
	}
      glyph_index = [self numberOfGlyphs] - 1;
      if (glyph_index == (unsigned int)-1)
	{ /* No information is available. Get default font height. */
	  NSFont *f = [_typingAttributes objectForKey: NSFontAttributeName];

	  /* will be -1 if there are no text containers */
	  *textContainer = num_textcontainers - 1;
	  r = NSMakeRect(0, 0, 1, [f boundingRectForFont].size.height);
	  if (num_textcontainers > 0)
	    {
              NSParagraphStyle *paragraph = [_typingAttributes objectForKey: NSParagraphStyleAttributeName];
              NSTextAlignment alignment = [paragraph alignment];

	      tc = textcontainers + num_textcontainers - 1;
	      r.origin.x += [tc->textContainer lineFragmentPadding];

              // Apply left/right/center justification...
              if (alignment == NSRightTextAlignment)
                {
                  r.origin.x += [tc->textContainer containerSize].width;
                }
              else if (alignment == NSCenterTextAlignment)
                {
                  r.origin.x += [tc->textContainer containerSize].width / 2;
                }
	    }
	  return r;
	}
      fraction_through = 1.0;
    }
  else
    [self _doLayoutToGlyph: glyph_index];

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyph_index)
      break;
  if (i == num_textcontainers)
    {
      *textContainer = -1;
      return NSZeroRect;
    }

  *textContainer = i;

  LINEFRAG_FOR_GLYPH(glyph_index);

  /* Special case if we are asked for the insertion point rectangle at the
     end of text, since the standard code yields an incorrect result if the
     last line fragment ends with an invisible character (e.g., a tab).
     Note that fraction_through is always less than 1 except when
     -_glyphIndexForCharacterIndex:fractionThrough: is called for
     cindex == [_textStorage length], in which case we set it to 1. */
  if (fraction_through == 1.0)
    {
      r = (lf == 0) ? NSZeroRect : lf->used_rect;
      r.origin.x += r.size.width;
      r.size.width = 1;
      return r;
    }

  {
    unsigned int i;
    int j;
    linefrag_point_t *lp;
    glyph_run_t *r;
    unsigned int gpos, cpos;

    for (j = 0, lp = lf->points; j < lf->num_points; j++, lp++)
      if (lp->pos + lp->length > glyph_index)
	break;

    x0 = lp->p.x + lf->rect.origin.x;
    r = run_for_glyph_index(lp->pos, glyphs, &gpos, &cpos);
    i = lp->pos - gpos;

    while (i + gpos < glyph_index)
      {
	if (!r->glyphs[i].isNotShown && r->glyphs[i].g &&
	    r->glyphs[i].g != NSControlGlyph)
	  {
	    x0 += r->glyphs[i].advancement.width;
	  }
	GLYPH_STEP_FORWARD(r, i, gpos, cpos)
      }
    x1 = x0;
    if (!r->glyphs[i].isNotShown && r->glyphs[i].g &&
	r->glyphs[i].g != NSControlGlyph)
      {
	x1 += r->glyphs[i].advancement.width;
      }
  }

  r = lf->rect;
  r.origin.x = x0 + (x1 - x0) * fraction_through;
  r.size.width = 1;

  return r;
}

-(NSRect) insertionPointRectForCharacterIndex: (unsigned int)cindex
			      inTextContainer: (NSTextContainer *)textContainer
{
  int i;
  NSRect r;

  r = [self _insertionPointRectForCharacterIndex: cindex
				   textContainer: &i];
  if (i == -1 || textcontainers[i].textContainer != textContainer)
    return NSZeroRect;

  return r;
}


-(unsigned int) characterIndexMoving: (GSInsertionPointMovementDirection)direction
		  fromCharacterIndex: (unsigned int)from
	      originalCharacterIndex: (unsigned int)original
			    distance: (float)distance
{
  NSRect from_rect, new_rect;
  int from_tc, new_tc;
  unsigned int new;
  unsigned int length = [_textStorage length];

  /* This call will ensure that layout is built to 'from', and that layout
  for the line 'from' is in is built. */
  from_rect = [self _insertionPointRectForCharacterIndex: from
					   textContainer: &from_tc];
  if (from_tc == -1)
    {
      /* The from character index is not in a text container, so move the
	 cursor to the start of the text. */
      return 0;
    }

  /* Simple case which moves one character left/right or one line up/down,
     but supports moving between text containers. */
  if (distance == 0.0)
    {
      if (direction == GSInsertionPointMoveLeft ||
	  direction == GSInsertionPointMoveRight)
	{
	  new = from;
	  if (direction == GSInsertionPointMoveLeft && new > 0)
	    new--;
	  if (direction == GSInsertionPointMoveRight && new < length)
	    new++;
	  
	  return new;
	}
      else if (direction == GSInsertionPointMoveUp ||
	       direction == GSInsertionPointMoveDown)
	{
	  int orig_tc;
	  const float target = [self _insertionPointRectForCharacterIndex: original
							    textContainer: &orig_tc].origin.x;

	  const int delta = (direction == GSInsertionPointMoveUp) ? -1 : 1;

	  /* First scan forward or backwards until we end up on a new line */
	  for (new = from; (direction == GSInsertionPointMoveUp) ? (new > 0) : (new < length); new += delta)
	    {
	      new_rect = [self _insertionPointRectForCharacterIndex: new
						      textContainer: &new_tc];
	      if (new_rect.origin.y != from_rect.origin.y || new_tc != from_tc)
		break;
	    }

	  /* We found the start of the line, now find the target character on that line. */
	  new_rect = [self _insertionPointRectForCharacterIndex: new
						  textContainer: &new_tc];
	  while ((direction == GSInsertionPointMoveUp) ? (new > 0) : (new < length))
	    {
	      int prev_tc = new_tc;
	      NSRect prev_rect = new_rect;
	      new_rect = [self _insertionPointRectForCharacterIndex: new + delta
						      textContainer: &new_tc];

	      /* 'new+delta' is on a different line than the target line, so the 
		 target character must be 'new'.*/
	      if (new_rect.origin.y != prev_rect.origin.y || new_tc != prev_tc)
		return new;

	      /* If we pass the target point, the character we want is either
		 'new' or 'new+delta' */
	      if ((direction == GSInsertionPointMoveDown && NSMinX(new_rect) >= target)
		  || (direction == GSInsertionPointMoveUp && NSMinX(new_rect) <= target))
		{
		  if (fabs(NSMinX(new_rect) - target) > fabs(NSMinX(prev_rect) - target))
		    return new;
		  
		  return new + delta;
		}

	      new += delta;
	    }
	  return new;
	}
    }

  /* The following more complex cases are for when a minimum distance is specified.
     However, they will not move out of from's text container.
  */

  if (direction == GSInsertionPointMoveLeft ||
      direction == GSInsertionPointMoveRight)
    {
      float target;

      /*
      This is probably very inefficient, but it shouldn't be a bottleneck,
      and it guarantees that insertion point movement matches insertion point
      positioning. It also lets us do this by character instead of by glyph.
      */
      new = from;
      if (direction == GSInsertionPointMoveLeft)
	{
	  target = from_rect.origin.x - distance;
	  while (new > 0)
	    {
	      new_rect = [self _insertionPointRectForCharacterIndex: new - 1
						      textContainer: &new_tc];
	      if (new_tc != from_tc)
		break;
	      if (new_rect.origin.y != from_rect.origin.y)
		break;
	      new--;
	      if (NSMaxX(new_rect) <= target)
		break;
	    }
	  return new;
	}
      else
	{
	  target = from_rect.origin.x + distance;
	  while (new < length)
	    {
	      new_rect = [self _insertionPointRectForCharacterIndex: new + 1
						      textContainer: &new_tc];
	      if (new_tc != from_tc)
		break;
	      if (new_rect.origin.y != from_rect.origin.y)
		break;
	      new++;
	      if (NSMinX(new_rect) >= target)
		break;
	    }
	  return new;
	}
    }

  if (direction == GSInsertionPointMoveUp ||
      direction == GSInsertionPointMoveDown)
    {
      NSRect orig_rect, prev_rect;
      int orig_tc;
      float target;
      textcontainer_t *tc;
      linefrag_t *lf;
      int i;

      orig_rect = [self _insertionPointRectForCharacterIndex: original
					       textContainer: &orig_tc];
      if (orig_tc == from_tc)
	target = orig_rect.origin.x;
      else
	target = from_rect.origin.x;

      tc = &textcontainers[from_tc];
      /* Find first line frag rect on the from line. */
      for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
	{
	  if (lf->rect.origin.y == from_rect.origin.y)
	    break;
	}

      /* If we don't have a line frag rect that matches the from position,
      the from position is probably on the last line, in the extra rect,
      and i == tc->num_linefrags. The movement direction specific code
      handles this case, as long as tc->num_linefrags > 0. */
      if (!tc->num_linefrags)
	return from; /* Impossible? Should be, since from_tc!=-1. */

      if (direction == GSInsertionPointMoveDown)
	{
	  [self _doLayoutToContainer: from_tc
		point: NSMakePoint(target, distance + NSMaxY(from_rect))];
	  tc = textcontainers + from_tc;
	  /* Find the target line. Move at least (should be up to?)
	  distance, and at least one line. */
	  for (; i < tc->num_linefrags; i++, lf++)
	    if (NSMaxY(lf->rect) >= distance + NSMaxY(from_rect) &&
	        NSMinY(lf->rect) != NSMinY(from_rect))
	      break;

	  if (i == tc->num_linefrags)
	    {
	      /* Special case for moving into the extra line at the end */
	      if (tc->textContainer == extra_textcontainer &&
		  NSMaxY(extra_rect) >= distance + NSMaxY(from_rect) &&
		  NSMinY(extra_rect) != NSMinY(from_rect))
		return length;

	      /* We can't move as far as we want to. In fact, we might not
	      have been able to move at all.
	      TODO: figure out how to handle this
	      */
	      return from;
	    }
	}
      else
	{
	  if (i == tc->num_linefrags)
	    i--, lf--;
	  /* Find the target line. Move at least (should be up to?)
	  distance, and at least one line. */
	  for (; i >= 0; i--, lf--)
	    if (NSMinY(lf->rect) <= NSMinY(from_rect) - distance &&
	        NSMinY(lf->rect) != NSMinY(from_rect))
	      break;
	  /* Now we have the last line frag of the target line. Move
	  backwards to the first one. */
	  for (; i > 0; i--, lf--)
	    if (NSMinY(lf->rect) != NSMinY(lf[-1].rect))
	      break;

	  if (i == -1)
	    {
	      /* We can't move as far as we want to. In fact, we might not
	      have been able to move at all.
	      TODO: figure out how to handle this
	      */
	      return from;
	    }
	}

      /* Now we have the first line frag of the target line and the
      target x position. */
      new = [self characterRangeForGlyphRange: NSMakeRange(lf->pos, 1)
			     actualGlyphRange: NULL].location;

      /* The first character index might not actually be in this line
      rect, so move forwards to the first character in the target line. */
      while (new < length)
	{
	  new_rect = [self _insertionPointRectForCharacterIndex: new + 1
						  textContainer: &new_tc];
	  if (new_tc > from_tc)
	    break;
	  if (new_rect.origin.y >= lf->rect.origin.y)
	    break;
	  new++;
	}

      /* Now find the target character in the line. */
      new_rect = [self _insertionPointRectForCharacterIndex: new
					      textContainer: &new_tc];
      while (new < length)
	{
	  prev_rect = new_rect;
	  new_rect = [self _insertionPointRectForCharacterIndex: new + 1
						  textContainer: &new_tc];
	  if (new_tc != from_tc)
	    break;
	  if (new_rect.origin.y != lf->rect.origin.y)
	    break;
	  if (NSMinX(new_rect) >= target)
	    {
	      /*
	      'new+1' is beyond 'target', so either 'new' or 'new+1' is the
	      character we want. Pick the closest one. (Note that 'new' might
	      also be beyond 'target'.)
	      */
	      if (fabs(NSMinX(new_rect) - target) < fabs(NSMinX(prev_rect) - target))
		new++;
	      return new;
	    }
	  new++;
	}
      return new;
    }

  NSLog(@"(%s): invalid direction %i (distance %g)",
    __PRETTY_FUNCTION__, direction, distance);
  return from;
}

- (void) ensureGlyphsForGlyphRange: (NSRange)glyphRange
{
  [self _generateGlyphsUpToGlyph: NSMaxRange(glyphRange) - 1];
}

- (void) ensureGlyphsForCharacterRange: (NSRange)charRange
{
  [self _generateGlyphsUpToCharacter: NSMaxRange(charRange) - 1];
}

- (void) ensureLayoutForGlyphRange: (NSRange)glyphRange
{
  [self _doLayoutToGlyph: NSMaxRange(glyphRange) - 1];
}

- (void) ensureLayoutForCharacterRange: (NSRange)charRange
{
  NSRange glyphRange;

  glyphRange = [self glyphRangeForCharacterRange: charRange 
                            actualCharacterRange: NULL];
  [self ensureLayoutForGlyphRange: glyphRange];
}

- (void) ensureLayoutForTextContainer: (NSTextContainer*)container
{
  int i;
  textcontainer_t *tc;
  NSSize size;

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: invalid text container", __PRETTY_FUNCTION__);
      return;
    }

  size = [container containerSize];
  [self _doLayoutToContainer: i
                       point: NSMakePoint(size.width, size.height)];
}

- (void) ensureLayoutForBoundingRect: (NSRect)bounds
                     inTextContainer: (NSTextContainer*)container
{
  int i;
  textcontainer_t *tc;

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: invalid text container", __PRETTY_FUNCTION__);
      return;
    }

  [self _doLayoutToContainer: i
                       point: NSMakePoint(NSMaxX(bounds), NSMaxY(bounds))];
}

- (void) invalidateLayoutForCharacterRange: (NSRange)charRange
                      actualCharacterRange: (NSRangePointer)actualCharRange
{
  [self invalidateLayoutForCharacterRange: charRange
                                   isSoft: NO
                     actualCharacterRange: actualCharRange];
}

- (void) invalidateGlyphsOnLayoutInvalidationForGlyphRange: (NSRange)glyphRange
{
  // FIXME
}

- (BOOL) allowsNonContiguousLayout
{
  return NO;
}

- (void) setAllowsNonContiguousLayout: (BOOL)flag;
{
}

- (BOOL) hasNonContiguousLayout;
{
  return NO;
}

@end




@implementation NSLayoutManager (drawing)


/** Drawing **/

/*
If a range passed to a drawing function isn't contained in the text
container that contains its first glyph, the range is silently clamped.
My thought with this is that the requested glyphs might not fit in the
text container (if it's the last text container, or there's only one).
In that case, it isn't really the caller's fault, and drawing as much as
will fit in the text container makes sense.

TODO: reconsider silently clamping ranges in these methods; might
want to make sure we don't do it if part of the range is in a second
container
*/

-(void) drawBackgroundForGlyphRange: (NSRange)range
			    atPoint: (NSPoint)containerOrigin
{
  NSTextContainer *textContainer;
  glyph_run_t *glyph_run;
  unsigned int glyph_pos, char_pos, first_char_pos;
  int i, j;
  NSRect *rects;
  NSUInteger count;
  NSColor *color = nil;
  NSColor *last_color = nil;

  NSGraphicsContext *ctxt = GSCurrentContext();


  if (!range.length)
    return;
  [self _doLayoutToGlyph: range.location + range.length - 1];

  {
    int i;
    textcontainer_t *tc;

    for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
      if (tc->pos + tc->length > range.location)
	break;
    if (i == num_textcontainers)
      {
	NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
	return;
      }

    if (range.location + range.length > tc->pos + tc->length)
      range.length = tc->pos + tc->length - range.location;

    textContainer = tc->textContainer;
  }

  glyph_run = run_for_glyph_index(range.location, glyphs, &glyph_pos, &char_pos);
  i = range.location - glyph_pos;
  first_char_pos = char_pos;
  while ((glyph_run != NULL) && (i + glyph_pos < range.location + range.length))
    {
      NSRange r = NSMakeRange(glyph_pos + i, glyph_run->head.glyph_length - i);

      if (NSMaxRange(r) > NSMaxRange(range))
        {
          r.length = NSMaxRange(range) - r.location;
        }

      color = [_textStorage attribute: NSBackgroundColorAttributeName
			      atIndex: char_pos
		       effectiveRange: NULL];
      if (color)
	{
	  rects = [self rectArrayForGlyphRange: r
		      withinSelectedGlyphRange: NSMakeRange(NSNotFound, 0)
			       inTextContainer: textContainer
				     rectCount: &count];

	  if (count)
	    {
	      if (last_color != color)
		{
		  [color set];
		  last_color = color;
		}
	      for (j = 0; j < count; j++, rects++)
		{
		  DPSrectfill(ctxt,
			      rects->origin.x + containerOrigin.x,
			      rects->origin.y + containerOrigin.y,
			      rects->size.width, rects->size.height);
		}
	    }
	}

      glyph_pos += glyph_run->head.glyph_length;
      char_pos += glyph_run->head.char_length;
      i = 0;
      glyph_run = (glyph_run_t *)glyph_run->head.next;
    }

  if (!_selected_range.length || _selected_range.location == NSNotFound)
    return;

  if (_selected_range.location >= char_pos
    || _selected_range.location + _selected_range.length <= first_char_pos)
    {
      return;
    }

  /* The selection (might) intersect our glyph range. */
  {
    NSRange r = [self glyphRangeForCharacterRange: _selected_range
		  actualCharacterRange: NULL];
    NSRange sel = r;
    NSTextView *ftv;

    if (r.location < range.location)
      {
	if (range.location - r.location > r.length)
	  return;
	r.length -= range.location - r.location;
	r.location = range.location;
      }
    if (r.location + r.length > range.location + range.length)
      {
	if (r.location > range.location + range.length)
	  return;
	r.length = range.location + range.length - r.location;
      }

    /* Use the text view's selected text attributes */
    if ((ftv = [self textViewForBeginningOfSelection]))
      color = [[ftv selectedTextAttributes] 
      	objectForKey: NSBackgroundColorAttributeName];

    if (!color)
      color = [NSColor selectedTextBackgroundColor];

    if (!color)
      return;

    rects = [self rectArrayForGlyphRange: r
	      withinSelectedGlyphRange: sel
	      inTextContainer: textContainer
	      rectCount: &count];

    if (count)
      {
	[color set];
	for (j = 0; j < count; j++, rects++)
	  {
	    DPSrectfill(ctxt,
			rects->origin.x + containerOrigin.x,
			rects->origin.y + containerOrigin.y,
			rects->size.width, rects->size.height);
	  }
      }
  }
}

static inline NSSize
attachmentSize(linefrag_t *lf, NSUInteger glyphIndex)
{
  linefrag_attachment_t *la;
  int la_i;

  la = lf->attachments;
  la_i = 0;

  if (la)
    {
      while (la->pos != glyphIndex && la_i < lf->num_attachments)
        {
          la++;
          la_i++;
        }
    }

  if (la_i >= lf->num_attachments)
    return NSMakeSize(-1.0, -1.0);
  
  return la->size;
}

- (NSSize) attachmentSizeForGlyphAtIndex: (NSUInteger)glyphIndex
{
  textcontainer_t *tc;
  int i;
  linefrag_t *lf;

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyphIndex)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeSize(-1.0, -1.0);
    }

  LINEFRAG_FOR_GLYPH(glyphIndex);

  return attachmentSize(lf, glyphIndex);
}

- (void) showAttachmentCell: (NSCell *)cell
		     inRect: (NSRect)rect
	     characterIndex: (NSUInteger)attachmentIndex
{
  [(id <NSTextAttachmentCell>)cell drawWithFrame: rect
                              inView: [NSView focusView]
                              characterIndex: attachmentIndex
                              layoutManager: self];
}

-(void) drawGlyphsForGlyphRange: (NSRange)range
			atPoint: (NSPoint)containerOrigin
{
  int i, j;
  textcontainer_t *tc;
  linefrag_t *lf;
  linefrag_point_t *lp;

  NSPoint p;
  unsigned int g;

  NSDictionary *attributes;
  NSFont *f;
  NSColor *color, *run_color;
  NSRange selectedGlyphRange;
  BOOL currentGlyphIsSelected;

  glyph_run_t *glyph_run;
  unsigned int glyph_pos, char_pos;
  glyph_t *glyph;

  NSGraphicsContext *ctxt = GSCurrentContext();

  /*
  For performance, it might (if benchmarks or profiling backs it up) be
  worthwhile to cache this across calls to this method. However, this
  color can change at runtime, so care would have to be taken to keep the
  cache in sync with the actual color.
  */
  NSColor *defaultTextColor = [NSColor textColor];
  NSColor *selectedTextColor = defaultTextColor;
  NSColor *link_color = nil;
  id linkValue;
 
#define GBUF_SIZE 16 /* TODO: tweak */
  NSGlyph gbuf[GBUF_SIZE];
  NSSize advancementbuf[GBUF_SIZE];
  int gbuf_len, gbuf_size;
  NSPoint gbuf_point = NSZeroPoint;

  if (!range.length)
    return;
  [self _doLayoutToGlyph: range.location + range.length - 1];

  /* Find the selected range of glyphs as it overlaps with the range we
   * are about to display.
   */
  if (_selected_range.length == 0)
    {
      selectedGlyphRange.location = 0;
      selectedGlyphRange.length = 0;
    }
  else
    {
      selectedGlyphRange = [self glyphRangeForCharacterRange: _selected_range
	actualCharacterRange: 0];
    }
  selectedGlyphRange = NSIntersectionRange(selectedGlyphRange, range);

  if ([ctxt isDrawingToScreen])
    gbuf_size = GBUF_SIZE;
  else
    gbuf_size = 1;

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > range.location)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return;
    }

  if (range.location + range.length > tc->pos + tc->length)
    range.length = tc->pos + tc->length - range.location;

  LINEFRAG_FOR_GLYPH(range.location);

  j = 0;
  lp = lf->points;
  while (lp->pos + lp->length < range.location)
    lp++, j++;

  glyph_run = run_for_glyph_index(lp->pos, glyphs, &glyph_pos, &char_pos);
  currentGlyphIsSelected = NSLocationInRange(lp->pos, selectedGlyphRange);
  glyph = glyph_run->glyphs + lp->pos - glyph_pos;
  attributes = [_textStorage attributesAtIndex: char_pos
				effectiveRange: NULL];
  run_color = [attributes valueForKey: NSForegroundColorAttributeName];
  if (run_color == nil)
    run_color = defaultTextColor;

  linkValue = [attributes objectForKey: NSLinkAttributeName];
  if (linkValue != nil)
    {
      if (link_color == nil)
	{
	  NSDictionary *link_attributes = [[self firstTextView] linkTextAttributes];
	  link_color = [link_attributes valueForKey: NSForegroundColorAttributeName];
	}
      if (link_color != nil)
	run_color = link_color;
    }

  if (selectedGlyphRange.length > 0)
    {
      /* Get the text view's color setting for selected text as we will
       * be needing to draw some selected glyphs.
       */
      selectedTextColor = [[[self textViewForBeginningOfSelection]
	selectedTextAttributes] objectForKey: NSForegroundColorAttributeName];

      /* FIXME ... should we fall back to using selectedTextColor or 
       * defaultTextColor?
       */
      if (selectedTextColor == nil)
	{
	  selectedTextColor = [NSColor selectedTextColor];
	}
    }

  color = (currentGlyphIsSelected ? selectedTextColor : run_color);
  [color set];
  f = glyph_run->font;
  [f set];

  p = lp->p;
  p.x += lf->rect.origin.x + containerOrigin.x;
  p.y += lf->rect.origin.y + containerOrigin.y;
  gbuf_len = 0;
  for (g = lp->pos; g < range.location + range.length; g++, glyph++)
    {
      if (currentGlyphIsSelected != NSLocationInRange(g, selectedGlyphRange))
	{
	  /* When we change between drawing selected and unselected glyphs
	   * we must flush any glyphs from the buffer and change trhe color
	   * we use for the text.
	   */
	  if (gbuf_len)
	    {
	      DPSmoveto(ctxt, gbuf_point.x, gbuf_point.y);
	      GSShowGlyphsWithAdvances(ctxt, gbuf, advancementbuf, gbuf_len);
	      DPSnewpath(ctxt);
	      gbuf_len = 0;
	    }
	  if (currentGlyphIsSelected == YES)
	    {
	      currentGlyphIsSelected = NO;
	      if (color != run_color)
	        {
		  color = run_color;
		  [color set];
		}
	    }
	  else
	    {
	      currentGlyphIsSelected = YES;
	      if (color != selectedTextColor)
	        {
		  color = selectedTextColor;
		  [color set];
		}
	    }
	}

      if (g == lp->pos + lp->length)
	{
	  if (gbuf_len)
	    {
	      DPSmoveto(ctxt, gbuf_point.x, gbuf_point.y);
	      GSShowGlyphsWithAdvances(ctxt, gbuf, advancementbuf, gbuf_len);
	      DPSnewpath(ctxt);
	      gbuf_len = 0;
	    }
	  j++;
	  lp++;
	  if (j == lf->num_points)
	    {
	      i++;
	      lf++;
	      j = 0;
	      lp = lf->points;
	    }
	  p = lp->p;
	  p.x += lf->rect.origin.x + containerOrigin.x;
	  p.y += lf->rect.origin.y + containerOrigin.y;
	}
      if (g == glyph_pos + glyph_run->head.glyph_length)
	{
	  glyph_pos += glyph_run->head.glyph_length;
	  char_pos += glyph_run->head.char_length;
	  glyph_run = (glyph_run_t *)glyph_run->head.next;
	  attributes = [_textStorage attributesAtIndex: char_pos
				     effectiveRange: NULL];
	  run_color = [attributes valueForKey: NSForegroundColorAttributeName];
	  if (run_color == nil)
	    {
	      run_color = defaultTextColor;
	    }

	  linkValue = [attributes objectForKey: NSLinkAttributeName];
	  if (linkValue != nil)
	      {
		if (link_color == nil)
		  {
		    NSDictionary *link_attributes = [[self firstTextView] linkTextAttributes];
		    link_color = [link_attributes valueForKey: NSForegroundColorAttributeName];
		  }
		if (link_color != nil)
		  run_color = link_color;
	      }

	  glyph = glyph_run->glyphs;

	  /* If the font has changed or the color has changed (and we are
	   * not drawing using the selected text color) then we must flush
	   * any buffered glyphs and set the new font and color.
	   */
	  if (glyph_run->font != f
	    || (currentGlyphIsSelected == NO && run_color != color))
	    {
	      if (gbuf_len)
		{
		  DPSmoveto(ctxt, gbuf_point.x, gbuf_point.y);
		  GSShowGlyphsWithAdvances(ctxt, gbuf, advancementbuf, gbuf_len);
		  DPSnewpath(ctxt);
		  gbuf_len = 0;
		}
	      if (f != glyph_run->font)
		{
		  f = glyph_run->font;
		  [f set];
		}
	      if (currentGlyphIsSelected == NO && run_color != color)
		{
		  color = run_color;
		  [color set];
		}
	    }
	}
      if (!glyph->isNotShown && glyph->g && glyph->g != NSControlGlyph)
	{
	  if (glyph->g == GSAttachmentGlyph)
	    {
	      if (g >= range.location)
		{
		  unsigned int char_index =
		    [self characterRangeForGlyphRange: NSMakeRange(g, 1)
				     actualGlyphRange: NULL].location;
		  id<NSTextAttachmentCell> cell = [[_textStorage attribute: NSAttachmentAttributeName
			atIndex: char_index
			effectiveRange: NULL] attachmentCell];
		  NSRect cellFrame;

		  cellFrame.origin = p;
		  cellFrame.size = attachmentSize(lf, g);
		  cellFrame.origin.y -= cellFrame.size.height;

		  /* Silently ignore if we don't have any size information for
		     it. */
		  if (NSEqualSizes(cellFrame.size, NSMakeSize(-1.0, -1.0)))
		    continue;

		  /* Drawing the cell might mess up our state, so we reset
		  the font and color afterwards. */
		  /* TODO:
		  optimize this?
		  collect attachments and draw them in bunches of eg. 4?

		  probably not worth effort. better to optimize font and
		  color setting :)

		  should they really be drawn in our coordinate system?
		  */
		  [self showAttachmentCell: (NSCell*)cell
				    inRect: cellFrame
                            characterIndex: char_index];
		  [f set];
		  [color set];
		}
	      continue;
	    }
	  if (g >= range.location)
	    {
	      if (!gbuf_len)
		{
		  gbuf[0] = glyph->g;
		  advancementbuf[0] = glyph->advancement;
		  gbuf_point = p;
		  gbuf_len = 1;
		}
	      else
		{
		  if (gbuf_len == gbuf_size)
		    {
		      DPSmoveto(ctxt, gbuf_point.x, gbuf_point.y);
		      GSShowGlyphsWithAdvances(ctxt, gbuf, advancementbuf, gbuf_size);
		      DPSnewpath(ctxt);
		      gbuf_len = 0;
		      gbuf_point = p;
		    }
		  gbuf[gbuf_len] = glyph->g;
		  advancementbuf[gbuf_len] = glyph->advancement;
		  gbuf_len++;
		}
	    }
	  p.x += glyph->advancement.width;
	}
    }
  if (gbuf_len)
    {
/*int i;
printf("%i at (%g %g) 4\n", gbuf_len, gbuf_point.x, gbuf_point.y);
for (i = 0; i < gbuf_len; i++) printf("   %3i : %04x\n", i, gbuf[i]); */
      DPSmoveto(ctxt, gbuf_point.x, gbuf_point.y);
      GSShowGlyphsWithAdvances(ctxt, gbuf, advancementbuf, gbuf_len);
      DPSnewpath(ctxt);
    }

#undef GBUF_SIZE

  // Draw underline where necessary
  // FIXME: Also draw strikeout
  {
    const NSRange characterRange = [self characterRangeForGlyphRange: range
						    actualGlyphRange: NULL];
    id linkUnderlineValue = nil;

    for (i=characterRange.location; i<NSMaxRange(characterRange); )
      {
	NSRange underlinedCharacterRange;
	NSRange linkCharacterRange;
	id underlineValue = nil;

	linkValue = [_textStorage attribute: NSLinkAttributeName
				    atIndex: i
		      longestEffectiveRange: &linkCharacterRange
				    inRange: characterRange];
	if (linkValue != nil)
	  {
	    if (linkUnderlineValue == nil)
	      {
		NSDictionary *link_attributes = [[self firstTextView] linkTextAttributes];
		linkUnderlineValue = [link_attributes valueForKey: NSUnderlineStyleAttributeName];
	      }
	    underlineValue = linkUnderlineValue;
	    underlinedCharacterRange = linkCharacterRange;
	  }
	else
	  {
	    underlineValue = [_textStorage attribute: NSUnderlineStyleAttributeName
					     atIndex: i
			       longestEffectiveRange: &underlinedCharacterRange
					     inRange: characterRange];
	    underlinedCharacterRange = NSIntersectionRange(underlinedCharacterRange, 
							   linkCharacterRange);
	  }
	if (underlineValue != nil && [underlineValue integerValue] != NSUnderlineStyleNone)
	  {
	    const NSRange underlinedGylphRange = [self glyphRangeForCharacterRange: underlinedCharacterRange
							      actualCharacterRange: NULL];
	  
	    // we have a range of glpyhs that need underlining, which might span
	    // multiple line fragments, so we need to iterate though the line fragments
	    for (j=underlinedGylphRange.location; j<NSMaxRange(underlinedGylphRange); )
	      {
		NSRange lineFragmentGlyphRange;
		const NSRect lineFragmentRect = [self lineFragmentRectForGlyphAtIndex: j
								       effectiveRange: &lineFragmentGlyphRange];
		const NSRange rangeToUnderline = NSIntersectionRange(underlinedGylphRange, lineFragmentGlyphRange);

		[self underlineGylphRange: rangeToUnderline
			    underlineType: [underlineValue integerValue]
			 lineFragmentRect: lineFragmentRect
		   lineFragmentGlyphRange: lineFragmentGlyphRange
			  containerOrigin: containerOrigin];

		j = NSMaxRange(rangeToUnderline);
	      }
	  }
	i += underlinedCharacterRange.length;
      }
  
    // Draw spelling state (i.e. red underline for misspelled words)

    if ([NSGraphicsContext currentContextDrawingToScreen])
      {
	for (i=characterRange.location; i<NSMaxRange(characterRange); )
	  {
	    NSRange underlinedCharacterRange;
	    id underlineValue = [self temporaryAttribute: NSSpellingStateAttributeName
					atCharacterIndex: i
				   longestEffectiveRange: &underlinedCharacterRange
						 inRange: characterRange];
	    if (underlineValue != nil && [underlineValue integerValue] != 0)
	      {
		const NSRange underlinedGylphRange = [self glyphRangeForCharacterRange: underlinedCharacterRange
								  actualCharacterRange: NULL];
		
		// we have a range of glpyhs that need underlining, which might span
		// multiple line fragments, so we need to iterate though the line fragments
		for (j=underlinedGylphRange.location; j<NSMaxRange(underlinedGylphRange); )
		  {
		    NSRange lineFragmentGlyphRange;
		    const NSRect lineFragmentRect = [self lineFragmentRectForGlyphAtIndex: j
									   effectiveRange: &lineFragmentGlyphRange];
		    const NSRange rangeToUnderline = NSIntersectionRange(underlinedGylphRange, lineFragmentGlyphRange);
		    
		    [self _drawSpellingState: [underlineValue integerValue]
			       forGylphRange: rangeToUnderline
			    lineFragmentRect: lineFragmentRect
		      lineFragmentGlyphRange: lineFragmentGlyphRange
			     containerOrigin: containerOrigin];
		    
		    j = NSMaxRange(rangeToUnderline);
		  }
	      }
	    i += underlinedCharacterRange.length;
	  }
      }
  }
}

-(void) underlineGylphRange: (NSRange)range
              underlineType: (NSInteger)type
           lineFragmentRect: (NSRect)fragmentRect
     lineFragmentGlyphRange: (NSRange)fragmentGlyphRange
            containerOrigin: (NSPoint)containerOrigin
{
  // FIXME: Implement underlining by word
  /*if ((type & NSUnderlineByWordMask) != 0)
    {
      NSCharacterSet *setToSkip = [NSCharacterSet whitespaceAndNewlineCharacterSet];
      NSRange characterRange = [self characterRangeForGlyphRange: range
						actualGlyphRange: NULL];
      NSString *string = [[self textStorage] string];
      NSRange examiningRange = range;
      while (1)
	{
	  NSRange nextRangeToSkip = [string rangeOfCharacterInSet: setToSkip
							   option: 0
							    range: NSMakeRange(i, range.length - (i - range.location))];
	  if (nextRangeToSkip.location == NSNotFound)
	    {
	    }
	  else
	    {

	    }
        }
    }
  else*/
    {
      [self drawUnderlineForGlyphRange: range
                         underlineType: type
                        baselineOffset: 0 // FIXME:
		      lineFragmentRect: fragmentRect
		lineFragmentGlyphRange: fragmentGlyphRange
		       containerOrigin: containerOrigin];
    }
}


static void GSDrawPatternLine(NSPoint start, NSPoint end, NSInteger pattern, CGFloat thickness, CGFloat phase)
{
  NSBezierPath *path = [NSBezierPath bezierPath];
  // FIXME: setLineDash should take CGFloat
  if ((pattern & NSUnderlinePatternDot) == NSUnderlinePatternDot)
    {
      const CGFloat dot[2] = {2.5 * thickness, 2.5 * thickness};
      [path setLineDash: dot count: 2 phase: phase];
    }
  else if ((pattern & NSUnderlinePatternDash) == NSUnderlinePatternDash)
    {
      const CGFloat dash[2] = {10 * thickness, 5 * thickness};   
      [path setLineDash: dash count: 2 phase: phase];
    }
  else if ((pattern & NSUnderlinePatternDashDot) == NSUnderlinePatternDashDot)
    {
      const CGFloat dashdot[4] = {10 * thickness, 3 * thickness, 3 * thickness, 3 * thickness};
      [path setLineDash: dashdot count: 4 phase: phase];
    }
  else if ((pattern & NSUnderlinePatternDashDotDot) == NSUnderlinePatternDashDotDot)
    {
      const CGFloat dashdotdot[6] = {10 * thickness, 3 * thickness, 3 * thickness, 3 * thickness, 3 * thickness, 3 * thickness};
      [path setLineDash: dashdotdot count: 6 phase: phase];
    }

  [path setLineWidth: thickness];
  [path moveToPoint: start];
  [path lineToPoint: end];
  [path stroke];
}

-(void) drawUnderlineForGlyphRange: (NSRange)underlineRange
                     underlineType: (NSInteger)type
                    baselineOffset: (CGFloat)offset
                  lineFragmentRect: (NSRect)fragmentRect
            lineFragmentGlyphRange: (NSRange)fragmentGlyphRange
                   containerOrigin: (NSPoint)containerOrigin
{       
  /*NSLog(@"drawUnderlineForGlyphRange:%@ underlineType:%d baselineOffset:%f lineFragmentRect:%@ lineFragmentGlyphRange:%@ containerOrigin:%@",
		  NSStringFromRange(underlineRange),
		  (int)type,
		  (float)offset,
		  NSStringFromRect(fragmentRect),
		  NSStringFromRange(fragmentGlyphRange),
		  NSStringFromPoint(containerOrigin));*/
  
  // We have to iterate through the attributes (again..) to find
  // contiguous regions with the same underline color.

  NSUInteger i;
  NSColor *link_color = nil;
  const NSRange characterRange = [self characterRangeForGlyphRange: underlineRange
						  actualGlyphRange: NULL];

  if (!(underlineRange.location >= fragmentGlyphRange.location &&
        NSMaxRange(underlineRange) <= NSMaxRange(fragmentGlyphRange)))
    {
      NSLog(@"Error, underlineRange must be inside fragmentGlyphRange");
      return;
    }

  for (i = characterRange.location; i < NSMaxRange(characterRange); )
    {
      NSRange underlineColorCharacterRange, foregroundColorCharacterRange, rangeToDraw;
      NSColor *underlineColor = nil;
      NSRange glyphRangeToDraw;
      NSRange linkCharacterRange;
      id linkValue;

      linkValue = [_textStorage attribute: NSLinkAttributeName
				  atIndex: i
		    longestEffectiveRange: &linkCharacterRange
				  inRange: NSMakeRange(i, NSMaxRange(characterRange)-i)];
      if (linkValue != nil)
	{
	  if (link_color == nil)
	    {
	      NSDictionary *link_attributes = [[self firstTextView] linkTextAttributes];
	      link_color = [link_attributes valueForKey: NSForegroundColorAttributeName];
	    }
	  if (link_color != nil)
	    underlineColor = link_color;

	  underlineColorCharacterRange = linkCharacterRange;
	}
      else
	{
	  underlineColor = (NSColor*)[[self textStorage] 
				       attribute: NSUnderlineColorAttributeName
					 atIndex: i
				       longestEffectiveRange: &underlineColorCharacterRange
					 inRange: NSMakeRange(i, NSMaxRange(characterRange)-i)];
	  underlineColorCharacterRange = NSIntersectionRange(underlineColorCharacterRange, 
							     linkCharacterRange);
	}

      if (underlineColor != nil)
	{
	  [underlineColor set];
	  rangeToDraw = underlineColorCharacterRange;
	}
      else
	{
	  NSColor *foregroundColor = (NSColor*)[[self textStorage] 
						 attribute: NSForegroundColorAttributeName
						   atIndex: i
						 longestEffectiveRange: &foregroundColorCharacterRange
						   inRange: NSMakeRange(i, NSMaxRange(characterRange)-i)];

	  if (foregroundColor != nil)
	    {
	      [foregroundColor set];
	    }
	  else 
	    {
	      [[NSColor textColor] set];
	    }

	  // Draw the smaller range
	  rangeToDraw = underlineColorCharacterRange.length < foregroundColorCharacterRange.length ?
	    underlineColorCharacterRange : foregroundColorCharacterRange;
	}
      
      glyphRangeToDraw = [self glyphRangeForCharacterRange: rangeToDraw
				      actualCharacterRange: NULL];
      if (glyphRangeToDraw.length > 0)
	{
	  // do the actual underline
	  
	  // FIXME: find the largest font within the range to underline
	  // NOTE: GS private method
	  NSFont *largestFont = [self effectiveFontForGlyphAtIndex: glyphRangeToDraw.location
							     range: NULL];

	  const CGFloat underlineWidth = [largestFont pointSize] * 
	    (((type & NSUnderlineStyleDouble) != 0) ? 0.05 : 0.07);
	  
	  NSPoint start = [self locationForGlyphAtIndex: glyphRangeToDraw.location];
	  NSPoint end = [self locationForGlyphAtIndex: NSMaxRange(glyphRangeToDraw) - 1];

	  // FIXME: remove this hack lowers the underline slightly
	  start.y += [largestFont pointSize] * 0.07;
	  end.y += [largestFont pointSize] * 0.07;

	  end.x += [self advancementForGlyphAtIndex: (NSMaxRange(glyphRangeToDraw) - 1)].width;
	  
	  start = NSMakePoint(start.x + containerOrigin.x + fragmentRect.origin.x, start.y + containerOrigin.y + fragmentRect.origin.y);
	  end = NSMakePoint(end.x + containerOrigin.x + fragmentRect.origin.x, end.y + containerOrigin.y + fragmentRect.origin.y);
	  
	  if ((type & NSUnderlineStyleDouble) == NSUnderlineStyleDouble)
	    {
	      GSDrawPatternLine(NSMakePoint(start.x, start.y - (underlineWidth / 2)), 
				NSMakePoint(end.x, end.y - (underlineWidth / 2)),
				type, underlineWidth / 2, start.x);
	      GSDrawPatternLine(NSMakePoint(start.x, start.y + (underlineWidth / 2)), 
				NSMakePoint(end.x, end.y + (underlineWidth / 2)),
				type, underlineWidth / 2, start.x);
	    }
	  else
	    {
	      GSDrawPatternLine(start, end, type, underlineWidth, start.x);
	    }
	}

      i += rangeToDraw.length;
    }
}

@end

@implementation NSLayoutManager (spelling)

-(void) _drawSpellingState: (NSInteger)spellingState
	     forGylphRange: (NSRange)range
	  lineFragmentRect: (NSRect)fragmentRect
    lineFragmentGlyphRange: (NSRange)fragmentGlyphRange
	   containerOrigin: (NSPoint)containerOrigin
{
  NSBezierPath *path;
  const CGFloat pattern[2] = {2.5, 1.0};
  NSFont *largestFont = [self effectiveFontForGlyphAtIndex: range.location // NOTE: GS private method
						     range: NULL];
  NSPoint start = [self locationForGlyphAtIndex: range.location];
  NSPoint end = [self locationForGlyphAtIndex: NSMaxRange(range) - 1]; //FIXME: check length > 0

  if (spellingState == 0)
    {
      return;
    }
  
  // FIXME: calculate the underline position correctly, using the font on both the start and end glyph
  start.y += [largestFont pointSize] * 0.07;
  end.y += [largestFont pointSize] * 0.07;

  end.x += [self advancementForGlyphAtIndex: (NSMaxRange(range) - 1)].width;
	  
  start = NSMakePoint(start.x + containerOrigin.x + fragmentRect.origin.x, start.y + containerOrigin.y + fragmentRect.origin.y);
  end = NSMakePoint(end.x + containerOrigin.x + fragmentRect.origin.x, end.y + containerOrigin.y + fragmentRect.origin.y);

  path = [NSBezierPath bezierPath];
  [path setLineDash: pattern count: 2 phase: 0];
  [path setLineWidth: 1.5];
  [path moveToPoint: start];
  [path lineToPoint: end];

  if ((spellingState & NSSpellingStateGrammarFlag) != 0)
    {
      [[NSColor greenColor] set];
    }
  else
    {
      [[NSColor redColor] set];
    }

  [path stroke];	  
}

@end


@implementation NSLayoutManager

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSLayoutManager class])
    {
      [self exposeBinding: @"hyphenationFactor"];
    }
}

-(void) insertTextContainer: (NSTextContainer *)aTextContainer
		    atIndex: (unsigned int)index
{
  int i;

  [super insertTextContainer: aTextContainer
  	atIndex: index];

  for (i = 0; i < num_textcontainers; i++)
    [[textcontainers[i].textContainer textView] _updateMultipleTextViews];
}

-(void) removeTextContainerAtIndex: (unsigned int)index
{
  int i;
  NSTextView *tv = [textcontainers[index].textContainer textView];

  RETAIN(tv);

  [super removeTextContainerAtIndex: index];

  [tv _updateMultipleTextViews];
  RELEASE(tv);

  for (i = 0; i < num_textcontainers; i++)
    [[textcontainers[i].textContainer textView] _updateMultipleTextViews];
}


-(void) dealloc
{
  // Remove all key value bindings for this object.
  [GSKeyValueBinding unbindAllForObject: self];

  DESTROY(_typingAttributes);
  DESTROY(_temporaryAttributes);
  [super dealloc];
}


/*
TODO: Add a general typesetterAttributes dictionary. Implement the
hyphenation factor methods by setting/getting an attribute in this
dictionary.
*/
-(float) hyphenationFactor
{
  return 0.0;
}

-(void) setHyphenationFactor: (float)factor
{
  NSLog(@"Warning: (NSLayoutManager) %s not implemented", __PRETTY_FUNCTION__);
}


-(NSTextView *) firstTextView
{
  int i;
  NSTextView *tv;
  for (i = 0; i < num_textcontainers; i++)
    {
      tv = [textcontainers[i].textContainer textView];
      if (tv)
        return tv;
    }
  return nil;
}

-(NSTextView *) textViewForBeginningOfSelection
{
  /* TODO */
  return [self firstTextView];
}

-(BOOL) layoutManagerOwnsFirstResponderInWindow: (NSWindow *)window
{
  int i;
  NSResponder *tv;
  NSResponder *v = [window firstResponder];

  for (i = 0; i < num_textcontainers; i++)
    {
      tv = [textcontainers[i].textContainer textView];
      if (tv == v)
        return YES;
    }
  return NO;
}


-(NSArray *) rulerMarkersForTextView: (NSTextView *)textView
		      paragraphStyle: (NSParagraphStyle *)paragraphStyle
			       ruler: (NSRulerView *)aRulerView
{
  NSRulerMarker *marker;
  NSTextTab *tab;
  NSImage *image;
  NSArray *tabs = [paragraphStyle tabStops];
  NSEnumerator *enumerator = [tabs objectEnumerator];
  NSMutableArray *markers = [NSMutableArray arrayWithCapacity: [tabs count]];

  while ((tab = [enumerator nextObject]) != nil)
    {
      switch ([tab tabStopType])
        {
	  case NSLeftTabStopType:
	    image = [NSImage imageNamed: @"common_LeftTabStop"];
	    break;
	  case NSRightTabStopType:
	    image = [NSImage imageNamed: @"common_RightTabStop"];
	    break;
	  case NSCenterTabStopType:
	    image = [NSImage imageNamed: @"common_CenterTabStop"];
	    break;
	  case NSDecimalTabStopType:
	    image = [NSImage imageNamed: @"common_DecimalTabStop"];
	    break;
	  default:
	    image = nil;
	    break;
	}
      marker = [[NSRulerMarker alloc] 
		   initWithRulerView: aRulerView
		   markerLocation: [tab location]
		   image: image
		   imageOrigin: NSMakePoint(0, 0)];
      [marker setRepresentedObject: tab];
      [markers addObject: marker];
      RELEASE(marker);
    }

  return markers;
}

-(NSView *) rulerAccessoryViewForTextView: (NSTextView *)textView
			   paragraphStyle: (NSParagraphStyle *)style
				    ruler: (NSRulerView *)ruler
				  enabled: (BOOL)isEnabled
{
  /* TODO */
  return nil;
}


/*
TODO: not really clear what these should do
*/
-(void) invalidateDisplayForGlyphRange: (NSRange)aRange
{
  int i;
  unsigned int m;
  NSRange r;
  NSRect rect;
  NSPoint p;
  NSTextView *tv;

  for (i = 0; i < num_textcontainers; i++)
    {
      if (!textcontainers[i].num_linefrags)
        break;

      if (textcontainers[i].pos >= aRange.location + aRange.length)
        break; /* we're past the end of the range */

      m = textcontainers[i].pos + textcontainers[i].length;
      if (m < aRange.location)
        continue;

      r.location = textcontainers[i].pos;
      if (aRange.location > r.location)
        r.location = aRange.location;

      if (m > aRange.location + aRange.length)
        m = aRange.location + aRange.length;

      r.length = m - r.location;

      /* Range r in this text view should be invalidated. */
      rect = [self boundingRectForGlyphRange: r
 	       inTextContainer: textcontainers[i].textContainer];
      tv = [textcontainers[i].textContainer textView];
      p = [tv textContainerOrigin];
      rect.origin.x += p.x;
      rect.origin.y += p.y;

      [tv setNeedsDisplayInRect: rect];
    }
}

-(void) invalidateDisplayForCharacterRange: (NSRange)aRange
{
  if (layout_char <= aRange.location)
    return;
  if (layout_char < aRange.location + aRange.length)
    aRange.length = layout_char - aRange.location;
  [self invalidateDisplayForGlyphRange:
    [self glyphRangeForCharacterRange: aRange
      actualCharacterRange: NULL]];

}


-(void) _didInvalidateLayout
{
  unsigned int g;
  int i;

  /* Invalidate from the first glyph not laid out (which will
  generally be the first glyph to have been invalidated). */
  g = layout_glyph;

  [super _didInvalidateLayout];

  for (i = 0; i < num_textcontainers; i++)
    {
      if (textcontainers[i].complete &&
	  g < textcontainers[i].pos + textcontainers[i].length)
        continue;

      [[textcontainers[i].textContainer textView] _layoutManagerDidInvalidateLayout];
    }
}


-(void) _dumpLayout
{
  int i, j, k;
  textcontainer_t *tc;
  linefrag_t *lf;
  linefrag_point_t *lp;
  linefrag_attachment_t *la;

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      printf("tc %2i, %5i+%5i  (complete %i)\n",
	i,tc->pos,tc->length,tc->complete);
      printf("  lfs: (%3i)\n", tc->num_linefrags);
      for (j = 0, lf = tc->linefrags; j < tc->num_linefrags; j++, lf++)
	{
	  printf("   %3i : %5i+%5i  (%g %g)+(%g %g)\n",
	    j,lf->pos,lf->length,
	    lf->rect.origin.x,lf->rect.origin.y,
	    lf->rect.size.width,lf->rect.size.height);
	  for (k = 0, lp = lf->points; k < lf->num_points; k++, lp++)
	    printf("               p%3i : %5i+%5i\n",k,lp->pos,lp->length);
	  for (k = 0, la = lf->attachments; k < lf->num_attachments; k++, la++)
	    printf("               a%3i : %5i+%5i\n",k,la->pos,la->length);
	}
      printf("  softs: (%3i)\n", tc->num_soft);
      for (; j < tc->num_linefrags + tc->num_soft; j++, lf++)
	{
	  printf("   %3i : %5i+%5i  (%g %g)+(%g %g)\n",
	    j,lf->pos,lf->length,
	    lf->rect.origin.x,lf->rect.origin.y,
	    lf->rect.size.width,lf->rect.size.height);
	  for (k = 0, lp = lf->points; k < lf->num_points; k++, lp++)
	    printf("               p%3i : %5i+%5i\n",k,lp->pos,lp->length);
	  for (k = 0, la = lf->attachments; k < lf->num_attachments; k++, la++)
	    printf("               a%3i : %5i+%5i\n",k,la->pos,la->length);
	}
    }
    printf("layout to: char %i, glyph %i\n",layout_char,layout_glyph);
}


/*
We completely override this method and use the extra information we have
about layout to do smarter invalidation. The comments at the beginning of
this file describes this.
*/
- (void) textStorage: (NSTextStorage *)aTextStorage
	      edited: (unsigned int)mask
	       range: (NSRange)range
      changeInLength: (int)lengthChange
    invalidatedRange: (NSRange)invalidatedRange
{
  NSRange r;
  unsigned int original_last_glyph;

/*  printf("\n*** invalidating\n");
  [self _dumpLayout];*/

  /*
  Using -glyphRangeForChara... here would be safer, but we must make
  absolutely sure that we don't cause any glyph generation until the
  invalidation is done.

  TODO: make sure last_glyph is set as expected
  */
  original_last_glyph = layout_glyph;

  if (!(mask & NSTextStorageEditedCharacters))
    lengthChange = 0;

  if (_temporaryAttributes != nil && (mask & NSTextStorageEditedCharacters) != 0)
    {
      int i;
      NSArray *attrs;
      NSRange oldRange = NSMakeRange(range.location, range.length - lengthChange);

      NSString *replacementString = [[GSDummyMutableString alloc] initWithLength: range.length];
      [_temporaryAttributes replaceCharactersInRange: oldRange
					  withString: replacementString];
      [replacementString release];
      
      // In addition, clear any temporary attributes that may have been extended
      // over the affected range

      if (range.length > 0)
	{
	  attrs = [[self temporaryAttributesAtCharacterIndex: range.location
					      effectiveRange: NULL] allKeys];
	  for (i=0; i<[attrs count]; i++)
	    {
	      [self removeTemporaryAttribute: [attrs objectAtIndex: i]
			   forCharacterRange: range];
	    }
	}
    }

  [self invalidateGlyphsForCharacterRange: invalidatedRange
	changeInLength: lengthChange
	actualCharacterRange: &r];

  /*
  If we had layout information and we had layout information for the range
  of characters that was modified, we need to invalidate layout information.

  TODO: This is broken. Even if we don't have layout for the modified
  characters, we might have layout for the preceeding line, and we then need
  to invalidate that line. Need to rework this a bit... I really really need
  to know the glyph length change here. :/
  (Alexander Malmberg 2004-03-22)
  */
  if (layout_char > 0 && layout_char >= r.location)
    {
      unsigned int glyph_index, last_glyph;
      textcontainer_t *tc;
      linefrag_t *lf;
      int i, j, k;
      int new_num;
      NSRange char_range;
      unsigned int new_last_glyph;
      int glyph_delta;

      /*
      If we had layout beyond the modified characters, update layout_char.
      Otherwise, just pretend that we have layout up to the end of the range
      after the change. This will give glyph_delta and last_glyph incorrect
      values, strictly speaking, but glyph_delta is only used if we have
      layout beyond the modified range, and last_glyph is used in a way that
      makes it safe to overestimate it (as we do here).

      When I can get exact information about the modified glyphs (TODO above),
      all this will become much cleaner...
      */
      if (layout_char >= r.location + r.length - lengthChange)
	layout_char += lengthChange;
      else
	layout_char = r.location + r.length;

      if (!layout_char)
	new_last_glyph = 0;
      else if (layout_char >= [_textStorage length])
	new_last_glyph = [self numberOfGlyphs];
      else
	new_last_glyph = [self glyphRangeForCharacterRange: NSMakeRange(layout_char, 1)
				      actualCharacterRange: NULL].location;

      glyph_delta = new_last_glyph - original_last_glyph;

      /*
      Note that r.location might not actually have any text container or
      line frag.
      */
      if (!r.location)
	{
	  glyph_index = 0;
	}
      else if (r.location == [_textStorage length])
	{
	  /*
	  Since layout was built beyond r.location, glyphs must have been
	  too, so invalidation only removed trailing glyphs and we still
	  have glyphs built up to the end. Thus, -numberOfGlyphs is cheap
	  to call.
	  */
	  glyph_index = [self numberOfGlyphs];
	  char_range.location = [_textStorage length];
	}
      else
	{
	  /*
	  Will cause generation of glyphs, but I consider that acceptable
	  for now. Soft-invalidation will cause even more glyph generation,
	  anyway.
	  */
	  glyph_index =
	    [self glyphRangeForCharacterRange: NSMakeRange(r.location,1)
			 actualCharacterRange: &char_range].location;
	}

      /*
      For soft invalidation, we need to know where to stop hard-invalidating.
      This will cause immediate glyph generation to fill the gaps the
      invalidation caused.
      */
      if (NSMaxRange(r) == [_textStorage length])
	{
	  last_glyph = [self numberOfGlyphs];
	}
      else
	{
	  last_glyph =
	    [self glyphRangeForCharacterRange: NSMakeRange(NSMaxRange(r),1)
			 actualCharacterRange: NULL].location;
	}
      last_glyph -= glyph_delta;

      /* glyph_index is the first index we should invalidate for. */
      for (j = 0, tc = textcontainers; j < num_textcontainers; j++, tc++)
	if (tc->pos + tc->length >= glyph_index)
	  break;

      LINEFRAG_FOR_GLYPH(glyph_index);

      /*
      We invalidate the entire line containing lf, and the entire
      previous line. Thus, we scan backwards to find the first line frag
      on the previous line.
      */
      while (i > 0 && lf[-1].rect.origin.y == lf->rect.origin.y)
	lf--, i--;
      /* Now we have the first line frag on this line. */
      if (i > 0)
	{
	  lf--, i--;
	}
      else
	{
	  /*
	  The previous line isn't in this text container, so we move
	  to the previous text container.
	  */
	  if (j > 0)
	    {
	      j--;
	      tc--;
	      i = tc->num_linefrags - 1;
	      lf = tc->linefrags + i;
	    }
	}
      /* Last line frag on previous line. */
      while (i > 0 && lf[-1].rect.origin.y == lf->rect.origin.y)
	lf--, i--;
      /* First line frag on previous line. */

      /* Invalidate all line frags that intersect the invalidated range. */
      new_num = i;
      while (1)
	{
	  for (; i < tc->num_linefrags + tc->num_soft; i++, lf++)
	    {
	      /*
	      Since we must invalidate whole lines, we can only stop if
	      the line frag is beyond the invalidated range, and the line
	      frag is the first line frag in a line.
	      */
	      if (lf->pos >= last_glyph &&
		  (!i || lf[-1].rect.origin.y != lf->rect.origin.y))
		{
		  break;
		}
	      if (lf->points)
		{
		  free(lf->points);
		  lf->points = NULL;
		}
	      if (lf->attachments)
		{
		  free(lf->attachments);
		  lf->attachments = NULL;
		}
	    }
	  if (i < tc->num_linefrags + tc->num_soft)
	    break;
	  tc->num_linefrags = new_num;
	  tc->num_soft = 0;
	  tc->was_invalidated = YES;
	  tc->complete = NO;
	  if (new_num)
	    {
	      tc->length = tc->linefrags[new_num-1].pos + tc->linefrags[new_num-1].length - tc->pos;
	    }
	  else
	    {
	      tc->pos = tc->length = 0;
	    }

	  j++, tc++;
	  if (j == num_textcontainers)
	    break;

	  new_num = 0;
	  i = 0;
	  lf = tc->linefrags;
	}

      if (j == num_textcontainers)
	goto no_soft_invalidation;

      if (new_num != i)
	{
	  /*
	  There's a gap between the last valid line frag and the first
	  soft line frag. Compact the linefrags.
	  */
	  memmove(tc->linefrags + new_num, lf, sizeof(linefrag_t) * (tc->num_linefrags + tc->num_soft - i));
	  tc->num_linefrags -= i - new_num;
	  i = new_num;
	  lf = tc->linefrags + i;
	}
      tc->num_soft += tc->num_linefrags - new_num;
      tc->num_linefrags = new_num;
      tc->was_invalidated = YES;
      tc->complete = NO;
      if (new_num)
	{
	  tc->length = tc->linefrags[new_num - 1].pos + tc->linefrags[new_num - 1].length - tc->pos;
	}
      else
	{
	  tc->pos = tc->length = 0;
	}

      /*
      Soft invalidate all remaining layout. Update their glyph positions
      and set the soft-invalidate markers in the text containers.
      */
      while (1)
	{
	  for (; i < tc->num_linefrags + tc->num_soft; i++, lf++)
	    {
	      lf->pos += glyph_delta;
	      for (k = 0; k < lf->num_points; k++)
		lf->points[k].pos += glyph_delta;
	      for (k = 0; k < lf->num_attachments; k++)
		lf->attachments[k].pos += glyph_delta;
	    }
	
	  j++, tc++;
	  if (j == num_textcontainers)
	    break;
	  i = 0;
	  lf = tc->linefrags;
	  tc->num_soft += tc->num_linefrags;
	  tc->num_linefrags = 0;
	  tc->was_invalidated = YES;
	  tc->complete = NO;
	}

no_soft_invalidation:
      /* Set layout_glyph and layout_char. */
      for (i = num_textcontainers - 1, tc = textcontainers + i; i >= 0; i--, tc--)
	{
	  if (tc->num_linefrags)
	    {
	      layout_glyph = tc->pos + tc->length;
	      if (layout_glyph == glyphs->glyph_length)
		layout_char = glyphs->char_length;
	      else
		layout_char = [self characterIndexForGlyphAtIndex: layout_glyph]; /* TODO? */
	      break;
	    }
        }
      if (i < 0)
	layout_glyph = layout_char = 0;
    }
  else
    {
      int i, j;
      linefrag_t *lf;
      textcontainer_t *tc;
      /*
      TODO: could handle this better, but it should be a rare case,
      handling it efficiently is tricky.

      For now, we simply clear out all soft invalidation information.
      */
      for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
	{
	  for (j = 0, lf = tc->linefrags + tc->num_linefrags; j < tc->num_soft; j++, lf++)
	    {
	      if (lf->points)
		{
		  free(lf->points);
		  lf->points = NULL;
		}
	      if (lf->attachments)
		{
		  free(lf->attachments);
		  lf->attachments = NULL;
		}
	    }
	  tc->num_soft = 0;
	  if (tc->pos + tc->length == r.location)
	    {
	      tc->complete = NO;
	    }
	}
    }

  /* Clear the extra line fragment information. */
  extra_textcontainer = nil;

/*  [self _dumpLayout];
  printf("*** done\n");*/

  [self _didInvalidateLayout];

  if (mask & NSTextStorageEditedCharacters)
    {
      /*
      Adjust the selected range so it's still valid. We don't try to
      be smart here (smart adjustments will have to be done by whoever
      made the change), we just want to keep it in range to avoid crashes.

      TODO: It feels slightly ugly to be doing this here, but there aren't
      many other places that can do this, and it gives reasonable behavior
      for select-only text views.

      One option is to only adjust when absolutely necessary to keep the
      selected range valid.


      Current behavior for all cases:

      Start     End       Action
(of selection, wrt range, before change)
      --------------------------
      after     after     location += lengthChange;
      in        after     length = NSMaxRange(sel)-(NSMaxRange(range)-lengthChange); location=NSMaxRange(range);
      in        in        length = 0; location=NSMaxRange(range);
      before    after     length += lengthChange;
      before    in        length = range.location-location;
      before    before    do nothing

      In other words, unless the selection spans over the entire changed
      range, the changed range is deselected.

      One important property of this behavior is that if length is 0 before,
      it will be 0 after.
      */
      NSRange newRange = _selected_range;

      if (_selected_range.location >= NSMaxRange(range) - lengthChange)
	{ /* after after */
	  newRange.location += lengthChange;
	}
      else if (_selected_range.location >= range.location)
	{
	  if (NSMaxRange(_selected_range) > NSMaxRange(range) - lengthChange)
	    { /* in after */
	      newRange.length = NSMaxRange(_selected_range) - (NSMaxRange(range) - lengthChange);
	      newRange.location = NSMaxRange(range);
	    }
	  else
	    { /* in in */
	      newRange.length = 0;
	      newRange.location = NSMaxRange(range);
	    }
	}
      else if (NSMaxRange(_selected_range) > NSMaxRange(range) - lengthChange)
	{ /* before after */
	  newRange.length += lengthChange;
	}
      else if (NSMaxRange(_selected_range) > range.location)
	{ /* before in */
	  newRange.length = range.location - _selected_range.location;
	}
      else
	{ /* before before */
	}

      /* sanity check */
      if (NSMaxRange(newRange) > [_textStorage length]) 
        {
          newRange = NSMakeRange(MIN(range.location, [_textStorage length]), 0);
        }

      /* If there are text views attached to us, let them handle the
      change. */
      if ([self firstTextView])
	[[self firstTextView] setSelectedRange: newRange];
      else
	_selected_range = newRange;
    }
}


- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      int flags =
	// FIXME attribute not yet supported by GNUstep
	//defaultAttachementScaling |
	(backgroundLayoutEnabled ? 0x04 : 0) |
	(showsInvisibleCharacters ? 0x08 : 0) |
	(showsControlCharacters ? 0x10 : 0);

      [aCoder encodeObject: [self textContainers] forKey: @"NSTextContainers"];
      [aCoder encodeObject: [self textStorage] forKey: @"NSTextStorage"];
      [aCoder encodeObject: [self delegate] forKey: @"NSDelegate"];
      [aCoder encodeInt: flags forKey: @"NSLMFlags"];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [self init];

  if ([aDecoder allowsKeyedCoding])
    {
      int i;
      int flags;
      NSArray *array = [aDecoder decodeObjectForKey: @"NSTextContainers"];
      NSTextStorage *storage = [aDecoder decodeObjectForKey: @"NSTextStorage"];
      id delegate = [aDecoder decodeObjectForKey: @"NSDelegate"];
      
      if ([aDecoder containsValueForKey: @"NSLMFlags"])
        {
	  flags = [aDecoder decodeIntForKey: @"NSLMFlags"];

	  // FIXME attribute not yet supported by GNUstep
	  //defaultAttachementScaling = (NSImageScaling)(flags & 0x03);
	  backgroundLayoutEnabled = (flags & 0x04) != 0;
	  showsInvisibleCharacters = (flags & 0x08) != 0;
	  showsControlCharacters = (flags & 0x10) != 0;
	}
      [self setDelegate: delegate];
      [storage addLayoutManager: self];
      for (i = 0; i < [array count]; i++)
        { 
	  [self addTextContainer: [array objectAtIndex: i]];
	}
      return self;
    }
  else
    {
      return self;
    }
}

- (NSDictionary *) typingAttributes
{
  return _typingAttributes;
}

@end


@implementation GSDummyMutableString
- (id)initWithLength: (NSUInteger)aLength
{
  self = [super init];
  if (self != nil)
    {
      self->_length = aLength;
    }
  return self;
}
- (NSUInteger)length
{
  return _length;
}
- (unichar)characterAtIndex: (NSUInteger)index
{
  return 0;
}
- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)aString
{
  _length = (_length - range.length) + [aString length];
}
- (id) copyWithZone: (NSZone*)zone
{
  return [self mutableCopyWithZone: zone];
}
- (id) mutableCopyWithZone: (NSZone*)zone
{
  return [[GSDummyMutableString allocWithZone: zone] initWithLength: _length];
}
@end

@implementation NSLayoutManager (temporaryattributes)

- (NSMutableAttributedString*) _temporaryAttributes
{
  if (_temporaryAttributes == nil)
    {
      NSString *dummyString = [[GSDummyMutableString alloc] initWithLength: [[self textStorage] length]];
      _temporaryAttributes = [[NSMutableAttributedString alloc] initWithString: dummyString];
      [dummyString release];
    }
  return _temporaryAttributes;
}

- (void) setTemporaryAttributes: (NSDictionary *)attrs 
              forCharacterRange: (NSRange)range
{
  [[self _temporaryAttributes] setAttributes: attrs range: range];
  [self invalidateDisplayForCharacterRange: range];
}

- (NSDictionary *) temporaryAttributesAtCharacterIndex: (NSUInteger)index 
                                        effectiveRange: (NSRange*)longestRange
{
  return [[self _temporaryAttributes] attributesAtIndex: index effectiveRange: longestRange];
}

- (void) addTemporaryAttributes: (NSDictionary *)attrs 
              forCharacterRange: (NSRange)range
{
  [[self _temporaryAttributes] addAttributes: attrs range: range];
  [self invalidateDisplayForCharacterRange: range];
}

- (void) addTemporaryAttribute: (NSString *)attr 
                         value: (id)value 
             forCharacterRange: (NSRange)range
{
  [[self _temporaryAttributes] addAttribute: attr value: value range: range];
  [self invalidateDisplayForCharacterRange: range];
}

- (void) removeTemporaryAttribute: (NSString *)attr 
                forCharacterRange: (NSRange)range
{
  [[self _temporaryAttributes] removeAttribute: attr range: range];
  [self invalidateDisplayForCharacterRange: range];
}

- (id) temporaryAttribute: (NSString *)attr 
         atCharacterIndex: (NSUInteger)index 
           effectiveRange: (NSRange*)range
{
  return [[self _temporaryAttributes] attribute: attr atIndex: index effectiveRange: range];
}

- (id) temporaryAttribute: (NSString *)attr 
         atCharacterIndex: (NSUInteger)index 
    longestEffectiveRange: (NSRange*)longestRange 
                  inRange: (NSRange)range
{
  return [[self _temporaryAttributes] attribute: attr atIndex: index longestEffectiveRange: longestRange inRange: range];
}

- (NSDictionary *) temporaryAttributesAtCharacterIndex: (NSUInteger)index
                                 longestEffectiveRange: (NSRange*)longestRange 
                                               inRange: (NSRange)range
{
  return [[self _temporaryAttributes] attributesAtIndex: index longestEffectiveRange: longestRange inRange: range];
}

/**
 * Most of this is implemented in GSLayoutManager
 */
- (void) setTextStorage: (NSTextStorage *)aTextStorage
{
  DESTROY(_temporaryAttributes);
  [super setTextStorage: aTextStorage];
}

@end
