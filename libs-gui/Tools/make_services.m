/* This tool builds a cache of service specifications like the
   NeXTstep/ OPENSTEP 'make_services' tool.  In addition it builds a list of
   applications and services-bundles found in the standard directories.

   Copyright (C) 1998-2013 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: November 1998

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
#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDistributedLock.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSSerialization.h>

static void scanApplications(NSMutableDictionary *services, NSString *path);
static void scanServices(NSMutableDictionary *services, NSString *path);
static void scanDynamic(NSMutableDictionary *services, NSString *path);
static NSMutableArray *validateEntry(id svcs, NSString* path);
static NSMutableDictionary *validateService(NSDictionary *service, NSString* path, unsigned i);

static NSString		*appsName = @".GNUstepAppList";
static NSString		*cacheName = @".GNUstepServices";

static	int verbose = 1;
static	NSMutableDictionary	*serviceMap;
static	NSMutableArray		*filterList;
static	NSMutableSet		*filterSet;
static	NSMutableDictionary	*printMap;
static	NSMutableDictionary	*spellMap;
static	NSMutableDictionary	*applicationMap;
static	NSMutableDictionary	*extensionsMap;
static	NSMutableDictionary	*schemesMap;

static Class aClass;
static Class dClass;
static Class sClass;

static BOOL CheckDirectory(NSString *path, NSError **error)
{
  NSFileManager	*mgr;
  BOOL		isDir;

  mgr = [NSFileManager defaultManager];
  if ([mgr fileExistsAtPath: path isDirectory: &isDir] && isDir)
    {
      return YES;
    }
  else
    {
      return [mgr createDirectoryAtPath: path
            withIntermediateDirectories: YES
                             attributes: nil
                                  error: error];
    }
}

int
main(int argc, char** argv, char **env_c)
{
  NSAutoreleasePool	*pool;
  NSData		*data;
  NSProcessInfo		*proc;
  NSFileManager		*mgr;
  NSMutableDictionary	*services;
  NSArray		*args;
  NSString		*usrRoot;
  NSString		*str;
  unsigned		index;
  NSMutableDictionary	*fullMap;
  NSDictionary		*oldMap;
  NSEnumerator		*enumerator;
  NSString		*path;
  NSError *error;

#ifdef GS_PASS_ARGUMENTS
  [NSProcessInfo initializeWithArguments:argv count:argc environment:env_c];
#endif
  pool = [NSAutoreleasePool new];

  mgr = [NSFileManager defaultManager];

  aClass = [NSArray class];
  dClass = [NSDictionary class];
  sClass = [NSString class];

  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      NSLog(@"unable to get process information!");
      exit(EXIT_SUCCESS);
    }

  [NSSerializer shouldBeCompact: YES];

  serviceMap = [NSMutableDictionary dictionaryWithCapacity: 64];
  filterList = [NSMutableArray arrayWithCapacity: 16];
  filterSet = [NSMutableSet setWithCapacity: 64];
  printMap = [NSMutableDictionary dictionaryWithCapacity: 8];
  spellMap = [NSMutableDictionary dictionaryWithCapacity: 8];
  applicationMap = [NSMutableDictionary dictionaryWithCapacity: 64];
  extensionsMap = [NSMutableDictionary dictionaryWithCapacity: 64];
  schemesMap = [NSMutableDictionary dictionaryWithCapacity: 64];

  args = [proc arguments];

  for (index = 1; index < [args count]; index++)
    {
      if ([[args objectAtIndex: index] isEqual: @"--verbose"])
	{
	  verbose++;
	}
      if ([[args objectAtIndex: index] isEqual: @"--quiet"])
	{
	  verbose--;
	}
      if ([[args objectAtIndex: index] isEqual: @"--help"])
	{
	  printf(
"make_services builds a validated cache of service information for use by\n"
"programs that want to use the OpenStep services facility.\n"
"This cache is stored in '%s' in the users GNUstep directory.\n"
"\n"
"You may use 'make_services --test filename' to test that the property list\n"
"in 'filename' contains a valid services definition.\n"
"You may use 'make_services --verbose' to produce descriptive output.\n"
"or --quiet to suppress any output (not recommended)\n",
[cacheName cString]);
	  exit(EXIT_SUCCESS);
	}
      if ([[args objectAtIndex: index] isEqual: @"--test"])
	{
	  while (++index < [args count])
	    {
	      NSString		*file = [args objectAtIndex: index];
	      NSDictionary	*info;

	      info = [NSDictionary dictionaryWithContentsOfFile: file];
	      if (info)
		{
		  id	svcs = [info objectForKey: @"NSServices"];

		  if (svcs)
		    {
		      validateEntry(svcs, file);
		    }
		  else if (verbose > 0)
		    {
		      NSLog(@"bad info - %@", file);
		    }
		}
	      else if (verbose > 0)
		{
		  NSLog(@"bad info - %@", file);
		}
	    }
	  exit(EXIT_SUCCESS);
	}
    }

  services = [NSMutableDictionary dictionaryWithCapacity: 200];

  /*
   *	Make sure that the users 'Services' directory exists.
   */
  usrRoot = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
    NSUserDomainMask, YES) lastObject];
  usrRoot = [usrRoot stringByAppendingPathComponent: @"Services"];
  if (!CheckDirectory(usrRoot, &error))
    {
      if (verbose > 0)
	NSLog(@"couldn't create %@ error: %@", usrRoot, error);
      [pool drain];
      exit(EXIT_FAILURE);
    }

  /*
   *	Before doing the main scan, we examine the 'Services' directory to
   *	see if any application has registered dynamic services - these take
   *	precedence over any listed in an applications Info_gnustep.plist.
   */
  scanDynamic(services, usrRoot);

  /*
   *	Scan for application information in all standard locations.
   */
  enumerator = [NSSearchPathForDirectoriesInDomains(
    NSAllApplicationsDirectory, NSAllDomainsMask, YES) objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      if ([path hasPrefix: @"."] == NO)
 	{
          scanApplications(services, path);
	}
    }

  /*
   *	Scan for service information in all standard locations.
   */
  enumerator = [NSSearchPathForDirectoriesInDomains(
    NSAllLibrariesDirectory, NSAllDomainsMask, YES) objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      scanServices(services,
	[path stringByAppendingPathComponent: @"Services"]);
    }

  fullMap = [NSMutableDictionary dictionaryWithCapacity: 5];
  [fullMap setObject: services forKey: @"ByPath"];
  [fullMap setObject: serviceMap forKey: @"ByService"];
  [fullMap setObject: filterList forKey: @"ByFilter"];
  [fullMap setObject: printMap forKey: @"ByPrint"];
  [fullMap setObject: spellMap forKey: @"BySpell"];

  str = [usrRoot stringByAppendingPathComponent: cacheName];
  if ([mgr fileExistsAtPath: str])
    {
      data = [NSData dataWithContentsOfFile: str];
      oldMap = [NSDeserializer deserializePropertyListFromData: data
					     mutableContainers: NO];
    }
  else
    {
      oldMap = nil;
    }
  if ([fullMap isEqual: oldMap] == NO)
    {
      data = [NSSerializer serializePropertyList: fullMap];
      if ([data writeToFile: str atomically: YES] == NO)
	{
	  if (verbose > 0)
	    NSLog(@"couldn't write %@", str);
	  exit(EXIT_FAILURE);
	}
    }

  str = [usrRoot stringByAppendingPathComponent: appsName];
  if ([mgr fileExistsAtPath: str])
    {
      data = [NSData dataWithContentsOfFile: str];
      oldMap = [NSDeserializer deserializePropertyListFromData: data
					     mutableContainers: NO];
    }
  else
    {
      oldMap = nil;
    }
  [applicationMap setObject: extensionsMap forKey: @"GSExtensionsMap"];
  [applicationMap setObject: schemesMap forKey: @"GSSchemesMap"];
  if ([applicationMap isEqual: oldMap] == NO)
    {
      data = [NSSerializer serializePropertyList: applicationMap];
      if ([data writeToFile: str atomically: YES] == NO)
	{
	  if (verbose > 0)
	    NSLog(@"couldn't write %@", str);
	  exit(EXIT_FAILURE);
	}
    }

  exit(EXIT_SUCCESS);
}

