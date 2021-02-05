/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSSocket.h>
#import "NSFileHandle_stream.h"

NSString * const NSFileHandleConnectionAcceptedNotification = @"NSFileHandleConnectionAcceptedNotification";
NSString * const NSFileHandleDataAvailableNotification = @"NSFileHandleDataAvailableNotification";
NSString * const NSFileHandleReadCompletionNotification = @"NSFileHandleReadCompletionNotification";
NSString * const NSFileHandleReadToEndOfFileCompletionNotification = @"NSFileHandleReadToEndOfFileCompletionNotification";

NSString * const NSFileHandleNotificationDataItem = @"NSFileHandleNotificationDataItem";
NSString * const NSFileHandleNotificationFileHandleItem = @"NSFileHandleNotificationFileHandleItem";

NSString * const NSFileHandleNotificationMonitorModes = @"NSFileHandleNotificationMonitorModes";

NSString * const NSFileHandleOperationException = @"NSFileHandleOperationException";

@interface NSFileHandle(ImplementedInPlatform)
+(Class)concreteSubclass;
@end

@implementation NSFileHandle

+allocWithZone:(NSZone *)zone {
   if(self==[NSFileHandle class])
    return NSAllocateObject([self concreteSubclass],0,NULL);

   return NSAllocateObject(self,0,zone);
}

+fileHandleForReadingAtPath:(NSString *)path {
   return [[self concreteSubclass] fileHandleForReadingAtPath:path];
}

+fileHandleForWritingAtPath:(NSString *)path {
   return [[self concreteSubclass] fileHandleForWritingAtPath:path];
}

+fileHandleForUpdatingAtPath:(NSString *)path {
   return [[self concreteSubclass] fileHandleForUpdatingAtPath:path];
}

+fileHandleWithNullDevice {
   return [[self concreteSubclass] fileHandleWithNullDevice];
}

+fileHandleWithStandardInput {
   return [[self concreteSubclass] fileHandleWithStandardInput];
}

+fileHandleWithStandardOutput {
   return [[self concreteSubclass] fileHandleWithStandardOutput];
}

+fileHandleWithStandardError {
   return [[self concreteSubclass] fileHandleWithStandardError];
}

-initWithFileDescriptor:(int)descriptor {
    return [self initWithFileDescriptor:descriptor closeOnDealloc:YES];
}

-initWithFileDescriptor:(int)descriptor closeOnDealloc:(BOOL)closeOnDealloc {
   NSSocket *socket=[[[NSSocket alloc] initWithFileDescriptor:descriptor] autorelease];
   
   [self dealloc];
   if(socket==nil)
    return nil;

   return [[NSFileHandle_stream alloc] initWithSocket:socket closeOnDealloc:closeOnDealloc];
}

-(int)fileDescriptor {
   NSInvalidAbstractInvocation();
   return -1;
}

-(void)closeFile {
   NSInvalidAbstractInvocation();
}

-(void)synchronizeFile {
   NSInvalidAbstractInvocation();
}

-(uint64_t)offsetInFile {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)seekToFileOffset:(uint64_t)offset {
   NSInvalidAbstractInvocation();
}

-(uint64_t)seekToEndOfFile {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSData *)readDataOfLength:(NSUInteger)length {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSData *)readDataToEndOfFile {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSData *)availableData {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)writeData:(NSData *)data {
   NSInvalidAbstractInvocation();
}

-(void)truncateFileAtOffset:(uint64_t)offset {
   NSInvalidAbstractInvocation();
}

-(void)readInBackgroundAndNotifyForModes:(NSArray *)modes {
   NSInvalidAbstractInvocation();
}

-(void)readInBackgroundAndNotify {
   [self readInBackgroundAndNotifyForModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

-(void)readToEndOfFileInBackgroundAndNotifyForModes:(NSArray *)modes {
   NSInvalidAbstractInvocation();
}

-(void)readToEndOfFileInBackgroundAndNotify {
   [self readToEndOfFileInBackgroundAndNotifyForModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

-(void)acceptConnectionInBackgroundAndNotifyForModes:(NSArray *)modes {
   NSInvalidAbstractInvocation();
}

-(void)acceptConnectionInBackgroundAndNotify {
   [self acceptConnectionInBackgroundAndNotifyForModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

-(void)waitForDataInBackgroundAndNotifyForModes:(NSArray *)modes {
   NSInvalidAbstractInvocation();
}

-(void)waitForDataInBackgroundAndNotify {
   [self waitForDataInBackgroundAndNotifyForModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

@end
