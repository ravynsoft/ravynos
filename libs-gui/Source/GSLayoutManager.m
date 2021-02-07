/*
   GSLayoutManager.m

   Copyright (C) 2002-2011 Free Software Foundation, Inc.

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

#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextView.h"

/* just for NSAttachmentCharacter */
#import "AppKit/NSTextAttachment.h"

#import "GNUstepGUI/GSFontInfo.h"
#import "GNUstepGUI/GSTypesetter.h"
#import "GNUstepGUI/GSLayoutManager_internal.h"

/* TODO: is using rand() here ok? */
static inline int random_level(void)
{
  int i;
  for (i = 0; i < SKIP_LIST_DEPTH - 2; i++)
    if ((rand() % SKIP_LIST_LEVEL_PROBABILITY) != 0)
      break;
  return i;
}

/*
 * Insert a new run with level into the context of the skip list.
 * Return the new run.
 */
static glyph_run_t *run_insert(glyph_run_head_t **context, int level)
{
  glyph_run_head_t *h;
  glyph_run_t *r;
  int i, size;

  size = sizeof(glyph_run_head_t) * level + sizeof(glyph_run_t);
  h = malloc(size);
  memset(h, 0, size);

  for (i = level; i >= 0; i--, h++)
    {
      h->next = context[i]->next;
      context[i]->next = h;
    }
  h--;

  r = (glyph_run_t *)h;
  r->level = level;
  r->prev = context[0];
  if (h->next)
    ((glyph_run_t *)h->next)->prev = h; 

  return r;
}

/*
 * Free the glphys of a run.
 */
static inline void run_free_glyphs(glyph_run_t *r)
{
  if (r->glyphs)
    {
      r->head.complete = 0;
      r->head.glyph_length = 0;
      free(r->glyphs);
      r->glyphs = NULL;
    }
 }

/*
 * Remove the run r from the context of the skip list and free it.
 * The context does not point at r, but to the run immediately before r.
 * context[0]->next == r
 */
static inline void run_remove(glyph_run_head_t **context, glyph_run_t *r)
{
  glyph_run_head_t *h;
  int i;

  // Free the glyphs
  run_free_glyphs(r);

  h = &r->head;
  if (h->next)
    ((glyph_run_t *)h->next)->prev = r->prev;

  for (i = 0; i <= r->level; i++)
    context[i]->next = context[i]->next->next;

  h -= r->level;
  free(h);
}

/* Recalculates char_length, glyph_length, and complete for a
glyph_run_head_t. All "children" of this head must have valid values. */
static void run_fix_head(glyph_run_head_t *h)
{
  glyph_run_head_t *h2, *next;

  next = h->next;
  if (next)
    next++;
  h2 = h + 1;

  h->complete = 1;
  h->glyph_length = 0;
  h->char_length = 0;

  // Loop over all heads below this one
  while (h2 != next)
    {
      h->char_length += h2->char_length;
      if (!h2->complete)
        h->complete = 0;
      else if (h->complete)
        h->glyph_length += h2->glyph_length;
      h2 = h2->next;
    }
}

/*
Private method used internally by GSLayoutManager for sanity checking.
*/
@interface NSTextStorage (GSLayoutManagerSanityChecking)
-(unsigned int) _editCount;
@end
@implementation NSTextStorage (GSLayoutManagerSanityChecking)
-(unsigned int) _editCount;
{
  return _editCount;
}
@end

@interface GSLayoutManager (backend)
-(unsigned int) _findSafeBreakMovingBackwardFrom: (unsigned int)ch;
-(unsigned int) _findSafeBreakMovingForwardFrom: (unsigned int)ch;
-(void) _generateGlyphsForRun: (glyph_run_t *)run  at: (unsigned int)cpos;
@end

/***** Glyph handling *****/

@implementation GSLayoutManager (GlyphsHelpers)

-(void) _run_cache_attributes: (glyph_run_t *)r : (NSDictionary *)attributes
{
  /* set up attributes for this run */
  NSNumber *n;
  NSFont *font;

  r->explicit_kern = !![attributes objectForKey: NSKernAttributeName];

  n = [attributes objectForKey: NSLigatureAttributeName];
  if (n)
    r->ligature = [n intValue];
  else
    r->ligature = 1;

  font = [typesetter fontForCharactersWithAttributes: attributes];
  /* TODO: it might be useful to change this slightly:
  Returning a nil font from -fontForCharactersWithAttributes: causes those
  characters to not be displayed (ie. no glyphs are generated).

  How would glyph<->char mapping be handled? Map the entire run to one
  NSNullGlyph?
  */
  if (font == nil)
    font = [NSFont userFontOfSize: 0];
  font = [self substituteFontForFont: font];
  ASSIGN(r->font, font);
}

-(void) _run_free_attributes: (glyph_run_t *)r
{
  [r->font release];
}

-(void) _run_copy_attributes: (glyph_run_t *)dst : (const glyph_run_t *)src
{
  dst->font = [src->font copy];
  dst->ligature = src->ligature;
  dst->explicit_kern = src->explicit_kern;
}

/*
 * Free up the skip list and all the glyph arrays
 */
- (void) _freeGlyphs
{
  glyph_run_t *cur, *next;
  glyph_run_head_t *h;

  if (!glyphs)
    return;

  cached_run = NULL;

  h = glyphs;
  h += SKIP_LIST_DEPTH - 1;

  for (cur = (glyph_run_t *)h->next; cur; cur = next)
    {
      next = (glyph_run_t *)cur->head.next;
      if (cur->glyphs)
        free(cur->glyphs);
      [self _run_free_attributes: cur];
      // Find the start of the allocated memory
      h = &cur->head;
      h -= cur->level;
      free(h);
    }

  // Free the head element
  free(glyphs);
  glyphs = NULL;
}

/*
 * Initialize the glyph skip list
 */
- (void) _initGlyphs
{
  int i, size;
  glyph_run_t *r;
  glyph_run_head_t *h;

  size = sizeof(glyph_run_head_t) * (SKIP_LIST_DEPTH - 1) + sizeof(glyph_run_t);
  glyphs = malloc(size);
  memset(glyphs, 0, size);

  for (h = glyphs, i = SKIP_LIST_DEPTH; i; i--, h++)
    {
      h->complete = 1;
    }
  h--;

  r = (glyph_run_t *)h;
  r->level = SKIP_LIST_DEPTH - 1;
}

- (void) _glyphDumpRuns
{
  printf("--- dumping runs\n");
  {
    glyph_run_t *h;
    unsigned int cpos = 0;

    h = (glyph_run_t *)(glyphs + SKIP_LIST_DEPTH - 1)->next;
    for (; h; h = (glyph_run_t *)h->head.next)
      {
        printf("%8p %i chars, %i glyphs, %i complete, prev %8p next %8p\n",
               h, h->head.char_length, h->head.glyph_length, h->head.complete,
               h->prev, h->head.next);
        printf("         level %i, continued %i\n", h->level, h->continued);
        if (h->head.complete)
          {
            unsigned int i;
            printf("glyphs:\n");
            for (i = 0;i < h->head.glyph_length;i++)
              printf("%5i %04x u%04x  ",
                     h->glyphs[i].char_offset,h->glyphs[i].g,
                     [[_textStorage string] characterAtIndex: cpos+h->glyphs[i].char_offset]);
            printf("\n");
          }
        cpos += h->head.char_length;
      }
  }
  printf("- structure\n");
  {
    glyph_run_head_t *h, *g;
    int i;

    printf("    head: ");
    for (i = 0, h = glyphs + SKIP_LIST_DEPTH - 1; i < SKIP_LIST_DEPTH; i++, h--)
      printf("%8p %i %3i %3i|", h->next, h->complete, h->char_length, h->glyph_length);
    printf("\n");
    h = (glyphs + SKIP_LIST_DEPTH - 1)->next;
    for (; h; h = h->next)
      {
        printf("%8p: ", h);
        for (g = h, i = ((glyph_run_t *)h)->level; i >= 0; i--, g--)
          printf("%8p %i %3i %3i|", g->next, g->complete, g->char_length, g->glyph_length);
        printf("\n");
      }
  }
  printf("--- done\n");
  fflush(stdout);
}

- (void) _sanityChecks
{
  glyph_run_t *g;

  g = (glyph_run_t *)&glyphs[SKIP_LIST_DEPTH - 1];
  while (g->head.next)
    {
      NSAssert((glyph_run_t *)((glyph_run_t *)g->head.next)->prev == g,
               @"glyph structure corrupted: g->next->prev!=g");
      g = (glyph_run_t *)g->head.next;
    }
}


/*
 * Returns the glyph run that contains glyphIndex, if there is any.
 * glyph_pos and char_pos, when supplied, will contain the starting 
 * glyph/character index for this run.
 */
- (glyph_run_t *)run_for_glyph_index: (unsigned int)glyphIndex
				    : (unsigned int *)glyph_pos
				    : (unsigned int *)char_pos
{
  int level;
  glyph_run_head_t *h;
  int pos, cpos;

  if (glyphs->glyph_length <= glyphIndex)
    {
      NSLog(@"run_for_glyph_index failed for %d", glyphIndex);
      return NULL;
    }

  if (cached_run)
    {
      if (glyphIndex >= cached_pos &&
          glyphIndex < cached_pos + cached_run->head.glyph_length)
        {
          if (glyph_pos)
            *glyph_pos = cached_pos;
          if (char_pos)
            *char_pos = cached_cpos;
          return cached_run;
        }
    }

  pos = cpos = 0;
  level = SKIP_LIST_DEPTH;
  h = glyphs;
  while (1)
    {
      // Find a head, where the glyphs are already created.
      if (!h->complete)
        {
          h++;
          level--;
          if (!level)
            {
              NSLog(@"run_for_glyph_index failed for %d", glyphIndex);
              return NULL;
            }
          continue;
        }
      // Find the head containing the index.
      if (glyphIndex >= pos + h->glyph_length)
        {
          pos += h->glyph_length;
          cpos += h->char_length;
          h = h->next;
          if (!h)
            {
              NSLog(@"run_for_glyph_index failed for %d", glyphIndex);
              return NULL;
            }
          continue;
        }
      // Go down one level
      if (level > 1)
        {
          h++;
          level--;
          continue;
        }

      // Level 1
      if (glyph_pos)
        *glyph_pos = pos;
      if (char_pos)
        *char_pos = cpos;

      cached_run = (glyph_run_t *)h;
      cached_pos = pos;
      cached_cpos = cpos;
      
      return (glyph_run_t *)h;
    }
}

