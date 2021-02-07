/* 
   GLPRSPrintOperation.m

   Controls operations generating EPS, PDF or PS print jobs.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Started implementation.
   Modified for Printing Backend Support
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

#include <config.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPrinter.h"
#import "AppKit/NSPrintPanel.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "GSGuiPrivate.h"
#import "GSLPRPrintOperation.h"

//A subclass of GSPrintOperation, NOT NSPrintOperation.
@implementation GSLPRPrintOperation
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
	    stringByAppendingPathComponent: @"GSLPRPrintJob-"];
  
  _path = [_path stringByAppendingString: 
		       [[NSProcessInfo processInfo] globallyUniqueString]];
           
  _path = [_path stringByAppendingPathExtension: @"ps"];
  
  RETAIN(_path);
  
  return self;
}


- (BOOL) _deliverSpooledResult
{
  int copies;
  NSDictionary *dict;
  NSTask *task;
  NSString *name, *status;
  NSMutableArray *args;
  
  name = [[[self printInfo] printer] name];
  status = [NSString stringWithFormat: _(@"Spooling to printer %@."), name];
  [[self printPanel] _setStatusStringValue: status];

  dict = [[self printInfo] dictionary];
  args = [NSMutableArray array];
  copies = [[dict objectForKey: NSPrintCopies] intValue];
  if (copies > 1)
    [args addObject: [NSString stringWithFormat: @"-#%0d", copies]];
  if ([name isEqual: @"Unknown"] == NO)
    {
      [args addObject: @"-P"];
      [args addObject: name];
    }
  [args addObject: _path];

  task = [NSTask new];
  [task setLaunchPath: @"lpr"];
  [task setArguments: args];
  [task launch];
  [task waitUntilExit];
  AUTORELEASE(task);
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
