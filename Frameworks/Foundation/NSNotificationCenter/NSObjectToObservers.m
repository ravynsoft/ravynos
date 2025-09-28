/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObjectToObservers.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>

@implementation NSObjectToObservers

-init {
   _objectToObservers=NSCreateMapTableWithZone(
     NSNonOwnedPointerMapKeyCallBacks,NSObjectMapValueCallBacks,0,[self zone]);
   return self;
}

-(void)dealloc {
   NSFreeMapTable(_objectToObservers);
   [super dealloc];
}

-(void)invalidate {
   NSResetMapTable(_objectToObservers);
}

-(void)addObserver:(NSNotificationObserver *)observer object:objectOrNil {
   id              object=(objectOrNil!=nil)?objectOrNil:(id)[NSNull null];
   NSMutableArray *observers=NSMapGet(_objectToObservers,object);

   if(observers==nil){
    observers=[NSMutableArray array];
    NSMapInsert(_objectToObservers,object,observers);
   }
   [observers addObject:observer];
}

-(void)removeObserver:anObserver object:object {
   id  removeKeys[NSCountMapTable(_objectToObservers)];
   int removeCount=0;

   if(object!=nil){
    NSMutableArray *observers=NSMapGet(_objectToObservers,object);
    NSInteger             count=[observers count];

    while(--count>=0)
     if(anObserver==[[observers objectAtIndex:count] observer])
      [observers removeObjectAtIndex:count];

    if([observers count]==0)
     removeKeys[removeCount++]=object;
   }
   else {
    NSMapEnumerator state=NSEnumerateMapTable(_objectToObservers);
    id              key;
    NSMutableArray *observers;

    while(NSNextMapEnumeratorPair(&state,(void **)&key,(void **)&observers)){
     NSInteger count=[observers count];

     while(--count>=0)
      if(anObserver==[[observers objectAtIndex:count] observer])
       [observers removeObjectAtIndex:count];

     if([observers count]==0)
      removeKeys[removeCount++]=key;
    }
   }

   while(--removeCount>=0)
    NSMapRemove(_objectToObservers,removeKeys[removeCount]);
}

-(void)postNotification:(NSNotification *)note {
// FIXME: NSNotificationCenter sends notifications in the order they are added for observation regardless of
// the object registered. This implementation stores objects for observation seperately so if you observe nil
// and a particular object you will always get the particular object notifications before the nil one instead
// of in the order they are registered.

// The copy and double check for presence is to deal with observers being removed during notification
   id         object=[note object];
   NSArray   *observers;
   NSInteger  count;
    
   if(object!=nil){
    observers=[NSArray arrayWithArray:(id)NSMapGet(_objectToObservers,object)];

    count=[observers count];
    while(--count>=0){
     id observer=[observers objectAtIndex:count];

     if([(NSArray *)NSMapGet(_objectToObservers,object) indexOfObjectIdenticalTo:observer]!=NSNotFound)
      [observer performSelector:_cmd withObject:note];
    }
   }

   observers=[NSArray arrayWithArray:(id)NSMapGet(_objectToObservers,[NSNull null])];

   count=[observers count];
   while(--count>=0){
    id observer=[observers objectAtIndex:count];

    if([(NSArray *)NSMapGet(_objectToObservers,[NSNull null]) indexOfObjectIdenticalTo:observer]!=NSNotFound)
     [observer performSelector:_cmd withObject:note];
   }
}

-(NSUInteger)count {
   return NSCountMapTable(_objectToObservers);
}

@end