/*
 * Returns the glyph run that contains charIndex, if there is any.
 * glyph_pos and char_pos, when supplied, will contain the starting 
 * glyph/character index for this run.
 */
- (glyph_run_t *)run_for_character_index: (unsigned int)charIndex
					: (unsigned int *)glyph_pos
					: (unsigned int *)char_pos
{
  int level;
  glyph_run_head_t *h;
  int pos, cpos;
  BOOL cache = YES;

  if (glyphs->char_length <= charIndex)
    {
      NSLog(@"run_for_character_index failed for %d", charIndex);
      return NULL;
    }

  if (cached_run)
    {
      if (charIndex >= cached_cpos &&
          charIndex < cached_cpos + cached_run->head.char_length)
        {
          if (glyph_pos)
            *glyph_pos = cached_pos;
          if (char_pos)
            *char_pos = cached_cpos;
          return cached_run;
        }
    }

  pos = cpos = 0;
  h = glyphs;
  for (level = SKIP_LIST_DEPTH - 1; level >= 0; level--)
    {
      // Find the head containing the index.
      while (charIndex >= cpos + h->char_length)
        {
          // Ignore pos at the end
          if (!h->complete)
            cache = NO;
          pos += h->glyph_length;
          cpos += h->char_length;
          h = h->next;
          if (!h)
            {
              NSLog(@"run_for_character_index failed for %d", charIndex);
              return NULL;
            }
        }

      // Go down one level
      h++;
    }
  h--;

  if (glyph_pos)
    *glyph_pos = pos;
  if (char_pos)
    *char_pos = cpos;
  
  if (cache) 
    {
      cached_run = (glyph_run_t *)h;
      cached_pos = pos;
      cached_cpos = cpos;
    }

   return (glyph_run_t *)h;
}

/*
 * Generate the glyph runs, but not the actual glyphs, up to the 
 * character index last.
 * Build up empty skip list entries to later hold the glyphs.
 * Only appends to the end of the skip list.
 * Only called after setting up a complete new layout.
 */
-(void) _generateRunsToCharacter: (unsigned int)last
{
  glyph_run_head_t *context[SKIP_LIST_DEPTH];
  glyph_run_head_t *h;
  unsigned int pos;
  unsigned int length;
  int level;

  length = [_textStorage length];
  if (last >= length)
    last = length - 1;

  h = glyphs;
  pos = 0;
  if (h->char_length > last)
    return;

  /* We haven't created any run for that character. Find the last run. */
  for (level = SKIP_LIST_DEPTH; level; level--)
    {
      while (h->next) pos += h->char_length, h = h->next;
      context[level - 1] = h;
      h++;
    }
  h--;
  pos += h->char_length;

  /* Create runs and add them to the skip list until we're past our
     target. */
  while (pos <= last)
    {
      NSRange maxRange;
      NSRange curRange;
      NSDictionary *attributes;
      glyph_run_t *new;
      int new_level;
      int i;

      maxRange = NSMakeRange(pos, length - pos);
      if (pos > 0)
        {
          maxRange.location--;
          maxRange.length++;
        }

      attributes = [_textStorage attributesAtIndex: pos
                                 longestEffectiveRange: &curRange
                                 inRange: maxRange];

      /*
      Optimize run structure by merging with the previous run under
      some circumstances. See the comments in
      -invalidateGlyphsForCharacterRange:changeInLength:actualCharacterRange:
      for more information.
      */
      if (curRange.location < pos && context[0]->char_length &&
          // FIXME: Why 16 and not MAX_RUN_LENGTH
          context[0]->char_length < 16)
        {
          curRange.length -= pos - curRange.location;
          curRange.location = pos;
          new = (glyph_run_t *)context[0];
          // FIXME: We could try to reuse the glyphs
          run_free_glyphs(new);
          new->head.char_length += curRange.length;
          for (i = 1; i < SKIP_LIST_DEPTH; i++)
            {
              run_fix_head(context[i]);
            }
          pos = NSMaxRange(curRange);
          continue;
        }

      if (curRange.location < pos)
        {
          curRange.length -= pos - curRange.location;
          curRange.location = pos;
        }

      /*
      TODO: this shouldn't really be necessary if all searches inside runs
      were binary. but for now, it helps performance, and it keeps things
      more balanced when there are long runs of text.
      */
      if (curRange.length > MAX_RUN_LENGTH)
        {
          unsigned int safe_break = curRange.location + MAX_RUN_LENGTH;
          safe_break = [self _findSafeBreakMovingForwardFrom: safe_break];
          if (safe_break < NSMaxRange(curRange))
            curRange.length = safe_break - curRange.location;
        }

      /* Since we'll be creating these in order, we can be smart about
         picking new levels. */
      {
        int i;

        /*
          FIXME: Not sure whether using an ivar here as the counter is a great idea.
          When the same range is edited over and over again this could lead to a strange structure.
          It works fine when adding a great chunk of text at the end.
        */
        glyph_num_end_runs++;
        for (i=0; i < SKIP_LIST_DEPTH - 2; i++)
          if (glyph_num_end_runs & (1 << i))
            break;
        new_level = i;
      }

      new = run_insert(context, new_level);
      [self _run_cache_attributes: new : attributes];

      h = &new->head;
      for (i = 0; i <= new_level; i++, h--)
        {
          h->char_length = curRange.length;
          context[i] = h;
        }

      for (; i < SKIP_LIST_DEPTH; i++)
        {
          context[i]->char_length += curRange.length;
          context[i]->complete = 0;
        }

      pos += curRange.length;
    }

  [self _sanityChecks];
}

/*
Recursive glyph generation helper method method.
Returns number of valid glyphs under h after generating up to last (sortof,
not completely accurate).
Fills in all glyph holes up to last. only looking at levels below level
*/
-(unsigned int) _generateGlyphs_char_r: (unsigned int)last : (unsigned int)cpos
                                      : (unsigned int)gpos : (int)level
                                      : (glyph_run_head_t *)h : (glyph_run_head_t *)stop
                                      : (BOOL *)all_complete
{
  int total = 0, sub_total;
  BOOL c;

  *all_complete = YES;
  while (h != stop && (cpos <= last || *all_complete))
    {
      if (!h->complete)
        {
          if (cpos > last)
            {
              *all_complete = NO;
              break;
            }

          if (level)
            {
              glyph_run_head_t *stopn;

              if (h->next)
                stopn = h->next + 1;
              else
                stopn = NULL;

              sub_total = [self _generateGlyphs_char_r: last : cpos : gpos + total 
                                : level - 1 : h + 1: stopn: &c];
              if (!c)
                *all_complete = NO;
              else
                h->complete = 1;
              h->glyph_length = sub_total;
            }
          else
            {
              NSUInteger cindex = cpos;
              NSUInteger gindex = gpos + total;
              
              // Cache the current run
              cached_run = (glyph_run_t *)h;
              cached_pos = gindex;
              cached_cpos = cindex;
              
              // Generate the glyphs for the run
              [_glyphGenerator generateGlyphsForGlyphStorage: self 
                               desiredNumberOfCharacters: h->char_length
                               glyphIndex: &gindex
                               characterIndex: &cindex];
              h->complete = 1;
            }
        }

      total += h->glyph_length;
      cpos += h->char_length;
      h = h->next;
    }

  return total;
}

/*
 * Generate all glyphs up to the character index last.
 */
-(void) _generateGlyphsUpToCharacter: (unsigned int)last
{
  unsigned int length;
  BOOL dummy;

  if (!_textStorage)
    return;

  /*
  Trying to do anything here while the text storage has unprocessed edits
  (ie. an edit count>0) breaks things badly, and in strange ways. Thus, we
  detect this and raise an exception.
  */
  if ([_textStorage _editCount])
    {
      [NSException raise: NSGenericException
	      format: @"Glyph generation was triggered for a layout manager "
		      @"while the text storage it was attached to had "
		      @"unprocessed editing. This is not allowed. Glyph "
		      @"generation may be triggered only at points where "
		      @"calls to -beginEditing and -endEditing are "
		      @"balanced."];
    }

  length = [_textStorage length];
  if (!length)
    return;
  if (last >= length)
    last = length - 1;

  if (glyphs->char_length <= last)
    [self _generateRunsToCharacter: last];

  // [self _glyphDumpRuns];
  [self _generateGlyphs_char_r: last : 0 : 0 : SKIP_LIST_DEPTH - 1: glyphs : NULL : &dummy];
  // [self _glyphDumpRuns];
}

-(void) _generateGlyphsUpToGlyph: (unsigned int)last
{
  unsigned int length;

  if (!_textStorage)
    return;
  length = [_textStorage length];

  while (glyphs->glyph_length <= last && (glyphs->char_length < length || !glyphs->complete))
    {
      // Make an estimate for the character position
      unsigned int char_last;

      if (glyphs->glyph_length == 0)
        char_last = last;
      else
        char_last = glyphs->char_length + 1 +
          (last - glyphs->glyph_length) * (glyphs->char_length / (glyphs->glyph_length + 1));

      [self _generateGlyphsUpToCharacter: char_last];
    }
}

/*
 * Find the glyph run that contains target and the glyph that matches to that char index.
 */
- (glyph_run_t *) _glyphForCharacter: (unsigned int)target
                               index: (unsigned int *)rindex
                           positions: (unsigned int *)rpos 
                                    : (unsigned int *)rcpos
{
  glyph_run_t *r;
  unsigned int pos, cpos;
  int lo, hi, mid, i;

  r = [self run_for_character_index: target : &pos : &cpos];
  if (!r)
    {
      [NSException raise: NSRangeException
                   format: @"%s character index %d out of range", __PRETTY_FUNCTION__, target];
      return NULL;
    }

  if (!r->glyphs)
    {
      // range, but no glyphs, may be an empty glyph run
      *rindex = 0;	// FIXME ... is this right?
      *rpos = pos;
      *rcpos = cpos;
      return r;
    }

  target -= cpos;

  lo = 0;
  hi = r->head.glyph_length - 1;
  while (lo < hi)
    {
      mid = (lo + hi) / 2;
      if (r->glyphs[mid].char_offset > target)
        hi = mid - 1;
      else if (r->glyphs[mid].char_offset < target)
        lo = mid + 1;
      else
        hi = lo = mid;
    }

  // This final correction is needed as multiple glyph may have
  // the same character offset and vise versa.
  i = lo;
  while (r->glyphs[i].char_offset > target)
    i--;
  while (i > 0 && r->glyphs[i - 1].char_offset == r->glyphs[i].char_offset)
    i--;

  *rindex = i;
  *rpos = pos;
  *rcpos = cpos;
  return r;
}

@end


@implementation GSLayoutManager (glyphs)

