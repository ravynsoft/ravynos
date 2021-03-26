#import <Foundation/NSObject.h>

@class NSManagedObjectID, NSMutableDictionary;

@interface NSAtomicStoreCacheNode : NSObject {
    NSManagedObjectID *_objectID;
    NSMutableDictionary *_propertyCache;
}

- initWithObjectID:(NSManagedObjectID *)objectID;

- (NSManagedObjectID *)objectID;

- (NSMutableDictionary *)propertyCache;

- (void)setPropertyCache:(NSMutableDictionary *)value;

- (void)setValue:value forKey:(NSString *)key;

- valueForKey:(NSString *)key;

@end
