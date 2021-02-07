/* This tool opens the appropriate application from the command line
   based on what type of file is being accessed. 

   Copyright (C) 2001 Free Software Foundation, Inc.

   Written by:  Gregory Casamento <greg_casamento@yahoo.com>
   Created: November 2001

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

#include <Foundation/NSArray.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSTask.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSException.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSFileHandle.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSUserDefaults.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSWorkspace.h>

static NSString*
absolutePath(NSFileManager *fm, NSString *path)
{
  path = [path stringByStandardizingPath];
  if ([path isAbsolutePath] == NO)
    {
      path = [[fm currentDirectoryPath] stringByAppendingPathComponent: path];
    }
  return path;
}

int
main(int argc, char** argv, char **env_c)
{
  CREATE_AUTORELEASE_POOL(pool);
  NSEnumerator  *argEnumerator = nil;
  NSWorkspace   *workspace = nil;
  NSFileManager *fm = nil;
  NSString      *arg = nil;
  NSString      *editor = nil;
  NSString      *terminal = nil;
  NSString      *application = nil;
  NSString      *filetoopen = nil;
  NSString      *filetoprint = nil;
  NSString      *nxhost = nil;
  BOOL		isDir;
  BOOL		exists;
  NSURL		*u;

#ifdef GS_PASS_ARGUMENTS
  [NSProcessInfo initializeWithArguments:argv count:argc environment:env_c];
#endif
  argEnumerator = [[[NSProcessInfo processInfo] arguments] objectEnumerator];
  workspace = [NSWorkspace sharedWorkspace];
  fm = [NSFileManager defaultManager];
  
  // Default applications for opening unregistered file types....
  editor = [[NSUserDefaults standardUserDefaults]
    stringForKey: @"GSDefaultEditor"];
  terminal = [[NSUserDefaults standardUserDefaults]
    stringForKey: @"GSDefaultTerminal"];

  // Process options...
  application = [[NSUserDefaults standardUserDefaults] stringForKey: @"a"];
  filetoopen = [[NSUserDefaults standardUserDefaults] stringForKey: @"o"];
  filetoprint = [[NSUserDefaults standardUserDefaults] stringForKey: @"p"];
  nxhost = [[NSUserDefaults standardUserDefaults] stringForKey: @"NXHost"];

  if (application)
    {
      /* Start the application now,
       * later on we will use it for file opening. 
       */
      [workspace launchApplication: application];
    }
  
  if (filetoopen)
    {
      exists = [fm fileExistsAtPath: arg isDirectory: &isDir];
      if (exists == NO)
	{
	  if ([filetoopen hasPrefix: @"/"] == NO
	    && (u = [NSURL URLWithString: filetoopen]) != nil)
	    {
	      [workspace openURL: u];
	    }
	}
      else
	{
	  filetoopen = absolutePath(fm, filetoopen);
	  [workspace openFile: filetoopen
	      withApplication: application];
	}
    }

  if (filetoprint)
    {
      filetoprint = absolutePath(fm, filetoprint);
      puts("Not implemented");
    }

  if (nxhost)
    {
      puts("Not implemented");
    }

  if (argc == 1)
    {
      NSFileHandle	*fh = [NSFileHandle fileHandleWithStandardInput];
      NSData		*data = [fh readDataToEndOfFile];
      NSString		*tempFile;
      int		processId;

      tempFile = NSTemporaryDirectory();
      tempFile = [tempFile stringByAppendingPathComponent: @"openfiletmp"];
      processId = [[NSProcessInfo processInfo] processIdentifier];
      tempFile = [tempFile stringByAppendingFormat: @"%d", processId];
      tempFile = [tempFile stringByAppendingString: @".txt"];
      [data writeToFile: tempFile atomically: YES];
      [workspace openFile: tempFile withApplication: editor];      
    }

  [argEnumerator nextObject]; // skip the first element, which is empty.
  while ((arg = [argEnumerator nextObject]) != nil)
    {
      NSString *ext = [arg pathExtension];
      
      if ([arg isEqualToString: @"-a"]
        || [arg isEqualToString: @"-o"]
        || [arg isEqualToString: @"-NXHost"])
	{
	  // skip since this is handled above...
	  arg = [argEnumerator nextObject];
	}
      else // no option specified
	{
	  NS_DURING
	    {
	      exists = [fm fileExistsAtPath: arg isDirectory: &isDir];
	      if (exists == YES)
		{
		  arg = absolutePath(fm, arg);
		  if (isDir == NO && [fm isExecutableFileAtPath: arg])
		    {
		      [workspace openFile: arg withApplication: terminal];
		    }
		  else // no argument specified
		    {
		      // First check to see if it's an application
		      if ([ext isEqualToString: @"app"]
			|| [ext isEqualToString: @"debug"]
			|| [ext isEqualToString: @"profile"])
			{
			  [workspace launchApplication: arg];
			}
		      else
			{
			  if (![workspace openFile: arg
				   withApplication: application])
			    {
			      // no recognized extension,
			      // run application indicated by environment var.
			      NSLog(@"Opening %@ with %@",arg,editor);
			      [workspace openFile: arg withApplication: editor];
			    }
			}
		    }
		}
	      else if ([arg hasPrefix: @"/"] == NO
		&& (u = [NSURL URLWithString: arg]) != nil
		&& [u scheme] != nil)
		{
		  [workspace openURL: u];
		}
	      else 
		{
		  arg = absolutePath(fm, arg);
		  GSPrintf(stdout, @"The file %@ does not exist.\n", arg);
		}
	    }
	  NS_HANDLER
	    {
	      NSLog(@"Exception while attempting open file %@ - %@: %@",
		    arg, [localException name], [localException reason]);
	    }
	  NS_ENDHANDLER
	}
    }
  RELEASE(pool);
  exit(EXIT_SUCCESS);
}
