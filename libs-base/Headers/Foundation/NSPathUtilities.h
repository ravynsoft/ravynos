/** Interface to file path utilities for GNUStep
   Copyright (C) 1996-2016 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: May 1996
   
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
 
   AutogsdocSource:	NSPathUtilities.m
   */ 

#ifndef __NSPathUtilities_h_GNUSTEP_BASE_INCLUDE
#define __NSPathUtilities_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>
#import	<Foundation/NSObjCRuntime.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSDictionary;
@class	NSMutableDictionary;
@class	NSString;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/**
 * This extension permits a change of username from that specified in the
 * LOGNAME environment variable.  Using it will almost certainly cause
 * trouble if the process does not posses the file access privileges of the
 * new name.  This is provided primarily for use by processes that run as
 * system-manager and need to act as particular users.  It uses the
 * [NSUserDefaults +resetUserDefaults] extension to reset the defaults system
 * to use the defaults belonging to the new user.
 */
GS_EXPORT void
GSSetUserName(NSString *aName);

/**
 * Returns a mutable copy of the system-wide configuration used to
 * determine paths to locate files etc.<br />
 * If the newConfig argument is non-nil it is used to set the config
 * overriding any other version.  You should not change the config
 * after the user defaults system has been initialised as the new
 * config will not be picked up by the defaults system.<br />
 * <br />
 * A typical sequence of operation might be to<br />
 * Call the function with a nil argument to obtain the configuration
 * information currently in use (usually obtained from the main GNUstep
 * configuration file).<br />
 * Modify the dictionary contents.<br />
 * Call the function again passing back in the modified config.<br />
 * <br />
 * If you call this function with a non-nil argument before the system
 * configuration file has been read, you will prevent the file from
 * being read.  However, you must take care doing this that creation
 * of the config dictionary you are going to pass in to the function
 * does not have any side-effects which would cause the config file
 * to be read earlier.<br />
 * If you want to prevent the user specific config file from being
 * read, you must set the GNUSTEP_USER_CONFIG_FILE value in the
 * dictionary to be an empty string.
 */
GS_EXPORT NSMutableDictionary*
GNUstepConfig(NSDictionary *newConfig);

/**
 * Returns the location of the defaults database for the specified user.
 * This uses the same information you get from GNUstepConfig() and
 * GNUstepUserConfig() and builds the path to the defaults database
 * fromm it.
 */
GS_EXPORT NSString*
GSDefaultsRootForUser(NSString *userName);

/**
 * The config dictionary passed to this function should be a
 * system-wide config as provided by GNUstepConfig() ... and
 * this function merges in user specific configuration file
 * information if such a file exists and is owned by the user.<br />
 * NB. If the GNUSTEP_USER_CONFIG_FILE value in the system-wide
 * config is an empty string, no user-specifc config will be
 * read.
 */
GS_EXPORT void
GNUstepUserConfig(NSMutableDictionary *config, NSString *userName);

#endif
GS_EXPORT NSString *NSUserName(void);
GS_EXPORT NSString *NSHomeDirectory(void);
GS_EXPORT NSString *NSHomeDirectoryForUser(NSString *loginName);

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
enum
{
  NSApplicationDirectory = 1,		/** Applications */
  NSDemoApplicationDirectory,		/** Demos */
  NSDeveloperApplicationDirectory,	/** Deprecated */
  NSAdminApplicationDirectory,		/** Administration */
  NSLibraryDirectory,			/** Library */
  NSDeveloperDirectory,			/** Developer (deprecated on OSX). */
  NSUserDirectory,			/** user home directories */
  NSDocumentationDirectory,		/** Documentation */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  NSDocumentDirectory,			/** Documents */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
  NSCoreServicesDirectory,		/** CoreServices */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
  NSDesktopDirectory = 12,		/** location of users desktop */
  NSCachesDirectory = 13,		/** location of users cache files */
  NSApplicationSupportDirectory = 14,	/** location of app support files */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
  NSDownloadsDirectory = 15,	        /** location of downloaded files */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
  NSMoviesDirectory = 17,	        /** location of video/movie files */
  NSMusicDirectory = 18,	        /** location of music files */
  NSPicturesDirectory = 19,	        /** location of picture/images files */
  NSItemReplacementDirectory = 99,      /** pass to URLFirDirectory:inDomain:
                                            appropriateForURL:create:error to create a temporary directory */
#endif  

