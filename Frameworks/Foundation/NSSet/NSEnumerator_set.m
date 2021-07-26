/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSEnumerator_set.h>
#import <Foundation/NSArray_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSEnumerator_set

NSEnumerator_set *NSEnumerator_setNew(NSZone *zone,id set,NSSetTable *table) {
   NSEnumerator_set *self=NSAllocateObject([NSEnumerator_set class],0,zone);
    if (self) {
       self->_set=[set retain];
       self->_table=table;
       self->_index=0;
       self->_bucket=NULL;

       for(;self->_index<self->_table->numBuckets;self->_index++)
        if((self->_bucket=self->_table->buckets[self->_index])!=NULL)
         break;
    }
   return self;
}

-(void)dealloc {
   [_set release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

id NSEnumerator_setNextObject(NSEnumerator_set *self){
   id object;

   if(self->_bucket==NULL)
    return nil;

   object=self->_bucket->key;

   if((self->_bucket=self->_bucket->next)==NULL){
    for(self->_index++;self->_index<self->_table->numBuckets;self->_index++)
     if((self->_bucket=self->_table->buckets[self->_index])!=NULL)
      break;
   }

   return object;
}

-nextObject {
   return NSEnumerator_setNextObject(self);
}

-(NSArray *)allObjects {
   NSMutableArray *array=[NSMutableArray array];
   id next;

   while((next=NSEnumerator_setNextObject(self))!=nil)
    [array addObject:next];

   return array;
}

@end
