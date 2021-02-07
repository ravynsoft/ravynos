/* Implementation of extension methods for base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/
#import "common.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSSet.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSTask+GNUstepBase.h"

@implementation	NSTask (GNUstepBase)

+ (NSSet*) executableExtensions
{
  static NSSet  *executable = nil;

#if defined(_WIN32)
  if (nil == executable)
    {
      NSMutableSet      *m;
      NSEnumerator      *e;
      NSString          *s;

      /* Get PATHEXT environment variable and split apart on ';'
       */
      e = [[[[[NSProcessInfo processInfo] environment]
        objectForKey: @"PATHEXT"]
        componentsSeparatedByString: @";"] objectEnumerator];

      m = [NSMutableSet set];
      while (nil != (s = [e nextObject]))
        {
          /* We don't have a '.' in a file extension, but the
           * environment variable probably does ... fix it.
           */
          s = [s stringByTrimmingSpaces];
          if ([s hasPrefix: @"."])
            {
              s = [s substringFromIndex: 1];
            }
          if ([s length] > 0)
            {
              [m addObject: s];
            }
        }
      /* Make sure we at least have the EXE extension.
       */
      [m addObject: @"EXE"];
      ASSIGNCOPY(executable, m);
    }
#endif
  return executable;
}

static	NSString*
executablePath(NSFileManager *mgr, NSString *path)
{
#if defined(_WIN32)
  NSString	*tmp = [path pathExtension];

  if ([tmp length] == 0)
    {
      NSSet     	*exts;
      NSString		*ext;
      NSEnumerator	*e;

      exts = [NSTask executableExtensions];
      e = [exts objectEnumerator];
      ext = @"EXE";

      /* Try 'EXE' first, but otherwise iterate through all available
       * extensions to find an executable path.
       */
      do
        {
          tmp = [path stringByAppendingPathExtension: ext];
          if ([mgr isExecutableFileAtPath: tmp])
            {
              return tmp;
            }
        }
      while (nil != (ext = [e nextObject]));
    }
  else
    {
      if ([mgr isExecutableFileAtPath: path])
        {
          return path;
        }
    }
#else
  if ([mgr isExecutableFileAtPath: path])
    {
      return path;
    }
#endif
  return nil;
}

+ (NSString*) executablePath: (NSString*)aPath
{
  return executablePath([NSFileManager defaultManager], aPath);
}

+ (NSString*) launchPathForTool: (NSString*)name
{
  NSEnumerator	*enumerator;
  NSDictionary	*env;
  NSString	*pathlist;
  NSString	*path;
  NSFileManager	*mgr;

  mgr = [NSFileManager defaultManager];

#if	defined(GNUSTEP)
  enumerator = [NSSearchPathForDirectoriesInDomains(
    GSToolsDirectory, NSAllDomainsMask, YES) objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      path = [path stringByAppendingPathComponent: name];
      if ((path = executablePath(mgr, path)) != nil)
	{
	  return path;
	}
    }
  enumerator = [NSSearchPathForDirectoriesInDomains(
    GSAdminToolsDirectory, NSAllDomainsMask, YES) objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      path = [path stringByAppendingPathComponent: name];
      if ((path = executablePath(mgr, path)) != nil)
	{
	  return path;
	}
    }
#endif

  env = [[NSProcessInfo processInfo] environment];
  pathlist = [env objectForKey:@"PATH"];
#if defined(_WIN32)
/* Windows 2000 and perhaps others have "Path" not "PATH" */
  if (pathlist == nil)
    {
      pathlist = [env objectForKey: @"Path"];
    }
  enumerator = [[pathlist componentsSeparatedByString: @";"] objectEnumerator];
#else
  enumerator = [[pathlist componentsSeparatedByString: @":"] objectEnumerator];
#endif
  while ((path = [enumerator nextObject]) != nil)
    {
      path = [path stringByAppendingPathComponent: name];
      if ((path = executablePath(mgr, path)) != nil)
	{
	  return path;
	}
    }
  return nil;
}
@end
