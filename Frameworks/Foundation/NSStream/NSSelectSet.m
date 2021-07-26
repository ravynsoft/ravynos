/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSSelectSet.h>
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSRaise.h>

NSString * const NSSelectSetOutputNotification=@"NSSelectSetOutputNotification";

@implementation NSSelectSet

-init {
   _readSet=[NSMutableSet new];
   _writeSet=[NSMutableSet new];
   _exceptionSet=[NSMutableSet new];
   return self;
}

-(void)dealloc {
   [_readSet release];
   [_writeSet release];
   [_exceptionSet release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   NSSelectSet *copy=NSCopyObject(self,0,zone);
   
   copy->_readSet=[_readSet mutableCopy];
   copy->_writeSet=[_writeSet mutableCopy];
   copy->_exceptionSet=[_exceptionSet mutableCopy];
   
   return copy;
}

-(void)addObjectForRead:object {
   [_readSet addObject:object];
}

-(void)addObjectForWrite:object {
   [_writeSet addObject:object];
}

-(void)addObjectForException:object {
   [_exceptionSet addObject:object];
}

-(void)removeObjectForRead:object {
   [_readSet removeObject:object];
}

-(void)removeObjectForWrite:object {
   [_writeSet removeObject:object];
}

-(void)removeObjectForException:object {
   [_exceptionSet removeObject:object];
}

-(void)removeAllObjects {
   [_readSet removeAllObjects];
   [_writeSet removeAllObjects];
   [_exceptionSet removeAllObjects];
}

-(BOOL)isEmpty {
   return ([_readSet count]==0) && ([_writeSet count]==0) && ([_exceptionSet count]==0);
}

-(BOOL)containsObjectForRead:object {
   return [_readSet containsObject:object];
}

-(BOOL)containsObjectForWrite:object {
   return [_writeSet containsObject:object];
}

-(BOOL)containsObjectForException:object {
   return [_exceptionSet containsObject:object];
}

-(void)waitInBackgroundInMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(NSError *)waitForSelectWithOutputSet:(NSSelectSet **)outputSet beforeDate:(NSDate *)beforeDate {
   NSInvalidAbstractInvocation();
   return nil;
}

@end
