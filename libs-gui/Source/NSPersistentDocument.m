/* Implementation of class NSPersistentDocument
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

#import <Foundation/NSURL.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSException.h>
#import <Foundation/NSUndoManager.h>
#import "AppKit/NSPersistentDocument.h"

static Class persistentDocumentClass = nil;

@implementation NSPersistentDocument

+ (void) setPersistentDocumentClass: (Class)clz
{
  persistentDocumentClass = clz;
}

+ (void) initialize
{
  if (self == [NSPersistentDocument class])
    {
      [self setVersion: 1];
      [self setPersistentDocumentClass: [NSPersistentDocument class]];
    }
}

+ (id) allocWithZone: (NSZone *)z
{
  if (persistentDocumentClass == self)
    {
      NSAssert(persistentDocumentClass != self,
               @"NSPersistentDocument error: Concrete class not set");
    }

  return NSAllocateObject(persistentDocumentClass, 0, z);
}

- (NSManagedObjectContext *) managedObjectContext
{
  return _managedObjectContext;
}

- (NSManagedObjectModel *) managedObjectModel
{
  return _managedObjectModel;
}

- (BOOL) configurePersistentStoreCoordinatorForURL: (NSURL *)url 
                                            ofType: (NSString *)fileType 
                                modelConfiguration: (NSString *)config 
                                      storeOptions: (NSDictionary *)options 
                                             error: (NSError **)err
{
  return NO;
}

- (NSString *) persistentStoreTypeForFileType: (NSString *)fileType
{
  return nil;
}

- (BOOL)hasUndoManager
{
  return YES; // overridden since uses stores undo manager
}

- (void) setHasUndoManager: (BOOL)flag
{
  // Implemented as NO-OP per documentation
}

- (void) setUndoManager: (NSUndoManager *)manager
{
  // implemented as NO-OP per documentation.
}

- (BOOL) isDocumentEdited
{
  return NO;
}

- (BOOL)readFromURL: (NSURL *)absoluteURL 
             ofType: (NSString *)typeName 
              error: (NSError **)err
{
  return NO;
}

- (BOOL)revertToContentsOfURL: (NSURL *)url
                       ofType: (NSString *)type
                        error: (NSError **)outErr
{
  return NO;
}
  
- (BOOL)  writeToURL: (NSURL *)url
              ofType: (NSString *)type 
    forSaveOperation: (NSSaveOperationType)saveOp 
 originalContentsURL: (NSURL *)originalContents
               error: (NSError **)err
{
  return NO;
}
  
- (BOOL)canAsynchronouslyWriteToURL: (NSURL *)url
                             ofType: (NSString *)type
                   forSaveOperation: (NSSaveOperationType)saveOp
{
  return NO;
}

- (BOOL)configurePersistentStoreCoordinatorForURL: (NSURL *)url 
                                           ofType: (NSString *)fileType 
                                            error: (NSError **)err
{
  return NO;
}

@end

