/*
   GSHorizontalTypesetter.m

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


#include <math.h>

#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSTextAttachment.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "GNUstepGUI/GSLayoutManager.h"
#import "GNUstepGUI/GSHorizontalTypesetter.h"



/*
Note that unless the user creates extra instances, there will only be one
instance of GSHorizontalTypesetter for all text typesetting, so we can
cache fairly aggressively without having to worry about memory consumption.
*/


@implementation GSHorizontalTypesetter

- init
{
  if (!(self = [super init])) return nil;
  lock = [[NSLock alloc] init];
  return self;
}

-(void) dealloc
{
  if (cache)
    {
      free(cache);
      cache = NULL;
    }
  if (line_frags)
    {
      free(line_frags);
      line_frags = NULL;
    }
  DESTROY(lock);
  [super dealloc];
}

+(GSHorizontalTypesetter *) sharedInstance
{
  NSMutableDictionary *threadDict = 
    [[NSThread currentThread] threadDictionary];
  GSHorizontalTypesetter *shared = 
    [threadDict objectForKey: @"sharedHorizontalTypesetter"];

  if (!shared)
    {
      shared = [[self alloc] init];
      [threadDict setObject: shared
		  forKey: @"sharedHorizontalTypesetter"];
      RELEASE(shared);
    }

  return shared;
}

#define CACHE_INITIAL 192
#define CACHE_STEP 192


struct GSHorizontalTypesetter_glyph_cache_s
{
  /* These fields are filled in by the caching: */
  NSGlyph g;
  unsigned int char_index;

  NSFont *font;
  struct
    {
      BOOL explicit_kern;
      float kern;
      float baseline_offset;
      int superscript;
    } attributes;

  /* These fields are filled in during layout: */
  BOOL nominal;
  NSPoint pos;    /* relative to the line's baseline */
  NSSize size;    /* height is used only for attachments */
  BOOL dont_show, outside_line_frag;
};
typedef struct GSHorizontalTypesetter_glyph_cache_s glyph_cache_t;

/* TODO: if we could know whether the layout manager had been modified since
the last time or not, we wouldn't need to clear the cache every time */
-(void) _cacheClear
{
  cache_length = 0;

  curParagraphStyle = nil;
  paragraphRange = NSMakeRange(0, 0);
  curAttributes = nil;
  attributeRange = NSMakeRange(0, 0);
  curFont = nil;
  fontRange = NSMakeRange(0, 0);
}

-(void) _cacheAttributes: (unsigned int)char_index
{
  NSNumber *n;

  if (NSLocationInRange(char_index, attributeRange))
    {
      return;
    }
  
  curAttributes = [curTextStorage attributesAtIndex: char_index
                                     effectiveRange: &attributeRange];

  n = [curAttributes objectForKey: NSKernAttributeName];
  if (!n)
    attributes.explicit_kern = NO;
  else
    {
      attributes.explicit_kern = YES;
      attributes.kern = [n floatValue];
    }

  n = [curAttributes objectForKey: NSBaselineOffsetAttributeName];
  if (n)
    attributes.baseline_offset = [n floatValue];
  else
    attributes.baseline_offset = 0.0;

  n = [curAttributes objectForKey: NSSuperscriptAttributeName];
  if (n)
    attributes.superscript = [n intValue];
  else
    attributes.superscript = 0;
}

-(void) _cacheMoveTo: (unsigned int)glyph
{
  BOOL valid;

  if (cache_base <= glyph && cache_base + cache_length > glyph)
    {
      int delta = glyph - cache_base;
      cache_length -= delta;
      memmove(cache, &cache[delta], sizeof(glyph_cache_t) * cache_length);
      cache_base = glyph;
      return;
    }

  cache_base = glyph;
  cache_length = 0;

  [curLayoutManager glyphAtIndex: glyph
		    isValidIndex: &valid];

  if (valid)
    {
      unsigned int i;

      at_end = NO;
      i = [curLayoutManager characterIndexForGlyphAtIndex: glyph];
      [self _cacheAttributes: i];

      paragraphRange = NSMakeRange(i, [curTextStorage length] - i);
      curParagraphStyle = [curTextStorage attribute: NSParagraphStyleAttributeName
					atIndex: i
					longestEffectiveRange: &paragraphRange
					inRange: paragraphRange];
      if (curParagraphStyle == nil)
        {
          curParagraphStyle = [NSParagraphStyle defaultParagraphStyle];
        }

      curFont = [curLayoutManager effectiveFontForGlyphAtIndex: glyph
				range: &fontRange];
    }
  else
    {
      at_end = YES;
    }
}