/*
 * Load information about the shemes of URLs that an application supports.
 * For each scheme found, produce a dictionary, keyed by app name, that
 * contains dictionaries giving scheme info for that extension.
 * NB. in order to make schemes case-insensiteve - we always convert
 * to lowercase.
 */
static void addSchemesForApplication(NSDictionary *info, NSString *app)
{
  unsigned int  i;
  id            o0;
  NSArray       *a0;


  o0 = [info objectForKey: @"CFBundleURLTypes"];

  if (o0)
    {
      if ([o0 isKindOfClass: aClass] == NO)
        {
	  if (verbose > 0)
	    NSLog(@"bad app CFBundleURLTypes (not an array) - %@", app);
          return;
        }
      a0 = (NSArray*)o0;
      i = [a0 count];
      while (i-- > 0)
        {
          NSDictionary          *t;
          NSArray               *a1;
          id                    o1 = [a0 objectAtIndex: i];
          unsigned int          j;

          if ([o1 isKindOfClass: dClass] == NO)
            {
	      if (verbose > 0)
		NSLog(@"bad app CFBundleURLTypes (type not a dictionary) - %@",
		  app);
              return;
            }
	  /*
	   * Set 't' to the dictionary defining a particular file type.
	   */
          t = (NSDictionary*)o1;
	  o1 = [t objectForKey: @"CFBundleURLSchemes"];
          if (o1 == nil)
            {
              continue;
            }
          if ([o1 isKindOfClass: aClass] == NO)
            {
	      if (verbose > 0)
		NSLog(@"bad app CFBundleURLTypes (schemes not an array) - %@",
		  app);
              return;
            }
          a1 = (NSArray*)o1;
          j = [a1 count];
          while (j-- > 0)
            {
              NSString			*e;
              NSMutableDictionary	*d;

              e = [[a1 objectAtIndex: j] lowercaseString];
	      if ([e length] == 0)
		{
		  if (verbose > 0)
		    NSLog(@"Illegal (nul) scheme ignored for - %@", app);
		  return;
		}
              d = [schemesMap objectForKey: e];
              if (d == nil)
                {
                  d = [NSMutableDictionary dictionaryWithCapacity: 1];
                  [schemesMap setObject: d forKey: e];
                }
              if ([d objectForKey: app] == nil)
                {
                  [d setObject: t forKey: app];
                }
            }
        }
    }
}

