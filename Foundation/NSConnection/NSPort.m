/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSPort.h>
#import <Foundation/NSRaise.h>

NSString * const NSPortDidBecomeInvalidNotification=@"NSPortDidBecomeInvalidNotification";

@implementation NSPort

+allocWithZone:(NSZone *)zone {
   if(self==[NSPort class])
    return NSAllocateObject([NSPort class],0,zone); // replace with NSMachPort
   else
    return NSAllocateObject(self,0,zone);
}

+(NSPort *)port {
   return [[[self alloc] init] autorelease];
}

-(id)delegate {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)setDelegate:delegate {
   NSInvalidAbstractInvocation();
}

-(void)invalidate {
   NSInvalidAbstractInvocation();
}

-(BOOL)isValid {
   NSInvalidAbstractInvocation();
   return NO;
}

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}


-(void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(void)addConnection:(NSConnection *)connection toRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(void)removeConnection:(NSConnection *)connection fromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(NSUInteger)reservedSpaceLength  {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)sendBeforeDate:(NSDate *)beforeDate components:(NSMutableArray *)components from:(NSPort *)fromPort reserved:(NSUInteger)reservedSpace  {
   NSInvalidAbstractInvocation();
   return NO;
}


-(BOOL)sendBeforeDate:(NSDate *)beforeData msgid:(NSUInteger)msgid components:(NSMutableArray *)components from:(NSPort *)fromPort reserved:(NSUInteger)reservedSpace  {
   NSInvalidAbstractInvocation();
   return NO;
}

@end
