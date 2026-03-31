/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef __LAUNCHD_H__
#define __LAUNCHD_H__

#include <mach/mach.h>
#include <mach/port.h>
#include "launch.h"
#include "bootstrap.h"
#include "runtime.h"

struct kevent;
struct conncb;

extern bool pid1_magic;
extern bool launchd_shutting_down;
extern bool fake_launchd_shutting_down;
extern bool network_up;
extern FILE *launchd_console;
extern uid_t launchd_uid;

void launchd_SessionCreate(void);
void launchd_shutdown(void);

enum {
	LAUNCHD_PERSISTENT_STORE_DB,
	LAUNCHD_PERSISTENT_STORE_LOGS,
};
char *launchd_copy_persistent_store(int type, const char *file);

int _fd(int fd);

void init_boot(bool sflag);
void init_pre_kevent(bool sflag);
void launchd_exit(int code)  __dead2;

#define launchd_assumes(e)      \
        (__builtin_expect(!(e), 0) ? _log_launchd_bug(0, __FILE__, __LINE__, #e), false : true)

void _log_launchd_bug(const char *rcs_rev, const char *path, unsigned int line, const char *test);


void update_ttys(void);
void catatonia(void);
bool init_check_pid(pid_t p);

pid_t launchd_fork(void);

extern bool uflag;

#define DEBUG_EXIT(x) \
	do { \
		syslog(LOG_EMERG | LOG_CONSOLE, "%s(%d):  about to exit", __FUNCTION__, __LINE__); \
		sleep(30); \
		launchd_exit(x); \
	} while (0)

#endif /* __LAUNCHD_H__ */
