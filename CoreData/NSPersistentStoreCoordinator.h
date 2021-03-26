/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSLock.h>
#import <CoreData/CoreDataExports.h>

@class NSManagedObjectModel, NSPersistentStore, NSManagedObjectID;

COREDATA_EXPORT NSString *const NSStoreTypeKey;
COREDATA_EXPORT NSString *const NSStoreUUIDKey;

COREDATA_EXPORT NSString *const NSXMLStoreType;
COREDATA_EXPORT NSString *const NSInMemoryStoreType;
COREDATA_EXPORT NSString *const NSMigratePersistentStoresAutomaticallyOption;

COREDATA_EXPORT NSString *const NSPersistentStoreCoordinatorStoresDidChangeNotification;
COREDATA_EXPORT NSString *const NSAddedPersistentStoresKey;
COREDATA_EXPORT NSString *const NSRemovedPersistentStoresKey;
COREDATA_EXPORT NSString *const NSUUIDChangedPersistentStoresKey;

@interface NSPersistentStoreCoordinator : NSObject <NSLocking> {
    NSLock *_lock;
    NSManagedObjectModel *_model;
    NSMutableArray *_stores;
}

+ (NSDictionary *)registeredStoreTypes;
+ (void)registerStoreClass:(Class)storeClass forStoreType:(NSString *)storeType;

- initWithManagedObjectModel:(NSManagedObjectModel *)model;

- (NSManagedObjectModel *)managedObjectModel;

- (NSPersistentStore *)addPersistentStoreWithType:(NSString *)storeType configuration:(NSString *)configuration URL:(NSURL *)storeURL options:(NSDictionary *)options error:(NSError **)error;

- (BOOL)setURL:(NSURL *)url forPersistentStore:(NSPersistentStore *)store;

- (BOOL)removePersistentStore:(NSPersistentStore *)store error:(NSError **)error;
- (NSPersistentStore *)migratePersistentStore:(NSPersistentStore *)store toURL:(NSURL *)URL options:(NSDictionary *)options withType:(NSString *)storeType error:(NSError **)error;
- (NSArray *)persistentStores;
- (NSPersistentStore *)persistentStoreForURL:(NSURL *)URL;
- (NSURL *)URLForPersistentStore:(NSPersistentStore *)store;

- (NSManagedObjectID *)managedObjectIDForURIRepresentation:(NSURL *)URL;

- (void)lock;
- (BOOL)tryLock;
- (void)unlock;

- (NSDictionary *)metadataForPersistentStore:(NSPersistentStore *)store;
- (void)setMetadata:(NSDictionary *)metadata forPersistentStore:(NSPersistentStore *)store;
+ (BOOL)setMetadata:(NSDictionary *)metadata forPersistentStoreOfType:(NSString *)storeType URL:(NSURL *)url error:(NSError **)error;
+ (NSDictionary *)metadataForPersistentStoreOfType:(NSString *)storeType URL:(NSURL *)url error:(NSError **)error;

@end
