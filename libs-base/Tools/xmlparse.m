/** This tool parses and validates xml documents.

   <title>xmlparse ... a tool to parse xml documents</title>
   Copyright (C) 2003 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: May 2003

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

#include <stdio.h>

#import "common.h"
#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSPathUtilities.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"
#import "GNUstepBase/Additions.h"

@interface GSXMLParse : GSXMLParser 
+ (NSString*) loadEntity: (NSString*)publicId
                      at: (NSString*)location;
@end
@implementation	GSXMLParse : GSXMLParser 
+ (NSString*) loadEntity: (NSString*)publicId
                      at: (NSString*)location
{
  char		buf[BUFSIZ];
  NSString	*str;
  int		len;

  GSPrintf(stdout, @"Enter filename to load entity '%@' at '%@': ",
    publicId, location);
  fgets(buf, sizeof(buf)-1, stdin);
  buf[sizeof(buf)-1] = '\0';
  len = strlen(buf);
  // Strip trailing space
  while (len > 0 && buf[len-1] <= ' ')
    {
      buf[--len] = '\0';
    }
  str = [NSString stringWithUTF8String: buf];
  return str;
}
@end

/** <p>This tool error-checks and validates xml documents.  The parse
    is simply discarded after checking.
</p> */
int
main(int argc, char **argv, char **env)
{
  NSProcessInfo		*proc;
  NSArray		*files;
  unsigned int		count;
  unsigned int		i;
  CREATE_AUTORELEASE_POOL(pool);

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif

#ifndef HAVE_LIBXML
  NSLog(@"ERROR: The GNUstep Base Library was built\n"
@"        without an available libxml library. xmlparse needs the libxml\n"
@"        library to function. Aborting");
  exit(EXIT_FAILURE);
#endif

  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"unable to get process information!");
      exit(EXIT_FAILURE);
    }

  files = [proc arguments];
  count = [files count];
  for (i = 1; i < count; i++)
    {
      NSString		*file = [files objectAtIndex: i];
      GSXMLNode		*root;
      GSXMLParser	*parser;

      parser = [GSXMLParse parserWithContentsOfFile: file];
      [parser substituteEntities: NO];
      [parser doValidityChecking: YES];
      [parser keepBlanks: NO];
      [parser saveMessages: YES];
      if ([parser parse] == NO)
	{
	  NSLog(@"WARNING %@ is not a valid document", file);
	  NSLog(@"Errors: %@", [parser messages]);
	}
      root = [[parser document] root];
      NSLog(@"Document is %@", [root name]);
    }
  RELEASE(pool);
  return 0;
}
