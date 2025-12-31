/*
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <time.h>
#include <asl.h>
#include <asl_msg.h>
#include <asl_core.h>
#include <asl_private.h>
#include <asl_store.h>
#include "asl_memory.h"
#include "asl_common.h"
#include <notify.h>
#include <notify_keys.h>
#include <launch.h>
#include <dispatch/dispatch.h>
#include <libkern/OSAtomic.h>

#include <TargetConditionals.h>

#define ADDFD_FLAGS_LOCAL 0x00000001

#define SELF_DB_NOTIFICATION "self.logger.message"

#define ASL_OPT_IGNORE "ignore"
#define ASL_OPT_STORE "store"
#define ASL_OPT_CONTROL "control"
#define ASL_OPT_DB_FILE "asl_db_file"
#define ASL_OPT_DB_MEMORY "asl_db_memory"

#if TARGET_OS_SIMULATOR
/* These paths are appropriately prefixed in the simulator */
extern const char *_path_pidfile;
extern const char *_path_syslogd_log;

#define _PATH_PIDFILE _path_pidfile
#define _PATH_SYSLOGD_LOG _path_syslogd_log
#else
#define _PATH_PIDFILE		"/var/run/syslog.pid"
#define _PATH_SYSLOG_CONF   "/etc/syslog.conf"
#define _PATH_SYSLOG_IN		"/var/run/syslog"
#define _PATH_KLOG			"/dev/klog"
#define _PATH_SYSLOGD_LOG	"/var/log/syslogd.log"
#endif

#define NOTIFY_PATH_SERVICE "com.apple.system.notify.service.path:0x87:"

#define DB_TYPE_FILE	0x00000001
#define DB_TYPE_MEMORY	0x00000002

#define KERN_DISASTER_LEVEL 3

#define SOURCE_UNKNOWN      0
#define SOURCE_INTERNAL     1
#define SOURCE_BSD_SOCKET   2
#define SOURCE_UDP_SOCKET   3
#define SOURCE_KERN         4
#define SOURCE_ASL_MESSAGE  5
#define SOURCE_LAUNCHD      6

#define SOURCE_SESSION    100 /* does not generate messages */

#define STORE_FLAGS_FILE_CACHE_SWEEP_REQUESTED 0x00000001

#define FS_TTL_SEC 31622400

#define SEC_PER_DAY 86400

typedef struct sender_stats_s
{
	char *sender;
	uint64_t count;
	uint64_t size;
	struct sender_stats_s *next;
} sender_stats_t;

typedef struct
{
	uint32_t mcount;
	uint32_t shim_count;
	uint32_t bucket_count;
	sender_stats_t **bucket;
} stats_table_t;

typedef struct
{
	const char *name;
	int enabled;
	int (*init)(void);
	int (*reset)(void);
	int (*close)(void);
} module_t;

struct global_s
{
	OSSpinLock lock;
	int client_count;
	int disaster_occurred;
	mach_port_t listen_set;
	mach_port_t server_port;
	mach_port_t dead_session_port;
	launch_data_t launch_dict;
	int *lockdown_session_fds;
	int lockdown_session_count;
	int watchers_active;
	int reset;
	pid_t pid;
	int32_t work_queue_count;
	/* memory_size must be aligned for OSAtomicAdd64 */
	__attribute__((aligned(8))) int64_t memory_size;
	int32_t asl_queue_count;
	int32_t bsd_queue_count;
	pthread_mutex_t *db_lock;
	pthread_cond_t work_queue_cond;
	dispatch_queue_t work_queue;
	dispatch_source_t mark_timer;
	dispatch_source_t sig_hup_src;
	asl_store_t *file_db;
	asl_memory_t *memory_db;
	asl_memory_t *disaster_db;
	int module_count;
	int bsd_out_enabled;
	int launchd_enabled;
	module_t **module;
	asl_out_module_t *asl_out_module;
	time_t stats_last;
	stats_table_t *stats;

	/* parameters below are configurable as command-line args or in /etc/asl.conf */
	int debug;
	char *debug_file;
	char *hostname;
	int dbtype;
	uint32_t db_file_max;
	uint32_t db_memory_max;
	uint32_t db_memory_str_max;
	uint32_t mps_limit;
	uint32_t remote_delay_time;
	uint64_t bsd_max_dup_time;
	uint64_t mark_time;
	time_t utmp_ttl;
	time_t stats_interval;
	int64_t memory_max;
};

extern struct global_s global;

void init_globals(void);

void config_debug(int enable, const char *path);
void config_data_store(int type, uint32_t file_max, uint32_t memory_max, uint32_t str_memory_max);

void asl_mark(void);
void asl_archive(void);

void asl_client_count_increment();
void asl_client_count_decrement();

int asldebug(const char *, ...);
int internal_log_message(const char *str);

void send_to_direct_watchers(asl_msg_t *msg);

#if !TARGET_OS_SIMULATOR
void launchd_callback();
#endif

int asl_syslog_faciliy_name_to_num(const char *fac);
const char *asl_syslog_faciliy_num_to_name(int num);
asl_msg_t *asl_input_parse(const char *in, int len, char *rhost, uint32_t source);

void process_message(asl_msg_t *msg, uint32_t source);
void asl_out_message(asl_msg_t *msg, int64_t msize);
void bsd_out_message(asl_msg_t *msg, int64_t msize);
int control_set_param(const char *s, bool eval);
int asl_action_control_set_param(const char *s);
void asl_action_out_module_query(asl_msg_t *q, asl_msg_t *m, bool all);

/* notify SPI */
uint32_t notify_register_plain(const char *name, int *out_token);

#endif /* __DAEMON_H__ */
