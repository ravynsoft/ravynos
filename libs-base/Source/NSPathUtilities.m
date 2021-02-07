/* Implementation of filesystem & path-related functions for GNUstep
   Copyright (C) 1996-2016 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <address@hidden>
   Created: May 1996
   Rewrite by:  Sheldon Gill
   Date:    Jan 2004
   Rewrites by:  Richard Frith-Macdonald
   Date:    2004-2005

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

   <title>NSPathUtilities function reference</title>
   $Date$ $Revision$
   */

/**
   <unit>
   <heading>Path Utility Functions</heading>
   <p>
   Path utilities provides functions to dynamically discover paths
   for the platform the application is running on.
   This avoids the need for hard coding paths, making porting easier
   and also allowing for places to change without breaking
   applications.
   (why do this? Well imagine we're running GNUstep 1 and the new
   wonderful GNUstep 2 becomes available but we're not sure of it
   yet. You could install /GNUstep/System2/ and have applications
   use which ever System you wanted at the time...)
   </p>
   <p>
   On unix systems, the paths are initialised by reading a configuration
   file. Something like "/etc/GNUstep/GNUstep.conf". This provides the basic
   information required by the library to establish all locations required.
   </p>
   <p>
   See <REF "filesystem.pdf">GNUstep File System Hierarchy</REF> document
   for more information and detailed descriptions.</p>
   </unit>
*/

/* The following define is needed for Solaris get(pw/gr)(nam/uid)_r declartions
   which default to pre POSIX declaration.  */
#define _POSIX_PTHREAD_SEMANTICS

#import "common.h"
#include "objc-load.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSException.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSBundle.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSUserDefaults.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

#import "GSPrivate.h"

#ifdef	HAVE_PWD_H
#include <pwd.h>		// for getpwnam_r() and getpwuid_r()
#endif
#include <sys/types.h>
#include <stdio.h>

NSMutableDictionary* GNUstepConfig(NSDictionary *newConfig);

void GNUstepUserConfig(NSMutableDictionary *config, NSString *userName);


/* The global configuration file. The real value is read from config.h */
#ifndef GNUSTEP_TARGET_CONFIG_FILE
# define   GNUSTEP_TARGET_CONFIG_FILE  "/etc/GNUstep/GNUstep.conf"
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
static NSString	*gnustep_is_flattened =
#ifdef GNUSTEP_IS_FLATTENED
  @GNUSTEP_IS_FLATTENED;
#else
  nil;
#endif

#if	defined(_WIN32)

#include	<sddl.h>
#include	<lmaccess.h>

/*
 * FIXME ... should check access properly if the file is on an NTFS volume.
 */
#define	ATTRMASK	0700
#else
#define	ATTRMASK	0777
#endif

#define	MGR()	[NSFileManager defaultManager]

/* ------------------ */
/* Internal variables */
/* ------------------ */

static NSString	*gnustepConfigPath = nil;

static NSString *gnustepUserHome = nil;
static NSString *gnustepUserID = nil;
static NSString *gnustepUserName = nil;
static NSString *gnustepUserDefaultsDir = nil;

static NSString *theUserName = nil;             /* The user's login name */
static NSString *theFullUserName = nil;         /* The user's full login name */
static NSString *tempDir = nil;                 /* user's temporary directory */

/* The following list entirely describe our filesystem configuration.  */
static NSString *gnustepMakefiles = nil;

static NSString *gnustepSystemUsersDir = nil;
static NSString *gnustepNetworkUsersDir = nil;
static NSString *gnustepLocalUsersDir = nil;

static NSString *gnustepSystemApps = nil;
static NSString *gnustepSystemAdminApps = nil;
static NSString *gnustepSystemWebApps = nil;
static NSString *gnustepSystemTools = nil;
static NSString *gnustepSystemAdminTools = nil;
static NSString *gnustepSystemLibrary = nil;
static NSString *gnustepSystemLibraries = nil;
static NSString *gnustepSystemHeaders = nil;
static NSString *gnustepSystemDocumentation = nil;
static NSString *gnustepSystemDocumentationInfo = nil;
static NSString *gnustepSystemDocumentationMan = nil;

static NSString *gnustepNetworkApps = nil;
static NSString *gnustepNetworkAdminApps = nil;
static NSString *gnustepNetworkWebApps = nil;
static NSString *gnustepNetworkTools = nil;
static NSString *gnustepNetworkAdminTools = nil;
static NSString *gnustepNetworkLibrary = nil;
static NSString *gnustepNetworkLibraries = nil;
static NSString *gnustepNetworkHeaders = nil;
static NSString *gnustepNetworkDocumentation = nil;
static NSString *gnustepNetworkDocumentationInfo = nil;
static NSString *gnustepNetworkDocumentationMan = nil;

static NSString *gnustepLocalApps = nil;
static NSString *gnustepLocalAdminApps = nil;
static NSString *gnustepLocalWebApps = nil;
static NSString *gnustepLocalTools = nil;
static NSString *gnustepLocalAdminTools = nil;
static NSString *gnustepLocalLibrary = nil;
static NSString *gnustepLocalLibraries = nil;
static NSString *gnustepLocalHeaders = nil;
static NSString *gnustepLocalDocumentation = nil;
static NSString *gnustepLocalDocumentationInfo = nil;
static NSString *gnustepLocalDocumentationMan = nil;

static NSString *gnustepUserApps = nil;
static NSString *gnustepUserAdminApps = nil;
static NSString *gnustepUserWebApps = nil;
static NSString *gnustepUserTools = nil;
static NSString *gnustepUserAdminTools = nil;
static NSString *gnustepUserLibrary = nil;
static NSString *gnustepUserLibraries = nil;
static NSString *gnustepUserHeaders = nil;
static NSString *gnustepUserDocumentation = nil;
static NSString *gnustepUserDocumentationInfo = nil;
static NSString *gnustepUserDocumentationMan = nil;

static NSString	*gnustepDeveloperDir = nil;

/* These are the same as the corresponding User variables, but
 * they hold the path before GNUSTEP_HOME is prepended.  It's what
 * we read from config files.
 */
static NSString *gnustepUserDirApps = nil;
static NSString *gnustepUserDirAdminApps = nil;
static NSString *gnustepUserDirWebApps = nil;
static NSString *gnustepUserDirTools = nil;
static NSString *gnustepUserDirAdminTools = nil;
static NSString *gnustepUserDirLibrary = nil;
static NSString *gnustepUserDirLibraries = nil;
static NSString *gnustepUserDirHeaders = nil;
static NSString *gnustepUserDirDocumentation = nil;
static NSString *gnustepUserDirDocumentationInfo = nil;
static NSString *gnustepUserDirDocumentationMan = nil;

static NSString	*uninstalled = nil;

static BOOL ParseConfigurationFile(NSString *name, NSMutableDictionary *dict,
  NSString *userName);

static void InitialisePathUtilities(void);
static void ShutdownPathUtilities(void);

@interface GSPathUtilities : NSObject
@end

/* Conditionally assign an object from a dictionary to var
 * We don't need to retain val before releasing var, because we
 * can be sure that if var is val it is retained by the dictionary
 * as well as being retained when it was first placed in var.
 */
#define ASSIGN_IF_SET(var, dictionary, key) ({\
  id val = [dictionary objectForKey: key];\
  if (val != nil)\
    {\
      RELEASE(var);\
      var = RETAIN(val);\
      [dictionary removeObjectForKey: key];\
    }\
})

/* Grab a path from the config file, making it relative to the config
 * file location if it is '.' or begins with './' or '../', and checking
 * that the result is an absolute path.
 */
#define ASSIGN_PATH(var, dictionary, key) ({\
  id val = getPathConfig(dictionary, key);\
  if (val != nil)\
    {\
      RELEASE(var);\
      var = RETAIN(val);\
      [dictionary removeObjectForKey: key];\
    }\
})

/* Like ASSIGN_PATH(), but permits the result to be a relative path as that
 * is what we normally use (the path is within the user's home directory).
 */
#define ASSIGN_USER(var, dictionary, key) ({\
  id val = [dictionary objectForKey: key];\
  if (val != nil)\
    {\
      val = getPath(val);\
    }\
  if (val != nil)\
    {\
      RELEASE(var);\
      var = RETAIN(val);\
      [dictionary removeObjectForKey: key];\
    }\
})

/* Conditionally assign lval to var only if var is nil

lval must be casted because Clang disallows ObjC literals comparison */
#define TEST_ASSIGN(var, lval) ({\
  if ((var == nil)&&((NSString *)lval != nil))\
    {\
      var = RETAIN(lval);\
    }\
  })

#define ASSIGN_DEFAULT_PATH(var, default) ({\
  if (var == nil)\
    {\
      var = default; \
      var = RETAIN(getPath(var)); \
    }\
  })

/* The user domain paths are normally located within the user's home directory,
 * but if they are specified as absolute paths they can be anywhere you like.
 *
 */
#define ASSIGN_USER_PATH(var, val) ({\
  if (nil == var) \
    {\
      ASSIGN(var, ([val isAbsolutePath] ? substUser(val) : \
        [gnustepUserHome stringByAppendingPathComponent: substUser(val)]));\
    }\
  })

/* For user domain paths, we allow '%u' as the username, '%i' as the userid,
 * and '%%' as a '%' in cases where it would otherwise be treated as a
 * username  or userid.
 */
static NSString*
substUser(NSString *str)
{
  if (str != nil && [str rangeOfString: @"%"].length > 0)
    {
      NSMutableString	*m = [[str mutableCopy] autorelease];
      int		l = [m length];
      int		i = 0;
      BOOL		percent = NO;

      while (i < l)
	{
          unichar	c = [m characterAtIndex: i];

	  if (YES == percent)
	    {
	      NSString	*s;

	      if (c == '%')
		{
		  s = @"%";		// Escaped percent
		}
	      else if (c == 'i')
		{
		  s = gnustepUserID;	// User ID
		}
	      else if (c == 'u')
		{
		  s = gnustepUserName;	// User name
		}
	      else
		{
		  s = nil;		// No substitution
		}
	      if (s != nil)
		{
		  int	diff = [s length] - 2;

		  [m replaceCharactersInRange: NSMakeRange(i-1, 2)
				   withString: s];
		  l += diff;
		  i += diff;
		}
	      percent = NO;
	    }
	  else if (c == '%')
	    {
	      percent = YES;
	    }
	  i++;
	}
      str = m;
    }
  return str;
}

/* Get a full path string */
static inline NSString *
getPath(NSString *path)
{
  if ([path hasPrefix: @"./"] == YES)
    {
      path = [gnustepConfigPath stringByAppendingPathComponent:
	[path substringFromIndex: 2]];
      path = [path stringByStandardizingPath];
    }
  else if ([path hasPrefix: @"../"] == YES)
    {
      path = [gnustepConfigPath stringByAppendingPathComponent: path];
      path = [path stringByStandardizingPath];
    }
  return path;
}

