/** <title>NSStringAdditions</title>

   <abstract>Categories which add drawing capabilities to NSAttributedString
   and NSString.</abstract>

   Copyright (C) 1999, 2003, 2004, 2017 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Mar 1999 - rewrite from scratch

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: November 2002 - February 2003 (rewrite to use NSLayoutManager et al)

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

#import <Foundation/NSException.h>
#import <Foundation/NSLock.h>

#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/DPSOperators.h"

/*
Apple uses this as the maximum width of an NSTextContainer.
For bigger values the width gets ignored.
*/
#define LARGE_SIZE 1e7


/*
A size of 16 and these constants give a hit rate of 80%-90% for normal app
use (based on real world statistics gathered with the help of some users
from #GNUstep).
We use the last entry of the cache as a scratch element to set up an initial 
text network.
*/
#define NUM_CACHE_ENTRIES 16
#define HIT_BOOST         2
#define MISS_COST         1


typedef struct
{
  int used;
  NSUInteger string_hash;
  BOOL hasSize, useScreenFonts;

  NSTextStorage *textStorage;
  NSLayoutManager *layoutManager;
  NSTextContainer *textContainer;

  NSSize givenSize;
  NSRect usedRect;
} cache_t;


static BOOL did_init = NO;
static cache_t cache[NUM_CACHE_ENTRIES + 1];


static NSRecursiveLock *cacheLock = nil;

/* For collecting statistics. */
//#define STATS

#ifdef STATS
static int total, hits, misses, hash_hits;

static void NSStringDrawing_dump_stats(void)
{
#define P(x) printf("%15i %s\n", x, #x);
  P(total)
  P(hits)
  P(misses)
  P(hash_hits)
#undef P
  printf("%15.8f hit ratio\n", hits / (double)total);
}
#endif

static void init_string_drawing(void)
{
  int i;
  NSTextStorage *textStorage;
  NSLayoutManager *layoutManager;
  NSTextContainer *textContainer;

  if (did_init)
    return;
  did_init = YES;

#ifdef STATS
  atexit(NSStringDrawing_dump_stats);
#endif
  
  for (i = 0; i < NUM_CACHE_ENTRIES + 1; i++)
    {
      textStorage = [[NSTextStorage alloc] init];
      layoutManager = [[NSLayoutManager alloc] init];
      [textStorage addLayoutManager: layoutManager];
      [layoutManager release];
      textContainer = [[NSTextContainer alloc]
			initWithContainerSize: NSMakeSize(10, 10)];
      [textContainer setLineFragmentPadding: 0];
      [layoutManager addTextContainer: textContainer];
      [textContainer release];

      cache[i].used = 0;
      cache[i].textStorage = textStorage;
      cache[i].layoutManager = layoutManager;
      cache[i].textContainer = textContainer;
    }
}

static inline void cache_lock()
{
  // FIXME: Put all the init code into an +initialize method
  // to let the runtime take care of it.
  if (cacheLock == nil)
    {
      cacheLock = [[NSRecursiveLock alloc] init];
    }
  [cacheLock lock];
  if (!did_init)
    {
      init_string_drawing();
    }
}

static inline void cache_unlock()
{
  [cacheLock unlock];
}

