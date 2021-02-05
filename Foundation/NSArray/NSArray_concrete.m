/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSArray_concrete.h"
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

@implementation NSArray_concrete

static inline NSArray_concrete *newWithCount(NSZone *zone,NSUInteger count){
   return NSAllocateObject([NSArray_concrete class],sizeof(id)*count,zone);
}

NSArray *NSArray_concreteNewWithCount(NSZone *zone,id **objects,NSUInteger count){
   NSArray_concrete *self=newWithCount(zone,count);

   self->_count=count;
   *objects=self->_objects;

   return self;
}

NSArray *NSArray_concreteNew(NSZone *zone,id *objects,NSUInteger count) {
   NSArray_concrete *self=newWithCount(zone,count);
   NSUInteger         i;

   self->_count=count;
   for(i=0;i<count;i++)
    self->_objects[i]=[objects[i] retain];

   return self;
}

NSArray *NSArray_concreteWithArrayAndObject(NSArray *array,id object) {
   NSUInteger         i,count=[array count];
   NSArray_concrete *self=newWithCount(NULL,count+1);

   self->_count=count+1;

   [array getObjects:self->_objects];
   for(i=0;i<count;i++)
    [self->_objects[i] retain];

   self->_objects[count]=[object retain];

   return self;
}

NSArray *NSArray_concreteWithArrayAndArray(NSArray *array1,NSArray *array2) {
   NSUInteger         i,count1=[array1 count],total=count1+[array2 count];
   NSArray_concrete *self=newWithCount(NULL,total);

   self->_count=total;

   [array1 getObjects:self->_objects];
   [array2 getObjects:self->_objects+count1];
   for(i=0;i<total;i++)
    [self->_objects[i] retain];

   return self;
}

NSArray *NSArray_concreteWithArrayRange(NSArray *array,NSRange range) {
   NSArray_concrete *self=newWithCount(NULL,range.length);
   NSUInteger         i;

   self->_count=range.length;

   [array getObjects:self->_objects range:range];

   for(i=0;i<range.length;i++)
    [self->_objects[i] retain];

   return self;
}

-(void)dealloc {
   NSInteger count=_count;

   while(--count>=0)
    [_objects[count] release];

   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)count { return _count; }

-objectAtIndex:(NSUInteger)index {
   if(index>=_count){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond count %d",index,_count);
    return nil;
   }
   
   return _objects[index];
}

-lastObject {
   if(_count==0)
    return nil;

   return _objects[_count-1];
}

-(NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)stackbuf count:(NSUInteger)length;
{
   if(state->state>=_count)
    return 0;
   
   state->itemsPtr=_objects;
   state->state=_count;
   
   state->mutationsPtr=(unsigned long*)self;
	
   return _count;
}

@end
