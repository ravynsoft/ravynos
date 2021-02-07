/*
   OpalFaceInfo.m
 
   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Ivan Vucica <ivan@vucica.net>
   Date: September 2013
 
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

#import "opal/OpalFaceInfo.h"

@implementation OpalFaceInfo 

- (void) dealloc
{
  if (_fontFace)
    {
      CGFontRelease(_fontFace);
    }
  [super dealloc];
}

- (void *)fontFace
{
  if (!_fontFace)
    {
      FcPattern *resolved;

      resolved = [self matchedPattern];

      _fontFace = OPFontCreateWithFcPattern(resolved);
      FcPatternDestroy(resolved);

      if (!_fontFace)
        {
          NSLog(@"Creating a font face failed %@", _familyName);
          return NULL;
        }
    }

  return _fontFace;
}

@end
