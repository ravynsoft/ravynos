/* NSURL.h - Class NSURL
   Copyright (C) 1999 Free Software Foundation, Inc.
   
   Written by: 	Manuel Guesdon <mguesdon@sbuilders.com>
   Date:	Jan 1999
   
   This file is part of the GNUstep Library.
   
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
*/

#ifndef __NSURL_h_GNUSTEP_BASE_INCLUDE
#define __NSURL_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSURLHandle.h>
#import <Foundation/NSRange.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

@class NSError;
@class NSNumber;
@class NSString;
@class NSDictionary;
@class NSArray;

/**
 *  URL scheme constant for use with [NSURL-initWithScheme:host:path:].
 */
GS_EXPORT NSString* const NSURLFileScheme;

/** URL Bookmark Resolution Options **/
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
typedef NSUInteger NSURLBookmarkResolutionOptions;
enum
{
  NSURLBookmarkResolutionWithoutUI = (1 << 8),
  NSURLBookmarkResolutionWithoutMounting = (1 << 9),
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
  NSURLBookmarkResolutionWithSecurityScope = (1 << 10)
#endif
};
#endif

/**
 * This class permits manipulation of URLs and the resources to which they
 * refer.  They can be used to represent absolute URLs or relative URLs
 * which are based upon an absolute URL.  The relevant RFCs describing
 * how a URL is formatted, and what is legal in a URL are -
 * 1808, 1738, and 2396.<br />
 * Handling of the underlying resources is carried out by NSURLHandle
 * objects, but NSURL provides a simplified API wrapping these objects.
 */
GS_EXPORT_CLASS
@interface NSURL: NSObject <NSCoding, NSCopying, NSURLHandleClient>
{
#if	GS_EXPOSE(NSURL)
@private
  NSString	*_urlString;
  NSURL		*_baseURL;
  void		*_clients;
  void		*_data;
#endif
}
 
/**
 * Create and return a file URL with the supplied path.<br />
 * The value of aPath must be a valid filesystem path.<br />
 * Calls -initFileURLWithPath: which escapes characters in the
 * path where necessary.
 */
+ (instancetype) fileURLWithPath: (NSString*)aPath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Creates a file URL using a path built from components.
 */
+ (instancetype) fileURLWithPathComponents: (NSArray*)components;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
+ (instancetype) fileURLWithPath: (NSString*)aPath isDirectory: (BOOL)isDir;
#endif
/**
 * Create and return a URL with the supplied string, which should
 * be a string (containing percent escape codes where necessary)
 * conforming to the description (in RFC2396) of an absolute URL.<br />
 * Calls -initWithString:
 */
+ (instancetype) URLWithString: (NSString*)aUrlString;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
+ (instancetype) URLByResolvingAliasFileAtURL: (NSURL*)url
                                      options: (NSURLBookmarkResolutionOptions)options
                                        error: (NSError**)error;
#endif

/**
 * Create and return a URL with the supplied string, which should
 * be a string (containing percent escape codes where necessary)
 * conforming to the description (in RFC2396) of a relative URL.<br />
 * Calls -initWithString:relativeToURL:
 */
+ (instancetype) URLWithString: (NSString*)aUrlString
                 relativeToURL: (NSURL*)aBaseUrl;

/**
 * Initialise as a file URL with the specified path (which must
 * be a valid path on the local filesystem).<br />
 * Raises NSInvalidArgumentException if aPath is nil.<br />
 * Converts relative paths to absolute ones.<br />
 * Appends a trailing slash to the path when necessary if it
 * specifies a directory.<br />
 * Calls -initWithScheme:host:path:
 */
- (instancetype) initFileURLWithPath: (NSString*)aPath;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
/**
 * Initialise as a file URL with the specified path (which must
 * be a valid path on the local filesystem).<br />
 * Raises NSInvalidArgumentException if aPath is nil.<br />
 * Converts relative paths to absolute ones.<br />
 * Appends a trailing slash to the path when necessary if it
 * specifies a directory.<br />
 * Calls -initWithScheme:host:path:
 */
- (instancetype) initFileURLWithPath: (NSString*)aPath
                         isDirectory: (BOOL)isDir;
#endif

