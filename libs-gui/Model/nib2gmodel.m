/*
   nib2gmodel.m

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: November 1997
   
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

#include <stdio.h>

#include <Foundation/Foundation.h>
#include "Translator.h"

int main ()
{
  id pool = [NSAutoreleasePool new];
  NSProcessInfo* processInfo = [NSProcessInfo processInfo];
  NSArray* arguments = [processInfo arguments];
  id translator;

  if ([arguments count] != 3) {
    printf ("Convert nib files to GNU model files.\n");
    printf ("usage: %s nib-file gmodel-file\n",
	    [[processInfo processName] cString]);
    return (1);
  }

  translator = [[Translator new] autorelease];
  [translator translateNibFile:[arguments objectAtIndex:1]
	      toModelFile:[arguments objectAtIndex:2]];

//  [pool release];

  return 0;
}
