/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef LINUX
#import <Foundation/Foundation.h>
#import <sys/sysinfo.h>
#import <sys/unistd.h>

// on Linux, page size is a compiled constant in the kernel
NSUInteger NSPageSize(void) {
   return getpagesize();
}

NSUInteger NSRealMemoryAvailable(void) 
{
   struct sysinfo sysInfo;

   if (sysinfo(&sysInfo) != 0) // sysinfo is a pretty cool call
       return -1; // dox don't say what an error return value might be

   return sysInfo.freeram * sysInfo.mem_unit;
}
#endif
