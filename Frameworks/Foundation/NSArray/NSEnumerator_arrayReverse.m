/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSEnumerator_arrayReverse.h>
#import <Foundation/NSArray.h>

@implementation NSEnumerator_arrayReverse

NSEnumerator *NSEnumerator_arrayReverseNew(NSArray *array) {
   NSEnumerator_arrayReverse *self=NSAllocateObject([NSEnumerator_arrayReverse class],0,NULL);
    if (self) {
       self->_index=[array count];
       self->_array=[array retain];
    }
   return self;
}

-(void)dealloc {
   [_array release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-nextObject {
   _index--;

   if(_index<0)
    return nil;

   return [_array objectAtIndex:_index];
}

@end
