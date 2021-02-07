/** Implementation of NSBundle class
   Copyright (C) 1993-2002 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: May 1993

   Author: Mirko Viviani <mirko.viviani@rccr.cremona.it>
   Date: October 2000  Added frameworks support

   Author: Nicola Pero <nicola@brainstorm.co.uk>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.


   <title>NSBundle class reference</title>
   $Date$ $Revision$
*/

#define	EXPOSE_NSBundle_IVARS	1
#import "common.h"
#include "objc-load.h"
#import "Foundation/NSBundle.h"
#import "Foundation/NSException.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSData.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSSet.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSTask+GNUstepBase.h"

#import "GSPrivate.h"

/* Constants */
NSString * const NSBundleDidLoadNotification = @"NSBundleDidLoadNotification";
NSString * const NSShowNonLocalizedStrings = @"NSShowNonLocalizedStrings";
NSString * const NSLoadedClasses = @"NSLoadedClasses";

static NSFileManager	*
manager()
{
  static NSFileManager	*mgr = nil;

  if (mgr == nil)
    {
      mgr = RETAIN([NSFileManager defaultManager]);
      [[NSObject leakAt: &mgr] release];
    }
  return mgr;
}

static NSDictionary     *langAliases = nil;
static NSDictionary     *langCanonical = nil;

/* Map a language name to any alternative versions.   This function should
 * return an array of alternative language/localisation directory names in
 * the preferred order of precedence (ie resources in the directories named
 * earlier in the array are to be preferred to those in directories named
 * later).
 * We should support regional language specifications (such as en-GB)
 * as our first priority, and then fall back to the more general names.
 * NB. Also handle the form  like en-GB_US (English language, British dialect,
 * in the United States region).
 */
static NSArray *
altLang(NSString *full)
{
  NSMutableArray        *a = nil;

  if (nil != full)
    {
      NSString  *alias = nil;
      NSString  *canon = nil;
      NSString  *lang = nil;
      NSString  *dialect = nil;
      NSString  *region = nil;
      NSRange   r;

      alias = [langAliases objectForKey: full];
      if (nil == alias)
        {
          canon = [langCanonical objectForKey: full];
          if (nil != canon)
            {
              alias = [langAliases objectForKey: canon];
            }
          if (nil == alias)
            {
              alias = full;
            }
        }
      canon = [langCanonical objectForKey: alias];
      if (nil == canon)
        {
          canon = [langCanonical objectForKey: full];
          if (nil == canon)
            {
              canon = full;
            }
        }

      if ((r = [canon rangeOfString: @"-"]).length > 1)
        {
          dialect = [canon substringFromIndex: NSMaxRange(r)];
          lang = [canon substringToIndex: r.location];
          if ((r = [dialect rangeOfString: @"_"]).length > 1)
            {
              region = [dialect substringFromIndex: NSMaxRange(r)];
              dialect = [dialect substringToIndex: r.location];
            }
        }
      else if ((r = [canon rangeOfString: @"_"]).length > 1)
        {
          region = [canon substringFromIndex: NSMaxRange(r)];
          lang = [canon substringToIndex: r.location];
        }
      else
        {
          lang = canon;
        }

      a = [NSMutableArray arrayWithCapacity: 5];
      if (nil != dialect && nil != region)
        {
          [a addObject: [NSString stringWithFormat: @"%@-%@_%@",
            lang, dialect, region]];
        }
      if (nil != dialect)
        {
          [a addObject: [NSString stringWithFormat: @"%@-%@",
            lang, dialect]];
        }
      if (nil != region)
        {
          [a addObject: [NSString stringWithFormat: @"%@_%@",
            lang, region]];
        }
      [a addObject: lang];
      if (NO == [a containsObject: alias])
        {
          [a addObject: alias];
        }
    }
  return a;
}

static NSLock *pathCacheLock = nil;
static NSMutableDictionary *pathCache = nil;

@interface NSObject (PrivateFrameworks)
+ (NSString*) frameworkVersion;
+ (NSString**) frameworkClasses;
@end

typedef enum {
  NSBUNDLE_BUNDLE = 1,
  NSBUNDLE_APPLICATION,
  NSBUNDLE_FRAMEWORK,
  NSBUNDLE_LIBRARY
} bundle_t;

/* Class variables - We keep track of all the bundles */
static NSBundle		*_mainBundle = nil;
static NSMapTable	*_bundles = NULL;
static NSMapTable	*_byClass = NULL;
static NSMapTable	*_byIdentifier = NULL;

/* Store the working directory at startup */
static NSString		*_launchDirectory = nil;

static NSString		*_base_version
  = OBJC_STRINGIFY(GNUSTEP_BASE_MAJOR_VERSION.GNUSTEP_BASE_MINOR_VERSION);

/*
 * An empty strings file table for use when localization files can't be found.
 */
static NSDictionary	*_emptyTable = nil;

/* When we are linking in an object file, GSPrivateLoadModule calls our
   callback routine for every Class and Category loaded.  The following
   variable stores the bundle that is currently doing the loading so we know
   where to store the class names.
*/
static NSBundle		*_loadingBundle = nil;
static NSBundle		*_gnustep_bundle = nil;
static NSRecursiveLock	*load_lock = nil;
static BOOL		_strip_after_loading = NO;

/* List of framework linked in the _loadingBundle */
static NSMutableArray	*_loadingFrameworks = nil;
static NSString         *_currentFrameworkName = nil;

static NSString	*gnustep_target_dir =
#ifdef GNUSTEP_TARGET_DIR
  @GNUSTEP_TARGET_DIR;
#else
  nil;
#endif
static NSString	*gnustep_target_cpu =
#ifdef GNUSTEP_TARGET_CPU
  @GNUSTEP_TARGET_CPU;
#else
  nil;
#endif
static NSString	*gnustep_target_os =
#ifdef GNUSTEP_TARGET_OS
  @GNUSTEP_TARGET_OS;
#else
  nil;
#endif
static NSString	*library_combo =
#ifdef LIBRARY_COMBO
  @LIBRARY_COMBO;
#else
  nil;
#endif

#ifdef __ANDROID__
static jobject _jassetManager = NULL;
static AAssetManager *_assetManager = NULL;
#endif


/*
 * Try to find the absolute path of an executable.
 * Search all the directoried in the PATH.
 * The atLaunch flag determines whether '.' is considered to be
 * the  current working directory or the working directory at the
 * time when the program was launched (technically the directory
 * at the point when NSBundle was first used ... so programs must
 * use NSBundle *before* changing their working directories).
 */
static NSString*
AbsolutePathOfExecutable(NSString *path, BOOL atLaunch)
{
  if (NO == [path isAbsolutePath])
    {
      NSFileManager	*mgr = manager();
      NSDictionary	*env;
      NSString		*pathlist;
      NSString		*prefix;
      id		patharr;
      NSString		*result = nil;

      env = [[NSProcessInfo processInfo] environment];
      pathlist = [env objectForKey: @"PATH"];

    /* Windows 2000 and perhaps others have "Path" not "PATH" */
      if (pathlist == nil)
	{
	  pathlist = [env objectForKey: @"Path"];
	}
#if defined(_WIN32)
      patharr = [pathlist componentsSeparatedByString: @";"];
#else
      patharr = [pathlist componentsSeparatedByString: @":"];
#endif
      /* Add . if not already in path */
      if ([patharr indexOfObject: @"."] == NSNotFound)
	{
	  patharr = AUTORELEASE([patharr mutableCopy]);
	  [patharr addObject: @"."];
	}
      patharr = [patharr objectEnumerator];
      while (nil != (prefix = [patharr nextObject]))
	{
	  if ([prefix isEqual: @"."])
	    {
	      if (atLaunch == YES)
		{
		  prefix = _launchDirectory;
		}
	      else
		{
		  prefix = [mgr currentDirectoryPath];
		}
	    }
	  prefix = [prefix stringByAppendingPathComponent: path];
	  if ([mgr isExecutableFileAtPath: prefix])
	    {
	      result = [prefix stringByStandardizingPath];
	      break;
	    }
#if defined(_WIN32)
	  else
	    {
	      NSString	*extension = [path pathExtension];

	      /* Also try adding any executable extensions on windows
               */
	      if ([extension length] == 0)
		{
                  static NSSet  *executable = nil;
		  NSString      *wpath;
                  NSEnumerator  *e;
                  NSString      *s;

                  if (nil == executable)
                    {
                      executable = [[NSTask executableExtensions] copy];
                    }

                  e = [executable objectEnumerator];
                  while (nil != (s = [e nextObject]))
                    {
                      wpath = [prefix stringByAppendingPathExtension: s];
                      if ([mgr isExecutableFileAtPath: wpath])
                        {
                          result = [wpath stringByStandardizingPath];
                          break;
                        }
		    }
		}
	    }
#endif
	}
      path = result;
    }
  path = [path stringByResolvingSymlinksInPath];
  path = [path stringByStandardizingPath];
  return path;
}

/*
 * Return the path to this executable.
 */
NSString *
GSPrivateExecutablePath()
{
  static NSString	*executablePath = nil;
  static BOOL		beenHere = NO;

  if (beenHere == NO)
    {
      [load_lock lock];
      if (beenHere == NO)
	{
#if	defined(PROCFS_EXE_LINK)
	  executablePath = [manager()
	    pathContentOfSymbolicLinkAtPath:
              [NSString stringWithUTF8String: PROCFS_EXE_LINK]];

	  /*
	  On some systems, the link is of the form "[device]:inode", which
	  can be used to open the executable, but is useless if you want
	  the path to it. Thus we check that the path is an actual absolute
	  path. (Using '/' here is safe; it isn't the path separator
	  everywhere, but it is on all systems that have PROCFS_EXE_LINK.)
	  */
	  if ([executablePath length] > 0
	    && [executablePath characterAtIndex: 0] != '/')
	    {
	      executablePath = nil;
	    }
#endif
	  if (executablePath == nil || [executablePath length] == 0)
	    {
	      executablePath
		= [[[NSProcessInfo processInfo] arguments] objectAtIndex: 0];
	    }
	  if (NO == [executablePath isAbsolutePath])
	    {
	      executablePath = AbsolutePathOfExecutable(executablePath, YES);
	    }
	  else
	    {
	      executablePath = [executablePath stringByResolvingSymlinksInPath];
	      executablePath = [executablePath stringByStandardizingPath];
	    }
	  IF_NO_GC([executablePath retain];)
	  beenHere = YES;
	}
      [load_lock unlock];
      NSCAssert(executablePath != nil, NSInternalInconsistencyException);
    }
  return executablePath;
}

static NSArray *
bundle_directory_readable(NSString *path)
{
  id	found;

  [pathCacheLock lock];
  found = [[[pathCache objectForKey: path] retain] autorelease];
  [pathCacheLock unlock];
  if (nil == found)
    {
      NSFileManager	*mgr = manager();

      found = [mgr directoryContentsAtPath: path];
      if (nil == found)
	{
	  found = [NSNull null];
	}
      [pathCacheLock lock];
      [pathCache setObject: found forKey: path];
      [pathCacheLock unlock];
    }
  if ((id)[NSNull null] == found)
    {
      found = nil;
    }
  return (NSArray*)found;
}

