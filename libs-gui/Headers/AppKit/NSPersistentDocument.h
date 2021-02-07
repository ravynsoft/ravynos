/* Definition of class NSPersistentDocument
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Thu May  7 00:04:09 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSPersistentDocument_h_GNUSTEP_GUI_INCLUDE
#define _NSPersistentDocument_h_GNUSTEP_GUI_INCLUDE

#import <AppKit/NSDocument.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSManagedObjectContext;
@class NSManagedObjectModel;
@class NSError;
@class NSURL;
@class NSString;
@class NSDictionary;
@class NSUndoManager;

@interface NSPersistentDocument : NSDocument
{
  NSManagedObjectContext *_managedObjectContext;
  NSManagedObjectModel *_managedObjectModel;
}
  
- (NSManagedObjectContext *) managedObjectContext;
- (NSManagedObjectModel *) managedObjectModel;  

- (BOOL) configurePersistentStoreCoordinatorForURL: (NSURL *)url 
                                            ofType: (NSString *)fileType 
                                modelConfiguration: (NSString *)config 
                                      storeOptions: (NSDictionary *)options 
                                             error: (NSError **)err;

- (NSString *) persistentStoreTypeForFileType: (NSString *)fileType;

- (BOOL)hasUndoManager;
- (void) setHasUndoManager: (BOOL)flag;

- (void) setUndoManager: (NSUndoManager *)manager;

- (BOOL) isDocumentEdited;

- (BOOL)readFromURL: (NSURL *)absoluteURL 
             ofType: (NSString *)typeName 
              error: (NSError **)err;

- (BOOL)revertToContentsOfURL: (NSURL *)url
                       ofType: (NSString *)type
                        error: (NSError **)outErr;
  
- (BOOL)  writeToURL: (NSURL *)url
              ofType: (NSString *)type 
    forSaveOperation: (NSSaveOperationType)saveOp 
 originalContentsURL: (NSURL *)originalContents
               error: (NSError **)err;
  
- (BOOL)canAsynchronouslyWriteToURL: (NSURL *)url
                             ofType: (NSString *)type
                   forSaveOperation: (NSSaveOperationType)saveOp;

  
- (BOOL)configurePersistentStoreCoordinatorForURL: (NSURL *)url 
                                           ofType: (NSString *)fileType 
                                            error: (NSError **)err;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPersistentDocument_h_GNUSTEP_GUI_INCLUDE */

