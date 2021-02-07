/*
   GSHorizontalTypesetter.h

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

#ifndef _GNUstep_H_GSHorizontalTypesetter
#define _GNUstep_H_GSHorizontalTypesetter

#import <GNUstepGUI/GSTypesetter.h>

@class NSLock;
@class GSLayoutManager, NSTextContainer, NSTextStorage;
@class NSDictionary;
@class NSParagraphStyle, NSFont;

@interface GSHorizontalTypesetter : GSTypesetter
{
  NSLock *lock;

  GSLayoutManager *curLayoutManager;
  NSTextContainer *curTextContainer;
  NSTextStorage *curTextStorage;

  unsigned int curGlyph;
  NSPoint curPoint;


  NSParagraphStyle *curParagraphStyle;
  NSRange paragraphRange; /* characters */

  NSDictionary *curAttributes;
  NSRange attributeRange; /* characters */
  struct
    {
      BOOL explicit_kern;
      float kern;
      float baseline_offset;
      int superscript;
    } attributes;

  NSFont *curFont;
  NSRange fontRange; /* glyphs */

  struct GSHorizontalTypesetter_glyph_cache_s *cache;
  /*
    cache_base: index of first glyph in cache within the text container
    cache_size: capacity of cache
    cache_length: how much of the cache is filled
   */
  unsigned int cache_base, cache_size, cache_length;
  BOOL at_end;


  struct GSHorizontalTypesetter_line_frag_s *line_frags;
  int line_frags_num, line_frags_size;
}

+(GSHorizontalTypesetter *) sharedInstance;

@end

#endif

