/*
 * Copyright (c) 2009 Vladimir Kirillov <proger@hackndev.com>
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

#import <Foundation/Foundation.h>

#include <unistd.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>
#endif

NSUInteger NSPageSize(void)
{
	return (NSUInteger)sysconf(_SC_PAGESIZE);
}

NSUInteger NSRealMemoryAvailable(void) 
{
#ifdef __FreeBSD__
   int mib[2];
   struct vmtotal vt;
   int len = sizeof(vt);
   mib[0] = CTL_VM;
   mib[1] = VM_TOTAL;
   sysctl(mib, 2, &vt, &len, NULL, 0);
   return (NSUInteger)vt.t_free;
#else
   return (NSUInteger)sysconf(_SC_AVPHYS_PAGES) * (NSUInteger)sysconf(_SC_PAGESIZE);
#endif
}
