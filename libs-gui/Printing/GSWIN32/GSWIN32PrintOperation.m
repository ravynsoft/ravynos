/* 
   GWIN32PrintOperation.m

   Controls operations generating windows print

   Copyright (C) 2014 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg.casamento@gmail.com>
   Date 2014

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

#include <config.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSData.h>
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPrinter.h"
#import "AppKit/NSPrintPanel.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "GSGuiPrivate.h"
#import "GSWIN32PrintOperation.h"

BOOL RawDataToPrinter(LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount)
{
  HANDLE     hPrinter;
  DOC_INFO_1 DocInfo;
  DWORD      dwJob;
  DWORD      dwBytesWritten;
  
  // Need a handle to the printer.
  if( ! OpenPrinter( szPrinterName, &hPrinter, NULL ) )
    {
      return FALSE;
    }

  // Fill in the structure with info about this "document."
  DocInfo.pDocName = "My Document";
  DocInfo.pOutputFile = NULL;
  DocInfo.pDatatype = "RAW";

  // Inform the spooler the document is beginning.
  if( (dwJob = StartDocPrinter( hPrinter, 1, (LPSTR)&DocInfo )) == 0 )
    {
      ClosePrinter( hPrinter );
      return FALSE;
    }

  // Start a page.
  if( ! StartPagePrinter( hPrinter ) )
    {
      EndDocPrinter( hPrinter );
      ClosePrinter( hPrinter );
      return FALSE;
    }

  // Send the data to the printer.
  if( ! WritePrinter( hPrinter, lpData, dwCount, &dwBytesWritten ) )
    {
      EndPagePrinter( hPrinter );
      EndDocPrinter( hPrinter );
      ClosePrinter( hPrinter );
      return FALSE;
    }

  // End the page.
  if( ! EndPagePrinter( hPrinter ) )
    {
      EndDocPrinter( hPrinter );
      ClosePrinter( hPrinter );
      return FALSE;
    }

  // Inform the spooler that the document is ending.
  if( ! EndDocPrinter( hPrinter ) )
    {
      ClosePrinter( hPrinter );
      return FALSE;
    }

  // Tidy up the printer handle.
  ClosePrinter( hPrinter );

  // Check to see if correct number of bytes were written.
  if( dwBytesWritten != dwCount )
    {
      return FALSE;
    }

  return TRUE;
}

//A subclass of GSPrintOperation, NOT NSPrintOperation.
@implementation GSWIN32PrintOperation
//
// Class methods
//
+ (id) allocWithZone: (NSZone*)zone
{
  return NSAllocateObject(self, 0, zone);
}

- (id)initWithView:(NSView *)aView
         printInfo:(NSPrintInfo *)aPrintInfo
{
  self = [super initWithView: aView
                   printInfo: aPrintInfo];
  if (self == nil)
    return nil;

  _path = [NSTemporaryDirectory()
	    stringByAppendingPathComponent: @"GSWIN32PrintJob-"];
  
  _path = [_path stringByAppendingString: 
		       [[NSProcessInfo processInfo] globallyUniqueString]];
           
  _path = [_path stringByAppendingPathExtension: @"ps"];
  
  RETAIN(_path);
  
  return self;
}


- (BOOL) _deliverSpooledResult
{
  NSString *name = nil;
  NSData *fileData = nil;
  LPSTR szPrinterName = NULL; 
  DWORD dwCount = 0;
  LPBYTE lpData = NULL;
  BOOL result = FALSE;
  
  name = [[[self printInfo] printer] name];
  
  if(name != nil)
    {
      szPrinterName = (LPSTR)[name cString];
      fileData = [NSData dataWithContentsOfFile:_path];
      if(fileData != nil)
	{
	  dwCount = (DWORD)[fileData length];
	  lpData = (LPBYTE)[fileData bytes];
	  result = RawDataToPrinter(szPrinterName, lpData, dwCount);
	}
      else
	{
	  NSLog(@"File is blank");
	}
    }
  else
    {
      NSLog(@"No printer name supplied");
    }

  return (result ? YES : NO); // Paranoid conversion to ObjC type...
}

- (NSGraphicsContext*)createContext
{
  NSMutableDictionary *info;
  NSString *output;
  
  if (_context)
    {
      NSDebugMLLog(@"GSPrinting", @"Already had context, returning it.");
      return _context;
    }
  NSDebugMLLog(@"GSPrinting", @"Creating context.");

  info = [[self printInfo] dictionary];

  output = [info objectForKey: NSPrintSavePath];
  if (output)
    {
      ASSIGN(_path, output);
    }
    
  NSDebugMLLog(@"GSPrinting", @"_path is %@", _path); 
  
  [info setObject: _path 
           forKey: @"NSOutputFile"];

  if ([[[_path pathExtension] lowercaseString] isEqualToString: @"pdf"])
    {
      [info setObject: NSGraphicsContextPDFFormat
	       forKey: NSGraphicsContextRepresentationFormatAttributeName];
    }
  else
    {
      [info setObject: NSGraphicsContextPSFormat
	       forKey: NSGraphicsContextRepresentationFormatAttributeName];
    }
  
  _context = RETAIN([NSGraphicsContext graphicsContextWithAttributes: info]);

  return _context;
}

@end