static inline BOOL is_size_match(cache_t *c, cache_t *scratch)
{
  if ((!c->hasSize && !scratch->hasSize) ||
      (c->hasSize && scratch->hasSize
       && NSEqualSizes(c->givenSize, scratch->givenSize)))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

static inline BOOL is_match(cache_t *c, cache_t *scratch)
{
  if (c->string_hash != scratch->string_hash
      || c->useScreenFonts != scratch->useScreenFonts)
    return NO;

#ifdef STATS
  hash_hits++;
#endif

  if (![scratch->textStorage isEqualToAttributedString: c->textStorage])
    return NO;

  /* String and attributes match, check size. */
  return is_size_match(c, scratch);
}

static cache_t *cache_match(cache_t *scratch, BOOL *matched)
{
  int i, j;
  cache_t *c;
  int least_used = -1;
  int replace = -1;

#ifdef STATS
  total++;
#endif

  /*
  A deterministic pattern for replacing cache entries can hit ugly worst
  cases on certain matching use patterns (where the cache is full of old
  unused entries, but the new entries keep replacing each other).

  By starting at a random index, we avoid this kind of problem.
  */
  j = rand() % NUM_CACHE_ENTRIES;
  for (i = 0; i < NUM_CACHE_ENTRIES; i++, j++)
    {
      if (j == NUM_CACHE_ENTRIES)
        {
          j = 0;
        }
      c = cache + j;
      if (least_used == -1 || c->used < least_used)
	{
	  least_used = c->used;
	  replace = j;
	}

      if (!c->used)
	continue;

      if (is_match(c, scratch))
	{
#ifdef STATS
	  hits++;
#endif

	  c->used += HIT_BOOST;
          *matched = YES;
	  return c;
	}
      else
        {
          if (c->used > MISS_COST)
            {
              c->used -= MISS_COST;
            }
          else
            {
              c->used = 1;
            }
        }
    }

  NSCAssert(replace != -1, @"Couldn't find a cache entry to replace.");

#ifdef STATS
  misses++;
#endif
  *matched = NO;

  /* We did not find a matching entry, return the least used one */
  return cache + replace;
}

static cache_t *cache_lookup(BOOL hasSize, NSSize size, BOOL useScreenFonts)
{
  BOOL hit;
  cache_t *c;
  cache_t *scratch = cache + NUM_CACHE_ENTRIES;

  scratch->used = 1;
  scratch->string_hash = [[scratch->textStorage string] hash];
  scratch->hasSize = hasSize;
  scratch->useScreenFonts = useScreenFonts;
  scratch->givenSize = size;
  
  c = cache_match(scratch, &hit);
  if (!hit)
    {
      // Swap c and scratch
      cache_t temp;

      temp = *c;
      *c = *scratch;
      *scratch = temp;

      // Cache miss, need to set up the text system
      if (hasSize)
        {
          [c->textContainer setContainerSize: NSMakeSize(size.width, size.height)];
        }
      else
        {
          [c->textContainer setContainerSize: NSMakeSize(LARGE_SIZE, LARGE_SIZE)];
        }
      [c->layoutManager setUsesScreenFonts: useScreenFonts];
      // Layout the whole container
      [c->layoutManager glyphRangeForTextContainer: c->textContainer];
      c->usedRect = [c->layoutManager usedRectForTextContainer: c->textContainer];
    }

  return c;
}

static inline void prepare_string(NSString *string, NSDictionary *attributes)
{
  cache_t *scratch = cache + NUM_CACHE_ENTRIES;
  NSTextStorage *scratchTextStorage = scratch->textStorage;

  [scratchTextStorage beginEditing];
  [scratchTextStorage replaceCharactersInRange: NSMakeRange(0, [scratchTextStorage length])
                                    withString: string];
  if ([string length])
    {
      [scratchTextStorage setAttributes: attributes
			  range: NSMakeRange(0, [string length])];
    }
  [scratchTextStorage endEditing];
}

static inline void prepare_attributed_string(NSAttributedString *string)
{
  cache_t *scratch = cache + NUM_CACHE_ENTRIES;
  NSTextStorage *scratchTextStorage = scratch->textStorage;

  [scratchTextStorage replaceCharactersInRange: NSMakeRange(0, [scratchTextStorage length])
                          withAttributedString: string];
}

static BOOL use_screen_fonts(void)
{
  NSGraphicsContext		*ctxt = GSCurrentContext();
  NSAffineTransform		*ctm = GSCurrentCTM(ctxt);
  NSAffineTransformStruct	ts = [ctm transformStruct];

  if (ts.m11 != 1.0 || ts.m12 != 0.0 || ts.m21 != 0.0 || fabs(ts.m22) != 1.0)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

/*
This is an ugly hack to get text to display correctly in non-flipped views.

The text system always has positive y down, so we flip the coordinate
system when drawing (if the view isn't flipped already). This causes the
glyphs to be drawn upside-down, so we need to tell NSFont to flip the fonts.
*/
@interface NSFont (FontFlipHack)
+(void) _setFontFlipHack: (BOOL)flip;
@end

static void draw_at_point(cache_t *c, NSPoint point)
{
  NSRange r;
  BOOL need_flip = ![[NSView focusView] isFlipped];
  NSGraphicsContext *ctxt = GSCurrentContext();

  r = NSMakeRange(0, [c->layoutManager numberOfGlyphs]);

  if (need_flip)
    {
      DPSscale(ctxt, 1, -1);
      point.y = -point.y;
      
      /*
        Adjust point.y so the lower left corner of the used rect is at the
        point that was passed to us.
      */
      point.y -= NSMaxY(c->usedRect);
	  
      [NSFont _setFontFlipHack: YES];
    }
      
  [c->layoutManager drawBackgroundForGlyphRange: r
                                        atPoint: point];
  [c->layoutManager drawGlyphsForGlyphRange: r
                                    atPoint: point];

  if (need_flip)
    {
      DPSscale(ctxt, 1, -1);
      [NSFont _setFontFlipHack: NO];
    }
}

static void draw_in_rect(cache_t *c, NSRect rect)
{
  NSRange r;
  BOOL need_flip = ![[NSView focusView] isFlipped];
  BOOL need_clip = NO;
  NSGraphicsContext *ctxt = GSCurrentContext();

  /*
    If the used rect fits completely in the rect we draw in, we save time
    by avoiding the DPSrectclip (and the state save and restore).
    
    This isn't completely safe; the used rect isn't guaranteed to contain
    all parts of all glyphs.
  */
  if (c->usedRect.origin.x >= 0 && c->usedRect.origin.y <= 0
      && NSMaxX(c->usedRect) <= rect.size.width
      && NSMaxY(c->usedRect) <= rect.size.height)
    {
      need_clip = NO;
    }
  else
    {
      need_clip = YES;
      DPSgsave(ctxt);
      DPSrectclip(ctxt, rect.origin.x, rect.origin.y,
                  rect.size.width, rect.size.height);
    }
    
  r = [c->layoutManager
          glyphRangeForBoundingRect: NSMakeRect(0, 0, rect.size.width,
                                                rect.size.height)
          inTextContainer: c->textContainer];

  if (need_flip)
    {
      DPSscale(ctxt, 1, -1);
      rect.origin.y = -NSMaxY(rect);
      [NSFont _setFontFlipHack: YES];
    }
  
  [c->layoutManager drawBackgroundForGlyphRange: r
                                        atPoint: rect.origin];
  [c->layoutManager drawGlyphsForGlyphRange: r
                                    atPoint: rect.origin];
  
  if (need_flip)
    {
      DPSscale(ctxt, 1, -1);
      [NSFont _setFontFlipHack: NO];
    }

  if (need_clip)
    {
      /* Restore the original clipping path. */
      DPSgrestore(ctxt);
    }
}

@implementation NSAttributedString (NSStringDrawing)

- (void) drawAtPoint: (NSPoint)point
{
  cache_t *c;

  cache_lock();
  NS_DURING
    {
      prepare_attributed_string(self);
      c = cache_lookup(NO, NSZeroSize, use_screen_fonts());

      draw_at_point(c, point);
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();
}

- (void) drawInRect: (NSRect)rect
{
  [self drawWithRect: rect
             options: NSStringDrawingUsesLineFragmentOrigin];
}

- (void) drawWithRect: (NSRect)rect
              options: (NSStringDrawingOptions)options
{
  // FIXME: This ignores options
  cache_t *c;

  if (rect.size.width <= 0 || rect.size.height <= 0)
    return;
      
  cache_lock();
  NS_DURING
    {
      prepare_attributed_string(self);
      c = cache_lookup(YES, rect.size, use_screen_fonts());
      draw_in_rect(c, rect);
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();
}

- (NSSize) size
{
  NSRect usedRect = [self boundingRectWithSize: NSZeroSize
                                       options: NSStringDrawingUsesLineFragmentOrigin];
  return usedRect.size;
}

- (NSRect) boundingRectWithSize: (NSSize)size
                        options: (NSStringDrawingOptions)options
{
  // FIXME: This ignores options
  cache_t *c;
  NSRect result = NSZeroRect;
  BOOL hasSize = !NSEqualSizes(NSZeroSize, size);

  cache_lock();
  NS_DURING
    {    
      prepare_attributed_string(self);
      c = cache_lookup(hasSize, size, YES);
      result = c->usedRect;
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();

  return result;
}

@end


@implementation NSString (NSStringDrawing)

- (void) drawAtPoint: (NSPoint)point withAttributes: (NSDictionary *)attrs
{
  cache_t *c;

  cache_lock();
  NS_DURING
    {
      prepare_string(self, attrs);
      c = cache_lookup(NO, NSZeroSize, use_screen_fonts());
      draw_at_point(c, point);
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();
}

- (void) drawInRect: (NSRect)rect withAttributes: (NSDictionary *)attrs
{
  [self drawWithRect: rect
             options: NSStringDrawingUsesLineFragmentOrigin
          attributes: attrs];
}

- (void) drawWithRect: (NSRect)rect
              options: (NSStringDrawingOptions)options
           attributes: (NSDictionary *)attrs
{
  // FIXME: This ignores options
  cache_t *c;

  if (rect.size.width <= 0 || rect.size.height <= 0)
    return;
  
  cache_lock();
  NS_DURING
    {    
      prepare_string(self, attrs);
      c = cache_lookup(YES, rect.size, use_screen_fonts());
      draw_in_rect(c, rect);
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();
}

- (NSSize) sizeWithAttributes: (NSDictionary *)attrs
{
  NSRect usedRect = [self boundingRectWithSize: NSZeroSize
                                       options: NSStringDrawingUsesLineFragmentOrigin
                                    attributes: attrs];
  return usedRect.size;
}

- (NSRect) boundingRectWithSize: (NSSize)size
                        options: (NSStringDrawingOptions)options
                     attributes: (NSDictionary *)attrs
{
  // FIXME: This ignores options
  cache_t *c;
  NSRect result = NSZeroRect;
  BOOL hasSize = !NSEqualSizes(NSZeroSize, size);

  cache_lock();
  NS_DURING
    {
      prepare_string(self, attrs);
      c = cache_lookup(hasSize, size, YES);
      result = c->usedRect;
    }
  NS_HANDLER
    {
      cache_unlock();
      [localException raise];
    }
  NS_ENDHANDLER;
  cache_unlock();

  return result;
}

@end


/*
Dummy function; see comment in NSApplication.m, +initialize.
*/
void GSStringDrawingDummyFunction(void)
{
}