-(void) _cacheGlyphs: (unsigned int)new_length
{
  glyph_cache_t *g;
  BOOL valid;

  if (cache_size < new_length)
    {
      cache_size = new_length;
      cache = realloc(cache, sizeof(glyph_cache_t) * cache_size);
    }

  for (g = &cache[cache_length]; cache_length < new_length; cache_length++, g++)
    {
      g->g = [curLayoutManager glyphAtIndex: cache_base + cache_length
			       isValidIndex: &valid];
      if (!valid)
	{
	  at_end = YES;
	  break;
	}
      g->char_index = [curLayoutManager characterIndexForGlyphAtIndex: cache_base + cache_length];
      if (g->char_index >= paragraphRange.location + paragraphRange.length)
	{
	  at_end = YES;
	  break;
	}

      /* cache attributes */
      if (g->char_index >= attributeRange.location + attributeRange.length)
	{
	  [self _cacheAttributes: g->char_index];
	}

      g->attributes.explicit_kern = attributes.explicit_kern;
      g->attributes.kern = attributes.kern;
      g->attributes.baseline_offset = attributes.baseline_offset;
      g->attributes.superscript = attributes.superscript;

      if (cache_base + cache_length >= fontRange.location + fontRange.length)
	{
	  curFont = [curLayoutManager effectiveFontForGlyphAtIndex: cache_base + cache_length
				    range: &fontRange];
	}
      g->font = curFont;

      g->dont_show = NO;
      g->outside_line_frag = NO;
      g->nominal = YES;

      // FIXME: This assumes the layout manager implements this GNUstep extension
      g->size = [curLayoutManager advancementForGlyphAtIndex: cache_base + cache_length];
    }
}


/*
Should return the first glyph on the next line, which must be <=gi and
>=cache_base (TODO: not enough. actually, it probably is now. the wrapping
logic below will fall back to char wrapping if necessary). Glyphs up to and
including gi will have been cached.
*/
-(unsigned int) breakLineByWordWrappingBefore: (unsigned int)gi
{
  glyph_cache_t *g;
  unichar ch;
  NSString *str = [curTextStorage string];

  gi -= cache_base;
  g = cache + gi;

  while (gi > 0)
    {
      if (g->g == NSControlGlyph)
        return gi + cache_base;
      ch = [str characterAtIndex: g->char_index];
      /* TODO: paragraph/line separator */
      if (ch == 0x20 || ch == 0x0a || ch == 0x0d)
        {
          g->dont_show = YES;
          if (gi > 0)
            {
              g->pos = g[-1].pos;
              g->pos.x += g[-1].size.width;
            }
          else
            g->pos = NSMakePoint(0, 0);
          g->size.width = 0;
          return gi + 1 + cache_base;
        }
      /* Each CJK glyph should be treated as a word when wrapping word.
         The range should work for most cases */
      else if ((ch > 0x2ff0) && (ch < 0x9fff))
         {
           g->dont_show = NO;
           if (gi > 0)
             {
               g->pos = g[-1].pos;
               g->pos.x += g[-1].size.width;
             }
           else
             g->pos = NSMakePoint(0,0);
           return gi + cache_base;
         }     
      gi--;
      g--;
    }
  return gi + cache_base;
}


struct GSHorizontalTypesetter_line_frag_s
{
  NSRect rect;
  CGFloat last_used;
  unsigned int last_glyph; /* last_glyph+1, actually */
};
typedef struct GSHorizontalTypesetter_line_frag_s line_frag_t;

/*
Apple uses this as the maximum width of an NSTextContainer.
For bigger values the width gets ignored.
*/
#define LARGE_SIZE 1e7

-(void) fullJustifyLine: (line_frag_t *)lf : (int)num_line_frags
{
  unsigned int i, start;
  CGFloat extra_space, delta;
  unsigned int num_spaces;
  NSString *str = [curTextStorage string];
  glyph_cache_t *g;
  unichar ch;

  if (lf->rect.size.width >= LARGE_SIZE)
    {
      return;
    }

  for (start = 0; num_line_frags; num_line_frags--, lf++)
    {
      num_spaces = 0;
      for (i = start, g = cache + i; i < lf->last_glyph; i++, g++)
	{
	  if (g->dont_show)
	    continue;
	  ch = [str characterAtIndex: g->char_index];
	  if (ch == 0x20)
	    num_spaces++;
	}
      if (!num_spaces)
	continue;

      extra_space = lf->rect.size.width - lf->last_used;
      extra_space /= num_spaces;
      delta = 0;
      for (i = start, g = cache + i; i < lf->last_glyph; i++, g++)
	{
	  g->pos.x += delta;
	  if (!g->dont_show && [str characterAtIndex: g->char_index] == 0x20)
	    {
	      if (i < lf->last_glyph)
		g[1].nominal = NO;
	      delta += extra_space;
	    }
	}
      start = lf->last_glyph;
      lf->last_used = lf->rect.size.width;
    }
}