- (unsigned int) numberOfGlyphs
{
  [self _generateGlyphsUpToCharacter: -1];
  return glyphs->glyph_length;
}

- (NSGlyph) glyphAtIndex: (unsigned int)glyphIndex
{
  BOOL valid;
  NSGlyph g;

  g = [self glyphAtIndex: glyphIndex isValidIndex: &valid];
  if (!valid)
    [NSException raise: NSRangeException
                 format: @"%s glyph index out of range", __PRETTY_FUNCTION__];

  return g;
}

- (NSGlyph) glyphAtIndex: (unsigned int)glyphIndex
            isValidIndex: (BOOL *)isValidIndex
{
  glyph_run_t *r;
  unsigned int pos;

  if (isValidIndex != NULL)
    *isValidIndex = NO;

  /* glyph '-1' is returned in other places as an "invalid" marker; this
  way, we can say that it isn't valid without building all glyphs */
  /* TODO: check if this is really safe or smart. if it isn't, some other
  methods will need to be changed so they can return "no glyph index" in
  some other way. */
  if (glyphIndex == (unsigned int)-1)
    return NSNullGlyph;

  if (glyphs->glyph_length <= glyphIndex)
    {
      [self _generateGlyphsUpToGlyph: glyphIndex];
      if (glyphs->glyph_length <= glyphIndex)
        return NSNullGlyph;
    }

  r = run_for_glyph_index(glyphIndex, glyphs, &pos, NULL);
  if (!r || !r->glyphs) /* shouldn't happen */
    return NSNullGlyph;

  if (isValidIndex != NULL)
    *isValidIndex = YES;

  return r->glyphs[glyphIndex - pos].g;
}

- (BOOL) isValidGlyphIndex: (unsigned int)glyphIndex
{
 if (glyphIndex == (unsigned int)-1)
    return NO;

  if (glyphs->glyph_length <= glyphIndex)
    {
      return NO;
    }
  else 
    {
      return YES;
    }
}

- (unsigned int) getGlyphs: (NSGlyph *)glyphArray
                     range: (NSRange)glyphRange
{
  glyph_run_t *r;
  NSGlyph *g;
  unsigned int pos;
  unsigned int num;
  unsigned int i, j, k;

  if (glyphRange.length == 0)
    {
      return 0;
    }

  pos = NSMaxRange(glyphRange) - 1;
  if (glyphs->glyph_length <= pos)
    {
      [self _generateGlyphsUpToGlyph: pos];
      if (glyphs->glyph_length <= pos)
        {
          [NSException raise: NSRangeException
                       format: @"%s glyph range out of range", __PRETTY_FUNCTION__];
          return 0;
        }
    }

  r = run_for_glyph_index(glyphRange.location, glyphs, &pos, NULL);
  if (!r)
    { /* shouldn't happen */
      [NSException raise: NSRangeException
		   format: @"%s glyph range out of range", __PRETTY_FUNCTION__];
      return 0;
    }

  g = glyphArray;
  num = 0;

  while (1)
    {
      if (pos < glyphRange.location)
        j = glyphRange.location - pos;
      else
        j = 0;

      k = NSMaxRange(glyphRange) - pos;
      if (k > r->head.glyph_length)
        k = r->head.glyph_length;
      if (k <= j)
        break;

      /* TODO? only "displayed" glyphs */
      for (i = j; i < k; i++)
        {
          *g++ = r->glyphs[i].g;
          num++;
        }

      pos += r->head.glyph_length;
      r = (glyph_run_t *)r->head.next;
      if (!r)
        break;
    }

  return num;
}

- (unsigned int) characterIndexForGlyphAtIndex: (unsigned int)glyphIndex
{
  glyph_run_t *r;
  unsigned int pos, cpos;

  if (glyphs->glyph_length <= glyphIndex)
    {
      [self _generateGlyphsUpToGlyph: glyphIndex];
      if (glyphs->glyph_length <= glyphIndex)
        {
          [NSException raise: NSRangeException
                       format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
          return 0;
        }
    }

  r = run_for_glyph_index(glyphIndex, glyphs, &pos, &cpos);
  if (!r)
    {
      [NSException raise: NSRangeException
                   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return 0;
    }

  if (r->head.glyph_length <= glyphIndex - pos)
    {
      return cpos;
    }

  return cpos + r->glyphs[glyphIndex - pos].char_offset;
}

- (NSRange) characterRangeForGlyphRange: (NSRange)glyphRange
                       actualGlyphRange: (NSRange *)actualGlyphRange
{
  glyph_run_t *r;
  NSRange real_range, char_range;
  unsigned int cpos, pos;
  unsigned j;

  if (NSMaxRange(glyphRange) == 0)
    {
      if (actualGlyphRange)
        *actualGlyphRange = glyphRange;
      return NSMakeRange(0, 0);
    }

  pos = NSMaxRange(glyphRange) - 1;
  if (glyphs->glyph_length <= pos)
    {
      [self _generateGlyphsUpToGlyph: pos];
      if (glyphs->glyph_length <= pos)
        {
          [NSException raise: NSRangeException
                       format: @"%s glyph range out of range", __PRETTY_FUNCTION__];
          return NSMakeRange(0, 0);
        }
    }

  r = run_for_glyph_index(glyphRange.location, glyphs, &pos, &cpos);
  if (!r)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph range out of range", __PRETTY_FUNCTION__];
      return NSMakeRange(0, 0);
    }

  if (r->head.glyph_length <= glyphRange.location - pos)
    {
      j = cpos;
    }
  else
    {
      j = cpos + r->glyphs[glyphRange.location - pos].char_offset;
    }
  char_range.location = j;

  /* scan backwards to find the real first glyph */
  {
    glyph_run_t *r2;
    unsigned int adj, cadj;
    int i;

    i = glyphRange.location - pos;
    r2 = r;
    adj = pos;
    cadj = cpos;
    while ((r2->head.glyph_length > i) && 
           (r2->glyphs[i].char_offset + cadj == j))
      {
        i--;
        while (i < 0)
          {
            if (!r2->prev)
              break;
            r2 = (glyph_run_t *)r2->prev;
            i = r2->head.glyph_length - 1;
            adj -= r2->head.glyph_length;
            cadj -= r2->head.char_length;
          }
        if (i < 0)
          break;
      }
    real_range.location = i + 1 + adj;
  }

  /* the range is likely short, so we can do better then a completely new
     search */
  r = run_for_glyph_index(glyphRange.location + glyphRange.length - 1,
                          glyphs, &pos, &cpos);
  if (!r)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph range out of range", __PRETTY_FUNCTION__];
      return NSMakeRange(0, 0);
    }

  if (r->head.glyph_length <= glyphRange.location + glyphRange.length - 1 - pos)
    {
      j = cpos;
    }
  else
    {
      j = cpos + r->glyphs[glyphRange.location + glyphRange.length - 1 - pos].char_offset;
    }

  /* scan forwards to find the real last glyph */
  {
    glyph_run_t *r2;
    unsigned int adj, cadj;
    unsigned int last = 0;
    unsigned int i;

    i = glyphRange.location + glyphRange.length - 1 - pos;
    r2 = r;
    adj = pos;
    cadj = cpos;
    while ((r2->head.glyph_length > i) && 
           (r2->glyphs[i].char_offset + cadj == j))
      {
        GLYPH_STEP_FORWARD(r2,i,adj,cadj)
        if (i==r2->head.glyph_length)
          {
            last = cadj + r2->head.char_length;
            goto found;
          }
      }
    if (r2->head.glyph_length > i)
      {
        last = r2->glyphs[i].char_offset + cadj;
      }
    else
      {
        last = j;
      }
  found:
    real_range.length = i + adj - real_range.location;
    char_range.length = last - char_range.location;
  }

  if (actualGlyphRange)
    *actualGlyphRange = real_range;
  return char_range;
}

- (NSRange) glyphRangeForCharacterRange: (NSRange)charRange 
                   actualCharacterRange: (NSRange *)actualCharRange
{
  NSRange char_range, glyph_range;
  glyph_run_t *r;
  unsigned int cpos, pos;
  unsigned int i, target;

  if (charRange.length == 0 && charRange.location == 0)
    {
      if (actualCharRange)
        *actualCharRange = charRange;
      return NSMakeRange(0, 0);
    }
  target = charRange.location;
  pos = NSMaxRange(charRange) - 1;
  [self _generateGlyphsUpToCharacter: pos];
  if (glyphs->char_length <= pos || glyphs->char_length <= target)
    {
      if (actualCharRange)
        *actualCharRange = NSMakeRange([[_textStorage string] length], 0);
      return NSMakeRange([self numberOfGlyphs], 0);
    }

  r = [self _glyphForCharacter: target
            index: &i
            positions: &pos : &cpos];
  glyph_range.location = i + pos;
  if (r->head.glyph_length > i)
    {
      char_range.location = r->glyphs[i].char_offset + cpos;
    }
  else
    {
      char_range.location = cpos;
    }

  target = NSMaxRange(charRange) - 1;
  r = [self _glyphForCharacter: target
            index: &i
            positions: &pos : &cpos];
  if (r->head.glyph_length > i)
    {
      GLYPH_SCAN_FORWARD(r, i, pos, cpos, r->glyphs[i].char_offset + cpos <= target)
    }

  glyph_range.length = i + pos - glyph_range.location;
  if (i >= r->head.glyph_length)
    char_range.length = glyphs->char_length - char_range.location;
  else
    char_range.length = r->glyphs[i].char_offset + cpos - char_range.location;

  if (actualCharRange)
    *actualCharRange = char_range;
  return glyph_range;
}


/*
TODO? this might currently lead to continued runs not being marked as
continued runs. this will only happen at safe break spots, though, so
it should still be safe. might lose opportunities to merge runs, though.
*/