/**
 * Initialise by building a URL string from the supplied parameters
 * and calling -initWithString:relativeToURL:<br />
 * This method adds percent escapes to aPath if it contains characters
 * which need escaping.<br />
 * Accepts RFC2732 style IPv6 host addresses either with or without the
 * enclosing square brackets (MacOS-X at least up to version 10.5 does
 * not handle these correctly, but GNUstep does).<br />
 * Permits the 'aHost' part to contain 'username:password@host:port' or
 * 'host:port' in addition to a simple host name or address.
 */
- (instancetype) initWithScheme: (NSString*)aScheme
                           host: (NSString*)aHost
                           path: (NSString*)aPath;

/**
 * Initialise as an absolute URL.<br />
 * Calls -initWithString:relativeToURL:
 */
- (instancetype) initWithString: (NSString*)aUrlString;

/** <init />
 * Initialised using aUrlString and aBaseUrl.  The value of aBaseUrl
 * may be nil, but aUrlString must be non-nil.<br />
 * Accepts RFC2732 style IPv6 host addresses.<br />
 * Parses a string wihthout a scheme as a simple path.<br />
 * Parses an empty string as an empty path.<br />
 * If the string cannot be parsed the method returns nil.
 */
- (instancetype) initWithString: (NSString*)aUrlString
                  relativeToURL: (NSURL*)aBaseUrl;

#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, getter=isFileURL) BOOL fileURL;
#else
- (BOOL) isFileURL;
#endif

/**
 * Returns the full string describing the receiver resolved against its base.
 */
- (NSString*) absoluteString;

/**
 * If the receiver is an absolute URL, returns self.  Otherwise returns an
 * absolute URL referring to the same resource as the receiver.
 */
- (NSURL*) absoluteURL;

/**
 * If the receiver is a relative URL, returns its base URL.<br />
 * Otherwise, returns nil.
 */
- (NSURL*) baseURL;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Attempts to load from the specified URL and provides an error
 * response if the data is unrachable.<br />
 * Returns YES on success, NO on failure.
 */
- (BOOL) checkResourceIsReachableAndReturnError: (NSError **)error;
#endif

/**
 * Returns the fragment portion of the receiver or nil if there is no
 * fragment supplied in the URL.<br />
 * The fragment is everything in the original URL string after a '#'<br />
 * File URLs do not have fragments.
 */
- (NSString*) fragment;

/**
 * Returns the host portion of the receiver or nil if there is no
 * host supplied in the URL.<br />
 * Percent escape sequences in the user string are translated and the string
 * treated as UTF8.<br />
 * Returns IPv6 addresses <em>without</em> the enclosing square brackets
 * required (by RFC2732) in URL strings.
 */
- (NSString*) host;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Returns the last (rightmost) path component of the receiver.
 */
- (NSString*) lastPathComponent;
#endif

/**
 * Loads resource data for the specified client.
 * <p>
 *   If shouldUseCache is YES then an attempt
 *   will be made to locate a cached NSURLHandle to provide the
 *   resource data, otherwise a new handle will be created and
 *   cached.
 * </p>
 * <p>
 *   If the handle does not have the data available, it will be
 *   asked to load the data in the background by calling its
 *   loadInBackground  method.
 * </p>
 * <p>
 *   The specified client (if non-nil) will be set up to receive
 *   notifications of the progress of the background load process.
 * </p>
 * <p>
 *   The processes current run loop must be run in order for the
 *   background load operation to operate!
 * </p>
 */
- (void) loadResourceDataNotifyingClient: (id)client
			      usingCache: (BOOL)shouldUseCache;

/**
 * Returns the parameter portion of the receiver or nil if there is no
 * parameter supplied in the URL.<br />
 * The parameters are everything in the original URL string after a ';'
 * but before the query.<br />
 * File URLs do not have parameters.
 */
- (NSString*) parameterString;

/**
 * Returns the password portion of the receiver or nil if there is no
 * password supplied in the URL.<br />
 * Percent escape sequences in the user string are translated and the string
 * treated as UTF8 in GNUstep but this appears to be broken in MacOS-X.<br />
 * NB. because of its security implications it is recommended that you
 * do not use URLs with users and passwords unless necessary.
 */
- (NSString*) password;

/**
 * Returns the path portion of the receiver.<br />
 * Replaces percent escapes with unescaped values, interpreting non-ascii
 * character sequences as UTF8.<br />
 * NB. This does not conform strictly to the RFCs, in that it includes a
 * leading slash ('/') character (whereas the path part of a URL strictly
 * should not) and the interpretation of non-ascii character is (strictly
 * speaking) undefined.<br />
 * Also, this breaks strict conformance in that a URL of file scheme is
 * treated as having a path (contrary to RFCs)
 */