/* Get the object file that should be located in the bundle of the same name */
static NSString *
bundle_object_name(NSString *path, NSString* executable)
{
  NSFileManager	*mgr = manager();
  NSString	*name, *path0, *path1, *path2;

  if (executable)
    {
      NSString	*exepath;

      name = [executable lastPathComponent];
      exepath = [executable stringByDeletingLastPathComponent];
      if ([exepath isEqualToString: @""] == NO)
	{
	  if ([exepath isAbsolutePath] == YES)
	    path = exepath;
	  else
	    path = [path stringByAppendingPathComponent: exepath];
	}
    }
  else
    {
      name = [[path lastPathComponent] stringByDeletingPathExtension];
      path = [path stringByDeletingLastPathComponent];
    }
  path0 = [path stringByAppendingPathComponent: name];
  path = [path stringByAppendingPathComponent: gnustep_target_dir];
  path1 = [path stringByAppendingPathComponent: name];
  path = [path stringByAppendingPathComponent: library_combo];
  path2 = [path stringByAppendingPathComponent: name];

  if ([mgr isReadableFileAtPath: path2] == YES)
    return path2;
  else if ([mgr isReadableFileAtPath: path1] == YES)
    return path1;
  else if ([mgr isReadableFileAtPath: path0] == YES)
    return path0;
#if defined(_WIN32)
  /* If we couldn't find the binary, and we are on windows, and the name
   * has no path extension, then let's try looking for a dll.
   */
  if ([name pathExtension] == nil)
    {
      if ([mgr isReadableFileAtPath:
	[path2 stringByAppendingPathExtension: @"dll"]] == YES)
	return [path2 stringByAppendingPathExtension: @"dll"];
      else if ([mgr isReadableFileAtPath:
	[path1 stringByAppendingPathExtension: @"dll"]] == YES)
	return [path1 stringByAppendingPathExtension: @"dll"];
      else if ([mgr isReadableFileAtPath:
	[path0 stringByAppendingPathExtension: @"dll"]] == YES)
	return [path0 stringByAppendingPathExtension: @"dll"];
    }
#endif
  return path0;
}

/* Construct a path from components */
static void
addBundlePath(NSMutableArray *list, NSArray *contents,
  NSString *path, NSString *subdir, NSString *lang)
{
  if (nil == contents)
    {
      return;
    }
  if (nil != subdir)
    {
      NSEnumerator      *e = [[subdir pathComponents] objectEnumerator];
      NSString          *subdirComponent;

      while ((subdirComponent = [e nextObject]) != nil)
	{
	  if (NO == [contents containsObject: subdirComponent])
	    {
	      return;
	    }
	  path = [path stringByAppendingPathComponent: subdirComponent];
	  if (nil == (contents = bundle_directory_readable(path)))
	    {
	      return;
	    }	  
	}
    }
  if (nil == lang)
    {
      [list addObject: path];
    }
  else
    {
      NSEnumerator      *enumerator = [altLang(lang) objectEnumerator];
      NSString          *alt;

      /* Add each language specific subdirectory in order.
       */
      while (nil != (alt = [enumerator nextObject]))
        {
          alt = [alt stringByAppendingPathExtension: @"lproj"];
          if (YES == [contents containsObject: alt])
            {
              alt = [path stringByAppendingPathComponent: alt];
              if (nil != (contents = bundle_directory_readable(alt)))
                {
                  [list addObject: alt];
                }
            }
        }
    }
}

/* Try to locate name framework in standard places
   which are like /Library/Frameworks/(name).framework */
static inline NSString *
_find_framework(NSString *name)
{
  NSArray	*paths;
  NSFileManager *file_mgr = manager();
  NSString	*file_name;
  NSString	*file_path;
  NSString	*path;
  NSEnumerator	*enumerator;

  NSCParameterAssert(name != nil);

  file_name = [name stringByAppendingPathExtension: @"framework"];
  paths = NSSearchPathForDirectoriesInDomains(GSFrameworksDirectory,
            NSAllDomainsMask,YES);

  enumerator = [paths objectEnumerator];
  while ((path = [enumerator nextObject]))
    {
      file_path = [path stringByAppendingPathComponent: file_name];

      if ([file_mgr fileExistsAtPath: file_path] == YES)
        {
          return file_path; // Found it!
        }
    }
  return nil;
}


/* Try to locate resources for tool name (which is this tool) in
 * standard places like xxx/Library/Tools/Resources/name */
/* This could be converted into a public +bundleForTool:
 * method.  At the moment it's only used privately
 * to locate the main bundle for this tool.
 */
static inline NSString *
_find_main_bundle_for_tool(NSString *toolName)
{
  NSArray *paths;
  NSEnumerator *enumerator;
  NSString *path;
  NSString *tail;
  NSFileManager *fm = manager();

  /*
   * Eliminate any base path or extensions.
   */
  toolName = [toolName lastPathComponent];
  do
    {
      toolName = [toolName stringByDeletingPathExtension];
    }
  while ([[toolName pathExtension] length] > 0);

  if ([toolName length] == 0)
    {
      return nil;
    }

  tail = [@"Tools" stringByAppendingPathComponent:
	     [@"Resources" stringByAppendingPathComponent:
		 toolName]];

  paths = NSSearchPathForDirectoriesInDomains (NSLibraryDirectory,
					       NSAllDomainsMask, YES);

  enumerator = [paths objectEnumerator];
  while ((path = [enumerator nextObject]))
    {
      BOOL isDir;
      path = [path stringByAppendingPathComponent: tail];

      if ([fm fileExistsAtPath: path  isDirectory: &isDir]  &&  isDir)
	{
	  return path;
	}
    }

  return nil;
}


@implementation NSBundle (Private)

+ (NSString *) _absolutePathOfExecutable: (NSString *)path
{
  return AbsolutePathOfExecutable(path, NO);
}

/* Nicola & Mirko:

   Frameworks can be used in an application in two different ways:

   () the framework is dynamically/manually loaded, as if it were a
   bundle.  This is the easier case, because we already have the
   bundle setup with the correct path (it's the programmer's
   responsibility to find the framework bundle on disk); we get all
   information from the bundle dictionary, such as the version; we
   also create the class list when loading the bundle, as for any
   other bundle.

   () the framework was linked into the application.  This is much
   more difficult, because without using tricks, we have no way of
   knowing where the framework bundle (needed eg for resources) is on
   disk, and we have no way of knowing what the class list is, or the
   version.  So the trick we use in this case to work around those
   problems is that gnustep-make generates a 'NSFramework_xxx' class
   and compiles it into each framework.  By asking to the class, we
   can get the version information and the list of classes which were
   compiled into the framework.  To get the location of the framework
   on disk, we try using advanced dynamic linker features to get the
   shared object file on disk from which the NSFramework_xxx class was
   loaded.  If that doesn't work, because the dynamic linker can't
   provide this information on this platform (or maybe because the
   framework was statically linked into the application), we have a
   fallback trick :-) We look for the framework in the standard
   locations and in the main bundle.  This might fail if the framework
   is not in a standard location or there is more than one installed
   framework of the same name (and different versions?).

   So at startup, we scan all classes which were compiled into the
   application.  For each NSFramework_ class, we call the following
   function, which records the name of the framework, the version,
   the classes belonging to it, and tries to determine the path
   on disk to the framework bundle.

   Bundles (and frameworks if dynamically loaded as bundles) could
   depend on other frameworks (linked togheter on platform that
   supports this behaviour) so whenever we dynamically load a bundle,
   we need to spot out any additional NSFramework_* classes which are
   loaded, and call this method (exactly as for frameworks linked into
   the main application) to record them, and try finding the path on
   disk to those framework bundles.

*/
+ (NSBundle*) _addFrameworkFromClass: (Class)frameworkClass
{
  NSBundle	*bundle = nil;
  NSString	**fmClasses;
  NSString	*bundlePath = nil;
  unsigned int	len;
  const char    *frameworkClassName;

  if (frameworkClass == Nil)
    {
      return nil;
    }

  frameworkClassName = class_getName(frameworkClass);

  len = strlen (frameworkClassName);

  if (len > 12 * sizeof(char)
    && !strncmp ("NSFramework_", frameworkClassName, 12))
    {
      /* The name of the framework.  */
      NSString *name;

      /* If the bundle for this framework class is already loaded,
       * simply return it.  The lookup will return an NSNull object
       * if the framework class has no known bundle.
       */
      bundle = (id)NSMapGet(_byClass, frameworkClass);
      if (nil != bundle)
        {
          if ((id)bundle == (id)[NSNull null])
            {
              bundle = nil;
            }
          return bundle;
        }

      name = [NSString stringWithUTF8String: &frameworkClassName[12]];
      /* Important - gnustep-make mangles framework names to encode
       * them as ObjC class names.  Here we need to demangle them.  We
       * apply the reverse transformations in the reverse order.
       */
      name = [name stringByReplacingString: @"_1"  withString: @"+"];
      name = [name stringByReplacingString: @"_0"  withString: @"-"];
      name = [name stringByReplacingString: @"__"  withString: @"_"];

      /* Try getting the path to the framework using the dynamic
       * linker.  When it works it's really cool :-) This is the only
       * really universal way of getting the framework path ... we can
       * locate the framework no matter where it is on disk!
       */
      bundlePath = GSPrivateSymbolPath(frameworkClass);

      if ([bundlePath isEqualToString: GSPrivateExecutablePath()])
	{
	  /* Oops ... the NSFramework_xxx class is linked in the main
	   * executable.  Maybe the framework was statically linked
	   * into the application ... resort to searching the
	   * framework bundle on the filesystem manually.
	   */
	  bundlePath = nil;
	}

      if (bundlePath != nil)
	{
	  NSString *pathComponent;

	  /* bundlePath should really be an absolute path; we
	   * recommend you use only absolute paths in LD_LIBRARY_PATH.
	   *
	   * If it isn't, we try to survive the situation; we assume
	   * it's relative to the launch directory.  That's how the
	   * dynamic linker would have found it after all.  This is
	   * fragile though, so please use absolute paths.
	   */
	  if ([bundlePath isAbsolutePath] == NO)
	    {
	      bundlePath = [_launchDirectory
			     stringByAppendingPathComponent: bundlePath];

	    }

	  /* Dereference symlinks, and standardize path.  This will
	   * only work properly if the original bundlePath is
	   * absolute.
	   */
	  bundlePath = [bundlePath stringByStandardizingPath];

	  /* We now have the location of the shared library object
	   * file inside the framework directory.  We need to walk up
	   * the directory tree up to the top of the framework.  To do
	   * so, we need to chop off the extra subdirectories, the
	   * library combo and the target cpu/os if they exist.  The
	   * framework and this library should match so we can use the
	   * compiled-in settings.
	   */
	  /* library name */
	  bundlePath = [bundlePath stringByDeletingLastPathComponent];
	  /* library combo */
	  pathComponent = [bundlePath lastPathComponent];
	  if ([pathComponent isEqual: library_combo])
	    {
	      bundlePath = [bundlePath stringByDeletingLastPathComponent];
	    }
	  /* target directory */
	  pathComponent = [bundlePath lastPathComponent];
	  if ([pathComponent isEqual: gnustep_target_dir])
	    {
	      bundlePath = [bundlePath stringByDeletingLastPathComponent];
	    }
#if defined(_WIN32)
	  /* On windows, the library (dll) is in the Tools area rather than
	   * in the framework, so we can adjust the path here.
	   */
	  if ([[bundlePath lastPathComponent] isEqual: @"Tools"])
	    {
	      bundlePath = [bundlePath stringByDeletingLastPathComponent];
	      bundlePath
		= [bundlePath stringByAppendingPathComponent: @"Library"];
	      bundlePath
		= [bundlePath stringByAppendingPathComponent: @"Frameworks"];
	      bundlePath = [bundlePath stringByAppendingPathComponent:
		[NSString stringWithFormat: @"%@%@", name, @".framework"]];
	    }
#else
	  /* There are no Versions on MinGW.  So the version check is only
	   * done on non-MinGW.  */
	  /* version name */
	  bundlePath = [bundlePath stringByDeletingLastPathComponent];

	  pathComponent = [bundlePath lastPathComponent];
          if ([pathComponent isEqual: @"Versions"])
	    {
	      bundlePath = [bundlePath stringByDeletingLastPathComponent];
#endif
	      pathComponent = [bundlePath lastPathComponent];

	      if ([pathComponent isEqualToString:
				   [NSString stringWithFormat: @"%@%@",
					     name, @".framework"]])
		{
		  /* Try creating the bundle.  */
		  if (bundlePath)
		    bundle = [[self alloc] initWithPath: bundlePath];
		}
#if !defined(_WIN32)
	    }
#endif

	  /* Failed - buu - try the fallback trick.  */
	  if (bundle == nil)
	    {
	      bundlePath = nil;
	    }
	}

      if (bundlePath == nil)
	{
	  /* NICOLA: In an ideal world, the following is just a hack
	   * for when GSPrivateSymbolPath() fails!  But in real life
	   * GSPrivateSymbolPath() is risky (some platforms don't
	   * have it at all!), so this hack might be used a lot!  It
	   * must be quite robust.  We try to look for the framework
	   * in the standard GNUstep installation dirs and in the main
	   * bundle.  This should be reasonably safe if the user is
	   * not being too clever ... :-)
	  */
          bundlePath = _find_framework(name);
	  if (bundlePath == nil)
	    {
	      bundlePath = [[NSBundle mainBundle] pathForResource: name
						  ofType: @"framework"
						  inDirectory: @"Frameworks"];
	    }

	  /* Try creating the bundle.  */
	  if (bundlePath != nil)
	    {
	      bundle = [[self alloc] initWithPath: bundlePath];
	    }
	}

      [load_lock lock];
      if (bundle == nil)
	{
          NSMapInsert(_byClass, frameworkClass, [NSNull null]);
          [load_lock unlock];
	  NSWarnMLog (@"Could not find framework %@ in any standard location",
	    name);
	  return nil;
	}
      else
        {
          bundle->_principalClass = frameworkClass;
          NSMapInsert(_byClass, frameworkClass, bundle);
          [load_lock unlock];
        }

      bundle->_bundleType = NSBUNDLE_FRAMEWORK;
      bundle->_codeLoaded = YES;
      /* frameworkVersion is something like 'A'.  */
      bundle->_frameworkVersion = RETAIN([frameworkClass frameworkVersion]);
      bundle->_bundleClasses = RETAIN([NSMutableArray arrayWithCapacity: 2]);

      /* A NULL terminated list of class names - the classes contained
	 in the framework.  */
      fmClasses = [frameworkClass frameworkClasses];

      while (*fmClasses != NULL)
	{
	  NSValue *value;
	  Class    class = NSClassFromString(*fmClasses);

          NSMapInsert(_byClass, class, bundle);
	  value = [NSValue valueWithPointer: (void*)class];
	  [bundle->_bundleClasses addObject: value];

	  fmClasses++;
	}

      /* If _loadingBundle is not nil, it means we reached this point
       * while loading a bundle.  This can happen if the framework is
       * linked into the bundle (then, the dynamic linker
       * automatically drags in the framework when the bundle is
       * loaded).  But then, the classes in the framework should be
       * removed from the list of classes in the bundle. Check that
       * _loadingBundle != bundle which happens on Windows machines when
       * loading in Frameworks.
       */
      if (_loadingBundle != nil && _loadingBundle != bundle)
	{
	  int i, j;
          id b = bundle->_bundleClasses;
          id l = _loadingBundle->_bundleClasses;

	  /* The following essentially does:
	   *
	   * [_loadingBundle->_bundleClasses
	   *  removeObjectsInArray: bundle->_bundleClasses];
	   *
	   * The problem with that code is isEqual: gets
	   * sent to the classes, which will cause them to be
	   * initialized (which should not happen.)
	   */
	  for (i = 0; i < [b count]; i++)
	    {
	      for (j = 0; j < [l count]; j++)
		{
		  if ([[l objectAtIndex: j] pointerValue]
		     == [[b objectAtIndex: i] pointerValue])
		    {
		      [l removeObjectAtIndex: j];
		    }
		}
	    }
	}
    }
  return bundle;
}

