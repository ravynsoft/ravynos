/* 
   GSPDFPrintOperation.m

   Controls operations generating PDF output files.

   Copyright (C) 1996, 2004 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Started implementation.
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004
   Modified for printing backend support, split off from NSPrintOperation.m
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


#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSView.h"
#import "GNUstepGUI/GSPDFPrintOperation.h"


/**
  <unit>
  <heading>Class Description</heading>
  <p>
  GSPDFPrintOperation produces PDF files for saving, previewing, etc
  </p>
  </unit>
*/ 


@implementation GSPDFPrintOperation

- (id) initWithView:(NSView *)aView 
         insideRect:(NSRect)rect 
             toData:(NSMutableData *)data 
          printInfo:(NSPrintInfo*)aPrintInfo
{
  self = [super initWithView: aView
                  insideRect: rect
                      toData: data
                   printInfo: aPrintInfo];
                   
  _path = [NSTemporaryDirectory() stringByAppendingPathComponent: @"GSPrint-"];
  
  _path = [_path stringByAppendingString: 
		               [[NSProcessInfo processInfo] globallyUniqueString]];
           
  _path = [_path stringByAppendingPathExtension: @"pdf"];
  RETAIN(_path);
                  
  return self;
}

- (id) initWithView:(NSView *)aView 
         insideRect:(NSRect)rect 
             toPath:(NSString *)path 
          printInfo:(NSPrintInfo*)aPrintInfo
{
  NSMutableData *data = [NSMutableData data];

  self = [super initWithView: aView	
                  insideRect: rect
                      toData: data
                   printInfo: aPrintInfo];

  ASSIGN(_path, path);

  return self;
}

- (NSGraphicsContext*)createContext
{
  NSMutableDictionary *info;

  if (_context)
    return _context;

  info = [[self printInfo] dictionary];

  // TODO: Instead we should support NSGraphicsContext writing to an 
  // NSMutableData directly.
  [info setObject: _path 
           forKey: @"NSOutputFile"];
  
  [info setObject: NSGraphicsContextPDFFormat
           forKey: NSGraphicsContextRepresentationFormatAttributeName];
           
  _context = RETAIN([NSGraphicsContext graphicsContextWithAttributes: info]);
  return _context;
}

- (void) _print
{
  // TODO: Copied-and-pasted from GSEPSPrintOperation. Factor out.

  /* Save this for the view to look at. Seems like there should
     be a better way to pass it to beginDocument */
  [[[self printInfo] dictionary] setObject: [NSValue valueWithRect: _rect]
                                 forKey: @"NSPrintSheetBounds"];

  [_view beginDocument];
  [_view beginPageInRect: _rect
             atPlacement: NSMakePoint(0,0)];

  [_view displayRectIgnoringOpacity: _rect inContext: [self context]];

  [_view endPage];
  [_view endDocument];

  // FIXME: Output comes out up-side-down
}

- (BOOL)deliverResult
{
  if (_data != nil && _path != nil)
    {
      [_data setData: [NSData dataWithContentsOfFile: _path]];
    }
  
  return YES;
}

@end
