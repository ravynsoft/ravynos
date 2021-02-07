/** Permit handling JSON as plists, and writing Objective-C literals.
   Copyright (C) 2020 Free Software Foundation, Inc.

   Written by:  Mingye Wang
   Created: feb 2020

   This file is part of the GNUstep Objective-C Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/

#import "Foundation/NSPropertyList.h"

/** Extra types supported by plutil. */
enum _PLUExtentedFormats
{
  NSPropertyListJSONFormat = NSPropertyListBinaryFormat_v1_0 + 100,
  /** https://clang.llvm.org/docs/ObjectiveCLiterals.html */
  NSPropertyListObjectiveCFormat,
  /** https://docs.swift.org/swift-book/ReferenceManual/zzSummaryOfTheGrammar.html */
  NSPropertyListSwiftFormat,
};

@interface NSPropertyListSerialization (PLUtilAdditions)
+ (NSData *) _pdataFromPropertyList: (id)aPropertyList
			     format: (NSPropertyListFormat)aFormat
		   errorDescription: (NSString **)anErrorString;
+ (id) _ppropertyListWithData: (NSData *)data
		      options: (NSPropertyListReadOptions)anOption
		       format: (NSPropertyListFormat *)aFormat
		        error: (out NSError **)error;
+ (void) load;
@end