+ (NSMutableArray*) _addFrameworks
{
  int                   i;
  int                   numClasses = 0;
  int                   newNumClasses;
  Class                 *classes = NULL;
  NSMutableArray        *added = nil;

  newNumClasses = objc_getClassList(NULL, 0);
  while (numClasses < newNumClasses)
    {
      numClasses = newNumClasses;
      classes = realloc(classes, sizeof(Class) * numClasses);
      newNumClasses = objc_getClassList(classes, numClasses);
    }
  for (i = 0; i < numClasses; i++)
    {
      NSBundle  *bundle = [self _addFrameworkFromClass: classes[i]];

      if (nil != bundle)
        {
          if (nil == added)
            {
              added = [NSMutableArray arrayWithCapacity: 100];
            }
          [added addObject: bundle];
        }
    }
  free(classes);
  return added;
}

+ (NSString*) _gnustep_target_cpu
{
  return gnustep_target_cpu;
}

+ (NSString*) _gnustep_target_dir
{
  return gnustep_target_dir;
}

+ (NSString*) _gnustep_target_os
{
  return gnustep_target_os;
}

+ (NSString*) _library_combo
{
  return library_combo;
}

@end

/*
  Mirko:

  The gnu-runtime calls the +load method of each class before the
  _bundle_load_callback() is called and we can't provide the list of classes
  ready for this method.

 */

static void
_bundle_load_callback(Class theClass, struct objc_category *theCategory)
{
  const char *className;
  NSCAssert(_loadingBundle, NSInternalInconsistencyException);
  NSCAssert(_loadingFrameworks, NSInternalInconsistencyException);

  /* We never record categories - if this is a category, just do nothing.  */
  if (theCategory != 0)
    {
      return;
    }
  className = class_getName(theClass);

  /* Don't store the internal NSFramework_xxx class into the list of
     bundle classes, but store the linked frameworks in _loadingFrameworks  */
  if (strlen (className) > 12
    &&  !strncmp ("NSFramework_", className, 12))
    {
      if (_currentFrameworkName)
	{
	  const char *frameworkName;

	  frameworkName = [_currentFrameworkName cString];

	  if (!strcmp(className, frameworkName))
	    return;
	}

      [_loadingFrameworks
	addObject: [NSValue valueWithPointer: (void*)theClass]];
      return;
    }

  /* Store classes (but don't store categories) */
  [(_loadingBundle)->_bundleClasses addObject:
    [NSValue valueWithPointer: (void*)theClass]];
}


@implementation NSBundle

+ (void) atExit
{
  if ([NSObject shouldCleanUp])
    {
      DESTROY(_emptyTable);
      DESTROY(langAliases);
      DESTROY(langCanonical);
      DESTROY(_bundles);
      DESTROY(_byClass);
      DESTROY(_byIdentifier);
      DESTROY(pathCache);
      DESTROY(pathCacheLock);
      DESTROY(load_lock);
      DESTROY(gnustep_target_cpu);
      DESTROY(gnustep_target_os);
      DESTROY(gnustep_target_dir);
      DESTROY(library_combo);
      DESTROY(_launchDirectory);
      DESTROY(_gnustep_bundle);
      DESTROY(_mainBundle);
    }
}

+ (void) initialize
{
  if (self == [NSBundle class])
    {
      extern const char	*GSPathHandling(const char *);
      NSAutoreleasePool *pool = [NSAutoreleasePool new];
      NSString          *file;
      const char	*mode;
      NSDictionary	*env;
      NSString		*str;

      /* Ensure we do 'right' path handling while initializing core paths.
       */
      mode = GSPathHandling("right");
      _emptyTable = [NSDictionary new];

      /* Create basic mapping dictionaries for bootstrapping and
       * for use if the full dictionaries can't be loaded from the
       * gnustep-base library resource bundle.
       */
      langAliases = [[NSDictionary alloc] initWithObjectsAndKeys:
        @"Dutch", @"nl",
        @"English", @"en",
        @"Esperanto", @"eo",
        @"French", @"fr",
        @"German", @"de",
        @"Hungarian", @"hu",
        @"Italian", @"it",
        @"Korean", @"ko",
        @"Russian", @"ru",
        @"Slovak", @"sk",
        @"Spanish", @"es",
        @"TraditionalChinese", @"zh",
        @"Ukrainian", @"uk",
        nil];
      langCanonical = [[NSDictionary alloc] initWithObjectsAndKeys:
        @"de", @"German",
        @"de", @"ger",
        @"de", @"deu",
        @"en", @"English",
        @"en", @"eng",
        @"ep", @"Esperanto",
        @"ep", @"epo",
        @"ep", @"epo",
        @"fr", @"French",
        @"fr", @"fra",
        @"fr", @"fre",
        @"hu", @"Hungarian",
        @"hu", @"hun",
        @"it", @"Italian",
        @"it", @"ita",
        @"ko", @"Korean",
        @"ko", @"kir",
        @"nl", @"Dutch",
        @"nl", @"dut",
        @"nl", @"nld",
        @"ru", @"Russian",
        @"ru", @"rus",
        @"sk", @"Slovak",
        @"sk", @"slo",
        @"sk", @"slk",
        @"sp", @"Spanish",
        @"sp", @"spa",
        @"uk", @"Ukrainian",
        @"uk", @"ukr",
        @"zh", @"TraditionalChinese",
        @"zh", @"chi",
        @"zh", @"zho",
        nil];

      /* Initialise manager here so it's thread-safe.
       */
      manager();

      /* Set up tables for bundle lookups
       */
      _bundles = NSCreateMapTable(NSObjectMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);
      _byClass = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);
      _byIdentifier = NSCreateMapTable(NSObjectMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);

      pathCacheLock = [NSLock new];
      [pathCacheLock setName: @"pathCacheLock"];
      pathCache = [NSMutableDictionary new];

      /* Need to make this recursive since both mainBundle and
       * initWithPath: want to lock the thread.
       */
      load_lock = [NSRecursiveLock new];
      [load_lock setName: @"load_lock"];
      env = [[NSProcessInfo processInfo] environment];

      /* These variables are used when we are running non-flattened.
       * This means that there are multiple binaries for different
       * OSes, and we need constantly to choose the right one (eg,
       * when loading a bundle or a framework).  The choice is based
       * on these environments variables that are set by GNUstep.sh
       * (you must source GNUstep.sh when non-flattened).
       */
      if ((str = [env objectForKey: @"GNUSTEP_TARGET_CPU"]) != nil)
	gnustep_target_cpu = RETAIN(str);
      else if ((str = [env objectForKey: @"GNUSTEP_HOST_CPU"]) != nil)
	gnustep_target_cpu = RETAIN(str);

      if ((str = [env objectForKey: @"GNUSTEP_TARGET_OS"]) != nil)
	gnustep_target_os = RETAIN(str);
      else if ((str = [env objectForKey: @"GNUSTEP_HOST_OS"]) != nil)
	gnustep_target_os = RETAIN(str);

      if ((str = [env objectForKey: @"GNUSTEP_TARGET_DIR"]) != nil)
	gnustep_target_dir = RETAIN(str);
      else if ((str = [env objectForKey: @"GNUSTEP_HOST_DIR"]) != nil)
	gnustep_target_dir = RETAIN(str);
      else if (gnustep_target_cpu != nil && gnustep_target_os != nil)
	gnustep_target_dir = [[NSString alloc] initWithFormat: @"%@-%@",
          gnustep_target_cpu, gnustep_target_os];

      if ((str = [env objectForKey: @"LIBRARY_COMBO"]) != nil)
	library_combo = RETAIN(str);

      _launchDirectory = RETAIN([manager() currentDirectoryPath]);

      _gnustep_bundle = RETAIN([self bundleForLibrary: @"gnustep-base"
					      version: _base_version]);

      /* The Locale aliases map converts canonical names to old-style names
       */
      file = [_gnustep_bundle pathForResource: @"Locale"
                                       ofType: @"aliases"
                                  inDirectory: @"Languages"];
      if (file != nil)
        {
          NSDictionary  *d;

          d = [[NSDictionary alloc] initWithContentsOfFile: file];
          if ([d count] > 0)
            {
              ASSIGN(langAliases, d);
            }
          [d release];
        }

      /* The Locale canonical map converts old-style names to ISO 639 names
       * and converts ISO 639-2 names to the preferred ISO 639-1 names where
       * an ISO 639-1 name exists.
       */
      file = [_gnustep_bundle pathForResource: @"Locale"
                                       ofType: @"canonical"
                                  inDirectory: @"Languages"];
      if (file != nil)
        {
          NSDictionary  *d;

          d = [[NSDictionary alloc] initWithContentsOfFile: file];
          if ([d count] > 0)
            {
              ASSIGN(langCanonical, d);
            }
          [d release];
        }

#if 0
      _loadingBundle = [self mainBundle];
      handle = objc_open_main_module(stderr);
      printf("%08x\n", handle);
#endif
      [self _addFrameworks];
#if 0
      //  _bundle_load_callback(class, NULL);
      //  bundle = (NSBundle *)NSMapGet(_bundles, bundlePath);

      objc_close_main_module(handle);
      _loadingBundle = nil;
#endif
      GSPathHandling(mode);
      [pool release];
      [self registerAtExit];
    }
}