/* Get a full path string from a dictionary */
static inline NSString *
getPathConfig(NSDictionary *dict, NSString *key)
{
  NSString	*path;

  path = [dict objectForKey: key];
  if (path != nil)
    {
      path = getPath(path);
      if ([path isAbsolutePath] == NO)
	{
	  fprintf(stderr, "GNUstep configuration file entry '%s' ('%s') is not "
	    "an absolute path.\nPlease fix your configuration file.\n",
	    [key UTF8String],
            [[[dict objectForKey: key] description] UTF8String]);
#if	defined(_WIN32)
	  if ([path length] > 2)
	    {
	      unichar	buf[3];

	      [path getCharacters: buf range: NSMakeRange(0, 3)];
	      if ((buf[0] == '/' || buf[0] == '\\') && isalpha(buf[1])
		&& (buf[2] == '/' || buf[2] == '\\'))
		{
		  path = [NSString stringWithFormat: @"%c:%@", (char)buf[1],
		    [path substringFromIndex: 2]];
		  path = [path stringByReplacingString: @"/"
					    withString: @"\\"];
		  fprintf(stderr, "I am guessing that you meant '%s'\n",
                    [path UTF8String]);
		}
	    }
#endif
	}
    }
  return path;
}

static void ExtractValuesFromConfig(NSDictionary *config)
{
  NSMutableDictionary	*c = [config mutableCopy];
  BOOL			createLibraryPath;
  id		        extra;

  /*
   * Move values out of the dictionary and into variables for rapid reference.
   */
  ASSIGN_IF_SET(gnustepUserDefaultsDir, c,
    @"GNUSTEP_USER_DEFAULTS_DIR");

  ASSIGN_PATH(gnustepMakefiles, c,
    @"GNUSTEP_MAKEFILES");

  ASSIGN_PATH(gnustepDeveloperDir, c,
    @"GNUSTEP_DEVELOPER_DIR");

  ASSIGN_PATH(gnustepSystemUsersDir, c,
    @"GNUSTEP_SYSTEM_USERS_DIR");
  ASSIGN_PATH(gnustepNetworkUsersDir, c,
    @"GNUSTEP_NETWORK_USERS_DIR");
  ASSIGN_PATH(gnustepLocalUsersDir, c,
    @"GNUSTEP_LOCAL_USERS_DIR");

  ASSIGN_PATH(gnustepSystemApps, c,
    @"GNUSTEP_SYSTEM_APPS");
  ASSIGN_PATH(gnustepSystemAdminApps, c,
    @"GNUSTEP_SYSTEM_ADMIN_APPS");
  ASSIGN_PATH(gnustepSystemWebApps, c,
    @"GNUSTEP_SYSTEM_WEB_APPS");
  ASSIGN_PATH(gnustepSystemTools, c,
    @"GNUSTEP_SYSTEM_TOOLS");
  ASSIGN_PATH(gnustepSystemAdminTools, c,
    @"GNUSTEP_SYSTEM_ADMIN_TOOLS");
  ASSIGN_PATH(gnustepSystemLibrary, c,
    @"GNUSTEP_SYSTEM_LIBRARY");
  ASSIGN_PATH(gnustepSystemLibraries, c,
    @"GNUSTEP_SYSTEM_LIBRARIES");
  ASSIGN_PATH(gnustepSystemHeaders, c,
    @"GNUSTEP_SYSTEM_HEADERS");
  ASSIGN_PATH(gnustepSystemDocumentation, c,
    @"GNUSTEP_SYSTEM_DOC");
  ASSIGN_PATH(gnustepSystemDocumentationMan, c,
    @"GNUSTEP_SYSTEM_DOC_MAN");
  ASSIGN_PATH(gnustepSystemDocumentationInfo, c,
    @"GNUSTEP_SYSTEM_DOC_INFO");

  ASSIGN_PATH(gnustepNetworkApps, c,
    @"GNUSTEP_NETWORK_APPS");
  ASSIGN_PATH(gnustepNetworkAdminApps, c,
    @"GNUSTEP_NETWORK_ADMIN_APPS");
  ASSIGN_PATH(gnustepNetworkWebApps, c,
    @"GNUSTEP_NETWORK_WEB_APPS");
  ASSIGN_PATH(gnustepNetworkTools, c,
    @"GNUSTEP_NETWORK_TOOLS");
  ASSIGN_PATH(gnustepNetworkAdminTools, c,
    @"GNUSTEP_NETWORK_ADMIN_TOOLS");
  ASSIGN_PATH(gnustepNetworkLibrary, c,
    @"GNUSTEP_NETWORK_LIBRARY");
  ASSIGN_PATH(gnustepNetworkLibraries, c,
    @"GNUSTEP_NETWORK_LIBRARIES");
  ASSIGN_PATH(gnustepNetworkHeaders, c,
    @"GNUSTEP_NETWORK_HEADERS");
  ASSIGN_PATH(gnustepNetworkDocumentation, c,
    @"GNUSTEP_NETWORK_DOC");
  ASSIGN_PATH(gnustepNetworkDocumentationMan, c,
    @"GNUSTEP_NETWORK_DOC_MAN");
  ASSIGN_PATH(gnustepNetworkDocumentationInfo, c,
    @"GNUSTEP_NETWORK_DOC_INFO");

  ASSIGN_PATH(gnustepLocalApps, c,
    @"GNUSTEP_LOCAL_APPS");
  ASSIGN_PATH(gnustepLocalAdminApps, c,
    @"GNUSTEP_LOCAL_ADMIN_APPS");
  ASSIGN_PATH(gnustepLocalWebApps, c,
    @"GNUSTEP_LOCAL_WEB_APPS");
  ASSIGN_PATH(gnustepLocalTools, c,
    @"GNUSTEP_LOCAL_TOOLS");
  ASSIGN_PATH(gnustepLocalAdminTools, c,
    @"GNUSTEP_LOCAL_ADMIN_TOOLS");
  ASSIGN_PATH(gnustepLocalLibrary, c,
    @"GNUSTEP_LOCAL_LIBRARY");
  ASSIGN_PATH(gnustepLocalLibraries, c,
    @"GNUSTEP_LOCAL_LIBRARIES");
  ASSIGN_PATH(gnustepLocalHeaders, c,
    @"GNUSTEP_LOCAL_HEADERS");
  ASSIGN_PATH(gnustepLocalDocumentation, c,
    @"GNUSTEP_LOCAL_DOC");
  ASSIGN_PATH(gnustepLocalDocumentationMan, c,
    @"GNUSTEP_LOCAL_DOC_MAN");
  ASSIGN_PATH(gnustepLocalDocumentationInfo, c,
    @"GNUSTEP_LOCAL_DOC_INFO");

  ASSIGN_USER(gnustepUserDirApps, c,
    @"GNUSTEP_USER_DIR_APPS");
  TEST_ASSIGN(gnustepUserDirApps,
    @GNUSTEP_TARGET_USER_DIR_APPS);
  ASSIGN_USER(gnustepUserDirAdminApps, c,
    @"GNUSTEP_USER_DIR_ADMIN_APPS");
  TEST_ASSIGN(gnustepUserDirAdminApps,
    @GNUSTEP_TARGET_USER_DIR_ADMIN_APPS);
  ASSIGN_USER(gnustepUserDirWebApps, c,
    @"GNUSTEP_USER_DIR_WEB_APPS");
  TEST_ASSIGN(gnustepUserDirWebApps,
    @GNUSTEP_TARGET_USER_DIR_WEB_APPS);
  ASSIGN_USER(gnustepUserDirTools, c,
    @"GNUSTEP_USER_DIR_TOOLS");
  TEST_ASSIGN(gnustepUserDirTools,
    @GNUSTEP_TARGET_USER_DIR_TOOLS);
  ASSIGN_USER(gnustepUserDirAdminTools, c,
    @"GNUSTEP_USER_DIR_ADMIN_TOOLS");
  TEST_ASSIGN(gnustepUserDirAdminTools,
    @GNUSTEP_TARGET_USER_DIR_ADMIN_TOOLS);
  ASSIGN_USER(gnustepUserDirLibrary, c,
    @"GNUSTEP_USER_DIR_LIBRARY");
  TEST_ASSIGN(gnustepUserDirLibrary,
    @GNUSTEP_TARGET_USER_DIR_LIBRARY);
  ASSIGN_USER(gnustepUserDirLibraries, c,
    @"GNUSTEP_USER_DIR_LIBRARIES");
  TEST_ASSIGN(gnustepUserDirLibraries,
    @GNUSTEP_TARGET_USER_DIR_LIBRARIES);
  ASSIGN_USER(gnustepUserDirHeaders, c,
    @"GNUSTEP_USER_DIR_HEADERS");
  TEST_ASSIGN(gnustepUserDirHeaders,
    @GNUSTEP_TARGET_USER_DIR_HEADERS);
  ASSIGN_USER(gnustepUserDirDocumentation, c,
    @"GNUSTEP_USER_DIR_DOC");
  TEST_ASSIGN(gnustepUserDirDocumentation,
    @GNUSTEP_TARGET_USER_DIR_DOC);
  ASSIGN_USER(gnustepUserDirDocumentationMan, c,
    @"GNUSTEP_USER_DIR_DOC_MAN");
  TEST_ASSIGN(gnustepUserDirDocumentationMan,
    @GNUSTEP_TARGET_USER_DIR_DOC_MAN);
  ASSIGN_USER(gnustepUserDirDocumentationInfo, c,
    @"GNUSTEP_USER_DIR_DOC_INFO");
  TEST_ASSIGN(gnustepUserDirDocumentationInfo,
    @GNUSTEP_TARGET_USER_DIR_DOC_INFO);

  /*
   * The GNUSTEP_EXTRA field may contain a list of extra keys which
   * we permit in the dictionary without generating a warning.
   */
  extra = [c objectForKey: @"GNUSTEP_EXTRA"];
  if (extra != nil)
    {
      NSEnumerator	*enumerator;
      NSString		*key;

      if ([extra isKindOfClass: [NSString class]] == YES)
        {
          extra = [extra componentsSeparatedByString: @","];
        }
      enumerator = [extra objectEnumerator];
      [c removeObjectForKey: @"GNUSTEP_EXTRA"];
      while ((key = [enumerator nextObject]) != nil)
        {
	  key = [key stringByTrimmingSpaces];
	  [c removeObjectForKey: key];
	}
    }
  [c removeObjectForKey: @"GNUSTEP_SYSTEM_DEFAULTS_FILE"];

  /* If GNUSTEP_CREATE_LIBRARY_PATH is YES then we should ensure that the
   * per-user directory and the Library subdirectory exist so resources
   * can safely be stored in them.
   */
  createLibraryPath
    = [[c objectForKey: @"GNUSTEP_CREATE_LIBRARY_PATH"] boolValue];
  [c removeObjectForKey: @"GNUSTEP_CREATE_LIBRARY_PATH"];

  /*
   * Remove any other dictionary entries we have used.
   */
  [c removeObjectForKey: @"GNUSTEP_USER_CONFIG_FILE"];

  /* FIXME ... for the time being we just ignore obsolete keys */
  [c removeObjectForKey: @"GNUSTEP_USER_ROOT"];
  [c removeObjectForKey: @"GNUSTEP_LOCAL_ROOT"];
  [c removeObjectForKey: @"GNUSTEP_SYSTEM_ROOT"];
  [c removeObjectForKey: @"GNUSTEP_NETWORK_ROOT"];
  [c removeObjectForKey: @"GNUSTEP_USER_DIR"];

  if ([c count] > 0)
    {
      /*
       * The dictionary should be empty ... report problems
       */
      fprintf(stderr, "Configuration contains unknown keys - %s\n"
        "Add them in the comma separated list GNUSTEP_EXTRA=... if required.\n",
	[[[c allKeys] description] UTF8String]);
    }
  DESTROY(c);

  /*
   * Set default locations for user files if necessary.
   */
  if (gnustepUserDefaultsDir == nil)
    {
      ASSIGN(gnustepUserDefaultsDir, @GNUSTEP_TARGET_USER_DEFAULTS_DIR);
    }
  if (gnustepUserDirApps == nil)
    {
      ASSIGN(gnustepUserDirApps, @GNUSTEP_TARGET_USER_DIR_APPS);
    }
  if (gnustepUserDirTools == nil)
    {
      ASSIGN(gnustepUserDirTools, @GNUSTEP_TARGET_USER_DIR_TOOLS);
    }
  if (gnustepUserDirLibrary == nil)
    {
      ASSIGN(gnustepUserDirLibrary, @GNUSTEP_TARGET_USER_DIR_LIBRARY);
    }
  if (gnustepUserDirLibraries == nil)
    {
      ASSIGN(gnustepUserDirLibraries, @GNUSTEP_TARGET_USER_DIR_LIBRARIES);
    }
  if (gnustepUserDirHeaders == nil)
    {
      ASSIGN(gnustepUserDirHeaders, @GNUSTEP_TARGET_USER_DIR_HEADERS);
    }
  if (gnustepUserDirDocumentation == nil)
    {
      ASSIGN(gnustepUserDirDocumentation,
	     @GNUSTEP_TARGET_USER_DIR_DOC);
    }
  if (gnustepUserDirDocumentationMan == nil)
    {
      ASSIGN(gnustepUserDirDocumentationMan,
	     @GNUSTEP_TARGET_USER_DIR_DOC_MAN);
    }
  if (gnustepUserDirDocumentationInfo == nil)
    {
      ASSIGN(gnustepUserDirDocumentationInfo,
	     @GNUSTEP_TARGET_USER_DIR_DOC_INFO);
    }

  /*
   * Set the GNUSTEP_USER_xxx variables from the user home and the
   * GNUSTEP_USER_DIR_xxx variables.
   */
  ASSIGN_USER_PATH(gnustepUserApps, gnustepUserDirApps);
  ASSIGN_USER_PATH(gnustepUserAdminApps, gnustepUserDirAdminApps);
  ASSIGN_USER_PATH(gnustepUserWebApps, gnustepUserDirWebApps);
  ASSIGN_USER_PATH(gnustepUserTools, gnustepUserDirTools);
  ASSIGN_USER_PATH(gnustepUserAdminTools, gnustepUserDirAdminTools);
  ASSIGN_USER_PATH(gnustepUserLibrary, gnustepUserDirLibrary);
  ASSIGN_USER_PATH(gnustepUserLibraries, gnustepUserDirLibraries);
  ASSIGN_USER_PATH(gnustepUserHeaders, gnustepUserDirHeaders);
  ASSIGN_USER_PATH(gnustepUserDocumentation, gnustepUserDocumentation);
  ASSIGN_USER_PATH(gnustepUserDocumentationMan, gnustepUserDocumentationMan);
  ASSIGN_USER_PATH(gnustepUserDocumentationInfo, gnustepUserDocumentationInfo);

  /*
   * Try to ensure that essential user directories exist if we need to.
   */
  if (YES == createLibraryPath)
    {
      NSFileManager	*manager;
      NSString		*path;
      NSDictionary	*attr;
      BOOL		flag;

      manager = [NSFileManager defaultManager];
      attr = [NSDictionary dictionaryWithObject: [NSNumber numberWithInt: 0750]
					 forKey: NSFilePosixPermissions];

      // Make sure library directory exists (to store resources).
      path = gnustepUserLibrary;
      if ([manager fileExistsAtPath: path isDirectory: &flag] == NO
	|| flag == NO)
	{
	  [manager createDirectoryAtPath: path
             withIntermediateDirectories: YES
                              attributes: attr
                                   error: NULL];
	}
    }

  /*
   * Finally set default locations for the essential paths if required.
   */
  ASSIGN_DEFAULT_PATH(gnustepSystemApps,
    @GNUSTEP_TARGET_SYSTEM_APPS);
  ASSIGN_DEFAULT_PATH(gnustepSystemAdminApps,
    @GNUSTEP_TARGET_SYSTEM_ADMIN_APPS);
  ASSIGN_DEFAULT_PATH(gnustepSystemWebApps,
    @GNUSTEP_TARGET_SYSTEM_WEB_APPS);
  ASSIGN_DEFAULT_PATH(gnustepSystemTools,
    @GNUSTEP_TARGET_SYSTEM_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepSystemAdminTools,
    @GNUSTEP_TARGET_SYSTEM_ADMIN_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepSystemLibrary,
    @GNUSTEP_TARGET_SYSTEM_LIBRARY);
  ASSIGN_DEFAULT_PATH(gnustepSystemLibraries,
    @GNUSTEP_TARGET_SYSTEM_LIBRARIES);
  ASSIGN_DEFAULT_PATH(gnustepSystemHeaders,
    @GNUSTEP_TARGET_SYSTEM_HEADERS);
  ASSIGN_DEFAULT_PATH(gnustepSystemDocumentation,
    @GNUSTEP_TARGET_SYSTEM_DOC);
  ASSIGN_DEFAULT_PATH(gnustepSystemDocumentationMan,
    @GNUSTEP_TARGET_SYSTEM_DOC_MAN);
  ASSIGN_DEFAULT_PATH(gnustepSystemDocumentationInfo,
    @GNUSTEP_TARGET_SYSTEM_DOC_INFO);

  ASSIGN_DEFAULT_PATH(gnustepNetworkApps,
    @GNUSTEP_TARGET_NETWORK_APPS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkAdminApps,
    @GNUSTEP_TARGET_NETWORK_ADMIN_APPS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkWebApps,
    @GNUSTEP_TARGET_NETWORK_WEB_APPS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkTools,
    @GNUSTEP_TARGET_NETWORK_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkAdminTools,
    @GNUSTEP_TARGET_NETWORK_ADMIN_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkLibrary,
    @GNUSTEP_TARGET_NETWORK_LIBRARY);
  ASSIGN_DEFAULT_PATH(gnustepNetworkLibraries,
    @GNUSTEP_TARGET_NETWORK_LIBRARIES);
  ASSIGN_DEFAULT_PATH(gnustepNetworkHeaders,
    @GNUSTEP_TARGET_NETWORK_HEADERS);
  ASSIGN_DEFAULT_PATH(gnustepNetworkDocumentation,
    @GNUSTEP_TARGET_NETWORK_DOC);
  ASSIGN_DEFAULT_PATH(gnustepNetworkDocumentationMan,
    @GNUSTEP_TARGET_NETWORK_DOC_MAN);
  ASSIGN_DEFAULT_PATH(gnustepNetworkDocumentationInfo,
    @GNUSTEP_TARGET_NETWORK_DOC_INFO);

  ASSIGN_DEFAULT_PATH(gnustepLocalApps,
    @GNUSTEP_TARGET_LOCAL_APPS);
  ASSIGN_DEFAULT_PATH(gnustepLocalAdminApps,
    @GNUSTEP_TARGET_LOCAL_ADMIN_APPS);
  ASSIGN_DEFAULT_PATH(gnustepLocalWebApps,
    @GNUSTEP_TARGET_LOCAL_WEB_APPS);
  ASSIGN_DEFAULT_PATH(gnustepLocalTools,
    @GNUSTEP_TARGET_LOCAL_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepLocalAdminTools,
    @GNUSTEP_TARGET_LOCAL_ADMIN_TOOLS);
  ASSIGN_DEFAULT_PATH(gnustepLocalLibrary,
    @GNUSTEP_TARGET_LOCAL_LIBRARY);
  ASSIGN_DEFAULT_PATH(gnustepLocalLibraries,
    @GNUSTEP_TARGET_LOCAL_LIBRARIES);
  ASSIGN_DEFAULT_PATH(gnustepLocalHeaders,
    @GNUSTEP_TARGET_LOCAL_HEADERS);
  ASSIGN_DEFAULT_PATH(gnustepLocalDocumentation,
    @GNUSTEP_TARGET_LOCAL_DOC);
  ASSIGN_DEFAULT_PATH(gnustepLocalDocumentationMan,
    @GNUSTEP_TARGET_LOCAL_DOC_MAN);
  ASSIGN_DEFAULT_PATH(gnustepLocalDocumentationInfo,
    @GNUSTEP_TARGET_LOCAL_DOC_INFO);

  ASSIGN_DEFAULT_PATH(gnustepMakefiles,
    @GNUSTEP_TARGET_MAKEFILES);

  ASSIGN_DEFAULT_PATH(gnustepSystemUsersDir,
    @GNUSTEP_TARGET_SYSTEM_USERS_DIR);
  ASSIGN_DEFAULT_PATH(gnustepNetworkUsersDir,
    @GNUSTEP_TARGET_NETWORK_USERS_DIR);
  ASSIGN_DEFAULT_PATH(gnustepLocalUsersDir,
    @GNUSTEP_TARGET_LOCAL_USERS_DIR);
}

