/*
   PXKFontEnumerator.m

   NSFontManager helper for GNUstep GUI X/DPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   A completely rewritten version of the original source of Scott Christley.
   
   This file is part of the GNUstep GUI X/DPS Library.

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

#include <stdio.h>
#include "config.h"

#include <DPS/psops.h>

#include <Foundation/NSDictionary.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSSet.h>
#include "xdps/NSDPSContext.h"
#include "AFMFileFontInfo.h"
#include "general.h"
#include "fonts.h"

@implementation PXKFontEnumerator

/* It would be probably more portable to use the standard PS resource
   enumerator functions, but the makepsres utility does not understand how
   DGS organize its resources. So we use the below techniques to identify
   the font and AFM files.

   For Adobe DPS implementations I use the PS resources because it's more
   portable.
 */

- (void)enumerateFontsAndFamilies
{
  int count, length;
  DPSContext ctxt;
  NSArray *fontList;
  NSMutableData *data;

  ctxt = [(NSDPSContext *)GSCurrentContext() xDPSContext];

  PSWEnumerateFonts(ctxt, "*", &count, &length);
  NSDebugLLog(@"Fonts", @"FontManager found %d font names\n", count);
  data = [NSMutableData dataWithLength: count+length];
  PSWGetFontList(ctxt, count+length, (char *)[data mutableBytes]);
  fontList = [[[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding]
		componentsSeparatedByString: @" "];
  allFontNames = RETAIN(fontList);
  // FIXME Enumeration of font families is missing
  allFontFamilies = nil;
}

@end
