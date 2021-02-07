/* Implementation of class NSPathControlItem
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Wed Apr 22 18:20:16 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSAttributedString.h>
#import "AppKit/NSPathControlItem.h"
#import "AppKit/NSImage.h"

@implementation NSPathControlItem
  
- (NSAttributedString *) attributedTitle
{
  return _attributedTitle;
}

- (void) setAttributedTitle: (NSAttributedString *)attributedTitle
{
  ASSIGNCOPY(_attributedTitle, attributedTitle);
}

- (NSImage *) image
{
  return _image;
}

- (void) setImage: (NSImage *)image
{
  ASSIGNCOPY(_image, image);
}

- (NSURL *) URL
{
  return _url;
}

- (void) setURL: (NSURL *)url
{
  ASSIGNCOPY(_url, url);
}

- (NSString *) title
{
  return [_attributedTitle string];
}

- (void) setTitle: (NSString *)title
{
  NSAttributedString *attrTitle = [[NSAttributedString alloc] initWithString: title];
  [self setAttributedTitle: attrTitle];
  RELEASE(attrTitle);
}

- (void) dealloc
{
  RELEASE(_attributedTitle);
  RELEASE(_image);
  RELEASE(_url);
  [super dealloc];
}

@end

