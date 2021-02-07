/** <title>NSFileWrapper</title>

   <abstract>Hold a file's contents in dynamic memory.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: Sept 1998
   Author: Jonathan Gapen <jagapen@whitewater.chem.wisc.edu>
   Date: Dec 1999
   
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

#include "config.h"

#import <AppKit/NSFileWrapper.h>
#import <AppKit/NSFileWrapperExtensions.h>
#import "AppKit/NSImage.h"
#import "AppKit/NSWorkspace.h"

@implementation NSFileWrapper (NSExtensions)

- (void) setIcon: (NSImage*)icon
{
  ASSIGN(_icon, icon);
}

- (NSImage*) icon
{
  if (_icon == nil && [self filename])
    {
      return [[NSWorkspace sharedWorkspace] iconForFile: [self filename]];
    }
  else
    {
      return (NSImage *)_icon;
    }
}

@end

