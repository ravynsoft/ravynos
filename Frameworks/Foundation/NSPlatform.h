/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSError.h>

@class NSTimeZone, NSThread, NSInputSource, NSInputSourceSet, NSError;

FOUNDATION_EXPORT NSString *const NSPlatformExecutableFileExtension;
FOUNDATION_EXPORT NSString *const NSPlatformLoadableObjectFileExtension;
FOUNDATION_EXPORT NSString *const NSPlatformLoadableObjectFilePrefix;
FOUNDATION_EXPORT NSString *const NSPlatformExecutableDirectory;
FOUNDATION_EXPORT NSString *const NSPlatformResourceNameSuffix;

@interface NSPlatform : NSObject

+ currentPlatform;

- (NSInputSource *)parentDeathInputSource;

- (Class)taskClass;
- (Class)socketClass;
- (Class)socketPortClass;
- (Class)pipeClass;
- (Class)lockClass;
- (Class)recursiveLockClass;
- (Class)conditionLockClass;
- (Class)persistantDomainClass;
- (Class)timeZoneClass;
- (Class)conditionClass;

- (NSString *)userName;
- (NSString *)fullUserName;
- (NSString *)homeDirectory;
- (NSString *)libraryDirectory;
- (NSString *)temporaryDirectory;

- (NSArray *)arguments;
- (NSDictionary *)environment;

- (NSString *)hostName;

- (NSString *)DNSHostName;
- (NSArray *)addressesForDNSHostName:(NSString *)name;
- (NSString *)hostNameByAddress:(NSString *)address;

- (void *)mapContentsOfFile:(NSString *)path length:(NSUInteger *)length;
- (void)unmapAddress:(void *)ptr length:(NSUInteger)length;

- (BOOL)writeContentsOfFile:(NSString *)path bytes:(const void *)bytes length:(NSUInteger)length options:(NSUInteger)options error:(NSError **)errorp;

- (void)checkEnvironmentKey:(NSString *)key value:(NSString *)value;

- (NSUInteger)processorCount;
- (NSUInteger)activeProcessorCount;
- (uint64_t)physicalMemory;
- (NSUInteger)operatingSystem;
- (NSString *)operatingSystemName;
- (NSString *)operatingSystemVersionString;
- (NSString *)CPUModel;
- (NSString *)hostUUID;

@end

FOUNDATION_EXPORT int NSPlatformProcessorCount();
FOUNDATION_EXPORT int NSPlatformProcessID();
FOUNDATION_EXPORT NSUInteger NSPlatformThreadID();
FOUNDATION_EXPORT NSTimeInterval NSPlatformTimeIntervalSinceReferenceDate();
FOUNDATION_EXPORT void NSPlatformLogString(NSString *string);
FOUNDATION_EXPORT void NSPlatformSleepThreadForTimeInterval(NSTimeInterval interval);
FOUNDATION_EXPORT void *NSPlatformContentsOfFile(NSString *path, NSUInteger *length);

// These functions are implemented in the platform subproject

NSThread *NSPlatformCurrentThread();
void NSPlatformSetCurrentThread(NSThread *thread);
#ifdef WINDOWS
NSUInteger NSPlatformDetachThread(unsigned (*__stdcall func)(void *arg), void *arg, NSError **errorp);
#else
NSUInteger NSPlatformDetachThread(void *(*func)(void *arg), void *arg, NSError **errorp);
#endif
