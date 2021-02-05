/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class NSData, NSArray;

FOUNDATION_EXPORT NSString *const NSFileHandleConnectionAcceptedNotification;
FOUNDATION_EXPORT NSString *const NSFileHandleDataAvailableNotification;
FOUNDATION_EXPORT NSString *const NSFileHandleReadCompletionNotification;
FOUNDATION_EXPORT NSString *const NSFileHandleReadToEndOfFileCompletionNotification;

FOUNDATION_EXPORT NSString *const NSFileHandleNotificationDataItem;
FOUNDATION_EXPORT NSString *const NSFileHandleNotificationFileHandleItem;

FOUNDATION_EXPORT NSString *const NSFileHandleNotificationMonitorModes;

FOUNDATION_EXPORT NSString *const NSFileHandleOperationException;

@interface NSFileHandle : NSObject

+ fileHandleForReadingAtPath:(NSString *)path;
+ fileHandleForWritingAtPath:(NSString *)path;
+ fileHandleForUpdatingAtPath:(NSString *)path;

+ fileHandleWithNullDevice;
+ fileHandleWithStandardInput;
+ fileHandleWithStandardOutput;
+ fileHandleWithStandardError;

- initWithFileDescriptor:(int)descriptor;
- initWithFileDescriptor:(int)descriptor closeOnDealloc:(BOOL)closeOnDealloc;

- (int)fileDescriptor;

- (void)closeFile;
- (void)synchronizeFile;

- (uint64_t)offsetInFile;
- (void)seekToFileOffset:(uint64_t)offset;
- (uint64_t)seekToEndOfFile;

- (NSData *)readDataOfLength:(NSUInteger)length;
- (NSData *)readDataToEndOfFile;
- (NSData *)availableData;

- (void)writeData:(NSData *)data;

- (void)truncateFileAtOffset:(uint64_t)offset;

- (void)readInBackgroundAndNotifyForModes:(NSArray *)modes;
- (void)readInBackgroundAndNotify;

- (void)readToEndOfFileInBackgroundAndNotifyForModes:(NSArray *)modes;
- (void)readToEndOfFileInBackgroundAndNotify;

- (void)acceptConnectionInBackgroundAndNotifyForModes:(NSArray *)modes;
- (void)acceptConnectionInBackgroundAndNotify;

- (void)waitForDataInBackgroundAndNotifyForModes:(NSArray *)modes;
- (void)waitForDataInBackgroundAndNotify;

@end
