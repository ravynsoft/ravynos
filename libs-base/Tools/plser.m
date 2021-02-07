/** This tool converts a text property list to a serialised representation.
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
#import	"Foundation/NSUserDefaults.h"


/** <p> This tool converts a text property list to a another serialised
 *  representation.
 * </p>
 */
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSUserDefaults	*defs;
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
      NSLog(@"plser: unable to get process information!\n");
      [pool release];
      exit(EXIT_SUCCESS);
    }

  defs = [NSUserDefaults standardUserDefaults];
  args = [proc arguments];

  if ([args count] <= 1
    || ([defs objectForKey: @"Format"] != nil && [args count] < 3))
    {
      GSPrintf(stderr, @"No file names given to serialize. Try --help\n");
    }
  else
    {
      NSString  *fmt = [defs stringForKey: @"Format"];

      for (i = 1; i < [args count]; i++)
	{
	  NSString	*file = [args objectAtIndex: i];

          if ([file isEqual: @"--help"])
            {
              GSPrintf(stdout,
                @"This program takes one or more property list files\n");
              GSPrintf(stdout,
                @"as input and reserialises them to stdout.\n");
              GSPrintf(stdout,
                @"The only permitted argument is -Format to\n");
              GSPrintf(stdout,
                @"specify the output format to use... one of:\n");
              GSPrintf(stdout,
                @"  NSPropertyListOpenStepFormat\n");
              GSPrintf(stdout,
                @"  NSPropertyListXMLFormat_v1_0\n");
              GSPrintf(stdout,
                @"  NSPropertyListBinaryFormat_v1_0\n");
              GSPrintf(stdout,
                @"  NSPropertyListGNUstepFormat\n");
              GSPrintf(stdout,
                @"  NSPropertyListGNUstepBinaryFormat\n");
              [pool release];
              exit(EXIT_SUCCESS);
            }
          if ([file isEqual: @"-Format"])
            {
              i++;
              continue;
            }
	  NS_DURING
	    {
	      NSData	                *myData;
	      id	                incoming;
              NSPropertyListFormat      inFormat;
              NSError                   *anError;

	      myData = [NSData dataWithContentsOfFile: file];
	      incoming = [NSPropertyListSerialization
                propertyListWithData: myData
                             options: NSPropertyListImmutable
                              format: &inFormat
                               error: &anError];
	      if (nil == incoming)
                {
                  GSPrintf(stderr, @"Loading '%@' - %@\n", file, anError);
                }
	      else
		{
		  NSFileHandle	        *out;
                  NSPropertyListFormat  outFormat;

                  outFormat = NSPropertyListGNUstepBinaryFormat;
                  if ([fmt isEqual: @"NSPropertyListOpenStepFormat"])
                    outFormat = NSPropertyListOpenStepFormat;
                  else if ([fmt isEqual: @"NSPropertyListXMLFormat_v1_0"])
                    outFormat = NSPropertyListXMLFormat_v1_0;
                  else if ([fmt isEqual: @"NSPropertyListBinaryFormat_v1_0"])
                    outFormat = NSPropertyListBinaryFormat_v1_0;
                  else if ([fmt isEqual: @"NSPropertyListGNUstepFormat"])
                    outFormat = NSPropertyListGNUstepFormat;
                  else if ([fmt isEqual: @"NSPropertyListGNUstepBinaryFormat"])
                    outFormat = NSPropertyListGNUstepBinaryFormat;

		  myData = [NSPropertyListSerialization
                    dataWithPropertyList: incoming
                                  format: outFormat
                                 options: 0
                                   error: &anError];
#if 0
/* Check serialisation/deserialisation gives original value.
 */
{
  id	                result;
  result = [NSPropertyListSerialization
    propertyListWithData: myData
                 options: NSPropertyListImmutable
                  format: 0
                   error: &anError];

  if (NO == [incoming isEqual: result])
    {
      NSLog(@"Lossy conversion");
    }
}
#endif
		  out = [NSFileHandle fileHandleWithStandardOutput];
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