- (NSString*) path;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Returns thepath components of the receiver.<br />
 * See [NSString-pathComponents].
 */
- (NSArray*) pathComponents;

/** Returns the file extension (text after the rightmost dot in the path)
 * of the receiver.<br />
 * see [NSString-pathExtension].
 */
- (NSString*) pathExtension;
#endif

/**
 * Returns the port portion of the receiver or nil if there is no
 * port supplied in the URL.<br />
 * Percent escape sequences in the user string are translated in GNUstep
 * but this appears to be broken in MacOS-X.
 */
- (NSNumber*) port;

/**
 * Asks a URL handle to return the property for the specified key and
 * returns the result.
 */
- (id) propertyForKey: (NSString*)propertyKey;

/**
 * Returns the query portion of the receiver or nil if there is no
 * query supplied in the URL.<br />
 * The query is everything in the original URL string after a '?'
 * but before the fragment.<br />
 * File URLs do not have queries.
 */
- (NSString*) query;

/**
 * Returns the path of the receiver, without taking any base URL into account.
 * If the receiver is an absolute URL, -relativePath is the same as -path.<br />
 * Returns nil if there is no path specified for the URL.
 */
- (NSString*) relativePath;

/**
 * Returns the relative portion of the URL string.  If the receiver is not
 * a relative URL, this returns the same as absoluteString.
 */
- (NSString*) relativeString;

/**
 * Loads the resource data for the represented URL and returns the result.
 * The shouldUseCache flag determines whether data previously retrieved by
 * an existing NSURLHandle can be used to provide the data, or if it should
 * be refetched.
 */
- (NSData*) resourceDataUsingCache: (BOOL)shouldUseCache;

/**
 * Returns the resource specifier of the URL ... the part which lies
 * after the scheme.
 */
- (NSString*) resourceSpecifier;

/**
 * Returns the scheme of the receiver.
 */
- (NSString*) scheme;

/**
 * Calls [NSURLHandle-writeProperty:forKey:] to set the named property.
 */
- (BOOL) setProperty: (id)property
	      forKey: (NSString*)propertyKey;

/**
 * Calls [NSURLHandle-writeData:] to write the specified data object
 * to the resource identified by the receiver URL.<br />
 * Returns the result.
 */
- (BOOL) setResourceData: (NSData*)data;

/**
 * Returns a URL with '/./' and '/../' sequences resolved etc.
 */
- (NSURL*) standardizedURL;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
/** Returns a URL formed by adding a path component to the path of the
 * receiver.<br />
 * See [NSString-stringByAppendingPathComponent:].
 */
- (NSURL*) URLByAppendingPathComponent: (NSString*)pathComponent;

/** Returns a URL formed by adding a path extension to the path of the
 * receiver.<br />
 * See [NSString-stringByAppendingPathExtension:].
 */
- (NSURL*) URLByAppendingPathExtension: (NSString*)pathExtension;

/** Returns a URL formed by removing a path component from the path of the
 * receiver.<br />
 * See [NSString-stringByDeletingLastPathComponent].
 */
- (NSURL*) URLByDeletingLastPathComponent;

/** Returns a URL formed by removing an extension from the path of the
 * receiver.<br />
 * See [NSString-stringByDeletingPathExtension].
 */
- (NSURL*) URLByDeletingPathExtension;

/** Returns self unless the receiver is a file URL, in which case it returns
 * a URL formed by calling [NSString-stringByResolvingSymlinksInPath].
 */
- (NSURL*) URLByResolvingSymlinksInPath;

/** Returns self unless the receiver is a file URL, in which case it returns
 * a URL formed by calling [NSString-stringByStandardizingPath].
 */
- (NSURL*) URLByStandardizingPath;

#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST) 
/** Returns a URL formed by adding a path component to the path of the
 * receiver, along with a trailing slash if the component is designated a
 * directory.<br />
 * See [NSString-stringByAppendingPathComponent:].
 */
- (NSURL *) URLByAppendingPathComponent:(NSString *)pathComponent
                            isDirectory:(BOOL)isDirectory;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (BOOL) isFileReferenceURL;

- (NSURL *) fileReferenceURL;

- (NSURL *) filePathURL;

- (BOOL) getResourceValue: (id*)value 
                   forKey: (NSString *)key 
                    error: (NSError**)error;
