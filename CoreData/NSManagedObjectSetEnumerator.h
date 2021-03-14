#import <Foundation/NSEnumerator.h>

@class NSManagedObjectContext;

@interface NSManagedObjectSetEnumerator : NSEnumerator {
    NSManagedObjectContext *_context;
    NSEnumerator *_enumerator;
}

- initWithManagedObjectContext:(NSManagedObjectContext *)context objectEnumerator:(NSEnumerator *)enumerator;

@end
