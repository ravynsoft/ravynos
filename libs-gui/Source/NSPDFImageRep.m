/* Implementation of class NSPDFImageRep
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Fri Nov 15 04:24:27 EST 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "config.h"

#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSPDFImageRep.h"
#import <GNUstepGUI/GSImageMagickImageRep.h>

@implementation NSPDFImageRep

+ (BOOL) canInitWithData: (NSData *)imageData
{
  NSData *header = [imageData subdataWithRange: NSMakeRange(0,4)];
  NSString *str = [[NSString alloc] initWithData: header encoding: NSUTF8StringEncoding];
  BOOL result = [str isEqualToString: @"%PDF"];
  AUTORELEASE(str);
  return result;
}

+ (NSArray *) imagePasteboardTypes
{
  return [NSArray arrayWithObject: NSPDFPboardType];
}

+ (NSArray *) imageUnfilteredPasteboardTypes
{
  return [self imagePasteboardTypes];
}

+ (NSArray *) imageFileTypes
{
  return [NSArray arrayWithObjects: @"pdf", @"PDF", nil];
}

+ (NSArray *) imageUnfilteredFileTypes
{
  return [self imageFileTypes];
}

+ (instancetype) imageRepWithData: (NSData *)imageData
{
  return [[[self class] alloc] initWithData: imageData];
}

- (instancetype) initWithData: (NSData *)imageData
{
  self = [super init];
  if(self != nil)
    {
#if HAVE_IMAGEMAGICK
      ASSIGN(_pageReps, [GSImageMagickImageRep imageRepsWithData: imageData]);
      _size = [[_pageReps objectAtIndex: 0] size];
      _currentPage = 1;
#else
      ASSIGN(_pageReps, [NSArray array]);
      _size = NSMakeSize(0,0);
      _currentPage = 0;
#endif
      ASSIGNCOPY(_pdfRepresentation, imageData);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_pageReps);
  RELEASE(_pdfRepresentation);
  [super dealloc];
}

- (NSRect) bounds
{
  NSSize size = [self size];
  NSRect rect = NSMakeRect(0, 0, size.width, size.height);
  return rect;
}

- (NSInteger) currentPage
{
  return _currentPage;
}

- (void) setCurrentPage: (NSInteger)currentPage
{
  _currentPage = currentPage;
}

- (NSInteger) pageCount
{
  return [_pageReps count];
}

- (NSData *) PDFRepresentation
{
  return _pdfRepresentation;
}

// Override to draw the specified page...
- (BOOL) draw
{
  NSBitmapImageRep *rep = [_pageReps objectAtIndex: _currentPage - 1];
  return [rep draw];
}
@end

