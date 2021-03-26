#import <CoreData/NSAtomicStoreCacheNode.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

@implementation NSAtomicStoreCacheNode

-initWithObjectID:(NSManagedObjectID *)objectID {
   _objectID=[objectID copy];
   _propertyCache=nil;
   return self;
}

-(void)dealloc {
   [_objectID release];
   [_propertyCache release];
   [super dealloc];
}

-(NSUInteger)hash {
   return [_objectID hash];
}

-(BOOL)isEqual:other {
   return [_objectID isEqual:other];
}

-(NSManagedObjectID *)objectID {
   return _objectID;
}

-(NSMutableDictionary *)propertyCache {
   return _propertyCache;
}

-(void)setPropertyCache:(NSMutableDictionary *)value {
   value=[value retain];
   [_propertyCache release];
   _propertyCache=value;
}

-(void)setValue:value forKey:(NSString *)key {
   if(_propertyCache==nil)
    _propertyCache=[[NSMutableDictionary alloc] init];

   if(value==nil)
    [_propertyCache removeObjectForKey:key];
   else {
    [_propertyCache setObject:value forKey:key];
   }
}

-valueForKey:(NSString *)key {
   return [_propertyCache objectForKey:key];
}

@end