/*
 * Load information about the types of files that an application supports.
 * For each extension found, produce a dictionary, keyed by app name, that
 * contains dictionaries giving type info for that extension.
 * NB. in order to make extensions case-insensiteve - we always convert
 * to lowercase.
 */
static void addExtensionsForApplication(NSDictionary *info, NSString *app)
{
  unsigned int  i;
  id            o0;
  NSArray       *a0;


  o0 = [info objectForKey: @"NSTypes"];
  if (o0 == nil)
    {
      o0 = [info objectForKey: @"CFBundleDocumentTypes"];
    }

  if (o0)
    {
      if ([o0 isKindOfClass: aClass] == NO)
        {
	  if (verbose > 0)
	    NSLog(@"bad app NSTypes (not an array) - %@", app);
          return;
        }
      a0 = (NSArray*)o0;
      i = [a0 count];
      while (i-- > 0)
        {
          NSDictionary          *t;
          NSArray               *a1;
          id                    o1 = [a0 objectAtIndex: i];
          unsigned int          j;

          if ([o1 isKindOfClass: dClass] == NO)
            {
	      if (verbose > 0)
		NSLog(@"bad app NSTypes (type not a dictionary) - %@", app);
              return;
            }
	  /*
	   * Set 't' to the dictionary defining a particular file type.
	   */
          t = (NSDictionary*)o1;
          o1 = [t objectForKey: @"NSUnixExtensions"];

	  if (o1 == nil)
	    {
	      o1 = [t objectForKey: @"CFBundleTypeExtensions"];
	    }
          if (o1 == nil)
            {
              continue;
            }
          if ([o1 isKindOfClass: aClass] == NO)
            {
	      if (verbose > 0)
		NSLog(@"bad app NSType (extensions not an array) - %@", app);
              return;
            }
          a1 = (NSArray*)o1;
          j = [a1 count];
          while (j-- > 0)
            {
              NSString			*e;
              NSMutableDictionary	*d;

              e = [[a1 objectAtIndex: j] lowercaseString];
	      if ([e length] == 0)
		{
		  if (verbose > 0)
		    NSLog(@"Illegal (nul) extension ignored for - %@", app);
		  return;
		}
              d = [extensionsMap objectForKey: e];
              if (d == nil)
                {
                  d = [NSMutableDictionary dictionaryWithCapacity: 1];
                  [extensionsMap setObject: d forKey: e];
                }
              if ([d objectForKey: app] == nil)
                {
                  [d setObject: t forKey: app];
                }
            }
        }
    }
  else
    {
      NSDictionary	*extensions;

      o0 = [info objectForKey: @"NSExtensions"];
      if (o0 == nil)
	{
	  o0 = [info objectForKey: @"CFBundleTypeExtensions"];
	}
      if (o0 == nil)
        {
          return;
        }
      if ([o0 isKindOfClass: dClass] == NO)
        {
	  if (verbose > 0)
	    NSLog(@"bad app NSExtensions (not a dictionary) - %@", app);
          return;
        }
      extensions = (NSDictionary *) o0;
      a0 = [extensions allKeys];
      i = [a0 count];
      while (i-- > 0)
        {
          id	tmp = [extensions objectForKey: [a0 objectAtIndex: i]];
          id	name;
          id	dict;

          if ([tmp isKindOfClass: dClass] == NO)
	    {
	      if (verbose > 0)
		NSLog(@"bad app NSExtensions (value isn't a dictionary) - %@",
		      app);
              continue;
	    }
          name = [[a0 objectAtIndex: i] lowercaseString];
          dict = [extensionsMap objectForKey: name];
          if (dict == nil)
	    {
              dict = [NSMutableDictionary dictionaryWithCapacity: 1];
	    }
          [dict setObject: tmp forKey: app];
          [extensionsMap setObject: dict forKey: name];
        }
    }
}

