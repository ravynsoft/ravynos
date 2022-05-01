/*
 * Copyright (c) 2006-2007 Christopher J. W. Lloyd
 * Copyright (c) 2009 Vladimir Kirillov <proger@hackndev.com>
 * Copyright (c) 2021 Zoe Knox
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
#include <time.h>
#include <unistd.h>

NSString	*NSPlatformClassName = @"NSPlatform_ravynOS";

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