-(void) rightAlignLine: (line_frag_t *)lf : (int)num_line_frags
{
  unsigned int i;
  CGFloat delta;
  glyph_cache_t *g;

  if (lf->rect.size.width >= LARGE_SIZE)
    {
      return;
    }

  for (i = 0, g = cache; num_line_frags; num_line_frags--, lf++)
    {
      delta = lf->rect.size.width - lf->last_used;
      for (; i < lf->last_glyph; i++, g++)
	g->pos.x += delta;
      lf->last_used += delta;
    }
}

-(void) centerAlignLine: (line_frag_t *)lf : (int)num_line_frags
{
  unsigned int i;
  CGFloat delta;
  glyph_cache_t *g;

  if (lf->rect.size.width >= LARGE_SIZE)
    {
      return;
    }

  for (i = 0, g = cache; num_line_frags; num_line_frags--, lf++)
    {
      delta = (lf->rect.size.width - lf->last_used) / 2.0;
      for (; i < lf->last_glyph; i++, g++)
	g->pos.x += delta;
      lf->last_used += delta;
    }
}


-(BOOL) _reuseSoftInvalidatedLayout
{
  /*
  We only handle the simple-horizontal-text-container case currently.
  */
  NSRect r0, r;
  NSSize shift;
  int i;
  unsigned int g, g2, first;
  CGFloat container_height;
  /*
  Ask the layout manager for soft-invalidated layout for the current
  glyph. If there is a set of line frags starting at the current glyph,
  and we can get rects with the same size and horizontal position, we
  tell the layout manager to use the soft-invalidated information.
  */
  r0 = [curLayoutManager _softInvalidateLineFragRect: 0
					  firstGlyph: &first
					   nextGlyph: &g
				     inTextContainer: curTextContainer];

  container_height = [curTextContainer containerSize].height;
  if (!(curPoint.y + r0.size.height <= container_height))
    return NO;

  /*
  We can shift the rects and still have things fit. Find all the line
  frags in the line and shift them.
  */
  shift.width = 0;
  shift.height = curPoint.y - r0.origin.y;
  i = 1;
  curPoint.y = NSMaxY(r0) + shift.height;
  for (; 1; i++)
    {
      r = [curLayoutManager _softInvalidateLineFragRect: i
					     firstGlyph: &first
					      nextGlyph: &g2
					inTextContainer: curTextContainer];

      /*
      If there's a gap in soft invalidated information, we need to
      fill it in before we can continue.
      */
      if (first != g)
	{
	  break;
	}

      if (NSIsEmptyRect(r) || NSMaxY(r) + shift.height > container_height)
	break;

      g = g2;
      curPoint.y = NSMaxY(r) + shift.height;
    }

  [curLayoutManager _softInvalidateUseLineFrags: i
				      withShift: shift
				inTextContainer: curTextContainer];

  curGlyph = g;
  return YES;
}


- (NSRect)_getProposedRectFor: (BOOL)newParagraph
               withLineHeight: (CGFloat) line_height 
{
  CGFloat hindent;
  CGFloat tindent = [curParagraphStyle tailIndent];

  if (newParagraph)
    hindent = [curParagraphStyle firstLineHeadIndent];
  else
    hindent = [curParagraphStyle headIndent];

  if (tindent <= 0.0)
    { 
      NSSize size;

      size = [curTextContainer containerSize];
      tindent = size.width + tindent;
    }

  return NSMakeRect(hindent,
                    curPoint.y,
                    tindent - hindent,
                    line_height + [curParagraphStyle lineSpacing]);
}

- (void) _addExtraLineFragment
{
  NSRect r, r2, remain;
  CGFloat line_height;

  /*
    We aren't actually interested in the glyph data, but we want the
    attributes for the final character so we can make the extra line
    frag rect match it. This call makes sure that curParagraphStyle
    and curFont are set.
  */
  if (curGlyph)
    {
      [self _cacheMoveTo: curGlyph - 1];
    }
  else
    {
      NSDictionary *typingAttributes = [curLayoutManager typingAttributes];
      curParagraphStyle = [typingAttributes
                            objectForKey: NSParagraphStyleAttributeName];
      if (curParagraphStyle == nil)
        {
          curParagraphStyle = [NSParagraphStyle defaultParagraphStyle];
        }
      curFont = [typingAttributes objectForKey: NSFontAttributeName];
    }

  if (curFont)
    {
      line_height = [curFont defaultLineHeightForFont];
    }
  else
    {
      line_height = 15.0;
    }

  r = [self _getProposedRectFor: YES
                 withLineHeight: line_height];
  r = [curTextContainer lineFragmentRectForProposedRect: r
                                         sweepDirection: NSLineSweepRight
                                      movementDirection: NSLineMovesDown
                                          remainingRect: &remain];
  
  if (!NSIsEmptyRect(r))
    {
      r2 = r;
      r2.size.width = 1;
      [curLayoutManager setExtraLineFragmentRect: r
                                        usedRect: r2
                                   textContainer: curTextContainer];
    }
}

