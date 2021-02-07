/** This tool converts a serialised property list to a text representation.
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
#import	"Foundation/NSException.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"
#import	"Foundation/NSFileHandle.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSPropertyList.h"


/** <p>This tool converts a serialised property list to a text
 *  representation.
 * </p>
 */
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*proc;
  NSArray		*args;
  unsigned		i;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
  pool = [NSAutoreleasePool new];
  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"pldes: unable to get process information!\n");
      [pool release];
      exit(EXIT_SUCCESS);
    }

  args = [proc arguments];

  if ([args count] <= 1)
    {
      GSPrintf(stderr, @"No file names given to deserialize.\n");
    }
  else
    {
      NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
      NSDictionary	*locale = [defs dictionaryRepresentation];

      for (i = 1; i < [args count]; i++)
	{
	  NSString	*file = [args objectAtIndex: i];

	  NS_DURING
	    {
	      NSData	                *myData;
	      NSString	                *myString;
	      id	                result;
              NSPropertyListFormat      aFormat;
              NSError                   *anError;

	      myData = [NSData dataWithContentsOfFile: file];
	      result = [NSPropertyListSerialization
                propertyListWithData: myData
                             options: NSPropertyListImmutable
                              format: &aFormat
                               error: &anError];
	      if (result == nil)
                {
                  GSPrintf(stderr, @"Loading '%@' - %@\n", file, anError);
                }
	      else
		{
		  NSFileHandle	*out;

		  myString = [result descriptionWithLocale: locale indent: 0];
		  out = [NSFileHandle fileHandleWithStandardOutput];
		  myData = [myString dataUsingEncoding: NSASCIIStringEncoding];
		  [out writeData: myData];
		  [out synchronizeFile];
		}
	    }
	  NS_HANDLER
	    {
	      GSPrintf(stderr, @"Loading '%@' - %@\n", file,
		[localException reason]);
	    }
	  NS_ENDHANDLER
	}
    }
  [pool release];
  return 0;
}
