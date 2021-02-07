/** <title>GSWIN32Printer</title>

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

#import "GSWIN32Printer.h"
#import "GNUstepGUI/GSPrinting.h"

NSMutableDictionary *EnumeratePrinters(DWORD flags)
{
    PRINTER_INFO_2* prninfo=NULL;
    DWORD needed, returned;
    NSMutableDictionary *printers = nil;

    //find required size for the buffer
    EnumPrinters(flags, NULL, 2, NULL, 0, &needed, &returned);
 
    //allocate array of PRINTER_INFO structures
    prninfo = (PRINTER_INFO_2*) GlobalAlloc(GPTR,needed);
 
    //call again
    if (!EnumPrinters(flags, NULL, 
        2, (LPBYTE) prninfo, needed, &needed, &returned))
      {
	DWORD errorCode = GetLastError();
        char *errorText = NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM||FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorText,
			0, NULL);
	NSLog(@"Error: %lu: %s\n", errorCode, errorText);
	LocalFree(errorText);
      }
    else
      {
	int i = 0;
	
	printers = [NSMutableDictionary dictionary];
	NSLog(@"Printers found: %d\n\n",returned);
	for (i = 0; i < returned; i++)
	  {
	    NSMutableDictionary *entry = [NSMutableDictionary dictionary];
	    NSString *printerName = 
	      [NSString stringWithCString:prninfo[i].pPrinterName 
				 encoding:NSASCIIStringEncoding];
	    
	    if (printerName != nil)
	      {
		NSString *portName = 
		  [NSString stringWithCString:prninfo[i].pPortName
				     encoding:NSASCIIStringEncoding];
		if (portName != nil)
		  {
		    [entry setObject:portName
			      forKey:@"Host"];
		    [entry setObject:[NSString stringWithFormat:@"Current Status: %d",prninfo[i].Status]
			      forKey:@"Note"];
		    [entry setObject:@"Generic Postscript Printer"
			      forKey:@"Type"];
		    
		    [printers setObject:entry
				 forKey:printerName];
		    /*
		    NSLog(@"%s on %s, Status = %d, jobs=%d\n",
			  prninfo[i].pPrinterName,
			  prninfo[i].pPortName,
			  prninfo[i].Status, 
			  prninfo[i].cJobs);
		    */
		  }
		else
		  {
		    NSLog(@"Port name is nil for %@",printerName);
		  }
	      }
	    else
	      {
		NSLog(@"Printer name is nil");
	      }
	  }
      }
    
    GlobalFree(prninfo);

    return printers;
}

@implementation GSWIN32Printer

//
// Class methods
//
+(void) initialize
{
  if (self == [GSWIN32Printer class])
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
                  format: @"(GSWIN32) Could not find printer named %@", name];
      return nil;
    }

  NSDebugMLLog(@"GSPrinting", @"Creating NSPrinter with Printer Entry: %@", 
               [printerEntry description]);

  printer = [(GSWIN32Printer*)[self alloc]
                    initWithName: name
                        withType: [printerEntry objectForKey: @"Type"]
                        withHost: [printerEntry objectForKey: @"Host"]
                        withNote: [printerEntry objectForKey: @"Note"]];

  // [printer parsePPDAtPath: [printerEntry objectForKey: @"PPDPath"]];
                         
  return AUTORELEASE(printer);
}


+ (NSArray *)printerNames
{
  return [[self printersDictionary] allKeys];
}

//
// Load the printer setup
//
+ (NSDictionary*) printersDictionary
{
  static BOOL didWarn;
  NSDictionary *printers;
  
  printers = EnumeratePrinters(PRINTER_ENUM_LOCAL);
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
      
      if (!didWarn)
	{
	  NSLog(@"Creating a default printer since no printer has been set "
		@"in the user defaults (under the GSWIN32Printers key).");
	  didWarn = YES;
	}
    }
  
  return printers;
}

@end
