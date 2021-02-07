/**
   NSFileManager.h

   Copyright (C) 1997,1999-2005 Free Software Foundation, Inc.

   Author: Mircea Oancea <mircea@jupiter.elcom.pub.ro>
   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: Feb 1997

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


<chapter>
  <heading>File management</heading>
  <section>
    <heading>Path handling</heading>
    <p>The rules for path handling depend on the value in the
    <code>GNUSTEP_PATH_HANDLING</code> environment variable and,
    to some extent, on the platform on which the program is running.<br />
    The understood values of GNUSTEP_PATH_HANDLING are <em>unix</em>
    and <em>windows</em>.  If GNUSTEP_PATH_HANDLING is any other value
    (or has not been set), GNUstep interprets this as meaning
    it should try to <em>do-the-right-thing</em><br />
    In the default mode of operation the system is very tolerant
    of paths and allows you to work with both unix and windows
    style paths.  The consequences of this are apparent in the 
    path handling methods of [NSString] rather than in [NSFileManager].
    </p>
    <subsect>
      <heading>unix</heading>
      <p>On all Unix platforms, Path components are separated by slashes
      and file names may contain any character other than slash.<br />
      The file names . and .. are special cases meaning current directory
      and the parent of the current directory respectively.<br />
      Multiple adjacent slash characters are treated as a single separator.
      </p>
      Here are various examples:
      <deflist>
	<term>/</term>
	<desc>An absolute path to the root directory. 
	</desc>
	<term>/etc/motd</term>
	<desc>An absolute path to the file named <em>motd</em>
	in the subdirectory <em>etc</em> of the root directory. 
	</desc>
	<term>..</term>
	<desc>A relative path to the parent of the current directory. 
	</desc>
	<term>program.m</term>
	<desc>A relative path to the file <em>program.m</em>
	in the current directory. 
	</desc>
	<term>Source/program.m</term>
	<desc>A relative path to the file <em>program.m</em> in the
	subdirectory <em>Source</em> of the current directory. 
	</desc>
	<term>../GNUmakefile</term>
	<desc>A relative path to the file <em>GNUmakefile</em>
	in the directory above the current directory.
	</desc>
      </deflist>
    </subsect>
    <subsect>
      <heading>windows</heading>
      <p>On Microsoft Windows the native paths may be either UNC
      or drive-relative, so GNUstep supports both.<br />
      Either or both slash (/) and backslash (\) may be used as
      separators for path components in either type of name.<br />
      UNC paths follow the general form //host/share/path/file,
      but must at least contain the host and share parts,
      i.e. //host/share is a UNC path, but //host is <em>not</em><br />
      Drive-relative names consist of an optional drive specifier
      (consisting of a single letter followed by a single colon)
      followed by an absolute or relative path.<br />
      In both forms, the names . and .. are refer to the current
      directory and the parent directory as in unix paths.
      </p>
      Here are various examples:
      <deflist>
	<term>//host/share/file</term>
	<desc>An absolute UNC path to a file called <em>file</em>
	in the top directory of the export point share on host.
	</desc>
	<term>C:</term>
	<desc>A relative path to the current directory on drive C.
	</desc>
	<term>C:program.m</term>
	<desc>A relative path to the file <em>program.m</em> on drive C.
	</desc>
	<term>C:\program.m</term>
	<desc>An absolute path to the file <em>program.m</em>
	in the top level directory on drive C.
	</desc>
	<term>/Source\program.m</term>
	<desc>A drive-relative path to <em>program.m</em> in the directory
	<em>Source</em> on the current drive.
	</desc>
	<term>\\name</term>
	<desc>A drive-relative path to <em>name</em> in the top level directory
	on the current drive.  The '\\' is treated as a single backslash as
	this is not a UNC name (there must be both a host and a share part in
	a UNC name).
	</desc>
      </deflist>
    </subsect>
    <subsect>
      <heading>gnustep</heading>
      <p>In the default mode, GNUstep handles both unix and windows paths so
      it treats both slash (/) and backslash (\) as separators and understands
      the windows UNC and drive relative path roots.<br />
      However, it treats any path beginning with a slash (/) as an absolute
      path <em>if running on a unix system</em>.
      </p>
    </subsect>
    <subsect>
      <heading>Portability</heading>
      <p>Attempting to pass absolute paths between applications working on
      different systems is fraught with difficulty ... just don't do it.<br />
      Where paths need to be passed around (eg. in property lists or archives)
      you should pass relative paths and use a standard mechanism to construct
      an absolute path in the receiving application, for instance, appending
      the relative path to the home directory of a user.
      </p>
      Even using relative paths you should take care ...
      <list>
        <item>Use only the slash (/) as a path separator, not backslash (\).
	</item>
        <item>Never use a backslash (\) in a file name.
	</item>
        <item>Avoid colons in file names.
	</item>
        <item>Use no more than three letters in a path extension.
	</item>
      </list>
      Remember that, while GNUstep will manipulate both windows and unix
      paths, any path actually used to reference a file or directory
      must be valid on the local system.
    </subsect>
    <subsect>
      <heading>Tilde substitution</heading>
      <p>GNUstep handles substitution of tilde (~) as follows:<br />
      If a path is just ~ or begins ~/ then the value returned by
      NSHomeDirectory() is substituted for the tilde.<br />
      If a path is of the form ~name or begins with a string like ~name/
      then name is used as the argument to NSHomeDirectoryForUser() and
      the return value from that method (if non-nil) is used to replace
      the tilde.
      </p>
    </subsect>
  </section>
</chapter>
*/