/*
Return values 0, 1, 2 are mostly the same as from
-layoutGlyphsInLayoutManager:.... Additions:

  0   Last typeset character was not a newline; next glyph does not start
      a new paragraph.

  3   Last typeset character was a newline; next glyph starts a new
      paragraph.

  4   Last typeset character may or may not have been a newline; must
      test before next call.

*/
-(int) layoutLineNewParagraph: (BOOL)newParagraph
{
  NSRect rect, remain;

  /* Baseline and line height handling. */
  CGFloat line_height;     /* Current line height. */
  CGFloat max_line_height; /* Maximum line height (usually from the paragraph style). */
  CGFloat baseline;        /* Baseline position (0 is top of line-height, positive is down). */
  CGFloat ascender;        /* Amount of space we want above the baseline (always>=0). */
  CGFloat descender;       /* Amount of space we want below the baseline (always>=0). */
  /*
  These are values for the line as a whole. We start out by initializing
  for the first glyph on the line and then update these as we add more
  glyphs.

  If we need to increase the line height, we jump back to 'restart:' and
  rebuild our array of line frag rects.

  (TODO (optimization): if we're dealing with a "simple rectangular
  text container", we should try to extend the existing line frag in place
  before jumping back to do all the expensive checking).
  */

  /*
  This calculation should match the calculation in [GSFontInfo
  -defaultLineHeightForFont], or text will look odd.
  */
#define COMPUTE_BASELINE  baseline = line_height - descender


  /* TODO: doesn't have to be a simple horizontal container, but it's easier
  to handle that way. */
  if ([curTextContainer isSimpleRectangularTextContainer] &&
      [curLayoutManager _softInvalidateFirstGlyphInTextContainer: curTextContainer] == curGlyph)
    {
      if ([self _reuseSoftInvalidatedLayout])
        return 4;
    }


  [self _cacheMoveTo: curGlyph];
  if (!cache_length)
    [self _cacheGlyphs: CACHE_INITIAL];
  if (!cache_length && at_end)
    {
      /*
      We've typeset all glyphs, and thus return 2. If we ended with a
      new-line, we set the extra line frag rect here so the insertion point
      will be properly positioned after a trailing newline in the text.
      */
      if (newParagraph)
        {
          [self _addExtraLineFragment];
        }

      return 2;
    }

  /* Set up our initial baseline info. */
  {
    CGFloat min = [curParagraphStyle minimumLineHeight];
    max_line_height = [curParagraphStyle maximumLineHeight];

    /* sanity */
    if (max_line_height > 0 && max_line_height < min)
      max_line_height = min;

    line_height = [cache->font defaultLineHeightForFont];
    ascender = [cache->font ascender];
    descender = -[cache->font descender];

    COMPUTE_BASELINE;

    if (line_height < min)
      line_height = min;

    if (max_line_height > 0 && line_height > max_line_height)
      line_height = max_line_height;
  }

  /*
  If we find out that we need to increase the line height, we have to
  start over. The increased line height might give _completely_ different
  line frag rects, so we can't reuse the layout information.

  OPT: However, we could recreate the line frag rects and see if they
  match before throwing away layout information, since most of the time
  they will be equivalent.

  Also, in the very common case of a simple rectangular text container, we
  can always extend the current line frag rects as long as they don't extend
  past the bottom of the container.
  */


#define WANT_LINE_HEIGHT(h) \
  do { \
    CGFloat __new_height = (h); \
    if (max_line_height > 0 && __new_height > max_line_height) \
      __new_height = max_line_height; \
    if (__new_height > line_height) \
      { \
	line_height = __new_height; \
	COMPUTE_BASELINE; \
	goto restart; \
      } \
  } while (0)


restart: ;
  remain = [self _getProposedRectFor: newParagraph
                 withLineHeight: line_height];

  /*
  Build a list of all line frag rects for this line.

  TODO: it's very convenient to do this in advance, but it might be
  inefficient, and in theory, we might end up with an insane number of line
  rects (eg. a text container with "hole"-columns every 100 points and
  width 1e8)
  */
  line_frags_num = 0;
  while (1)
    {
      rect = [curTextContainer lineFragmentRectForProposedRect: remain
			     sweepDirection: NSLineSweepRight
			     movementDirection: line_frags_num?NSLineDoesntMove:NSLineMovesDown
			     remainingRect: &remain];
      if (NSIsEmptyRect(rect))
        break;

      line_frags_num++;
      if (line_frags_num > line_frags_size)
	{
	  line_frags_size += 2;
	  line_frags = realloc(line_frags, sizeof(line_frag_t) * line_frags_size);
	}
      line_frags[line_frags_num - 1].rect = rect;
    }
  if (!line_frags_num)
    {
      if (curPoint.y == 0.0 &&
	  line_height > [curTextContainer containerSize].height &&
	  [curTextContainer containerSize].height > 0.0)
	{
	  /* Try to make sure each container contains at least one line frag
	  rect by shrinking our line height. */
	  line_height = [curTextContainer containerSize].height;
	  max_line_height = line_height;
	  goto restart;
	}
      return 1;
    }


  {
    unsigned int i = 0;
    glyph_cache_t *g;

    NSPoint p;
    
    NSFont *f = cache->font;

    CGFloat f_ascender = [f ascender], f_descender = -[f descender];

    NSGlyph last_glyph = NSNullGlyph;
    NSPoint last_p;

    unsigned int first_glyph;
    line_frag_t *lf = line_frags;
    int lfi = 0;

    BOOL prev_had_non_nominal_width;


    last_p = p = NSMakePoint(0, 0);

    g = cache;
    first_glyph = 0;
    prev_had_non_nominal_width = NO;
    /*
    Main glyph layout loop.
    */
    /* TODO: handling of newParagraph is ugly. must be set on all exits
    from this loop */
    while (1)
      {
//        printf("at %3i+%3i\n", cache_base, i);
	/* Update the cache. */
	if (i >= cache_length)
	  {
	    if (at_end)
	      {
		newParagraph = NO;
		break;
	      }
	    [self _cacheGlyphs: cache_length + CACHE_STEP];
	    if (i >= cache_length)
	      {
		newParagraph = NO;
		break;
	      }
	    g = cache + i;
	  }

/*printf("at %3i+%2i, glyph %08x, char %04x (%i)\n",
	cache_base, i,
	g->g,
	[[curTextStorage string] characterAtIndex: g->char_index], g->char_index);*/

	/*
	At this point:

	  p is the current point (sortof); the point where a nominally
	  spaced glyph would be placed.

	  g is the current glyph. i is the current glyph index, relative to
	  the start of the cache.

	  last_p and last_glyph are used for kerning and hold the previous
	  glyph and its position. If there's no previous glyph (for kerning
	  purposes), last_glyph is NSNullGlyph and last_p is undefined.

	  lf and lfi track the current line frag rect. first_glyph is the
	  first glyph in the current line frag rect.

	Note that the variables tracking the previous glyph shouldn't be
	updated until we know that the current glyph will fit in the line
	frag rect.

	*/

	/* If there's a font change, check if the baseline or line height
	needs adjusting. We update the ascender and descender too, even
	though there might not actually be any glyphs for this font.
	(TODO?) */
	if (g->font != f)
	  {
	    CGFloat new_height;
	    f = g->font;
	    f_ascender = [f ascender];
	    f_descender = -[f descender];
	    last_glyph = NSNullGlyph;

	    new_height = [f defaultLineHeightForFont];

	    if (f_ascender > ascender)
	      ascender = f_ascender;
	    if (f_descender > descender)
	      descender = f_descender;

	    COMPUTE_BASELINE;

	    WANT_LINE_HEIGHT(new_height);
	  }

	if (g->g == NSControlGlyph)
	  {
	    unichar ch = [[curTextStorage string] characterAtIndex: g->char_index];

	    /* TODO: need to handle other control characters */

	    g->pos = p;
	    g->size.width = 0;
	    g->dont_show = YES;
	    g->nominal = !prev_had_non_nominal_width;
	    i++;
	    g++;
	    last_glyph = NSNullGlyph;

	    prev_had_non_nominal_width = NO;

	    if (ch == 0xa)
	      {
		newParagraph = YES;
		break;
	      }

	    if (ch == 0x9)
	      {
		/*
		Handle tabs. This is a very basic and stupid implementation.
		TODO: implement properly
		*/
		NSArray *tabs = [curParagraphStyle tabStops];
		NSTextTab *tab = nil;
		CGFloat defaultInterval = [curParagraphStyle defaultTabInterval];
		/* Set it to something reasonable if unset */
		if (defaultInterval == 0.0) {
                  defaultInterval = 100.0;
		}
		int i, c = [tabs count];
		/* Find first tab beyond our current position. */
		for (i = 0; i < c; i++)
		  {
		    tab = [tabs objectAtIndex: i];
		    /*
		    We cannot use a tab at our exact location; we must
		    use one beyond it. The reason is that several tabs in
		    a row would get very odd behavior. Eg. given "\t\t",
		    the first tab would move (exactly) to the next tab
		    stop, and the next tab stop would move to the same
		    tab, thus having no effect.
		    */
		    if ([tab location] > p.x + lf->rect.origin.x)
                      {
                        break;
                      }
		  }
		if (i == c)
		  {
		    /*
		    Tabs after the last value in tabStops should use the
		    defaultTabInterval provided by NSParagraphStyle.
		    */
		    p.x = (floor(p.x / defaultInterval) + 1.0) * defaultInterval;
		  }
		else
		  {
		    p.x = [tab location] - lf->rect.origin.x;
		  }
		prev_had_non_nominal_width = YES;
		continue;
	      }

	    NSDebugLLog(@"GSHorizontalTypesetter",
	      @"ignoring unknown control character %04x\n", ch);

	    continue;
	  }


	/* Set up glyph information. */

	/*
	TODO:
	Currently, the attributes of the attachment character (eg. font)
	affect the layout. Think hard about this.
	*/
	g->nominal = !prev_had_non_nominal_width;

	if (g->attributes.explicit_kern &&
	    g->attributes.kern != 0)
	  {
	    p.x += g->attributes.kern;
	    g->nominal = NO;
	  }

	/* Baseline adjustments. */
	{
	  CGFloat y = 0;

	  /* Attributes are up-side-down in our coordinate system. */
	  if (g->attributes.superscript)
	    {
	      y -= g->attributes.superscript * [f xHeight];
	    }
	  if (g->attributes.baseline_offset)
	    {
	      /* And baseline_offset is up-side-down again. TODO? */
	      y += g->attributes.baseline_offset;
	    }

	  if (y != p.y)
	    {
	      p.y = y;
	      g->nominal = NO;
	    }

	  /* The y==0 case is taken care of when the font is changed. */
	  if (y < 0 && f_ascender - y > ascender)
	    ascender = f_ascender - y;
	  if (y > 0 && f_descender + y > descender)
	    descender = f_descender + y;

	  COMPUTE_BASELINE;
	  WANT_LINE_HEIGHT(ascender + descender);
	}

	if (g->g == GSAttachmentGlyph)
	  {
	    NSTextAttachment *attach;
	    NSTextAttachmentCell *cell;
	    NSRect r;

	    attach = [curTextStorage attribute: NSAttachmentAttributeName
	      atIndex: g->char_index
	      effectiveRange: NULL];
	    cell = (NSTextAttachmentCell*)[attach attachmentCell];
	    if (!cell)
	      {
		g->pos = p;
		g->size = NSMakeSize(0, 0);
		g->dont_show = YES;
		g->nominal = YES;
		i++;
		g++;
		last_glyph = NSNullGlyph;
		continue;
	      }

	    r = [cell cellFrameForTextContainer: curTextContainer
		  proposedLineFragment: lf->rect
		  glyphPosition: NSMakePoint(p.x,
					     lf->rect.size.height - baseline)
		  characterIndex: g->char_index];

/*	    printf("cell at %i, (%g %g) in (%g %g)+(%g %g), got rect (%g %g)+(%g %g)\n",
	      g->char_index,p.x,p.y,
	      lf->rect.origin.x,lf->rect.origin.y,
	      lf->rect.size.width,lf->rect.size.height,
	      r.origin.x,r.origin.y,
	      r.size.width,r.size.height);*/

	    /* For some obscure reason, the rectangle we get is up-side-down
	    compared to everything else here, and has it's origin in p.
	    (Makes sense from the cell's pov, though.) */

	    if (-NSMinY(r) > descender)
	      descender = -NSMinY(r);

	    if (NSMaxY(r) > ascender)
	      ascender = NSMaxY(r);

	    /* Update ascender and descender. Adjust line height and
	    baseline if necessary. */
	    COMPUTE_BASELINE;
	    WANT_LINE_HEIGHT(ascender + descender);

	    g->size = r.size;
	    g->pos.x = p.x + r.origin.x;
	    g->pos.y = p.y - r.origin.y;

	    p.x = g->pos.x + g->size.width;

	    /* An attachment is always in a point range of its own. */
	    g->nominal = NO;
	  }
	else
	  {
	    /* TODO: this is a major bottleneck */
/*	    if (last_glyph)
	      {
		BOOL n;
		p = [f positionOfGlyph: g->g
		      precededByGlyph: last_glyph
		      isNominal: &n];
		if (!n)
		  g->nominal = NO;
		p.x += last_p.x;
		p.y += last_p.y;
	      }*/

	    last_p = g->pos = p;
	    /* Only the width is used. */
	    p.x += g->size.width;
	  }

	/* Did the glyph fit in the line frag rect? */
	if (p.x > lf->rect.size.width)
	  {
	    /* It didn't. Try to break the line. */
	    switch ([curParagraphStyle lineBreakMode])
	      { /* TODO: implement all modes */
	      default:
	      case NSLineBreakByCharWrapping:
	      char_wrapping:
		lf->last_glyph = i;
		break;

	      case NSLineBreakByWordWrapping:
		lf->last_glyph = [self breakLineByWordWrappingBefore: cache_base + i] - cache_base;
		if (lf->last_glyph <= first_glyph)
		  goto char_wrapping;
		break;

	      case NSLineBreakByTruncatingHead:
	      case NSLineBreakByTruncatingMiddle:
	      case NSLineBreakByTruncatingTail:
		/* Pretending that these are clipping is far from prefect,
		but it's the closest we've got. */
	      case NSLineBreakByClipping:
		/* Scan forward to the next paragraph separator and mark
		all the glyphs up to there as not visible. */
		g->outside_line_frag = YES;
		while (1)
		  {
		    i++;
		    g++;
		    /* Update the cache. */
		    if (i >= cache_length)
		      {
			if (at_end)
			  {
			    newParagraph = NO;
			    i--;
			    break;
			  }
			[self _cacheGlyphs: cache_length + CACHE_STEP];
			if (i >= cache_length)
			  {
			    newParagraph = NO;
			    i--;
			    break;
			  }
			g = cache + i;
		      }
		    g->dont_show = YES;
		    g->pos = p;
		    if (g->g == NSControlGlyph
			&& [[curTextStorage string]
			       characterAtIndex: g->char_index] == 0xa)
		      break;
		  }
		lf->last_glyph = i + 1;
		break;
	      }

	    /* We force at least one glyph into each line frag rect. This
	    ensures that typesetting will never get stuck (ie. if the text
	    container is too narrow to fit even a single glyph). */
	    if (lf->last_glyph <= first_glyph)
	      lf->last_glyph = i + 1;

	    last_p = p = NSMakePoint(0, 0);
	    i = lf->last_glyph;
	    g = cache + i;
	    /* The -1 is always valid since there's at least one glyph in the
	    line frag rect (see above). */
	    lf->last_used = g[-1].pos.x + g[-1].size.width;
	    last_glyph = NSNullGlyph;
	    prev_had_non_nominal_width = NO;

	    lf++;
	    lfi++;
	    if (lfi == line_frags_num)
	      {
		newParagraph = NO;
		break;
	      }
	    first_glyph = i;
	  }
	else
	  {
	    /* Move to next glyph. */
	    last_glyph = g->g;
	    if (last_glyph == GSAttachmentGlyph)
	      {
		last_glyph = NSNullGlyph;
		prev_had_non_nominal_width = YES;
	      }
	    else
	      {
		prev_had_non_nominal_width = NO;
	      }
	    i++;
	    g++;
	  }
      }
    /* Basic layout is done. */

    /* Take care of the alignments. */
    if (lfi != line_frags_num)
      {
	lf->last_glyph = i;
	lf->last_used = p.x;

	/* TODO: incorrect if there is more than one line frag */
	if ([curParagraphStyle alignment] == NSRightTextAlignment)
	  [self rightAlignLine: line_frags : line_frags_num];
	else if ([curParagraphStyle alignment] == NSCenterTextAlignment)
	  [self centerAlignLine: line_frags : line_frags_num];
      }
    else
      {
	if ([curParagraphStyle lineBreakMode] == NSLineBreakByWordWrapping &&
	    [curParagraphStyle alignment] == NSJustifiedTextAlignment)
	  [self fullJustifyLine: line_frags : line_frags_num];
	else if ([curParagraphStyle alignment] == NSRightTextAlignment)
	  [self rightAlignLine: line_frags : line_frags_num];
	else if ([curParagraphStyle alignment] == NSCenterTextAlignment)
	  [self centerAlignLine: line_frags : line_frags_num];

	lfi--;
      }

    /* Layout is complete. Package it and give it to the layout manager. */
    [curLayoutManager setTextContainer: curTextContainer
		      forGlyphRange: NSMakeRange(cache_base, i)];
    curGlyph = i + cache_base;
    {
      line_frag_t *lf;
      NSPoint p;
      unsigned int i, j;
      glyph_cache_t *g;
      NSRect used_rect;

      for (lf = line_frags, i = 0, g = cache; lfi >= 0; lfi--, lf++)
	{
	  used_rect.origin.x = g->pos.x + lf->rect.origin.x;
	  used_rect.size.width = lf->last_used - g->pos.x;
	  /* TODO: be pickier about height? */
	  used_rect.origin.y = lf->rect.origin.y;
	  used_rect.size.height = lf->rect.size.height;

	  [curLayoutManager setLineFragmentRect: lf->rect
			    forGlyphRange: NSMakeRange(cache_base + i, lf->last_glyph - i)
			    usedRect: used_rect];
	  p = g->pos;
	  p.y += baseline;
	  j = i;
	  while (i < lf->last_glyph)
	    {
	      if (g->outside_line_frag)
		{
		  [curLayoutManager setDrawsOutsideLineFragment: YES
		    forGlyphAtIndex: cache_base + i];
		}
	      if (g->dont_show)
		{
		  [curLayoutManager setNotShownAttribute: YES
					 forGlyphAtIndex: cache_base + i];
		}
	      if (!g->nominal && i != j)
		{
		  [curLayoutManager setLocation: p
				    forStartOfGlyphRange: NSMakeRange(cache_base + j, i - j)];
		  if (g[-1].g == GSAttachmentGlyph)
		    {
		      [curLayoutManager setAttachmentSize: g[-1].size
			forGlyphRange: NSMakeRange(cache_base + j, i - j)];
		    }
		  p = g->pos;
		  p.y += baseline;
		  j = i;
		}
	      i++;
	      g++;
	    }
	  if (i != j)
	    {
	      [curLayoutManager setLocation: p
				forStartOfGlyphRange: NSMakeRange(cache_base + j, i - j)];
	      if (g[-1].g == GSAttachmentGlyph)
		{
		  [curLayoutManager setAttachmentSize: g[-1].size
		    forGlyphRange: NSMakeRange(cache_base + j, i - j)];
		}
	    }
	}
    }
  }

  curPoint = NSMakePoint(0, NSMaxY(line_frags->rect));

  if (newParagraph)
    return 3;
  else
    return 0;
}


