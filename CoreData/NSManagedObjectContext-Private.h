#import <CoreData/NSManagedObjectContext.h>

@class NSAtomicStoreCacheNode;

@interface NSManagedObjectContext (private)
- (NSAtomicStoreCacheNode *)_cacheNodeForObjectID:(NSManagedObjectID *)objectID;
- (void)_registerObject:(NSManagedObject *)object;
@end
