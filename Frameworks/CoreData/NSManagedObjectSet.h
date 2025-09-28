#import <Foundation/NSSet.h>

@class NSManagedObjectContext;

@interface NSManagedObjectSet : NSSet {
    NSManagedObjectContext *_context;
    NSSet *_set;
}

- initWithManagedObjectContext:(NSManagedObjectContext *)context set:(NSSet *)set;

@end
