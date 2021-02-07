/* 
   NSFileWrapper.h

   NSFileWrapper objects hold a file's contents in dynamic memory.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: Sept 1998

   This file is part of the GNUstep BASE Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSFileWrapper
#define _GNUstep_H_NSFileWrapper
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSData;
@class NSDictionary;
@class NSError;
@class NSMutableDictionary;
@class NSString;
@class NSURL;

typedef enum
{
  GSFileWrapperDirectoryType,
  GSFileWrapperRegularFileType,
  GSFileWrapperSymbolicLinkType
} GSFileWrapperType;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
enum {
   NSFileWrapperReadingImmediate = 1,
   NSFileWrapperReadingWithoutMapping = 2,
};
typedef NSUInteger NSFileWrapperReadingOptions; 

enum {
   NSFileWrapperWritingAtomic = 1,
   NSFileWrapperWritingWithNameUpdating = 2,
};
typedef NSUInteger NSFileWrapperWritingOptions;
#endif

GS_EXPORT_CLASS
@interface NSFileWrapper : NSObject 
{
  NSString		*_filename;
  NSString		*_preferredFilename;
  NSMutableDictionary	*_fileAttributes;
  GSFileWrapperType	_wrapperType;
  id			_wrapperData;
  id		        _icon;
}

//
// Initialization 
//

// Init instance of directory type
- (id)initDirectoryWithFileWrappers:(NSDictionary *)docs;	 

// Init instance of regular file type
- (id)initRegularFileWithContents:(NSData *)data;		 

// Init instance of symbolic link type
- (id)initSymbolicLinkWithDestination:(NSString *)path;

// Init an instance from the file, directory, or symbolic link at path. 
// This can create a tree of instances 
- (id)initWithPath:(NSString *)path;	// with a directory instance at the top

// Init an instance from data in standard serial format.  Serial format
// is the same as that used by NSText's RTFDFromRange: method.  This can 
// create a tree of instances with a directory instance at the top
- (id)initWithSerializedRepresentation:(NSData *)data;

//
// General methods 
//

// write instace to disk at path. if directory type, this method is recursive
// if flag is YES, the wrapper will be updated with the name used in writing
// the file
- (BOOL)writeToFile:(NSString *)path
         atomically:(BOOL)atomicFlag
    updateFilenames:(BOOL)updateFilenamesFlag;

- (NSData *)serializedRepresentation;

- (void)setFilename:(NSString *)filename;
- (NSString *)filename;

- (void)setPreferredFilename:(NSString *)filename;
- (NSString *)preferredFilename;

- (void)setFileAttributes:(NSDictionary *)attributes;
- (NSDictionary *)fileAttributes;

- (BOOL)isRegularFile;
- (BOOL)isDirectory;
- (BOOL)isSymbolicLink;

- (BOOL)needsToBeUpdatedFromPath:(NSString *)path;
- (BOOL)updateFromPath:(NSString *)path;

//								
// Directory type methods 				  
//

// these messages raise an exception when sent to non-directory type wrappers!
- (NSString *)addFileWrapper:(NSFileWrapper *)doc;
- (void)removeFileWrapper:(NSFileWrapper *)doc;
- (NSDictionary *)fileWrappers;
- (NSString *)keyForFileWrapper:(NSFileWrapper *)doc;
- (NSString *)addFileWithPath:(NSString *)path;
- (NSString *)addRegularFileWithContents:(NSData *)data 
                       preferredFilename:(NSString *)filename;
- (NSString *)addSymbolicLinkWithDestination:(NSString *)path 
                           preferredFilename:(NSString *)filename;

//
// Regular file type methods 				  
//

- (NSData *)regularFileContents;

//
// Symbolic link type methods 				  
//

- (NSString *)symbolicLinkDestination;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (id)initSymbolicLinkWithDestinationURL:(NSURL *)url;
- (id)initWithURL:(NSURL *)url 
          options:(NSFileWrapperReadingOptions)options
            error:(NSError **)outError;
- (BOOL)matchesContentsOfURL:(NSURL *)url;
- (BOOL)readFromURL:(NSURL *)url
            options:(NSFileWrapperReadingOptions)options
              error:(NSError **)outError;
- (NSURL *)symbolicLinkDestinationURL;
- (BOOL)writeToURL:(NSURL *)url
           options:(NSFileWrapperWritingOptions)options
originalContentsURL:(NSURL *)originalContentsURL
             error:(NSError **)outError;
#endif

@end

#endif // _GNUstep_H_NSFileWrapper
