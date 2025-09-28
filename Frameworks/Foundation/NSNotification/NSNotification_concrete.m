/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSNotification_concrete.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSRaise.h>

@implementation NSNotification_concrete


NSNotification_concrete *NSNotification_concreteNew(NSZone *zone,
  NSString *name,id object,NSDictionary *userInfo) {
   NSNotification_concrete *self=NSAllocateObject([NSNotification_concrete class],0,zone);
    if (self) {
       self->_name=[name copyWithZone:zone];
       self->_object=[object retain];
       self->_userInfo=[userInfo retain];
    }
   return self;  
}

-initWithName:(NSString *)name object:object userInfo:(NSDictionary *)userInfo {
   _name=[name copy];
   _object=[object retain];
   _userInfo=[userInfo retain];
   return self;
}

-(void)dealloc {
   [_name release];
   [_object release];
   [_userInfo release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}


-(NSString *)name {
   return _name;
}


-object {
   return _object;
}


-(NSDictionary *)userInfo {
   return _userInfo;
}


-copyWithZone:(NSZone *)zone {
   return [self retain];
}

@end