/**
 * Returns an array of all the bundles which do not belong to frameworks.<br />
 * This always contains the main bundle.
 */
+ (NSArray *) allBundles
{
  NSMutableArray	*array = [NSMutableArray arrayWithCapacity: 2];
  NSMapEnumerator	enumerate;
  void		        *key;
  NSBundle		*bundle;

  [load_lock lock];
  if (!_mainBundle)
    {
      [self mainBundle];
    }

  enumerate = NSEnumerateMapTable(_bundles);
  while (NSNextMapEnumeratorPair(&enumerate, &key, (void **)&bundle))
    {
      if (bundle->_bundleType == NSBUNDLE_FRAMEWORK)
        {
          continue;
        }
      if ([array indexOfObjectIdenticalTo: bundle] == NSNotFound)
        {
          [array addObject: bundle];
        }
    }
  NSEndMapTableEnumeration(&enumerate);

  [load_lock unlock];
  return array;
}

/**
 * Returns an array containing all the known bundles representing frameworks.
 */
+ (NSArray *) allFrameworks
{
  NSMapEnumerator  enumerate;
  NSMutableArray  *array = [NSMutableArray arrayWithCapacity: 2];
  void		  *key;
  NSBundle	  *bundle;

  [load_lock lock];
  enumerate = NSEnumerateMapTable(_bundles);
  while (NSNextMapEnumeratorPair(&enumerate, &key, (void **)&bundle))
    {
      if (bundle->_bundleType == NSBUNDLE_FRAMEWORK
	&& [array indexOfObjectIdenticalTo: bundle] == NSNotFound)
	{
	  [array addObject: bundle];
	}
    }
  NSEndMapTableEnumeration(&enumerate);
  [load_lock unlock];
  return array;
}

/**
 * For an application, returns the main bundle of the application.<br />
 * For a tool, returns the main bundle associated with the tool.<br />
 * <br />
 * For an application, the structure is as follows -
 * <p>
 * The executable is Gomoku.app/ix86/linux-gnu/gnu-gnu-gnu/Gomoku
 * and the main bundle directory is Gomoku.app/.
 * </p>
 * For a tool, the structure is as follows -
 * <p>
 * The executable is xxx/Tools/ix86/linux-gnu/gnu-gnu-gnu/Control
 * and the main bundle directory is xxx/Tools/Resources/Control.
 * </p>
 * <p>(when the tool has not yet been installed, it's similar -
 * xxx/obj/ix86/linux-gnu/gnu-gnu-gnu/Control
 * and the main bundle directory is xxx/Resources/Control).
 * </p>
 * <p>(For a flattened structure, the structure is the same without the
 * ix86/linux-gnu/gnu-gnu-gnu directories).
 * </p>
 */
+ (NSBundle *) mainBundle
{
  [load_lock lock];
  if (!_mainBundle)
    {
      /* We figure out the main bundle directory by examining the location
	 of the executable on disk.  */
      NSString *path, *s;

      /* We don't know at the beginning if it's a tool or an application.  */
      BOOL isApplication = YES;

      /* Sometimes we detect that this is a non-installed tool.  That is
       * special because we want to lookup local resources before installed
       * ones.  Keep track of this special case in this variable.
       */
      BOOL isNonInstalledTool = NO;

      /* If it's a tool, we will need the tool name.  Since we don't
         know yet if it's a tool or an application, we always store
         the executable name here - just in case it turns out it's a
         tool.  */
      NSString *toolName = [GSPrivateExecutablePath() lastPathComponent];
#if defined(_WIN32) || defined(__CYGWIN__)
      toolName = [toolName stringByDeletingPathExtension];
#endif

      /* Strip off the name of the program */
      path = [GSPrivateExecutablePath() stringByDeletingLastPathComponent];

      /* We now need to chop off the extra subdirectories, the library
	 combo and the target directory if they exist.  The executable
	 and this library should match so that is why we can use the
	 compiled-in settings. */
      /* library combo */
      s = [path lastPathComponent];
      if ([s isEqual: library_combo])
	{
	  path = [path stringByDeletingLastPathComponent];
	}
      /* target dir */
      s = [path lastPathComponent];
      if ([s isEqual: gnustep_target_dir])
	{
	  path = [path stringByDeletingLastPathComponent];
	}
      /* object dir */
      s = [path lastPathComponent];
      if ([s hasSuffix: @"obj"])
	{
	  path = [path stringByDeletingLastPathComponent];
	  /* if it has an object dir it can only be a
             non-yet-installed tool.  */
	  isApplication = NO;
	  isNonInstalledTool = YES;
	}

#ifndef __ANDROID__ /* don't check suffix on Android's fake executable path */
      if (isApplication == YES)
	{
	  s = [path lastPathComponent];

	  if ([s hasSuffix: @".app"] == NO
	    && [s hasSuffix: @".debug"] == NO
	    && [s hasSuffix: @".profile"] == NO
	    && [s hasSuffix: @".gswa"] == NO	// GNUstep Web
	    && [s hasSuffix: @".woa"] == NO	// GNUstep Web
	    )
	    {
	      NSFileManager	*mgr = manager();
	      BOOL		f;

	      /* Not one of the common extensions, but
	       * might be an app wrapper with another extension...
	       * Look for Info-gnustep.plist or Info.plist in a
	       * Resources subdirectory.
	       */
	      s = [path stringByAppendingPathComponent: @"Resources"];
	      if ([mgr fileExistsAtPath: s isDirectory: &f] == NO || f == NO)
		{
		  isApplication = NO;
		}
	      else
		{
		  NSString	*i;

		  i = [s stringByAppendingPathComponent: @"Info-gnustep.plist"];
		  if ([mgr isReadableFileAtPath: i] == NO)
		    {
		      i = [s stringByAppendingPathComponent: @"Info.plist"];
		      if ([mgr isReadableFileAtPath: i] == NO)
			{
			  isApplication = NO;
			}
		    }
		}
	    }
	}
#endif /* !__ANDROID__ */

      if (isApplication == NO)
	{
	  NSString *maybePath = nil;

	  if (isNonInstalledTool)
	    {
	      /* We're pretty confident about this case.  'path' is
	       * obtained by {tool location on disk} and walking up
	       * until we got out of the obj directory.  So we're
	       * now in GNUSTEP_BUILD_DIR.  Resources will be in
	       * Resources/{toolName}.
	       */
	      path = [path stringByAppendingPathComponent: @"Resources"];
	      maybePath = [path stringByAppendingPathComponent: toolName];

	      /* PS: We could check here if we found the resources,
	       * and if not, keep going with the other attempts at
	       * locating them.  But if we know that this is an
	       * uninstalled tool, really we don't want to use
	       * installed resources - we prefer resource lookup to
	       * fail so the developer will fix whatever issue they
	       * have with their building.
	       */
	    }
	  else
	    {
	      if (maybePath == nil)
		{
		  /* This is for gnustep-make version 2, where tool resources
		   * are in GNUSTEP_*_LIBRARY/Tools/Resources/{toolName}.
		   */
		  maybePath = _find_main_bundle_for_tool (toolName);
		}

	      /* If that didn't work, maybe the tool was created with
	       * gnustep-make version 1.  So we try {tool location on
	       * disk after walking up the non-flattened
	       * dirs}/Resources/{toolName}, which is where
	       * gnustep-make version 1 would put resources.
	       */
	      if (maybePath == nil)
		{
		  path = [path stringByAppendingPathComponent: @"Resources"];
		  maybePath = [path stringByAppendingPathComponent: toolName];
		}
	    }

	  path = maybePath;
	}

      NSDebugMLLog(@"NSBundle", @"Found main in %@\n", path);
      /* We do alloc and init separately so initWithPath: knows we are
          the _mainBundle.  Please note that we do *not* autorelease
          mainBundle, because we don't want it to be ever released.  */
      _mainBundle = [self alloc];
      /* Please note that _mainBundle should *not* be nil.  */
      _mainBundle = [_mainBundle initWithPath: path];
      NSAssert(_mainBundle != nil, NSInternalInconsistencyException);
    }

  [load_lock unlock];
  return _mainBundle;
}

/**
 * Returns the bundle whose code contains the specified class.<br />
 * NB: We will not find a class if the bundle has not been loaded yet!
 */
+ (NSBundle *) bundleForClass: (Class)aClass
{
  void		*key;
  NSBundle	*bundle;
  NSMapEnumerator enumerate;

  if (!aClass)
    return nil;

  /* This is asked relatively frequently inside gnustep-base itself;
   * shortcut it.
   */
  if (aClass == [NSObject class])
    {
      if (nil != _gnustep_bundle)
	{
	  return _gnustep_bundle;
	}
    }

  [load_lock lock];
  /* Try lookup ... if not found, make sure that all loaded bundles have
   * class->bundle map entries set up and check again.
   */
  bundle = (NSBundle *)NSMapGet(_byClass, aClass);
  if ((id)bundle == (id)[NSNull null])
    {
      [load_lock unlock];
      return nil;
    }
  if (nil == bundle)
    {
      enumerate = NSEnumerateMapTable(_bundles);
      while (NSNextMapEnumeratorPair(&enumerate, &key, (void **)&bundle))
        {
          NSArray           *classes = bundle->_bundleClasses;
          NSUInteger        count = [classes count];

          if (count > 0
            && 0 == NSMapGet(_byClass, [[classes lastObject] pointerValue]))
            {
              while (count-- > 0)
                {
                  NSMapInsert(_byClass,
                    (void*)[[classes objectAtIndex: count] pointerValue],
                    (void*)bundle);
                }
            }
        }
      NSEndMapTableEnumeration(&enumerate);
      bundle = (NSBundle *)NSMapGet(_byClass, aClass);
      if ((id)bundle == (id)[NSNull null])
        {
          [load_lock unlock];
          return nil;
        }
    }

  if (nil == bundle)
    {
      /* Is it in the main bundle or a library? */
      if (!class_isMetaClass(aClass))
        {
	  NSString	*lib;

	  /*
	   * Take the path to the binary containing the class and
	   * convert it to the format for a library name as used for
	   * obtaining a library resource bundle.
	   */
	  lib = GSPrivateSymbolPath(aClass);
	  if ([lib isEqual: GSPrivateExecutablePath()] == YES)
	    {
	      lib = nil;	// In program, not library.
	    }

	  /*
	   * Get the library bundle ... if there wasn't one then we
	   * will check to see if it's in a newly loaded framework
           * and if not, assume the class was in the program executable
	   * and return the mainBundle instead.
	   */
	  bundle = [NSBundle bundleForLibrary: lib];
          if (nil == bundle && [[self _addFrameworks] count] > 0)
            {
              bundle = (NSBundle *)NSMapGet(_byClass, aClass);
              if ((id)bundle == (id)[NSNull null])
                {
                  [load_lock unlock];
                  return nil;
                }
            }
	  if (nil == bundle)
	    {
	      bundle = [self mainBundle];
	    }

	  /*
	   * Add the class to the list of classes known to be in the
	   * library or executable.  We didn't find it there to start
	   * with, so we know it's safe to add now.
	   */
	  if (bundle->_bundleClasses == nil)
	    {
	      bundle->_bundleClasses
		= [[NSMutableArray alloc] initWithCapacity: 2];
	    }
	  [bundle->_bundleClasses addObject:
	    [NSValue valueWithPointer: (void*)aClass]];
	}
    }
  [load_lock unlock];

  return bundle;
}

+ (NSBundle*) bundleWithPath: (NSString*)path
{
  return AUTORELEASE([[self alloc] initWithPath: path]);
}

+ (NSBundle*) bundleWithURL: (NSURL*)url
{
  return AUTORELEASE([[self alloc] initWithURL: url]);
}

