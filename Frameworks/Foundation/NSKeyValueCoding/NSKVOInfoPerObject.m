#import "NSKVOInfoPerObject.h"
#import "NSKeyObserver.h"
#import "NSKeyPathObserver.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

@implementation NSKVOInfoPerObject

-init {
   _lock=(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
   _dictionary=[[NSMutableDictionary allocWithZone:NULL] init];
   return self;
}

-(void)dealloc {
   [_dictionary release];
   [super dealloc];
}

-(BOOL)isEmpty {
   return ([_dictionary count]==0)?YES:NO;
}

-objectForKey:key {
   return [_dictionary objectForKey:key];
}

-(void)setObject:object forKey:key {
   [_dictionary setObject:object forKey:key];
}

-(NSArray *)keyObserversForKey:(NSString *)key {
   return [_dictionary objectForKey:key];
}

-(void)addKeyObserver:(NSKeyObserver *)keyObserver {
   NSString       *key=[keyObserver key];
   NSMutableArray *observers=[_dictionary objectForKey:key];
   
   if(observers==nil){
    observers=[NSMutableArray array];
    [_dictionary setObject:observers forKey:key];
   }
   
   [observers addObject:keyObserver];
}

-(void)removeKeyObserver:(NSKeyObserver *)keyObserver {
   NSString       *key=[[[keyObserver key] retain] autorelease];  // do the retain/autorelease dance, because we just might release the key out from under us when the keyObserver is deallocated.
   NSMutableArray *observers=[_dictionary objectForKey:key];
   
   [observers removeObjectIdenticalTo:keyObserver];
   if([observers count]==0)
    [_dictionary removeObjectForKey:key];
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"%@:%p %@", [self class], self, _dictionary];
}
@end
