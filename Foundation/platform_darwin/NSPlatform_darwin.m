/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef __APPLE__
#import <objc/runtime.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform_darwin.h>

#import <rpc/types.h>
#include <time.h>
#import <sys/param.h>
#include <netdb.h>
#include <unistd.h>
#include <crt_externs.h>
#import <sys/sysctl.h>

// Handy functions for extracting various values from sysctl.
// These should work on FreeBSD as well, though the sysctl names might be different.

static int int32SysctlByName(const char *sysctlName) {
  int32_t sysctlInt32Value = 0; size_t len = sizeof(int32_t);
  sysctlbyname(sysctlName, &sysctlInt32Value, &len, NULL, 0);
  return(sysctlInt32Value);
}

static int64_t int64SysctlByName(const char *sysctlName) {
  int64_t sysctlInt64Value = 0; size_t len = sizeof(int64_t);
  sysctlbyname(sysctlName, &sysctlInt64Value, &len, NULL, 0);
  return(sysctlInt64Value);
}

static NSString *stringSysctlByName(const char *sysctlName) {
  char sysctlBuffer[1024]; size_t len = 1020; memset(sysctlBuffer, 0, 1024);
  sysctlbyname(sysctlName, &sysctlBuffer[0], &len, NULL, 0);
  return([NSString stringWithUTF8String:sysctlBuffer]);
}


NSString *NSPlatformClassName=@"NSPlatform_darwin";

@implementation NSPlatform_darwin

// nanosleep() is IEEE Std 1003.1b-1993, POSIX.1
// This can probably move down to NSPlatform_posix

void NSPlatformSleepThreadForTimeInterval(NSTimeInterval interval) {
  double intervalIntegralPart, intervalFractionalPart;
  struct timespec intervalTimeSpec;
  
  intervalFractionalPart   = modf((double)interval, &intervalIntegralPart);
  intervalTimeSpec.tv_sec  = (long)(intervalIntegralPart);
  intervalTimeSpec.tv_nsec = (long)(intervalFractionalPart * 1.0E9);

  nanosleep(&intervalTimeSpec, NULL);
}

/*
 SVr4,  4.4BSD   (this  function first appeared in 4.2BSD).
 POSIX.1 does  not  define  these  functions,  but  ISO/IEC
 9945-1:1990 mentions them in B.4.4.1.
 */
-(NSString *)hostName {
    char buf[MAXHOSTNAMELEN];
    gethostname(buf, MAXHOSTNAMELEN);
    return [NSString stringWithCString:buf];
}

-(NSString *)DNSHostName {
    // if we wanted to get crazy, we could open a dummy socket
    // and then get its local address, the do a gethostbyaddr on that...
    return [self hostName];
}

NSString * const NSPlatformExecutableDirectory=@"Darwin";
NSString * const NSPlatformResourceNameSuffix=@"darwin";

NSString * const NSPlatformExecutableFileExtension=@"";
NSString * const NSPlatformLoadableObjectFileExtension=@"dylib";
NSString * const NSPlatformLoadableObjectFilePrefix=@"";

- (NSUInteger)processorCount
{
  return((NSUInteger)int32SysctlByName("hw.ncpu"));
}

- (NSUInteger)activeProcessorCount
{
  return((NSUInteger)int32SysctlByName("hw.activecpu"));
}

-(uint64_t)physicalMemory {
  return((uint64_t)int64SysctlByName("hw.memsize"));
}

-(NSUInteger)operatingSystem {
  return(NSMACHOperatingSystem);
}

-(NSString *)operatingSystemName {
  return(@"NSMACHOperatingSystem");
}

-(NSString *)operatingSystemVersionString {
  static NSString *operatingSystemVersionString = NULL;
  
  if(operatingSystemVersionString == NULL) {  
    NSDictionary *operatingSystemVersionDictionary = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    if(operatingSystemVersionDictionary == NULL) { operatingSystemVersionDictionary = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/ServerVersion.plist"]; }
    if(operatingSystemVersionDictionary != NULL) {
      operatingSystemVersionString = [[NSString alloc] initWithFormat:@"Version %@ (Build %@)", [operatingSystemVersionDictionary objectForKey:@"ProductVersion"], [operatingSystemVersionDictionary objectForKey:@"ProductBuildVersion"]];
    } else {
      operatingSystemVersionString = [[NSString alloc] initWithFormat:@"%@ Version %@ (Build %@)", stringSysctlByName("kern.ostype"), stringSysctlByName("kern.osrelease"), stringSysctlByName("kern.osversion")];
    }
  }
  
  return(operatingSystemVersionString);
}

@end

char **NSPlatform_environ() {   
   return *_NSGetEnviron();
}
#endif