#ifndef __NSFileManager_h_GNUSTEP_BASE_INCLUDE
#define __NSFileManager_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
#import	<Foundation/NSDictionary.h>
#import	<Foundation/NSEnumerator.h>
#import <Foundation/NSPathUtilities.h>
#if	defined(__cplusplus)
extern "C" {
#endif

@class NSNumber;
@class NSString;
@class NSData;
@class NSDate;
@class NSArray;
@class NSMutableArray;
@class NSEnumerator;
@class NSDirectoryEnumerator;
@class NSError;
@class NSURL;

@protocol NSFileManagerDelegate;

/* MacOS-X defines OSType as a 32bit unsigned integer.
 */
#ifndef OSTYPE_DECLARED
typedef	uint32_t	OSType;
#define OSTYPE_DECLARED
#endif

DEFINE_BLOCK_TYPE(GSDirEnumErrorHandler, BOOL, NSURL*, NSError*);
  
enum _NSDirectoryEnumerationOptions
  {
    NSDirectoryEnumerationSkipsSubdirectoryDescendants = 1L << 0,
    NSDirectoryEnumerationSkipsPackageDescendants = 1L << 1,
    NSDirectoryEnumerationSkipsHiddenFiles = 1L << 2
  };
typedef NSUInteger NSDirectoryEnumerationOptions; 
  
GS_EXPORT_CLASS
@interface NSFileManager : NSObject
{
#if	GS_EXPOSE(NSFileManager)
@private
  id<NSFileManagerDelegate> _delegate;
  NSString	*_lastError;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/**
 * Returns a shared default file manager which may be used throughout an
 * application.
 */
+ (NSFileManager*) defaultManager;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property (assign) id<NSFileManagerDelegate> delegate;
#else
- (id<NSFileManagerDelegate>) delegate;
- (void) setDelegate: (id<NSFileManagerDelegate>)delegate;
#endif
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSDictionary *) attributesOfItemAtPath: (NSString*)path
				    error: (NSError**)error;

/**
 * Copies the item specified by the src path to the 
 * location specified by the dst path. If the src is a directory,
 * it is copied recursively with all of its contents.<br />
 * Errors are returned in the error variable.
 * Returns YES on success, NO otherwise.
 */
- (BOOL) copyItemAtPath: (NSString*)src
		 toPath: (NSString*)dst
		  error: (NSError**)error;
/**
 * Moves a file or directory specified by src to 
 * its destination specified by dst, errors are
 * returned in error.<br />
 * Returns YES on success, NO otherwise.
 */
- (BOOL) moveItemAtPath: (NSString*)src
		 toPath: (NSString*)dst
		  error: (NSError**)error;

/**
 * Removes the file or directory specified by the path
 * to be removed. If the path points to a directory,
 * the directory is deleted recursively.<br />
 * Returns YES on success, otherwise NO.
 */
- (BOOL) removeItemAtPath: (NSString*)path
                    error: (NSError**)error;

/**
 * Copies the a file or directory specified by the src URL to the 
 * location specified by the dst URL. If the src is a directory,
 * it is copied recursively with all of its contents.<br />
 * Errors are returned in the error variable.
 * Returns YES on success, NO otherwise.
 */
- (BOOL) copyItemAtURL: (NSURL*)src
		 toURL: (NSURL*)dst
		 error: (NSError**)error;

/**
 * Moves a file or directory specified by src to 
 * its destination specified by dst, errors are
 * returned in error.<br />
 * Returns YES on success, NO otherwise.
 */
- (BOOL) moveItemAtURL: (NSURL*)src
		 toURL: (NSURL*)dst
		 error: (NSError**)error;
/**
 * Removes the file or directory specified by the url
 * to be removed. If the url points to a directory,
 * the directory is deleted recursively.<br />
 * Returns YES on success, otherwise NO.
 */
- (BOOL) removeItemAtURL: (NSURL*)url
                   error: (NSError**)error;
/**
 * Creates a symbolic link at the path
 * that point to the destination path.<br />
 * Returns YES on success, otherwise NO.
 */
- (BOOL) createSymbolicLinkAtPath: (NSString*)path
              withDestinationPath: (NSString*)destPath
                            error: (NSError**)error;
#endif

/**
 * Changes the current directory used for all subsequent operations.<br />
 * All non-absolute paths are interpreted relative to this directory.<br />
 * The current directory is set on a per-task basis, so the current
 * directory for other file manager instances will also be changed
 * by this method.
 */
- (BOOL) changeCurrentDirectoryPath: (NSString*)path;
- (BOOL) changeFileAttributes: (NSDictionary*)attributes
		       atPath: (NSString*)path;
- (NSArray*) componentsToDisplayForPath: (NSString*)path;
- (NSData*) contentsAtPath: (NSString*)path;
- (BOOL) contentsEqualAtPath: (NSString*)path1
		     andPath: (NSString*)path2;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/**
 * Returns an array of NSURL of the contents of the specified directory. <br />
 * The listing is shallow and does not recurse into subdirectories.
 * The special files '.' and '..' are excluded but it can return
 * hidden files.<br />
 * The only mask option supported is
 * NSDirectoryEnumerationSkipsHiddenFiles.<br />
 * The current implementation handles only files and property keys are ignored.
 */
- (NSArray*) contentsOfDirectoryAtURL: (NSURL*)url
           includingPropertiesForKeys: (NSArray*)keys
                              options: (NSDirectoryEnumerationOptions)mask
                                error: (NSError **)error;

/**
 * Locates and, optionally, creates the specified common directory in
 * domain
 */
- (NSURL *)URLForDirectory: (NSSearchPathDirectory)directory 
                  inDomain: (NSSearchPathDomainMask)domain 
         appropriateForURL: (NSURL *)url 
                    create: (BOOL)shouldCreate 
                     error: (NSError **)error;

/**
 * Enumerate over the contents of a directory.
 */
- (NSDirectoryEnumerator *)enumeratorAtURL: (NSURL *)url
                includingPropertiesForKeys: (NSArray *)keys 
                                   options: (NSDirectoryEnumerationOptions)mask 
                              errorHandler: (GSDirEnumErrorHandler)handler;
#endif
  
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/**
 * Returns an array of NSStrings of the contents of the
 * specified directory.<br />
 * The listing does <strong>not</strong> recursively list subdirectories.<br />
 * The special files '.' and '..' are not listed.<br />
 * Indicates an error by returning nil (eg. if path is not a directory or
 * it can't be read for some reason).
 */
- (NSArray*) contentsOfDirectoryAtPath: (NSString*)path error: (NSError**)error;

- (NSDictionary*) attributesOfFileSystemForPath: (NSString*)path
                                          error: (NSError**)error;
#endif

- (BOOL) copyPath: (NSString*)source
	   toPath: (NSString*)destination
	  handler: (id)handler;
- (BOOL) createDirectoryAtPath: (NSString *)path
   withIntermediateDirectories: (BOOL)flag
		    attributes: (NSDictionary *)attributes
                         error: (NSError **) error;
- (BOOL) createDirectoryAtURL: (NSURL *)url
  withIntermediateDirectories: (BOOL)flag
		   attributes: (NSDictionary *)attributes
                        error: (NSError **) error;
- (BOOL) createDirectoryAtPath: (NSString*)path
		    attributes: (NSDictionary*)attributes;
- (BOOL) createFileAtPath: (NSString*)path
		 contents: (NSData*)contents
	       attributes: (NSDictionary*)attributes;
- (BOOL) createSymbolicLinkAtPath: (NSString*)path
		      pathContent: (NSString*)otherPath;
- (NSString*) currentDirectoryPath;
- (NSArray*) directoryContentsAtPath: (NSString*)path;
- (NSString*) displayNameAtPath: (NSString*)path;
/**
 * <p>Returns an enumerator which can be used to return each item with
 * the directory at path in turn.
 * </p>
 * <p>The enumeration is recursive ... following all nested subdirectories.
 * </p>
 * <p>The order in which directory contents are enumerated is undefined,
 * and in the current implementation the natural order of the underlying
 * filesystem is used.
 * </p>
 */
- (NSDirectoryEnumerator*) enumeratorAtPath: (NSString*)path;

/** Returns the attributes dictionary for the file at the specified path.
 * If that file is a symbolic link, the flag determines whether the attributes
 * returned are those of the link or those of the destination file.
 */
- (NSDictionary*) fileAttributesAtPath: (NSString*)path
			  traverseLink: (BOOL)flag;

/**
 * Returns YES if a file (or directory etc) exists at the specified path.
 */
- (BOOL) fileExistsAtPath: (NSString*)path;

/**
 * Returns YES if a file (or directory etc) exists at the specified path.<br />
 * If the isDirectory argument is not a nul pointer, stores a flag
 * in the location it points to, indicating whether the file is a
 * directory or not.<br />
 */
- (BOOL) fileExistsAtPath: (NSString*)path isDirectory: (BOOL*)isDirectory;
- (NSDictionary*) fileSystemAttributesAtPath: (NSString*)path;

/**
 * Convert from OpenStep internal string format to a string in
 * the local filesystem format, suitable for passing to system functions.<br />
 * This representation may vary between filesystems.<br />
 * Converts the standard path separator ('/') and path extension ('.')
 * characters to the local representation if necessary.<br />
 * On mingw32 systems, the filesystem representation is 16-bit unicode and is
 * expected to be used in conjunction with the variants of system calls which
 * work with unicode strings.<br />
 * Raises an exception if the character conversion is not possible.
 */
- (const GSNativeChar*) fileSystemRepresentationWithPath: (NSString*)path;

- (BOOL) isExecutableFileAtPath: (NSString*)path;
- (BOOL) isDeletableFileAtPath: (NSString*)path;
- (BOOL) isReadableFileAtPath: (NSString*)path;
- (BOOL) isWritableFileAtPath: (NSString*)path;
- (BOOL) linkPath: (NSString*)source
	   toPath: (NSString*)destination
	  handler: (id)handler;
- (BOOL) movePath: (NSString*)source
	   toPath: (NSString*)destination 
	  handler: (id)handler;
- (NSString*) pathContentOfSymbolicLinkAtPath: (NSString*)path;

/**
 * Removes the file or directory at path, using a
 * handler object which should respond to
 * [NSObject(NSFileManagerHandler)-fileManager:willProcessPath:] and
 * [NSObject(NSFileManagerHandler)-fileManager:shouldProceedAfterError:]
 * messages.
 */
- (BOOL) removeFileAtPath: (NSString*)path
		  handler: (id)handler;

/**
 * Convert to OpenStep internal string format from a string in
 * the local filesystem format, as returned by system functions.<br />
 * This representation may vary between filesystems.<br />
 * The GNUstep version of this method currently does not bother to change
 * any path separator and extension characters to the standard values
 * ('/' and '.' respectively) as the path handling methods of [NSString]
 * should be able to handle native format strings.<br />
 * On mingw32 systems, the filesystem representation is 16-bit unicode and
 * is expected to have come from the variant of a system call which works
 * with unicode strings.
 */
- (NSString*) stringWithFileSystemRepresentation: (const GSNativeChar*)string
					  length: (NSUInteger)len;

- (NSArray*) subpathsAtPath: (NSString*)path;

@end /* NSFileManager */

/**
 * An informal protocol to which handler objects should conform
 * if they wish to deal with copy and move operations performed
 * by NSFileManager.
 */

@interface NSObject (NSFileManagerHandler)
/**
 * <p>When an error occurs during a copy or move operation, the file manager
 * will send this message to the handler, and will use the return value to
 * determine whether the operation should proceed.  If the method returns
 * YES then the operation will proceed after the error, if it returns NO
 * then it will be aborted.
 * </p>
 * <p>If the handler does not implement this method it will be treated as
 * if it returns NO.
 * </p>
 * The error dictionary contains the following
 * <list>
 *   <item><strong>"Error"</strong>
 *     contains a description of the error.
 *   </item>
 *   <item><strong>"Path"</strong>
 *     contains the path that is being processed when
 *     an error occurred.   If an error occurs during an
 *     operation involving two files, like copying, and
 *     it is not clear which file triggers the error it
 *     will default to the source file.
 *   </item>          
 *   <item><strong>"FromPath"</strong>
 *     (Optional)  contains the path involved in reading.
 *   </item>
 *   <item><strong>"ToPath"</strong>
 *     (Optional)  contains the path involved in writing.
 *   </item>
 * </list>
 *
 * <p>Note that the <code>FromPath</code> is a GNUstep extension.
 * </p>
 * <p>Also the <code>FromPath</code> and <code>ToPath</code> are filled
 * in when appropriate.  So when copying a file they will typically
 * both have a value and when reading only <code>FromPath</code>.
 * </p>
 */
- (BOOL) fileManager: (NSFileManager*)fileManager
  shouldProceedAfterError: (NSDictionary*)errorDictionary;

/**
 * The file manager sends this method to the handler immediately before
 * performing part of a directory move or copy operation.  This provides
 * the handler object with information it can use in the event of an
 * error, to decide whether processing should proceed after the error.
 */
- (void) fileManager: (NSFileManager*)fileManager
     willProcessPath: (NSString*)path;
@end

/**
 *  <p>This is a subclass of <code>NSEnumerator</code> which provides a full
 *  listing of all the files beneath a directory and its subdirectories.
 *  Instances can be obtained through [NSFileManager-enumeratorAtPath:].
 *  </p>
 *
 *  <p>This implementation is optimized and performance should be comparable
 *  to the speed of standard Unix tools for large directories.</p>
 *
 *  <p>The order in which directory contents are enumerated is undefined,
 *  and in the current implementation the natural order of the underlying
 *  filesystem is used.</p>
 */
GS_EXPORT_CLASS
@interface NSDirectoryEnumerator : NSEnumerator
{
#if	GS_EXPOSE(NSDirectoryEnumerator)
@private
  void *_stack; /* GSIArray */
  NSString *_topPath;
  NSString *_currentFilePath;
  NSFileManager *_mgr;
  GSDirEnumErrorHandler _errorHandler; 
  struct _NSDirectoryEnumeratorFlags      // tag for objc++ w/gcc 4.6 
  {
    BOOL isRecursive: 1;
    BOOL isFollowing: 1;
    BOOL justContents: 1;
    BOOL skipHidden: 1;
  } _flags;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
- (NSDictionary*) directoryAttributes;
- (NSDictionary*) fileAttributes;
- (void) skipDescendents;

@end /* NSDirectoryEnumerator */

/* File Attributes */
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileAppendOnly;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileCreationDate;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileDeviceIdentifier;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileExtensionHidden;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileGroupOwnerAccountID;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileGroupOwnerAccountName;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileHFSCreatorCode;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileHFSTypeCode;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileImmutable;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileModificationDate;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileOwnerAccountID;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileOwnerAccountName;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFilePosixPermissions;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileReferenceCount;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSize;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemFileNumber;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemNumber;
/** File attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileType;

/* File Types */

/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeDirectory;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeRegular;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeSymbolicLink;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeSocket;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeFifo;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeCharacterSpecial;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeBlockSpecial;
/** Possible value for '<code>NSFileType</code>' key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileTypeUnknown;

/* FileSystem Attributes */

/** File system attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemSize;
/** File system attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemFreeSize;
/** File system attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemNodes;
/** File system attribute key in dictionary returned by
    [NSFileManager-fileAttributesAtPath:traverseLink:]. */
GS_EXPORT NSString* const NSFileSystemFreeNodes;

/* Easy access to attributes in a dictionary */

@interface NSDictionary(NSFileAttributes)
- (NSDate*) fileCreationDate;
- (BOOL) fileExtensionHidden;
- (OSType) fileHFSCreatorCode;
- (OSType) fileHFSTypeCode;
- (BOOL) fileIsAppendOnly;
- (BOOL) fileIsImmutable;
- (unsigned long long) fileSize;
- (NSString*) fileType;
- (NSNumber*) fileOwnerAccountID;
- (NSString*) fileOwnerAccountName;
- (NSNumber*) fileGroupOwnerAccountID;
- (NSString*) fileGroupOwnerAccountName;
- (NSDate*) fileModificationDate;
- (NSUInteger) filePosixPermissions;
- (NSUInteger) fileSystemNumber;
- (NSUInteger) fileSystemFileNumber;
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST)

@protocol NSFileManagerDelegate <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSFileManagerDelegate)
#endif
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldCopyItemAtPath: (NSString *)srcPath
                toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldCopyItemAtURL: (NSURL *)srcURL
                toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
        copyingItemAtPath: (NSString *)srcPath
                   toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
         copyingItemAtURL: (NSURL *)srcURL
                    toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldMoveItemAtPath: (NSString *)srcPath
                toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldMoveItemAtURL: (NSURL *)srcURL
                toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
         movingItemAtPath: (NSString *)srcPath
                   toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
          movingItemAtURL: (NSURL *)srcURL
                    toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldLinkItemAtPath: (NSString *)srcPath
                toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldLinkItemAtURL: (NSURL *)srcURL
                toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
        linkingItemAtPath: (NSString *)srcPath
                   toPath: (NSString *)dstPath;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
         linkingItemAtURL: (NSURL *)srcURL
                    toURL: (NSURL *)dstURL;

- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldRemoveItemAtPath: (NSString *)path;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldRemoveItemAtURL: (NSURL *)URL;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
       removingItemAtPath: (NSString *)path;
- (BOOL)fileManager: (NSFileManager *)fileManager
  shouldProceedAfterError: (NSError *)error
        removingItemAtURL: (NSURL *)URL;
@end

#endif

#if	defined(__cplusplus)
}
#endif

#endif
#endif /* __NSFileManager_h_GNUSTEP_BASE_INCLUDE */