#endif
/**
 * Returns an NSURLHandle instance which may be used to write data to the
 * resource represented by the receiver URL, or read data from it.<br />
 * The shouldUseCache flag indicates whether a cached handle may be returned
 * or a new one should be created.
 */
- (NSURLHandle*)URLHandleUsingCache: (BOOL)shouldUseCache;

/**
 * Returns the user portion of the receiver or nil if there is no
 * user supplied in the URL.<br />
 * Percent escape sequences in the user string are translated and
 * the whole is treated as UTF8 data.<br />
 * NB. because of its security implications it is recommended that you
 * do not use URLs with users and passwords unless necessary.
 */
- (NSString*) user;

@end

@interface NSObject (NSURLClient)

/** <override-dummy />
 * Some data has become available.  Note that this does not mean that all data
 * has become available, only that a chunk of data has arrived.
 */
- (void) URL: (NSURL*)sender
  resourceDataDidBecomeAvailable: (NSData*)newBytes;

/** <override-dummy />
 * Loading of resource data is complete.
 */
- (void) URLResourceDidFinishLoading: (NSURL*)sender;

/** <override-dummy />
 * Loading of resource data was cancelled by programmatic request
 * (not an error).
 */
- (void) URLResourceDidCancelLoading: (NSURL*)sender;

/** <override-dummy />
 * Loading of resource data has failed, for given human-readable reason.
 */