/*
This is hairy.

The ranges passed in and out of this method are ranges _after_ the change.
Internally, we switch between before- and after-indices. Comments mark the
places where we switch.
*/
- (void) invalidateGlyphsForCharacterRange: (NSRange)range
                            changeInLength: (int)lengthChange
                      actualCharacterRange: (NSRange *)actualRange
{
  glyph_run_head_t *context[SKIP_LIST_DEPTH];
  glyph_run_head_t *h;
  glyph_run_t *r;
  NSRange rng;
  int position[SKIP_LIST_DEPTH];
  unsigned int cpos;
  int level;
  unsigned int ch;
  unsigned int max;

  /*
  We always clear out the cached run information to be safe. This is only needed 
  if the cached run is affected by the invalidation, that is if
  NSMinRange(range) < cpos + cached_run->head.char_lenght
  */
  cached_run = NULL;

  /* Set it now for early returns. */
  if (actualRange)
    *actualRange = range;

//  printf("\n +++ range=(%i+%i) lengthChange=%i\n", range.location, range.length, lengthChange);
  [self _sanityChecks];
//  [self _glyphDumpRuns];

  if ((range.location == 0) && (range.length >= [_textStorage length]))
    {
      // Full invalidation
      [self _invalidateEverything];
      return;
    }

  /*
  Find out what range we actually need to invalidate. This depends on how
  context affects glyph generation.
  */
  ch = range.location;
  if (ch > 0)
    {
      ch = [self _findSafeBreakMovingBackwardFrom: ch];
      range.length += range.location - ch;
      range.location = ch;
    }

  max = ch + range.length;
  if (max < [_textStorage length])
    {
      max = [self _findSafeBreakMovingForwardFrom: max];
      range.length = max - range.location;
    }
  //  printf("adjusted to %i+%i\n", range.location, range.length);

  // Last affected character (indix before the change).
  max -= lengthChange;

  /*
  Find the first run (and context) for the range.
  */
  h = glyphs;
  cpos = 0;
  for (level = SKIP_LIST_DEPTH - 1; level >= 0; level--)
    {
      while (cpos + h->char_length <= ch)
        {
          cpos += h->char_length;
          h = h->next;
          if (!h)
            {
              /*
                No runs have been created for the range, so there's nothing
                to invalidate.
              */
//	      printf("no runs created yet\n");
              return;
            }
        }
      context[level] = h;
      position[level] = cpos;
      h++;
    }
  h--;
  r = (glyph_run_t *)h;

  /*
  Now we have the first run that intersects the range we're invalidating
  in 'r' (and context in 'context' and 'position').
  */

  //printf("split if %i+%i > %i+%i\n", cpos, r->head.char_length, ch, range.length);
  /*
  If 'r' extends beyond the invalidated range, split off the trailing, valid
  part to a new run. The reason we need to do this is that we must have runs
  for the first glyph not invalidated or the deletion loop below will fail.
  */
  if (cpos + r->head.char_length > max && ch != cpos)
    {
      glyph_run_t *new;
      glyph_run_head_t *hn;
      int i;

      new = run_insert(context,  random_level());
      new->head.char_length = cpos + r->head.char_length - max;
      [self _run_copy_attributes: new : r];

      /* OPT: keep valid glyphs
      this seems to be a fairly rare case
      */
      hn = &new->head;
      hn--;
      for (i = 1; i <= new->level; i++, hn--)
        {
          // FIXME: Use simpler adjustment
          run_fix_head(hn);
        }

      r->head.char_length -= new->head.char_length;
      // Glyphs get freed later
    }

  /*
  Set things up. We want 'r' to be the last run we want to keep.
  */
  if (ch == cpos)
    {
      /*
      This run begins exactly at the beginning of the invalidated range.
      Since we want 'r' to be the last run to keep, we actually want it
      to be the previous run. Thus, we step backwards in the skip list
      to get the previous run.
      */
      glyph_run_head_t *h2;

      h2 = h - r->level;
      h = context[r->level + 1];
      cpos = position[r->level + 1];
      h++;
      for (level = r->level; level >= 0; level--)
        {
          while (h->next != h2)
            {
              cpos += h->char_length;
              h = h->next;
            }
          // Fix up old context before switching
          if (level)
            run_fix_head(context[level]);

          position[level] = cpos;
          context[level] = h;
          h++;
          h2++;
        }
      h--;
      r = (glyph_run_t *)h;
      cpos += r->head.char_length;
    }
  else
    {
      /*
      This run begins before the invalidated range. Resize it so it ends
      just before it.
      */
      int len = r->head.char_length;

      r->head.char_length = ch - cpos;
      cpos += len;
      /* OPT!!! keep valid glyphs */
      run_free_glyphs(r);
    }

  /*
  'r' is the last run we should keep, 'context' and 'position' are set up
  for it. cpos

  Now we delete all runs completely invalidated.
  */
  {
    glyph_run_t *next;

    while (1)
      {
        next = (glyph_run_t *)r->head.next;

        /* We reached the end of all created runs. */
        if (!next)
          break;

        NSAssert(max >= cpos,
                 @"no run for first glyph beyond invalidated range");
        
        /* Clean cut, just stop. */
        if (max == cpos)
          break;
        
        /*
          Part of this run extends beyond the invalidated range. Resize it
          so it's completely beyond the invalidated range and stop.
        */
        if (max < cpos + next->head.char_length)
          {
            glyph_run_head_t *hn;
            int i;

            /* adjust final run */
            /* OPT!!! keep valid glyphs */
            run_free_glyphs(next);

            next->head.char_length -= max - cpos;
            
            hn = &next->head;
            hn--;
            for (i = 1; i <= next->level; i++, hn--)
              run_fix_head(hn);
            
            break;
          }

        cpos += next->head.char_length;

        /*
          This run is completely inside the invalidated range. Remove it.
          The context run heads will be adjusted later.
        */
        [self _run_free_attributes: next];
        run_remove(context, next);
      }
  }

/*  printf("deleted\n");
  [self _glyphDumpRuns];*/

  /*
  From now one we are use indexes after after the length change.
  'r' is the last run we want to keep, and the next run is the next
  uninvalidated run. We need to insert new runs for invalidated range
  after 'r'.

  As we create new runs, we move the context forward. When we do this, we
  adjust their heads with updated information. When we're done, we update
  all the remaining heads.

  FIXME: Much of this code could be shared with the implementation in _generateRunsToCharacter:
  */
  //printf("create runs for %i+%i\n", range.location, range.length);
  { /* OPT: this is creating more runs than it needs to */
    NSDictionary *attributes;
    glyph_run_t *new;
    unsigned int max = range.location + range.length;
    int i;

    ch = range.location;
    while (ch < max)
      {
        attributes = [_textStorage attributesAtIndex: ch
                                   longestEffectiveRange: &rng
                                   inRange: NSMakeRange(0, [_textStorage length])];

        /* printf("at %i, max=%i, effective range (%i+%i)\n",
           ch, max, rng.location, rng.length);*/

        /*
          Catch a common case. If the new run would be a continuation of the
          previous run, and the previous run is short, we resize the previous
          run instead of creating a new run.
          
          (Note that we must make sure that we don't merge with the dummy runs
          at the very front.)
          
          This happens a lot with repeated single-character insertions, aka.
          typing in a text view.
        */
        if (rng.location < ch && context[0]->char_length &&
            // FIXME: Why 16 and not MAX_RUN_LENGTH
            context[0]->char_length < 16)
          {
            rng.length -= ch - rng.location;
            rng.location = ch;
            if (ch + rng.length > max)
              {
                rng.length = max - ch;
              }
            new = (glyph_run_t *)context[0];
            // FIXME: We could try to reuse the glyphs
            run_free_glyphs(new);
            new->head.char_length += rng.length;
            ch = NSMaxRange(rng);
            continue;
          }
        
        new = run_insert(context, random_level());
        [self _run_cache_attributes: new : attributes];
        
        /*
          We have the longest range the attributes allow us to create a run
          for. Since this might overlap the previous and next runs, we might
          need to adjust the location and length of the range we create a
          run for.

          OPT: If the overlapped run is short, we might want to clear out
          its glyphs and extend it to cover our range. This should result
          in fewer runs being created for large sequences of single character
          adds.
        */
        if (rng.location < ch)
          {
            /*
              The new run has the same attributes as the previous run, so we
              mark it is as a continued run.
            */
            new->continued = 1;
            rng.length -= ch - rng.location;
            rng.location = ch;
          }
        if (ch + rng.length > max)
          {
            /*
              The new run has the same attributes as the next run, so we mark
              the next run as a continued run.
            */
            if (new->head.next)
              ((glyph_run_t *)new->head.next)->continued = 1;
            rng.length = max - ch;
          }
      
        /* See comment in -_generateRunsToCharacter:. */
        if (rng.length > MAX_RUN_LENGTH)
          {
            unsigned int safe_break = rng.location + MAX_RUN_LENGTH;
            safe_break = [self _findSafeBreakMovingForwardFrom: safe_break];
            if (safe_break < NSMaxRange(rng))
              rng.length = safe_break - rng.location;
          }
        
        // printf("adjusted length: %i\n", rng.length);
        
        h = &new->head;
        h->char_length = rng.length;
        for (i = 0; i <= new->level; i++, h--)
          {
            if (i)
              {
                // FIXME: Simpler adjustment
                run_fix_head(context[i]);
                run_fix_head(h);
              }
            //h->char_length = rng.length;
            context[i] = h;
          }

        for (; i < SKIP_LIST_DEPTH; i++)
          {
            context[i]->char_length += rng.length;
            context[i]->complete = 0;
          }

        ch += rng.length;
      }
  }
 
  // Final fix up of context
  {
    int i;

    for (i = 1; i < SKIP_LIST_DEPTH; i++)
      {
        run_fix_head(context[i]);     
      }
  }

  if (actualRange)
    *actualRange = range;

  [self _sanityChecks];
  //[self _glyphDumpRuns];
}


#define GET_GLYPH \
	glyph_run_t *r; \
	unsigned int pos, cpos; \
\
	if (glyphs->glyph_length <= idx) \
	{ \
		[self _generateGlyphsUpToGlyph: idx]; \
		if (glyphs->glyph_length <= idx) \
		{ \
			[NSException raise: NSRangeException \
				format: @"%s glyph range out of range", __PRETTY_FUNCTION__]; \
		} \
	} \
 \
	r = run_for_glyph_index(idx, glyphs, &pos, &cpos); \
	if (!r) \
	{ \
		[NSException raise: NSRangeException \
			format: @"%s glyph range out of range", __PRETTY_FUNCTION__]; \
	} \
	idx -= pos;

- (void) setDrawsOutsideLineFragment: (BOOL)flag 
                     forGlyphAtIndex: (unsigned int)idx
{
  GET_GLYPH
  r->glyphs[idx].drawsOutsideLineFragment = !!flag;
}
- (BOOL) drawsOutsideLineFragmentForGlyphAtIndex: (unsigned int)idx
{
  GET_GLYPH
  return r->glyphs[idx].drawsOutsideLineFragment;
}

- (void) setNotShownAttribute: (BOOL)flag 
              forGlyphAtIndex: (unsigned int)idx
{
  GET_GLYPH
  r->glyphs[idx].isNotShown = !!flag;
}
- (BOOL) notShownAttributeForGlyphAtIndex: (unsigned int)idx
{
  GET_GLYPH
  return r->glyphs[idx].isNotShown;
}