static void
addDefaults(NSString *defs, NSMutableDictionary *conf)
{
  if ([MGR() isReadableFileAtPath: defs] == YES)
    {
      NSDictionary      *d;
      NSDictionary      *attributes;

      attributes = [MGR() fileAttributesAtPath: defs
				  traverseLink: YES];
      if (([attributes filePosixPermissions]
	& (0022 & ATTRMASK)) != 0)
	{
#if defined(_WIN32)
	  fprintf(stderr,
	    "\nThe file '%S' is writable by someone other than"
	    " its owner (permissions 0%lo).\nIgnoring it.\n",
	    [defs fileSystemRepresentation],
	    (long)[attributes filePosixPermissions]);
#else
	  fprintf(stderr,
	    "\nThe file '%s' is writable by someone other than"
	    " its owner (permissions 0%lo).\nIgnoring it.\n",
	    [defs fileSystemRepresentation],
	    (long)[attributes filePosixPermissions]);
#endif
	  d = nil;
	}
      else
	{
	  NSString	*s;

	  /* Here we load the string and convert to a property list
	   * rather than using higher-level methods likely to raise
	   * an exception or otherwise use the NSUserDefaults
           * system and cause recursion on error.
	   */
	  s = [[NSString allocWithZone: NSDefaultMallocZone()]
	    initWithContentsOfFile: defs];
	  if (nil == s)
	    {
	      d = nil;
	    }
	  else
	    {
	      NS_DURING
		{
		  d = [s propertyList];
		  if (NO == [d isKindOfClass: [NSDictionary class]])
		    {
		      d = nil;
		    }
		}
	      NS_HANDLER
		{
		  d = nil;
		}
	      NS_ENDHANDLER
	      RELEASE(s);
	    }
	  if (nil == d)
	    {
#if defined(_WIN32)
	  fprintf(stderr,
	    "\nThe file '%S' is not parseable as a property list"
	    " containing a dictionary.\nIgnoring it.\n",
	    [defs fileSystemRepresentation]);
#else
	  fprintf(stderr,
	    "\nThe file '%s' is not parseable as a property list"
	    " containing a dictionary.\nIgnoring it.\n",
	    [defs fileSystemRepresentation]);
#endif
	    }
	}

      if (d != nil)
	{
	  NSEnumerator  *enumerator;
	  NSString      *key;
	  id            extra;

	  extra = [conf objectForKey: @"GNUSTEP_EXTRA"];
	  if ([extra isKindOfClass: [NSString class]])
	    {
	      extra = [extra componentsSeparatedByString: @","];
	    }
	  extra = [extra mutableCopy];
	  if (extra == nil)
	    {
	      extra = [NSMutableArray new];
	    }
	  enumerator = [d keyEnumerator];
	  while ((key = [enumerator nextObject]) != nil)
	    {
	      if ([conf objectForKey: key] == nil)
		{
		  [extra addObject: key];
		}
	      else
		{
		  fprintf(stderr, "Value for key '%s' in '%s' replaces"
		    " earlier setting.\n", [key UTF8String], [defs UTF8String]);
		}
	    }
	  [conf addEntriesFromDictionary: d];
	  if ([extra count] > 0)
	    {
	      NSArray   *c = [extra copy];

	      [conf setObject: c forKey: @"GNUSTEP_EXTRA"];
	      RELEASE(c);
	    }
	  RELEASE(extra);
	}
    }
}

