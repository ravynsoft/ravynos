/** <title>GSLPRPrinter</title>

   <abstract>Class representing a printer's or printer model's capabilities.</abstract>

   Copyright (C) 1996, 1997, 2004 Free Software Foundation, Inc.

   Authors: Simon Frankau <sgf@frankau.demon.co.uk>
   Date: June 1997
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004
   
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

#include "config.h"
#import <Foundation/NSDebug.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import "GSLPRPrinter.h"
#import "GNUstepGUI/GSPrinting.h"


@implementation GSLPRPrinter

//
// Class methods
//
+(void) initialize
{
  if (self == [GSLPRPrinter class])
    {
      // Initial version
      [self setVersion:1];
    }
}


+(id) allocWithZone: (NSZone*) zone
{
  return NSAllocateObject(self, 0, zone);
}

//
// Finding an NSPrinter 
//
+ (NSPrinter*) printerWithName: (NSString*) name
{
  NSDictionary* printersDict;
  NSDictionary* printerEntry;
  NSPrinter* printer;

  printersDict = [self printersDictionary];
  printerEntry = [printersDict objectForKey: name];

  if( printerEntry == nil)
    {
      [NSException raise: NSGenericException
                  format: @"(GSLPR) Could not find printer named %@", name];
      return nil;
    }

  NSDebugMLLog(@"GSPrinting", @"Creating NSPrinter with Printer Entry: %@", 
               [printerEntry description]);

  printer = [(GSLPRPrinter*)[self alloc]
                    initWithName: name
                        withType: [printerEntry objectForKey: @"Type"]
                        withHost: [printerEntry objectForKey: @"Host"]
                        withNote: [printerEntry objectForKey: @"Note"]];

  [printer parsePPDAtPath: [printerEntry objectForKey: @"PPDPath"]];
                         
  return AUTORELEASE(printer);
}


+ (NSArray *)printerNames
{
  return [[self printersDictionary] allKeys];
}

//
// Load the printer setup from NSUserDefaults
//
+ (NSDictionary*) printersDictionary
{
  static BOOL didWarn;
  NSUserDefaults* defaults;
  NSDictionary *printers;
  
  defaults = [NSUserDefaults standardUserDefaults];
  printers = [defaults dictionaryForKey: @"GSLPRPrinters"];

  if (!printers) //Not set, make a default printer because we are nice.
    {
      NSString *ppdPath;
      NSMutableDictionary *printerEntry;

      printers = [NSMutableDictionary dictionary];
      printerEntry = [NSMutableDictionary dictionary];

      ppdPath = [NSBundle
	pathForLibraryResource: @"Generic-PostScript_Printer-Postscript"
			ofType: @"ppd"
		   inDirectory: @"PostScript/PPD"];
      NSAssert(ppdPath,
	       @"Couldn't find the PPD file for the fallback printer.");

      [printerEntry setObject: ppdPath
		       forKey: @"PPDPath"];

      [printerEntry setObject: @"localhost"
                       forKey: @"Host"];

      [printerEntry setObject: @"Automatically Generated"
                       forKey: @"Note"];

      [printerEntry setObject: @"Unknown"
                       forKey: @"Type"];

      [(NSMutableDictionary*)printers setObject: printerEntry
                                         forKey: @"Unnamed"];

      [defaults setObject: printers forKey: @"GSLPRPrinters"];
      [defaults synchronize];

      if (!didWarn)
	{
	  NSLog(@"Creating a default printer since no printer has been set "
 		@"in the user defaults (under the GSLPRPrinters key).");
	  didWarn = YES;
	}
    }

  return printers;
}

@end
