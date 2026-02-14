/*
 * Copyright (c) 2009-2010 Mark Heily <mark@heily.com>
 * All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "getprogname.h"

#if !HAVE_GETPROGNAME

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif /* _WIN32_WINNT */
#include <windows.h>
#include <stdlib.h>

static INIT_ONCE getprogname_init_once = INIT_ONCE_STATIC_INIT;
static TCHAR progname[_MAX_FNAME];

static BOOL CALLBACK
getprogname_init_once_handler(PINIT_ONCE InitOnce, PVOID Parameter,
	PVOID *lpContext)
{
	TCHAR path[MAX_PATH];
	DWORD length = GetModuleFileName(NULL, path, sizeof(path));
	
	if (length < 0) {
		progname[0] = '\0';
		return TRUE;
	} else {
		const char *filename;
		
		path[MAX_PATH - 1] = '\0';
		filename = strrchr(path, '\\');
		if (filename != NULL) {
			filename++;
		} else {
			filename = path;
		}
		strcpy_s(progname, sizeof(progname), filename);
		return TRUE;
	}
}

const char *
getprogname(void)
{
	(void)InitOnceExecuteOnce(&getprogname_init_once,
			getprogname_init_once_handler,
			NULL,
			NULL);
	return progname;
}
#endif /* _WIN32 */
#endif /* HAVE_GETPROGNAME */
