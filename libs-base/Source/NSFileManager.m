 /**
   NSFileManager.m

   Copyright (C) 1997-2020 Free Software Foundation, Inc.

   Author: Mircea Oancea <mircea@jupiter.elcom.pub.ro>
   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: Feb 1997
   Updates and fixes: Richard Frith-Macdonald

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: Apr 2001
   Rewritten NSDirectoryEnumerator

   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: Sep 2002
   Rewritten attribute handling code

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

   <title>NSFileManager class reference</title>
   $Date$ $Revision$
*/

/* The following define is needed for Solaris get(pw/gr)(nam/uid)_r declartions
   which default to pre POSIX declaration.  */
#define _POSIX_PTHREAD_SEMANTICS

#import "common.h"
#define	EXPOSE_NSFileManager_IVARS	1
#define	EXPOSE_NSDirectoryEnumerator_IVARS	1
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSValue.h"
#import "GSPrivate.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSTask+GNUstepBase.h"

#include <stdio.h>

/* determine directory reading files */

#if defined(HAVE_DIRENT_H)
# include <dirent.h>
#elif defined(HAVE_SYS_DIR_H)
# include <sys/dir.h>
#elif defined(HAVE_SYS_NDIR_H)
# include <sys/ndir.h>
#elif defined(HAVE_NDIR_H)
# include <ndir.h>
#endif

#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif

#if	defined(_WIN32)
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <accctrl.h>
#include <aclapi.h>
#define	WIN32ERR	((DWORD)0xFFFFFFFF)
#endif

/* determine filesystem max path length */

#if defined(_POSIX_VERSION) || defined(_WIN32)
# if defined(_WIN32)
#   include <sys/utime.h>
# else
#   include <utime.h>
# endif
#endif

#ifdef HAVE_SYS_CDEFS_H
# include <sys/cdefs.h>
#endif
#ifdef HAVE_SYS_SYSLIMITS_H
# include <sys/syslimits.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>		/* for MAXPATHLEN */
#endif

#ifndef PATH_MAX
# ifdef _POSIX_VERSION
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  ifdef MAXPATHLEN
#   define PATH_MAX MAXPATHLEN
#  else
#   define PATH_MAX 1024
#  endif
# endif
#endif

/* determine if we have statfs struct and function */

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif

#if	defined(HAVE_SYS_FILE_H)
# include <sys/file.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>     /* For struct passwd */
#endif
#ifdef HAVE_GRP_H
#include <grp.h>     /* For struct group */
#endif
#ifdef HAVE_UTIME_H
# include <utime.h>
#endif

/*
 * On systems that have the O_BINARY flag, use it for a binary copy.
 */
#if defined(O_BINARY)
#define	GSBINIO	O_BINARY
#else
#define	GSBINIO	0
#endif

@interface NSDirectoryEnumerator (Local)
- (id) initWithDirectoryPath: (NSString*)path 
   recurseIntoSubdirectories: (BOOL)recurse
              followSymlinks: (BOOL)follow
                justContents: (BOOL)justContents
			 for: (NSFileManager*)mgr;

- (id) initWithDirectoryPath: (NSString*)path
   recurseIntoSubdirectories: (BOOL)recurse
	      followSymlinks: (BOOL)follow
		justContents: (BOOL)justContents
                  skipHidden: (BOOL)skipHidden
                errorHandler: (GSDirEnumErrorHandler) handler
			 for: (NSFileManager*)mgr;

- (void) _setSkipHidden: (BOOL)flag;
- (void) _setErrorHandler: (GSDirEnumErrorHandler) handler;
@end

/*
 * Macros to handle unichar filesystem support.
 */

#if defined(_MSC_VER)

#warning NSFileManager is currently unsupported on Windows MSVC

#define	_CHMOD(A,B)	0
#define	_CLOSEDIR(A)
#define	_OPENDIR(A)	NULL
#define	_READDIR(A)	NULL
#define	_RENAME(A,B)	-1
#define	_RMDIR(A)	-1
#define	_STAT(A,B)	-1
#define	_UTIME(A,B)	-1

#define	_CHAR		unichar
#define	_DIR		void
#define	_DIRENT		{const char *d_name;}
#define	_STATB		{int st_ctime; int st_gid; int st_atime; int st_mtime; int st_mode; int st_uid; int st_size; int st_ino; int st_dev; int st_nlink;}
#define	_UTIMB		{int actime; int modtime;}

#define	_NUL		L'\0'

#elif	defined(_WIN32)

#define	_CHMOD(A,B)	_wchmod(A,B)
#define	_CLOSEDIR(A)	_wclosedir(A)
#define	_OPENDIR(A)	_wopendir(A)
#define	_READDIR(A)	_wreaddir(A)
#define	_RENAME(A,B)	(MoveFileExW(A,B,MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)==0)?-1:0
#define	_RMDIR(A)	_wrmdir(A)
#define	_STAT(A,B)	_wstat(A,B)
#define	_UTIME(A,B)	_wutime(A,B)

#define	_CHAR		unichar
#define	_DIR		_WDIR
#define	_DIRENT		_wdirent
#define	_STATB		_stat
#define	_UTIMB		_utimbuf

#define	_NUL		L'\0'

#else

#define	_CHMOD(A,B)	chmod(A,B)
#define	_CLOSEDIR(A)	closedir(A)
#define	_OPENDIR(A)	opendir(A)
#define	_READDIR(A)	readdir(A)
#define	_RENAME(A,B)	rename(A,B)
#define	_RMDIR(A)	rmdir(A)
#define	_STAT(A,B)	stat(A,B)
#define	_UTIME(A,B)	utime(A,B)

#define	_CHAR		char
#define	_DIR		DIR
#define	_DIRENT		dirent
#define	_STATB		stat
#define	_UTIMB		utimbuf

#define	_NUL		'\0'

#endif

#define	_CCP		const _CHAR*




/*
 * GSAttrDictionary is a private NSDictionary subclass used to
 * handle file attributes efficiently ...  using lazy evaluation
 * to ensure that we only do the minimum work necessary at any time.
 */
@interface	GSAttrDictionary : NSDictionary
{
@public
  struct _STATB	statbuf;
  _CHAR		_path[0];
}
+ (NSDictionary*) attributesAt: (NSString *)path
		  traverseLink: (BOOL)traverse;
@end

static Class	GSAttrDictionaryClass = 0;

/*
 * We also need a special enumerator class to enumerate the dictionary.
 */
@interface	GSAttrDictionaryEnumerator : NSEnumerator
{
  NSDictionary	*dictionary;
  NSEnumerator	*enumerator;
}
+ (NSEnumerator*) enumeratorFor: (NSDictionary*)d;
@end



@interface NSFileManager (PrivateMethods)

/* Copies the contents of source file to destination file. Assumes source
   and destination are regular files or symbolic links. */
- (BOOL) _copyFile: (NSString*)source
	    toFile: (NSString*)destination
	   handler: (id)handler;

/* Recursively copies the contents of source directory to destination. */
- (BOOL) _copyPath: (NSString*)source
	    toPath: (NSString*)destination
	   handler: (id)handler;

/* Recursively links the contents of source directory to destination. */
- (BOOL) _linkPath: (NSString*)source
	    toPath: (NSString*)destination
	   handler: handler;

/* encapsulates the will Process check for existence of selector. */
- (void) _sendToHandler: (id) handler
        willProcessPath: (NSString*) path;

/* methods to encapsulates setting up and calling the handler
   in case of an error */
- (BOOL) _proceedAccordingToHandler: (id) handler
                           forError: (NSString*) error
                             inPath: (NSString*) path;

- (BOOL) _proceedAccordingToHandler: (id) handler
                           forError: (NSString*) error
                             inPath: (NSString*) path
                           fromPath: (NSString*) fromPath
                             toPath: (NSString*) toPath;

/* A convenience method to return an NSError object.
 * If the _lastError message is set, this creates an NSError using
 * that message in the NSCocoaErrorDomain, otherwise it used the
 * most recent system error and the Posix error domain.
 * The userInfo is set to contain NSLocalizedDescriptionKey for the
 * message text, 'Path' if only the fromPath argument is specified,
 * and 'FromPath' and 'ToPath' if both path argument are specified.
 */
- (NSError*) _errorFrom: (NSString*)fromPath to: (NSString*)toPath;
				   
@end /* NSFileManager (PrivateMethods) */

/**
 *  This is the main class for platform-independent management of the local
 *  filesystem, which allows you to read and save files, create/list
 *  directories, and move or delete files and directories.  In addition to
 *  simply listing directories, you may obtain an [NSDirectoryEnumerator]
 *  instance for recursive directory contents enumeration.
 */
@implementation NSFileManager

// Getting the default manager

static NSFileManager* defaultManager = nil;
static NSStringEncoding	defaultEncoding;

+ (NSFileManager*) defaultManager
{
  if (defaultManager == nil)
    {
      NS_DURING
	{
	  [gnustep_global_lock lock];
	  if (defaultManager == nil)
	    {
	      defaultManager = [[self alloc] init];
	    }
	  [gnustep_global_lock unlock];
	}
      NS_HANDLER
	{
	  // unlock then re-raise the exception
	  [gnustep_global_lock unlock];
	  [localException raise];
	}
      NS_ENDHANDLER
    }
  return defaultManager;
}

+ (void) initialize
{
  defaultEncoding = [NSString defaultCStringEncoding];
  GSAttrDictionaryClass = [GSAttrDictionary class];
}

- (void) dealloc
{
  TEST_RELEASE(_lastError);
  [super dealloc];
}

- (id<NSFileManagerDelegate>) delegate
{
  return _delegate;
}

- (void) setDelegate: (id<NSFileManagerDelegate>)delegate
{
  _delegate = delegate;
}

- (BOOL) changeCurrentDirectoryPath: (NSString*)path
{
  static Class	bundleClass = 0;
  const _CHAR	*lpath = [self fileSystemRepresentationWithPath: path];

  /*
   * On some systems the only way NSBundle can determine the path to the
   * executable is by searching for it ... so it needs to know what was
   * the current directory at launch time ... so we must make sure it is
   * initialised before we change the current directory.
   */
  if (bundleClass == 0)
    {
      bundleClass = [NSBundle class];
    }
#if defined(_WIN32)
  return SetCurrentDirectoryW(lpath) == TRUE ? YES : NO;
#else
  return (chdir(lpath) == 0) ? YES : NO;
#endif
}

/**
 * Change the attributes of the file at path to those specified.<br />
 * Returns YES if all requested changes were made (or if the dictionary
 * was nil or empty, so no changes were requested), NO otherwise.<br />
 * On failure, some of the requested changes may have taken place.<br />
 */
