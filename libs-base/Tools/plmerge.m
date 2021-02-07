/** This tool merges text property lists into a single property list.
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Jonathan Gapen  <jagapen@whitewater.chem.wisc.edu>
   Created: April 2000

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

#import	"common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSFileManager.h"
#import	"Foundation/NSProcessInfo.h"
#import "GNUstepBase/Additions.h"


/** <p> This tool merges text property lists into a single property list.
 </p> */
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*procinfo;
  NSArray		*args;
  NSString		*destName;
  NSString		*fileContents;
  NSMutableDictionary	*plist = nil;
  unsigned		i;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
  pool = [NSAutoreleasePool new];
  procinfo = [NSProcessInfo processInfo];
  if (procinfo == nil)
    {
      NSLog(@"plmerge: unable to get process information!");
      [pool release];
      exit(EXIT_SUCCESS);
    }

  args = [procinfo arguments];

  if ([args count] < 3)
    {
      GSPrintf(stderr, @"Usage: %@ [destination-file] [input-file ...]\n",
	[procinfo processName]);
      [pool release];
      exit(EXIT_SUCCESS);
    }

  destName = [args objectAtIndex: 1];
  if ([[NSFileManager defaultManager] fileExistsAtPath: destName])
    {
      NS_DURING
        {
          fileContents = [NSString stringWithContentsOfFile: destName];
          plist = [fileContents propertyList];
        }
      NS_HANDLER
        {
          GSPrintf(stderr, @"Parsing '%@' - %@\n", destName,
	    [localException reason]);
        }
      NS_ENDHANDLER

      if ((plist == nil) || ![plist isKindOfClass: [NSDictionary class]])
        {
          GSPrintf(stderr,
	    @"The destination property list must contain an NSDictionary.\n");
          [pool release];
          exit(EXIT_FAILURE);
        }
      plist = [plist mutableCopy];
    }
  else
    {
      plist = [NSMutableDictionary new];
    }

  for (i = 2; i < [args count]; i++)
    {
      NSString		*filename = [args objectAtIndex: i];
      NSString		*key = filename;
      id		object = nil;

      NS_DURING
        {
          fileContents = [NSString stringWithContentsOfFile: filename];
          object = [fileContents propertyList];
        }
      NS_HANDLER
        {
          GSPrintf(stderr, @"Parsing '%@' - %@\n", filename,
	    [localException reason]);
        }
      NS_ENDHANDLER

      if ([[filename pathExtension] isEqualToString: @"plist"])
	{
	  key = [filename stringByDeletingPathExtension];
	}

      if (object == nil)
        GSPrintf(stderr, @"Parsing '%@' - nil property list\n", filename);
      else if ([object isKindOfClass: [NSArray class]] == YES)
        [plist setObject: object forKey: key];
      else if ([object isKindOfClass: [NSData class]] == YES)
        [plist setObject: object forKey: key];
      else if ([object isKindOfClass: [NSDictionary class]] == YES)
        [plist addEntriesFromDictionary: object];
      else if ([object isKindOfClass: [NSString class]] == YES)
        [plist setObject: object forKey: key];
      else
        GSPrintf(stderr, @"Parsing '%@' - unexpected class - %@\n",
                filename, [[object class] description]);
    }

  if ([plist writeToFile: destName atomically: YES] == NO)
    GSPrintf(stderr, @"Error writing property list to '%@'\n", destName);

  RELEASE(plist);
  [pool release];
  exit(EXIT_SUCCESS);
}
