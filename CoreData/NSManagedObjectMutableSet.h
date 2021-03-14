#import <Foundation/NSMutableSet.h>

@class NSManagedObject;

@interface NSManagedObjectMutableSet : NSMutableSet {
    NSManagedObject *_object;
    NSString *_key;
}

- initWithManagedObject:(NSManagedObject *)object key:(NSString *)key;

@end