+ (NSBundle*) bundleWithIdentifier: (NSString*)identifier
{
  NSBundle	*bundle = nil;

  [load_lock lock];
  if (_byIdentifier)
    {
      bundle = (NSBundle *)NSMapGet(_byIdentifier, identifier);
IF_NO_GC(
	[bundle retain]; /* retain - look as if we were alloc'ed */
)
    }
  [load_lock unlock];
  // Some OS X apps  try to get the foundation bundle by looking it up by
  // identifier.  This is expected to be faster than looking it up by class, so
  // we lazily insert it into the table if it's requested.
  if (nil == bundle && [@"com.apple.Foundation" isEqualToString: identifier])
    {
      NSBundle *foundation = [self bundleForClass: self];
      [load_lock lock];
      NSMapInsert(_byIdentifier, @"com.apple.Foundation", foundation);
      [load_lock unlock];
      return foundation;
    }
  return AUTORELEASE(bundle);
}

- (id) initWithPath: (NSString*)path
{
  NSString	*identifier;
  NSBundle	*bundle;

  self = [super init];

  if (!path || [path length] == 0)
    {
      NSDebugMLog(@"No path specified for bundle");
      [self dealloc];
      return nil;
    }

  /*
   * Make sure we have an absolute and fully expanded path,
   * so we can manipulate it without having to worry about
   * details like that throughout the code.
   */

  /* 1. make path absolute.
   */
  if ([path isAbsolutePath] == NO)
    {
      NSWarnMLog(@"NSBundle -initWithPath: requires absolute path names, "
	@"given '%@'", path);

#if defined(_WIN32)
      if ([path length] > 0 &&
	([path characterAtIndex: 0]=='/' || [path characterAtIndex: 0]=='\\'))
	{
	  NSString	*root;
	  unsigned	length;

	  /* The path has a leading path separator, so we try assuming
	   * that it's a path on the current filesystem, and append it
	   * to the filesystem root.
	   */
	  root = [manager() currentDirectoryPath];
	  length = [root length];
	  root = [root stringByDeletingLastPathComponent];
	  while ([root length] != length)
	    {
	      length = [root length];
	      root = [root stringByDeletingLastPathComponent];
	    }
	  path = [root stringByAppendingPathComponent: path];
	}
      else
	{
	  /* Try appending to the current working directory.
	   */
	  path = [[manager() currentDirectoryPath]
	    stringByAppendingPathComponent: path];
	}
#else
      path = [[manager() currentDirectoryPath]
        stringByAppendingPathComponent: path];
#endif
    }

  /* 2. Expand any symbolic links.
   */
  path = [path stringByResolvingSymlinksInPath];

  /* 3. Standardize so we can be sure that cache lookup is consistent.
   */
  path = [path stringByStandardizingPath];

  /* check if we were already initialized for this directory */
  [load_lock lock];
  bundle = (NSBundle *)NSMapGet(_bundles, path);
  if (bundle != nil)
    {
      IF_NO_GC([bundle retain];)
      [load_lock unlock];
      [self dealloc];
      return bundle;
    }
  [load_lock unlock];

  if (bundle_directory_readable(path) == nil)
    {
      NSDebugMLLog(@"NSBundle", @"Could not access path %@ for bundle", path);
      // if this is not the main bundle ... deallocate and return.
      if (self != _mainBundle)
	{
	  [self dealloc];
	  return nil;
	}
    }

  /* OK ... this is a new bundle ... need to insert it in the global map
   * to be found by this path so that a leter call to -bundleIdentifier
   * can work.
   */
  _path = [path copy];
  [load_lock lock];
  NSMapInsert(_bundles, _path, self);
  [load_lock unlock];

  if ([[[_path lastPathComponent] pathExtension] isEqual: @"framework"] == YES)
    {
      _bundleType = (unsigned int)NSBUNDLE_FRAMEWORK;
    }
  else
    {
      if (self == _mainBundle)
	_bundleType = (unsigned int)NSBUNDLE_APPLICATION;
      else
	_bundleType = (unsigned int)NSBUNDLE_BUNDLE;
    }

  identifier = [self bundleIdentifier];

  [load_lock lock];
  if (identifier != nil)
    {
      NSBundle	*bundle = (NSBundle *)NSMapGet(_byIdentifier, identifier);

      if (bundle != self)
        {
          if (bundle != nil)
            {
              IF_NO_GC([bundle retain];)
              [load_lock unlock];
              [self dealloc];
              return bundle;
            }
          NSMapInsert(_byIdentifier, identifier, self);
        }
    }
  [load_lock unlock];

  return self;
}

- (id) initWithURL: (NSURL*)url
{
  // FIXME
  return [self initWithPath: [url path]];
}

- (void) dealloc
{
  if ([self isLoaded] == YES && self != _mainBundle
    && self ->_bundleType != NSBUNDLE_LIBRARY)
    {
      /*
       * Prevent unloading of bundles where code has been loaded ...
       * the objc runtime does not currently support unloading of
       * dynamically loaded code, so we want to prevent a bundle
       * being loaded twice.
       */
      IF_NO_GC([self retain];)
      return;
    }
  if (_path != nil)
    {
      NSString		*identifier = [self bundleIdentifier];
      NSUInteger        count;

      [load_lock lock];
      if (_bundles != nil)
        {
          NSMapRemove(_bundles, _path);
        }
      if (identifier != nil)
        {
	  NSMapRemove(_byIdentifier, identifier);
        }
      if (_principalClass != nil)
        {
	  NSMapRemove(_byClass, _principalClass);
        }
      if (_byClass != nil)
        {
          count = [_bundleClasses count];
          while (count-- > 0)
            {
              NSMapRemove(_byClass,
                [[_bundleClasses objectAtIndex: count] pointerValue]);
            }
        }
      [load_lock unlock];

      /* Clean up path cache for this bundle.
       */
      [self cleanPathCache];
      RELEASE(_path);
    }
  TEST_RELEASE(_frameworkVersion);
  TEST_RELEASE(_bundleClasses);
  TEST_RELEASE(_infoDict);
  TEST_RELEASE(_localizations);
  [super dealloc];
}

- (NSString*) description
{
  return  [[super description] stringByAppendingFormat:
    @" <%@>%@", [self bundlePath], [self isLoaded] ? @" (loaded)" : @""];
}

- (NSString*) bundlePath
{
  return _path;
}

- (NSURL*) bundleURL
{
  return [NSURL fileURLWithPath: [self bundlePath]];
}

- (Class) classNamed: (NSString *)className
{
  int     i, j;
  Class   theClass = Nil;

  if (!_codeLoaded)
    {
      if (self != _mainBundle && ![self load])
	{
	  NSLog(@"No classes in bundle");
	  return Nil;
	}
    }

  if (self == _mainBundle || self == _gnustep_bundle)
    {
      theClass = NSClassFromString(className);
      if (theClass && [[self class] bundleForClass: theClass] != self)
        {
	  theClass = Nil;
	}
    }
  else
    {
      BOOL found = NO;

      theClass = NSClassFromString(className);
      [load_lock lock];
      j = [_bundleClasses count];

      for (i = 0; i < j  &&  found == NO; i++)
	{
	  Class c = (Class)[[_bundleClasses objectAtIndex: i] pointerValue];

	  if (c == theClass)
	    {
	      found = YES;
	    }
	}
      [load_lock unlock];

      if (found == NO)
	{
	  theClass = Nil;
	}
    }

  return theClass;
}

- (Class) principalClass
{
  NSString	*class_name;

  if (_principalClass)
    {
      return _principalClass;
    }

  if ([self load] == NO)
    {
      return Nil;
    }

  class_name = [[self infoDictionary] objectForKey: @"NSPrincipalClass"];

  if (class_name)
    {
      _principalClass = NSClassFromString(class_name);
    }
  else if (self == _gnustep_bundle)
    {
      _principalClass = [NSObject class];
    }

  if (_principalClass == nil)
    {
      [load_lock lock];
      if (_principalClass == nil && [_bundleClasses count] > 0)
	{
	  _principalClass = (Class)[[_bundleClasses objectAtIndex: 0]
	    pointerValue];
	}
      [load_lock unlock];
    }
  return _principalClass;
}

/**
 * Returns YES if the receiver's code is loaded, otherwise, returns NO.
 */
- (BOOL) isLoaded
{
  return _codeLoaded;
}

- (BOOL) load
{
  if (self == _mainBundle || self ->_bundleType == NSBUNDLE_LIBRARY)
    {
      _codeLoaded = YES;
      return YES;
    }

  [load_lock lock];

  if (_codeLoaded == NO)
    {
      NSString       *object;
      NSEnumerator   *classEnumerator;
      NSMutableArray *classNames;
      NSValue        *class;
      NSBundle       *savedLoadingBundle;

      /* Get the binary and set up fraework name if it is a framework.
       */
      object = [self executablePath];
      if (object == nil || [object length] == 0)
	{
	  [load_lock unlock];
	  return NO;
	}
      savedLoadingBundle = _loadingBundle;
      _loadingBundle = self;
      _bundleClasses = [[NSMutableArray alloc] initWithCapacity: 2];

      if (nil == savedLoadingBundle)
        {
          _loadingFrameworks = [[NSMutableArray alloc] initWithCapacity: 2];
        }

      /* This code is executed twice if a class linked in the bundle calls a
	 NSBundle method inside +load (-principalClass). To avoid this we set
	 _codeLoaded before loading the bundle. */
      _codeLoaded = YES;

      if (GSPrivateLoadModule(object, stderr, _bundle_load_callback, 0, 0))
	{
	  _codeLoaded = NO;
          _loadingBundle = savedLoadingBundle;
          if (nil == _loadingBundle)
            {
              DESTROY(_loadingFrameworks);
              DESTROY(_currentFrameworkName);
            }
	  [load_lock unlock];
	  return NO;
	}

      /* We now construct the list of bundles from frameworks linked with
	 this one */
      classEnumerator = [_loadingFrameworks objectEnumerator];
      while ((class = [classEnumerator nextObject]) != nil)
	{
	  [NSBundle _addFrameworkFromClass: (Class)[class pointerValue]];
	}

      /* After we load code from a bundle, we retain the bundle until
	 we unload it (because we never unload bundles, that is
	 forever).  The reason why we retain it is that we need it!
	 We need it to answer calls like bundleForClass:; also, users
	 normally want all loaded bundles to appear when they call
	 +allBundles.  */
      IF_NO_GC([self retain];)

      classNames = [NSMutableArray arrayWithCapacity: [_bundleClasses count]];
      classEnumerator = [_bundleClasses objectEnumerator];
      while ((class = [classEnumerator nextObject]) != nil)
	{
          NSMapInsert(_byClass, class, self);
	  [classNames addObject:
	    NSStringFromClass((Class)[class pointerValue])];
	}

      _loadingBundle = savedLoadingBundle;
      if (nil == _loadingBundle)
        {
          DESTROY(_loadingFrameworks);
          DESTROY(_currentFrameworkName);
        }
      [load_lock unlock];

      [[NSNotificationCenter defaultCenter]
        postNotificationName: NSBundleDidLoadNotification
        object: self
        userInfo: [NSDictionary dictionaryWithObject: classNames
	  forKey: NSLoadedClasses]];

      return YES;
    }
  [load_lock unlock];
  return YES;
}

- (oneway void) release
{
  /* We lock during release so that other threads can't grab the
   * object between us checking the reference count and deallocating.
   */
  [load_lock lock];
  if (NSDecrementExtraRefCountWasZero(self))
    {
      [self dealloc];
    }
  [load_lock unlock];
}

