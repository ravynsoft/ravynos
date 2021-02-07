/* 
   GSCUPSPrintInfo.m

   Stores information used in printing

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Chad Hardin <cehardin@mac.com>
   Date: October 2004
   
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

#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#import "AppKit/NSPrinter.h"
#import "GSCUPSPrintInfo.h"
#import "GSCUPSPrinter.h"
// There are broken versions of cups.h where __BLOCKS__ requires
// libdispatch to be present. This has long been fixed in CUPS,
// but the file is still in CentOS 7. 
#undef __BLOCKS__
#include <cups/cups.h>


@implementation GSCUPSPrintInfo

//
// Class methods
//
+ (void)initialize
{
  if (self == [GSCUPSPrintInfo class])
    {
      // Initial version
      [self setVersion: 1];
    }
}

// Required because the super class redefines the default
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject (self, 0, z);
}

+ (NSPrinter*) defaultPrinter
{
  NSString *defaultName;
  int numDests;
  cups_dest_t* dests;
  cups_dest_t* dest = NULL;
 
  numDests = cupsGetDests( &dests );
  if (dests) 
    {
      // use NULL to request the default printer
      dest = cupsGetDest( NULL, NULL, numDests, dests );
    }
 
  if (dest)
     {
       defaultName = [NSString stringWithCString: dest->name];
     }
   else
     {
       defaultName = GSCUPSDummyPrinterName;
     }
  NSDebugLLog(@"GSCUPS", @"The default printer name is %@", defaultName);
  cupsFreeDests( numDests, dests );
  
  return [NSPrinter printerWithName: defaultName];  
}


+ (void)setDefaultPrinter:(NSPrinter *)printer
{
  NSString* name;
  int numDests;
  cups_dest_t* dests;
  int n;
  BOOL found = NO;
  
  name = [printer name];
  
  numDests = cupsGetDests( &dests );

  for( n = 0; n < numDests; n++ )
    {
      if( [name isEqualToString: [NSString stringWithCString: dests[n].name]] &&
          dests[n].instance == NULL)
        {
          found = YES;
          break;
        }
    }

  if( found == NO )
    {
      NSDebugMLLog(@"GSPrinting", @"Printer %@ not found", name);
      return;
    }

  for( n = 0; n < numDests; n++ )
    {
      dests[n].is_default = 0;
    }

  for( n = 0; n < numDests; n++ )
    {
      if( [name isEqualToString: [NSString stringWithCString: dests[n].name]] &&
          dests[n].instance == NULL)
        {
          dests[n].is_default = 1;
          break;
        }
    }

  cupsSetDests( numDests, dests );
  cupsFreeDests( numDests, dests );
}

@end