- (BOOL) changeFileAttributes: (NSDictionary*)attributes atPath: (NSString*)path
{
  NSDictionary  *old;
  const _CHAR	*lpath = 0;
  NSUInteger	num;
  NSString	*str;
  NSDate	*date;
  BOOL		allOk = YES;

  if (0 == [attributes count])
    {
      return YES;
    }
  old = [self fileAttributesAtPath: path traverseLink: YES];
  lpath = [defaultManager fileSystemRepresentationWithPath: path];

#ifndef _WIN32
  if (object_getClass(attributes) == GSAttrDictionaryClass)
    {
      num = ((GSAttrDictionary*)attributes)->statbuf.st_uid;
    }
  else
    {
      NSNumber	*tmpNum = [attributes fileOwnerAccountID];

      num = tmpNum ? [tmpNum unsignedLongValue] : NSNotFound;
    }
  if (num != NSNotFound && num != [[old fileOwnerAccountID] unsignedLongValue])
    {
      if (chown(lpath, num, -1) != 0)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFileOwnerAccountID to '%"PRIuPTR"' - %@",
	    num, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }
  else
    {
      if ((str = [attributes fileOwnerAccountName]) != nil
        && NO == [str isEqual: [old fileOwnerAccountName]])
	{
	  BOOL	ok = NO;
#ifdef HAVE_PWD_H
#if     defined(HAVE_GETPWNAM_R)
	  struct passwd pw;
	  struct passwd *p;
          char buf[BUFSIZ*10];

	  if (getpwnam_r([str cStringUsingEncoding: defaultEncoding],
            &pw, buf, sizeof(buf), &p) == 0)
	    {
	      ok = (chown(lpath, pw.pw_uid, -1) == 0);
	      (void)chown(lpath, -1, pw.pw_gid);
	    }
#else
#if     defined(HAVE_GETPWNAM)
	  struct passwd *pw;

          [gnustep_global_lock lock];
	  pw = getpwnam([str cStringUsingEncoding: defaultEncoding]);
	  if (pw != 0)
	    {
	      ok = (chown(lpath, pw->pw_uid, -1) == 0);
	      (void)chown(lpath, -1, pw->pw_gid);
	    }
          [gnustep_global_lock unlock];
#endif
#endif
#endif
	  if (ok == NO)
	    {
	      allOk = NO;
	      str = [NSString stringWithFormat:
		@"Unable to change NSFileOwnerAccountName to '%@' - %@",
		str, [NSError _last]];
	      ASSIGN(_lastError, str);
	    }
	}
    }

  if (object_getClass(attributes) == GSAttrDictionaryClass)
    {
      num = ((GSAttrDictionary*)attributes)->statbuf.st_gid;
    }
  else
    {
      NSNumber	*tmpNum = [attributes fileGroupOwnerAccountID];

      num = tmpNum ? [tmpNum unsignedLongValue] : NSNotFound;
    }
  if (num != NSNotFound
    && num != [[old fileGroupOwnerAccountID] unsignedLongValue])
    {
      if (chown(lpath, -1, num) != 0)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFileGroupOwnerAccountID to '%"PRIuPTR"' - %@",
	    num, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }
  else if ((str = [attributes fileGroupOwnerAccountName]) != nil
    && NO == [str isEqual: [old fileGroupOwnerAccountName]])
    {
      BOOL	ok = NO;
#ifdef HAVE_GRP_H
#ifdef HAVE_GETGRNAM_R
      struct group gp;
      struct group *p;
      char buf[BUFSIZ*10];

      if (getgrnam_r([str cStringUsingEncoding: defaultEncoding], &gp,
        buf, sizeof(buf), &p) == 0)
        {
	  if (chown(lpath, -1, gp.gr_gid) == 0)
	    ok = YES;
        }
#else
#ifdef HAVE_GETGRNAM
      struct group *gp;
      
      [gnustep_global_lock lock];
      gp = getgrnam([str cStringUsingEncoding: defaultEncoding]);
      if (gp)
	{
	  if (chown(lpath, -1, gp->gr_gid) == 0)
	    ok = YES;
	}
      [gnustep_global_lock unlock];
#endif
#endif
#endif
      if (ok == NO)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFileGroupOwnerAccountName to '%@' - %@",
	    str, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }
#endif	/* _WIN32 */

  num = [attributes filePosixPermissions];
  if (num != NSNotFound && num != [old filePosixPermissions])
    {
      if (_CHMOD(lpath, num) != 0)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFilePosixPermissions to '%o' - %@",
	    (unsigned)num, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }

  date = [attributes fileCreationDate];
  if (date != nil && NO == [date isEqual: [old fileCreationDate]])
    {
      BOOL		ok = NO;
      struct _STATB	sb;
#if  defined(_WIN32)
      const _CHAR *lpath;
#else
      const char  *lpath;
#endif

      lpath = [self fileSystemRepresentationWithPath: path];
      if (_STAT(lpath, &sb) != 0)
	{
	  ok = NO;
	}
#if  defined(_WIN32)
      else if (sb.st_mode & _S_IFDIR)
	{
	  ok = YES;	// Directories don't have creation times.
	}
#endif
      else
	{
#if  defined(_WIN32)
          FILETIME ctime;
	  HANDLE fh;
          ULONGLONG nanosecs = ((ULONGLONG)([date timeIntervalSince1970]*10000000)+116444736000000000ULL);
          fh = CreateFileW(lpath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
          if (fh == INVALID_HANDLE_VALUE)
            {
              ok = NO;
            }
	  else
            {
	      ctime.dwLowDateTime  = (DWORD) (nanosecs & 0xFFFFFFFF );
              ctime.dwHighDateTime = (DWORD) (nanosecs >> 32 );
	      ok = SetFileTime(fh, &ctime, NULL, NULL);
              CloseHandle(fh);
	    }
#else
	  NSTimeInterval ti = [date timeIntervalSince1970];
/* on Unix we try setting the creation date by setting the modification date earlier than the current one */
#if defined (HAVE_UTIMENSAT)
          struct timespec ub[2];
	  ub[0].tv_sec = 0;
	  ub[0].tv_nsec = UTIME_OMIT; // we don't touch access time
	  ub[1].tv_sec = (time_t)trunc(ti);
	  ub[1].tv_nsec = (long)trunc((ti - trunc(ti)) * 1.0e9);

	  ok = (utimensat(AT_FDCWD, lpath, ub, 0) == 0);
#elif  defined(_POSIX_VERSION)
          struct _UTIMB ub;
	  ub.actime = sb.st_atime;
	  ub.modtime = ti;
	  ok = (_UTIME(lpath, &ub) == 0);
#else
          time_t ub[2];
	  ub[0] = sb.st_atime;
	  ub[1] = ti;
	  ok = (_UTIME(lpath, ub) == 0);
#endif
#endif
	}
      if (ok == NO)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFileCreationDate to '%@' - %@",
	    date, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }

  date = [attributes fileModificationDate];
  if (date != nil && NO == [date isEqual: [old fileModificationDate]])
    {
      BOOL		ok = NO;
      struct _STATB	sb;

      if (_STAT(lpath, &sb) != 0)
	{
	  ok = NO;
	}
#if  defined(_WIN32)
      else if (sb.st_mode & _S_IFDIR)
	{
	  ok = YES;	// Directories don't have modification times.
	}
#endif
      else
	{
	  NSTimeInterval ti = [date timeIntervalSince1970];
#if defined (HAVE_UTIMENSAT)
          struct timespec ub[2];
	  ub[0].tv_sec = 0;
	  ub[0].tv_nsec = UTIME_OMIT; // we don't touch access time
	  ub[1].tv_sec = (time_t)trunc(ti);
	  ub[1].tv_nsec = (long)trunc((ti - trunc(ti)) * 1.0e9);

	  ok = (utimensat(AT_FDCWD, lpath, ub, 0) == 0);
#elif  defined(_WIN32) || defined(_POSIX_VERSION)
          struct _UTIMB ub;
	  ub.actime = sb.st_atime;
	  ub.modtime = ti;
	  ok = (_UTIME(lpath, &ub) == 0);
#else
          time_t ub[2];
	  ub[0] = sb.st_atime;
	  ub[1] = ti;
	  ok = (_UTIME(lpath, ub) == 0);
#endif
	}
      if (ok == NO)
	{
	  allOk = NO;
	  str = [NSString stringWithFormat:
	    @"Unable to change NSFileModificationDate to '%@' - %@",
	    date, [NSError _last]];
	  ASSIGN(_lastError, str);
	}
    }

  return allOk;
}

/**
 * Returns an array of path components suitably modified for display
 * to the end user.  This modification may render the returned strings
 * unusable for path manipulation, so you should work with two arrays ...
 * one returned by this method (for display to the user), and a
 * parallel one returned by [NSString-pathComponents] (for path
 * manipulation).
 */
- (NSArray*) componentsToDisplayForPath: (NSString*)path
{
  return [path pathComponents];
}

/**
 * Reads the file at path an returns its contents as an NSData object.<br />
 * If an error occurs or if path specifies a directory etc then nil is
 * returned.
 */
- (NSData*) contentsAtPath: (NSString*)path
{
  return [NSData dataWithContentsOfFile: path];
}

/**
 * Returns YES if the contents of the file or directory at path1 are the same
 * as those at path2.<br />
 * If path1 and path2 are files, this is a simple comparison.  If they are
 * directories, the contents of the files in those subdirectories are
 * compared recursively.<br />
 * Symbolic links are not followed.<br />
 * A comparison checks first file identity, then size, then content.
 */
- (BOOL) contentsEqualAtPath: (NSString*)path1 andPath: (NSString*)path2
{
  NSDictionary	*d1;
  NSDictionary	*d2;
  NSString	*t;

  if ([path1 isEqual: path2])
    return YES;
  d1 = [self fileAttributesAtPath: path1 traverseLink: NO];
  d2 = [self fileAttributesAtPath: path2 traverseLink: NO];
  t = [d1 fileType];
  if ([t isEqual: [d2 fileType]] == NO)
    {
      return NO;
    }
  if ([t isEqual: NSFileTypeRegular])
    {
      if ([d1 fileSize] == [d2 fileSize])
	{
	  NSData	*c1 = [NSData dataWithContentsOfFile: path1];
	  NSData	*c2 = [NSData dataWithContentsOfFile: path2];

	  if ([c1 isEqual: c2])
	    {
	      return YES;
	    }
	}
      return NO;
    }
  else if ([t isEqual: NSFileTypeDirectory])
    {
      NSArray	*a1 = [self directoryContentsAtPath: path1];
      NSArray	*a2 = [self directoryContentsAtPath: path2];
      unsigned	index, count = [a1 count];
      BOOL	ok = YES;

      if ([a1 isEqual: a2] == NO)
	{
	  return NO;
	}
      for (index = 0; ok == YES && index < count; index++)
	{
	  NSString	*n = [a1 objectAtIndex: index];
	  NSString	*p1;
	  NSString	*p2;
	  ENTER_POOL

	  p1 = [path1 stringByAppendingPathComponent: n];
	  p2 = [path2 stringByAppendingPathComponent: n];
	  d1 = [self fileAttributesAtPath: p1 traverseLink: NO];
	  d2 = [self fileAttributesAtPath: p2 traverseLink: NO];
	  t = [d1 fileType];
	  if ([t isEqual: [d2 fileType]] == NO)
	    {
	      ok = NO;
	    }
	  else if ([t isEqual: NSFileTypeDirectory]
            || [t isEqual: NSFileTypeRegular])
	    {
	      ok = [self contentsEqualAtPath: p1 andPath: p2];
	    }
	  LEAVE_POOL
	}
      return ok;
    }
  else
    {
      return YES;
    }
}

- (NSArray*) contentsOfDirectoryAtURL: (NSURL*)url
           includingPropertiesForKeys: (NSArray*)keys
                              options: (NSDirectoryEnumerationOptions)mask
                                error: (NSError **)error
{
  NSArray               *result;
  NSDirectoryEnumerator *direnum;
  NSString              *path;
  
  DESTROY(_lastError);

  if (![[url scheme] isEqualToString: @"file"])
    {
      return nil;
    }
  path = [url path];
  
  direnum = [[NSDirectoryEnumerator alloc]
		       initWithDirectoryPath: path
                   recurseIntoSubdirectories: NO
                              followSymlinks: NO
                                justContents: NO
                                         for: self];

  /* we make an array of NSURLs */
  result = nil;
  if (nil != direnum)
    {
      IMP	        nxtImp;
      NSMutableArray    *urlArray;
      NSString          *tempPath;


      nxtImp = [direnum methodForSelector: @selector(nextObject)];

      urlArray = [NSMutableArray arrayWithCapacity: 128];
      while ((tempPath = (*nxtImp)(direnum, @selector(nextObject))) != nil)
	{
          NSURL         *tempURL;
          NSString      *lastComponent;
      
          tempURL = [NSURL fileURLWithPath: tempPath];
          lastComponent = [tempPath lastPathComponent];
          
          /* we purge files beginning with . */
          if (!((mask & NSDirectoryEnumerationSkipsHiddenFiles)
            && [lastComponent hasPrefix: @"."]))
            {
              [urlArray addObject: tempURL];
            }
	}
      RELEASE(direnum);
 
      if ([urlArray count] > 0)
        {
          result = [NSArray arrayWithArray: urlArray];
        }
    }

  if (error != NULL)
    {
      if (nil == result)
	{
	  *error = [self _errorFrom: path to: nil];
	}
    }

  return result;  
}

- (NSURL *)URLForDirectory: (NSSearchPathDirectory)directory 
                  inDomain: (NSSearchPathDomainMask)domain 
         appropriateForURL: (NSURL *)url 
                    create: (BOOL)shouldCreate 
                     error: (NSError **)error
{
  NSURL *result = nil;
  NSArray *urlArray = NSSearchPathForDirectoriesInDomains(directory, domain, YES);

  // Find out the URL exists...
  if ([urlArray count] > 0)
    {
      result = [NSURL URLWithString: [urlArray objectAtIndex: 0]];
    }

  if (directory == NSItemReplacementDirectory)
    {
      result = [NSURL URLWithString: NSTemporaryDirectory()];
    }

  if (![self fileExistsAtPath: [result absoluteString]])
      {
        // If we should created it, create it...
        if (shouldCreate)
          {
            [self       createDirectoryAtPath: [result absoluteString]
                  withIntermediateDirectories: YES
                                   attributes: nil
                                        error: error];
          }
      }
  
  return result;
}

- (NSDirectoryEnumerator *)enumeratorAtURL: (NSURL *)url
                includingPropertiesForKeys: (NSArray *)keys 
                                   options: (NSDirectoryEnumerationOptions)mask 
                              errorHandler: (GSDirEnumErrorHandler)handler
{
  NSDirectoryEnumerator *direnum;
  NSString              *path;
  
  DESTROY(_lastError);

  if (![[url scheme] isEqualToString: @"file"])
    {
      return nil;
    }
  path = [url path];
  
  direnum = [[NSDirectoryEnumerator alloc]
		       initWithDirectoryPath: path
                   recurseIntoSubdirectories: !(mask & NSDirectoryEnumerationSkipsSubdirectoryDescendants) 
                              followSymlinks: NO
                                justContents: NO
                                  skipHidden: (mask & NSDirectoryEnumerationSkipsHiddenFiles)
                                errorHandler: handler
                                         for: self];

  return direnum;  
}

- (NSArray*) contentsOfDirectoryAtPath: (NSString*)path error: (NSError**)error
{
  NSArray       *result;

  DESTROY(_lastError);
  result = [self directoryContentsAtPath: path];

  if (error != NULL)
    {
      if (nil == result)
	{
	  *error = [self _errorFrom: path to: nil];
	}
    }

  return result; 
}

/**
 * Creates a new directory (and all intermediate directories if flag is YES).
 * Creates only the last directory in the path if flag is NO.<br />
 * The directory is created with the attributes specified, and any problem
 * is returned in error.<br />
 * Returns YES if the directory is created (or flag is YES and the directory
 * already exists), NO on failure.
 */
- (BOOL) createDirectoryAtPath: (NSString *)path
   withIntermediateDirectories: (BOOL)flag
		    attributes: (NSDictionary *)attributes
			 error: (NSError **)error
{
  BOOL result = NO;

  DESTROY(_lastError);
  if (YES == flag)
    {
      NSEnumerator      *paths = [[path pathComponents] objectEnumerator];
      NSString          *path = nil;
      NSString          *dir = [NSString string];

      result = YES;
      while (YES == result && (path = (NSString *)[paths nextObject]) != nil)
	{
	  dir = [dir stringByAppendingPathComponent: path];
	  // create directory only if it doesn't exist
	  if (NO == [self fileExistsAtPath: dir])
	    {
	      result = [self createDirectoryAtPath: dir
		     			attributes: attributes];
	    }
	}
    }
  else
    {
      BOOL isDir;

      if ([self fileExistsAtPath: [path stringByDeletingLastPathComponent]
	isDirectory: &isDir] && isDir)
        {
          result = [self createDirectoryAtPath: path
                                    attributes: attributes];
        }
      else
        {
          result = NO;  
          ASSIGN(_lastError, @"Could not create directory - intermediate path did not exist or was not a directory");
        }
    }  

  if (error != NULL)
    {
      if (NO == result)
	{
	  *error = [self _errorFrom: path to: nil];
	}
    }

  return result;
}

/**
 * Creates a new directory and all intermediate directories in the file URL
 * if flag is YES.<br />
 * Creates only the last directory in the URL if flag is NO.<br />
 * The directory is created with the attributes specified and any problem
 * is returned in error.<br />
 * Returns YES if the directory is created (or flag is YES and the directory
 * already exists), NO on failure.
 */
- (BOOL) createDirectoryAtURL: (NSURL *)url
  withIntermediateDirectories: (BOOL)flag
		   attributes: (NSDictionary *)attributes
			error: (NSError **) error
{
  return [self createDirectoryAtPath: [url path]
	 withIntermediateDirectories: flag 
			  attributes: attributes
			       error: error];
}

/**
 * Creates a new directory, and sets its attributes as specified.<br />
 * Fails if directories in the path are missing.<br />
 * Returns YES if the directory was actually created, NO otherwise.
 */
- (BOOL) createDirectoryAtPath: (NSString*)path
		    attributes: (NSDictionary*)attributes
{
  BOOL  isDir;

  /* This is consistent with MacOSX - just return NO for an invalid path. */
  if ([path length] == 0)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

  if (YES == [self fileExistsAtPath: path isDirectory: &isDir])
    {
      NSString  *e;

      if (NO == isDir)
        {
          e = [NSString stringWithFormat:
            @"path %@ exists, but is not a directory", path];
        }
      else
        {
          e = [NSString stringWithFormat:
            @"path %@ exists ... cannot create", path];
        }
      ASSIGN(_lastError, e);
      return NO;
    }
  else
    {
#if defined(_WIN32)
      const _CHAR   *lpath;
          
      lpath = [self fileSystemRepresentationWithPath: path];
      isDir = (CreateDirectoryW(lpath, 0) != FALSE) ? YES : NO;
#else
      const char    *lpath;

      lpath = [self fileSystemRepresentationWithPath: path];
      isDir = (mkdir(lpath, 0777) == 0) ? YES : NO;
      if (YES == isDir)
        {
          /*
           * If there is no file owner specified, and we are running
           * setuid to root, then we assume we need to change ownership
           * to the correct user.
           */
          if (attributes == nil || ([attributes fileOwnerAccountID] == nil
            && [attributes fileOwnerAccountName] == nil))
            {
              if (geteuid() == 0
                && [@"root" isEqualToString: NSUserName()] == NO)
                {
                  NSMutableDictionary       *m;

                  m = [[attributes mutableCopy] autorelease];
                  if (nil == m)
                    {
                      m = [NSMutableDictionary dictionaryWithCapacity: 1];
                    }
                  [m setObject: NSUserName()
                        forKey: NSFileOwnerAccountName];
                  attributes = m;
                }
            }
        }
#endif
      if (NO == isDir)
        {
          NSString	*e;

          e = [NSString stringWithFormat:
            @"Could not create '%@' - '%@'",
            path, [NSError _last]];
          ASSIGN(_lastError, e);
          return NO;
        }
    }

  return [self changeFileAttributes: attributes atPath: path];
}

/**
 * Creates a new file, and sets its attributes as specified.<br />
 * Initialises the file content with the specified data.<br />
 * Returns YES on success, NO on failure.
 */
- (BOOL) createFileAtPath: (NSString*)path
		 contents: (NSData*)contents
	       attributes: (NSDictionary*)attributes
{
#if	defined(_WIN32)
  const _CHAR *lpath = [self fileSystemRepresentationWithPath: path];
  HANDLE fh;
  DWORD	written = 0;
  DWORD	len = [contents length];
#else
  const char	*lpath;
  int	fd;
  int	len;
  int	written;
#endif

  /* This is consistent with MacOSX - just return NO for an invalid path. */
  if ([path length] == 0)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if	defined(_WIN32)
  fh = CreateFileW(lpath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL, 0);
  if (fh == INVALID_HANDLE_VALUE)
    {
      return NO;
    }
  else
    {
      if (len > 0)
	{
	  WriteFile(fh, [contents bytes], len, &written, 0);
	}
      CloseHandle(fh);
      if (attributes != nil
	&& [self changeFileAttributes: attributes atPath: path] == NO)
	{
	  return NO;
	}
      return YES;
    }
#else
  lpath = [self fileSystemRepresentationWithPath: path];

  fd = open(lpath, GSBINIO|O_WRONLY|O_TRUNC|O_CREAT, 0644);
  if (fd < 0)
    {
      return NO;
    }
  if (attributes != nil
    && [self changeFileAttributes: attributes atPath: path] == NO)
    {
      close (fd);
      return NO;
    }

  /*
   * If there is no file owner specified, and we are running setuid to
   * root, then we assume we need to change ownership to correct user.
   */
  if (attributes == nil || ([attributes fileOwnerAccountID] == nil
    && [attributes fileOwnerAccountName] == nil))
    {
      if (geteuid() == 0 && [@"root" isEqualToString: NSUserName()] == NO)
	{
	  attributes = [NSDictionary dictionaryWithObjectsAndKeys:
	    NSFileOwnerAccountName, NSUserName(), nil];
	  if (![self changeFileAttributes: attributes atPath: path])
	    {
	      NSDebugLog(@"Failed to change ownership of '%@' to '%@'",
		path, NSUserName());
	    }
	}
    }
  len = [contents length];
  if (len > 0)
    {
      written = write(fd, [contents bytes], len);
    }
  else
    {
      written = 0;
    }
  close (fd);
#endif
  return written == len;
}

/**
 * Returns the current working directory used by all instance of the file
 * manager in the current task.
 */
- (NSString*) currentDirectoryPath
{
  NSString *currentDir = nil;

#if defined(_WIN32)
  int len = GetCurrentDirectoryW(0, 0);
  if (len > 0)
    {
      _CHAR *lpath = (_CHAR*)calloc(len+10,sizeof(_CHAR));

      if (lpath != 0)
	{
	  if (GetCurrentDirectoryW(len, lpath)>0)
	    {
	      NSString	*path;

	      // Windows may count the trailing nul ... we don't want to.
	      if (len > 0 && lpath[len] == 0) len--;
	      path = [[NSString alloc] initWithCharacters: lpath length: len];
	      // Standardise to get rid of backslashes
	      currentDir = [path stringByStandardizingPath];
	      RELEASE(path);
	    }
	  free(lpath);
	}
    }
#else
  char path[PATH_MAX];
#ifdef HAVE_GETCWD
  if (getcwd(path, PATH_MAX-1) == 0)
    return nil;
#else
  if (getwd(path) == 0)
    return nil;
#endif /* HAVE_GETCWD */
  currentDir = [self stringWithFileSystemRepresentation: path
						 length: strlen(path)];
#endif /* !_WIN32 */

  return currentDir;
}

/**
 * Copies the file or directory at source to destination, using a
 * handler object which should respond to
 * [NSObject(NSFileManagerHandler)-fileManager:willProcessPath:] and
 * [NSObject(NSFileManagerHandler)-fileManager:shouldProceedAfterError:]
 * messages.<br />
 * Will not copy to a destination which already exists.
 */
- (BOOL) copyPath: (NSString*)source
	   toPath: (NSString*)destination
	  handler: (id)handler
{
  NSDictionary	*attrs;
  NSString	*fileType;

  if ([self fileExistsAtPath: destination] == YES)
    {
      return NO;
    }
  attrs = [self fileAttributesAtPath: source traverseLink: NO];
  if (attrs == nil)
    {
      return NO;
    }
  fileType = [attrs fileType];

  /* Don't attempt to retain ownership of copy ... we want the copy
   * to be owned by the current user.
   * However, the new copy should have the creation/modification date
   * of the original (unlike Posix semantics).
   */
  attrs = AUTORELEASE([attrs mutableCopy]);
  [(NSMutableDictionary*)attrs removeObjectForKey: NSFileOwnerAccountID];
  [(NSMutableDictionary*)attrs removeObjectForKey: NSFileGroupOwnerAccountID];
  [(NSMutableDictionary*)attrs removeObjectForKey: NSFileGroupOwnerAccountName];
  [(NSMutableDictionary*)attrs setObject: NSUserName()
                                  forKey: NSFileOwnerAccountName];

  if ([fileType isEqualToString: NSFileTypeDirectory] == YES)
    {

      /* If destination directory is a descendant of source directory copying
       * isn't possible.
       */
      if ([[destination stringByAppendingString: @"/"]
	hasPrefix: [source stringByAppendingString: @"/"]])
	{
	  ASSIGN(_lastError,
            @"Could not copy - destination is a descendant of source");
	  return NO;
	}

      [self _sendToHandler: handler willProcessPath: destination];

      if ([self createDirectoryAtPath: destination attributes: attrs] == NO)
	{
          return [self _proceedAccordingToHandler: handler
					 forError: _lastError
					   inPath: destination
					 fromPath: source
					   toPath: destination];
	}

      if ([self _copyPath: source toPath: destination handler: handler] == NO)
	{
	  return NO;
	}
    }
  else if ([fileType isEqualToString: NSFileTypeSymbolicLink] == YES)
    {
      NSString	*path;
      BOOL	result;

      [self _sendToHandler: handler willProcessPath: source];

      path = [self pathContentOfSymbolicLinkAtPath: source];
      result = [self createSymbolicLinkAtPath: destination pathContent: path];
      if (result == NO)
	{
          result = [self _proceedAccordingToHandler: handler
					   forError: @"cannot link to file"
					     inPath: source
					   fromPath: source
					     toPath: destination];

	  if (result == NO)
	    {
	      return NO;
	    }
	}
    }
  else
    {
      [self _sendToHandler: handler willProcessPath: source];

      if ([self _copyFile: source toFile: destination handler: handler] == NO)
	{
	  return NO;
	}
    }
  [self changeFileAttributes: attrs atPath: destination];
  return YES;
}

- (BOOL) copyItemAtPath: (NSString*)src
		 toPath: (NSString*)dst
		  error: (NSError**)error
{
  BOOL  result;

  DESTROY(_lastError);
  result = [self copyPath: src toPath: dst handler: nil];

  if (error != NULL)
    {
      if (NO == result)
	{
	  *error = [self _errorFrom: src to: dst];
	}
    }

  return result;
}

- (BOOL) copyItemAtURL: (NSURL*)src
		 toURL: (NSURL*)dst
		 error: (NSError**)error
{
  return [self copyItemAtPath: [src path] toPath: [dst path] error: error];
}

/**
 * Moves the file or directory at source to destination, using a
 * handler object which should respond to
 * [NSObject(NSFileManagerHandler)-fileManager:willProcessPath:] and
 * [NSObject(NSFileManagerHandler)-fileManager:shouldProceedAfterError:]
 * messages.
 * Will not move to a destination which already exists.<br />
 */
- (BOOL) movePath: (NSString*)source
	   toPath: (NSString*)destination
	  handler: (id)handler
{
  BOOL		sourceIsDir;
  BOOL		fileExists;
  NSString	*destinationParent;
  unsigned int	sourceDevice;
  unsigned int	destinationDevice;
  const _CHAR	*sourcePath;
  const _CHAR	*destPath;

  sourcePath = [self fileSystemRepresentationWithPath: source];
  destPath = [self fileSystemRepresentationWithPath: destination];

  if ([self fileExistsAtPath: destination] == YES)
    {
      return NO;
    }
  fileExists = [self fileExistsAtPath: source isDirectory: &sourceIsDir];
  if (!fileExists)
    {
      return NO;
    }

  /* Check to see if the source and destination's parent are on the same
     physical device so we can perform a rename syscall directly. */
  sourceDevice = [[self fileSystemAttributesAtPath: source] fileSystemNumber];
  destinationParent = [destination stringByDeletingLastPathComponent];
  if ([destinationParent isEqual: @""])
    destinationParent = @".";
  destinationDevice
    = [[self fileSystemAttributesAtPath: destinationParent] fileSystemNumber];

  if (sourceDevice != destinationDevice)
    {
      /* If destination directory is a descendant of source directory moving
	  isn't possible. */
      if (sourceIsDir && [[destination stringByAppendingString: @"/"]
	hasPrefix: [source stringByAppendingString: @"/"]])
	{
	  ASSIGN(_lastError, @"Could not move - destination is a descendant of source");
	  return NO;
	}

      if ([self copyPath: source toPath: destination handler: handler])
	{
	  NSDictionary	*attributes;

	  attributes = [self fileAttributesAtPath: source
				     traverseLink: NO];
	  [self changeFileAttributes: attributes atPath: destination];
	  return [self removeFileAtPath: source handler: handler];
	}
      else
	{
	  return NO;
	}
    }
  else
    {
      /* source and destination are on the same device so we can simply
	 invoke rename on source. */
      [self _sendToHandler: handler willProcessPath: source];

      if (_RENAME (sourcePath, destPath) == -1)
	{
          return [self _proceedAccordingToHandler: handler
					 forError: @"cannot move file"
					   inPath: source
					 fromPath: source
					   toPath: destination];
	}
      return YES;
    }

  return NO;
}

- (BOOL) moveItemAtPath: (NSString*)src
		 toPath: (NSString*)dst
		  error: (NSError**)error
{
  BOOL  result;

  DESTROY(_lastError);
  result = [self movePath: src toPath: dst handler: nil];
  
  if (error != NULL)
    {
      if (NO == result)
	{
	  *error = [self _errorFrom: src to: dst];
	}
    }

  return result;
}

- (BOOL) moveItemAtURL: (NSURL*)src
		 toURL: (NSURL*)dst
		 error: (NSError**)error
{
  return [self moveItemAtPath: [src path] toPath: [dst path] error: error];
}

/**
 * <p>Links the file or directory at source to destination, using a
 * handler object which should respond to
 * [NSObject(NSFileManagerHandler)-fileManager:willProcessPath:] and
 * [NSObject(NSFileManagerHandler)-fileManager:shouldProceedAfterError:]
 * messages.
 * </p>
 * <p>If the destination is a directory, the source path is linked
 * into that directory, otherwise the destination must not exist,
 * but its parent directory must exist and the source will be linked
 * into the parent as the name specified by the destination.
 * </p>
 * <p>If the source is a symbolic link, it is copied to the destination.<br />
 * If the source is a directory, it is copied to the destination and its
 * contents are linked into the new directory.<br />
 * Otherwise, a hard link is made from the destination to the source.
 * </p>
 */
- (BOOL) linkPath: (NSString*)source
	   toPath: (NSString*)destination
	  handler: (id)handler
{
#ifdef HAVE_LINK
  NSDictionary	*attrs;
  NSString	*fileType;
  BOOL		isDir;

  if ([self fileExistsAtPath: destination isDirectory: &isDir] == YES
    && isDir == YES)
    {
      destination = [destination stringByAppendingPathComponent:
	[source lastPathComponent]];
    }

  attrs = [self fileAttributesAtPath: source traverseLink: NO];
  if (attrs == nil)
    {
      return NO;
    }

  [self _sendToHandler: handler willProcessPath: destination];

  fileType = [attrs fileType];
  if ([fileType isEqualToString: NSFileTypeDirectory] == YES)
    {
      /* If destination directory is a descendant of source directory linking
	  isn't possible because of recursion. */
      if ([[destination stringByAppendingString: @"/"]
	hasPrefix: [source stringByAppendingString: @"/"]])
	{
	  ASSIGN(_lastError, @"Could not link - destination is a descendant of source");
	  return NO;
	}

      if ([self createDirectoryAtPath: destination attributes: attrs] == NO)
	{
          return [self _proceedAccordingToHandler: handler
					 forError: _lastError
					   inPath: destination
					 fromPath: source
					   toPath: destination];
	}

      if ([self _linkPath: source toPath: destination handler: handler] == NO)
	{
	  return NO;
	}
    }
  else if ([fileType isEqual: NSFileTypeSymbolicLink])
    {
      NSString	*path;

      path = [self pathContentOfSymbolicLinkAtPath: source];
      if ([self createSymbolicLinkAtPath: destination
			     pathContent: path] == NO)
	{
	  if ([self _proceedAccordingToHandler: handler
				      forError: @"cannot create symbolic link"
					inPath: source
				      fromPath: source
					toPath: destination] == NO)
	    {
	      return NO;
	    }
	}
    }
  else
    {
      if (link([self fileSystemRepresentationWithPath: source],
	[self fileSystemRepresentationWithPath: destination]) < 0)
	{
	  if ([self _proceedAccordingToHandler: handler
				      forError: @"cannot create hard link"
					inPath: source
				      fromPath: source
					toPath: destination] == NO)
	    {
	      return NO;
	    }
	}
    }
  [self changeFileAttributes: attrs atPath: destination];
  return YES;
#else
  ASSIGN(_lastError, @"Links not supported on this platform");
  return NO;
#endif
}

- (BOOL) removeFileAtPath: (NSString*)path
		  handler: handler
{
  BOOL		is_dir;
  const _CHAR	*lpath;

  if ([path isEqualToString: @"."] || [path isEqualToString: @".."])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Attempt to remove illegal path"];
    }

  [self _sendToHandler: handler willProcessPath: path];

  lpath = [self fileSystemRepresentationWithPath: path];
  if (lpath == 0 || *lpath == 0)
    {
      ASSIGN(_lastError, @"Could not remove - no path");
      return NO;
    }
  else
    {
#if defined(_WIN32)
      DWORD res;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}
      if (res & FILE_ATTRIBUTE_DIRECTORY)
	{
	  is_dir = YES;
	}
      else
	{
	  is_dir = NO;
	}
#else
      struct _STATB statbuf;

      if (lstat(lpath, &statbuf) != 0)
	{
	  return NO;
	}
      is_dir = ((statbuf.st_mode & S_IFMT) == S_IFDIR);
#endif /* _WIN32 */
    }

  if (!is_dir)
    {
#if defined(_WIN32)
      if (DeleteFileW(lpath) == FALSE)
#else
      if (unlink(lpath) < 0)
#endif
	{
	  NSString	*message = [[NSError _last] localizedDescription];

	  return [self _proceedAccordingToHandler: handler
					 forError: message
					   inPath: path];
	}
      else
	{
	  return YES;
	}
    }
  else
    {
      NSArray   *contents = [self directoryContentsAtPath: path];
      unsigned	count = [contents count];
      unsigned	i;

      for (i = 0; i < count; i++)
	{
	  NSString		*item;
	  NSString		*next;
	  BOOL			result;
	  ENTER_POOL

	  item = [contents objectAtIndex: i];
	  next = [path stringByAppendingPathComponent: item];
	  result = [self removeFileAtPath: next handler: handler];
	  LEAVE_POOL
	  if (result == NO)
	    {
	      return NO;
	    }
	}

      if (_RMDIR([self fileSystemRepresentationWithPath: path]) < 0)
	{
	  NSString	*message = [[NSError _last] localizedDescription];

	  return [self _proceedAccordingToHandler: handler
					 forError: message
					   inPath: path];
	}
      else
	{
	  return YES;
	}
    }
}