/* This method is the backbone of the resource searching for NSBundle. It
   constructs an array of paths, where each path is a possible location
   for a resource in the bundle.  The current algorithm for searching goes:

     <rootPath>/Resources/<bundlePath>
     <rootPath>/Resources/<bundlePath>/<language.lproj>
     <rootPath>/<bundlePath>
     <rootPath>/<bundlePath>/<language.lproj>
*/
+ (NSArray *) _bundleResourcePathsWithRootPath: (NSString*)rootPath
				       subPath: (NSString*)subPath
				  localization: (NSString*)localization
{
  NSString		*primary;
  NSString		*language;
  NSArray		*languages;
  NSArray		*contents;
  NSMutableArray	*array;
  NSEnumerator		*enumerate;

  array = [NSMutableArray arrayWithCapacity: 8];
  languages = [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"];

  primary = [rootPath stringByAppendingPathComponent: @"Resources"];
  contents = bundle_directory_readable(primary);
  addBundlePath(array, contents, primary, subPath, nil);
  /* If we have been asked for a specific localization, we add it.
   */
  if (localization != nil)
    {
      addBundlePath(array, contents, primary, subPath, localization);
    }
  else
    {
      /* This matches OS X behavior, which only searches languages that
       * are in the user's preference. Don't use -preferredLocalizations -
       * that would cause a recursive loop.
       */
      enumerate = [languages objectEnumerator];
      while ((language = [enumerate nextObject]))
	{
	  addBundlePath(array, contents, primary, subPath, language);
	}
    }
  
#ifdef __ANDROID__
  /* Android: check subdir and localization directly, as AAssetDir and thereby
   * NSDirectoryEnumerator doesn't list directories
   */
  NSString *originalPrimary = primary;
  if (subPath)
    {
      primary = [originalPrimary stringByAppendingPathComponent: subPath];
      contents = bundle_directory_readable(primary);
      addBundlePath(array, contents, primary, nil, nil);
      
      if (localization)
	{
	  primary = [primary stringByAppendingPathComponent:
	    [localization stringByAppendingPathExtension: @"lproj"]];
	  contents = bundle_directory_readable(primary);
	  addBundlePath(array, contents, primary, nil, nil);
	}
      else
	{
	  NSString *subPathPrimary = primary;

	  enumerate = [languages objectEnumerator];
	  while ((language = [enumerate nextObject]))
	    {
	      primary = [subPathPrimary stringByAppendingPathComponent:
		[language stringByAppendingPathExtension: @"lproj"]];
	      contents = bundle_directory_readable(primary);
	      addBundlePath(array, contents, primary, nil, nil);
	    }
	}
    }
  if (localization)
    {
      primary = [originalPrimary stringByAppendingPathComponent:
	[localization stringByAppendingPathExtension: @"lproj"]];
      contents = bundle_directory_readable(primary);
      addBundlePath(array, contents, primary, nil, nil);
    }
  else
    {
      enumerate = [languages objectEnumerator];
      while ((language = [enumerate nextObject]))
	{
	  primary = [originalPrimary stringByAppendingPathComponent:
	    [language stringByAppendingPathExtension: @"lproj"]];
	  contents = bundle_directory_readable(primary);
	  addBundlePath(array, contents, primary, nil, nil);
	}
    }
#endif /* __ANDROID__ */
  
  primary = rootPath;
  contents = bundle_directory_readable(primary);
  addBundlePath(array, contents, primary, subPath, nil);
  if (localization != nil)
    {
      addBundlePath(array, contents, primary, subPath, localization);
    }
  else
    {
      enumerate = [languages objectEnumerator];
      while ((language = [enumerate nextObject]))
	{
	  addBundlePath(array, contents, primary, subPath, language);
	}
    }
  return array;
}

+ (NSString *) _pathForResource: (NSString *)name
			 ofType: (NSString *)extension
		     inRootPath: (NSString *)rootPath
		    inDirectory: (NSString *)subPath
{
  NSFileManager	*mgr = manager();
  NSString	*path;
  NSString	*file;
  NSEnumerator	*pathlist;

  if (name == nil)
    {
      name = @"";
    }
  if ([extension length] == 0)
    {
      file = name;
    }
  else
    {
      file = [name stringByAppendingPathExtension: extension];
    }

  pathlist = [[self _bundleResourcePathsWithRootPath: rootPath
    subPath: subPath localization: nil] objectEnumerator];
  while ((path = [pathlist nextObject]) != nil)
    {
      NSArray	*paths = bundle_directory_readable(path);

      if (YES == [paths containsObject: file])
	{
	  path = [path stringByAppendingPathComponent: file];
	  if (YES == [mgr isReadableFileAtPath: path])
	    {
	      return path;
	    }
	}
    }

#ifdef __ANDROID__
  /* Android: check for directory resources by passing file path as subpath,
   * as AAssetDir and thereby NSDirectoryEnumerator doesn't list directories
   */
  subPath = subPath ? [subPath stringByAppendingPathComponent: file] : file;
  pathlist = [[self _bundleResourcePathsWithRootPath: rootPath
    subPath: subPath localization: nil] objectEnumerator];
  while ((path = [pathlist nextObject]) != nil)
    {
      NSString *lastPathComponent = [path lastPathComponent];
      if ([lastPathComponent isEqualToString:file]
        && [mgr isReadableFileAtPath: path])
        {
          return path;
        }
    }
#endif /* __ANDROID__ */

  return nil;
}


+ (NSString *) pathForResource: (NSString *)name
			ofType: (NSString *)extension
		   inDirectory: (NSString *)bundlePath
		   withVersion: (int)version
{
  return [self _pathForResource: name
			 ofType: extension
		     inRootPath: bundlePath
		    inDirectory: nil];
}

+ (NSString *) pathForResource: (NSString *)name
			ofType: (NSString *)extension
		   inDirectory: (NSString *)bundlePath
{
  return [self _pathForResource: name
			 ofType: extension
		     inRootPath: bundlePath
		    inDirectory: nil];
}

+ (NSURL*) URLForResource: (NSString*)name
            withExtension: (NSString*)extension
             subdirectory: (NSString*)subpath
          inBundleWithURL: (NSURL*)bundleURL
{
  NSBundle *root = [self bundleWithURL: bundleURL];

  return [root URLForResource: name
                withExtension: extension
                 subdirectory: subpath];
}

- (NSString *) pathForResource: (NSString *)name
			ofType: (NSString *)extension
{
  return [self pathForResource: name
			ofType: extension
		   inDirectory: nil];
}

- (NSString *) pathForResource: (NSString *)name
			ofType: (NSString *)extension
		   inDirectory: (NSString *)subPath
{
  NSString *rootPath;

#if !defined(_WIN32)
  if (_frameworkVersion)
    rootPath = [NSString stringWithFormat: @"%@/Versions/%@", [self bundlePath],
      _frameworkVersion];
  else
#endif
    rootPath = [self bundlePath];

  return [NSBundle _pathForResource: name
			     ofType: extension
			 inRootPath: rootPath
		        inDirectory: subPath];
}

- (NSURL *) URLForResource: (NSString *)name
	     withExtension: (NSString *)extension
{
  return [self URLForResource: name
                withExtension: extension
                 subdirectory: nil
                 localization: nil];
}

- (NSURL *) URLForResource: (NSString *)name
	     withExtension: (NSString *)extension
              subdirectory: (NSString *)subpath
{
  return [self URLForResource: name
                withExtension: extension
                 subdirectory: subpath
                 localization: nil];
}

- (NSURL *) URLForResource: (NSString *)name
	     withExtension: (NSString *)extension
	      subdirectory: (NSString *)subpath
	      localization: (NSString *)localizationName
{
  NSString	*path;

  path = [self pathForResource: name
			ofType: extension
		   inDirectory: subpath
	       forLocalization: localizationName];
  if (nil == path)
    {
      return nil;
    }
  return [NSURL fileURLWithPath: path];
}

+ (NSArray*) _pathsForResourcesOfType: (NSString*)extension
		      inRootDirectory: (NSString*)bundlePath
		       inSubDirectory: (NSString*)subPath
			 localization: (NSString*)localization
{
  BOOL allfiles;
  NSString *path;
  NSMutableArray *resources;
  NSEnumerator *pathlist;

  pathlist = [[NSBundle _bundleResourcePathsWithRootPath: bundlePath
    subPath: subPath localization: localization] objectEnumerator];
  resources = [NSMutableArray arrayWithCapacity: 2];
  allfiles = (extension == nil || [extension length] == 0);

  while ((path = [pathlist nextObject]))
    {
      NSEnumerator *filelist;
      NSString	*match;

      filelist = [bundle_directory_readable(path) objectEnumerator];
      while ((match = [filelist nextObject]))
	{
	  if (allfiles || [extension isEqual: [match pathExtension]])
	    [resources addObject: [path stringByAppendingPathComponent: match]];
	}
    }

  return resources;
}

+ (NSArray*) pathsForResourcesOfType: (NSString*)extension
			 inDirectory: (NSString*)bundlePath
{
  return [self _pathsForResourcesOfType: extension
			inRootDirectory: bundlePath
			 inSubDirectory: nil
			   localization: nil];
}

- (NSArray *) pathsForResourcesOfType: (NSString *)extension
			  inDirectory: (NSString *)subPath
{
  return [[self class] _pathsForResourcesOfType: extension
				inRootDirectory: [self bundlePath]
				 inSubDirectory: subPath
				   localization: nil];
}

- (NSArray*) pathsForResourcesOfType: (NSString*)extension
			 inDirectory: (NSString*)subPath
		     forLocalization: (NSString*)localizationName
{
  NSArray         *paths = nil;
  NSMutableArray  *result = nil;
  NSEnumerator    *enumerator = nil;
  NSString        *path = nil;

  result = [NSMutableArray array];
  paths = [[self class] _pathsForResourcesOfType: extension
				 inRootDirectory: [self bundlePath]
				  inSubDirectory: subPath
				    localization: localizationName];

  enumerator = [paths objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      /* Add all non-localized paths, plus ones in the particular localization
	 (if there is one). */
      NSString  *theDir = [path stringByDeletingLastPathComponent];
      NSString  *last = [theDir lastPathComponent];

      if ([[last pathExtension] isEqual: @"lproj"] == NO)
	{
	  [result addObject: path];
	}
      else
        {
          NSString      *lang = [last stringByDeletingPathExtension];
          NSArray       *alternatives = altLang(lang);

          if ([alternatives count] > 0)
            {
              [result addObject: path];
            }
	}
    }

  return result;
}

- (NSString*) pathForResource: (NSString*)name
		       ofType: (NSString*)extension
		  inDirectory: (NSString*)subPath
	      forLocalization: (NSString*)localizationName
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSString		*result = nil;
  NSArray		*array;

  if ([extension length] == 0)
    {
      extension = [name pathExtension];
      if (extension != nil)
	{
	  name = [name stringByDeletingPathExtension];
	}
    }
  array = [self pathsForResourcesOfType: extension
                            inDirectory: subPath
                        forLocalization: localizationName];

  if (array != nil)
    {
      NSEnumerator	*enumerator = [array objectEnumerator];
      NSString		*path;

      name = [name stringByAppendingPathExtension: extension];
      while ((path = [enumerator nextObject]) != nil)
	{
	  NSString	*found = [path lastPathComponent];

	  if ([found isEqualToString: name] == YES)
	    {
	      result = path;
	      break;		// localised paths occur before non-localised
	    }
	}
    }
  [result retain];
  [arp drain];
  return [result autorelease];
}

