/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSSet_concrete.h>
#import <Foundation/NSEnumerator_set.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSSet_concrete

NSSet *NSSet_concreteNew(NSZone *zone,id *objects,NSUInteger count) {
   NSUInteger       i,capacity=NSSetTableRoundCount(count);
   NSSet_concrete *self=NSAllocateObject([NSSet_concrete class],
     sizeof(NSSetBucket *)*capacity,zone);
    if (self) {
       self->_table.count=0;
       self->_table.numBuckets=capacity;
       self->_table.buckets=self->_buckets;

       for(i=0;i<count;i++)
        NSSetTableAddObjectNoGrow(&(self->_table),objects[i]);
    }

   return self;
}

-(void)dealloc {
   NSSetTableFreeObjects(&_table);
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

@end