- (BOOL) removeItemAtPath: (NSString*)path
		    error: (NSError**)error
{
  BOOL  result;

  DESTROY(_lastError);
  result = [self removeFileAtPath: path handler: nil];

  if (error != NULL)
    {
      if (NO == result)
	{
	  *error = [self _errorFrom: path to: nil];
	}
    }

  return result;
}

- (BOOL) removeItemAtURL: (NSURL*)url
		   error: (NSError**)error
{
  return [self removeItemAtPath: [url path] error: error];
}

- (BOOL) createSymbolicLinkAtPath: (NSString*)path
              withDestinationPath: (NSString*)destPath
                            error: (NSError**)error
{
  BOOL  result;

  DESTROY(_lastError);
  result = [self createSymbolicLinkAtPath: path pathContent: destPath];

  if (error != NULL)
    {
      if (NO == result)
	{
	  *error = [self _errorFrom: path to: destPath];
	}
    }

  return result;
}

- (BOOL) fileExistsAtPath: (NSString*)path
{
  return [self fileExistsAtPath: path isDirectory: 0];
}

- (BOOL) fileExistsAtPath: (NSString*)path isDirectory: (BOOL*)isDirectory
{
  const _CHAR *lpath = [self fileSystemRepresentationWithPath: path];

  if (isDirectory != 0)
    {
      *isDirectory = NO;
    }

  if (lpath == 0 || *lpath == _NUL)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if defined(_WIN32)
    {
      DWORD res;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}
      if (isDirectory != 0)
	{
	  if (res & FILE_ATTRIBUTE_DIRECTORY)
	    {
	      *isDirectory = YES;
	    }
	}
      return YES;
    }
