#import <CoreData/NSPersistentStore.h>

@class NSMutableSet, NSSet, NSAtomicStoreCacheNode, NSManagedObjectID, NSManagedObject, NSEntityDescription, NSMutableDictionary, NSDictionary;

@interface NSAtomicStore : NSPersistentStore {
    NSDictionary *_metadata;
    NSMutableSet *_cacheNodes;
    NSMutableDictionary *_objectIDToCacheNode;
    NSMutableDictionary *_objectIDTable;
}

- initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator configurationName:(NSString *)configurationName URL:(NSURL *)url options:(NSDictionary *)options;

- (NSSet *)cacheNodes;

- (NSDictionary *)metadata;
- (void)setMetadata:(NSDictionary *)value;

- (void)addCacheNodes:(NSSet *)value;

- (NSAtomicStoreCacheNode *)cacheNodeForObjectID:(NSManagedObjectID *)objectID;
- (NSAtomicStoreCacheNode *)newCacheNodeForManagedObject:(NSManagedObject *)managedObject;

- newReferenceObjectForManagedObject:(NSManagedObject *)managedObject;

- (NSManagedObjectID *)objectIDForEntity:(NSEntityDescription *)entity referenceObject:data;

- referenceObjectForObjectID:(NSManagedObjectID *)objectID;

- (void)updateCacheNode:(NSAtomicStoreCacheNode *)node fromManagedObject:(NSManagedObject *)managedObject;

- (void)willRemoveCacheNodes:(NSSet *)cacheNodes;

- (BOOL)load:(NSError **)error;
- (BOOL)save:(NSError **)error;

@end
