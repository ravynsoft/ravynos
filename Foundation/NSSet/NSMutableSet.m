/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSMutableSet_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSMutableSet : NSSet

+allocWithZone:(NSZone *)zone {
   if(self==[NSMutableSet class])
    return NSAllocateObject([NSMutableSet_concrete class],0,zone);

   return NSAllocateObject(self,0,zone);
}

-initWithCapacity:(NSUInteger)capacity {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithObjects:(id *)objects count:(NSUInteger)count {
   NSUInteger i;

   self=[self initWithCapacity:count];
   for(i=0;i<count;i++)
    [self addObject:objects[i]];

   return self;
}

-(Class)classForCoder {
   return objc_lookUpClass("NSMutableSet");
}

-copyWithZone:(NSZone *)zone {
   return [[NSSet allocWithZone:zone] initWithSet:self];
}

+set {
   if(self==[NSMutableSet class])
    return NSAutorelease(NSMutableSet_concreteNew(NULL,0));

   return [[[self allocWithZone:NULL] initWithCapacity:0] autorelease];;
}

+setWithArray:(NSArray *)array {
   if(self==[NSMutableSet class])
    return NSAutorelease(NSMutableSet_concreteNewWithArray(NULL,array));

   return [[[self allocWithZone:NULL] initWithArray:array] autorelease];;
}

+setWithObject:object {
   if(self==[NSMutableSet class])
    return NSAutorelease(NSMutableSet_concreteNewWithObjects(NULL,&object,1));

   return [[[self allocWithZone:NULL] initWithObjects:&object count:1] autorelease];
}

+setWithObjects:first,... {
   va_list  arguments;
   NSUInteger i,count;
   id      *objects;

   va_start(arguments,first);
   count=1;
   while(va_arg(arguments,id)!=nil)
    count++;
   va_end(arguments);

   objects=__builtin_alloca(sizeof(id)*count);

   va_start(arguments,first);
   objects[0]=first;
   for(i=1;i<count;i++)
    objects[i]=va_arg(arguments,id);

   va_end(arguments);

   if(self==[NSMutableSet class]){
    return NSAutorelease(NSMutableSet_concreteNewWithObjects(NULL,
     objects,count));
   }

   return [[[self allocWithZone:NULL] initWithObjects:objects count:count] autorelease];
}

+setWithObjects:(id *)objects count:(NSUInteger)count {
   return [[[self allocWithZone:NULL] initWithObjects:objects count:count] autorelease];
}

+setWithCapacity:(NSUInteger)capacity {
   if(self==[NSMutableSet class])
    return NSAutorelease(NSMutableSet_concreteNew(NULL,capacity));

   return [[[self allocWithZone:NULL] initWithCapacity:capacity] autorelease];
}

-(void)addObject:object {
   NSInvalidAbstractInvocation();
}

-(void)addObjectsFromArray:(NSArray *)array {
   NSUInteger i,count=[array count];

   for(i=0;i<count;i++)
    [self addObject:[array objectAtIndex:i]];
}

-(void)setSet:(NSSet *)other {
   NSEnumerator *state;
   id            object;

   [self removeAllObjects];
   
   state=[other objectEnumerator];
   while((object=[state nextObject])!=nil)
    [self addObject:object];
}

-(void)unionSet:(NSSet *)other {
   NSEnumerator *state=[other objectEnumerator];
   id            object;

   while((object=[state nextObject])!=nil){
    if([self member:object]==nil)
     [self addObject:object];
   }
}

-(void)removeObject:object {
   NSInvalidAbstractInvocation();
}

-(void)removeAllObjects {
   NSArray *allObjects=[self allObjects];
   NSInteger      i,count=[allObjects count];

   for(i=0;i<count;i++)
    [self removeObject:[allObjects objectAtIndex:i]];
}

-(void)minusSet:(NSSet *)other {
   NSEnumerator *state=[other objectEnumerator];
   id            object;

   while((object=[state nextObject])!=nil)
    [self removeObject:object];
}

-(void)intersectSet:(NSSet *)other {
   NSArray *allObjects=[self allObjects];
   NSInteger      i,count=[allObjects count];

   for(i=0;i<count;i++){
    id object=[allObjects objectAtIndex:i];

    if([other member:object]==nil)
     [self removeObject:object];
   }
}

@end