#else
    {
      struct _STATB statbuf;

      if (_STAT(lpath, &statbuf) != 0)
	{
#ifdef __ANDROID__
          /* Android: try using asset manager if path is in
           * main bundle resources
           */
          AAsset *asset = [NSBundle assetForPath: path];
          if (asset)
	    {
	      AAsset_close(asset);
	      return YES;
	    }
          
          AAssetDir *assetDir = [NSBundle assetDirForPath: path];
          if (assetDir)
	    {
	      AAssetDir_close(assetDir);
	      if (isDirectory)
		{
		  *isDirectory = YES;
		}
	      return YES;
	    }
#endif
          
	  return NO;
	}

      if (isDirectory)
	{
	  if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
	    {
	      *isDirectory = YES;
	    }
	}

      return YES;
    }
#endif /* _WIN32 */
}

/**
 * Returns YES if a file (or directory etc) exists at the specified path
 * and is readable.
 */
- (BOOL) isReadableFileAtPath: (NSString*)path
{
  const _CHAR* lpath = [self fileSystemRepresentationWithPath: path];

  if (lpath == 0 || *lpath == _NUL)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if defined(_WIN32)
    {
      DWORD res;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}
      return YES;
    }
#else
    {
      if (access(lpath, R_OK) == 0)
	{
	  return YES;
	}

#ifdef __ANDROID__
        /* Android: try using asset manager if path is in
         * main bundle resources
         */
        AAsset *asset = [NSBundle assetForPath: path];
        if (asset)
	  {
	    AAsset_close(asset);
	    return YES;
	  }

        AAssetDir *assetDir = [NSBundle assetDirForPath: path];
        if (assetDir)
	  {
	    AAssetDir_close(assetDir);
	    return YES;
	  }
#endif

      return NO;
    }
#endif
}

/**
 * Returns YES if a file (or directory etc) exists at the specified path
 * and is writable.
 */
- (BOOL) isWritableFileAtPath: (NSString*)path
{
  const _CHAR* lpath = [self fileSystemRepresentationWithPath: path];

  if (lpath == 0 || *lpath == _NUL)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if defined(_WIN32)
    {
      DWORD res;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}
      if (res & FILE_ATTRIBUTE_READONLY)
	{
	  return NO;
	}
      return YES;
    }
#else
    {
      if (access(lpath, W_OK) == 0)
	{
	  return YES;
	}
      return NO;
    }
#endif
}

/**
 * Returns YES if a file (or directory etc) exists at the specified path
 * and is executable (if a directory is executable, you can access its
 * contents).
 */
- (BOOL) isExecutableFileAtPath: (NSString*)path
{
  const _CHAR* lpath = [self fileSystemRepresentationWithPath: path];

  if (lpath == 0 || *lpath == _NUL)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if defined(_WIN32)
    {
      DWORD res;
      NSString  *ext;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}

      ext = [[path pathExtension] uppercaseString];
      if ([ext length] > 0)
        {
          static NSSet  *executable = nil;

          if (nil == executable)
            {
              executable = [[NSTask executableExtensions] copy];
            }
          if (nil != [executable member: ext])
            {
              return YES;
            }
	}
      /* FIXME: On unix, directory accessible == executable, so we simulate that
      here for Windows. Is there a better check for directory access? */
      if (res & FILE_ATTRIBUTE_DIRECTORY)
	{
	  return YES;
	}
      return NO;
    }
#else
    {
      if (access(lpath, X_OK) == 0)
	{
	  return YES;
	}
      return NO;
    }
#endif
}

/**
 * Returns YES if a file (or directory etc) exists at the specified path
 * and is deletable.
 */
- (BOOL) isDeletableFileAtPath: (NSString*)path
{
  const _CHAR* lpath = [self fileSystemRepresentationWithPath: path];

  if (lpath == 0 || *lpath == _NUL)
    {
      ASSIGN(_lastError, @"no path given");
      return NO;
    }

#if defined(_WIN32)
      // TODO - handle directories
    {
      DWORD res;

      res = GetFileAttributesW(lpath);

      if (res == WIN32ERR)
	{
	  return NO;
	}
      return (res & FILE_ATTRIBUTE_READONLY) ? NO : YES;
    }
#else
    {
      // TODO - handle directories
      path = [path stringByDeletingLastPathComponent];
      if ([path length] == 0)
	{
	  path = @".";
	}
      lpath = [self fileSystemRepresentationWithPath: path];

      if (access(lpath, X_OK | W_OK) == 0)
	{
	  return YES;
	}
      return NO;
    }
#endif
}


/**
 * If a file (or directory etc) exists at the specified path, and can be
 * queried for its attributes, this method returns a dictionary containing
 * the various attributes of that file.  Otherwise nil is returned.<br />
 * If the flag is NO and the file is a symbolic link, the attributes of
 * the link itself (rather than the file it points to) are returned.<br />
 * <p>
 *   The dictionary keys for attributes are -
 * </p>
 * <deflist>
 *   <term><code>NSFileAppendOnly</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileCreationDate</code></term>
 *   <desc>NSDate when the file was created (if supported)</desc>
 *   <term><code>NSFileDeviceIdentifier</code></term>
 *   <desc>NSNumber (identifies the device on which the file is stored)</desc>
 *   <term><code>NSFileExtensionHidden</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileGroupOwnerAccountName</code></term>
 *   <desc>NSString name of the file group</desc>
 *   <term><code>NSFileGroupOwnerAccountID</code></term>
 *   <desc>NSNumber ID of the file group</desc>
 *   <term><code>NSFileHFSCreatorCode</code></term>
 *   <desc>NSNumber not used</desc>
 *   <term><code>NSFileHFSTypeCode</code></term>
 *   <desc>NSNumber not used</desc>
 *   <term><code>NSFileImmutable</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileModificationDate</code></term>
 *   <desc>NSDate when the file was last modified</desc>
 *   <term><code>NSFileOwnerAccountName</code></term>
 *   <desc>NSString name of the file owner</desc>
 *   <term><code>NSFileOwnerAccountID</code></term>
 *   <desc>NSNumber ID of the file owner</desc>
 *   <term><code>NSFilePosixPermissions</code></term>
 *   <desc>NSNumber posix access permissions mask</desc>
 *   <term><code>NSFileReferenceCount</code></term>
 *   <desc>NSNumber number of links to this file</desc>
 *   <term><code>NSFileSize</code></term>
 *   <desc>NSNumber size of the file in bytes</desc>
 *   <term><code>NSFileSystemFileNumber</code></term>
 *   <desc>NSNumber the identifier for the file on the filesystem</desc>
 *   <term><code>NSFileSystemNumber</code></term>
 *   <desc>NSNumber the filesystem on which the file is stored</desc>
 *   <term><code>NSFileType</code></term>
 *   <desc>NSString the type of file</desc>
 * </deflist>
 * <p>
 *   The [NSDictionary] class also has a set of convenience accessor methods
 *   which enable you to get at file attribute information more efficiently
 *   than using the keys above to extract it.  You should generally
 *   use the accessor methods where they are available.
 * </p>
 * <list>
 *   <item>[NSDictionary(NSFileAttributes)-fileCreationDate]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileExtensionHidden]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileHFSCreatorCode]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileHFSTypeCode]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileIsAppendOnly]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileIsImmutable]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSize]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileType]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileOwnerAccountName]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileOwnerAccountID]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileGroupOwnerAccountName]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileGroupOwnerAccountID]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileModificationDate]</item>
 *   <item>[NSDictionary(NSFileAttributes)-filePosixPermissions]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSystemNumber]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSystemFileNumber]</item>
 * </list>
 */
- (NSDictionary*) fileAttributesAtPath: (NSString*)path traverseLink: (BOOL)flag
{
  NSDictionary	*d;

  d = [GSAttrDictionaryClass attributesAt: path traverseLink: flag];
  return d;
}

/**
 * If a file (or directory etc) exists at the specified path, and can be
 * queried for its attributes, this method returns a dictionary containing
 * the various attributes of that file.  Otherwise nil is returned.<br />
 * If an error occurs, error describes the problem.
 * Pass NULL if you do not want error information.
 * <p>
 *   The dictionary keys for attributes are -
 * </p>
 * <deflist>
 *   <term><code>NSFileAppendOnly</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileCreationDate</code></term>
 *   <desc>NSDate when the file was created (if supported)</desc>
 *   <term><code>NSFileDeviceIdentifier</code></term>
 *   <desc>NSNumber (identifies the device on which the file is stored)</desc>
 *   <term><code>NSFileExtensionHidden</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileGroupOwnerAccountName</code></term>
 *   <desc>NSString name of the file group</desc>
 *   <term><code>NSFileGroupOwnerAccountID</code></term>
 *   <desc>NSNumber ID of the file group</desc>
 *   <term><code>NSFileHFSCreatorCode</code></term>
 *   <desc>NSNumber not used</desc>
 *   <term><code>NSFileHFSTypeCode</code></term>
 *   <desc>NSNumber not used</desc>
 *   <term><code>NSFileImmutable</code></term>
 *   <desc>NSNumber ... boolean</desc>
 *   <term><code>NSFileModificationDate</code></term>
 *   <desc>NSDate when the file was last modified</desc>
 *   <term><code>NSFileOwnerAccountName</code></term>
 *   <desc>NSString name of the file owner</desc>
 *   <term><code>NSFileOwnerAccountID</code></term>
 *   <desc>NSNumber ID of the file owner</desc>
 *   <term><code>NSFilePosixPermissions</code></term>
 *   <desc>NSNumber posix access permissions mask</desc>
 *   <term><code>NSFileReferenceCount</code></term>
 *   <desc>NSNumber number of links to this file</desc>
 *   <term><code>NSFileSize</code></term>
 *   <desc>NSNumber size of the file in bytes</desc>
 *   <term><code>NSFileSystemFileNumber</code></term>
 *   <desc>NSNumber the identifier for the file on the filesystem</desc>
 *   <term><code>NSFileSystemNumber</code></term>
 *   <desc>NSNumber the filesystem on which the file is stored</desc>
 *   <term><code>NSFileType</code></term>
 *   <desc>NSString the type of file</desc>
 * </deflist>
 * <p>
 *   The [NSDictionary] class also has a set of convenience accessor methods
 *   which enable you to get at file attribute information more efficiently
 *   than using the keys above to extract it.  You should generally
 *   use the accessor methods where they are available.
 * </p>
 * <list>
 *   <item>[NSDictionary(NSFileAttributes)-fileCreationDate]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileExtensionHidden]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileHFSCreatorCode]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileHFSTypeCode]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileIsAppendOnly]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileIsImmutable]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSize]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileType]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileOwnerAccountName]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileOwnerAccountID]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileGroupOwnerAccountName]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileGroupOwnerAccountID]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileModificationDate]</item>
 *   <item>[NSDictionary(NSFileAttributes)-filePosixPermissions]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSystemNumber]</item>
 *   <item>[NSDictionary(NSFileAttributes)-fileSystemFileNumber]</item>
 * </list>
 */