NSMutableDictionary*
GNUstepConfig(NSDictionary *newConfig)
{
  static NSDictionary	*config = nil;
  static BOOL beenHere = NO;
  NSMutableDictionary	*conf = nil;
  BOOL			changedSystemConfig = NO;

  [gnustep_global_lock lock];
  if (NO == beenHere)
    {
      beenHere = YES;
      [[NSObject leakAt: &config] release];
    }
  if (config == nil || (newConfig != nil && [config isEqual: newConfig] == NO))
    {
      NS_DURING
	{
	  if (newConfig == nil)
	    {
	      NSString		*file = nil;
	      NSString		*path;
	      NSEnumerator	*e;
	      NSString      	*defs;
	      BOOL		fromEnvironment = YES;
	      BOOL		bareDirectory = NO;

	      conf = [[NSMutableDictionary alloc] initWithCapacity: 32];

	      /* Now we source the configuration file if it exists */
#if	!OPTION_NO_ENVIRONMENT
	      file = [[[NSProcessInfo processInfo] environment]
		objectForKey: @"GNUSTEP_CONFIG_FILE"];
#endif
	      if (file == nil)
		{
		  fromEnvironment = NO;
		  file = @GNUSTEP_TARGET_CONFIG_FILE;
		}

	      /*
	       * Is the file missing from the path ... if so we won't
	       * be reading it.
	       */
	      if ([file hasSuffix: @"/"] || [file hasSuffix: @"\\"])
		{
		  bareDirectory = YES;
		}

	      /*
	       * Special case ... if the config file location begins './'
	       * or '../' then we determine it's actual path by working
               * relative to the gnustep-base library.
	       */
	      if ([file hasPrefix: @"./"] == YES
	        || [file hasPrefix: @"../"] == YES)
		{
		  Class		c = [NSProcessInfo class];

		  path = GSPrivateSymbolPath(c);
		  // Remove library name from path
		  path = [path stringByDeletingLastPathComponent];
                  if ([file hasPrefix: @"./"] == YES)
                    {
                      file = [file substringFromIndex: 2];
                    }
		  // Join the two together
		  file = [path stringByAppendingPathComponent: file];
		}
	      file = [file stringByStandardizingPath];

	      if ([file isAbsolutePath] == NO)
		{
		  if (fromEnvironment ==  YES)
		    {
		      fprintf(stderr, "GNUSTEP_CONFIG_FILE value ('%s') is not"
			" an absolute path.  Please fix the environment"
			" variable.\n", [file UTF8String]);
		    }
		  else
		    {
		      fprintf(stderr, "GNUSTEP_CONFIG_FILE value ('%s') is not"
			" an absolute path.  Please rebuild GNUstep-base"
			" specifying a valid path to the config file.\n",
                        [file UTF8String]);
		    }
#if	defined(_WIN32)
		  if ([file length] > 2)
		    {
		      unichar	buf[3];

		      [file getCharacters: buf range: NSMakeRange(0, 3)];
		      if ((buf[0] == '/' || buf[0] == '\\') && isalpha(buf[1])
			&& (buf[2] == '/' || buf[2] == '\\'))
			{
			  file = [NSString stringWithFormat: @"%c:%@",
			    (char)buf[1], [file substringFromIndex: 2]];
			  file = [file stringByReplacingString: @"/"
						    withString: @"\\"];
			  fprintf(stderr, "I am guessing that you meant '%s'\n",
                            [file UTF8String]);
			}
		    }
#endif
		}

	      if (bareDirectory == YES)
		{
		  /* Set the directory name, but don't try to read file.
		   */
		  gnustepConfigPath = RETAIN(file);
		}
	      else
		{
		  gnustepConfigPath
		    = RETAIN([file stringByDeletingLastPathComponent]);
		  ParseConfigurationFile(file, conf, nil);
		}

	      /* Merge in any values from property lists in the
	       * GlobalDefaults directory.
	       */
	      path = [gnustepConfigPath stringByAppendingPathComponent:
		@"GlobalDefaults"];
	      e = [[MGR() directoryContentsAtPath: path] objectEnumerator];
	      while ((defs = [e nextObject]) != nil)
		{
		  if ([[defs pathExtension] isEqualToString: @"plist"])
		    {
		      defs = [path stringByAppendingPathComponent: defs];
		      addDefaults(defs, conf);
		    }
		}

	      /* And merge in value from GlobalDefaults.plist
	       */
	      defs = [gnustepConfigPath stringByAppendingPathComponent:
		@"GlobalDefaults.plist"];
	      addDefaults(defs, conf);
	    }
	  else
	    {
	      conf = [newConfig mutableCopy];
	    }
	  /* System admins may force the user and defaults paths by
	   * setting GNUSTEP_USER_CONFIG_FILE to be an empty string.
	   * If they simply don't define it at all, we assign a default.
	   */
	  if ([conf objectForKey: @"GNUSTEP_USER_CONFIG_FILE"] == nil)
	    {
	      [conf setObject: @GNUSTEP_TARGET_USER_CONFIG_FILE
		       forKey: @"GNUSTEP_USER_CONFIG_FILE"];
	    }
	  if (config != nil)
	    {
	      changedSystemConfig = YES;
	    }
	  config = [conf copy];
	  DESTROY(conf);
	}
      NS_HANDLER
	{
	  [gnustep_global_lock unlock];
	  config = nil;
	  DESTROY(conf);
	  [localException raise];
	}
      NS_ENDHANDLER
    }
  [gnustep_global_lock unlock];

  if (changedSystemConfig == YES)
    {
      /*
       * The main configuration was changed by passing in a dictionary to
       * this function, so we need to reset the path utilities system to use
       * any new values from the config.
       */
      ShutdownPathUtilities();
      InitialisePathUtilities();
    }

  return AUTORELEASE([config mutableCopy]);
}

void
GNUstepUserConfig(NSMutableDictionary *config, NSString *userName)
{
#ifdef HAVE_GETEUID
  if (userName != nil)
    {
      /*
       * A program which is running setuid cannot be trusted
       * to pick up user specific config, so we clear the userName
       * to force the system configuration to be returned rather
       * than a per-user config.
       */
      if (getuid() != geteuid())
	{
	  userName = nil;
	}
    }
#endif

  if (userName != nil)
    {
      NSString		*file;
      NSString		*home;
      NSString		*path;

      file = RETAIN([config objectForKey: @"GNUSTEP_USER_CONFIG_FILE"]);
      if ([file length] > 0)
	{
	  home = NSHomeDirectoryForUser(userName);
	  path = [home stringByAppendingPathComponent: file];
	  ParseConfigurationFile(path, config, userName);
	}
      /*
       * We don't let the user config file override the GNUSTEP_USER_CONFIG_FILE
       * variable ... that would be silly/pointless.
       */
      [config setObject: file forKey: @"GNUSTEP_USER_CONFIG_FILE"];
      RELEASE(file);
    }
}