+ (NSArray *) preferredLocalizationsFromArray: (NSArray *)localizationsArray
{
  return [self preferredLocalizationsFromArray: localizationsArray
    forPreferences: [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"]];
}

+ (NSArray *) preferredLocalizationsFromArray: (NSArray *)localizationsArray
			       forPreferences: (NSArray *)preferencesArray
{
  NSString	*locale;
  NSMutableArray	*array;
  NSEnumerator	*enumerate;

  array = [NSMutableArray arrayWithCapacity: 2];
  enumerate = [preferencesArray objectEnumerator];
  while ((locale = [enumerate nextObject]))
    {
      if ([localizationsArray indexOfObject: locale] != NSNotFound)
	[array addObject: locale];
    }
  /* I guess this is arbitrary if we can't find a match? */
  if ([array count] == 0 && [localizationsArray count] > 0)
    [array addObject: [localizationsArray objectAtIndex: 0]];
  return GS_IMMUTABLE(array);
}

- (NSDictionary*) localizedInfoDictionary
{
  NSString  *path;
  NSArray   *locales;
  NSString  *locale = nil;
  NSDictionary *dict = nil;

  locales = [self preferredLocalizations];
  if ([locales count] > 0)
    locale = [locales objectAtIndex: 0];
  path = [self pathForResource: @"Info-gnustep"
                        ofType: @"plist"
                   inDirectory: nil
               forLocalization: locale];
  if (path)
    {
      dict = [[NSDictionary alloc] initWithContentsOfFile: path];
    }
  else
    {
      path = [self pathForResource: @"Info"
                            ofType: @"plist"
                       inDirectory: nil
                   forLocalization: locale];
      if (path)
	{
	  dict = [[NSDictionary alloc] initWithContentsOfFile: path];
	}
    }
  if (nil == [dict autorelease])
    {
      dict = [self infoDictionary];
    }
  return dict;
}

- (id) objectForInfoDictionaryKey: (NSString *)key
{
  return [[self infoDictionary] objectForKey: key];
}

- (NSString*) developmentLocalization
{
  return nil;
}

- (NSArray *) localizations
{
  NSString *locale;
  NSArray *localizations;
  NSEnumerator* enumerate;
  NSMutableArray *array = [NSMutableArray arrayWithCapacity: 2];

  localizations = [self pathsForResourcesOfType: @"lproj"
	                            inDirectory: nil];
  enumerate = [localizations objectEnumerator];
  while ((locale = [enumerate nextObject]))
    {
      locale = [[locale lastPathComponent] stringByDeletingPathExtension];
      [array addObject: locale];
    }

#ifdef __ANDROID__
    // Android: Check known languages for localizations directly, as AAssetDir
    // and thereby NSDirectoryEnumerator doesn't list directories and the above
    // call to list lproj resources will therefore come up empty.
    NSArray *languages = [[NSUserDefaults standardUserDefaults]
      stringArrayForKey: @"NSLanguages"];
    
    for (locale in languages)
      {
	NSString *path = [self pathForResource: @"Localizable"
					ofType: @"strings"
				   inDirectory: nil
			       forLocalization: locale];
	if (path)
	  {
	    [array addObject: locale];
	  }
      }
#endif /* __ANDROID__ */

  return GS_IMMUTABLE(array);
}

- (NSArray *) preferredLocalizations
{
  return [NSBundle preferredLocalizationsFromArray: [self localizations]];
}

- (NSString *) localizedStringForKey: (NSString *)key
                               value: (NSString *)value
                               table: (NSString *)tableName
{
  NSDictionary	*table;
  NSString	*newString = nil;

  if (_localizations == nil)
    _localizations = [[NSMutableDictionary alloc] initWithCapacity: 1];

  if (tableName == nil || [tableName isEqualToString: @""] == YES)
    {
      tableName = @"Localizable";
      table = [_localizations objectForKey: tableName];
    }
  else if ((table = [_localizations objectForKey: tableName]) == nil
    && [@"strings" isEqual: [tableName pathExtension]] == YES)
    {
      tableName = [tableName stringByDeletingPathExtension];
      table = [_localizations objectForKey: tableName];
    }

  if (table == nil)
    {
      NSString	*tablePath;

      /*
       * Make sure we have an empty table in place in case anything
       * we do somehow causes recursion.  The recursive call will look
       * up the string in the empty table.
       */
      [_localizations setObject: _emptyTable forKey: tableName];

      tablePath = [self pathForResource: tableName ofType: @"strings"];
      if (tablePath != nil)
        {
          NSStringEncoding	encoding;
          NSString		*tableContent;
          NSData		*tableData;
          const unsigned char	*bytes;
          unsigned		length;

          tableData = [[NSData alloc] initWithContentsOfFile: tablePath];
          bytes = [tableData bytes];
          length = [tableData length];
          /*
           * A localisation file can be ...
           * UTF16 with a leading BOM,
           * UTF8 with a leading BOM,
           * or ASCII (the original standard) with \U escapes.
           */
          if (length > 2
              && ((bytes[0] == 0xFF && bytes[1] == 0xFE)
                  || (bytes[0] == 0xFE && bytes[1] == 0xFF)))
            {
              encoding = NSUnicodeStringEncoding;
            }
          else if (length > 2
                   && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
            {
              encoding = NSUTF8StringEncoding;
            }
          else
            {
              encoding = NSASCIIStringEncoding;
            }
          tableContent = [[NSString alloc] initWithData: tableData
                                           encoding: encoding];
          if (tableContent == nil && encoding == NSASCIIStringEncoding)
            {
              encoding = [NSString defaultCStringEncoding];
              tableContent = [[NSString alloc] initWithData: tableData
                                               encoding: encoding];
              if (tableContent != nil)
                {
                  NSWarnMLog (@"Localisation file %@ not in portable encoding"
		    @" so I'm using the default encoding for the current"
		    @" system, which may not display messages correctly.\n"
		    @"The file should be ASCII (using \\U escapes for unicode"
		    @" characters) or Unicode (UTF16 or UTF8) with a leading "
		    @"byte-order-marker.\n", tablePath);
                }
            }
          if (tableContent == nil)
            {
              NSWarnMLog(@"Failed to load strings file %@ - bad character"
                         @" encoding", tablePath);
            }
          else
            {
              NS_DURING
                {
                  table = [tableContent propertyListFromStringsFileFormat];
                }
              NS_HANDLER
                {
                  NSWarnMLog(@"Failed to parse strings file %@ - %@",
                             tablePath, localException);
                }
              NS_ENDHANDLER
            }
          RELEASE(tableData);
          RELEASE(tableContent);
        }
      else
        {
          NSDebugMLLog(@"NSBundle", @"Failed to locate strings file %@",
                       tableName);
        }
      /*
       * If we couldn't found and parsed the strings table, we put it in
       * the cache of strings tables in this bundle, otherwise we will just
       * be keeping the empty table in the cache so we don't keep retrying.
       */
      if (table != nil)
        [_localizations setObject: table forKey: tableName];
    }

  if (key == nil || (newString = [table objectForKey: key]) == nil)
    {
      NSString	*show = [[NSUserDefaults standardUserDefaults]
                            objectForKey: NSShowNonLocalizedStrings];
      if (show && [show isEqual: @"YES"])
        {
          /* It would be bad to localize this string! */
          NSLog(@"Non-localized string: %@\n", key);
          newString = [key uppercaseString];
        }
      else
        {
          newString = value;
          if (newString == nil || [newString isEqualToString: @""] == YES)
            newString = key;
        }
      if (newString == nil)
        newString = @"";
    }

  return newString;
}

+ (void) stripAfterLoading: (BOOL)flag
{
  _strip_after_loading = flag;
}

- (NSArray *) executableArchitectures
{
  return nil;
}
- (BOOL) preflightAndReturnError: (NSError **)error
{
  return NO;
}
- (BOOL) loadAndReturnError: (NSError **)error
{
  return NO;
}


- (NSString *) executablePath
{
  NSString *object, *path;

  if (!_mainBundle)
    {
      [NSBundle mainBundle];
    }
  if (self == _mainBundle)
    {
      return GSPrivateExecutablePath();
    }
  if (self->_bundleType == NSBUNDLE_LIBRARY)
    {
      return GSPrivateSymbolPath([self principalClass]);
    }
  object = [[self infoDictionary] objectForKey: @"NSExecutable"];
  if (object == nil || [object length] == 0)
    {
      object = [[self infoDictionary] objectForKey: @"CFBundleExecutable"];
      if (object == nil || [object length] == 0)
	{
	  return nil;
	}
    }
  if (_bundleType == NSBUNDLE_FRAMEWORK)
    {
      /* Mangle the name before building the _currentFrameworkName,
       * which really is a class name.
       */
      NSString *mangledName = object;
      mangledName = [mangledName stringByReplacingString: @"_"
				 withString: @"__"];
      mangledName = [mangledName stringByReplacingString: @"-"
				 withString: @"_0"];
      mangledName = [mangledName stringByReplacingString: @"+"
				 withString: @"_1"];

#if !defined(_WIN32)
      path = [_path stringByAppendingPathComponent: @"Versions/Current"];
#else
      path = _path;
#endif

      _currentFrameworkName = RETAIN(([NSString stringWithFormat:
						  @"NSFramework_%@",
						mangledName]));
    }
  else
    {
      path = _path;
    }

  object = bundle_object_name(path, object);
  return object;
}

- (NSURL *) executableURL
{
  return [NSURL fileURLWithPath: [self executablePath]];
}

- (NSString *) pathForAuxiliaryExecutable: (NSString *) executableName
{
  NSString  *version = _frameworkVersion;

  if (!version)
    version = @"Current";

  if (_bundleType == NSBUNDLE_FRAMEWORK)
    {
#if !defined(_WIN32)
      return [_path stringByAppendingPathComponent:
                      [NSString stringWithFormat: @"Versions/%@/%@",
                      version, executableName]];
#else
      return [_path stringByAppendingPathComponent: executableName];
#endif
    }
  else
    {
      return [_path stringByAppendingPathComponent: executableName];
    }
}

- (NSURL *) URLForAuxiliaryExecutable: (NSString *) executableName
{
  return [NSURL fileURLWithPath: [self pathForAuxiliaryExecutable:
                       executableName]];
}

- (NSString *) resourcePath
{
  NSString *version = _frameworkVersion;

  if (!version)
    version = @"Current";

  if (_bundleType == NSBUNDLE_FRAMEWORK)
    {
#if !defined(_WIN32)
      return [_path stringByAppendingPathComponent:
		      [NSString stringWithFormat: @"Versions/%@/Resources",
				version]];
#else
      /* No Versions (that require symlinks) on mswindows */
      return [_path stringByAppendingPathComponent: @"Resources"];
#endif
    }
  else
    {
      return [_path stringByAppendingPathComponent: @"Resources"];
    }
}

- (NSURL *) resourceURL
{
  return [NSURL fileURLWithPath: [self resourcePath]];
}


- (NSDictionary *) infoDictionary
{
  NSString* path;

  if (_infoDict)
    return _infoDict;

  path = [self pathForResource: @"Info-gnustep" ofType: @"plist"];
  if (path)
    {
      _infoDict = [[NSDictionary alloc] initWithContentsOfFile: path];
    }
  else
    {
      path = [self pathForResource: @"Info" ofType: @"plist"];
      if (path)
	{
	  _infoDict = [[NSDictionary alloc] initWithContentsOfFile: path];
	}
      else
	{
	  _infoDict = RETAIN([NSDictionary dictionary]);
	}
    }
  return _infoDict;
}

- (NSString *) builtInPlugInsPath
{
  NSString  *version = _frameworkVersion;

  if (!version)
    version = @"Current";

  if (_bundleType == NSBUNDLE_FRAMEWORK)
    {
#if !defined(_WIN32)
      return [_path stringByAppendingPathComponent:
                      [NSString stringWithFormat: @"Versions/%@/PlugIns",
                      version]];
#else
      return [_path stringByAppendingPathComponent: @"PlugIns"];
#endif
    }
  else
    {
      return [_path stringByAppendingPathComponent: @"PlugIns"];
    }
}

- (NSURL *) builtInPlugInsURL
{
  return [NSURL fileURLWithPath: [self builtInPlugInsPath]];
}

- (NSString *) privateFrameworksPath
{
  NSString  *version = _frameworkVersion;

  if (!version)
    version = @"Current";

  if (_bundleType == NSBUNDLE_FRAMEWORK)
    {
#if !defined(_WIN32)
      return [_path stringByAppendingPathComponent:
	[NSString stringWithFormat: @"Versions/%@/PrivateFrameworks",
	version]];
#else
      return [_path stringByAppendingPathComponent: @"PrivateFrameworks"];
#endif
    }
  else
    {
      return [_path stringByAppendingPathComponent: @"PrivateFrameworks"];
    }
}

- (NSURL *) privateFrameworksURL
{
  return [NSURL fileURLWithPath: [self privateFrameworksPath]];
}


- (NSString*) bundleIdentifier
{
  return [[self infoDictionary] objectForKey: @"CFBundleIdentifier"];
}

- (unsigned) bundleVersion
{
  return _version;
}

- (void) setBundleVersion: (unsigned)version
{
  _version = version;
}

- (BOOL) unload
{
  return NO;
}
@end

@implementation NSBundle (GNUstep)

+ (NSBundle *) bundleForLibrary: (NSString *)libraryName
{
  return [self bundleForLibrary: libraryName  version: nil];
}

+ (NSBundle *) bundleForLibrary: (NSString *)libraryName
			version: (NSString *)interfaceVersion
{
  /* Important: if you change this code, make sure to also
   * change NSUserDefault's manual gnustep-base resource
   * lookup to match.
   */
  NSArray *paths;
  NSEnumerator *enumerator;
  NSString *path;
  NSFileManager *fm = manager();
  NSRange	r;

  if ([libraryName length] == 0)
    {
      return nil;
    }
  /*
   * Eliminate any base path or extensions.
   */
  libraryName = [libraryName lastPathComponent];

#if defined(_WIN32)
  /* A dll is usually of the form 'xxx-maj_min.dll'
   * so we can extract the version info and use it.
   */
  if ([[libraryName pathExtension] isEqual: @"dll"])
    {
      libraryName = [libraryName stringByDeletingPathExtension];
      r = [libraryName rangeOfString: @"-" options: NSBackwardsSearch];
      if (r.length > 0)
	{
	  NSString	*ver;

	  ver = [[libraryName substringFromIndex: NSMaxRange(r)]
	    stringByReplacingString: @"_" withString: @"."];
	  libraryName = [libraryName substringToIndex: r.location];
	  if (interfaceVersion == nil)
	    {
	      interfaceVersion = ver;
	    }
	}
    }
#elif defined(__APPLE__)
  /* A .dylib is usually of the form 'libxxx.maj.min.sub.dylib',
   * but GNUstep-make installs them with 'libxxx.dylib.maj.min.sub'.
   * For maximum compatibility with support both forms here.
   */
  if ([[libraryName pathExtension] isEqual: @"dylib"])
    {
      NSString	*s = [libraryName stringByDeletingPathExtension];
      NSArray	*a = [s componentsSeparatedByString: @"."];

      if ([a count] > 1)
	{
	  libraryName = [a objectAtIndex: 0];
	  if (interfaceVersion == nil && [a count] >= 3)
	    {
	      interfaceVersion = [NSString stringWithFormat: @"%@.%@",
		[a objectAtIndex: 1], [a objectAtIndex: 2]];
	    }
	}
    }
  else
    {
      r = [libraryName rangeOfString: @".dylib."];
      if (r.length > 0)
	{
	  NSString *s = [libraryName substringFromIndex: NSMaxRange(r)];
	  NSArray  *a = [s componentsSeparatedByString: @"."];

	  libraryName = [libraryName substringToIndex: r.location];
	  if (interfaceVersion == nil && [a count] >= 2)
	    {
	      interfaceVersion = [NSString stringWithFormat: @"%@.%@",
		[a objectAtIndex: 0], [a objectAtIndex: 1]];
	    }
	}
    }
#else
  /* A .so is usually of the form 'libxxx.so.maj.min.sub'
   * so we can extract the version info and use it.
   */
  r = [libraryName rangeOfString: @".so."];
  if (r.length > 0)
    {
      NSString	*s = [libraryName substringFromIndex: NSMaxRange(r)];
      NSArray	*a = [s componentsSeparatedByString: @"."];

      libraryName = [libraryName substringToIndex: r.location];
      if (interfaceVersion == nil && [a count] >= 2)
	{
	  interfaceVersion = [NSString stringWithFormat: @"%@.%@",
	    [a objectAtIndex: 0], [a objectAtIndex: 1]];
	}
    }
#endif

  while ([[libraryName pathExtension] length] > 0)
    {
      libraryName = [libraryName stringByDeletingPathExtension];
    }

  /*
   * Discard leading 'lib'
   */
  if ([libraryName hasPrefix: @"lib"] == YES)
    {
      libraryName = [libraryName substringFromIndex: 3];
    }

  if ([libraryName length] == 0)
    {
      return nil;
    }

  /*
   * We expect to find the library resources in the GNUSTEP_LIBRARY domain in:
   *
   * Libraries/<libraryName>/Versions/<interfaceVersion>/Resources/
   *
   * if no <interfaceVersion> is specified, and if can't find any versioned
   * resources in those directories, we'll also accept the old unversioned
   * subdirectory:
   *
   * Libraries/Resources/<libraryName>/
   *
   */
  paths = NSSearchPathForDirectoriesInDomains (NSLibraryDirectory,
					       NSAllDomainsMask, YES);

  enumerator = [paths objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      NSBundle	*b;
      BOOL isDir;
      path = [path stringByAppendingPathComponent: @"Libraries"];

      if ([fm fileExistsAtPath: path  isDirectory: &isDir]  &&  isDir)
	{
	  /* As a special case, if we have been asked to get the base
	   * library bundle without a version, we check to see if the
	   * bundle for the current version is available and use that
	   * in preference to all others.
	   * This lets older code (using the non-versioned api) work
	   * on systems where multiple versions are installed.
	   */
	  if (interfaceVersion == nil
	    && [libraryName isEqualToString: @"gnustep-base"])
	    {
	      NSString	*p;

	      p = [[[[path stringByAppendingPathComponent: libraryName]
			 stringByAppendingPathComponent: @"Versions"]
			stringByAppendingPathComponent: _base_version]
		       stringByAppendingPathComponent: @"Resources"];
	      if ([fm fileExistsAtPath: p  isDirectory: &isDir]  &&  isDir)
	        {
		  interfaceVersion = _base_version;
		}
	    }

	  if (interfaceVersion != nil)
	    {
	      /* We're looking for a specific version.  */
	      path = [[[[path stringByAppendingPathComponent: libraryName]
			 stringByAppendingPathComponent: @"Versions"]
			stringByAppendingPathComponent: interfaceVersion]
		       stringByAppendingPathComponent: @"Resources"];
	      if ([fm fileExistsAtPath: path  isDirectory: &isDir]  &&  isDir)
		{
		  b = [self bundleWithPath: path];

		  if (b != nil && b->_bundleType == NSBUNDLE_BUNDLE)
		    {
		      b->_bundleType = NSBUNDLE_LIBRARY;
		    }
		  return b;
		}
	    }
	  else
	    {
	      /* Any version will do.  */
	      NSString *versionsPath;

	      versionsPath
		= [[path stringByAppendingPathComponent: libraryName]
			 stringByAppendingPathComponent: @"Versions"];

	      if ([fm fileExistsAtPath: versionsPath  isDirectory: &isDir]
	        && isDir)
		{
		  /* TODO: Ignore subdirectories.  */
		  NSEnumerator *fileEnumerator;
		  NSString *potentialPath;

		  fileEnumerator = [fm enumeratorAtPath: versionsPath];
		  while ((potentialPath = [fileEnumerator nextObject]) != nil)
		    {
		      potentialPath = [potentialPath
			stringByAppendingPathComponent: @"Resources"];
		      potentialPath = [versionsPath
			stringByAppendingPathComponent: potentialPath];
		      if ([fm fileExistsAtPath: potentialPath
				   isDirectory: &isDir]  &&  isDir)
			{
			  b = [self bundleWithPath: potentialPath];

			  if (b != nil && b->_bundleType == NSBUNDLE_BUNDLE)
			    {
			      b->_bundleType = NSBUNDLE_LIBRARY;
			    }
			  return b;
			}
		    }
		}

	      /* We didn't find anything!  For backwards
	       * compatibility, try the unversioned directory itself:
	       * we used to put library resources directly in
	       * unversioned directories such as
	       * GNUSTEP_LIBRARY/Libraries/Resources/gnustep-base/{resources
	       * here}.  This was deprecated/obsoleted on 9 March 2007
	       * when we added library resource versioning.
	       */
	      {
		NSString *oldResourcesPath;

		oldResourcesPath = [path
		  stringByAppendingPathComponent: @"Resources"];
		oldResourcesPath = [oldResourcesPath
		  stringByAppendingPathComponent: libraryName];
		if ([fm fileExistsAtPath: oldResourcesPath
		  isDirectory: &isDir]  &&  isDir)
		  {
		    b = [self bundleWithPath: oldResourcesPath];
		    if (b != nil && b->_bundleType == NSBUNDLE_BUNDLE)
		      {
			b->_bundleType = NSBUNDLE_LIBRARY;
		      }
		    return b;
		  }
	      }
	    }
	}
    }

  return nil;
}

+ (NSString *) pathForLibraryResource: (NSString *)name
			       ofType: (NSString *)extension
			  inDirectory: (NSString *)bundlePath
{
  NSString	*path = nil;
  NSString	*bundle_path = nil;
  NSArray	*paths;
  NSBundle	*bundle;
  NSEnumerator	*enumerator;

  /* Gather up the paths */
  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                              NSAllDomainsMask, YES);

  enumerator = [paths objectEnumerator];
  while ((path == nil) && (bundle_path = [enumerator nextObject]))
    {
      bundle = [self bundleWithPath: bundle_path];
      path = [bundle pathForResource: name
                              ofType: extension
                         inDirectory: bundlePath];
    }

  return path;
}

- (void)cleanPathCache
{
  NSUInteger	plen = [_path length];
  NSEnumerator	*enumerator;
  NSString		*path;
  
  [pathCacheLock lock];
  enumerator = [pathCache keyEnumerator];
  while (nil != (path = [enumerator nextObject]))
  {
    if (YES == [path hasPrefix: _path])
      {
        if ([path length] == plen)
        {
          /* Remove the bundle directory path from the cache.
           */
          [pathCache removeObjectForKey: path];
        }
        else
        {
          unichar	c = [path characterAtIndex: plen];

          /* if the directory is inside the bundle, remove from cache.
           */
          if ('/' == c)
          {
            [pathCache removeObjectForKey: path];
          }
#if defined(_WIN32)
          else if ('\\' == c)
          {
            [pathCache removeObjectForKey: path];
          }
#endif
        }
      }
  }
  [pathCacheLock unlock];
  
  /* also destroy cached variables depending on bundle paths */
  DESTROY(_infoDict);
  DESTROY(_localizations);
}

#ifdef __ANDROID__

+ (AAssetManager *)assetManager
{
  return _assetManager;
}

+ (void) setJavaAssetManager: (jobject)jassetManager withJNIEnv: (JNIEnv *)env
{ 
  /* create global reference to Java asset manager to prevent garbage
   * collection
   */
  _jassetManager = (*env)->NewGlobalRef(env, jassetManager);
  
  // get native asset manager (may be shared across multiple threads)
  _assetManager = AAssetManager_fromJava(env, _jassetManager);

  // clean main bundle path cache in case it was accessed before
  [_mainBundle cleanPathCache];
}

+ (AAsset *) assetForPath: (NSString *)path
{
  return [self assetForPath: path withMode: AASSET_MODE_UNKNOWN];
}

+ (AAsset *) assetForPath: (NSString *)path withMode: (int)mode
{
  AAsset *asset = NULL;
  
  if (_assetManager && _mainBundle)
    {
      NSString *resourcePath = [_mainBundle resourcePath];

      if ([path hasPrefix: resourcePath]
	&& [path length] > [resourcePath length])
	{
	  NSString *assetPath;

	  assetPath = [path substringFromIndex: [resourcePath length] + 1];
	  asset = AAssetManager_open(_assetManager,
	    [assetPath fileSystemRepresentation], mode);
	}
    }
  
  return asset;
}

+ (AAssetDir *) assetDirForPath: (NSString *)path
{
  AAssetDir *assetDir = NULL;
  
  if (_assetManager && _mainBundle)
    {
      NSString *resourcePath = [_mainBundle resourcePath];

      if ([path hasPrefix: resourcePath])
	{
	  NSString *assetPath = @"";

	  if ([path length] > [resourcePath length])
	    {
	      assetPath = [path substringFromIndex: [resourcePath length] + 1];
	    }

	  assetDir = AAssetManager_openDir(_assetManager,
	    [assetPath fileSystemRepresentation]);
	  
	  if (assetDir)
	    {
	      /* AAssetManager_openDir() always returns an object,
	       * so we check if the directory exists by ensuring
	       * it contains a file
	       */
	      BOOL exists = AAssetDir_getNextFileName(assetDir) != NULL;
	      if (exists)
		{
		  AAssetDir_rewind(assetDir);
		}
	      else
		{
		  AAssetDir_close(assetDir);
		  assetDir = NULL;
		}
	    }
	}
    }
  
  return assetDir;
}

#endif /* __ANDROID__ */

@end