- (void) URL: (NSURL*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason;
@end

/** URL Resource Value Constants **/
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
GS_EXPORT NSString* const NSURLNameKey;
GS_EXPORT NSString* const NSURLLocalizedNameKey;
GS_EXPORT NSString* const NSURLIsRegularFileKey;
GS_EXPORT NSString* const NSURLIsDirectoryKey;
GS_EXPORT NSString* const NSURLIsSymbolicLinkKey;
GS_EXPORT NSString* const NSURLIsVolumeKey;
GS_EXPORT NSString* const NSURLIsPackageKey;
GS_EXPORT NSString* const NSURLIsSystemImmutableKey;
GS_EXPORT NSString* const NSURLIsUserImmutableKey;
GS_EXPORT NSString* const NSURLIsHiddenKey;
GS_EXPORT NSString* const NSURLHasHiddenExtensionKey;
GS_EXPORT NSString* const NSURLCreationDateKey;
GS_EXPORT NSString* const NSURLContentAccessDateKey;
GS_EXPORT NSString* const NSURLContentModificationDateKey;
GS_EXPORT NSString* const NSURLAttributeModificationDateKey;
GS_EXPORT NSString* const NSURLLinkCountKey;
GS_EXPORT NSString* const NSURLParentDirectoryURLKey;
GS_EXPORT NSString* const NSURLVolumeURLKey;
GS_EXPORT NSString* const NSURLTypeIdentifierKey;
GS_EXPORT NSString* const NSURLLocalizedTypeDescriptionKey;
GS_EXPORT NSString* const NSURLLabelNumberKey;
GS_EXPORT NSString* const NSURLLabelColorKey;
GS_EXPORT NSString* const NSURLLocalizedLabelKey;
GS_EXPORT NSString* const NSURLEffectiveIconKey;
GS_EXPORT NSString* const NSURLCustomIconKey;
GS_EXPORT NSString* const NSURLFileSizeKey;
GS_EXPORT NSString* const NSURLFileAllocatedSizeKey;
GS_EXPORT NSString* const NSURLIsAliasFileKey;
GS_EXPORT NSString* const NSURLVolumeLocalizedFormatDescriptionKey;
GS_EXPORT NSString* const NSURLVolumeTotalCapacityKey;
GS_EXPORT NSString* const NSURLVolumeAvailableCapacityKey;
GS_EXPORT NSString* const NSURLVolumeResourceCountKey;
GS_EXPORT NSString* const NSURLVolumeSupportsPersistentIDsKey;
GS_EXPORT NSString* const NSURLVolumeSupportsSymbolicLinksKey;
GS_EXPORT NSString* const NSURLVolumeSupportsHardLinksKey;
GS_EXPORT NSString* const NSURLVolumeSupportsJournalingKey;
GS_EXPORT NSString* const NSURLVolumeIsJournalingKey;
GS_EXPORT NSString* const NSURLVolumeSupportsSparseFilesKey;
GS_EXPORT NSString* const NSURLVolumeSupportsZeroRunsKey;
GS_EXPORT NSString* const NSURLVolumeSupportsCaseSensitiveNamesKey;
GS_EXPORT NSString* const NSURLVolumeSupportsCasePreservedNamesKey;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
GS_EXPORT NSString* const NSURLFileResourceIdentifierKey;
GS_EXPORT NSString* const NSURLVolumeIdentifierKey;
GS_EXPORT NSString* const NSURLPreferredIOBlockSizeKey;
GS_EXPORT NSString* const NSURLIsReadableKey;
GS_EXPORT NSString* const NSURLIsWritableKey;
GS_EXPORT NSString* const NSURLIsExecutableKey;
GS_EXPORT NSString* const NSURLFileSecurityKey;
GS_EXPORT NSString* const NSURLIsMountTriggerKey;
GS_EXPORT NSString* const NSURLFileResourceTypeKey;
GS_EXPORT NSString* const NSURLTotalFileSizeKey;
GS_EXPORT NSString* const NSURLTotalFileAllocatedSizeKey;
GS_EXPORT NSString* const NSURLVolumeSupportsRootDirectoryDatesKey;
GS_EXPORT NSString* const NSURLVolumeSupportsVolumeSizesKey;
GS_EXPORT NSString* const NSURLVolumeSupportsRenamingKey;
GS_EXPORT NSString* const NSURLVolumeSupportsAdvisoryFileLockingKey;
GS_EXPORT NSString* const NSURLVolumeSupportsExtendedSecurityKey;
GS_EXPORT NSString* const NSURLVolumeIsBrowsableKey;
GS_EXPORT NSString* const NSURLVolumeMaximumFileSizeKey;
GS_EXPORT NSString* const NSURLVolumeIsEjectableKey;
GS_EXPORT NSString* const NSURLVolumeIsRemovableKey;
GS_EXPORT NSString* const NSURLVolumeIsInternalKey;
GS_EXPORT NSString* const NSURLVolumeIsAutomountedKey;
GS_EXPORT NSString* const NSURLVolumeIsLocalKey;
GS_EXPORT NSString* const NSURLVolumeIsReadOnlyKey;
GS_EXPORT NSString* const NSURLVolumeCreationDateKey;
GS_EXPORT NSString* const NSURLVolumeURLForRemountingKey;
GS_EXPORT NSString* const NSURLVolumeUUIDStringKey;
GS_EXPORT NSString* const NSURLVolumeNameKey;
GS_EXPORT NSString* const NSURLVolumeLocalizedNameKey;
GS_EXPORT NSString* const NSURLIsUbiquitousItemKey;
GS_EXPORT NSString* const NSURLUbiquitousItemHasUnresolvedConflictsKey;
GS_EXPORT NSString* const NSURLUbiquitousItemIsDownloadingKey;
GS_EXPORT NSString* const NSURLUbiquitousItemIsUploadedKey;
GS_EXPORT NSString* const NSURLUbiquitousItemIsUploadingKey;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
GS_EXPORT NSString* const NSURLIsExcludedFromBackupKey;
GS_EXPORT NSString* const NSURLPathKey;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
GS_EXPORT NSString* const NSURLTagNamesKey;
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadingStatusKey;
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadingErrorKey;
GS_EXPORT NSString* const NSURLUbiquitousItemUploadingErrorKey;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
GS_EXPORT NSString* const NSURLGenerationIdentifierKey;
GS_EXPORT NSString* const NSURLDocumentIdentifierKey;
GS_EXPORT NSString* const NSURLAddedToDirectoryDateKey;
GS_EXPORT NSString* const NSURLQuarantinePropertiesKey;
GS_EXPORT NSString* const NSThumbnail1024x1024SizeKey;
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadRequestedKey;
GS_EXPORT NSString* const NSURLUbiquitousItemContainerDisplayNameKey;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_11, GS_API_LATEST)
GS_EXPORT NSString* const NSURLIsApplicationKey;
GS_EXPORT NSString* const NSURLApplicationIsScriptableKey;
#endif