- (NSDictionary*) attributesOfItemAtPath: (NSString*)path
				   error: (NSError**)error
{
  NSDictionary	*d;

  DESTROY(_lastError);
  d = [GSAttrDictionaryClass attributesAt: path traverseLink: NO];
  
  if (error != NULL)
    {
      if (nil == d)
	{
	  *error = [self _errorFrom: path to: nil];
	}
    }
  
  return d;
}

- (NSDictionary*) attributesOfFileSystemForPath: (NSString*)path
		                          error: (NSError**)error
{
#if defined(_WIN32)
  unsigned long long totalsize, freesize;
  id  values[5];
  id	keys[5] = {
    NSFileSystemSize,
    NSFileSystemFreeSize,
    NSFileSystemNodes,
    NSFileSystemFreeNodes,
    NSFileSystemNumber
  };
  DWORD SectorsPerCluster, BytesPerSector, NumberFreeClusters;
  DWORD TotalNumberClusters;
  DWORD volumeSerialNumber = 0;
  const _CHAR *lpath = [self fileSystemRepresentationWithPath: path];
  _CHAR volumePathName[128];

  if (!GetVolumePathNameW(lpath, volumePathName, 128))
    {
      if (error != NULL)
	{
	  *error = [NSError _last];
	}
      return nil;
    }
  GetVolumeInformationW(volumePathName, NULL, 0, &volumeSerialNumber,
    NULL, NULL, NULL, 0);

  if (!GetDiskFreeSpaceW(volumePathName, &SectorsPerCluster,
    &BytesPerSector, &NumberFreeClusters, &TotalNumberClusters))
    {
      if (error != NULL)
	{
	  *error = [NSError _last];
	}
      return nil;
    }

  totalsize = (unsigned long long)TotalNumberClusters
    * (unsigned long long)SectorsPerCluster
    * (unsigned long long)BytesPerSector;
  freesize = (unsigned long long)NumberFreeClusters
    * (unsigned long long)SectorsPerCluster
    * (unsigned long long)BytesPerSector;

  values[0] = [NSNumber numberWithUnsignedLongLong: totalsize];
  values[1] = [NSNumber numberWithUnsignedLongLong: freesize];
  values[2] = [NSNumber numberWithLong: LONG_MAX];
  values[3] = [NSNumber numberWithLong: LONG_MAX];
  values[4] = [NSNumber numberWithUnsignedInt: volumeSerialNumber];

  return [NSDictionary dictionaryWithObjects: values forKeys: keys count: 5];

#else
#if defined(HAVE_SYS_VFS_H) || defined(HAVE_SYS_STATFS_H) \
  || defined(HAVE_SYS_MOUNT_H)
  struct _STATB statbuf;
#ifdef HAVE_STATVFS
  struct statvfs statfsbuf;
#else
  struct statfs statfsbuf;
#endif
  unsigned long long totalsize, freesize;
  unsigned long blocksize;
  const char* lpath = [self fileSystemRepresentationWithPath: path];

  id  values[5];
  id	keys[5] = {
    NSFileSystemSize,
    NSFileSystemFreeSize,
    NSFileSystemNodes,
    NSFileSystemFreeNodes,
    NSFileSystemNumber
  };

  if (_STAT(lpath, &statbuf) != 0)
    {
      if (error != NULL)
	{
	  *error = [NSError _last];
	}
      NSDebugMLLog(@"NSFileManager", @"stat failed for '%s' ... %@",
        lpath, [NSError _last]);
      return nil;
    }
#ifdef HAVE_STATVFS
  if (statvfs(lpath, &statfsbuf) != 0)
    {
      if (error != NULL)
	{
	  *error = [NSError _last];
	}
      NSDebugMLLog(@"NSFileManager", @"statvfs failed for '%s' ... %@",
        lpath, [NSError _last]);
      return nil;
    }
  blocksize = statfsbuf.f_frsize;
#else
  if (statfs(lpath, &statfsbuf) != 0)
    {
      if (error != NULL)
	{
	  *error = [NSError _last];
	}
      NSDebugMLLog(@"NSFileManager", @"statfs failed for '%s' ... %@",
        lpath, [NSError _last]);
      return nil;
    }
  blocksize = statfsbuf.f_bsize;
#endif

  totalsize = (unsigned long long) blocksize
    * (unsigned long long) statfsbuf.f_blocks;
  freesize = (unsigned long long) blocksize
    * (unsigned long long) statfsbuf.f_bavail;

  values[0] = [NSNumber numberWithUnsignedLongLong: totalsize];
  values[1] = [NSNumber numberWithUnsignedLongLong: freesize];
  values[2] = [NSNumber numberWithLong: statfsbuf.f_files];
  values[3] = [NSNumber numberWithLong: statfsbuf.f_ffree];
  values[4] = [NSNumber numberWithUnsignedLong: statbuf.st_dev];

  return [NSDictionary dictionaryWithObjects: values forKeys: keys count: 5];
#else
  GSOnceMLog(@"NSFileManager", @"no support for filesystem attributes");
  ASSIGN(_lastError, @"no support for filesystem attributes");
  return nil;
#endif
#endif /* _WIN32 */
}

/**
 * Returns a dictionary containing the filesystem attributes for the
 * specified path (or nil if the path is not valid).<br />
 * <deflist>
 *   <term><code>NSFileSystemSize</code></term>
 *   <desc>NSNumber the size of the filesystem in bytes</desc>
 *   <term><code>NSFileSystemFreeSize</code></term>
 *   <desc>NSNumber the amount of unused space on the filesystem in bytes</desc>
 *   <term><code>NSFileSystemNodes</code></term>
 *   <desc>NSNumber the number of nodes in use to store files</desc>
 *   <term><code>NSFileSystemFreeNodes</code></term>
 *   <desc>NSNumber the number of nodes available to create files</desc>
 *   <term><code>NSFileSystemNumber</code></term>
 *   <desc>NSNumber the identifying number for the filesystem</desc>
 * </deflist>
 */
- (NSDictionary*) fileSystemAttributesAtPath: (NSString*)path
{
  return [self attributesOfFileSystemForPath: path
				       error: NULL];
}

/**
 * Returns an array of the contents of the specified directory.<br />
 * The listing does <strong>not</strong> recursively list subdirectories.<br />
 * The special files '.' and '..' are not listed.<br />
 * Indicates an error by returning nil (eg. if path is not a directory or
 * it can't be read for some reason).
 */
- (NSArray*) directoryContentsAtPath: (NSString*)path
{
  NSDirectoryEnumerator	*direnum;
  NSMutableArray	*content;
  BOOL			is_dir;

  /*
   * See if this is a directory (don't follow links).
   */
  if ([self fileExistsAtPath: path isDirectory: &is_dir] == NO || is_dir == NO)
    {
      return nil;
    }
  content = [NSMutableArray arrayWithCapacity: 128];
  /* We initialize the directory enumerator with justContents == YES,
     which tells the NSDirectoryEnumerator code that we only enumerate
     the contents non-recursively once, and exit.  NSDirectoryEnumerator
     can perform some optimisations using this assumption. */
  direnum = [[NSDirectoryEnumerator alloc] initWithDirectoryPath: path
				       recurseIntoSubdirectories: NO
						  followSymlinks: NO
						    justContents: YES
							     for: self];
  if (nil != direnum)
    {
      IMP	nxtImp;
      IMP	addImp;

      nxtImp = [direnum methodForSelector: @selector(nextObject)];
      addImp = [content methodForSelector: @selector(addObject:)];

      while ((path = (*nxtImp)(direnum, @selector(nextObject))) != nil)
	{
	  (*addImp)(content, @selector(addObject:), path);
	}
      RELEASE(direnum);
    }
  return GS_IMMUTABLE(content);
}

/**
 * Returns the name of the file or directory at path.  Converts it into
 * a format for display to an end user.  This may render it unusable as
 * part of a file/path name.<br />
 * For instance, if a user has elected not to see file extensions, this
 * method may return filenames with the extension removed.<br />
 * The default operation is to return the result of calling
 * [NSString-lastPathComponent] on the path.
 */
- (NSString*) displayNameAtPath: (NSString*)path
{
  return [path lastPathComponent];
}

- (NSDirectoryEnumerator*) enumeratorAtPath: (NSString*)path
{
  return AUTORELEASE([[NSDirectoryEnumerator alloc]
		       initWithDirectoryPath: path
		       recurseIntoSubdirectories: YES
		       followSymlinks: NO
		       justContents: NO
		       for: self]);
}

/**
 * Returns an array containing the (relative) paths of all the items
 * in the directory at path.<br />
 * The listing follows all subdirectories, so it can produce a very
 * large array ... use with care.
 */
- (NSArray*) subpathsAtPath: (NSString*)path
{
  NSDirectoryEnumerator	*direnum;
  NSMutableArray	*content;
  BOOL			isDir;

  if (![self fileExistsAtPath: path isDirectory: &isDir] || !isDir)
    {
      return nil;
    }
  content = [NSMutableArray arrayWithCapacity: 128];
  direnum = [[NSDirectoryEnumerator alloc] initWithDirectoryPath: path
				       recurseIntoSubdirectories: YES
						  followSymlinks: NO
						    justContents: NO
							     for: self];
  if (nil != direnum)
    {
      IMP	nxtImp;
      IMP	addImp;

      nxtImp = [direnum methodForSelector: @selector(nextObject)];
      addImp = [content methodForSelector: @selector(addObject:)];

      while ((path = (*nxtImp)(direnum, @selector(nextObject))) != nil)
	{
	  (*addImp)(content, @selector(addObject:), path);
	}
      RELEASE(direnum);
    }
  return GS_IMMUTABLE(content);
}

/**
 * Creates a symbolic link at path which links to the location
 * specified by otherPath.
 */
- (BOOL) createSymbolicLinkAtPath: (NSString*)path
		      pathContent: (NSString*)otherPath
{
#ifdef HAVE_SYMLINK
  const char* newpath = [self fileSystemRepresentationWithPath: path];
  const char* oldpath = [self fileSystemRepresentationWithPath: otherPath];

  return (symlink(oldpath, newpath) == 0);
#else
  ASSIGN(_lastError, @"symbolic links not supported on this system");
  return NO;
#endif
}

/**
 * Returns the name of the file or directory that the symbolic link
 * at path points to.
 */
- (NSString*) pathContentOfSymbolicLinkAtPath: (NSString*)path
{
#ifdef HAVE_READLINK
  char  buf[PATH_MAX];
  const char* lpath = [self fileSystemRepresentationWithPath: path];
  int   llen = readlink(lpath, buf, PATH_MAX-1);

  if (llen > 0)
    {
      return [self stringWithFileSystemRepresentation: buf length: llen];
    }
  else
    {
      return nil;
    }
#else
  ASSIGN(_lastError, @"symbolic links not supported on this system");
  return nil;
#endif
}

#if	defined(_WIN32)
- (const GSNativeChar*) fileSystemRepresentationWithPath: (NSString*)path
{
  if (path != nil && [path rangeOfString: @"/"].length > 0)
    {
      path = [path stringByReplacingString: @"/" withString: @"\\"];
    }
  return
    (const GSNativeChar*)[path cStringUsingEncoding: NSUnicodeStringEncoding];
}
- (NSString*) stringWithFileSystemRepresentation: (const GSNativeChar*)string
					  length: (NSUInteger)len
{
  return [NSString stringWithCharacters: string length: len];
}
#else
- (const GSNativeChar*) fileSystemRepresentationWithPath: (NSString*)path
{
  return
    (const GSNativeChar*)[path cStringUsingEncoding: defaultEncoding];
}
- (NSString*) stringWithFileSystemRepresentation: (const GSNativeChar*)string
					  length: (NSUInteger)len
{
  return AUTORELEASE([[NSString allocWithZone: NSDefaultMallocZone()]
    initWithBytes: string length: len encoding: defaultEncoding]);
}
#endif

@end /* NSFileManager */

/* A directory to enumerate.  We keep a stack of the directories we
   still have to enumerate.  We start by putting the top-level
   directory into the stack, then we start reading files from it
   (using readdir).  If we find a file which is actually a directory,
   and if we have to recurse into it, we create a new
   GSEnumeratedDirectory struct for the subdirectory, open its DIR
   *pointer for reading, and put it on top of the stack, so next time
   -nextObject is called, it will read from that directory instead of
   the top level one.  Once all the subdirectory is read, it is
   removed from the stack, so the top of the stack if the top
   directory again, and enumeration continues in there.  */
typedef	struct	_GSEnumeratedDirectory {
  NSString *path;
  _DIR *pointer;
#ifdef __ANDROID__
  AAssetDir *assetDir;
#endif
} GSEnumeratedDirectory;


static inline void gsedRelease(GSEnumeratedDirectory X)
{
  DESTROY(X.path);
  _CLOSEDIR(X.pointer);
#ifdef __ANDROID__
  if (X.assetDir)
    {
      AAssetDir_close(X.assetDir);
    }
#endif
}

#define GSI_ARRAY_TYPES	0
#define GSI_ARRAY_TYPE	GSEnumeratedDirectory
#define GSI_ARRAY_RELEASE(A, X)   gsedRelease(X.ext)
#define GSI_ARRAY_RETAIN(A, X)

#include "GNUstepBase/GSIArray.h"


@implementation NSDirectoryEnumerator
/*
 * The Objective-C interface hides a traditional C implementation.
 * This was the only way I could get near the speed of standard unix
 * tools for big directories.
 */

+ (void) initialize
{
  if (self == [NSDirectoryEnumerator class])
    {
    }
}

- (void) _setSkipHidden: (BOOL)flag
{
  _flags.skipHidden = flag;
}

- (void) _setErrorHandler: (GSDirEnumErrorHandler) handler
{
  _errorHandler = handler;
}