-(int) layoutGlyphsInLayoutManager: (GSLayoutManager *)layoutManager
		   inTextContainer: (NSTextContainer *)textContainer
	      startingAtGlyphIndex: (unsigned int)glyphIndex
	  previousLineFragmentRect: (NSRect)previousLineFragRect
		    nextGlyphIndex: (unsigned int *)nextGlyphIndex
	     numberOfLineFragments: (unsigned int)howMany
{
  int ret, real_ret;
  BOOL newParagraph;

  if (![lock tryLock])
    {
      /* Since we might be the shared system typesetter, we must be
      reentrant. Thus, if we are already in use and can't lock our lock,
      we create a new instance and let it handle the call. */
      GSHorizontalTypesetter *temp;

      temp = [[object_getClass(self) alloc] init];
      ret = [temp layoutGlyphsInLayoutManager: layoutManager
			      inTextContainer: textContainer
			 startingAtGlyphIndex: glyphIndex
		     previousLineFragmentRect: previousLineFragRect
			       nextGlyphIndex: nextGlyphIndex
			numberOfLineFragments: howMany];
      DESTROY(temp);
      return ret;
    }

NS_DURING
  curLayoutManager = layoutManager;
  curTextContainer = textContainer;
  curTextStorage = [layoutManager textStorage];

/*	printf("*** layout some stuff |%@|\n", curTextStorage);
	[curLayoutManager _glyphDumpRuns];*/

  curGlyph = glyphIndex;

  [self _cacheClear];

  real_ret = 4;
  curPoint = NSMakePoint(0, NSMaxY(previousLineFragRect));
  while (1)
    {
      if (real_ret == 4)
	{
	  /*
	  -layoutLineNewParagraph: needs to know if the starting glyph is the
	  first glyph of a paragraph so it can apply eg. -firstLineHeadIndent and
	  paragraph spacing properly.
	  */
	  if (!curGlyph)
	    {
	      newParagraph = YES;
	    }
	  else
	    {
	      unsigned int chi;
	      unichar ch;
	      chi = [curLayoutManager characterRangeForGlyphRange: NSMakeRange(curGlyph - 1, 1)
						 actualGlyphRange: NULL].location;
	      ch = [[curTextStorage string] characterAtIndex: chi];
	
	      if (ch == '\n')
		newParagraph = YES;
	      else
		newParagraph = NO;
	    }
	}
      else if (real_ret == 3)
	{
	  newParagraph = YES;
	}
      else
	{
	  newParagraph = NO;
	}

      ret = [self layoutLineNewParagraph: newParagraph];

      real_ret = ret;
      if (ret == 3 || ret == 4)
	ret = 0;

      if (ret)
	break;

      if (howMany)
	if (!--howMany)
	  break;
   }

  *nextGlyphIndex = curGlyph;
NS_HANDLER
  NSLog(@"GSHorizontalTypesetter - %@", [localException reason]);
  [lock unlock];
  [localException raise];
  ret=0; /* This is never reached, but it shuts up the compiler. */
NS_ENDHANDLER
  [lock unlock];
  return ret;
}

@end

