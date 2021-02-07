/** This utility provides path/directory layout information for GNUstep.
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: July 2005

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
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSPathUtilities.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"


/**
 <p>The 'gspath' utility prints out various items of path/directory
 information (one item at a time).<br />
 The program always takes a single argument ... selecting the information
 to be printed.</p>
 The arguments and their meanings are -<br />
 <deflist>
   <term>defaults</term>
   <desc>The GNUstep defaults directory of the current user</desc>
   <term>devpath</term>
   <desc>A path specification which may be used to add the root(s) of
     the GNUstep development environment on the current system.</desc>
   <term>libpath</term>
   <desc>A path specification which may be used to add all the standard
     GNUstep directories where dynamic libraries are normally stored.<br />
     you might do 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`gspath libpath`' to make
     use of this.</desc>
   <term>path</term>
   <desc>A path specification which may be used to add all the standard
     GNUstep directories where command-line programs are normally stored.<br />
     you might do 'PATH=$PATH:`gspath path`' to make use of this.</desc>
   <term>user</term>
   <desc>The GNUstep home directory of the current user</desc>
 </deflist>
 */
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*proc;
  NSArray		*args;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
  pool = [NSAutoreleasePool new];
  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      GSPrintf(stderr, @"gspath: unable to get process information!\n");
      [pool drain];
      return 1;
    }

  args = [proc arguments];

  if ([args count] == 2)
    {
      BOOL	ok = YES;
      NSString	*name = [[args objectAtIndex: 1] lowercaseString];
      NSString	*sep;

#ifdef	__MINGW__
      sep = @";";
#else
      sep = @":";
#endif

      if ([name isEqualToString: @"defaults"] == YES)
	{
	  GSPrintf(stdout, @"%@", GSDefaultsRootForUser(nil));
	}
      else if ([name isEqualToString: @"path"] == YES)
	{
	  NSArray	*directories;
	  NSString	*path;

	  directories = NSSearchPathForDirectoriesInDomains
	    (GSToolsDirectory, NSAllDomainsMask, YES);
	  path = [directories componentsJoinedByString: sep];
	  GSPrintf(stdout, @"%@", path);
	}
      else if ([name isEqualToString: @"devpath"] == YES)
	{
	  NSArray	*directories;
	  NSString	*path;

	  directories = NSSearchPathForDirectoriesInDomains
	    (NSDeveloperDirectory, NSAllDomainsMask, YES);
	  path = [directories componentsJoinedByString: sep];
	  GSPrintf(stdout, @"%@", path);
	}
      else if ([name isEqualToString: @"libpath"] == YES)
	{
	  NSArray	*directories;
	  NSString	*path;

	  directories = NSSearchPathForDirectoriesInDomains
	    (GSLibrariesDirectory, NSAllDomainsMask, YES);
	  path = [directories componentsJoinedByString: sep];
	  GSPrintf(stdout, @"%@", path);
	}
      else if ([name isEqualToString: @"user"] == YES)
	{
	  GSPrintf(stdout, @"%@", NSHomeDirectory());
	}
      else
	{
	  ok = NO;	// Unrecognised option
	}
      if (ok == YES)
	{
	  [pool drain];
	  return 0;
	}
    }

  GSPrintf(stderr,
@"The 'gspath' utility prints out various items of path/directory\n"
@"information (one item at a time).\n"
@"The program always takes a single argument ... selecting the information\n"
@"to be printed.\n\n"
@"The arguments and their meanings are -\n\n"
@"defaults\n"
@"  The GNUstep defaults directory of the current user.\n\n"
@"devpath\n"
@"  A path specification which may be used to add the root(s) of\n"
@"  the GNUstep development environment on the current system.\n\n"
@"libpath\n"
@"  A path specification which may be used to add all the standard GNUstep\n"
@"  directories where dynamic libraries are normally stored.\n\n"
@"  you might do 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`gspath libpath`' to make\n"
@"  use of this.\n\n"
@"path\n"
@"  A path specification which may be used to add all the standard GNUstep\n"
@"  directories where command-line programs are normally stored.\n"
@"  you might do 'PATH=$PATH:`gspath path`' to make use of this.\n\n"
@"user\n"
@"  The GNUstep home directory of the current user\n\n"
);
  [pool drain];
  return 1;
}
