/**
 * linuxshims.h
 * Author: Zoe Knox      Created: 2026-02-28
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 * SPDX: BSD-2-Clause
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
 * A collection of great dance songs, and some shims to make a _very_ stripped
 * down version of `kextcache` run on Linux. It's just enough to create a
 * prelinked kernel image.
 */

#ifndef _LINUX_SHIMS_H
#define _LINUX_SHIMS_H

#define HOST_BUILD 1

#define __deprecated __attribute__((deprecated))
#define __deprecated_enum_msg(x) __attribute__((deprecated))
#define __unused __attribute__((unused))
#define LS_PRIVATE extern __attribute__((visibility(("hidden"))))
#define __printflike(a, b) __attribute__((format(printf, a, b)))

#define _FORTIFY_SOURCE 0
#define __BYTE_ORDER __BYTE_ORDER__
#define __LP64__ 1
#define NO_BOOT_ROOT 1

#define MNAMELEN 1024
#define MAXCOMLEN 16
#define F_GETPATH 350
#define ELAST EHWPOISON /* highest errno value */
#define O_SYMLINK 0
#define MNT_RDONLY 1 /* MS_RDONLY in sys/mount.h on linux */

#include <Availability.h>
#include <stdarg.h>
#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <os/log.h>
#include <sys/vfs.h> /* struct statfs */

#define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
	(ts)->tv_sec = (tv)->tv_sec;                                    \
	(ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
	(tv)->tv_sec = (ts)->tv_sec;                                    \
	(tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}

typedef char * uuid_string_t;
typedef unsigned long long segsz_t;
typedef unsigned long fixpt_t;
typedef unsigned int os_signpost_id_t;
typedef int OSStatus;
typedef void * SecStaticCodeRef;
typedef void * SecCertificateRef;

LS_PRIVATE void os_log_with_args(os_log_t kextlog, int logtype,
				 char *fmt, va_list *args, void *addr);
LS_PRIVATE int basename_r(char *path, char **out);
LS_PRIVATE int dirname_r(char *path, char **out);

#endif /* _LINUX_SHIMS_H */