  NSAllApplicationsDirectory = 100,	/** all app directories */
  NSAllLibrariesDirectory = 101,	/** all library resources */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
  NSTrashDirectory = 102,	        /** location of the trash directory */ 
#endif

#define  GSApplicationSupportDirectory NSApplicationSupportDirectory
/*  GNUstep Directory Identifiers
 *  Start at 1000, we hope Apple will never overlap.
 */
  GSLibrariesDirectory = 1000,		/** libraries (binary code) */
  GSToolsDirectory,			/** non-gui programs */
  GSFontsDirectory,			/** font storage */
  GSFrameworksDirectory, 		/** frameworks */
  GSWebApplicationsDirectory,		/** web applications (GSWeb or SOPE) */
  GSAdminToolsDirectory	        	/** admin non-gui programs */
};
/**
 * Enumeration of possible requested directory type specifiers for
 * NSSearchPathForDirectoriesInDomains() function.  On a traditional
 * GNUstep filesystem layout these correspond to the subdirectories
 * that may be found in the various domains, such as "Library"
 * and "Applications".  In a different filesystem layout these
 * directories might be located anywhere on disk.
 <example>
{
  NSApplicationDirectory,
  NSDemoApplicationDirectory,
  NSDeveloperApplicationDirectory,
  NSAdminApplicationDirectory,
  NSLibraryDirectory,
  NSDeveloperDirectory,
  NSUserDirectory,
  NSDocumentationDirectory,
  NSDocumentDirectory,
  NSCoreServiceDirectory,
  NSDesktopDirectory,
  NSCachesDirectory,
  NSApplicationSupportDirectory
  NSAllApplicationsDirectory,
  NSAllLibrariesDirectory,
  NSItemReplacementDirectory,
  GSLibrariesDirectory,
  GSToolsDirectory,
  GSAdminToolsDirectory,
  GSFontsDirectory,
  GSFrameworksDirectory,
  GSWebApplicationsDirectory
}
 </example>
 */
typedef NSUInteger NSSearchPathDirectory;

enum
{
  NSUserDomainMask = 1,		/** The user's personal items */
  NSLocalDomainMask = 2,	/** Local for all users on the machine */
  NSNetworkDomainMask = 4,	/** Public for all users on network */
  NSSystemDomainMask = 8,	/** Standard GNUstep items */
  NSAllDomainsMask = 0x0ffff	/** all domains */
};
/**
 * Mask type for NSSearchPathForDirectoriesInDomains() function.  A bitwise OR
 * of one or more of <code>NSUserDomainMask, NSLocalDomainMask,
 * NSNetworkDomainMask, NSSystemDomainMask, NSAllDomainsMask</code>.
 */
typedef NSUInteger NSSearchPathDomainMask;

/**
 * Returns an array of search paths to look at for resources.<br/ >
 * The paths are returned in domain order:
 * USER, LOCAL, NETWORK then SYSTEM.<br />
 * The presence of a path in this list does <em>not</em> mean that the
 * path actually exists in the filesystem.<br />
 * If you are wanting to locate an existing resource, you should normally
 * call this function with NSAllDomainsMask, but if you wish to find the
 * path in which you should create a new file, you would generally
 * specify a particular domain, and then create the path in the file
 * system if it does not already exist.
 */
GS_EXPORT NSArray *NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory directoryKey, NSSearchPathDomainMask domainMask, BOOL expandTilde);

/**
 * Returns the full username of the current user.
 * If unable to determine this, returns the standard user name.
 */
GS_EXPORT NSString *NSFullUserName(void);

/**
 * Returns the standard paths in which applications are stored and
 * should be searched for.  Calls NSSearchPathForDirectoriesInDomains()<br/ >
 * Refer to the GNUstep File System Hierarchy documentation for more info.
 */
GS_EXPORT NSArray *NSStandardApplicationPaths(void);

/**
 * Returns the standard paths in which resources are stored and
 * should be searched for.  Calls NSSearchPathForDirectoriesInDomains()<br/ >
 * Refer to the GNUstep File System Hierarchy documentation for more info.
 */
GS_EXPORT NSArray *NSStandardLibraryPaths(void);

/**
 * Returns the name of a directory in which temporary files can be stored.
 * Under GNUstep this is a location which is not readable by other users.
 * <br />
 * If a suitable directory can't be found or created, this function raises an
 * NSGenericException.
 */
GS_EXPORT NSString *NSTemporaryDirectory(void);

/**
 * Returns the location of the <em>root</em> directory of the file
 * hierarchy. This lets you build paths in a system independent manner
 * (for instance the root on unix is '/' but on windows it is 'C:\')
 * by appending path components to the root.<br />
 * Don't assume that /System, /Network etc exist in this path (generally
 * they don't)! Use other path utility functions such as
 * NSSearchPathForDirectoriesInDomains() to find standard locations
 * for libraries, applications etc.<br />
 * Refer to the GNUstep File System Hierarchy documentation for more info.
 */
GS_EXPORT NSString *NSOpenStepRootDirectory(void);
#endif /* GS_API_MACOSX */

#if	defined(__cplusplus)
}
#endif

#endif /* __NSPathUtilities_h_GNUSTEP_BASE_INCLUDE */
