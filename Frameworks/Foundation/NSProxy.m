/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSProxy.h>
#import <Foundation/NSException.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSString.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>
#import <objc/message.h>

@interface NSInvocation(private)
+(NSInvocation *)invocationWithMethodSignature:(NSMethodSignature *)signature arguments:(void *)arguments;
@end

@implementation NSProxy

+(void)load {
}


+(Class)class {
   return self;
}

/*
 FIXME: should we implement this? The Apple implementation does _not_ throw an exception, so we may not, either
+(BOOL)respondsToSelector:(SEL)selector {
   NSUnimplementedMethod();
   return NO;
}
 */

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject(self,0,zone);
}

+alloc {
   return [self allocWithZone:NULL];
}

-(void)dealloc {
   NSDeallocateObject((id)self);
}

-(void)finalize {
   // do nothing?
}

-(void)doesNotRecognizeSelector:(SEL)selector {
	[NSException raise:NSInvalidArgumentException
				format:@"%c[%@ %@]: selector not recognized", class_isMetaClass(isa)?'+':'-',
	 NSStringFromClass(isa),NSStringFromSelector(selector)];
}

-(NSMethodSignature *)methodSignatureForSelector:(SEL)selector {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)forwardInvocation:(NSInvocation *)invocation {
   NSInvalidAbstractInvocation();
}

-(id)forwardSelector:(SEL)selector arguments:(void *)arguments {
   NSMethodSignature *signature=[self methodSignatureForSelector:selector];

   if(signature==nil){
    [self doesNotRecognizeSelector:selector];
    return nil;
   }
   else {
    NSInvocation *invocation=[NSInvocation invocationWithMethodSignature:signature arguments:arguments];
   // char          result[[signature methodReturnLength]];
    id              result;

    [self forwardInvocation:invocation];
    [invocation getReturnValue:&result];

   // __builtin_return(result); Can we use __builtin_return like this? It still doesn't seem to work on float/doubles ?
    return result;
   }
}

-(NSUInteger)hash {
   return (NSUInteger)self>>4;
}

-(BOOL)isEqual:object {
   return (self==object);
}

-self {
   return self;
}


-(Class)class {
   return isa;
}


-(Class)superclass {
   return class_getSuperclass(isa);
}


-(NSZone *)zone {
   return NSZoneFromPointer(self);
}


- performSelector:(SEL)selector
{
#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)
    IMP imp = class_getMethodImplementation(object_getClass(self), selector);
#else
    IMP imp = objc_msg_lookup(self, selector);
#endif
    return imp(self, selector);
}


- performSelector:(SEL)selector withObject:object1
{
#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)
    IMP imp = class_getMethodImplementation(object_getClass(self), selector);
#else
    IMP imp = objc_msg_lookup(self, selector);
#endif
    return imp(self, selector, object1);
}


- performSelector:(SEL)selector withObject:object1 withObject:object2
{
#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)
    IMP imp = class_getMethodImplementation(object_getClass(self), selector);
#else
    IMP imp = objc_msg_lookup(self, selector);
#endif
    return imp(self, selector, object1, object2);
}


-(BOOL)isProxy {
   return YES;
}


-(BOOL)isKindOfClass:(Class)class {
   NSMethodSignature *signature=[self methodSignatureForSelector:_cmd];
   NSInvocation      *invocation=[NSInvocation
     invocationWithMethodSignature:signature];
   BOOL               returnValue;

   [self forwardInvocation:invocation];

   [invocation getReturnValue:&returnValue];

   return returnValue;
}


-(BOOL)isMemberOfClass:(Class)class {
   NSMethodSignature *signature=[self methodSignatureForSelector:_cmd];
   NSInvocation      *invocation=[NSInvocation
     invocationWithMethodSignature:signature];
   BOOL               returnValue;

   [self forwardInvocation:invocation];

   [invocation getReturnValue:&returnValue];

   return returnValue;
}


-(BOOL)conformsToProtocol:(Protocol *)protocol {
   NSMethodSignature *signature=[self methodSignatureForSelector:_cmd];
   NSInvocation      *invocation=[NSInvocation
     invocationWithMethodSignature:signature];
   BOOL               returnValue;

   [self forwardInvocation:invocation];

   [invocation getReturnValue:&returnValue];

   return returnValue;
}

-(BOOL)respondsToSelector:(SEL)selector {
   NSMethodSignature *signature=[self methodSignatureForSelector:_cmd];
   NSInvocation      *invocation=[NSInvocation
     invocationWithMethodSignature:signature];
   BOOL               returnValue;

   [self forwardInvocation:invocation];

   [invocation getReturnValue:&returnValue];

   return returnValue;
}

-autorelease {
   return NSAutorelease(self);
}


-(oneway void)release {
   if(NSDecrementExtraRefCountWasZero(self))
    [self dealloc];
}


-retain {
   NSIncrementExtraRefCount(self);
   return self;
}


-(NSUInteger)retainCount {
   return NSExtraRefCount(self);
}


-(NSString *)description {
   return NSStringWithFormat(@"<%@: 0x%0x>",NSStringFromClass(isa),self);
}

-(NSString *)debugDescription {
    return [self description];
}

@end
