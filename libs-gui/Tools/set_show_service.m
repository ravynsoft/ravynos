/* 
   set_show_service.m

   GNUstep utility to enable or disable a service for the current user.

   Copyright (C) 1998 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: November 1998
   
   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.

*/ 

#include <stdlib.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import "AppKit/NSApplication.h"

int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*proc;
  NSArray		*args;
  unsigned		index;

  // [NSObject enableDoubleReleaseCheck: YES];
#ifdef GS_PASS_ARGUMENTS
  [NSProcessInfo initializeWithArguments:argv count:argc environment:env];
#endif

  pool = [NSAutoreleasePool new];

  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"unable to get process information!\n");
      exit(EXIT_SUCCESS);
    }

  args = [proc arguments];

  for (index = 1; index < [args count]; index++)
    {
      if ([[args objectAtIndex: index] isEqual: @"--help"])
	{
	  printf(
"set_show_service enables or disables the display of a specified service\n"
"item.  It's should be in the form 'set_show_service --enable name' or \n"
"'set_show_service --disable name' where 'name' is a service name.\n");
	  exit(EXIT_SUCCESS);
	}
      if ([[args objectAtIndex: index] isEqual: @"--enable"])
	{
	  if (index >= [args count] - 1)
	    {
	      NSLog(@"No name specified for enable.\n");
	      exit(EXIT_FAILURE);
	    }
	  NSSetShowsServicesMenuItem([args objectAtIndex: ++index], YES);
	  exit(EXIT_SUCCESS);
	}
      if ([[args objectAtIndex: index] isEqual: @"--disable"])
	{
	  if (index >= [args count] - 1)
	    {
	      NSLog(@"No name specified for disable.\n");
	      exit(EXIT_FAILURE);
	    }
	  NSSetShowsServicesMenuItem([args objectAtIndex: ++index], NO);
	  exit(EXIT_SUCCESS);
	}
    }

  NSLog(@"Nothing to do.\n");
  [pool drain];
  return(1);
}

