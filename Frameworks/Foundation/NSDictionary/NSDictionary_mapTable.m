/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSDictionary_mapTable.h>
#import <Foundation/NSEnumerator_dictionaryKeys.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSCoder.h>

@implementation NSDictionary_mapTable

-(NSUInteger)count {
   return NSCountMapTable(_table);
}

-objectForKey:key {
   return NSMapGet(_table,key);
}

-(NSEnumerator *)keyEnumerator {
   return NSAutorelease(NSEnumerator_dictionaryKeysNew(_table));
}

-init {
   return [self initWithObjects:NULL forKeys:NULL count:0];
}

-initWithObjects:(id *)objects forKeys:(id *)keys count:(NSUInteger)count {
   NSInteger i;

   _table=NSCreateMapTableWithZone(NSObjectMapKeyCallBacks,
     NSObjectMapValueCallBacks,count,NULL);

   for(i=0;i<count;i++){
       if (keys[i]==nil){
           [self autorelease];
           NSRaiseException(NSInvalidArgumentException,self,_cmd,@"Attempt to insert object with nil key");
       }
       else if(objects[i]==nil){
           [self autorelease];
           NSRaiseException(NSInvalidArgumentException,self,_cmd,@"Attempt to insert nil object for key %@", keys[i]);
       }

    id key=[keys[i] copy];
    NSMapInsert(_table,key,objects[i]);
    [key release];
   }
   return self;
}

-(void)dealloc {
   if(_table!=NULL)
    NSFreeMapTable(_table);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

@end
