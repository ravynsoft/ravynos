#import <CoreData/NSManagedObjectID.h>

@interface NSManagedObjectID (private)
- initWithEntity:(NSEntityDescription *)entity;
- (NSString *)storeIdentifier;

// Do not use referenceObject directly except for debugging
// Use -[NSAtomicStore referenceObjectForUniqueID:] as this needs to unique the ID
- referenceObject;

- (void)setStoreIdentifier:(NSString *)storeIdentifier;
- (void)setReferenceObject:value;
- (void)setPersistentStore:(NSPersistentStore *)store;
@end