// GNUstep extension
- (NSFont *) effectiveFontForGlyphAtIndex: (unsigned int)idx
                                    range: (NSRange *)range
{
  GET_GLYPH
  if (range)
    *range = NSMakeRange(pos, r->head.glyph_length);
  return r->font;
}

/**
 * GNUstep extension
 */
- (NSSize) advancementForGlyphAtIndex: (unsigned int)idx
{
  GET_GLYPH
  return r->glyphs[idx].advancement;
}

- (void) insertGlyph: (NSGlyph)aGlyph
        atGlyphIndex: (unsigned int)glyphIndex
      characterIndex: (unsigned int)charIndex
{
  [self insertGlyphs: &aGlyph
        length: 1
        forStartingGlyphAtIndex: glyphIndex
        characterIndex: charIndex];
}

- (void) replaceGlyphAtIndex: (unsigned int)glyphIndex
                   withGlyph: (NSGlyph)newGlyph
{
  glyph_run_t *r;
  unsigned int pos, cpos;

  if (glyphs->glyph_length <= glyphIndex)
    {
      [NSException raise: NSRangeException
		  format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  r = run_for_glyph_index(glyphIndex, glyphs, &pos, &cpos);
  if (!r)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  if (!r->glyphs || r->head.glyph_length < glyphIndex - pos)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  r->glyphs[glyphIndex - pos].g = newGlyph;
}

- (void) deleteGlyphsInRange: (NSRange)aRange
{
  /* See invalidateGlyphsForCharacterRange:changeInLength:actualCharacterRange:
  glyph_run_t *run;
  unsigned int pos, cpos;
  unsigned int glyphIndex;
  unsigned int lastGlyphIndex;
  glyph_run_head_t *context[SKIP_LIST_DEPTH];

  glyphIndex = NSMinRange(aRange);
  lastGlyphIndex = NSMaxRange(aRange) - 1;
  while (glyphIndex <= lastGlyphIndex)
    {
      run = run_for_glyph_index(glyphIndex, glyphs, &pos, &cpos);
      if (!run)
	{
	  [NSException raise: NSRangeException
		      format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
	  return;
	}

        // FIXME: remove all invalid glyphs from run
        if ((pos == 0) && (lastGlyphIndex >= glyphIndex - pos + run->head.glyph_length))
          {
            run_free_glyphs(run);
          }
        else
          {
            if (lastGlyphIndex >= glyphIndex - pos + run->head.glyph_length)
              {
              }
            r->head.glyph_length = len; 
          }
        // FIXME: Need to invalidate the entries above this one.

        // FIXME Cache this value
        glyphIndex += r->head.glyph_length - pos;
    }
  */
  NSLog(@"Internal method %s called", __PRETTY_FUNCTION__);
}

- (void) setCharacterIndex: (unsigned int)charIndex
           forGlyphAtIndex: (unsigned int)glyphIndex
{
  glyph_run_t *r;
  unsigned int pos, cpos;

  if (glyphs->glyph_length <= glyphIndex)
    {
      [NSException raise: NSRangeException
		  format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  r = run_for_glyph_index(glyphIndex, glyphs, &pos, &cpos);
  if (!r)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  if (!r->glyphs || r->head.glyph_length < glyphIndex - pos)
    {
      [NSException raise: NSRangeException
		   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  r->glyphs[glyphIndex - pos].char_offset = charIndex - cpos;
  // What should happen to the following glyphs?
}

- (int) intAttribute: (int)attributeTag
     forGlyphAtIndex: (unsigned int)glyphIndex
{
  glyph_run_t *run;
  glyph_t *g;
  unsigned int pos;

  run = run_for_glyph_index(glyphIndex, glyphs, &pos, NULL);
  if (run && run->glyphs && (run->head.glyph_length < glyphIndex - pos))
    {
      g = &run->glyphs[glyphIndex - pos];

      if (attributeTag == NSGlyphAttributeInscribe)
        return g->inscription;
      else if (attributeTag == NSGlyphAttributeSoft)
        return g->soft;
      else if (attributeTag == NSGlyphAttributeElastic)
        return g->elasitc;
      else if (attributeTag == NSGlyphAttributeBidiLevel)
        return g->bidilevel;
    }
 
  return 0;
}

@end



/***** Layout handling *****/

@implementation GSLayoutManager (LayoutHelpers)

-(void) _invalidateLayoutFromContainer: (int)idx
{
  int i, j;
  textcontainer_t *tc;
  linefrag_t *lf;

  extra_textcontainer = nil;

  for (i = idx, tc = textcontainers + idx; i < num_textcontainers; i++, tc++)
    {
      tc->complete = NO;
      if (tc->linefrags)
	{
	  for (j = 0, lf = tc->linefrags; j < tc->num_linefrags + tc->num_soft; j++, lf++)
	    {
	      if (lf->points)
		free(lf->points);
	      if (lf->attachments)
		free(lf->attachments);
	    }

	  free(tc->linefrags);
	}
      tc->linefrags = NULL;
      tc->num_linefrags = tc->num_soft = 0;
      tc->size_linefrags = 0;
      tc->pos = tc->length = 0;
      tc->was_invalidated = YES;
    }
  for (i = idx - 1, tc = textcontainers + idx - 1; i >= 0; i--, tc--)
    {
      if (tc->num_linefrags)
	{
	  layout_glyph = tc->pos + tc->length;
	  if (layout_glyph == glyphs->glyph_length)
	    layout_char = glyphs->char_length;
	  else
	    layout_char = [self characterIndexForGlyphAtIndex: layout_glyph]; /* TODO? */
	  return;
	}
    }
  layout_glyph = layout_char = 0;
}

-(void) _freeLayout
{
  [self _invalidateLayoutFromContainer: 0];
}

-(void) _invalidateEverything
{
  [self _freeLayout];
  [self _freeGlyphs];
  [self _initGlyphs];
}

-(void) _doLayout
{
  [self _doLayoutToContainer: num_textcontainers - 1];
}

-(void) _doLayoutToGlyph: (unsigned int)glyphIndex
{
  int i, j;
  textcontainer_t *tc;
  unsigned int next;
  NSRect prev;
  BOOL delegate_responds;

  delegate_responds = [_delegate respondsToSelector:
    @selector(layoutManager:didCompleteLayoutForTextContainer:atEnd:)];

  next = layout_glyph;
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      if (tc->complete)
          continue;

      while (1)
        {
          if (tc->num_linefrags)
            prev = tc->linefrags[tc->num_linefrags - 1].rect;
          else
            prev = NSZeroRect;
          j = [typesetter layoutGlyphsInLayoutManager: self
                          inTextContainer: tc->textContainer
                          startingAtGlyphIndex: next
                          previousLineFragmentRect: prev
                          nextGlyphIndex: &next
                          numberOfLineFragments: 0];
          if (j)
            break;

          if (next > glyphIndex)
            {
              // If all the requested work is done just leave
              return;
            }
        }
      tc->complete = YES;
      tc->usedRectValid = NO;
      if (tc->num_soft)
        {
          /*
            If there is any soft invalidated layout information left, remove
            it.
          */
          int k;
          linefrag_t *lf;
          for (k = tc->num_linefrags, lf = tc->linefrags + k; 
               k < tc->num_linefrags + tc->num_soft; k++, lf++)
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
        }
      if (delegate_responds)
        {
          [_delegate layoutManager: self
                     didCompleteLayoutForTextContainer: tc->textContainer
                     atEnd: j == 2];
          /* The call might have resulted in more text containers being
             added, so 'textcontainers' might have moved. */
          tc = textcontainers + i;
        }
      if (j == 2)
        {
          break;
        }
      if (i == num_textcontainers && delegate_responds)
        {
          [_delegate layoutManager: self
                     didCompleteLayoutForTextContainer: nil
                     atEnd: NO];
        }
    }
}

-(void) _doLayoutToContainer: (int)cindex
{
  int i, j;
  textcontainer_t *tc;
  unsigned int next;
  NSRect prev;
  BOOL delegate_responds;

  delegate_responds = [_delegate respondsToSelector:
    @selector(layoutManager:didCompleteLayoutForTextContainer:atEnd:)];

  next = layout_glyph;
  for (i = 0, tc = textcontainers; i <= cindex; i++, tc++)
    {
      if (tc->complete)
          continue;

      while (1)
        {
          if (tc->num_linefrags)
            prev = tc->linefrags[tc->num_linefrags - 1].rect;
          else
            prev = NSZeroRect;
          j = [typesetter layoutGlyphsInLayoutManager: self
                          inTextContainer: tc->textContainer
                          startingAtGlyphIndex: next
                          previousLineFragmentRect: prev
                          nextGlyphIndex: &next
                          numberOfLineFragments: 0];
          if (j)
            break;
        }
      tc->complete = YES;
      tc->usedRectValid = NO;
      if (tc->num_soft)
        {
          /*
            If there is any soft invalidated layout information left, remove
            it.
          */
          int k;
          linefrag_t *lf;
          for (k = tc->num_linefrags, lf = tc->linefrags + k; 
               k < tc->num_linefrags + tc->num_soft; k++, lf++)
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
        }
      if (delegate_responds)
        {
          [_delegate layoutManager: self
                     didCompleteLayoutForTextContainer: tc->textContainer
                     atEnd: j == 2];
          /* The call might have resulted in more text containers being
             added, so 'textcontainers' might have moved. */
          tc = textcontainers + i;
        }
      if (j == 2)
        {
          break;
        }
      if (i == num_textcontainers && delegate_responds)
        {
          [_delegate layoutManager: self
                     didCompleteLayoutForTextContainer: nil
                     atEnd: NO];
        }
    }
}

-(void) _didInvalidateLayout
{
  int i;
  textcontainer_t *tc;

  for (tc = textcontainers, i = 0; i < num_textcontainers; i++, tc++)
    {
      // FIXME: This value never gets used
      tc->was_invalidated = YES;
    }
}

@end


@implementation GSLayoutManager (layout)


/*
In the general case, we can't make any assumptions about how layout might
interact between line frag rects. To be safe in all cases, we must
invalidate all layout information.

TODO:
We could handle this by assuming that whoever calls this knows exactly what
needs to be invalidated. We won't be using it internally, anyway, so it
doesn't matter much to us, and it would make more advanced things possible
for external callers. On the other hand, it would be easy to break things
by calling this incorrectly.
*/
- (void) invalidateLayoutForCharacterRange: (NSRange)aRange 
				    isSoft: (BOOL)flag
		      actualCharacterRange: (NSRange *)actualRange
{
  [self _invalidateLayoutFromContainer: 0];
}


#define SETUP_STUFF \
  unsigned int max = glyphRange.location + glyphRange.length; \
  \
  [self _generateGlyphsUpToGlyph: max - 1]; \
  if (glyphs->glyph_length < max) \
    { \
      [NSException raise: NSRangeException \
		  format: @"%s: glyph range out of range", __PRETTY_FUNCTION__]; \
      return; \
    }

- (void) setTextContainer: (NSTextContainer *)aTextContainer 
	forGlyphRange: (NSRange)glyphRange
{
  textcontainer_t *tc;
  int i;
  SETUP_STUFF

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == aTextContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: doesn't own text container", __PRETTY_FUNCTION__);
      return;
    }

  /* Assume that no line frags means that layout hasn't started yet. */
  if (tc->num_linefrags)
    {
      if (glyphRange.location != tc->pos + tc->length)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
      tc->length += glyphRange.length;
    }
  else if (!i)
    {
      if (glyphRange.location)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
      tc->pos = 0;
      tc->length = glyphRange.length;
    }
  else
    {
      if (tc[-1].pos + tc[-1].length != glyphRange.location)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
      tc->pos = glyphRange.location;
      tc->length = glyphRange.length;
    }

  {
    unsigned int gpos;
    unsigned int g;
    glyph_t *glyph;
    glyph_run_t *run = run_for_glyph_index(glyphRange.location, glyphs, &gpos, NULL);

    g = glyphRange.location;
    glyph = &run->glyphs[g - gpos];
    while (g < glyphRange.location + glyphRange.length)
      {
	if (g == gpos + run->head.glyph_length)
	  {
	    gpos += run->head.glyph_length;
	    run = (glyph_run_t *)run->head.next;
	    glyph = run->glyphs;
	  }

	glyph->isNotShown = NO;
	glyph->drawsOutsideLineFragment = NO;
	g++;
	glyph++;
      }
  }

  layout_glyph = tc->pos + tc->length;
  if (layout_glyph == glyphs->glyph_length)
    layout_char = glyphs->char_length;
  else
    layout_char = [self characterIndexForGlyphAtIndex: layout_glyph];
}

- (void) setLineFragmentRect: (NSRect)fragmentRect 
	       forGlyphRange: (NSRange)glyphRange
		    usedRect: (NSRect)usedRect
{
  textcontainer_t *tc;
  int i;
  linefrag_t *lf;

  SETUP_STUFF

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      if (tc->pos <= glyphRange.location &&
	  tc->pos + tc->length >= glyphRange.location + glyphRange.length)
	break;
    }
  if (i == num_textcontainers)
    {
      [NSException raise: NSRangeException
		  format: @"%s: glyph range not consistent with existing layout",
			  __PRETTY_FUNCTION__];
      return;
    }

  /* Make sure the given glyph range matches earlier layout. */
  if (!tc->num_linefrags)
    {
      if (glyphRange.location != tc->pos)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
    }
  else
    {
      lf = &tc->linefrags[tc->num_linefrags - 1];
      if (lf->pos + lf->length != glyphRange.location)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
    }

  if (!(tc->num_linefrags + tc->num_soft))
    {
      if (!tc->size_linefrags)
	{
	  tc->size_linefrags = 16;
	  tc->linefrags = malloc(sizeof(linefrag_t) * tc->size_linefrags);
	}
      tc->num_linefrags = 1;
      lf = tc->linefrags;
    }
  else if (!tc->num_soft)
    {
      if (tc->size_linefrags <= tc->num_linefrags)
	{
	  tc->size_linefrags += tc->size_linefrags / 2;
	  tc->linefrags = realloc(tc->linefrags, sizeof(linefrag_t) * tc->size_linefrags);
	}
      tc->num_linefrags++;
      lf = &tc->linefrags[tc->num_linefrags - 1];
    }
  else
    {
      int i;
      for (i = tc->num_linefrags, lf = tc->linefrags + i; i < tc->num_linefrags + tc->num_soft; i++, lf++)
	{
	  if (lf->pos >= NSMaxRange(glyphRange))
	    break;
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

      if (i == tc->num_linefrags)
	{
	  /*
	  If we should keep all soft frags, we need to enlarge the array
	  to fit the new line frag.
	  */
	  if (tc->size_linefrags <= tc->num_linefrags + tc->num_soft)
	    {
	      tc->size_linefrags += tc->size_linefrags / 2;
	      tc->linefrags = realloc(tc->linefrags, sizeof(linefrag_t) * tc->size_linefrags);
	    }
	  memmove(&tc->linefrags[tc->num_linefrags + 1], &tc->linefrags[tc->num_linefrags], tc->num_soft * sizeof(linefrag_t));
	}
      else if (i > tc->num_linefrags + 1)
	{
	  tc->num_soft -= i - tc->num_linefrags;
	  memmove(&tc->linefrags[tc->num_linefrags + 1], &tc->linefrags[i], tc->num_soft * sizeof(linefrag_t));
	}
      else
	{
	  /*
	  If i == tc->num_linefrags + 1, we're lucky and everything already
	  lines up, so no moving is necessary.
	  */
	  tc->num_soft--;
	}

      tc->num_linefrags++;
      lf = &tc->linefrags[tc->num_linefrags - 1];
    }

  memset(lf, 0, sizeof(linefrag_t));
  lf->rect = fragmentRect;
  lf->used_rect = usedRect;
  lf->pos = glyphRange.location;
  lf->length = glyphRange.length;
}

- (void) setLocation: (NSPoint)location 
forStartOfGlyphRange: (NSRange)glyphRange
{
  textcontainer_t *tc;
  int i;
  linefrag_t *lf;
  linefrag_point_t *lp;

  SETUP_STUFF

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      if (tc->pos <= glyphRange.location &&
	  tc->pos + tc->length >= glyphRange.location + glyphRange.length)
	break;
    }
  if (i == num_textcontainers)
    {
      [NSException raise: NSRangeException
		  format: @"%s: glyph range not consistent with existing layout",
			  __PRETTY_FUNCTION__];
      return;
    }

  for (i = tc->num_linefrags - 1, lf = tc->linefrags + i; i >= 0; i--, lf--)
    {
      if (lf->pos <= glyphRange.location &&
	  lf->pos + lf->length >= glyphRange.location + glyphRange.length)
	break;
    }
  if (i < 0)
    {
      [NSException raise: NSRangeException
		  format: @"%s: glyph range not consistent with existing layout",
			  __PRETTY_FUNCTION__];
      return;
    }

  if (!lf->num_points)
    {
      if (glyphRange.location != lf->pos)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
      lp = lf->points = malloc(sizeof(linefrag_point_t));
      lf->num_points++;
    }
  else
    {
      lp = &lf->points[lf->num_points - 1];
      if (lp->pos + lp->length != glyphRange.location)
	{
	  [NSException raise: NSRangeException
		      format: @"%s: glyph range not consistent with existing layout",
			      __PRETTY_FUNCTION__];
	  return;
	}
      lf->num_points++;
      lf->points = realloc(lf->points, sizeof(linefrag_point_t) * lf->num_points);
      lp = &lf->points[lf->num_points - 1];
    }
  lp->pos = glyphRange.location;
  lp->length = glyphRange.length;
  lp->p = location;
}


-(void) setAttachmentSize: (NSSize)size
	    forGlyphRange: (NSRange)glyphRange
{
  textcontainer_t *tc;
  int i;
  linefrag_t *lf;
  linefrag_attachment_t *la;

  SETUP_STUFF

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      if (tc->pos <= glyphRange.location &&
	  tc->pos + tc->length >= glyphRange.location + glyphRange.length)
	break;
    }
  if (i == num_textcontainers)
    {
      [NSException raise: NSRangeException
		  format: @"%s: glyph range not consistent with existing layout",
			  __PRETTY_FUNCTION__];
      return;
    }

  for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
    {
      if (lf->pos <= glyphRange.location &&
	  lf->pos + lf->length >= glyphRange.location + glyphRange.length)
	break;
    }
  if (i == tc->num_linefrags)
    {
      [NSException raise: NSRangeException
		  format: @"%s: glyph range not consistent with existing layout",
			  __PRETTY_FUNCTION__];
      return;
    }

  /* TODO: we do no sanity checking of attachment size ranges. might want
  to consider doing it */
  lf->attachments = realloc(lf->attachments,
		      sizeof(linefrag_attachment_t) * (lf->num_attachments + 1));
  la = &lf->attachments[lf->num_attachments++];

  memset(la, 0, sizeof(linefrag_attachment_t));
  la->pos = glyphRange.location;
  la->length = glyphRange.length;
  la->size = size;
}

#undef SETUP_STUFF

- (NSTextContainer *) textContainerForGlyphAtIndex: (NSUInteger)glyphIndex
                                    effectiveRange: (NSRange *)effectiveRange
{
  return [self textContainerForGlyphAtIndex: glyphIndex
               effectiveRange: effectiveRange
               withoutAdditionalLayout: NO];
}

- (NSTextContainer *) textContainerForGlyphAtIndex: (NSUInteger)glyphIndex
                                    effectiveRange: (NSRange *)effectiveRange
                           withoutAdditionalLayout: (BOOL)flag
{
  textcontainer_t *tc;
  int i;

  if (!flag)
    [self _doLayoutToGlyph: glyphIndex];

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyphIndex)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return nil;
    }

  if (effectiveRange)
    {
      [self _doLayoutToContainer: i];
      tc = textcontainers + i;
      *effectiveRange = NSMakeRange(tc->pos, tc->length);
    }
  return tc->textContainer;
}