#if	0	// UNUSED
static void
scanDirectory(NSMutableDictionary *services, NSString *path)
{
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSArray		*contents = [mgr directoryContentsAtPath: path];
  unsigned		index;

  for (index = 0; index < [contents count]; index++)
    {
      NSString	*name = [contents objectAtIndex: index];
      NSString	*ext = [name pathExtension];
      NSString	*newPath;
      BOOL	isDir;

      /*
       *	Ignore anything with a leading dot.
       */
      if ([name hasPrefix: @"."] == YES)
 	{
	  continue;
	}
      if (ext != nil
	&& ([ext isEqualToString: @"app"] || [ext isEqualToString: @"debug"]
	|| [ext isEqualToString: @"profile"]))
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      NSString		*oldPath;
	      NSBundle		*bundle;
	      NSDictionary	*info;

	      /*
	       *	All application paths are noted by name
	       *	in the 'applicationMap' dictionary.
	       */
              if ((oldPath = [applicationMap objectForKey: name]) == nil)
                {
                  [applicationMap setObject: newPath forKey: name];
                }
              else
                {
                  /*
                   * If we already have an entry for an application with
                   * this name, we skip this one - the first one takes
                   * precedence.
                   */
		  if (verbose > 0)
		    NSLog(@"duplicate app (%@) at '%@' and '%@'",
			  name, oldPath, newPath);
                  continue;
                }

	      bundle = [NSBundle bundleWithPath: newPath];
	      info = [bundle infoDictionary];
	      if (info)
		{
		  id	obj;

		  /*
		   * Load and validate any services definitions.
		   */
		  obj = [info objectForKey: @"NSServices"];
		  if (obj)
		    {
		      NSMutableArray	*entry;

		      entry = validateEntry(obj, newPath);
		      if (entry)
			{
			  [services setObject: entry forKey: newPath];
			}
		    }

		  addExtensionsForApplication(info, name);
		  addSchemesForApplication(info, name);
		}
	      else if (verbose > 0)
		{
		  NSLog(@"bad app info - %@", newPath);
		}
	    }
	  else if (verbose > 0)
	    {
	      NSLog(@"bad application - %@", newPath);
	    }
	}
      else if (ext != nil && [ext isEqualToString: @"service"])
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      NSBundle		*bundle;
	      NSDictionary	*info;

	      bundle = [NSBundle bundleWithPath: newPath];
	      info = [bundle infoDictionary];
	      if (info)
		{
		  id	svcs = [info objectForKey: @"NSServices"];

		  if (svcs)
		    {
		      NSMutableArray	*entry;

		      entry = validateEntry(svcs, newPath);
		      if (entry)
			{
			  [services setObject: entry forKey: newPath];
			}
		    }
		  else if (verbose > 0)
		    {
		      NSLog(@"missing info - %@", newPath);
		    }
		}
	      else if (verbose > 0)
		{
		  NSLog(@"bad service info - %@", newPath);
		}
	    }
	  else if (verbose > 0)
	    {
	      NSLog(@"bad services bundle - %@", newPath);
	    }
	}
      else
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      scanDirectory(services, newPath);
	    }
	}
    }
  [arp release];
}
#endif