- (id) initWithDirectoryPath: (NSString*)path
   recurseIntoSubdirectories: (BOOL)recurse
	      followSymlinks: (BOOL)follow
		justContents: (BOOL)justContents
                  skipHidden: (BOOL)skipHidden
                errorHandler: (GSDirEnumErrorHandler) handler
			 for: (NSFileManager*)mgr
{
  if (nil != (self = [super init]))
    {
    //TODO: the justContents flag is currently basically useless and should be
    //      removed
      _DIR		*dir_pointer;
      const _CHAR	*localPath;

      _mgr = RETAIN(mgr);
      _stack = NSZoneMalloc([self zone], sizeof(GSIArray_t));
      GSIArrayInitWithZoneAndCapacity(_stack, [self zone], 64);

      _flags.isRecursive = recurse;
      _flags.isFollowing = follow;
      _flags.justContents = justContents;
      _flags.skipHidden = skipHidden;
      _errorHandler = handler;
      
      _topPath = [[NSString alloc] initWithString: path];

      localPath = [_mgr fileSystemRepresentationWithPath: path];
      dir_pointer = _OPENDIR(localPath);
      
#ifdef __ANDROID__
      AAssetDir *assetDir = NULL;
      if (!dir_pointer)
	{
	  /* Android: try using asset manager if path is in
	   * main bundle resources
	   */
	  assetDir = [NSBundle assetDirForPath: path];
	}
      
      if (dir_pointer || assetDir)
#else 
      if (dir_pointer)
#endif
        {
          GSIArrayItem item;

          item.ext.path = @"";
          item.ext.pointer = dir_pointer;
#ifdef __ANDROID__
          item.ext.assetDir = assetDir;
#endif

          GSIArrayAddItem(_stack, item);
        }
      else
        {
          NSDebugLog(@"Failed to recurse into directory '%@' - %@", path,
            [NSError _last]);
        }
    }
  return self;
}

/**
 *  Initialize instance to enumerate contents at path, which should be a
 *  directory and can be specified in relative or absolute, and may include
 *  Unix conventions like '<code>~</code>' for user home directory, which will
 *  be appropriately converted on Windoze systems.  The justContents flag, if
 *  set, is equivalent to recurseIntoSubdirectories = NO and followSymlinks =
 *  NO, but the implementation will be made more efficient.
 */
- (id) initWithDirectoryPath: (NSString*)path
   recurseIntoSubdirectories: (BOOL)recurse
	      followSymlinks: (BOOL)follow
		justContents: (BOOL)justContents
			 for: (NSFileManager*)mgr
{
  return [self initWithDirectoryPath: path
           recurseIntoSubdirectories: recurse
                      followSymlinks: follow
                        justContents: justContents
                          skipHidden: NO
                        errorHandler: NULL
                                 for: mgr];
}

- (void) dealloc
{
  GSIArrayEmpty(_stack);
  NSZoneFree([self zone], _stack);
  DESTROY(_topPath);
  DESTROY(_currentFilePath);
  DESTROY(_mgr);
  [super dealloc];
}

/**
 * Returns a dictionary containing the attributes of the directory
 * at which enumeration started. <br />
 * The contents of this dictionary are as produced by
 * [NSFileManager-fileAttributesAtPath:traverseLink:]
 */
- (NSDictionary*) directoryAttributes
{
  return [_mgr fileAttributesAtPath: _topPath
		       traverseLink: _flags.isFollowing];
}

/**
 * Returns a dictionary containing the attributes of the file
 * currently being enumerated. <br />
 * The contents of this dictionary are as produced by
 * [NSFileManager-fileAttributesAtPath:traverseLink:]
 */
- (NSDictionary*) fileAttributes
{
  return [_mgr fileAttributesAtPath: _currentFilePath
		       traverseLink: _flags.isFollowing];
}

/**
 * Informs the receiver that any descendents of the current directory
 * should be skipped rather than enumerated.  Use this to avoid enumerating
 * the contents of directories you are not interested in.
 */
- (void) skipDescendents
{
  if (GSIArrayCount(_stack) > 0)
    {
      GSIArrayRemoveLastItem(_stack);
      if (_currentFilePath != 0)
	{
	  DESTROY(_currentFilePath);
	}
    }
}

/*
 * finds the next file according to the top enumerator
 * - if there is a next file it is put in currentFile
 * - if the current file is a directory and if isRecursive calls
 * recurseIntoDirectory: currentFile
 * - if the current file is a symlink to a directory and if isRecursive
 * and isFollowing calls recurseIntoDirectory: currentFile
 * - if at end of current directory pops stack and attempts to
 * find the next entry in the parent
 * - sets currentFile to nil if there are no more files to enumerate
 */
- (id) nextObject
{
  NSString *returnFileName = 0;

  if (_currentFilePath != 0)
    {
      DESTROY(_currentFilePath);
    }

  while (GSIArrayCount(_stack) > 0)
    {
      GSEnumeratedDirectory dir = GSIArrayLastItem(_stack).ext;
      struct _STATB	statbuf;
#if defined(_WIN32)
      const wchar_t *dirname = NULL;
#else
      const char *dirname = NULL;
#endif

#ifdef __ANDROID__
      if (dir.assetDir)
	{
	  /* This will only return files and not directories, which means that
	   * recursion is not supported.
	   * See https://issuetracker.google.com/issues/37002833
	   */
	  dirname = AAssetDir_getNextFileName(dir.assetDir);
	}
      else if (dir.pointer)
#endif
      {
        struct _DIRENT *dirbuf = _READDIR(dir.pointer);
        if (dirbuf)
	  {
	    dirname = dirbuf->d_name;
	  }
      }

      if (dirname)
	{
          // Skip it if it is hidden and flag is yes...
          if ([[dir.path lastPathComponent] hasPrefix: @"."]
	    && _flags.skipHidden == YES)
            {
              continue;
            }
          
#if defined(_WIN32)
	  /* Skip "." and ".." directory entries */
	  if (wcscmp(dirname, L".") == 0
	    || wcscmp(dirname, L"..") == 0)
	    {
	      continue;
	    }
          
	  /* Name of file to return  */
	  returnFileName = [_mgr
	    stringWithFileSystemRepresentation: dirname
	    length: wcslen(dirname)];
#else
	  /* Skip "." and ".." directory entries */
	  if (strcmp(dirname, ".") == 0
	    || strcmp(dirname, "..") == 0)
	    {
	      continue;
	    }

          /* Name of file to return  */
	  returnFileName = [_mgr
	    stringWithFileSystemRepresentation: dirname
	    length: strlen(dirname)];
#endif
	  /* if we have a null FileName something went wrong (charset?)
	   * and we skip it */
	  if (returnFileName == nil)
	    continue;
	  
	  returnFileName = RETAIN([dir.path stringByAppendingPathComponent:
	    returnFileName]);

	  if (!_flags.justContents)
	    _currentFilePath = RETAIN([_topPath stringByAppendingPathComponent:
	      returnFileName]);

	  if (_flags.isRecursive == YES)
	    {
	      // Do not follow links
#ifdef S_IFLNK
#ifdef _WIN32
#warning "lstat does not support unichars"
#else
	      if (!_flags.isFollowing)
		{
		  if (lstat([_mgr fileSystemRepresentationWithPath:
		    _currentFilePath], &statbuf) != 0)
		    {
		      break;
		    }
		  // If link then return it as link
		  if (S_IFLNK == (S_IFMT & statbuf.st_mode))
		    {
		      break;
		    }
		}
	      else
#endif
#endif
		{
		  if (_STAT([_mgr fileSystemRepresentationWithPath:
		    _currentFilePath], &statbuf) != 0)
		    {
		      break;
		    }
		}
	      if (S_IFDIR == (S_IFMT & statbuf.st_mode))
		{
		  _DIR  *dir_pointer;

		  dir_pointer
		    = _OPENDIR([_mgr fileSystemRepresentationWithPath:
		    _currentFilePath]);
		  if (dir_pointer)
		    {
		      GSIArrayItem item;

		      item.ext.path = RETAIN(returnFileName);
		      item.ext.pointer = dir_pointer;

		      GSIArrayAddItem(_stack, item);
		    }
		  else
		    {
                      BOOL flag = YES;

		      NSDebugLog(@"Failed to recurse into directory '%@' - %@",
			_currentFilePath, [NSError _last]);
                      if (_errorHandler != NULL)
                        {
                          flag = CALL_BLOCK(_errorHandler,
			    [NSURL URLWithString: _currentFilePath],
			    [NSError _last]);
                        }
                      if (flag == NO)
                        {
                          return nil; // Stop enumeration...
                        }
                    }
		}
	    }
	  break;	// Got a file name - break out of loop
	}
      else
	{
	  GSIArrayRemoveLastItem(_stack);
	  if (_currentFilePath != 0)
	    {
	      DESTROY(_currentFilePath);
	    }
	}
    }
  return AUTORELEASE(returnFileName);
}

@end /* NSDirectoryEnumerator */

/**
 * Convenience methods for accessing named file attributes in a dictionary.
 */
@implementation NSDictionary(NSFileAttributes)

/**
 * Return the file creation date attribute (or nil if not found).
 */
- (NSDate*) fileCreationDate
{
  return [self objectForKey: NSFileCreationDate];
}

/**
 * Return the file extension hidden attribute (or NO if not found).
 */
- (BOOL) fileExtensionHidden
{
  return [[self objectForKey: NSFileExtensionHidden] boolValue];
}

/**
 *  Returns HFS creator attribute (OS X).
 */
- (OSType) fileHFSCreatorCode
{
  return [[self objectForKey: NSFileHFSCreatorCode] unsignedLongValue];
}

/**
 *  Returns HFS type code attribute (OS X).
 */
- (OSType) fileHFSTypeCode
{
  return [[self objectForKey: NSFileHFSTypeCode] unsignedLongValue];
}

/**
 * Return the file append only attribute (or NO if not found).
 */
- (BOOL) fileIsAppendOnly
{
  return [[self objectForKey: NSFileAppendOnly] boolValue];
}

/**
 * Return the file immutable attribute (or NO if not found).
 */
- (BOOL) fileIsImmutable
{
  return [[self objectForKey: NSFileImmutable] boolValue];
}

/**
 * Return the size of the file, or NSNotFound if the file size attribute
 * is not found in the dictionary.
 */
- (unsigned long long) fileSize
{
  NSNumber	*n = [self objectForKey: NSFileSize];

  if (n == nil)
    {
      return NSNotFound;
    }
  return [n unsignedLongLongValue];
}

/**
 * Return the file type attribute or nil if not present.
 */
- (NSString*) fileType
{
  return [self objectForKey: NSFileType];
}

/**
 * Return the file owner account name attribute or nil if not present.
 */
- (NSString*) fileOwnerAccountName
{
  return [self objectForKey: NSFileOwnerAccountName];
}

/**
 * Return an NSNumber with the numeric value of the NSFileOwnerAccountID attribute
 * in the dictionary, or nil if the attribute is not present.
 */
- (NSNumber*) fileOwnerAccountID
{
  return [self objectForKey: NSFileOwnerAccountID];
}

/**
 * Return the file group owner account name attribute or nil if not present.
 */
- (NSString*) fileGroupOwnerAccountName
{
  return [self objectForKey: NSFileGroupOwnerAccountName];
}

/**
 * Return an NSNumber with the numeric value of the NSFileGroupOwnerAccountID attribute
 * in the dictionary, or nil if the attribute is not present.
 */
- (NSNumber*) fileGroupOwnerAccountID
{
  return [self objectForKey: NSFileGroupOwnerAccountID];
}

/**
 * Return the file modification date attribute (or nil if not found)
 */
- (NSDate*) fileModificationDate
{
  return [self objectForKey: NSFileModificationDate];
}

/**
 * Return the file posix permissions attribute (or NSNotFound if
 * the attribute is not present in the dictionary).
 */
- (NSUInteger) filePosixPermissions
{
  NSNumber	*n = [self objectForKey: NSFilePosixPermissions];

  if (n == nil)
    {
      return NSNotFound;
    }
  return [n unsignedIntegerValue];
}

/**
 * Return the file system number attribute (or NSNotFound if
 * the attribute is not present in the dictionary).
 */
- (NSUInteger) fileSystemNumber
{
  NSNumber	*n = [self objectForKey: NSFileSystemNumber];

  if (n == nil)
    {
      return NSNotFound;
    }
  return [n unsignedIntegerValue];
}

/**
 * Return the file system file identification number attribute
 * or NSNotFound if the attribute is not present in the dictionary).
 */
- (NSUInteger) fileSystemFileNumber
{
  NSNumber	*n = [self objectForKey: NSFileSystemFileNumber];

  if (n == nil)
    {
      return NSNotFound;
    }
  return [n unsignedIntegerValue];
}
@end

@implementation NSFileManager (PrivateMethods)

