/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableDictionary_mapTable.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSMutableDictionary_mapTable.h>

@implementation NSMutableDictionary

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSMutableDictionary_CF class],0,zone);
}

-initWithCapacity:(NSUInteger)capacity {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithObjects:(id *)objects forKeys:(id *)keys count:(NSUInteger)count {
   int i;

   self=[self initWithCapacity:count];

   for(i=0;i<count;i++)
    [self setObject:objects[i] forKey:keys[i]];

   return self;
}

-copy {
   return [[NSDictionary allocWithZone:NULL] initWithDictionary:self];
}

-copyWithZone:(NSZone *)zone {
   return [[NSDictionary allocWithZone:zone] initWithDictionary:self];
}

-(Class)classForCoder {
   return objc_lookUpClass("NSMutableDictionary");
}

+dictionaryWithCapacity:(NSUInteger)capacity {
   return [[[self allocWithZone:NULL] initWithCapacity:capacity] autorelease];
}


-(void)setObject:object forKey:aKey {
   NSInvalidAbstractInvocation();
}

-(void)addEntriesFromDictionary:(NSDictionary *)dictionary {
   NSEnumerator *keyEnum=[dictionary keyEnumerator];
   id key;

   while((key=[keyEnum nextObject])!=nil)
    [self setObject:[dictionary objectForKey:key] forKey:key];
}

-(void)setDictionary:(NSDictionary *)dictionary {
   [self removeAllObjects];
   [self addEntriesFromDictionary:dictionary];
}

-(void)removeObjectForKey:aKey {
   NSInvalidAbstractInvocation();
}

-(void)removeAllObjects {
   NSArray *allKeys=[self allKeys];
   NSInteger      count=[allKeys count];

   while(--count>=0)
    [self removeObjectForKey:[allKeys objectAtIndex:count]];
}

-(void)removeObjectsForKeys:(NSArray *)keys {
   NSInteger count=[keys count];

   while(--count>=0)
    [self removeObjectForKey:[keys objectAtIndex:count]];
}

@end
