/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableSet_concrete.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator_set.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSMutableSet_concrete

NSSet *NSMutableSet_concreteNew(NSZone *zone,NSUInteger capacity) {
   NSMutableSet_concrete *self=NSAllocateObject([NSMutableSet_concrete class],0,zone);
    if (self) {
        NSSetTableInit(&(self->_table),capacity,zone);
    }

   return self;
}

NSSet *NSMutableSet_concreteNewWithObjects(NSZone *zone,id *objects,
  NSUInteger count) {
   NSMutableSet_concrete *self=NSAllocateObject([NSMutableSet_concrete class],0,zone);
    if (self) {
       NSUInteger i;

       NSSetTableInit(&(self->_table),count,zone);
       for(i=0;i<count;i++)
        NSSetTableAddObjectNoGrow(&(self->_table),objects[i]);
    }
   return self;
}

NSSet *NSMutableSet_concreteNewWithArray(NSZone *zone,NSArray *array) {
   NSUInteger count=[array count];
   id       objects[count];

   [array getObjects:objects];

   return NSMutableSet_concreteNewWithObjects(zone,objects,count);
}

-initWithCapacity:(NSUInteger)capacity {
   NSSetTableInit(&_table,capacity,[self zone]);
   return self;
}

-(void)dealloc {
   NSSetTableFreeObjects(&_table);
   NSSetTableFreeBuckets(&_table);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)count {
   return _table.count;
}

-member:object {
   return NSSetTableMember(&_table,object);
}

-(NSEnumerator *)objectEnumerator {
   return NSAutorelease(NSEnumerator_setNew(NULL,self,&_table));
}

-(void)addObject:object {
   NSSetTableAddObject(&_table,object);
}

-(void)removeObject:object {
   NSSetTableRemoveObject(&_table,object);
}

#if 0
-(NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)stackbuf count:(NSUInteger)length {
    if(_table.count==0)
     return 0;
    if(state->state==(unsigned long)self)
     return 0;
     
	state->itemsPtr=stackbuf;
	state->mutationsPtr=(unsigned long*)self;
    
    NSEnumerator_set *enumerator;
    
	if((enumerator=(NSEnumerator_set *)state->state)==0)
     enumerator=NSEnumerator_setNew(NULL,self,&_table);
 	
	NSInteger i;

	for(i=0; i<length; i++)
     if((state->itemsPtr[i]=NSEnumerator_setNextObject(enumerator))==nil)
      break;

    if(state->state==0){

     if(i<length){
      state->state=(unsigned long)self;
      [enumerator release];
     }
     else {
      state->state=(unsigned long)enumerator;
      [enumerator autorelease];
     }
    }
    
	return i;
}
#endif

@end
