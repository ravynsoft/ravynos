/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
// -*- mode: objc -*-

#import <AppKit/NSDocument.h>

@class NSManagedObjectContext;
@class NSManagedObjectModel;
@interface NSPersistentDocument : NSDocument {
    NSManagedObjectContext *_context;
}

- (NSManagedObjectContext *)managedObjectContext;
- (NSManagedObjectModel *)managedObjectModel;
- (void)setManagedObjectContext:(NSManagedObjectContext *)managedObjectContext;
- (BOOL)configurePersistentStoreCoordinatorForURL:(NSURL *)url
                                           ofType:(NSString *)fileType
                               modelConfiguration:(NSString *)configuration
                                     storeOptions:(NSDictionary *)storeOptions
                                            error:(NSError **)error;
- (NSString *)persistentStoreTypeForFileType:(NSString *)fileType;

- (BOOL)hasUndoManager;
- (void)setHasUndoManager:(BOOL)flag;
- (void)setUndoManager:(NSUndoManager *)undoManager;
- (BOOL)isDocumentEdited;

- (BOOL)readFromURL:(NSURL *)absoluteURL
             ofType:(NSString *)typeName
              error:(NSError **)outError;
- (BOOL)revertToContentsOfURL:(NSURL *)inAbsoluteURL
                       ofType:(NSString *)inTypeName
                        error:(NSError **)outError;
- (BOOL)writeToURL:(NSURL *)absoluteURL
                 ofType:(NSString *)typeName
       forSaveOperation:(NSSaveOperationType)saveOperation
    originalContentsURL:(NSURL *)absoluteOriginalContentsURL
                  error:(NSError **)error;

@end