- (NSRect) lineFragmentRectForGlyphAtIndex: (NSUInteger)glyphIndex
			    effectiveRange: (NSRange *)effectiveGlyphRange
{
  return [self lineFragmentRectForGlyphAtIndex: glyphIndex
               effectiveRange: effectiveGlyphRange
               withoutAdditionalLayout: NO];
}

- (NSRect) lineFragmentRectForGlyphAtIndex: (NSUInteger)glyphIndex
                            effectiveRange: (NSRange *)effectiveGlyphRange
                   withoutAdditionalLayout: (BOOL)flag
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;

  if (!flag)
    [self _doLayoutToGlyph: glyphIndex];

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyphIndex)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSZeroRect;
    }

  for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
    if (lf->pos + lf->length > glyphIndex)
      break;
  if (i == tc->num_linefrags)
    {
      NSLog(@"%s: can't find line frag rect for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSZeroRect;
    }

  if (effectiveGlyphRange)
    {
      *effectiveGlyphRange = NSMakeRange(lf->pos, lf->length);
    }
  return lf->rect;
}

- (NSRect) lineFragmentUsedRectForGlyphAtIndex: (NSUInteger)glyphIndex
				effectiveRange: (NSRange *)effectiveGlyphRange
{
  return [self lineFragmentUsedRectForGlyphAtIndex: glyphIndex
               effectiveRange: effectiveGlyphRange
               withoutAdditionalLayout: NO];
}

