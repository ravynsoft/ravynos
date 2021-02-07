/** This tool checks that a file contains a valid text property-list.
   Copyright (C) 1999 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: February 1999

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
#import	"Foundation/NSCharacterSet.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"
#import	"Foundation/NSAutoreleasePool.h"

/*
 * If there is any non-ascii characrer in the string,
 * and the file data did not begin with a unicode BOM to identify
 * it as unicode data, we return the location of the first
 * bad character, otherwise return -1;
 */
static int
firstBadCharacter(NSString *file, NSString *content)
{
  static NSCharacterSet	*cs = nil;
  NSData		*d;
  NSRange		r;

  if (cs == nil)
    {
      cs = [NSCharacterSet characterSetWithRange: NSMakeRange(1, 127)];
      cs = RETAIN([cs invertedSet]);
    }

  r = [content rangeOfCharacterFromSet: cs];
  if (r.length == 0)
    {
      return -1;
    }
  d = [NSData dataWithContentsOfFile: file];
  if ([d length] > 2)
    {
      const unsigned char	*ptr = (const unsigned char*)[d bytes];

      if ((ptr[0] == 0xff && ptr[1] == 0xfe)			// UCS2
	|| (ptr[0] == 0xfe && ptr[1] == 0xff)			// UCS2
	|| (ptr[0] == 0xef && ptr[1] == 0xbb && ptr[2] == 0xbf)) // UTF8
	{
	  return -1;
	}
    }
  return r.location;
}

/** <p>
    This tool checks that a file contains a valid text property-list.
 </p> */
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*proc;
  NSArray		*args;
  unsigned		i;
  int			retval = 0;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
  pool = [NSAutoreleasePool new];
  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"plparse: unable to get process information!\n");
      [pool release];
      exit(EXIT_FAILURE);
    }

  args = [proc arguments];

  if ([args count] <= 1)
    {
      GSPrintf(stderr, @"No file names given to parse.\n");
    }
  else
    {
      for (i = 1; i < [args count]; i++)
	{
	  NSString	*file = [args objectAtIndex: i];

	  NS_DURING
	    {
	      NSString	*myString;
	      id	result;
	      int	bad;

	      myString = [NSString stringWithContentsOfFile: file];
	      if (myString == nil)
		GSPrintf(stderr, @"Parsing '%@' - not valid string\n", file);
	      else if ((bad = firstBadCharacter(file, myString)) >= 0)
		GSPrintf(stderr, @"Parsing '%@' - bad char '\\U%04x' at %d\n",
		  file, [myString characterAtIndex: bad], bad);
	      else if ((result = [myString propertyList]) == nil)
		GSPrintf(stderr, @"Parsing '%@' - nil property list\n", file);
	      else if ([result isKindOfClass: [NSDictionary class]] == YES)
		GSPrintf(stderr, @"Parsing '%@' - a dictionary\n", file);
	      else if ([result isKindOfClass: [NSArray class]] == YES)
		GSPrintf(stderr, @"Parsing '%@' - an array\n", file);
	      else if ([result isKindOfClass: [NSData class]] == YES)
		GSPrintf(stderr, @"Parsing '%@' - a data object\n", file);
	      else if ([result isKindOfClass: [NSString class]] == YES)
		GSPrintf(stderr, @"Parsing '%@' - a string\n", file);
	      else
		GSPrintf(stderr, @"Parsing '%@' - unexpected class - %@\n",
		  file, [[result class] description]);
	    }
	  NS_HANDLER
	    {
	      GSPrintf(stderr, @"Parsing '%@' - %@\n", file,
		[localException reason]);
	      retval = 1;
	    }
	  NS_ENDHANDLER
	}
    }
  [pool release];
  return retval;
}
