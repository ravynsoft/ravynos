/** <title>GSCUPSPrinter</title>

   <abstract>Class representing a printer's or printer model's capabilities.</abstract>

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

/* NB:
 * There are a few FIXMEs in the functionality left.
 * Parsing of the PPDs is somewhat suboptimal.
 * (I think it's best to leave optimisation until more of GNUstep is done).
 * The *OpenUI, *CloseUI, *OpenGroup and *CloseGroup are not processed.
 * (This is not required in the OpenStep standard, but could be useful).
 */

#import "config.h"
#import <Foundation/NSDebug.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSSet.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSGraphics.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GSCUPSPrinter.h"

#undef __BLOCKS__
#include <cups/cups.h>
#include <cups/ppd.h>


NSString *GSCUPSDummyPrinterName = @"GSCUPSDummyPrinter";

@implementation GSCUPSPrinter

//
// Class methods
//
+(void) initialize
{
  if (self == [GSCUPSPrinter class])
    {
      // Initial version
      [self setVersion:1];
    }
}

// Required because the super class redefines the default
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject (self, 0, z);
}

//
// Finding an NSPrinter 
//
+ (NSPrinter*) printerWithName: (NSString*) name
{
  NSPrinter* printer;
  const char* ppdFile;

  if ([name isEqual: GSCUPSDummyPrinterName])
    {
      /* Create a dummy printer as a fallback.  */
static BOOL didWarn;
      NSString *ppdPath;

      if (!didWarn)
	{
	  NSLog(@"Creating a default printer since no default printer has "
		@"been set in CUPS.");
	  didWarn = YES;
	}

      ppdPath = [NSBundle
	pathForLibraryResource: @"Generic-PostScript_Printer-Postscript"
			ofType: @"ppd"
		   inDirectory: @"PostScript/PPD"];
      NSAssert(ppdPath,
	       @"Couldn't find the PPD file for the fallback printer.");

      printer = [(GSCUPSPrinter*)[self alloc]
		  initWithName: name
		      withType: @"Unknown"
		      withHost: @"Unknown"
		      withNote: @"Automatically Generated"];

      [printer parsePPDAtPath: ppdPath];

      return AUTORELEASE(printer);
    }

  printer = [[GSCUPSPrinter alloc]
                    initWithName: name
                        withType: @"Type Unknown"
                        withHost: @"Host Unknown"
                        withNote: @"No Note"];

  ppdFile = cupsGetPPD( [name UTF8String] );

  if( ppdFile )
    {
      [printer parsePPDAtPath: [NSString stringWithCString: ppdFile]];
      unlink( ppdFile );
    }
  else
    {
      NSLog(@"Printer %@ does not have a PPD!", name);
    }
                         
  return AUTORELEASE(printer);
}


+ (NSArray *)printerNames
{
  NSMutableSet *set;
  int numDests;
  cups_dest_t* dests;
  int n;

  set = [[NSMutableSet alloc] init];
  AUTORELEASE(set);

  numDests = cupsGetDests(&dests);
  
  for (n = 0; n < numDests; n++)
    {
      [set addObject: [NSString stringWithCString: dests[n].name]];
    }

  cupsFreeDests(numDests, dests);

  // No printer found, return at least the dummy printer
  if ([set count] == 0)
    {
      [set addObject: GSCUPSDummyPrinterName];
    }

  return [set allObjects];
}

@end

