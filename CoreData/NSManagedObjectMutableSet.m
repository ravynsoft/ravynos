#import "NSManagedObjectMutableSet.h"
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSManagedObjectContext.h>
#import <Foundation/NSRaise.h>

@implementation NSManagedObjectMutableSet

-initWithManagedObject:(NSManagedObject *)object key:(NSString *)key {
   _object=[object retain];
   _key=[key copy];
   return self;
}

-(void)dealloc {
   [_object release];
   [_key release];
   [super dealloc];
}

-(NSUInteger)count {
   return [[_object valueForKey:_key] count];
}

-member:object {
   return [[_object valueForKey:_key] member:[object objectID]];
}

-(NSEnumerator *)objectEnumerator {
   NSUnimplementedMethod();
   return nil;
}

-(void)addObject:object {
   NSMutableSet *set=[NSMutableSet setWithSet:[_object valueForKey:_key]];
   
   [set addObject:object];
   
   [_object setValue:set forKey:_key];
}

-(void)removeObject:object {
   NSMutableSet *set=[NSMutableSet setWithSet:[_object valueForKey:_key]];
   
   [set removeObject:object];
   
   [_object setValue:set forKey:_key];
}

@end
