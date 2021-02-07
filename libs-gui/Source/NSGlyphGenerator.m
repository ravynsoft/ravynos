/*
   NSGlyphGenerator.m

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: April 2008

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
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>
#import <GNUstepBase/Unicode.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGlyphGenerator.h"
/* just for NSAttachmentCharacter */
#import "AppKit/NSTextAttachment.h"
#import "GNUstepGUI/GSFontInfo.h"

static NSGlyphGenerator* instance;

@interface NSGlyphGenerator (Private)
- (NSFont *) fontForCharactersWithAttributes: (NSDictionary *)attributes;
@end

@implementation NSGlyphGenerator

+ (id) sharedGlyphGenerator
{
  if (!instance)
    instance = [[NSGlyphGenerator alloc] init];

  return instance;
}

// Send a run of glyphs where each glyph corresponds to one character.
#define SEND_GLYPHS() \
{ \
  NSUInteger length; \
 \
  if ((length = g - glyphs)) \
    { \
      [storage insertGlyphs: glyphs \
               length: length \
               forStartingGlyphAtIndex: *glyph \
               characterIndex: *index]; \
      *index += i - cstart + 1; \
      *glyph += length; \
      g = glyphs; \
      cstart = i + 1; \
    } \
}

/*
This is a fairly simple implementation. It will use "ff", "fl", "fi",
"ffl", and "ffi" ligatures if available. 

TODO: how should words like "pfffffffffff" be handled?

0066 'f'
0069 'i'
006c 'l'
fb00 'ff'
fb01 'fi'
fb02 'fl'
fb03 'ffi'
fb04 'ffl'
*/
- (void) generateGlyphsForGlyphStorage: (id <NSGlyphStorage>)storage
             desiredNumberOfCharacters: (NSUInteger)num
                            glyphIndex: (NSUInteger*)glyph
                        characterIndex: (NSUInteger*)index
{
  // Try to get enough space for all glyphs
  NSGlyph glyphs[2 * num];
  NSGlyph *g;
  NSGlyph gl;
  NSAttributedString *attrstr = [storage attributedString];
  GSFontInfo *fi;
  int i;
  unichar buf[num];
  unsigned int cstart = 0;
  NSRange maxRange = NSMakeRange(*index, num);
  NSRange curRange;
  NSDictionary *attributes;
  NSNumber *n;
  int ligature;
  BOOL surr;
  NSCharacterSet *cs = [NSCharacterSet controlCharacterSet];
  SEL cim_sel = @selector(characterIsMember:);
  BOOL (*characterIsMember)(id, SEL, unichar)
    = (BOOL(*)(id, SEL, unichar)) [cs methodForSelector: cim_sel];
  SEL gfc_sel = @selector(glyphForCharacter:);
  NSGlyph (*glyphForCharacter)(id, SEL, unichar);
  NSGlyph fallback = NSNullGlyph;

  [[attrstr string] getCharacters: buf range: maxRange];
  attributes = [attrstr attributesAtIndex: *index
                        longestEffectiveRange: &curRange
                        inRange: maxRange];
  fi = [[self fontForCharactersWithAttributes: attributes] fontInfo];
  if (!fi)
    {
      [NSException raise: NSGenericException
                   format: @"Glyph generation with no font."];
      return;
    }
  glyphForCharacter = (NSGlyph(*)(id, SEL, unichar)) [fi methodForSelector: gfc_sel];

  n = [attributes objectForKey: NSLigatureAttributeName];
  if (n)
    ligature = [n intValue];
  else
    ligature = 1;

  g = glyphs;
  for (i = 0; i < num; i++)
    {
      unsigned int ch, ch2;

      ch = buf[i];
      if (characterIsMember(cs, cim_sel, ch))
        {
          *g = NSControlGlyph;
          g++;
          continue;
        }
      if (ch == NSAttachmentCharacter)
        {
          *g = GSAttachmentGlyph;
          g++;
          continue;
        }

      // Simple ligature processing
      if ((ligature >= 1) && (i + 1 < num))
        {
          ch2 = buf[i + 1];

          if (ch == 'f')
            {
              if ((i + 2 < num) && (ch2 == 'f'))
                {
                  // ffl
                  if ((buf[i + 2] == 'l') 
                      && (NSNullGlyph != (gl = glyphForCharacter(fi, gfc_sel, 0xfb04))))
                    {
                      *g = gl;
                      g++;
                      i += 2;
                      SEND_GLYPHS();
                      continue;
                    }
                  // ffi
                  if ((buf[i + 2] == 'i') 
                      && (NSNullGlyph != (gl = glyphForCharacter(fi, gfc_sel, 0xfb03))))
                    {
                      *g = gl;
                      g++;
                      i += 2;
                      SEND_GLYPHS();
                      continue;
                    }
                }
              
              // ff
              if ((ch2 == 'f')
                  && (NSNullGlyph != (gl = glyphForCharacter(fi, gfc_sel, 0xfb00))))
                {
                  *g = gl;
                  g++;
                  i++;
                  SEND_GLYPHS();
                  continue;
                }
              // fi
              if ((ch2 == 'i')
                  && (NSNullGlyph != (gl = glyphForCharacter(fi, gfc_sel, 0xfb01))))
                {
                  *g = gl;
                  g++;
                  i++;
                  SEND_GLYPHS();
                  continue;
                }
              // fl
              if ((ch2 == 'l')
                  && (NSNullGlyph != (gl = glyphForCharacter(fi, gfc_sel, 0xfb02))))
                {
                  *g = gl;
                  g++;
                  i++;
                  SEND_GLYPHS();
                  continue;
                }
            }
        }

      surr = NO;
      // Check for surrogate pairs
      if (ch >= 0xd800 && ch <= 0xdfff)
        {
          if (ch >= 0xd800 && ch < 0xdc00 
              && (i + 1 < num) && (ch2 = buf[i + 1]) >= 0xdc00 
              && ch2 <= 0xdfff)
            {
              ch = ((ch & 0x3ff) << 10) + (ch2 & 0x3ff) + 0x10000;
              i++;
              surr = YES;
            }
          else
            {
              ch = 0xfffd;
            }
        }

      gl = glyphForCharacter(fi, gfc_sel, ch);
      if (gl != NSNullGlyph)
        {
          *g = gl;
          g++;
          if (surr)
            SEND_GLYPHS();

          continue;
        }

      if (ch < 0x10000)
        {
          unichar *decomp;

          decomp = uni_is_decomp(ch);
          if (decomp)
            {              
              for (; *decomp; decomp++)
                {
                  gl = glyphForCharacter(fi, gfc_sel, *decomp);
                  if (gl == NSNullGlyph)
                    {
                      break;
                    }
                  *g = gl;
                  g++;
                  SEND_GLYPHS();
                }

              continue;
            }
        }

      // No glyph found add fallback
      if (fallback == NSNullGlyph)
        {
          // FIXME: Find a suitable fallback glyph
            unichar uc = '?';
            
            fallback = glyphForCharacter(fi, gfc_sel, uc);
        }
      *g = fallback;
      g++;

      // On a NSNullGLyph, send all previous glyphs
      SEND_GLYPHS();  
    }

  // Send all remaining glyphs
  SEND_GLYPHS(); 
}

@end

@implementation NSGlyphGenerator (Private)

- (NSFont *) fontForCharactersWithAttributes: (NSDictionary *)attributes
{
  NSFont *f = [attributes valueForKey: NSFontAttributeName];
  if (!f)
    f = [NSFont userFontOfSize: 0];

  //f = [storage substituteFontForFont: f];
  return f;
}

@end
