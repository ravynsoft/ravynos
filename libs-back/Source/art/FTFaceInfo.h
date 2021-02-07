/*
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

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

#ifndef FTFaceInfo_h
#define FTFaceInfo_h

#include <Foundation/NSObject.h>

@class NSString;
@class NSArray;

@interface FTFaceInfo : NSObject
{
@public
  NSString *familyName;

  /* the following two are localized */
  NSString *faceName;
  NSString *displayName;

  NSArray *files;
  struct
  {
    int pixel_size;
    NSArray *files;
  } *sizes;
  int num_sizes;

  int weight;
  unsigned int traits;

  /*
  hinting hints
    0: 1 to use the auto-hinter
    1: 1 to use hinting
  byte 0 and 1 contain hinting hints for un-antialiased and antialiased
  rendering, respectively.

   16: 0=un-antialiased by default, 1=antialiased by default
  */
  unsigned int render_hints_hack;
}

@end

#endif
