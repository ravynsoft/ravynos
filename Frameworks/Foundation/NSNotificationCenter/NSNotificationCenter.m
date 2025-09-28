/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSNotification.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSString.h>

#import <Foundation/NSObjectToObservers.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSNotification_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSNotificationCenter

static NSNotificationCenter *defaultCenter = nil;

+(NSNotificationCenter *)defaultCenter {
	@synchronized(self) {
		if (defaultCenter == nil) {
			defaultCenter = [[[self class] alloc] init];
		}
	}
   return defaultCenter;
}

-init {
   _nameToRegistry=[[NSMutableDictionary allocWithZone:[self zone]] init];
   _noNameRegistry=[[NSObjectToObservers allocWithZone:[self zone]] init];
   return self;
}

-(void)dealloc {
   [_nameToRegistry release];
   [_noNameRegistry release];
   [super dealloc];
}

-(void)addObserver:anObserver selector:(SEL)selector name:(NSString *)name
   object:object {
	@synchronized(self) {
   NSNotificationObserver *observer=[[[NSNotificationObserver allocWithZone:[self zone]] 
        initWithObserver:anObserver selector:selector] autorelease];
   NSObjectToObservers *registry;

   if(name==nil)
    registry=_noNameRegistry;
   else{
    registry=[_nameToRegistry objectForKey:name];

    if(registry==nil){
     registry=[[[NSObjectToObservers allocWithZone:[self zone]] init]
       autorelease];
     [_nameToRegistry setObject:registry forKey:name];
    }
   }

   [registry addObserver:observer object:object];
	}
}

-(void)removeObserver:anObserver {
	@synchronized(self) {
   NSMutableArray          *removeRegistries=[NSMutableArray array];
   NSEnumerator            *keyEnumerator=[_nameToRegistry keyEnumerator];
   NSString                *key;
   NSObjectToObservers *registry;
   NSInteger                count;

   while((key=[keyEnumerator nextObject])!=nil){
    registry=[_nameToRegistry objectForKey:key];
    [registry removeObserver:anObserver object:nil];
    if([registry count]==0)
     [removeRegistries addObject:key];
   }

   [_noNameRegistry removeObserver:anObserver object:nil];

   count=[removeRegistries count];
   while(--count>=0)
    [_nameToRegistry removeObjectForKey:[removeRegistries objectAtIndex:count]];
	}
}

-(void)removeObserver:anObserver name:(NSString *)name object:object {
	@synchronized(self) {
   NSMutableArray          *removeRegistries=[NSMutableArray array];
   NSObjectToObservers *registry;
   NSInteger               count;

   if(name!=nil){
    registry=[_nameToRegistry objectForKey:name];

    [registry removeObserver:anObserver object:object];

    if([registry count]==0)
     [removeRegistries addObject:name];
   }
   else {
    NSEnumerator *keyEnumerator=[_nameToRegistry keyEnumerator];
    NSString     *key;

    while((key=[keyEnumerator nextObject])!=nil){
     registry=[_nameToRegistry objectForKey:key];

     [registry removeObserver:anObserver object:object];

     if([registry count]==0)
      [removeRegistries addObject:key];
    }

    [_noNameRegistry removeObserver:anObserver object:object];
   }

   count=[removeRegistries count];
   while(--count>=0){
    NSString            *key=[removeRegistries objectAtIndex:count];
    NSObjectToObservers *oto=[_nameToRegistry objectForKey:key];

    [oto invalidate];

    [_nameToRegistry removeObjectForKey:key];
   }
	}
}

static inline void postNotification(NSNotificationCenter *self,NSNotification *note){
   NSAutoreleasePool       *pool=[NSAutoreleasePool new];

   @synchronized(self) {
   	NSObjectToObservers *registry=self->_noNameRegistry;

   	[registry postNotification:note];
   	registry=[self->_nameToRegistry objectForKey:[note name]];
   	[[registry retain] postNotification:note];
   	[registry release];
   }

   [pool release];
}

-(void)postNotification:(NSNotification *)note {
   postNotification(self,note);
}

-(void)postNotificationName:(NSString *)name object:object userInfo:(NSDictionary *)userInfo {
	NSNotification *note = NSNotification_concreteNew(NULL,name,object,userInfo);
	postNotification(self,note);
	[note release];
}

-(void)postNotificationName:(NSString *)name object:object {
	NSNotification *note = NSNotification_concreteNew(NULL,name,object,nil);
	postNotification(self,note);
	[note release];
}

@end
