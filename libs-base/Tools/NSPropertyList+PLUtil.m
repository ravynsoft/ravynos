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
#import "NSPropertyList+PLUtil.h"
#import "GNUstepBase/GSObjCRuntime.h"
#import "Foundation/NSData.h"
#import "Foundation/NSError.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSJSONSerialization.h"

static IMP originalRead = 0;
static IMP originalWrite = 0;

@implementation NSPropertyListSerialization (PLUtilAdditions)
+ (NSData*) _pdataFromPropertyList: (id)aPropertyList
			    format: (NSPropertyListFormat)aFormat
		  errorDescription: (NSString **)anErrorString
{
  NSError 	*myError = nil;
  NSData 	*dest;
  NSDictionary	*loc;

  loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
  switch (aFormat)
    {
      case NSPropertyListJSONFormat:
	dest = [NSJSONSerialization
	  dataWithJSONObject: aPropertyList
		     options: loc != nil ? NSJSONWritingPrettyPrinted : 0
		       error: &myError];
	if (myError != nil && anErrorString != NULL)
	  {
	    *anErrorString = [myError description];
	  }
	return dest;
      case NSPropertyListObjectiveCFormat:
      case NSPropertyListSwiftFormat:
	*anErrorString = @"Not implemented";
	return nil;
      default:
	return (*originalWrite)(self, _cmd, aPropertyList,
	  aFormat, anErrorString);
    }
}

+ (id) _ppropertyListWithData: (NSData *)data
		      options: (NSPropertyListReadOptions)anOption
		       format: (NSPropertyListFormat *)aFormat
			error: (out NSError **)error;
{
  NSError 		*myError = nil;
  NSPropertyListFormat	format;
  NSJSONReadingOptions	jsonOptions = NSJSONReadingAllowFragments;
  id 			prop;

  prop = (*originalRead)(self, _cmd, data, anOption, &format, &myError);
  if (nil == prop)
    {
      if (format == NSPropertyListOpenStepFormat
	|| format == NSPropertyListGNUstepFormat)
	// rescue as json when we know it is not anything else
	{
	  switch (anOption)
	    {
	      case NSPropertyListMutableContainersAndLeaves:
		jsonOptions |= NSJSONReadingMutableLeaves;
		/* FALLTHROUGH */
	      case NSPropertyListMutableContainers:
		jsonOptions |= NSJSONReadingMutableContainers;
	    }
	  format = NSPropertyListJSONFormat;
	  prop = [NSJSONSerialization JSONObjectWithData: data
						 options: jsonOptions
						   error: &myError];
	}
    }
  if (error != NULL)
    {
      *error = myError;
    }
  if (aFormat != NULL)
    {
      *aFormat = format;
    }
  return prop;
}

+ (void) load
{
  Method replacementRead;
  Method replacementWrite;

  replacementRead = class_getClassMethod(self,
    @selector(_ppropertyListWithData:options:format:error:));
  replacementWrite = class_getClassMethod(self,
    @selector(_pdataFromPropertyList:format:errorDescription:));

  originalRead = class_replaceMethod(object_getClass(self),
    @selector(propertyListWithData:options:format:error:),
    method_getImplementation(replacementRead),
    method_getTypeEncoding(replacementRead));
  originalWrite = class_replaceMethod(object_getClass(self),
    @selector(dataFromPropertyList:format:errorDescription:),
    method_getImplementation(replacementWrite),
    method_getTypeEncoding(replacementWrite));
}
@end