- (BOOL) _copyFile: (NSString*)source
	    toFile: (NSString*)destination
	   handler: (id)handler
{
#if defined(_WIN32)
  if (CopyFileW([self fileSystemRepresentationWithPath: source],
    [self fileSystemRepresentationWithPath: destination], NO))
    {
      return YES;
    }

  return [self _proceedAccordingToHandler: handler
				 forError: @"cannot copy file"
				   inPath: source
				 fromPath: source
				   toPath: destination];

#else
  NSDictionary	*attributes;
  NSDate        *modification;
  unsigned long long	fileSize;
  unsigned long long	i;
  int		bufsize = 8096;
  int		sourceFd;
  int		destFd;
  int		fileMode;
  int		rbytes;
  int		wbytes;
  char		buffer[bufsize];
#ifdef __ANDROID__
  AAsset	*asset = NULL;
#endif

  attributes = [self fileAttributesAtPath: source traverseLink: NO];
  if (nil == attributes)
    {
      return [self _proceedAccordingToHandler: handler
				     forError: @"source file does not exist"
				       inPath: source
				     fromPath: source
				       toPath: destination];
    }

  fileSize = [attributes fileSize];
  fileMode = [attributes filePosixPermissions];
  modification = [attributes fileModificationDate];

  /* Open the source file. In case of error call the handler. */
  sourceFd = open([self fileSystemRepresentationWithPath: source],
    GSBINIO|O_RDONLY);
#ifdef __ANDROID__
  if (sourceFd < 0)
    {
      // Android: try using asset manager if path is in main bundle resources
      asset = [NSBundle assetForPath: source withMode: AASSET_MODE_STREAMING];
    }
  if (sourceFd < 0 && asset == NULL)
#else
  if (sourceFd < 0)
#endif
    {
      return [self _proceedAccordingToHandler: handler
				     forError: @"cannot open file for reading"
				       inPath: source
				     fromPath: source
				       toPath: destination];
    }

  /* Open the destination file. In case of error call the handler. */
  destFd = open([self fileSystemRepresentationWithPath: destination],
    GSBINIO|O_WRONLY|O_CREAT|O_TRUNC, fileMode);
  if (destFd < 0)
    {
#ifdef __ANDROID__
      if (asset)
	{
	  AAsset_close(asset);
	}
      else
#endif
      close (sourceFd);

      return [self _proceedAccordingToHandler: handler
				     forError:  @"cannot open file for writing"
				       inPath: destination
				     fromPath: source
				       toPath: destination];
    }

  /* Read bufsize bytes from source file and write them into the destination
     file. In case of errors call the handler and abort the operation. */
  for (i = 0; i < fileSize; i += rbytes)
    {
#ifdef __ANDROID__
      if (asset)
	{
	  rbytes = AAsset_read(asset, buffer, bufsize);
	}
      else
#endif
      rbytes = read (sourceFd, buffer, bufsize);
      if (rbytes <= 0)
	{
          if (0 == rbytes)
            {
              break;    // End of input file
            }
#ifdef __ANDROID__
          if (asset)
	    {
	      AAsset_close(asset);
	    }
          else
#endif
          close (sourceFd);
          close (destFd);

          return [self _proceedAccordingToHandler: handler
					 forError: @"cannot read from file"
					   inPath: source
					 fromPath: source
					   toPath: destination];
	}

      wbytes = write (destFd, buffer, rbytes);
      if (wbytes != rbytes)
	{
#ifdef __ANDROID__
          if (asset)
	    {
	      AAsset_close(asset);
	    }
          else
#endif
          close (sourceFd);
          close (destFd);

          return [self _proceedAccordingToHandler: handler
					 forError: @"cannot write to file"
					   inPath: destination
					 fromPath: source
					   toPath: destination];
        }
    }
#ifdef __ANDROID__
  if (asset)
    {
      AAsset_close(asset);
    }
  else
#endif
  close (sourceFd);
  close (destFd);

  /* Check for modification during copy.
   */
  attributes = [self fileAttributesAtPath: source traverseLink: NO];
  if (NO == [modification isEqual: [attributes fileModificationDate]]
    || [attributes fileSize] != fileSize)
    {
      return [self _proceedAccordingToHandler: handler
                                     forError: @"source modified during copy"
                                       inPath: destination
                                     fromPath: source
                                       toPath: destination];
    }
  return YES;
#endif
}

- (BOOL) _copyPath: (NSString*)source
	    toPath: (NSString*)destination
	   handler: handler
{
  NSDirectoryEnumerator	*enumerator;
  NSString		*dirEntry;
  BOOL			result = YES;
  ENTER_POOL

  enumerator = [self enumeratorAtPath: source];
  while ((dirEntry = [enumerator nextObject]))
    {
      NSString		*sourceFile;
      NSString		*fileType;
      NSString		*destinationFile;
      NSDictionary	*attributes;

      attributes = [enumerator fileAttributes];
      fileType = [attributes fileType];
      sourceFile = [source stringByAppendingPathComponent: dirEntry];
      destinationFile
	= [destination stringByAppendingPathComponent: dirEntry];

      [self _sendToHandler: handler willProcessPath: sourceFile];

      if ([fileType isEqual: NSFileTypeDirectory])
	{
          NSMutableDictionary   *newAttributes;
	  BOOL	                dirOK;

          newAttributes = [attributes mutableCopy];
          [newAttributes removeObjectForKey: NSFileOwnerAccountID];
          [newAttributes removeObjectForKey: NSFileGroupOwnerAccountID];
          [newAttributes removeObjectForKey: NSFileGroupOwnerAccountName];
          [newAttributes setObject: NSUserName()
                            forKey: NSFileOwnerAccountName];
	  dirOK = [self createDirectoryAtPath: destinationFile
				   attributes: newAttributes];
          RELEASE(newAttributes);
	  if (dirOK == NO)
	    {
              if (![self _proceedAccordingToHandler: handler
					   forError: _lastError
					     inPath: destinationFile
					   fromPath: sourceFile
					     toPath: destinationFile])
                {
                  result = NO;
		  break;
                }
	      /*
	       * We may have managed to create the directory but not set
	       * its attributes ... if so we can continue copying.
	       */
	      if (![self fileExistsAtPath: destinationFile isDirectory: &dirOK])
	        {
		  dirOK = NO;
	        }
	    }
	  if (dirOK == YES)
	    {
	      [enumerator skipDescendents];
	      if (![self _copyPath: sourceFile
                            toPath: destinationFile
                           handler: handler])
                {
                  result = NO;
                  break;
                }
	    }
	}
      else if ([fileType isEqual: NSFileTypeRegular])
	{
	  if (![self _copyFile: sourceFile
			toFile: destinationFile
		       handler: handler])
            {
              result = NO;
              break;
            }
	}
      else if ([fileType isEqual: NSFileTypeSymbolicLink])
	{
	  NSString	*path;

	  path = [self pathContentOfSymbolicLinkAtPath: sourceFile];
	  if (![self createSymbolicLinkAtPath: destinationFile
				  pathContent: path])
	    {
              if (![self _proceedAccordingToHandler: handler
		forError: @"cannot create symbolic link"
		inPath: sourceFile
		fromPath: sourceFile
		toPath: destinationFile])
                {
                  result = NO;
		  break;
                }
	    }
	}
      else
	{
	  NSString	*s;

	  s = [NSString stringWithFormat: @"cannot copy file type '%@'",
	    fileType];
	  ASSIGN(_lastError, s);
	  NSDebugLog(@"%@: %@", sourceFile, s);
	  continue;
	}
      [self changeFileAttributes: attributes atPath: destinationFile];
    }
  LEAVE_POOL

  return result;
}

- (BOOL) _linkPath: (NSString*)source
	    toPath: (NSString*)destination
	   handler: handler
{
#ifdef HAVE_LINK
  NSDirectoryEnumerator	*enumerator;
  NSString		*dirEntry;
  BOOL			result = YES;
  ENTER_POOL

  enumerator = [self enumeratorAtPath: source];
  while ((dirEntry = [enumerator nextObject]))
    {
      NSString		*sourceFile;
      NSString		*fileType;
      NSString		*destinationFile;
      NSDictionary	*attributes;

      attributes = [enumerator fileAttributes];
      fileType = [attributes fileType];
      sourceFile = [source stringByAppendingPathComponent: dirEntry];
      destinationFile
	= [destination stringByAppendingPathComponent: dirEntry];

      [self _sendToHandler: handler willProcessPath: sourceFile];

      if ([fileType isEqual: NSFileTypeDirectory] == YES)
	{
	  if ([self createDirectoryAtPath: destinationFile
			       attributes: attributes] == NO)
	    {
              if ([self _proceedAccordingToHandler: handler
					  forError: _lastError
					    inPath: destinationFile
					  fromPath: sourceFile
					    toPath: destinationFile] == NO)
                {
                  result = NO;
		  break;
                }
	    }
	  else
	    {
	      [enumerator skipDescendents];
	      if ([self _linkPath: sourceFile
			   toPath: destinationFile
			  handler: handler] == NO)
		{
		  result = NO;
		  break;
		}
	    }
	}
      else if ([fileType isEqual: NSFileTypeSymbolicLink])
	{
	  NSString	*path;

	  path = [self pathContentOfSymbolicLinkAtPath: sourceFile];
	  if ([self createSymbolicLinkAtPath: destinationFile
				 pathContent: path] == NO)
	    {
              if ([self _proceedAccordingToHandler: handler
		forError: @"cannot create symbolic link"
		inPath: sourceFile
		fromPath: sourceFile
		toPath: destinationFile] == NO)
                {
                  result = NO;
		  break;
                }
	    }
	}
      else
	{
	  if (link([self fileSystemRepresentationWithPath: sourceFile],
	    [self fileSystemRepresentationWithPath: destinationFile]) < 0)
	    {
              if ([self _proceedAccordingToHandler: handler
		forError: @"cannot create hard link"
		inPath: sourceFile
		fromPath: sourceFile
		toPath: destinationFile] == NO)
                {
                  result = NO;
		  break;
                }
	    }
	}
      [self changeFileAttributes: attributes atPath: destinationFile];
    }
  LEAVE_POOL
  return result;
#else
  ASSIGN(_lastError, @"Links not supported on this platform");
  return NO;
#endif
}

- (void) _sendToHandler: (id) handler
        willProcessPath: (NSString*) path
{
  if ([handler respondsToSelector: @selector (fileManager:willProcessPath:)])
    {
      [handler fileManager: self willProcessPath: path];
    }
}

- (BOOL) _proceedAccordingToHandler: (id) handler
                           forError: (NSString*) error
                             inPath: (NSString*) path
{
  if ([handler respondsToSelector:
    @selector (fileManager:shouldProceedAfterError:)])
    {
      NSDictionary *errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                                path, NSFilePathErrorKey,
                                              error, @"Error", nil];
      return [handler fileManager: self
	  shouldProceedAfterError: errorInfo];
    }
  return NO;
}

- (BOOL) _proceedAccordingToHandler: (id) handler
                           forError: (NSString*) error
                             inPath: (NSString*) path
                           fromPath: (NSString*) fromPath
                             toPath: (NSString*) toPath
{
  if ([handler respondsToSelector:
    @selector (fileManager:shouldProceedAfterError:)])
    {
      NSDictionary *errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                                path, NSFilePathErrorKey,
                                              fromPath, @"FromPath",
                                              toPath, @"ToPath",
                                              error, @"Error", nil];
      return [handler fileManager: self
	  shouldProceedAfterError: errorInfo];
    }
  return NO;
}

- (NSError*) _errorFrom: (NSString *)fromPath to: (NSString *)toPath
{
  NSError       *error;
  NSDictionary  *errorInfo;
  NSString      *message;
  NSString      *domain;
  NSInteger     code;

  if (_lastError)
    {
      message = _lastError;
      domain = NSCocoaErrorDomain;
      code = 0;
    }
  else
    {
      error = [NSError _last];
      message = [error localizedDescription];
      domain = [error domain];
      code = [error code];
    }

  if (fromPath && toPath)
    {
      errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
        fromPath, @"FromPath",
        toPath, @"ToPath",
        message, NSLocalizedDescriptionKey,
        nil];
    }
  else if (fromPath)
    {
      errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
        fromPath, NSFilePathErrorKey,
        message, NSLocalizedDescriptionKey,
        nil];      
    }
  else
    {
      errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
        message, NSLocalizedDescriptionKey,
        nil];      
    }

  error = [NSError errorWithDomain: domain
                              code: code
                          userInfo: errorInfo];
  DESTROY(_lastError);
  return error;
}

@end /* NSFileManager (PrivateMethods) */



@implementation	GSAttrDictionary

static NSSet	*fileKeys = nil;

+ (NSDictionary*) attributesAt: (NSString *)path
		  traverseLink: (BOOL)traverse
{
  GSAttrDictionary	*d;
  unsigned		l = 0;
  unsigned		i;
  const _CHAR *lpath = [defaultManager fileSystemRepresentationWithPath: path];
#ifdef __ANDROID__
  AAsset *asset = NULL;
#endif

  if (lpath == 0 || *lpath == 0)
    {
      return nil;
    }
  while (lpath[l] != 0)
    {
      l++;
    }
  d = (GSAttrDictionary*)NSAllocateObject(self, (l+1)*sizeof(_CHAR),
    NSDefaultMallocZone());

#if defined(S_IFLNK) && !defined(_WIN32)
  if (traverse == NO)
    {
      if (lstat(lpath, &d->statbuf) != 0)
	{
#ifdef __ANDROID__
	  /* Android: try using asset manager if path is in
	   * main bundle resources
	   */
	  asset = [NSBundle assetForPath: path];
	  if (asset == NULL)
#endif
	  DESTROY(d);
	}
    }
  else
#endif
  if (_STAT(lpath, &d->statbuf) != 0)
    {
#ifdef __ANDROID__
      // Android: try using asset manager if path is in main bundle resources
      asset = [NSBundle assetForPath: path];
      if (asset == NULL)
#endif
      DESTROY(d);
    }
  if (d != nil)
    {
      for (i = 0; i <= l; i++)
	{
	  d->_path[i] = lpath[i];
	}
#ifdef __ANDROID__
      if (asset)
	{
	  // set some basic stat values for Android assets
	  memset(&d->statbuf, 0, sizeof(d->statbuf));
	  d->statbuf.st_mode = S_IRUSR;
	  d->statbuf.st_size = AAsset_getLength(asset);
	  AAsset_close(asset);
	}
#endif
    }
  return AUTORELEASE(d);
}

+ (void) initialize
{
  if (fileKeys == nil)
    {
      fileKeys = [[NSSet alloc] initWithObjects:
	NSFileAppendOnly,
	NSFileCreationDate,
	NSFileDeviceIdentifier,
	NSFileExtensionHidden,
	NSFileGroupOwnerAccountName,
	NSFileGroupOwnerAccountID,
	NSFileHFSCreatorCode,
	NSFileHFSTypeCode,
	NSFileImmutable,
	NSFileModificationDate,
	NSFileOwnerAccountName,
	NSFileOwnerAccountID,
	NSFilePosixPermissions,
	NSFileReferenceCount,
	NSFileSize,
	NSFileSystemFileNumber,
	NSFileSystemNumber,
	NSFileType,
	nil];
      [[NSObject leakAt: &fileKeys] release];
    }
}

- (NSUInteger) count
{
  return [fileKeys count];
}

- (NSDate*) fileCreationDate
{
#if defined(_WIN32)
  return [NSDate dateWithTimeIntervalSince1970: statbuf.st_ctime];
#elif defined (HAVE_STRUCT_STAT_ST_BIRTHTIM)
  NSTimeInterval ti;
  ti = statbuf.st_birthtim.tv_sec + (double)statbuf.st_birthtim.tv_nsec / 1.0e9;
  return [NSDate dateWithTimeIntervalSince1970: ti];
#elif defined (HAVE_STRUCT_STAT_ST_BIRTHTIME)
  return [NSDate dateWithTimeIntervalSince1970: statbuf.st_birthtime];
#elif defined (HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC) || defined (HAVE_STRUCT_STAT64_ST_BIRTHTIMESPEC)
  NSTimeInterval ti;
  ti = statbuf.st_birthtimespec.tv_sec + (double)statbuf.st_birthtimespec.tv_nsec / 1.0e9;
  return [NSDate dateWithTimeIntervalSince1970: ti];
#else
  /* We don't know a better way to get creation date, it is not defined in POSIX
   * Use the earlier of ctime or mtime
   */
  if (statbuf.st_ctime < statbuf.st_mtime)
    return [NSDate dateWithTimeIntervalSince1970: statbuf.st_ctime];
  else
    return [NSDate dateWithTimeIntervalSince1970: statbuf.st_mtime];
#endif
}

