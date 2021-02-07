/* 
   NSDataLink.h

   Link between a source and dependent document

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
   This file is part of the GNUstep GUI Library.

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

#ifndef _GNUstep_H_NSDataLink
#define _GNUstep_H_NSDataLink
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSString;
@class NSArray;
@class NSDate;

@class NSDataLinkManager;
@class NSSelection;
@class NSPasteboard;

typedef int NSDataLinkNumber;

typedef enum _NSDataLinkDisposition {
  NSLinkInDestination,
  NSLinkInSource,
  NSLinkBroken 
} NSDataLinkDisposition;

typedef enum _NSDataLinkUpdateMode {
  NSUpdateContinuously,
  NSUpdateWhenSourceSaved,
  NSUpdateManually,
  NSUpdateNever
} NSDataLinkUpdateMode;

APPKIT_EXPORT NSString *NSDataLinkFileNameExtension;

@interface NSDataLink : NSObject <NSCoding>
{
  // Attributes
  @private
  // link info.
  NSDataLinkNumber      linkNumber;
  NSDataLinkDisposition disposition;
  NSDataLinkUpdateMode  updateMode;

  // info about the source.
  NSDate                *lastUpdateTime;
  NSString              *sourceApplicationName;
  NSString              *sourceFilename;
  NSSelection           *sourceSelection;
  id                    sourceManager;

  // info about the destination
  NSString              *destinationApplicationName;
  NSString              *destinationFilename;
  NSSelection           *destinationSelection;
  id                    destinationManager;
  
  // types.
  NSArray               *types;

  // other flags
  struct __linkFlags {
    unsigned   appVerifies:1;
    unsigned   broken:1;
    unsigned   canUpdateContinuously:1;
    unsigned   isDirty:1;
    unsigned   willOpenSource:1;
    unsigned   willUpdate:1;
    unsigned   isMarker:1;
  } _flags;
}

//
// Initializing a Link
//
- (id)initLinkedToFile:(NSString *)filename;
- (id)initLinkedToSourceSelection:(NSSelection *)selection
			managedBy:(NSDataLinkManager *)linkManager
		  supportingTypes:(NSArray *)newTypes;
- (id)initWithContentsOfFile:(NSString *)filename;
- (id)initWithPasteboard:(NSPasteboard *)pasteboard;

//
// Exporting a Link
//
- (BOOL)saveLinkIn:(NSString *)directoryName;
- (BOOL)writeToFile:(NSString *)filename;
- (void)writeToPasteboard:(NSPasteboard *)pasteboard;

//
// Information about the Link
//
- (NSDataLinkDisposition)disposition;
- (NSDataLinkNumber)linkNumber;
- (NSDataLinkManager *)manager;

//
// Information about the Link's Source
//
- (NSDate *)lastUpdateTime;
- (BOOL)openSource;
- (NSString *)sourceApplicationName;
- (NSString *)sourceFilename;
- (NSSelection *)sourceSelection;
- (NSArray *)types;

//
// Information about the Link's Destination
//
- (NSString *)destinationApplicationName;
- (NSString *)destinationFilename;
- (NSSelection *)destinationSelection;

//
// Changing the Link
//
- (BOOL)break;
- (void)noteSourceEdited;
- (void)setUpdateMode:(NSDataLinkUpdateMode)mode;
- (BOOL)updateDestination;
- (NSDataLinkUpdateMode)updateMode;
@end

APPKIT_EXPORT NSString *NSDataLinkFilenameExtension;
APPKIT_EXPORT NSString *NSDataLinkPboardType;

#endif // _GNUstep_H_NSDataLink