static void
scanApplications(NSMutableDictionary *services, NSString *path)
{
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSArray		*contents = [mgr directoryContentsAtPath: path];
  unsigned		index;

  for (index = 0; index < [contents count]; index++)
    {
      NSString	*name = [contents objectAtIndex: index];
      NSString	*ext = [name pathExtension];
      NSString	*newPath;
      BOOL	isDir;

      /*
       *	Ignore anything with a leading dot.
       */
      if ([name hasPrefix: @"."] == YES)
 	{
	  continue;
	}
      if (ext != nil
	&& ([ext isEqualToString: @"app"] || [ext isEqualToString: @"debug"]
	|| [ext isEqualToString: @"profile"]))
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      NSString		*oldPath;
	      NSBundle		*bundle;
	      NSDictionary	*info;

	      /*
	       *	All application paths are noted by name
	       *	in the 'applicationMap' dictionary.
	       */
              if ((oldPath = [applicationMap objectForKey: name]) == nil)
                {
                  [applicationMap setObject: newPath forKey: name];
                }
              else
                {
                  /*
                   * If we already have an entry for an application with
                   * this name, we skip this one - the first one takes
                   * precedence.
                   */
		  if (verbose > 0)
		    NSLog(@"duplicate app (%@) at '%@' and '%@'",
			  name, oldPath, newPath);
                  continue;
                }

	      bundle = [NSBundle bundleWithPath: newPath];
	      info = [bundle infoDictionary];
	      if (info)
		{
		  id	obj;

		  /*
		   * Load and validate any services definitions.
		   */
		  obj = [info objectForKey: @"NSServices"];
		  if (obj)
		    {
		      NSMutableArray	*entry;

		      entry = validateEntry(obj, newPath);
		      if (entry)
			{
			  [services setObject: entry forKey: newPath];
			}
		    }

		  addExtensionsForApplication(info, name);
		  addSchemesForApplication(info, name);
		}
	      else if (verbose > 0)
		{
		  NSLog(@"bad app info - %@", newPath);
		}
	    }
	  else if (verbose > 0)
	    {
	      NSLog(@"bad application - %@", newPath);
	    }
	}
      else
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      scanApplications(services, newPath);
	    }
	}
    }
  [arp drain];
}

static void
scanDynamic(NSMutableDictionary *services, NSString *path)
{
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSArray		*contents = [mgr directoryContentsAtPath: path];
  unsigned		index;

  for (index = 0; index < [contents count]; index++)
    {
      NSString		*name = [contents objectAtIndex: index];
      NSString		*infPath;
      NSDictionary	*info;

      /*
       *	Ignore anything with a leading dot.
       */
      if ([name hasPrefix: @"."])
	{
	  continue;
	}

      /* *.service bundles are handled in scanDirectory */
      if ([[name pathExtension] isEqualToString: @"service"])
	continue;

      infPath = [path stringByAppendingPathComponent: name];

      info = [NSDictionary dictionaryWithContentsOfFile: infPath];
      if (info)
	{
	  id	svcs = [info objectForKey: @"NSServices"];

	  if (svcs)
	    {
	      NSMutableArray	*entry;

	      entry = validateEntry(svcs, infPath);
	      if (entry)
		{
		  [services setObject: entry forKey: infPath];
		}
	    }
	}
      else if (verbose > 0)
	{
	  NSLog(@"bad app info - %@", infPath);
	}
    }
  [arp drain];
}