- (NSRect) lineFragmentUsedRectForGlyphAtIndex: (NSUInteger)glyphIndex
                                effectiveRange: (NSRange *)effectiveGlyphRange
                       withoutAdditionalLayout: (BOOL)flag
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;

  if (!flag)
    [self _doLayoutToGlyph: glyphIndex];

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyphIndex)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeRect(0, 0, 0, 0);
    }

  for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
    if (lf->pos + lf->length > glyphIndex)
      break;
  if (i == tc->num_linefrags)
    {
      NSLog(@"%s: can't find line frag rect for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeRect(0, 0, 0, 0);
    }

  if (effectiveGlyphRange)
    {
      *effectiveGlyphRange = NSMakeRange(lf->pos, lf->length);
    }
  return lf->used_rect;
}

- (NSRange) rangeOfNominallySpacedGlyphsContainingIndex: (unsigned int)glyphIndex
					  startLocation: (NSPoint *)p
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  linefrag_point_t *lp;

  [self _doLayoutToGlyph: glyphIndex];
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->pos + tc->length > glyphIndex)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: can't find text container for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeRange(NSNotFound, 0);
    }

  for (i = 0, lf = tc->linefrags; i < tc->num_linefrags; i++, lf++)
    if (lf->pos + lf->length > glyphIndex)
      break;
  if (i == tc->num_linefrags)
    {
      NSLog(@"%s: can't find line frag rect for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeRange(NSNotFound, 0);
    }

  for (i = 0, lp = lf->points; i < lf->num_points; i++, lp++)
    if (lp->pos + lp->length > glyphIndex)
      break;
  if (i == lf->num_points)
    {
      NSLog(@"%s: can't find location for glyph (internal error)", __PRETTY_FUNCTION__);
      return NSMakeRange(NSNotFound, 0);
    }

  if (p)
    *p = lp->p;
  return NSMakeRange(lp->pos, lp->length);
}


- (NSRange) rangeOfNominallySpacedGlyphsContainingIndex:(unsigned int)glyphIndex
{
  return [self rangeOfNominallySpacedGlyphsContainingIndex: glyphIndex
	       startLocation: NULL];
}


/* The union of all line frag rects' used rects. */
- (NSRect) usedRectForTextContainer: (NSTextContainer *)container
{
  textcontainer_t *tc;
  linefrag_t *lf;
  int i;
  NSRect used;

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: doesn't own text container", __PRETTY_FUNCTION__);
      return NSMakeRect(0, 0, 0, 0);
    }
  if (!tc->complete)
    {
      [self _doLayoutToContainer: i];
      tc = textcontainers + i;
    }

  if (tc->usedRectValid)
    {
      used = tc->usedRect;
      if (tc->textContainer == extra_textcontainer)
        {
          used = NSUnionRect(used, extra_used_rect);
        }
      return used;
    }

  if (tc->num_linefrags)
    {
      double x0, y0, x1, y1;
      i = 0;
      lf = tc->linefrags;
      x0 = NSMinX(lf->used_rect);
      y0 = NSMinY(lf->used_rect);
      x1 = NSMaxX(lf->used_rect);
      y1 = NSMaxY(lf->used_rect);
      for (i++, lf++; i < tc->num_linefrags; i++, lf++)
	{
	  if (NSMinX(lf->used_rect) < x0)
	    x0 = NSMinX(lf->used_rect);
	  if (NSMinY(lf->used_rect) < y0)
	    y0 = NSMinY(lf->used_rect);
	  if (NSMaxX(lf->used_rect) > x1)
	    x1 = NSMaxX(lf->used_rect);
	  if (NSMaxY(lf->used_rect) > y1)
	    y1 = NSMaxY(lf->used_rect);
	}
      used = NSMakeRect(x0, y0, x1 - x0, y1 - y0);
    }
  else
    {
      used = NSZeroRect;
    }  
  tc->usedRect = used;
  tc->usedRectValid = YES;
  if (tc->textContainer == extra_textcontainer)
    {
      used = NSUnionRect(used, extra_used_rect);
    }
  return used;
}

- (NSRange) glyphRangeForTextContainer: (NSTextContainer *)container
{
  textcontainer_t *tc;
  int i;

  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == container)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: doesn't own text container", __PRETTY_FUNCTION__);
      return NSMakeRange(NSNotFound, 0);
    }

  [self _doLayoutToContainer: i];
  tc = textcontainers + i;
  return NSMakeRange(tc->pos, tc->length);
}


/* TODO: make more efficient */
- (NSArray *) textContainers
{
  NSMutableArray *ma;
  int i;

  ma = [[NSMutableArray alloc] initWithCapacity: num_textcontainers];
  for (i = 0; i < num_textcontainers; i++)
    [ma addObject: textcontainers[i].textContainer];
  return [ma autorelease];
}

- (void) addTextContainer: (NSTextContainer *)container
{
  [self insertTextContainer: container
	atIndex: num_textcontainers];
}

- (void) insertTextContainer: (NSTextContainer *)aTextContainer
		     atIndex: (unsigned int)index
{
  unsigned int i;

  if (index < num_textcontainers)
    [self _invalidateLayoutFromContainer: index];

  num_textcontainers++;
  textcontainers = realloc(textcontainers,
			 sizeof(textcontainer_t) * num_textcontainers);

  for (i = num_textcontainers - 1; i > index; i--)
    textcontainers[i] = textcontainers[i - 1];

  memset(&textcontainers[i], 0, sizeof(textcontainer_t));
  textcontainers[i].textContainer = [aTextContainer retain];

  [aTextContainer setLayoutManager: self];

  [self _didInvalidateLayout];
}

- (void) removeTextContainerAtIndex: (unsigned int)index
{
  int i;
  textcontainer_t *tc = &textcontainers[index];

  [self _invalidateLayoutFromContainer: index];
  [tc->textContainer setLayoutManager: nil];
  [tc->textContainer release];

  num_textcontainers--;
  for (i = index; i < num_textcontainers; i++)
    textcontainers[i] = textcontainers[i + 1];

  if (num_textcontainers)
    textcontainers = realloc(textcontainers,
			   sizeof(textcontainer_t) * num_textcontainers);
  else
    {
      free(textcontainers);
      textcontainers = NULL;
    }

  [self _didInvalidateLayout];
}

- (void) textContainerChangedGeometry: (NSTextContainer *)aContainer
{
  int i;
  for (i = 0; i < num_textcontainers; i++)
    if (textcontainers[i].textContainer == aContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"%s: does not own text container", __PRETTY_FUNCTION__);
      return;
    }
  [self _invalidateLayoutFromContainer: i];
  [self _didInvalidateLayout];
}

- (unsigned int) firstUnlaidCharacterIndex
{
  return layout_char;
}

- (unsigned int) firstUnlaidGlyphIndex
{
  return layout_glyph;
}

-(void) getFirstUnlaidCharacterIndex: (unsigned int *)cindex
			  glyphIndex: (unsigned int *)gindex
{
  if (cindex)
    *cindex = [self firstUnlaidCharacterIndex];
  if (gindex)
    *gindex = [self firstUnlaidGlyphIndex];
}

-(void) setExtraLineFragmentRect: (NSRect)linefrag
			usedRect: (NSRect)used
		   textContainer: (NSTextContainer *)tc
{
  extra_rect = linefrag;
  extra_used_rect = used;
  extra_textcontainer = tc;
}

-(NSRect) extraLineFragmentRect
{
  return extra_rect;
}

-(NSRect) extraLineFragmentUsedRect
{
  return extra_used_rect;
}

-(NSTextContainer *) extraLineFragmentTextContainer
{
  return extra_textcontainer;
}


-(void) _softInvalidateUseLineFrags: (int)num
			  withShift: (NSSize)shift
		    inTextContainer: (NSTextContainer *)textContainer
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == textContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"(%s): does not own text container", __PRETTY_FUNCTION__);
      return;
    }

  if (shift.width || shift.height)
    {
      for (i = 0, lf = &tc->linefrags[tc->num_linefrags]; i < num; i++, lf++)
	{
	  lf->rect.origin.x += shift.width;
	  lf->rect.origin.y += shift.height;
	  lf->used_rect.origin.x += shift.width;
	  lf->used_rect.origin.y += shift.height;
	}
    }
  tc->num_soft -= num;
  tc->num_linefrags += num;
  lf = &tc->linefrags[tc->num_linefrags - 1];
  tc->length = lf->pos + lf->length - tc->pos;

  layout_glyph = tc->pos + tc->length;
  /*
  We must have glyphs beyond all the soft-invalidated line frags,
  so comparing with glyphs->glyph_length is ok.
  */
  if (layout_glyph == glyphs->glyph_length)
    layout_char = glyphs->char_length;
  else
    layout_char = [self characterIndexForGlyphAtIndex: layout_glyph]; /* TODO? */
}

-(NSRect) _softInvalidateLineFragRect: (int)index
			   firstGlyph: (unsigned int *)first_glyph
			    nextGlyph: (unsigned int *)next_glyph
		      inTextContainer: (NSTextContainer *)textContainer
{
  int i;
  textcontainer_t *tc;
  linefrag_t *lf;
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == textContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"(%s): does not own text container", __PRETTY_FUNCTION__);
      return NSZeroRect;
    }

  if (index >= tc->num_soft)
    return NSZeroRect;

  lf = &tc->linefrags[tc->num_linefrags + index];
  *first_glyph = lf->pos;
  *next_glyph = lf->pos + lf->length;
  return lf->rect;
}

-(unsigned int) _softInvalidateFirstGlyphInTextContainer: (NSTextContainer *)textContainer
{
  int i;
  textcontainer_t *tc;
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == textContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"(%s): does not own text container", __PRETTY_FUNCTION__);
      return (unsigned int)-1;
    }
  if (tc->num_soft)
    return tc->linefrags[tc->num_linefrags].pos;
  else
    return (unsigned int)-1;
}

-(unsigned int) _softInvalidateNumberOfLineFragsInTextContainer: (NSTextContainer *)textContainer
{
  int i;
  textcontainer_t *tc;
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    if (tc->textContainer == textContainer)
      break;
  if (i == num_textcontainers)
    {
      NSLog(@"(%s): does not own text container", __PRETTY_FUNCTION__);
      return (unsigned int)-1;
    }
  return tc->num_soft;
}

