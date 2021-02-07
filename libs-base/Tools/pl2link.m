/*
   This tool produces a desktop link file for KDE and Gnome out of a GNUstep
   property list.
   Copyright (C) 2001, 2011 Free Software Foundation, Inc.

   Written by:  Fred Kiefer <FredKiefer@gmx.de>
   Created: December 2001

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

#import <stdlib.h>

#import	"common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSFileManager.h"
#import	"Foundation/NSProcessInfo.h"
#import "Foundation/NSPathUtilities.h"

#ifdef _MSC_VER
#define popen _popen
#endif

int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSProcessInfo		*procinfo;
  NSArray		*args;
  NSString		*sourceName;
  NSString		*destName;
  NSMutableString	*fileContents;
  NSDictionary	        *plist = nil;
  NSArray		*list;
  NSString		*entry;
  NSString              *installPath = @"";
  NSString              *appName = @"";

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

  if ([args count] < 2)
    {
      GSPrintf(stderr, @"Usage: %@ input-file [destination-file]\n",
	[procinfo processName]);
      [pool release];
      exit(EXIT_SUCCESS);
    }

  sourceName = [args objectAtIndex: 1];
  if ([args count] > 2)
    {
      destName = [args objectAtIndex: 2];
    }
  else
    {
      /* Filled in later */
      destName = nil;
    }
  NS_DURING
    {
      fileContents = [NSString stringWithContentsOfFile: sourceName];
      plist = [fileContents propertyList];
    }
  NS_HANDLER
    {
      GSPrintf(stderr, @"Parsing '%@' - %@\n", sourceName,
	[localException reason]);
    }
  NS_ENDHANDLER

  if ((plist == nil) || ![plist isKindOfClass: [NSDictionary class]])
    {
      GSPrintf(stderr,
	@"The source property list must contain an NSDictionary.\n");
      [pool release];
      exit(EXIT_FAILURE);
    }

  fileContents = [NSMutableString stringWithCapacity: 200];
  [fileContents appendString:
    @"[Desktop Entry]\nType=Application\n"];
  list = [plist objectForKey: @"FreeDesktopCategories"];
  if (list != nil && [list isKindOfClass: [NSArray class]] && [list count] > 0)
    {
      [fileContents appendString: @"Categories="];
      [fileContents appendString: [list componentsJoinedByString: @";"]];
      [fileContents appendString: @";\n"];
    }
  else
    {
      [fileContents appendString:
                      @"Categories=X-GNUstep;\n"];
    }
  entry = [plist objectForKey: @"ApplicationName"];
  if (entry != nil)
    {
      appName = entry;
      [fileContents appendFormat: @"Name=%@\n", entry];
      [fileContents appendFormat: @"StartupWMClass=%@\n", entry];
      if (destName == nil)
	destName = [entry stringByAppendingString: @".desktop"];
    }
  entry = [plist objectForKey: @"ApplicationDescription"];
  if (entry != nil)
    {
      [fileContents appendFormat: @"Comment=%@\n", entry];
    }

  /* Try to guess where the application will be installed.  PS: At the
     moment, this is only required for NSIcon, so I suppose we could
     skip it if there is no NSIcon.  */
  {
    /* The default installation domain is the local domain.  Assume
       that's the case unless something is specified.  */
    NSSearchPathDomainMask domain = NSLocalDomainMask;
    NSString *installDomain;
    NSArray *installPaths;

    installDomain = [[procinfo environment] objectForKey: @"GNUSTEP_INSTALLATION_DOMAIN"];
    if(installDomain != nil)
      {
	if ([installDomain isEqualToString: @"SYSTEM"])
	  {
	    domain = NSSystemDomainMask;
	  }
	else if ([installDomain isEqualToString: @"NETWORK"])
	  {
	    domain = NSNetworkDomainMask;
	  }
	else if ([installDomain isEqualToString: @"USER"])
	  {
	    domain = NSUserDomainMask;
	  }
	
	/* In all other cases, we leave domain == NSLocalDomainMask.  */
      }

    installPaths = NSSearchPathForDirectoriesInDomains (NSApplicationDirectory, domain, YES);
    if ([installPaths count] > 0)
      {
	installPath = [installPaths objectAtIndex: 0];
      }
    else
      {
	/* Ahm - this should never happen.  */
	GSPrintf(stderr, @"Error determining the application installation location\n");

	/* Try to get any application installation path.  */
	installPaths = NSSearchPathForDirectoriesInDomains (NSApplicationDirectory, NSAllDomainsMask, YES);
	if ([installPaths count] > 0)
	  {
	    installPath = [installPaths objectAtIndex: 0];
	    GSPrintf(stderr, @"Assuming it will be installed into '%@' (this may be wrong!)\n", installPath);
	  }
	else
	  {
	    [pool release];
	    exit(EXIT_FAILURE);
	  }
      }
  }

  entry = [plist objectForKey: @"NSIcon"];
  if (entry != nil)
    {
      NSString *iconPath = [[[[installPath stringByAppendingPathComponent:appName] 
			       stringByAppendingPathExtension:@"app"] 
			      stringByAppendingPathComponent:@"Resources"]
			     stringByAppendingPathComponent:entry];

      if ([[iconPath pathExtension] isEqualToString: @""])
	{
	  [fileContents appendFormat: @"Icon=%@.tiff\n", iconPath];
	}
      else
	{
	  [fileContents appendFormat: @"Icon=%@\n", iconPath];
	}
    }
  entry = [plist objectForKey: @"NSExecutable"];
  if (entry != nil)
    {
      FILE *fp;
      char line[130];
      NSString *execPath = nil;
      int l = 0;

      fp = popen("which openapp","r");
      fgets(line,sizeof line,fp);
      l = strlen(line);
      line[l-1] = '\0';

      // Build the string to execute the application...
      execPath = [NSString stringWithCString: line
			   encoding: NSASCIIStringEncoding];
      [fileContents appendFormat: @"Exec=%@ %@\n", execPath, entry];
    }

  list = [plist objectForKey: @"NSTypes"];
  if (list != nil)
    {
      if([list count] > 0)
	{  
	  unsigned int i;
      
	  [fileContents appendString: @"MimeType="];
	  for (i = 0; i < [list count]; i++)
	    {
	      NSArray *types;
	      unsigned int j;
	      
	      plist = [list objectAtIndex: i];
	      types = [plist objectForKey: @"NSMIMETypes"];
	      if (types != nil)
		{
		  for (j = 0; j < [types count]; j++)
		    {
		      entry = [types objectAtIndex: j];
		      [fileContents appendFormat: @"%@;", entry];
		    }
		}
	    }
	  [fileContents appendString: @"\n"];
	}
    }

  if ([[fileContents dataUsingEncoding: NSUTF8StringEncoding]
    writeToFile: destName atomically: YES] == NO)
    {
      GSPrintf(stderr, @"Error writing property list to '%@'\n", destName);
    }
  [pool release];
  exit(EXIT_SUCCESS);
}
