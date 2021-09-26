/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class NSArray, NSDictionary;

FOUNDATION_EXPORT NSString *const NSTaskDidTerminateNotification;

@interface NSTask : NSObject {
    NSString *launchPath;
    NSArray *arguments;
    NSString *currentDirectoryPath;
    NSDictionary *environment;

    id standardInput;
    id standardOutput;
    id standardError;
}

+ (NSTask *)launchedTaskWithLaunchPath:(NSString *)path arguments:(NSArray *)arguments;

- init;

- (NSString *)currentDirectoryPath;
- (NSString *)launchPath;
- (NSArray *)arguments;
- (NSDictionary *)environment;

- (id)standardError;
- (id)standardInput;
- (id)standardOutput;

- (void)setCurrentDirectoryPath:(NSString *)path;
- (void)setLaunchPath:(NSString *)path;
- (void)setArguments:(NSArray *)arguments;
- (void)setEnvironment:(NSDictionary *)values;

- (void)setStandardInput:(id)input;
- (void)setStandardOutput:(id)output;
- (void)setStandardError:(id)error;

- (void)launch;
- (BOOL)isRunning;

- (void)interrupt;

- (BOOL)suspend;
- (BOOL)resume;

- (void)terminate;
- (int)terminationStatus;

- (void)waitUntilExit;
- (void)blockAndWaitUntilExit;

- (int)processIdentifier;

@end