@end


/***** The rest *****/

@implementation GSLayoutManager

- init
{
  if (!(self = [super init]))
    return nil;

  [self setTypesetter: [GSTypesetter sharedSystemTypesetter]];
  [self setGlyphGenerator: [NSGlyphGenerator sharedGlyphGenerator]];

  usesScreenFonts = YES;
  [self _initGlyphs];

  return self;
}

-(void) dealloc
{
  int i;
  textcontainer_t *tc;

  free(rect_array);
  rect_array_size = 0;
  rect_array = NULL;

  [self _freeLayout];
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      [tc->textContainer setLayoutManager: nil];
      [tc->textContainer release];
    }
  free(textcontainers);
  textcontainers = NULL;

  [self _freeGlyphs];

  DESTROY(typesetter);
  DESTROY(_glyphGenerator);

  [super dealloc];
}

/**
 * Sets the text storage for the layout manager.
 * Use -replaceTextStorage: instead as a rule. - this method is really
 * more for internal use by the text system.
 * Invalidates the entire layout (should it??)
 */
/*
See [NSTextView -setTextContainer:] for more information about these calls.
*/
- (void) setTextStorage: (NSTextStorage *)aTextStorage
{
  int i;
  textcontainer_t *tc;

  [self _invalidateEverything];

  /*
   * Make a note of the new text storage object, but don't retain it.
   * The text storage is owning us - it retains us.
   */
  _textStorage = aTextStorage;

  /*
  We send this message to all text containers so they can respond to the
  change (most importantly to let them tell their text views).
  */
  for (i = 0, tc = textcontainers; i < num_textcontainers; i++, tc++)
    {
      [tc->textContainer setLayoutManager: self];
    }
  [self _didInvalidateLayout];
}

/**
 * Returns the text storage for this layout manager.
 */
- (NSTextStorage *) textStorage
{
  return _textStorage;
}

/**
 * Replaces the text storage with a new one.<br />
 * Takes care (since layout managers are owned by text storages)
 * not to get self deallocated.
 */
- (void) replaceTextStorage: (NSTextStorage *)newTextStorage
{
  NSArray		*layoutManagers = [_textStorage layoutManagers];
  NSEnumerator		*enumerator = [layoutManagers objectEnumerator];
  GSLayoutManager	*object;

  /* Remove layout managers from old NSTextStorage object and add them to the
     new one.  NSTextStorage's addLayoutManager invokes GSLayoutManager's
     setTextStorage method automatically, and that includes self.  */

  while ((object = (GSLayoutManager*)[enumerator nextObject]) != nil)
    {
      RETAIN(object);
      [_textStorage removeLayoutManager: object];
      [newTextStorage addLayoutManager: object];
      RELEASE(object);
    }
}

- (NSGlyphGenerator *) glyphGenerator
{
  return _glyphGenerator;
}
- (void) setGlyphGenerator: (NSGlyphGenerator *)glyphGenerator
{
  ASSIGN(_glyphGenerator, glyphGenerator);
}

- (id) delegate
{
  return _delegate;
}
- (void) setDelegate: (id)aDelegate
{
  _delegate = aDelegate;
}


-(GSTypesetter *) typesetter
{
  return typesetter;
}
-(void) setTypesetter: (GSTypesetter *)a_typesetter
{
  ASSIGN(typesetter, a_typesetter);
}

- (BOOL) usesScreenFonts
{
  return usesScreenFonts;
}

- (void) setUsesScreenFonts: (BOOL)flag
{
  flag = !!flag;
  if (flag == usesScreenFonts)
    return;
  usesScreenFonts = flag;
  [self _invalidateEverything];
  [self _didInvalidateLayout];
}

- (NSFont *) substituteFontForFont: (NSFont *)originalFont
{
  NSFont *f;
  if (usesScreenFonts)
    {
      f = [originalFont screenFont];
      if (f)
	return f;
    }
  return originalFont;
}


- (void) setBackgroundLayoutEnabled: (BOOL)flag
{
  flag = !!flag;
  if (flag == backgroundLayoutEnabled)
    return;
  backgroundLayoutEnabled = flag;
  /* TODO */
}
- (BOOL) backgroundLayoutEnabled
{
  return backgroundLayoutEnabled;
}

- (void) setShowsInvisibleCharacters: (BOOL)flag
{
  flag = !!flag;
  if (flag == showsInvisibleCharacters)
    return;

  showsInvisibleCharacters = flag;
  [self _invalidateEverything];
  [self _didInvalidateLayout];
}

- (BOOL) showsInvisibleCharacters
{
  return showsInvisibleCharacters;
}

- (void) setShowsControlCharacters: (BOOL)flag
{
  flag = !!flag;
  if (flag == showsControlCharacters)
    return;
  showsControlCharacters = flag;
  [self _invalidateEverything];
  [self _didInvalidateLayout];
}

- (BOOL) showsControlCharacters
{
  return showsControlCharacters;
}

/*
Note that NSLayoutManager completely overrides this (to perform more
intelligent invalidation of layout using the constraints on layout it
has).
*/
- (void) textStorage: (NSTextStorage *)aTextStorage
              edited: (unsigned int)mask
               range: (NSRange)range
      changeInLength: (int)lengthChange
    invalidatedRange: (NSRange)invalidatedRange
{
  NSRange r;

  if (!(mask & NSTextStorageEditedCharacters))
    lengthChange = 0;

  [self invalidateGlyphsForCharacterRange: invalidatedRange
        changeInLength: lengthChange
        actualCharacterRange: &r];

  /*
  See the comments above -invalidateLayoutForCharacterRange:isSoft:
  actualCharacterRange: for information on why we invalidate everything
  here.
  */
  [self _invalidateLayoutFromContainer: 0];
  [self _didInvalidateLayout];
}

-(unsigned int) _findSafeBreakMovingBackwardFrom: (unsigned int)ch
{
  NSString *str = [_textStorage string];

  // FIXME: Better check for ligature
  while (ch > 0 && [str characterAtIndex: ch-1] == 'f')
    ch--;
  return ch;
}

-(unsigned int) _findSafeBreakMovingForwardFrom: (unsigned int)ch
{
  unsigned int len = [_textStorage length];
  NSString *str = [_textStorage string];

  // FIXME: Better check for ligature
  while (ch < len && [str characterAtIndex: ch] == 'f')
    ch++;
  if (ch < len && ch > 0 && [str characterAtIndex: ch-1] == 'f')
    ch++;

  return ch;
}

/*
 * NSGlyphStorage protocol
 */ 
- (NSAttributedString*) attributedString
{
  return _textStorage;
}

/**
 * GNUstep extension
 */
- (void) insertGlyphs: (const NSGlyph*)glyph_list
     withAdvancements: (const NSSize*)advancements
               length: (NSUInteger)length
forStartingGlyphAtIndex: (NSUInteger)glyph
       characterIndex: (NSUInteger)index
{
  glyph_run_t *run;
  int i;
  glyph_t *g;
  int len;
  unsigned int gpos = 0;
  unsigned int cpos = 0;

  //NSLog(@"Insert %d glyphs at %d for index %d", length, glyph, index);

  run = [self run_for_character_index: index : &gpos : &cpos];
  if (!run)
    {
      [NSException raise: NSRangeException
                   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  len = glyph - gpos + length;
  if (len < 0)
    {
      NSLog(@"Insert %d glyphs at %d for index %d", (int)length, (int)glyph, (int)index);
      NSLog(@"Found gpos %d cpos %d len %d", gpos, cpos, len);
      [NSException raise: NSRangeException
                   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }

  if (!run->glyphs)
    {
      run->glyphs = malloc(sizeof(glyph_t) * len);
      memset(run->glyphs, 0, sizeof(glyph_t) * len);
    }
  else if (run->head.glyph_length < len)
    {
      run->glyphs = realloc(run->glyphs, sizeof(glyph_t) * len);
      memset(&run->glyphs[glyph - gpos], 0, sizeof(glyph_t) * length);
    }
  run->head.glyph_length = len;

  // Add the glyphs to the run
  g = run->glyphs + (glyph - gpos);
  for (i = 0; i < length; i++)
    {
      // We expect to get a nominal glyph run
      g->char_offset = i + index - cpos;
      g->g = glyph_list[i];
      g->advancement = advancements[i];
      g++;
    }
}

- (void) insertGlyphs: (const NSGlyph*)glyph_list
               length: (NSUInteger)length
forStartingGlyphAtIndex: (NSUInteger)glyph
       characterIndex: (NSUInteger)index
{
  glyph_run_t *run;
  int i;
  unsigned int gpos, cpos;
  NSSize advances[length];

  run = [self run_for_character_index: index : &gpos : &cpos];
  if (!run)
    {
      [NSException raise: NSRangeException
                   format: @"%s glyph index out of range", __PRETTY_FUNCTION__];
      return;
    }
    
  for (i=0; i<length; i++)
    {
      if ((glyph_list[i] != NSControlGlyph) && (glyph_list[i] != GSAttachmentGlyph))
        {
          advances[i] = [run->font advancementForGlyph: glyph_list[i]];
        }
      else
        {
          advances[i] = NSZeroSize;
        }
    }

  [self insertGlyphs: glyph_list
    withAdvancements: advances
	      length: length
	forStartingGlyphAtIndex: glyph
      characterIndex: index];
}

- (NSUInteger) layoutOptions
{
  NSUInteger options = 0;

  if (showsInvisibleCharacters)
    options |= NSShowInvisibleGlyphs;
  if (showsInvisibleCharacters)
    options |= NSShowControlGlyphs;

  return options;
}

- (void) setIntAttribute: (NSInteger)attributeTag 
                   value: (NSInteger)anInt
         forGlyphAtIndex: (NSUInteger)glyphIndex
{
  glyph_run_t *run;
  glyph_t *g;
  unsigned int pos;

  run = run_for_glyph_index(glyphIndex, glyphs, &pos, NULL);
  if (run && run->glyphs && (run->head.glyph_length < glyphIndex - pos))
    {
      g = &run->glyphs[glyphIndex - pos];

      if (attributeTag == NSGlyphAttributeInscribe)
        g->inscription = anInt;
      else if (attributeTag == NSGlyphAttributeSoft)
        g->soft = anInt;
      else if (attributeTag == NSGlyphAttributeElastic)
        g->elasitc = anInt;
      else if (attributeTag == NSGlyphAttributeBidiLevel)
        g->bidilevel = anInt;
    }
}

- (NSDictionary *) typingAttributes
{
  return [NSTextView defaultTypingAttributes];
}


/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  // FIXME
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  // FIXME
  return self;
}

@end
