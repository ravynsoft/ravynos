#import <CoreData/NSManagedObject.h>

@interface NSManagedObject (private)
- initWithObjectID:(NSManagedObjectID *)objectID managedObjectContext:(NSManagedObjectContext *)context;
@end
