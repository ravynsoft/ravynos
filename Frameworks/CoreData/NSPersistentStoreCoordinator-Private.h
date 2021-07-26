#import <CoreData/NSPersistentStoreCoordinator.h>

@class NSManagedObjectID;

@interface NSPersistentStoreCoordinator (private)
- (NSPersistentStore *)_persistentStoreWithIdentifier:(NSString *)identifier;
- (NSPersistentStore *)_persistentStoreForObjectID:(NSManagedObjectID *)object;
- (NSPersistentStore *)_persistentStoreForObject:(NSManagedObject *)object;
@end
