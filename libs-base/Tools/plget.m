/** This tool extracts a string value from a dictionary in a property list.
   Copyright (C) 1999 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: may 1999

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYINGv3.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   */

#import "common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"
#import	"Foundation/NSFileHandle.h"
#import	"Foundation/NSAutoreleasePool.h"


/** <p> This tool extracts a string value from a dictionary in a property
    list representation.<br />
    It takes one or more argument (the key to be extracted).<br />
    It expects to read the property list from STDIN.<br />
    It writes the string value (if any) on STDOUT<br />
    Where multiple keys are specified, they are used to extract nested
    values from dictionaries within the outermost dictionary.<br />
    Where the resulting object exists and is not a string,
    its description is written to STDOUT.
 </p> */
int
main(int argc, char** argv, char **env)
{
  CREATE_AUTORELEASE_POOL(pool);
  NSProcessInfo		*proc;
  NSArray		*args;
  int			status = EXIT_SUCCESS;
  int			count;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"plget: unable to get process information.");
      RELEASE(pool);
      exit(EXIT_FAILURE);
    }

  args = [proc arguments];

  if ((count = [args count]) <= 1)
    {
      NSLog(@"plget: no key given to get.");
      RELEASE(pool);
      exit(EXIT_FAILURE);
    }
  else
    {
      NSFileHandle	*fileHandle;
      NSData		*inputData;
      NSString		*inputString;
      NSData		*outputData;

      NS_DURING
	{
	  int	i = 1;
	  id	value;

	  fileHandle = [NSFileHandle fileHandleWithStandardInput];
	  inputData = [fileHandle readDataToEndOfFile];
	  inputString = [[NSString alloc] initWithData: inputData
					      encoding: NSUTF8StringEncoding];
	  if (inputString == nil)
	    {
	      inputString = [[NSString alloc] initWithData: inputData
		encoding: [NSString defaultCStringEncoding]];
	    }
	  value = [inputString propertyList];
	  RELEASE(inputString);
	  while (i < count-1)
	    {
	      value = [(NSDictionary*)value objectForKey:
		[args objectAtIndex: i++]];
	    }
	  value = [(NSDictionary*)value objectForKey: [args objectAtIndex: i]];
	  if ([value isKindOfClass: [NSString class]] == NO)
	    {
	      value = [value description];
	    }
	  outputData = [value dataUsingEncoding:
	    [NSString defaultCStringEncoding]];
	  if ([outputData length] > 0)
	    {
	      fileHandle = [NSFileHandle fileHandleWithStandardOutput];
	      [fileHandle writeData: outputData];
	    }
	}
      NS_HANDLER
	{
	  NSLog(@"Problem: %@", localException);
	  status = EXIT_FAILURE;
	}
      NS_ENDHANDLER
    }
  RELEASE(pool);
  return status;
}
