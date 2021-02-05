/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <objc/runtime.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform_solaris.h>

#import <rpc/types.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>

NSString *NSPlatformClassName=@"NSPlatform_solaris";

@implementation NSPlatform_solaris

/*
 BSD  4.3.  The SUSv2 version returns int, and this is also
 the prototype used by glibc 2.2.2.  Only the EINVAL  error
 return is documented by SUSv2.
 */
void NSPlatformSleepThreadForTimeInterval(NSTimeInterval interval) {
    if (interval <= 0.0)
        return;

    if (interval > 1.0)
        sleep((unsigned int) interval);
    else {
     unsigned long value= 1000.0*interval;
     poll(NULL,0,value);
    }
}

/*
 SVr4,  4.4BSD   (this  function first appeared in 4.2BSD).
 POSIX.1 does  not  define  these  functions,  but  ISO/IEC
 9945-1:1990 mentions them in B.4.4.1.
 */
-(NSString *)hostName {
    char buf[MAXHOSTNAMELEN];
    gethostname(buf, MAXHOSTNAMELEN);
    return [NSString stringWithUTF8String:buf];
}

-(NSString *)DNSHostName {
    // if we wanted to get crazy, we could open a dummy socket
    // and then get its local address, the do a gethostbyaddr on that...
    return [self hostName];
}

NSString * const NSPlatformExecutableDirectory=@"Solaris";
NSString * const NSPlatformResourceNameSuffix=@"solaris";

NSString * const NSPlatformExecutableFileExtension=@"";
NSString * const NSPlatformLoadableObjectFileExtension=@"so";
NSString * const NSPlatformLoadableObjectFilePrefix=@"lib";

@end

char **NSPlatform_environ() {
   extern char **environ;
   
   return environ;
}