/* Initialise all things required by this module */
static void InitialisePathUtilities(void)
{
  if (gnustepMakefiles != nil)
    {
      return;	// Protect from multiple calls
    }

  /* Set up our root paths */
  NS_DURING
    {
      NSMutableDictionary	*config;
      static BOOL               beenHere = NO;

      [gnustep_global_lock lock];
      if (NO == beenHere)
        {
          beenHere = YES;
          if (YES == [NSObject shouldCleanUp])
            {
              // Get path utilities shutdown called at process exit.
              [GSPathUtilities class];
            }
        }
      ASSIGNCOPY(uninstalled, [[[NSProcessInfo processInfo] environment]
	objectForKey: @"GNUSTEP_UNINSTALLED_LIBRARY_DIRECTORY"]);
      gnustepUserName = [NSUserName() copy];
#if defined(_WIN32)
      {
        union {
	  SID	sid;
	  char	dummy[1024];
	} s;
	LPTSTR	str;
	unichar buf[1024];
	unichar dom[1024];
	SID_NAME_USE use;
	DWORD bsize = 1024;
	DWORD ssize = 1024;

	if (GetUserNameW(buf, &bsize) == 0 || buf[0] == '\0')
	  {
	    [NSException raise: NSInternalInconsistencyException
			format: @"Unable to determine current user name: %@",
	      [NSError _last]];
	  }
	bsize = 1024;
	if (LookupAccountNameW(0, buf, &s.sid, &ssize, dom, &bsize, &use) == 0)
	  {
	    [NSException raise: NSInternalInconsistencyException
			format: @"Unable to determine current account: %@",
	      [NSError _last]];
	  }
	if (ConvertSidToStringSid(&s.sid, &str) == 0)
	  {
	    [NSException raise: NSInternalInconsistencyException
			format: @"Unable to get current user ID string: %@",
	      [NSError _last]];
	  }
        gnustepUserID = [[NSString alloc] initWithUTF8String: str];
      }
#else
      gnustepUserID = [[NSString alloc] initWithFormat: @"%ld", (long)getuid()];
#endif
      config = GNUstepConfig(nil);
      GNUstepUserConfig(config, gnustepUserName);
      gnustepUserHome = [NSHomeDirectoryForUser(gnustepUserName) copy];
      ExtractValuesFromConfig(config);

      [gnustep_global_lock unlock];
    }
  NS_HANDLER
    {
      /* unlock then re-raise the exception */
      [gnustep_global_lock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
}

/*
 * Close down and release all things allocated.
 */
static void ShutdownPathUtilities(void)
{
  DESTROY(gnustepUserHome);
  DESTROY(gnustepUserID);
  DESTROY(gnustepUserName);
  DESTROY(gnustepUserDefaultsDir);

  DESTROY(gnustepMakefiles);

  DESTROY(gnustepSystemUsersDir);
  DESTROY(gnustepNetworkUsersDir);
  DESTROY(gnustepLocalUsersDir);

  DESTROY(gnustepSystemApps);
  DESTROY(gnustepSystemAdminApps);
  DESTROY(gnustepSystemWebApps);
  DESTROY(gnustepSystemTools);
  DESTROY(gnustepSystemAdminTools);
  DESTROY(gnustepSystemLibrary);
  DESTROY(gnustepSystemLibraries);
  DESTROY(gnustepSystemHeaders);
  DESTROY(gnustepSystemDocumentation);
  DESTROY(gnustepSystemDocumentationMan);
  DESTROY(gnustepSystemDocumentationInfo);

  DESTROY(gnustepNetworkApps);
  DESTROY(gnustepNetworkAdminApps);
  DESTROY(gnustepNetworkWebApps);
  DESTROY(gnustepNetworkTools);
  DESTROY(gnustepNetworkAdminTools);
  DESTROY(gnustepNetworkLibrary);
  DESTROY(gnustepNetworkLibraries);
  DESTROY(gnustepNetworkHeaders);
  DESTROY(gnustepNetworkDocumentation);
  DESTROY(gnustepNetworkDocumentationMan);
  DESTROY(gnustepNetworkDocumentationInfo);

  DESTROY(gnustepLocalApps);
  DESTROY(gnustepLocalAdminApps);
  DESTROY(gnustepLocalWebApps);
  DESTROY(gnustepLocalTools);
  DESTROY(gnustepLocalAdminTools);
  DESTROY(gnustepLocalLibrary);
  DESTROY(gnustepLocalLibraries);
  DESTROY(gnustepLocalHeaders);
  DESTROY(gnustepLocalDocumentation);
  DESTROY(gnustepLocalDocumentationMan);
  DESTROY(gnustepLocalDocumentationInfo);

  DESTROY(gnustepDeveloperDir);

  DESTROY(gnustepUserApps);
  DESTROY(gnustepUserAdminApps);
  DESTROY(gnustepUserWebApps);
  DESTROY(gnustepUserTools);
  DESTROY(gnustepUserAdminTools);
  DESTROY(gnustepUserLibrary);
  DESTROY(gnustepUserLibraries);
  DESTROY(gnustepUserHeaders);
  DESTROY(gnustepUserDocumentation);
  DESTROY(gnustepUserDocumentationMan);
  DESTROY(gnustepUserDocumentationInfo);

  DESTROY(tempDir);
}

/**
 * Reads a file and expects it to be in basic unix "conf" style format with
 * one key = value per line (the format a unix shell can 'source' in order
 * to define shell variables).<br />
 * This attempts to mimic the escape sequence and quoting conventions of
 * the standard bourne shell, so that a config file sourced by the make
 * package will produce the same results as one parsed by this function.<br />
 * Keys, by convention, consiste of uppercase letters, digits,
 * and underscores, and must not begin with a digit.<br />
 * A value may be any quoted string (or an unquoted string containing no
 * white space).<br />
 * Lines beginning with a hash '#' are deemed comment lines and ignored.<br/ >
 * The backslash character may be used as an escape character anywhere
 * in the file  except within a singly quoted string
 * (where it is taken literally).<br />
 * A backslash followed immediately by a newline (except in a singly
 * quoted string) is removed completely along with the newline ... it
 * thus serves to join lines so that they are treated as a single line.<br />
 * NB. Since ms-windows uses backslash characters in paths, it is a good
 * idea to specify path values in the config file as singly quoted
 * strings to avoid having to double all occurrences of the backslash.<br />
 * Returns a dictionary of the (key,value) pairs.<br/ >
 * If the file does not exist,
 * the function makes no changes to dict and returns NO.
 */
static BOOL
ParseConfigurationFile(NSString *fileName, NSMutableDictionary *dict,
  NSString *userName)
{
  NSDictionary	*attributes;
  NSString      *file;
  unsigned	l;
  unichar	*src;
  unichar	*dst;
  unichar	*end;
  unichar	*spos;
  unichar	*dpos;
  BOOL		newLine = YES;
  BOOL		wantKey = YES;
  BOOL		wantVal = NO;
  NSString	*key = nil;

  if ([MGR() isReadableFileAtPath: fileName] == NO)
    {
      return NO;
    }

  attributes = [MGR() fileAttributesAtPath: fileName traverseLink: YES];
  if (userName != nil)
    {
      NSString	*fileOwner = [attributes fileOwnerAccountName];

      if ([userName isEqual: fileOwner] == NO)
	{
#if defined(_WIN32)
	  fprintf(stderr, "The file '%S' is owned by '%s' but we expect it"
	    " to be the personal config file of '%s'.\nIgnoring it.\n",
	    [fileName fileSystemRepresentation],
	    [fileOwner UTF8String], [userName UTF8String]);
#else
	  fprintf(stderr, "The file '%s' is owned by '%s' but we expect it"
	    " to be the personal config file of '%s'.\nIgnoring it.\n",
	    [fileName fileSystemRepresentation],
	    [fileOwner UTF8String], [userName UTF8String]);
#endif
          return NO;
	}
    }
  if (([attributes filePosixPermissions] & (0022 & ATTRMASK)) != 0)
    {
#if defined(_WIN32)
      fprintf(stderr, "The file '%S' is writable by someone other than"
	" its owner (permissions 0%lo).\nIgnoring it.\n",
	[fileName fileSystemRepresentation],
        (long)[attributes filePosixPermissions]);
#else
      fprintf(stderr, "The file '%s' is writable by someone other than"
	" its owner (permissions 0%lo).\nIgnoring it.\n",
	[fileName fileSystemRepresentation],
        (long)[attributes filePosixPermissions]);
#endif
      return NO;
    }

  if (dict == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"No destination dictionary supplied"];
    }

  file = [NSString stringWithContentsOfFile: fileName];
  l = [file length];
  src = (unichar*)NSZoneMalloc(NSDefaultMallocZone(), sizeof(unichar) * l);
  spos = src;
  end = src + l;
  dst = (unichar*)NSZoneMalloc(NSDefaultMallocZone(), sizeof(unichar) * l);
  dpos = dst;
  [file getCharacters: src];

  while (spos < end)
    {
      /*
       * Step past any whitespace ... including blank lines
       */
      while (spos < end)
	{
	  if (*spos == '\\')
	    {
	      spos++;
	      if (spos >= end)
		{
		  break;	// At end of file ... odd but not fatal
		}
	    }
	  if (*spos > ' ')
	    {
	      break;		// OK ... found a non space character.
	    }
	  if (*spos == '\r' || *spos == '\n')
	    {
	      newLine = YES;
	    }
	  spos++;
	}

      /*
       * Handle any comments .. hash on a new line.
       */
      if (newLine == YES)
	{
	  if (wantVal == YES)
	    {
	      /*
	       * On a newline ...so the last key had no value set.
	       * Put an empty value in the dictionary.
	       */
	      [dict setObject: @"" forKey: key];
	      DESTROY(key);
	      wantVal = NO;
	    }
	  if (spos < end && *spos == '#')
	    {
	      // Got a comment ... ignore remainder of line.
	      while (spos < end && *spos != '\n' && *spos != '\r')
		{
		  spos++;
		}
	      continue;	// restart loop ... skip space at start of line
	    }
	  newLine = NO;
	  wantKey = YES;
	}

      if (spos >= end)
	{
	  break;	// At end of file ... odd but not fatal
	}

      if (*spos == '=')
	{
	  if (wantKey == NO)
	    {
	      wantVal = YES;
	    }
	  spos++;
	}
      else if (*spos == '\'')
	{
	  spos++;
	  while (spos < end)
	    {
	      if (*spos == '\'')
		{
		  spos++;
		  break;
		}
	      *dpos++ = *spos++;
	    }
	  if (wantVal == YES)
	    {
	      NSString	*val = [NSString alloc];

	      val = [val initWithCharacters: dst length: dpos - dst];
	      if (val != nil)
		{
		  [dict setObject: val forKey: key];
		  DESTROY(key);
		  DESTROY(val);
		  wantVal = NO;
		}
	    }
	  dpos = dst;	// reset output buffer
	}
      else if (*spos == '"')
	{
	  spos++;
	  while (spos < end)
	    {
	      BOOL	escaped = NO;

	      if (*spos == '\\')
		{
		  spos++;
		  if (spos >= end)
		    {
		      break;	// Unexpected end of file
		    }
		  if (*spos == '\n')
		    {
		      spos++;
		      continue;	// escaped newline is removed.
		    }
		  if (*spos == '\r')
		    {
		      spos++;
		      if (spos < end && *spos == '\n')
			{
			  spos++;
			}
		      continue;	// escaped newline is removed.
		    }
		  escaped = YES;
		}
	      if (*spos == '"' && escaped == NO)
		{
		  spos++;
		  break;
		}
	      *dpos++ = *spos++;
	    }
	  if (wantVal == YES)
	    {
	      NSString	*val = [NSString alloc];

	      val = [val initWithCharacters: dst length: dpos - dst];
	      if (val != nil)
		{
		  [dict setObject: val forKey: key];
		  DESTROY(key);
		  DESTROY(val);
		  wantVal = NO;
		}
	    }
	  dpos = dst;	// reset output buffer
	}
      else
	{
	  while (spos < end)
	    {
	      if (*spos == '\\')
		{
		  spos++;
		  if (spos >= end)
		    {
		      break;	// Unexpected end of file
		    }
		  if (*spos == '\n')
		    {
		      spos++;
		      continue;	// escaped newline is removed.
		    }
		  if (*spos == '\r')
		    {
		      spos++;
		      if (spos < end && *spos == '\n')
			{
			  spos++;
			}
		      continue;	// escaped newline is removed.
		    }
		}
	      if (isspace(*spos) || *spos == '=')
		{
		  break;
		}
	      *dpos++ = *spos++;
	    }

	  if (wantKey == YES)
	    {
	      key = [NSString alloc];
	      key = [key initWithCharacters: dst length: dpos - dst];
	      if (key != nil)
		{
		  wantKey = NO;
		}
	    }
	  else if (wantVal == YES)
	    {
	      NSString	*val = [NSString alloc];

	      val = [val initWithCharacters: dst length: dpos - dst];
	      if (val != nil)
		{
		  [dict setObject: val forKey: key];
		  DESTROY(key);
		  DESTROY(val);
		  wantVal = NO;
		}
	    }
	  dpos = dst;	// reset output buffer
	}
    }
  if (wantVal == YES)
    {
      [dict setObject: @"" forKey: key];
    }
  DESTROY(key);
  NSZoneFree(NSDefaultMallocZone(), src);
  NSZoneFree(NSDefaultMallocZone(), dst);
  return YES;
}


/* See NSPathUtilities.h for description */
void
GSSetUserName(NSString *aName)
{
  NSCParameterAssert([aName length] > 0);

  /*
   * Do nothing if it's not a different user.
   */
  if ([theUserName isEqualToString: aName])
    {
      return;
    }

  /*
   * Release the memory
   */
  [gnustep_global_lock lock];
  ShutdownPathUtilities();

  /*
   * Reset things as new user
   */
  ASSIGN(theUserName, aName);
  DESTROY(theFullUserName);
  InitialisePathUtilities();
  [NSUserDefaults resetStandardUserDefaults];

  [gnustep_global_lock unlock];
}

/**
 * Return the caller's login name as an NSString object.<br/ >
 * Under unix-like systems, the name associated with the current
 * effective user ID is used.<br/ >
 * Under ms-windows, the 'LOGNAME' environment is used, or if that fails, the
 * GetUserName() call is used to find the user name.<br />
 * Raises an exception on failure.
 */
/* NOTE FOR DEVELOPERS.
 * If you change the behavior of this method you must also change
 * user_home.c in the makefiles package to match.
 */
NSString *
NSUserName(void)
{
#if defined(_WIN32)
  if (theUserName == nil)
    {
      /* Use the LOGNAME environment variable if set. */
      theUserName = [[[NSProcessInfo processInfo] environment]
	objectForKey: @"LOGNAME"];
      if ([theUserName length] > 0)
	{
	  RETAIN(theUserName);
	}
      else
	{
	  /* The GetUserName function returns the current user name */
	  unichar buf[1024];
	  DWORD n = 1024;

	  if (GetUserNameW(buf, &n) != 0 && buf[0] != '\0')
	    {
	      theUserName = [[NSString alloc] initWithCharacters: buf
							  length: wcslen(buf)];
	    }
	  else
	    {
	      theUserName = nil;
	      [NSException raise: NSInternalInconsistencyException
			  format: @"Unable to determine current user name"];
	    }
	}
    }
#else
  /* Set olduid to some invalid uid that we could never start off running
     as.  */
  static int	olduid = -1;
#ifdef HAVE_GETEUID
  int uid = geteuid();
#else
  int uid = getuid();
#endif /* HAVE_GETEUID */

  if (theUserName == nil || uid != olduid)
    {
      const char *loginName = 0;
      char buf[BUFSIZ*10];
#if     defined(HAVE_GETPWUID_R)
      struct passwd pwent;
      struct passwd *p;

      if (getpwuid_r(uid, &pwent, buf, sizeof(buf), &p) == 0)
        {
          loginName = pwent.pw_name;
        }
#else
#if     defined(HAVE_GETPWUID)
      struct passwd *pwent;

      [gnustep_global_lock lock];
      pwent = getpwuid (uid);
      strncpy(buf, pwent->pw_name, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = '\0';
      [gnustep_global_lock unlock];
      loginName = buf;
#endif /* HAVE_GETPWUID */
#endif /* HAVE_GETPWUID_R */
      olduid = uid;
      if (loginName)
	theUserName = [[NSString alloc] initWithCString: loginName];
      else
	[NSException raise: NSInternalInconsistencyException
		    format: @"Unable to determine current user name"];
    }
#endif
  return theUserName;
}


/**
 * Return the caller's home directory as an NSString object.
 * Calls NSHomeDirectoryForUser() to do this.
 */
NSString *
NSHomeDirectory(void)
{
  return NSHomeDirectoryForUser(NSUserName());
}

/**
 * Returns loginName's home directory as an NSString object.<br />
 * On most systems this returns the directory specified in the system's
 * password file (ar at least whatever is returned by the standard
 * operating system API for retrieving password file records),
 * but on ms-windows this is determined by examining the
 * HOMEPATH and HOMEDRIVE environment variables or (if those don't make
 * sense) the USERPROFILE environment variable.
 */
/* NOTE FOR DEVELOPERS.
 * If you change the behavior of this method you must also change
 * user_home.c in the makefiles package to match.
 */
NSString *
NSHomeDirectoryForUser(NSString *loginName)
{
  NSString	*s = nil;

#ifdef __ANDROID__
  s = [[NSProcessInfo processInfo] androidFilesDir];
#elif !defined(_WIN32)
#if     defined(HAVE_GETPWNAM_R)
  struct passwd pw;
  struct passwd *p;
  char buf[BUFSIZ*10];

  if (getpwnam_r([loginName cString], &pw, buf, sizeof(buf), &p) == 0
    && p != 0 && pw.pw_dir != 0 && pw.pw_dir[0] != '\0')
    {
      s = [NSString stringWithUTF8String: pw.pw_dir];
    }
#else
#if     defined(HAVE_GETPWNAM)
  struct passwd *pw;

  [gnustep_global_lock lock];
  pw = getpwnam ([loginName cString]);
  if (pw != 0 && pw->pw_dir != 0 && pw->pw_dir[0] != '\0')
    {
      s = [NSString stringWithUTF8String: pw->pw_dir];
    }
  [gnustep_global_lock unlock];
#endif
#endif
#else
  if ([loginName isEqual: NSUserName()] == YES)
    {
      NSDictionary	*e = [[NSProcessInfo processInfo] environment];

      /*
       * The environment variable HOMEPATH holds the home directory
       * for the user on Windows NT;
       * For OPENSTEP compatibility (and because USERPROFILE is usually
       * unusable because it contains spaces), we use HOMEPATH in
       * preference to USERPROFILE, except when MINGW has set HOMEPATH to '\'
       * which isn't very useful, so we prefer USERPROFILE in that case.
       */
      s = [e objectForKey: @"HOMEPATH"];
      if ([s isEqualToString:@"\\"] && [e objectForKey: @"USERPROFILE"] != nil)
        {
          s = [e objectForKey: @"USERPROFILE"];
        }
      else if (s != nil && ([s length] < 2 || [s characterAtIndex: 1] != ':'))
        {
          s = [[e objectForKey: @"HOMEDRIVE"] stringByAppendingString: s];
        }
      if (s == nil)
        {
          s = [e objectForKey: @"USERPROFILE"];
        }
      if (s == nil)
        {
          ; // FIXME: Talk to the NET API and get the profile path
        }
    }
  else
    {
      s = nil;
      fprintf(stderr, "Trying to get home for '%s' when user is '%s'\n",
	[loginName UTF8String], [NSUserName() UTF8String]);
      fprintf(stderr,
        "Can't determine other user home directories in Win32.\n");
    }

  if ([s length] == 0 && [loginName length] != 1)
    {
      s = nil;
      fprintf(stderr, "NSHomeDirectoryForUser(%s) failed.\n",
        [loginName UTF8String]);
    }
  if (nil != s)
    {
      s = [s stringByStandardizingPath];
    }
#endif
  return s;
}

NSString *
NSFullUserName(void)
{
  if (theFullUserName == nil)
    {
      NSString	*userName = NSUserName();
#if defined(_WIN32)
      struct _USER_INFO_2	*userInfo;

      if (NetUserGetInfo(NULL, (unichar*)[userName cStringUsingEncoding:
	NSUnicodeStringEncoding], 2, (LPBYTE*)&userInfo) == 0)
	{
	  int	length = wcslen(userInfo->usri2_full_name);

	  if (length > 0)
	    {
	      userName = [NSString
		stringWithCharacters: userInfo->usri2_full_name
		length: length];
	    }
	}
#else
#ifdef  HAVE_PWD_H
#if     defined(HAVE_GETPWNAM_R)
#if     defined(HAVE_PW_GECOS_IN_PASSWD)
      struct passwd pw;
      struct passwd *p;
      char buf[BUFSIZ*10];

      if (getpwnam_r([userName cString], &pw, buf, sizeof(buf), &p) == 0)
        {
	  if (*pw.pw_gecos)
	    {
              userName = [NSString stringWithUTF8String: pw.pw_gecos];
	    }
        }
#endif /* HAVE_PW_GECOS_IN_PASSWD */
#else
#if     defined(HAVE_GETPWNAM)
#if     defined(HAVE_PW_GECOS_IN_PASSWD)
      struct passwd	*pw;

      [gnustep_global_lock lock];
      pw = getpwnam([userName cString]);
      if (pw->pw_gecos)
	{
          userName = [NSString stringWithUTF8String: pw->pw_gecos];
        }
      [gnustep_global_lock lock];
#endif /* HAVE_PW_GECOS_IN_PASSWD */
#endif /* HAVE_GETPWNAM */
#endif /* HAVE_GETPWNAM_R */
#endif /* HAVE_PWD_H */
#endif /* defined(__Win32__) else */
      ASSIGN(theFullUserName, userName);
    }
  return theFullUserName;
}

/**
 * Return the path of the defaults directory for userName.<br />
 * This examines the GNUSTEP_USER_CONFIG_FILE for the specified user,
 * with settings in it over-riding those in the main GNUstep.conf.
 */
NSString *
GSDefaultsRootForUser(NSString *userName)
{
  NSString *defaultsDir;

  InitialisePathUtilities();
  if ([userName length] == 0)
    {
      userName = NSUserName();
    }
  if ([userName isEqual: NSUserName()])
    {
      defaultsDir = gnustepUserDefaultsDir;
    }
  else
    {
      NSMutableDictionary	*config;

      config = GNUstepConfig(nil);
      GNUstepUserConfig(config, userName);
      defaultsDir = [config objectForKey: @"GNUSTEP_USER_DEFAULTS_DIR"];
      if (defaultsDir == nil)
	{
	  defaultsDir = @GNUSTEP_TARGET_USER_DEFAULTS_DIR;
	}
    }
#if	defined(_WIN32)
  if ([defaultsDir rangeOfString: @":REGISTRY:"].length > 0)
    {
      return defaultsDir;	// Just use windows registry.
    }
#endif
  if (NO == [defaultsDir isAbsolutePath])
    {
      NSString  *home = NSHomeDirectoryForUser(userName);

      defaultsDir = [home stringByAppendingPathComponent: defaultsDir];
    }

  return defaultsDir;
}

NSArray *
NSStandardApplicationPaths(void)
{
  return NSSearchPathForDirectoriesInDomains(NSAllApplicationsDirectory,
                                             NSAllDomainsMask, YES);
}

NSArray *
NSStandardLibraryPaths(void)
{
  return NSSearchPathForDirectoriesInDomains(NSAllLibrariesDirectory,
                                             NSAllDomainsMask, YES);
}

NSString *
NSTemporaryDirectory(void)
{
  NSFileManager	*manager;
  NSString	*tempDirName;
  NSString	*baseTempDirName = nil;
  BOOL		flag;
#if !defined(_WIN32)
  NSDictionary	*attr;
  int		perm;
  int		owner;
#if !defined(__ANDROID__)
  int		uid;
#endif
#endif

#if defined(_WIN32)
  unichar buffer[1024];
  if (GetTempPathW(1024, buffer))
    {
      baseTempDirName = [NSString stringWithCharacters: buffer
						length: wcslen(buffer)];
    }
#elif defined(__ANDROID__)
  /*
   * Use subfolder of cache directory as temp dir on Android, as there
   * is no official temp dir prior to API level 26, and the cache dir
   * is at least auto-purged by the system if disk space is needed.
   * We also clean it up on launch in GSInitializeProcessAndroid().
   */
  NSString *cacheDir = [[NSProcessInfo processInfo] androidCacheDir];
  if (cacheDir)
    {
      baseTempDirName = [cacheDir stringByAppendingPathComponent: @"tmp"];
    }
#endif

  /*
   * If the user has supplied a directory name in the TEMP or TMP
   * environment variable, attempt to use that unless we already
   * have a temporary directory specified.
   */
  if (baseTempDirName == nil)
    {
      NSDictionary	*env = [[NSProcessInfo processInfo] environment];

      baseTempDirName = [env objectForKey: @"TEMP"];
      if (baseTempDirName == nil)
	{
	  baseTempDirName = [env objectForKey: @"TMP"];
	  if (baseTempDirName == nil)
	    {
#if	defined(__CYGWIN__)
#warning Basing temporary directory in /cygdrive/c; any reason?
	      baseTempDirName = @"/cygdrive/c/";
#elif	defined(_WIN32)
	      baseTempDirName = @"C:\\";
#elif   defined(__APPLE__)
	      /*
	       * Create temporary directory on /var/tmp since /tmp is
	       * cleaned regularly on Darwin by default
	       */
	      baseTempDirName = @"/var/tmp";
#else
	      baseTempDirName = @"/tmp";
#endif
	    }
	}
    }

  /*
   * Check that the base directory exists ... if it doesn't we can't
   * go any further.
   */
  tempDirName = baseTempDirName;
  manager = [NSFileManager defaultManager];
  if ([manager fileExistsAtPath: tempDirName isDirectory: &flag] == NO
    || flag == NO)
    {
#ifdef __ANDROID__
      /*
       * Create our own temp dir on Android. We can disregard attributes
       * since they are not supported.
       */
      if ([manager createDirectoryAtPath: tempDirName
             withIntermediateDirectories: YES
                              attributes: nil
                                   error: NULL] == NO)
        {
          NSWarnFLog(@"Attempt to create temporary directory (%@)"
            @" failed.", tempDirName);
          return nil;
        }
#else
      NSWarnFLog(@"Temporary directory (%@) does not exist", tempDirName);
      return nil;
#endif
    }

// Mateu Batle: secure temporary directories don't work in MinGW
// Ivan Vucica: there are also problems with Cygwin
//              probable cause: http://stackoverflow.com/q/9561759/39974
#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__ANDROID__)

  /*
   * Check that we are the directory owner, and that we, and nobody else,
   * have access to it. If other people have access, try to create a secure
   * subdirectory.
   */
  attr = [manager fileAttributesAtPath: tempDirName traverseLink: YES];
  owner = [[attr objectForKey: NSFileOwnerAccountID] intValue];
  perm = [[attr objectForKey: NSFilePosixPermissions] intValue];
  perm = perm & 0777;

#ifdef HAVE_GETEUID
  uid = geteuid();
#else
  uid = getuid();
#endif /* HAVE_GETEUID */
  if ((perm != 0700 && perm != 0600) || owner != uid)
    {
      NSString	*secure;
      NSNumber	*p = [NSNumber numberWithInt: 0700];

      attr = [NSDictionary dictionaryWithObject: p
                                         forKey: NSFilePosixPermissions];

      /*
       * The name of the secure subdirectory reflects the user ID rather
       * than the user name, since it is possible to have an account with
       * lots of names on a unix system (ie multiple entries in the password
       * file but a single userid).  The private directory is secure within
       * the account, not to a particular user name.
       */
      secure = [NSString stringWithFormat: @"GNUstepSecure%d", uid];
      tempDirName
	= [baseTempDirName stringByAppendingPathComponent: secure];

      if ([manager fileExistsAtPath: tempDirName] == NO)
	{
	  if ([manager createDirectoryAtPath: tempDirName
                 withIntermediateDirectories: YES
				  attributes: attr
                                       error: NULL] == NO)
	    {
	      NSWarnFLog(@"Attempt to create a secure temporary"
	        @" directory (%@) failed.", tempDirName);
	      return nil;
	    }
	}
      else
        {
          [manager changeFileAttributes: attr atPath: tempDirName];
        }

      /*
       * Check that the new directory is really secure.
       */
      attr = [manager fileAttributesAtPath: tempDirName traverseLink: YES];
      owner = [[attr objectForKey: NSFileOwnerAccountID] intValue];
      perm = [[attr objectForKey: NSFilePosixPermissions] intValue];
      perm = perm & 0777;
      if ((perm != 0700 && perm != 0600) || owner != uid)
	{
	  NSWarnFLog(@"Attempt to create a secure temporary"
	    @" directory (%@) failed.", tempDirName);
	  return nil;
	}
    }
#endif

  if ([manager isWritableFileAtPath: tempDirName] == NO)
    {
      NSWarnFLog(@"Temporary directory (%@) is not writable", tempDirName);
      return nil;
    }
  return tempDirName;
}

NSString *
NSOpenStepRootDirectory(void)
{
  NSString	*root;

#if	defined(__CYGWIN__)
  root = @"/cygdrive/c/";
#elif	defined(_WIN32)
  root = @"C:/";
#else
  root = @"/";
#endif
  return root;
}

#if	defined(_WIN32)
/* The developer root on a windows system (where we have an msys environment
 * set up) is the point in the filesystem where we can reference make.exe via
 * msys/.../bin/.  That is, it's the windows path at which msys is installed.
 */
static NSString*
devroot(NSFileManager *manager, NSString *path)
{
  NSString      *tmp = @"";

  while (NO == [tmp isEqual: path])
    {
      NSString	*pb;
      NSString	*msys;
      BOOL 	isDir;

      msys = [path stringByAppendingPathComponent: @"msys"];
      if (YES == [manager fileExistsAtPath: msys isDirectory: &isDir]
	&& YES == isDir)
	{
	  NSEnumerator  *e;
	  NSString      *file;

	  e = [[manager directoryContentsAtPath: msys] objectEnumerator];
	  while (nil != (file = [e nextObject]))
	    {
	      if (isdigit([file characterAtIndex: 0]))
		{
		  float 	v = atof([file UTF8String]);

		  if (v <= 0)
		    {
		      continue;
		    }
		  file = [msys stringByAppendingPathComponent: file];
		  pb = [file stringByAppendingPathComponent: @"bin/make.exe"];
		  if ([manager isExecutableFileAtPath: pb])
		    {
		      return path;
		    }
		}
	    }
	}
      tmp = path;
      path = [tmp stringByDeletingLastPathComponent];
    }
  return nil;
}
#endif

NSArray *
NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory directoryKey,
  NSSearchPathDomainMask domainMask, BOOL expandTilde)
{
  NSMutableArray  *paths;
  NSString        *path;
  unsigned        i;
  unsigned        count;

  InitialisePathUtilities();

  NSCAssert(gnustepMakefiles!=nil,@"Path utilities without initialisation!");
  paths = [NSMutableArray array];

  /*
   * The order in which we return paths is important - user must come
   * first, followed by local, followed by network, followed by system.
   * The calling code can then loop on the returned paths, and stop as
   * soon as it finds something.  So things in user automatically
   * override things in system etc.
   */

#define ADD_PATH(mask, base_dir, add_dir) \
if (domainMask & mask) \
{ \
  path = [base_dir stringByAppendingPathComponent: add_dir]; \
  if ([path length] > 0 && [paths containsObject: path] == NO) \
    [paths addObject: path]; \
}
#define ADD_PLATFORM_PATH(mask, add_dir) \
if (domainMask & mask) \
{ \
  if ([add_dir length] > 0 && [paths containsObject: add_dir] == NO) \
    [paths addObject: add_dir]; \
}

  switch (directoryKey)
    {
      case NSAllApplicationsDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemApps);

	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserAdminApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalAdminApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkAdminApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemAdminApps);
	}
	break;

      case NSApplicationDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemApps);
	}
	break;

      case NSDemoApplicationDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemApps);

	  /* I imagine if ever wanted a separate Demo directory, the
	   * only way for this to have some meaning across filesystems
	   * would be as a subdirectory of Applications, as follows.
	   */
	  /*
	    ADD_PATH(NSUserDomainMask, gnustepUserApps, @"Demos");
	    ADD_PATH(NSLocalDomainMask, gnustepLocalApps, @"Demos");
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkApps, @"Demos");
	    ADD_PATH(NSSystemDomainMask, gnustepSystemApps, @"Demos");
	  */
	}
	break;

      case NSCoreServicesDirectory:
	{
	  ADD_PATH(NSSystemDomainMask, gnustepSystemLibrary, @"CoreServices");
	}
	break;

      case NSDesktopDirectory:
	{
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Desktop");
	}
	break;

      case NSDeveloperApplicationDirectory:
	{
	  /* Deprecated key ... for now just point to normal apps.
	   */
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemApps);
	}
	break;

      case NSAdminApplicationDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserAdminApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalAdminApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkAdminApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemAdminApps);
	}
	break;

      case NSAllLibrariesDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserLibrary);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalLibrary);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkLibrary);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemLibrary);
	}
	break;

      case NSLibraryDirectory:
	{
	  /* We allow the environment to be used to specify the location of
	   * the resources of uninstalled libraries so that we can run tests
	   * on software prior to installation.
	   */
	  ADD_PLATFORM_PATH(NSAllDomainsMask, uninstalled);

	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserLibrary);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalLibrary);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkLibrary);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemLibrary);
	}
	break;

      case NSDeveloperDirectory:
	{
#if	defined(_WIN32)
          if (nil == gnustepDeveloperDir)
            {
              NSString          *path = nil;
	      NSString		*bpath = nil;
	      NSString		*ipath = nil;
	      NSString		*mpath = nil;
              NSFileManager	*mgr;

              mgr = [NSFileManager defaultManager];

              /* See if we can find the developer root above the
               * system tools directory of the current running process.
               */
	      path = devroot(mgr, gnustepSystemTools);

	      /* Failing that, try looking above the base library.
	       */
	      if (nil == path)
		{
		  NSBundle	*baseBundle;

		  baseBundle = [NSBundle bundleForLibrary: @"gnustep-base"];
		  bpath = [baseBundle bundlePath];
		  path = devroot(mgr, bpath);
		}

              /* If we havent found the developer area relative to the
	       * hierarchy used by the current process, look for the
	       * GNUstep package installation root in case we have the
	       * developer environment installed from a package.
               */
              if (nil == path)
                {
                  HKEY	regKey;
                  BOOL	found = NO;

                  /* Open the key for the current user or local machine where
                   * the installation path for the GNUstep package is stored.
                   */
                  if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER,
L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\GNUstep",
                    0,
                    KEY_READ,
                    &regKey))
                    {
                      found = YES;
                    }
                  else
                    {
                      if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE,
L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\GNUstep",
                        0,
                        KEY_READ,
                        &regKey))
                        {
                          found = YES;
                        }
                    }
                  if (found)
                    {
                      wchar_t       buf[1024];
                      DWORD         bufsize;
                      DWORD         type;

                      bufsize = sizeof(buf);
                      if (ERROR_SUCCESS == RegQueryValueExW(regKey,
                        0, 0, &type, (BYTE *)buf, &bufsize))
                        {
                          ipath = [NSString stringWithCharacters: buf
                            length: wcslen(buf)];

			  path = devroot(mgr, ipath);
                        }
                      RegCloseKey(regKey);
                    }
                }
              if (nil == path)
                {
		  mpath = [NSBundle _absolutePathOfExecutable: @"make.exe"];
		  if (nil != mpath)
		    {
		      path = devroot(mgr, mpath);
		    }
		}
              ASSIGNCOPY(gnustepDeveloperDir, path);
	      if (nil == gnustepDeveloperDir)
		{
	          fprintf(stderr, "Failed to locate NSDeveloperDirectory"
                    " by GNUstep configuration, installed GNUstep package,"
                    " or process PATH.\n");
		}
            }