- (BOOL) fileExtensionHidden
{
  return NO;
}

- (NSNumber*) fileGroupOwnerAccountID
{
  return [NSNumber numberWithInt: statbuf.st_gid];
}

- (NSString*) fileGroupOwnerAccountName
{
  NSString	*group = @"UnknownGroup";

#if	defined(_WIN32)
  DWORD		returnCode = 0;
  PSID		sidOwner;
  int		result = TRUE;
  _CHAR		account[BUFSIZ];
  _CHAR		domain[BUFSIZ];
  DWORD		accountSize = 1024;
  DWORD		domainSize = 1024;
  SID_NAME_USE	eUse = SidTypeUnknown;
  HANDLE	hFile;
  PSECURITY_DESCRIPTOR pSD;

  // Get the handle of the file object.
  hFile = CreateFileW(
    _path,
    GENERIC_READ,
    FILE_SHARE_READ,
    0,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS,
    0);

  // Check GetLastError for CreateFile error code.
  if (hFile == INVALID_HANDLE_VALUE)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      NSDebugMLog(@"Error %d getting file handle for '%S'",
        dwErrorCode, _path);
      return group;
    }

  // Get the group SID of the file.
  returnCode = GetSecurityInfo(
    hFile,
    SE_FILE_OBJECT,
    GROUP_SECURITY_INFORMATION,
    0,
    &sidOwner,
    0,
    0,
    &pSD);

  CloseHandle(hFile);

  // Check GetLastError for GetSecurityInfo error condition.
  if (returnCode != ERROR_SUCCESS)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      NSDebugMLog(@"Error %d getting security info for '%S'",
        dwErrorCode, _path);
      return group;
    }

  // First call to LookupAccountSid to get the buffer sizes.
  result = LookupAccountSidW(
    0,           // local computer
    sidOwner,
    account,
    (LPDWORD)&accountSize,
    domain,
    (LPDWORD)&domainSize,
    &eUse);

  // Check GetLastError for LookupAccountSid error condition.
  if (result == FALSE)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      if (dwErrorCode == ERROR_NONE_MAPPED)
	NSDebugMLog(@"Error %d in LookupAccountSid for '%S'", _path);
      else
        NSDebugMLog(@"Error %d getting security info for '%S'",
          dwErrorCode, _path);
      return group;
    }

  if (accountSize >= 1024)
    {
      NSDebugMLog(@"Account name for '%S' is unreasonably long", _path);
      return group;
    }
  return [NSString stringWithCharacters: account length: accountSize];
#else
#if defined(HAVE_GRP_H)
#if defined(HAVE_GETGRGID_H)
  struct group gp;
  struct group *p;
  char buf[BUFSIZ*10];

  if (getgrgid_r(statbuf.st_gid, &gp, buf, sizeof(buf), &p) == 0)
    {
      group = [NSString stringWithCString: gp.gr_name
				 encoding: defaultEncoding];
    }
#else
#if defined(HAVE_GETGRGID)
  struct group	*gp;

  [gnustep_global_lock lock];
  gp = getgrgid(statbuf.st_gid);
  if (gp != 0)
    {
      group = [NSString stringWithCString: gp->gr_name
				 encoding: defaultEncoding];
    }
  [gnustep_global_lock unlock];
#endif
#endif
#endif
#endif
  return group;
}

- (OSType) fileHFSCreatorCode
{
  return 0;
}

- (OSType) fileHFSTypeCode
{
  return 0;
}

- (BOOL) fileIsAppendOnly
{
  return 0;
}

- (BOOL) fileIsImmutable
{
  return 0;
}

- (NSDate*) fileModificationDate
{
  NSTimeInterval ti;

#if defined (HAVE_STRUCT_STAT_ST_MTIM)
  ti = statbuf.st_mtim.tv_sec + (double)statbuf.st_mtim.tv_nsec / 1.0e9;
#else
  ti = (double)statbuf.st_mtime;
#endif

  return [NSDate dateWithTimeIntervalSince1970: ti];
}

- (NSUInteger) filePosixPermissions
{
  return (statbuf.st_mode & ~S_IFMT);
}

- (NSNumber*) fileOwnerAccountID
{
  return [NSNumber numberWithInt: statbuf.st_uid];
}

- (NSString*) fileOwnerAccountName
{
  NSString	*owner = @"UnknownUser";

#if	defined(_WIN32)
  DWORD		returnCode = 0;
  PSID		sidOwner;
  int		result = TRUE;
  _CHAR		account[BUFSIZ];
  _CHAR		domain[BUFSIZ];
  DWORD		accountSize = 1024;
  DWORD		domainSize = 1024;
  SID_NAME_USE	eUse = SidTypeUnknown;
  HANDLE	hFile;
  PSECURITY_DESCRIPTOR pSD;

  // Get the handle of the file object.
  hFile = CreateFileW(
    _path,
    GENERIC_READ,
    FILE_SHARE_READ,
    0,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS,
    0);

  // Check GetLastError for CreateFile error code.
  if (hFile == INVALID_HANDLE_VALUE)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      NSDebugMLog(@"Error %d getting file handle for '%S'",
        dwErrorCode, _path);
      return owner;
    }

  // Get the owner SID of the file.
  returnCode = GetSecurityInfo(
    hFile,
    SE_FILE_OBJECT,
    OWNER_SECURITY_INFORMATION,
    &sidOwner,
    0,
    0,
    0,
    &pSD);

  CloseHandle(hFile);

  // Check GetLastError for GetSecurityInfo error condition.
  if (returnCode != ERROR_SUCCESS)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      NSDebugMLog(@"Error %d getting security info for '%S'",
        dwErrorCode, _path);
      return owner;
    }

  // First call to LookupAccountSid to get the buffer sizes.
  result = LookupAccountSidW(
    0,           // local computer
    sidOwner,
    account,
    (LPDWORD)&accountSize,
    domain,
    (LPDWORD)&domainSize,
    &eUse);

  // Check GetLastError for LookupAccountSid error condition.
  if (result == FALSE)
    {
      DWORD dwErrorCode = 0;

      dwErrorCode = GetLastError();
      if (dwErrorCode == ERROR_NONE_MAPPED)
	NSDebugMLog(@"Error %d in LookupAccountSid for '%S'", _path);
      else
        NSDebugMLog(@"Error %d getting security info for '%S'",
          dwErrorCode, _path);
      return owner;
    }

  if (accountSize >= 1024)
    {
      NSDebugMLog(@"Account name for '%S' is unreasonably long", _path);
      return owner;
    }
  return [NSString stringWithCharacters: account length: accountSize];
#else
#ifdef HAVE_PWD_H
#if     defined(HAVE_GETPWUID_R)
  struct passwd pw;
  struct passwd *p;
  char buf[BUFSIZ*10];

  if (getpwuid_r(statbuf.st_uid, &pw, buf, sizeof(buf), &p) == 0)
    {
      owner = [NSString stringWithCString: pw.pw_name
				 encoding: defaultEncoding];
    }
#else
#if     defined(HAVE_GETPWUID)
  struct passwd *pw;

  [gnustep_global_lock lock];
  pw = getpwuid(statbuf.st_uid);
  if (pw != 0)
    {
      owner = [NSString stringWithCString: pw->pw_name
				 encoding: defaultEncoding];
    }
  [gnustep_global_lock unlock];
#endif
#endif
#endif /* HAVE_PWD_H */
#endif
  return owner;
}

- (unsigned long long) fileSize
{
  return statbuf.st_size;
}

- (NSUInteger) fileSystemFileNumber
{
  return statbuf.st_ino;
}

- (NSUInteger) fileSystemNumber
{
#if defined(_WIN32)
  DWORD volumeSerialNumber = 0;
  _CHAR volumePathName[128];
  if (GetVolumePathNameW(_path,volumePathName,128))
  {
    GetVolumeInformationW(volumePathName,NULL,0,&volumeSerialNumber,NULL,NULL,NULL,0);
  }

  return (NSUInteger)volumeSerialNumber;
#else
  return statbuf.st_dev;
#endif
}

- (NSString*) fileType
{
  switch (statbuf.st_mode & S_IFMT)
    {
      case S_IFREG: return NSFileTypeRegular;
      case S_IFDIR: return NSFileTypeDirectory;
      case S_IFCHR: return NSFileTypeCharacterSpecial;
#ifdef S_IFBLK
      case S_IFBLK: return NSFileTypeBlockSpecial;
#endif
#ifdef S_IFLNK
      case S_IFLNK: return NSFileTypeSymbolicLink;
#endif
#ifdef S_IFIFO
      case S_IFIFO: return NSFileTypeFifo;
#endif
#ifdef S_IFSOCK
      case S_IFSOCK: return NSFileTypeSocket;
#endif
      default: return NSFileTypeUnknown;
    }
}

- (NSEnumerator*) keyEnumerator
{
  return [fileKeys objectEnumerator];
}

- (NSEnumerator*) objectEnumerator
{
  return [GSAttrDictionaryEnumerator enumeratorFor: self];
}

- (id) objectForKey: (id)key
{
  int	count = 0;

  while (key != 0 && count < 2)
    {
      if (key == NSFileAppendOnly)
	return [NSNumber numberWithBool: [self fileIsAppendOnly]];
      if (key == NSFileCreationDate)
	return [self fileCreationDate];
      if (key == NSFileDeviceIdentifier)
	return [NSNumber numberWithUnsignedInt: statbuf.st_dev];
      if (key == NSFileExtensionHidden)
	return [NSNumber numberWithBool: [self fileExtensionHidden]];
      if (key == NSFileGroupOwnerAccountName)
	return [self fileGroupOwnerAccountName];
      if (key == NSFileGroupOwnerAccountID)
	return [self fileGroupOwnerAccountID];
      if (key == NSFileHFSCreatorCode)
	return [NSNumber numberWithUnsignedLong: [self fileHFSCreatorCode]];
      if (key == NSFileHFSTypeCode)
	return [NSNumber numberWithUnsignedLong: [self fileHFSTypeCode]];
      if (key == NSFileImmutable)
	return [NSNumber numberWithBool: [self fileIsImmutable]];
      if (key == NSFileModificationDate)
	return [self fileModificationDate];
      if (key == NSFileOwnerAccountName)
	return [self fileOwnerAccountName];
      if (key == NSFileOwnerAccountID)
	return [self fileOwnerAccountID];
      if (key == NSFilePosixPermissions)
	return [NSNumber numberWithUnsignedInt: [self filePosixPermissions]];
      if (key == NSFileReferenceCount)
	return [NSNumber numberWithUnsignedInt: statbuf.st_nlink];
      if (key == NSFileSize)
	return [NSNumber numberWithUnsignedLongLong: [self fileSize]];
      if (key == NSFileSystemFileNumber)
	return [NSNumber numberWithUnsignedInt: [self fileSystemFileNumber]];
      if (key == NSFileSystemNumber)
	return [NSNumber numberWithUnsignedInt: [self fileSystemNumber]];
      if (key == NSFileType)
	return [self fileType];

      /*
       * Now, if we didn't get an exact pointer match, check for
       * string equalities and ensure we get an exact match next
       * time round the loop.
       */
      count++;
      key = [fileKeys member: key];
    }
  if (count >= 2)
    {
      NSDebugLog(@"Warning ... key '%@' not handled", key);
    }
  return nil;
}

@end	/* GSAttrDictionary */

@implementation	GSAttrDictionaryEnumerator
+ (NSEnumerator*) enumeratorFor: (NSDictionary*)d
{
  GSAttrDictionaryEnumerator	*e;

  e = (GSAttrDictionaryEnumerator*)
    NSAllocateObject(self, 0, NSDefaultMallocZone());
  e->dictionary = RETAIN(d);
  e->enumerator = RETAIN([fileKeys objectEnumerator]);
  return AUTORELEASE(e);
}

- (void) dealloc
{
  RELEASE(enumerator);
  RELEASE(dictionary);
  [super dealloc];
}

- (id) nextObject
{
  NSString	*key = [enumerator nextObject];
  id		val = nil;

  if (key != nil)
    {
      val = [dictionary objectForKey: key];
    }
  return val;
}
@end

NSString * const NSFileAppendOnly = @"NSFileAppendOnly";
NSString * const NSFileCreationDate = @"NSFileCreationDate";
NSString * const NSFileDeviceIdentifier = @"NSFileDeviceIdentifier";
NSString * const NSFileExtensionHidden = @"NSFileExtensionHidden";
NSString * const NSFileGroupOwnerAccountID = @"NSFileGroupOwnerAccountID";
NSString * const NSFileGroupOwnerAccountName = @"NSFileGroupOwnerAccountName";
NSString * const NSFileHFSCreatorCode = @"NSFileHFSCreatorCode";
NSString * const NSFileHFSTypeCode = @"NSFileHFSTypeCode";
NSString * const NSFileImmutable = @"NSFileImmutable";
NSString * const NSFileModificationDate = @"NSFileModificationDate";
NSString * const NSFileOwnerAccountID = @"NSFileOwnerAccountID";
NSString * const NSFileOwnerAccountName = @"NSFileOwnerAccountName";
NSString * const NSFilePosixPermissions = @"NSFilePosixPermissions";
NSString * const NSFileReferenceCount = @"NSFileReferenceCount";
NSString * const NSFileSize = @"NSFileSize";
NSString * const NSFileSystemFileNumber = @"NSFileSystemFileNumber";
NSString * const NSFileSystemFreeNodes = @"NSFileSystemFreeNodes";
NSString * const NSFileSystemFreeSize = @"NSFileSystemFreeSize";
NSString * const NSFileSystemNodes = @"NSFileSystemNodes";
NSString * const NSFileSystemNumber = @"NSFileSystemNumber";
NSString * const NSFileSystemSize = @"NSFileSystemSize";
NSString * const NSFileType = @"NSFileType";
NSString * const NSFileTypeBlockSpecial = @"NSFileTypeBlockSpecial";
NSString * const NSFileTypeCharacterSpecial = @"NSFileTypeCharacterSpecial";
NSString * const NSFileTypeDirectory = @"NSFileTypeDirectory";
NSString * const NSFileTypeFifo = @"NSFileTypeFifo";
NSString * const NSFileTypeRegular = @"NSFileTypeRegular";
NSString * const NSFileTypeSocket = @"NSFileTypeSocket";
NSString * const NSFileTypeSymbolicLink = @"NSFileTypeSymbolicLink";
NSString * const NSFileTypeUnknown = @"NSFileTypeUnknown";


