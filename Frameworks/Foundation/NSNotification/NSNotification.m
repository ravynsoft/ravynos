/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNotification_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCoder.h>

@implementation NSNotification

+allocWithZone:(NSZone *)zone {
   if(self==[NSNotification class])
    return NSAllocateObject([NSNotification_concrete class],0,zone);

   return NSAllocateObject(self,0,zone);
}

-initWithName:(NSString *)name object:object userInfo:(NSDictionary *)userInfo {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithCoder:(NSCoder *)coder {
   NSString     *name=[coder decodeObject];
   id            object=[coder decodeObject];
   NSDictionary *userInfo=[coder decodeObject];

   return [self initWithName:name object:object userInfo:userInfo];
}

-copyWithZone:(NSZone *)zone {
   NSInvalidAbstractInvocation();
   return nil;
}

-(Class)classForCoder {
   return objc_lookUpClass("NSNotification");
}

-(void)encodeWithCoder:(NSCoder *)coder {
   [coder encodeObject:[self name]];
   [coder encodeObject:[self object]];
   [coder encodeObject:[self userInfo]];
}

+(NSNotification *)notificationWithName:(NSString *)name object:object {
   return NSAutorelease(NSNotification_concreteNew(NULL,name,object,nil));
}

+(NSNotification *)notificationWithName:(NSString *)name object:object
  userInfo:(NSDictionary *)userInfo {
   return NSAutorelease(NSNotification_concreteNew(NULL,name,object,userInfo));
}

-(NSString *)name {
   NSInvalidAbstractInvocation();
   return nil;
}

-object {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDictionary *)userInfo {
   NSInvalidAbstractInvocation();
   return nil;
}

@end
