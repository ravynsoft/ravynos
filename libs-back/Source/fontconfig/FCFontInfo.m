/*
   FCFontInfo.m
 
   Copyright (C) 2003 Free Software Foundation, Inc.

   August 31, 2003
   Written by Banlu Kemiyatorn <object at gmail dot com>
   Base on original code of Alex Malmberg

   This file is part of GNUstep.

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

#include "GNUstepBase/Unicode.h"
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include "fontconfig/FCFontInfo.h"
#include "fontconfig/FCFontEnumerator.h"

#include <math.h>

@implementation FCFontInfo 

- (void) setCacheSize: (unsigned int)size
{
  _cacheSize = size;
  if (_cachedSizes)
    {
      free(_cachedSizes);
    }
  if (_cachedGlyphs)
    {
      free(_cachedGlyphs);
    }
  _cachedSizes = malloc(sizeof(NSSize) * size);
  if (_cachedSizes)
    {
      memset(_cachedSizes, 0, sizeof(NSSize) * size);
    }
  _cachedGlyphs = malloc(sizeof(unsigned int) * size);
  if (_cachedGlyphs)
    {
      memset(_cachedGlyphs, 0, sizeof(unsigned int) * size);
    }
}

- (BOOL) setupAttributes
{
  ASSIGN(_faceInfo, [FCFontEnumerator fontWithName: fontName]);
  if (!_faceInfo)
    {
      return NO;
    }

  // check for font specific cache size from face info
  [self setCacheSize: [_faceInfo cacheSize]];

  /* setting GSFontInfo:
   * weight, traits, familyName,
   * mostCompatibleStringEncoding, encodingScheme, coveredCharacterSet
   */

  weight = [_faceInfo weight];
  traits = [_faceInfo traits];
  familyName = [[_faceInfo familyName] copy];
  mostCompatibleStringEncoding = NSUTF8StringEncoding;
  encodingScheme = @"iso10646-1";
  coveredCharacterSet = [[_faceInfo characterSet] retain];

  return YES;
}

- (id) initWithFontName: (NSString *)name 
                 matrix: (const CGFloat *)fmatrix 
             screenFont: (BOOL)p_screenFont
{
  self = [super init];
  if (!self)
    return nil;

  _screenFont = p_screenFont;
  fontName = [name copy];
  memcpy(matrix, fmatrix, sizeof(matrix));

  if (_screenFont)
    {
      /* Round up; makes the text more legible. */
      matrix[0] = ceil(matrix[0]);
      if (matrix[3] < 0.0)
        matrix[3] = floor(matrix[3]);
      else
        matrix[3] = ceil(matrix[3]);
    }

  if (![self setupAttributes])
    {
      RELEASE(self);
      return nil;
    }

  return self;
}

- (void) dealloc
{
  RELEASE(_faceInfo);
  if (_cachedSizes)
    free(_cachedSizes);
  if (_cachedGlyphs)
    free(_cachedGlyphs);
  [super dealloc];
}

- (CGFloat) defaultLineHeightForFont
{
  return lineHeight;
}

- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  [self subclassResponsibility: _cmd];
  return YES;
}

- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  [self subclassResponsibility: _cmd];
  return NSZeroSize;
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
  [self subclassResponsibility: _cmd];
  return NSZeroRect;
}

- (NSString *) displayName
{
  return [_faceInfo displayName];
}

- (CGFloat) widthOfString: (NSString *)string
{
  [self subclassResponsibility: _cmd];
  return 0.0;
}

- (NSGlyph) glyphWithName: (NSString *) glyphName
{
  /* subclass should override */
  /* terrible! FIXME */
  NSGlyph g = [glyphName cString][0];

  return g;
}

- (void) appendBezierPathWithGlyphs: (NSGlyph *)glyphs 
                              count: (int)length 
                       toBezierPath: (NSBezierPath *)path
{
  [self subclassResponsibility: _cmd];
}

@end
