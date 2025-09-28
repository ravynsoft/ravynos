/*
 * Copyright (c) 2006-2007 Christopher J. W. Lloyd
 * Copyright (c) 2009 Vladimir Kirillov <proger@hackndev.com>
 * Copyright (c) 2021-2022 Zoe Knox
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
 * Original - David Young <daver@geeks.org>
 * based on NSPlatform_linux port
 */

#import <objc/runtime.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform_ravynOS.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <time.h>
#include <unistd.h>

NSString	*NSPlatformClassName = @"NSPlatform_ravynOS";

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


@implementation NSPlatform_ravynOS

void NSPlatformSleepThreadForTimeInterval(NSTimeInterval interval) {
	 if (interval <= 0.0)
		  return;

	 if (interval > 1.0)
		  sleep((unsigned int)interval);
	 else 
		  usleep((unsigned long)(1000000.0 * interval));
}

- (NSString *)hostName
{
	 char	buf[MAXHOSTNAMELEN];

	 gethostname(buf, MAXHOSTNAMELEN);
	 return [NSString stringWithCString:buf];
}

- (NSString *)DNSHostName
{
	 return [self hostName];
}

- (NSUInteger)processorCount
{
  return((NSUInteger)int32SysctlByName("hw.ncpu"));
}

- (NSUInteger)activeProcessorCount
{
  return((NSUInteger)int32SysctlByName("hw.ncpu"));
}

-(uint64_t)physicalMemory {
  return((uint64_t)int64SysctlByName("hw.physmem"));
}

-(NSUInteger)operatingSystem {
  return(NSMACHOperatingSystem);
}

-(NSString *)operatingSystemName {
  return(@"ravynOS");
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

-(NSString *)CPUModel
{
    int mib[2];
    char model[128];
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_MODEL;
    len = sizeof(model)*sizeof(char);
    sysctl((const int *)mib, 2, model, &len, NULL,	0);
    return [[NSString stringWithUTF8String:model] autorelease];
}

-(NSString *)hostUUID
{
    int mib[2];
    char uuid[64];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_HOSTUUID;
    len = sizeof(uuid)*sizeof(char);
    sysctl((const int *)mib, 2, uuid, &len, NULL,	0);
    return [[NSString stringWithUTF8String:uuid] autorelease];
}


NSString * const NSPlatformExecutableDirectory=@"ravynOS";
NSString * const NSPlatformResourceNameSuffix=@"ravynOS";

NSString * const NSPlatformExecutableFileExtension=@"";
NSString * const NSPlatformLoadableObjectFileExtension=@"so";
NSString * const NSPlatformLoadableObjectFilePrefix=@"lib";


@end

char **NSPlatform_environ()
{	
	extern char **environ;
	return environ;
}

