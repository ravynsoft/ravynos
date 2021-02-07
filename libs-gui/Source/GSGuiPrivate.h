/* 
   GSGuiPrivate.h

   Define private functions for use in the GNUstep GUI Library

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Nicola Pero <nicola@brainstorm.co.uk>
   Date: 2001
   
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

#ifndef _GNUstep_H_GSGuiPrivate
#define _GNUstep_H_GSGuiPrivate

#import <Foundation/NSBundle.h>
#import <Foundation/NSCoder.h>
#include "GNUstepBase/GSConfig.h"
#include <math.h>

/*
 * Return the gnustep-gui bundle used to load gnustep-gui resources.
 * Should be only used inside the gnustep-gui library.  Implemented
 * in Source/NSApplication.m
 */
NSBundle *GSGuiBundle (void);

/*
 * Localize a message of the gnustep-gui library.  
 */
static inline NSString *GSGuiLocalizedString (NSString *key, NSString *comment)
{
  NSBundle *b = GSGuiBundle ();

  if (b != nil)
    {
      return [b localizedStringForKey: key  value: @""  table: nil];
    }
  else
    {
      return key;
    }
}

/*
 * Redefine _() to be our own GSGuiLocalizedString().  This is so that
 * make_strings will recognize it, and for easy typing.
 */
#ifdef _
# undef _
#endif
#define _(X) GSGuiLocalizedString (X, @"")

#ifdef NSLocalizedString
# undef NSLocalizedString
#endif
#define NSLocalizedString(key,comment) GSGuiLocalizedString (key, comment)

/**
 * Rounds to the nearest integer, and in the case of ties, round to the 
 * larger integer. This is the recommended rounding function for rounding
 * graphics points.
 *
 * For example:
 * GSRoundTowardsInfinity(0.8) == 1.0
 * GSRoundTowardsInfinity(0.5) == 1.0
 * GSRoundTowardsInfinity(0.1) == 0.0
 * GSRoundTowardsInfinity(-2.5) == -2.0
 */
static inline CGFloat GSRoundTowardsInfinity(CGFloat x)
{
  return floor(x + 0.5);
}

/**
 * Rounds to the nearest integer, and in the case of ties, round to the 
 * smaller integer.
 *
 * For example:
 * GSRoundTowardsNegativeInfinity(0.8) == 1.0
 * GSRoundTowardsNegativeInfinity(0.5) == 0.0
 * GSRoundTowardsNegativeInfinity(0.1) == 0.0
 * GSRoundTowardsNegativeInfinity(-2.5) == -3.0
 */
static inline CGFloat GSRoundTowardsNegativeInfinity(CGFloat x)
{
  return ceil(x - 0.5);
}

#if !defined(GS_DECODER)
#define GS_DECODER(type) \
static inline void decode_##type(NSCoder *coder, type *value) \
{ \
[coder decodeValueOfObjCType: @encode(type) at: value]; \
}

#define GS_ENCODER(type) \
static inline void encode_##type(NSCoder *coder, type *value) \
{ \
[coder encodeValueOfObjCType: @encode(type) at: value]; \
}
#endif

GS_ENCODER(NSUInteger)
GS_DECODER(NSUInteger)
GS_ENCODER(NSInteger)
GS_DECODER(NSInteger)

#endif /* _GNUstep_H_GSGuiPrivate */

