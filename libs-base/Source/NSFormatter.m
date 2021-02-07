/** Implementation of NSFormatter class
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

   This file is part of the GNUstep Base Library.

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

   <title>NSFormatter class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSFormatter.h"

@implementation NSFormatter

- (NSAttributedString*) attributedStringForObjectValue: (id)anObject
				 withDefaultAttributes: (NSDictionary*)attr
{
  return nil;
}

- (id) copyWithZone: (NSZone*)zone
{
  return [[self class] allocWithZone: zone];
}

- (NSString*) editingStringForObjectValue: (id)anObject
{
  return [self stringForObjectValue: anObject];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{

}

- (BOOL) getObjectValue: (id*)anObject
	      forString: (NSString*)string
       errorDescription: (NSString**)error
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  return [super init];
}

- (BOOL) isPartialStringValid: (NSString*)partialString
	     newEditingString: (NSString**)newString
	     errorDescription: (NSString**)error
{
  *newString = nil;
  *error = nil;
  return YES;
}

- (BOOL) isPartialStringValid: (NSString**)partialStringPtr
	proposedSelectedRange: (NSRange*)proposedSelRangePtr
	       originalString: (NSString*)origString
	originalSelectedRange: (NSRange)originalSelRangePtr
	     errorDescription: (NSString**)error
{
  *error = nil;
  return YES;
}

- (NSString*) stringForObjectValue: (id)anObject
{
  [self subclassResponsibility: _cmd];
  return nil;
}
@end

