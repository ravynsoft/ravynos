/* 
   GSCUPSPrintOperation.m

   Controls operations generating EPS, PDF or PS print jobs.

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Chad Hardin
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

#include <math.h>
#include <config.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSView.h>
#import <AppKit/NSPrinter.h>
#import <AppKit/NSPrintPanel.h>
#import <AppKit/NSPrintInfo.h>
#import <AppKit/NSPrintOperation.h>
#import "GSGuiPrivate.h"
#import "GSCUPSPrintOperation.h"
#undef __BLOCKS__
#include <cups/cups.h>


//A subclass of GSPrintOperation, NOT NSPrintOperation.
@implementation GSCUPSPrintOperation


// Required because the super class redefines the default
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject (self, 0, z);
}

- (id)initWithView:(NSView *)aView
         printInfo:(NSPrintInfo *)aPrintInfo
{
  self = [super initWithView: aView
                   printInfo: aPrintInfo];
  if (self == nil)
    return nil;

  _path = [NSTemporaryDirectory()
	    stringByAppendingPathComponent: @"GSCUPSPrintJob-"];
  
  _path = [_path stringByAppendingString: 
		       [[NSProcessInfo processInfo] globallyUniqueString]];
           
  _path = [_path stringByAppendingPathExtension: @"ps"];
  
  RETAIN(_path);
  
  return self;
}
  


- (BOOL) _deliverSpooledResult
{
  NSString *name, *status;
  
  name = [[[self printInfo] printer] name];
  status = [NSString stringWithFormat: _(@"Spooling to printer %@."), name];
  [[self printPanel] _setStatusStringValue: status];

  cupsPrintFile( [name UTF8String], 
                 [_path UTF8String], 
                 [_path UTF8String], 
                 0, NULL );
  return YES;
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