/** Possible values for File Resource Type Key **/
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
GS_EXPORT NSString* const NSURLFileResourceTypeNamedPipe;
GS_EXPORT NSString* const NSURLFileResourceTypeCharacterSpecial;
GS_EXPORT NSString* const NSURLFileResourceTypeDirectory;
GS_EXPORT NSString* const NSURLFileResourceTypeBlockSpecial;
GS_EXPORT NSString* const NSURLFileResourceTypeRegular;
GS_EXPORT NSString* const NSURLFileResourceTypeSymbolicLink;
GS_EXPORT NSString* const NSURLFileResourceTypeSocket;
GS_EXPORT NSString* const NSURLFileResourceTypeUnknown;
#endif

/** Possible values for Ubiquitous Item Downloading Key **/
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadingStatusNotDownloaded;
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadingStatusDownloaded;
GS_EXPORT NSString* const NSURLUbiquitousItemDownloadingStatusCurrent;
#endif

#endif	/* GS_API_MACOSX */

#if     !NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import <GNUstepBase/NSURL+GNUstepBase.h>
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

GS_EXPORT_CLASS
@interface NSURLQueryItem : NSObject <NSCopying, NSCoding>
{
#if	GS_EXPOSE(NSURLQueryItem)
#endif
#if     GS_NONFRAGILE
#  if	defined(GS_NSURLQueryItem_IVARS)
@public
GS_NSURLQueryItem_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

// Creating query items.
+ (instancetype)queryItemWithName:(NSString *)name 
                            value:(NSString *)value;
- (instancetype)initWithName:(NSString *)name 
                       value:(NSString *)value;

// Reading a name and value from a query
- (NSString *) name;  
- (NSString *) value;
@end

#endif // OS_API_VERSION

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)

GS_EXPORT_CLASS
@interface NSURLComponents : NSObject <NSCopying>
{
#if	GS_EXPOSE(NSURLComponents)
#endif
#if     GS_NONFRAGILE
#  if	defined(GS_NSURLComponents_IVARS)
@public
GS_NSURLComponents_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
  // Creating URL components...
+ (instancetype) componentsWithString:(NSString *)URLString;
+ (instancetype) componentsWithURL:(NSURL *)url 
           resolvingAgainstBaseURL:(BOOL)resolve;
- (instancetype) init;
- (instancetype)initWithString:(NSString *)URLString;

- (instancetype)initWithURL:(NSURL *)url 
    resolvingAgainstBaseURL:(BOOL)resolve;

// Getting the URL
- (NSString *) string;
- (void) setString: (NSString *)urlString;
- (NSURL *) URL;
- (void) setURL: (NSURL *)url;
- (NSURL *)URLRelativeToURL: (NSURL *)baseURL;

// Accessing Components in Native Format
- (NSString *) fragment;
- (void) setFragment: (NSString *)fragment;
- (NSString *) host;
- (void) setHost: (NSString *)host;
- (NSString *) password;
- (void) setPassword: (NSString *)password;
- (NSString *) path;
- (void) setPath: (NSString *)path;
- (NSNumber *) port;
- (void) setPort: (NSNumber *)port;
- (NSString *) query;
- (void) setQuery: (NSString *)query;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
- (NSArray *) queryItems;
- (void) setQueryItems: (NSArray *)queryItems;
#endif
- (NSString *) scheme;
- (void) setScheme: (NSString *)scheme;
- (NSString *) user;
- (void) setUser: (NSString *)user;

// Accessing Components in PercentEncoded Format
- (NSString *) percentEncodedFragment; 
- (void) setPercentEncodedFragment: (NSString *)fragment;
- (NSString *) percentEncodedHost;
- (void) setPercentEncodedHost: (NSString *)host;
- (NSString *) percentEncodedPassword;
- (void) setPercentEncodedPassword: (NSString *)password;
- (NSString *) percentEncodedPath;
- (void) setPercentEncodedPath: (NSString *)path;
- (NSString *) percentEncodedQuery;
- (void) setPercentEncodedQuery: (NSString *)query;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
- (NSArray *) percentEncodedQueryItems;
- (void) setPercentEncodedQueryItems: (NSArray *)queryItems;
#endif
- (NSString *) percentEncodedUser;
- (void) setPercentEncodedUser: (NSString *)user;

// Locating components of the URL string representation
- (NSRange) rangeOfFragment;
- (NSRange) rangeOfHost;
- (NSRange) rangeOfPassword;
- (NSRange) rangeOfPath;
- (NSRange) rangeOfPort;
- (NSRange) rangeOfQuery;
- (NSRange) rangeOfScheme;
- (NSRange) rangeOfUser;
  
@end

#if defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* __NSURL_h_GNUSTEP_BASE_INCLUDE */




