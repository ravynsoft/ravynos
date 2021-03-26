/* Copyright (c) 2008 Dan Knapp
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSDocument.h>
#import <AppKit/NSNib.h>
#import <Foundation/NSRaise.h>
#import "NSPersistentDocument.h"
#import <CoreData/NSManagedObjectContext.h>
#import <CoreData/NSPersistentStoreCoordinator.h>

@implementation NSPersistentDocument

- (id) init {
    self = [super init];
    if(self) {
        _context = [[NSManagedObjectContext alloc] init];
        NSManagedObjectModel *model = [[NSManagedObjectModel alloc]
                                       initWithName: [self className]];
        NSPersistentStoreCoordinator *coordinator
	    = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: model];
        [_context setPersistentStoreCoordinator: coordinator];
    }
    return self;
}


- (void) dealloc {
    [_context release];
    [super dealloc];
}


- (NSManagedObjectContext *) managedObjectContext {
    return _context;
}


- (NSManagedObjectModel *) managedObjectModel {
    return [[_context persistentStoreCoordinator] managedObjectModel];
}


- (void) setManagedObjectContext: (NSManagedObjectContext *) managedObjectContext {
    NSUnimplementedMethod();
}


- (BOOL) configurePersistentStoreCoordinatorForURL: (NSURL *) url
                                            ofType: (NSString *) fileType
                                modelConfiguration: (NSString *) configuration
                                      storeOptions: (NSDictionary *) storeOptions
                                             error: (NSError **) error
{
    NSUnimplementedMethod();
    return NO;
}


- (NSString *) persistentStoreTypeForFileType: (NSString *) fileType {
    NSUnimplementedMethod();
    return nil;
}


- (BOOL) hasUndoManager {
    return YES;
}


- (void) setHasUndoManager: (BOOL) flag {
    /* Ignored.  Always has an undo manager. */
}


- (void) setUndoManager: (NSUndoManager *) undoManager {
    /* Ignored.  Always uses the undo manager of its persistent context. */
}


- (BOOL)isDocumentEdited {
    NSUnimplementedMethod();
    return NO;
}


- (BOOL) readFromURL: (NSURL *) absoluteURL
              ofType: (NSString *) typeName
               error: (NSError **) outError
{
    NSUnimplementedMethod();
    return NO;
}


- (BOOL) revertToContentsOfURL: (NSURL *) inAbsoluteURL
                        ofType: (NSString *) inTypeName
                         error: (NSError **) outError
{
    NSUnimplementedMethod();
    return NO;
}


- (BOOL) writeToURL: (NSURL *) absoluteURL
             ofType: (NSString *) typeName
   forSaveOperation: (NSSaveOperationType) saveOperation
originalContentsURL: (NSURL *) absoluteOriginalContentsURL
              error: (NSError **) error
{
    NSUnimplementedMethod();
    return NO;
}

@end