static void
scanServices(NSMutableDictionary *services, NSString *path)
{
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSArray		*contents = [mgr directoryContentsAtPath: path];
  unsigned		index;

  for (index = 0; index < [contents count]; index++)
    {
      NSString	*name = [contents objectAtIndex: index];
      NSString	*ext = [name pathExtension];
      NSString	*newPath;
      BOOL	isDir;

      /*
       *	Ignore anything with a leading dot.
       */
      if ([name hasPrefix: @"."] == YES)
 	{
	  continue;
	}
      if (ext != nil && [ext isEqualToString: @"service"])
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      NSBundle		*bundle;
	      NSDictionary	*info;

	      bundle = [NSBundle bundleWithPath: newPath];
	      info = [bundle infoDictionary];
	      if (info)
		{
		  id	svcs = [info objectForKey: @"NSServices"];

		  if (svcs)
		    {
		      NSMutableArray	*entry;

		      entry = validateEntry(svcs, newPath);
		      if (entry)
			{
			  [services setObject: entry forKey: newPath];
			}
		    }
		  else if (verbose > 0)
		    {
		      NSLog(@"missing info - %@", newPath);
		    }
		}
	      else if (verbose > 0)
		{
		  NSLog(@"bad service info - %@", newPath);
		}
	    }
	  else if (verbose > 0)
	    {
	      NSLog(@"bad services bundle - %@", newPath);
	    }
	}
      else
	{
	  newPath = [path stringByAppendingPathComponent: name];
	  newPath = [newPath stringByStandardizingPath];
	  if ([mgr fileExistsAtPath: newPath isDirectory: &isDir] && isDir)
	    {
	      scanServices(services, newPath);
	    }
	}
    }
  [arp drain];
}

static NSMutableArray*
validateEntry(id svcs, NSString *path)
{
  NSMutableArray	*newServices;
  NSArray		*services;
  unsigned		pos;

  if ([svcs isKindOfClass: aClass] == NO)
    {
      if (verbose > 0)
	NSLog(@"NSServices entry not an array - %@", path);
      return nil;
    }

  services = (NSArray*)svcs;
  newServices = [NSMutableArray arrayWithCapacity: [services count]];
  for (pos = 0; pos < [services count]; pos++)
    {
      id			svc;

      svc = [services objectAtIndex: pos];
      if ([svc isKindOfClass: dClass])
	{
	  NSDictionary		*service = (NSDictionary*)svc;
	  NSMutableDictionary	*newService;

	  newService = validateService(service, path, pos);
	  if (newService)
	    {
	      [newServices addObject: newService];
	    }
	}
      else if (verbose > 0)
	{
	  NSLog(@"NSServices entry %u not a dictionary - %@",
	    pos, path);
	}
    }
  return newServices;
}