#endif
	  if (nil == gnustepDeveloperDir)
	    {
	      gnustepDeveloperDir = RETAIN(NSOpenStepRootDirectory());
	    }

	  /* The Developer directory is deprecated on OSX, but for GNUstep
	   * specific apps we return the root of the system containing the
	   * development environment.  On most systems, that's the root
	   * directory, but on windows it can be anywhere the user has put it.
	   * If not explicitly defined, or found relative to the hierarchy
	   * of the running process, *with the GNUstep package installed it's
	   * the location of the msys filesystem within the GNUstep package
	   * installation.
	   */
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepDeveloperDir);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepDeveloperDir);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepDeveloperDir);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepDeveloperDir);
	}
	break;

      case NSUserDirectory:
	{
	  /* This is the directory in which user directories are located.
	   * You can not have user directories in your own user directory,
	   * so NSUserDomainMask will always return ''.
	   */
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalUsersDir);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkUsersDir);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemUsersDir);
	}
	break;

      case NSDocumentationDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserDocumentation);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalDocumentation);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkDocumentation);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemDocumentation);
	}
	break;

      case NSDocumentDirectory:
	{
	  /* this is relative to the user home and for the user domain only
	   * verified on Macintosh
	   * despite the name it is Documents and not Document....
	   */
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Documents");
	}
	break;

      case NSDownloadsDirectory:
	{
	  /* Be consistent with NSDocumentDirectory */
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Downloads");
	}
	break;
	
      case NSMoviesDirectory:
	{
	  /* Be consistent with NSDocumentDirectory */
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Videos");
	}
	break;

      case NSMusicDirectory:
	{
	  /* Be consistent with NSDocumentDirectory */
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Music");
	}
	break;

      case NSPicturesDirectory:
	{
	  /* Be consistent with NSDocumentDirectory */
	  ADD_PATH(NSUserDomainMask, gnustepUserHome, @"Images");
	}
	break;

      case NSCachesDirectory:
	{
#ifdef __ANDROID__
	  /* Use system-provided cache directory on Android */
	  ADD_PATH(NSUserDomainMask, [[NSProcessInfo processInfo] androidCacheDir], @"");
#else
	  /* Uff - at the moment the only place to put Caches seems to
	   * be Library.  Unfortunately under GNU/Linux Library will
	   * end up in /usr/lib/GNUstep which could be mounted
	   * read-only!
	   */
	  ADD_PATH(NSUserDomainMask, gnustepUserLibrary, @"Caches");
#endif
	  ADD_PATH(NSLocalDomainMask, gnustepLocalLibrary, @"Caches");
	  ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibrary, @"Caches");
	  ADD_PATH(NSSystemDomainMask, gnustepSystemLibrary, @"Caches");
	}
	break;

      case NSApplicationSupportDirectory:
	{
	  ADD_PATH(NSUserDomainMask, gnustepUserLibrary, @"ApplicationSupport");
	  ADD_PATH(NSLocalDomainMask, gnustepLocalLibrary,
	    @"ApplicationSupport");
	  ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibrary,
	    @"ApplicationSupport");
	  ADD_PATH(NSSystemDomainMask, gnustepSystemLibrary,
	    @"ApplicationSupport");
	}
	break;

      /* Now the GNUstep additions */
      case GSFrameworksDirectory:
	{
	  ADD_PATH(NSUserDomainMask, gnustepUserLibrary, @"Frameworks");
	  ADD_PATH(NSLocalDomainMask, gnustepLocalLibrary, @"Frameworks");
	  ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibrary, @"Frameworks");
	  ADD_PATH(NSSystemDomainMask, gnustepSystemLibrary, @"Frameworks");
	}
	break;

      case GSFontsDirectory:
	{
	  ADD_PATH(NSUserDomainMask, gnustepUserLibrary, @"Fonts");
	  ADD_PATH(NSLocalDomainMask, gnustepLocalLibrary, @"Fonts");
	  ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibrary, @"Fonts");
	  ADD_PATH(NSSystemDomainMask, gnustepSystemLibrary, @"Fonts");
	}
	break;

      case GSLibrariesDirectory:
	{
	  NSString *full = nil;
	  NSString *part = nil;

	  if ([gnustep_is_flattened boolValue] == NO
	    && gnustep_target_cpu != nil && gnustep_target_os != nil)
	    {
	      part = [NSString stringWithFormat: @"%@-%@",
                gnustep_target_cpu, gnustep_target_os];
	      if (library_combo != nil)
		{
		  full = [part stringByAppendingPathComponent: library_combo];
		}
	    }

	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserLibraries);
	  if (full)
	    ADD_PATH(NSUserDomainMask, gnustepUserLibraries, full);
	  if (part)
	    ADD_PATH(NSUserDomainMask, gnustepUserLibraries, part);

	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalLibraries);
	  if (full)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalLibraries, full);
	  if (part)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalLibraries, part);

	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkLibraries);
	  if (full)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibraries, full);
	  if (part)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkLibraries, part);

	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemLibraries);
	  if (full)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemLibraries, full);
	  if (part)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemLibraries, part);
	}
	break;

      case GSToolsDirectory:
	{
	  NSString	*full = nil;
	  NSString	*part = nil;

	  if ([gnustep_is_flattened boolValue] == NO
	    && gnustep_target_cpu != nil && gnustep_target_os != nil)
	    {
	      part = [NSString stringWithFormat: @"%@-%@",
                gnustep_target_cpu, gnustep_target_os];
	      if (library_combo != nil)
		{
		  full = [part stringByAppendingPathComponent: library_combo];
		}
	    }

	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserTools);
	  if (full)
	    ADD_PATH(NSUserDomainMask, gnustepUserTools, full);
	  if (part)
	    ADD_PATH(NSUserDomainMask, gnustepUserTools, part);

	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalTools);
	  if (full)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalTools, full);
	  if (part)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalTools, part);

	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkTools);
	  if (full)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkTools, full);
	  if (part)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkTools, part);

	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemTools);
	  if (full)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemTools, full);
	  if (part)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemTools, part);
	}
	break;

      case GSAdminToolsDirectory:
	{
	  NSString	*full = nil;
	  NSString	*part = nil;

	  if ([gnustep_is_flattened boolValue] == NO
	    && gnustep_target_cpu != nil && gnustep_target_os != nil)
	    {
	      part = [NSString stringWithFormat: @"%@-%@",
                gnustep_target_cpu, gnustep_target_os];
	      if (library_combo != nil)
		{
		  full = [part stringByAppendingPathComponent: library_combo];
		}
	    }

	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserAdminTools);
	  if (full)
	    ADD_PATH(NSUserDomainMask, gnustepUserAdminTools, full);
	  if (part)
	    ADD_PATH(NSUserDomainMask, gnustepUserAdminTools, part);

	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalAdminTools);
	  if (full)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalAdminTools, full);
	  if (part)
	    ADD_PATH(NSLocalDomainMask, gnustepLocalAdminTools, part);

	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkAdminTools);
	  if (full)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkAdminTools, full);
	  if (part)
	    ADD_PATH(NSNetworkDomainMask, gnustepNetworkAdminTools, part);

	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemAdminTools);
	  if (full)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemAdminTools, full);
	  if (part)
	    ADD_PATH(NSSystemDomainMask, gnustepSystemAdminTools, part);
	}
	break;

      case GSWebApplicationsDirectory:
	{
	  ADD_PLATFORM_PATH(NSUserDomainMask, gnustepUserWebApps);
	  ADD_PLATFORM_PATH(NSLocalDomainMask, gnustepLocalWebApps);
	  ADD_PLATFORM_PATH(NSNetworkDomainMask, gnustepNetworkWebApps);
	  ADD_PLATFORM_PATH(NSSystemDomainMask, gnustepSystemWebApps);
	}
	break;
    }

#undef ADD_PATH
#undef ADD_PLATFORM_PATH

  count = [paths count];
  for (i = 0; i < count; i++)
    {
      path = [paths objectAtIndex: i];

      if (expandTilde == YES)
        {
          [paths replaceObjectAtIndex: i
                           withObject: [path stringByExpandingTildeInPath]];
        }
      else
        {
          [paths replaceObjectAtIndex: i
            withObject: [path stringByAbbreviatingWithTildeInPath]];
        }
    }

  return paths;
}

@implementation GSPathUtilities
+ (void) atExit
{
  ShutdownPathUtilities();
}
+ (void) initialize
{
  [self registerAtExit];
}
@end
