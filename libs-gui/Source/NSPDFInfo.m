/* Implementation of class NSPDFInfo
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat Nov 16 21:20:46 EST 2019

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

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSPDFInfo.h>

@implementation NSPDFInfo

- (instancetype) initWithCoder: (NSCoder *)coder
{
  return nil;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

- (void) dealloc
{
  RELEASE(_url);
  RELEASE(_tagNames);
  RELEASE(_attributes);
  [super dealloc];
}

- (instancetype) copyWithZone: (NSZone *)zone
{
  return nil;
}

- (NSURL *) URL
{
  return _url;
}

- (BOOL) isFileExtensionHidden
{
  return _fileExtensionHidden;
}

- (void) setFileExtensionHidden: (BOOL)flag
{
  _fileExtensionHidden = flag;
}
  
- (NSArray *) tagNames
{
  return _tagNames;
}

- (NSPaperOrientation) orientation
{
  return _orientation;
}

- (void) setOrientation: (NSPaperOrientation)orientation
{
  _orientation = orientation;
}

- (NSSize) paperSize;
{
  return _paperSize;
}

- (void) setPaperSize: (NSSize)size
{
  _paperSize = size;
}

- (NSMutableDictionary *) attributes
{
  return _attributes;
}

@end