static NSMutableDictionary*
validateService(NSDictionary *service, NSString *path, unsigned pos)
{
  static NSDictionary	*fields = nil;
  NSEnumerator		*e;
  NSMutableDictionary	*result;
  NSString		*k;
  id			obj;

  if (fields == nil)
    {
      fields = [NSDictionary dictionaryWithObjectsAndKeys:
	@"string", @"NSMessage",
	@"string", @"NSPortName",
	@"array", @"NSSendTypes",
	@"array", @"NSReturnTypes",
	@"dictionary", @"NSMenuItem",
	@"dictionary", @"NSKeyEquivalent",
	@"string", @"NSUserData",
	@"string", @"NSTimeout",
	@"string", @"NSHost",
	@"string", @"NSExecutable",
	@"string", @"NSFilter",
	@"string", @"NSInputMechanism",
	@"string", @"NSPrintFilter",
	@"string", @"NSDeviceDependent",
	@"array", @"NSLanguages",
	@"string", @"NSSpellChecker",
	nil];
      [fields retain];
    }

  result = [NSMutableDictionary dictionaryWithCapacity: [service count]];

  /*
   *	Step through and check that each field is a known one and of the
   *	correct type.
   */
  e = [service keyEnumerator];
  while ((k = [e nextObject]) != nil)
    {
      NSString	*type = [fields objectForKey: k];

      if (type == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u spurious field (%@)- %@", pos, k, path);
	}
      else
	{
	  obj = [service objectForKey: k];
	  if ([type isEqualToString: @"string"])
	    {
	      if ([obj isKindOfClass: sClass] == NO)
		{
		  if (verbose > 0)
		    NSLog(@"NSServices entry %u field %@ is not a string "
			  @"- %@", pos, k, path);
		  return nil;
		}
	      [result setObject: obj forKey: k];
	    }
	  else if ([type isEqualToString: @"array"])
	    {
	      NSArray	*a;

	      if ([obj isKindOfClass: aClass] == NO)
		{
		  if (verbose > 0)
		    NSLog(@"NSServices entry %u field %@ is not an array "
		    @"- %@", pos, k, path);
		  return nil;
		}
	      a = (NSArray*)obj;
	      if ([a count] == 0)
		{
		  if (verbose > 0)
		    NSLog(@"NSServices entry %u field %@ is an empty array "
			  @"- %@", pos, k, path);
		}
	      else
		{
		  unsigned	i;

		  for (i = 0; i < [a count]; i++)
		    {
		      if ([[a objectAtIndex: i] isKindOfClass: sClass] == NO)
			{
			  if (verbose > 0)
			    NSLog(@"NSServices entry %u field %@ element %u is "
				  @"not a string - %@", pos, k, i, path);
			  return nil;
			}
		    }
		  [result setObject: a forKey: k];
		}
	    }
	  else if ([type isEqualToString: @"dictionary"])
	    {
	      NSDictionary	*d;

	      if ([obj isKindOfClass: dClass] == NO)
		{
		  if (verbose > 0)
		    NSLog(@"NSServices entry %u field %@ is not a dictionary "
			  @"- %@", pos, k, path);
		  return nil;
		}
	      d = (NSDictionary*)obj;
	      if ([d objectForKey: @"default"] == nil)
		{
		  if (verbose > 0)
		    NSLog(@"NSServices entry %u field %@ has no default value "
			  @"- %@", pos, k, path);
		}
	      else
		{
		  NSEnumerator	*e = [d objectEnumerator];

		  while ((obj = [e nextObject]) != nil)
		    {
		      if ([obj isKindOfClass: sClass] == NO)
			{
			  if (verbose > 0)
			    NSLog(@"NSServices entry %u field %@ contains "
				  @"non-string value - %@", pos, k, path);
			  return nil;
			}
		    }
		}
	      [result setObject: d forKey: k];
	    }
	}
    }

  /*
   *	Record in this service dictionary where it is to be found.
   */
  [result setObject: path forKey: @"ServicePath"];

  /*
   *	Now check that we have the required fields for the service.
   */
  if ((obj = [result objectForKey: @"NSFilter"]) != nil)
    {
      NSString		*str;
      NSArray		*snd;
      NSArray		*ret;
      BOOL		notPresent = NO;

      snd = [result objectForKey: @"NSSendTypes"];
      ret = [result objectForKey: @"NSReturnTypes"];
      str = [result objectForKey: @"NSInputMechanism"];
      if (str != nil)
	{
	  if ([str isEqualToString: @"NSUnixStdio"] == YES
	    || [str isEqualToString: @"NSMapFile"] == YES)
	    {
	      unsigned	i = [snd count];

	      while (i-- > 0)
		{

		  str = [snd objectAtIndex: i];
		  /* For UNIX I/O or file mapping, the send type must be a
		   * filename ... which means it must either be the generic
		   * filenames pasteboard type, or one of the pasteboard
		   * types corresponding to names for a particular file type.
		   */
		  if (NO == [str isEqual: @"NSFilenamesPboardType"]
		    && NO == [str hasPrefix: @"NSTypedFilenamesPboardType:"])
		    {
		      if (verbose > 0)
			{
			  NSLog(@"NSServices entry %u bad NSSendTypes "
			    @"(must be file names types) - %@",
			    pos, path);
			}
		    }
		}
	    }
	  else if ([str isEqualToString: @"NSIdentity"] == NO)
	    {
	      if (verbose > 0)
		{
		  NSLog(@"NSServices entry %u bad input mechanism - %@",
		    pos, path);
		}
	      return nil;
	    }
	}
      else if ([result objectForKey: @"NSPortName"] == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u NSPortName missing - %@", pos, path);
	  return nil;
	}

      if ([snd count] == 0 || [ret count] == 0)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u types empty or missing - %@",
	      pos, path);
	  return nil;
	}
      else
	{
	  unsigned	i = [snd count];

	  /*
	   * See if this filter handles any send/return combination
	   * which is not alreadly present.
	   */
	  while (notPresent == NO && i-- > 0)
	    {
	      unsigned	j = [ret count];

	      while (notPresent == NO && j-- > 0)
		{
		  str = [NSString stringWithFormat: @"%@==>%@",
		    [snd objectAtIndex: i], [ret objectAtIndex: j]];
		  if ([filterSet member: str] == nil)
		    {
		      notPresent = YES;
		      [filterSet addObject: str];
		      [filterList addObject: result];
		    }
		}
	    }
	}
      if (notPresent == NO)
	{
	  if (verbose > 0)
	    {
	      NSLog(@"Ignoring duplicate %u in %@ -\n%@", pos, path, result);
	    }
	  return nil;
	}
    }
  else if ((obj = [result objectForKey: @"NSMessage"]) != nil)
    {
      NSDictionary	*item;
      NSEnumerator	*e;
      NSString		*k;
      BOOL		used = NO;

      if ([result objectForKey: @"NSPortName"] == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u NSPortName missing - %@", pos, path);
	  return nil;
	}
      if ([result objectForKey: @"NSSendTypes"] == nil
	&& [result objectForKey: @"NSReturnTypes"] == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u types missing - %@", pos, path);
	  return nil;
	}
      if ((item = [result objectForKey: @"NSMenuItem"]) == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u NSMenuItem missing - %@", pos, path);
	  return nil;
	}

      /*
       *	For each language, check to see if we already have a service
       *	by this name - if so - we ignore this one.
       */
      e = [item keyEnumerator];
      while ((k = [e nextObject]) != nil)
	{
	  NSString		*name = [item objectForKey: k];
	  NSMutableDictionary	*names;

	  names = [serviceMap objectForKey: k];
	  if (names == nil)
	    {
	      names = [NSMutableDictionary dictionaryWithCapacity: 1];
	      [serviceMap setObject: names forKey: k];
	    }
	  if ([names objectForKey: name] == nil)
	    {
	      [names setObject: result forKey: name];
	      used = YES;
	    }
	}
      if (used == NO)
	{
	  if (verbose > 0)
	    {
	      NSLog(@"Ignoring entry %u in %@ -\n%@", pos, path, result);
	    }
	  return nil;	/* Ignore - already got service with this name	*/
	}
    }
  else if ((obj = [result objectForKey: @"NSPrintFilter"]) != nil)
    {
      NSDictionary	*item;
      NSEnumerator	*e;
      NSString		*k;
      BOOL		used = NO;

      if ((item = [result objectForKey: @"NSMenuItem"]) == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry %u NSMenuItem missing - %@", pos, path);
	  return nil;
	}
      /*
       *	For each language, check to see if we already have a print
       *	filter by this name - if so - we ignore this one.
       */
      e = [item keyEnumerator];
      while ((k = [e nextObject]) != nil)
	{
	  NSString		*name = [item objectForKey: k];
	  NSMutableDictionary	*names;

	  names = [printMap objectForKey: k];
	  if (names == nil)
	    {
	      names = [NSMutableDictionary dictionaryWithCapacity: 1];
	      [printMap setObject: names forKey: k];
	    }
	  if ([names objectForKey: name] == nil)
	    {
	      [names setObject: result forKey: name];
	      used = YES;
	    }
	}
      if (used == NO)
	{
	  if (verbose > 0)
	    {
	      NSLog(@"Ignoring entry %u in %@ -\n%@", pos, path, result);
	    }
	  return nil;	/* Ignore - already got filter with this name	*/
	}
    }
  else if ((obj = [result objectForKey: @"NSSpellChecker"]) != nil)
    {
      NSArray	*item;
      unsigned	pos;
      BOOL	used = NO;

      if ((item = [result objectForKey: @"NSLanguages"]) == nil)
	{
	  if (verbose > 0)
	    NSLog(@"NSServices entry NSLanguages missing - %@", path);
	  return nil;
	}
      /*
       *	For each language, check to see if we already have a spell
       *	checker by this name - if so - we ignore this one.
       */
      pos = [item count];
      while (pos-- > 0)
	{
	  NSString	*lang = [item objectAtIndex: pos];

	  if ([spellMap objectForKey: lang] == nil)
	    {
	      [spellMap setObject: result forKey: lang];
	      used = YES;
	    }
	}
      if (used == NO)
	{
	  if (verbose > 0)
	    {
	      NSLog(@"Ignoring entry %u in %@ -\n%@", pos, path, result);
	    }
	  return nil;	/* Ignore - already got speller with language.	*/
	}
    }
  else
    {
      if (verbose > 0)
	NSLog(@"NSServices entry %u unknown service/filter - %@", pos, path);
      return nil;
    }

  return result;
}

