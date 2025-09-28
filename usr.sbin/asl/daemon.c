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

#include <TargetConditionals.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ucred.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SYSLOG_NAMES
#include <syslog.h>
//#include <sys/fslog.h>
#include <vproc.h>
#include <pthread.h>
#include <vproc_priv.h>
#include <mach/mach.h>
#include <libkern/OSAtomic.h>
#include <libproc.h>
#include <uuid/uuid.h>
#include "daemon.h"

#define LIST_SIZE_DELTA 256

#define forever for(;;)

#define ASL_MSG_TYPE_MASK 0x0000000f
#define ASL_TYPE_ERROR 2

#define ASL_KEY_FACILITY "Facility"

#define FACILITY_USER "user"
#define FACILITY_CONSOLE "com.apple.console"
#define SYSTEM_RESERVED "com.apple.system"
#define SYSTEM_RESERVED_LEN 16

#define VERIFY_STATUS_OK 0
#define VERIFY_STATUS_INVALID_MESSAGE 1
#define VERIFY_STATUS_EXCEEDED_QUOTA 2

extern void disaster_message(asl_msg_t *m);
extern int asl_action_reset(void);
extern int asl_action_control_set_param(const char *s);

static char myname[MAXHOSTNAMELEN + 1] = {0};
static int name_change_token = -1;

static OSSpinLock count_lock = 0;
static int aslmanager_triggered = 0;


#if !TARGET_OS_EMBEDDED
static vproc_transaction_t vproc_trans = {0};
#define DEFAULT_WORK_QUEUE_SIZE_MAX 10240000
#else
#define DEFAULT_WORK_QUEUE_SIZE_MAX 4096000
#endif

#define QUOTA_KERN_EXCEEDED_MESSAGE "*** kernel exceeded %d log message per second limit  -  remaining messages this second discarded ***"

#define DEFAULT_DB_FILE_MAX 25600000
#define DEFAULT_DB_MEMORY_MAX 512
#define DEFAULT_DB_MEMORY_STR_MAX 4096000
#define DEFAULT_MPS_LIMIT 500
#define DEFAULT_REMOTE_DELAY 5000
#define DEFAULT_BSD_MAX_DUP_SEC 30
#define DEFAULT_MARK_SEC 0
#define DEFAULT_UTMP_TTL_SEC 31622400

static time_t quota_time = 0;
static int32_t kern_quota;
static int32_t kern_level;

static const char *kern_notify_key[] = 
{
	"com.apple.system.log.kernel.emergency",
	"com.apple.system.log.kernel.alert",
	"com.apple.system.log.kernel.critical",
	"com.apple.system.log.kernel.error",
	"com.apple.system.log.kernel.warning",
	"com.apple.system.log.kernel.notice",
	"com.apple.system.log.kernel.info",
	"com.apple.system.log.kernel.debug"
};

static int kern_notify_token[8] = {-1, -1, -1, -1, -1, -1, -1, -1 };

static uint32_t
kern_quota_check(time_t now, asl_msg_t *msg, uint32_t level)
{
	char *str, lstr[2];

	if (msg == NULL) return VERIFY_STATUS_INVALID_MESSAGE;
	if (global.mps_limit == 0) return VERIFY_STATUS_OK;

	if (quota_time != now)
	{
		kern_quota = global.mps_limit;
		kern_level = 7;
		quota_time = now;
	}

	if (level < kern_level) kern_level = level;
	if (kern_quota > 0) kern_quota--;

	if (kern_quota > 0) return VERIFY_STATUS_OK;
	if (kern_quota < 0)	return VERIFY_STATUS_EXCEEDED_QUOTA;

	kern_quota = -1;

	str = NULL;
	asprintf(&str, QUOTA_KERN_EXCEEDED_MESSAGE, global.mps_limit);
	if (str != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_MSG, str);
		free(str);
		lstr[0] = kern_level + '0';
		lstr[1] = 0;
		asl_msg_set_key_val(msg, ASL_KEY_LEVEL, lstr);
	}

	return VERIFY_STATUS_OK;
}

static const char *
whatsmyhostname()
{
	static dispatch_once_t once;
	int check, status;

#if 0
//	dispatch_once(&once, ^{
//		snprintf(myname, sizeof(myname), "%s", "localhost");
//		notify_register_check(kNotifySCHostNameChange, &name_change_token);
//	});
#endif

	check = 1;
	status = 0;

	if (name_change_token >= 0) status = notify_check(name_change_token, &check);

	if ((status == 0) && (check == 0)) return (const char *)myname;

	if (gethostname(myname, MAXHOSTNAMELEN) < 0)
	{
		snprintf(myname, sizeof(myname), "%s", "localhost");
	}
	else
	{
		char *dot;
		dot = strchr(myname, '.');
		if (dot != NULL) *dot = '\0';
	}

	return (const char *)myname;
}

void
asl_client_count_increment()
{
	OSSpinLockLock(&count_lock);

#if !TARGET_OS_EMBEDDED
	if (global.client_count == 0) vproc_trans = vproc_transaction_begin(NULL);
#endif
	global.client_count++;

	OSSpinLockUnlock(&count_lock);
}

void
asl_client_count_decrement()
{
	OSSpinLockLock(&count_lock);

	if (global.client_count > 0) global.client_count--;
#if !TARGET_OS_EMBEDDED
	if (global.client_count == 0) vproc_transaction_end(NULL, vproc_trans);
#endif

	OSSpinLockUnlock(&count_lock);
}

/*
 * Checks message content and sets attributes as required
 *
 * SOURCE_INTERNAL log messages sent by syslogd itself
 * SOURCE_ASL_SOCKET legacy asl(3) TCP socket
 * SOURCE_BSD_SOCKET legacy syslog(3) UDP socket
 * SOURCE_UDP_SOCKET from the network
 * SOURCE_KERN from the kernel
 * SOURCE_ASL_MESSAGE mach messages sent from Libc by asl(3) and syslog(3)
 * SOURCE_LAUNCHD forwarded from launchd
 */

static uint32_t
aslmsg_verify(asl_msg_t *msg, uint32_t source, int32_t *kern_post_level, uid_t *uid_out)
{
	const char *val, *fac, *ruval, *rgval;
	char buf[64];
	time_t tick, now;
	uid_t uid;
	gid_t gid;
	uint32_t status, level, fnum;
	pid_t pid;
	uuid_string_t ustr;
	struct proc_uniqidentifierinfo pinfo;

	if (msg == NULL) return VERIFY_STATUS_INVALID_MESSAGE;

	/* Time */
	now = time(NULL);

	if (kern_post_level != NULL) *kern_post_level = -1;
	if (uid_out != NULL) *uid_out = -2;

	/* PID */
	pid = 0;

	val = asl_msg_get_val_for_key(msg, ASL_KEY_PID);
	if (val == NULL) asl_msg_set_key_val(msg, ASL_KEY_PID, "0");
	else pid = (pid_t)atoi(val);

	/* if PID is 1 (launchd), use the refpid if provided */
	if (pid == 1)
	{
		val = asl_msg_get_val_for_key(msg, ASL_KEY_REF_PID);
		if (val != NULL) pid = (pid_t)atoi(val);
	}

	/* Level */
	val = asl_msg_get_val_for_key(msg, ASL_KEY_LEVEL);
	level = ASL_LEVEL_DEBUG;
	if (source == SOURCE_KERN) level = ASL_LEVEL_NOTICE;

	if ((val != NULL) && (val[1] == '\0') && (val[0] >= '0') && (val[0] <= '7')) level = val[0] - '0';
	snprintf(buf, sizeof(buf), "%d", level);
	asl_msg_set_key_val(msg, ASL_KEY_LEVEL, buf);

	/* check kernel quota if enabled and no processes are watching */
	if ((pid == 0) && (global.mps_limit > 0) && (global.watchers_active == 0))
	{
		status = kern_quota_check(now, msg, level);
		if (status != VERIFY_STATUS_OK) return status;
	}

	if (pid != 0)
	{
		/* set Sender_Mach_UUID */
		uuid_clear(pinfo.p_uuid);
		if (proc_pidinfo(pid, PROC_PIDUNIQIDENTIFIERINFO, 1, &pinfo, sizeof(pinfo)) == sizeof(pinfo))
		{
			uuid_unparse(pinfo.p_uuid, ustr);
			asl_msg_set_key_val(msg, ASL_KEY_SENDER_MACH_UUID, ustr);
		}
	}

	tick = 0;
	val = asl_msg_get_val_for_key(msg, ASL_KEY_TIME);
	if (val != NULL) tick = asl_core_parse_time(val, NULL);

	/* Set time to now if it is unset or from the future (not allowed!) */
	if ((tick == 0) || (tick > now)) tick = now;

	/* Canonical form: seconds since the epoch */
	snprintf(buf, sizeof(buf) - 1, "%lu", tick);
	asl_msg_set_key_val(msg, ASL_KEY_TIME, buf);

	/* Host */
	val = asl_msg_get_val_for_key(msg, ASL_KEY_HOST);
	if (val == NULL) asl_msg_set_key_val(msg, ASL_KEY_HOST, whatsmyhostname());

	/* UID  & GID */
	uid = -2;
	val = asl_msg_get_val_for_key(msg, ASL_KEY_UID);
	if (val == NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_UID, "-2");
	}
	else
	{
		uid = atoi(val);
		if ((uid == 0) && strcmp(val, "0"))
		{
			uid = -2;
			asl_msg_set_key_val(msg, ASL_KEY_UID, "-2");
		}
	}

	if (uid_out != NULL) *uid_out = uid;

	gid = -2;
	val = asl_msg_get_val_for_key(msg, ASL_KEY_GID);
	if (val == NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_GID, "-2");
	}
	else
	{
		gid = atoi(val);
		if ((gid == 0) && strcmp(val, "0"))
		{
			gid = -2;
			asl_msg_set_key_val(msg, ASL_KEY_GID, "-2");
		}
	}

	switch (source)
	{
		case SOURCE_KERN:
		case SOURCE_INTERNAL:
		{
			uid = 0;
			asl_msg_set_key_val(msg, ASL_KEY_UID, "0");

			gid = 0;
			asl_msg_set_key_val(msg, ASL_KEY_GID, "0");

			break;
		}
		case SOURCE_UDP_SOCKET:
		case SOURCE_ASL_MESSAGE:
		case SOURCE_LAUNCHD:
		{
			break;
		}
		default:
		{
			/* do not trust the UID 0 or GID 0 or 80 in SOURCE_BSD_SOCKET or SOURCE_UNKNOWN */
			if (uid == 0)
			{
				uid = -2;
				asl_msg_set_key_val(msg, ASL_KEY_UID, "-2");
			}

			if ((gid == 0) || (gid == 80))
			{
				gid = -2;
				asl_msg_set_key_val(msg, ASL_KEY_GID, "-2");
			}
		}
	}

	/* Sender */
	val = asl_msg_get_val_for_key(msg, ASL_KEY_SENDER);
	if (val == NULL)
	{
		switch (source)
		{
			case SOURCE_KERN:
			{
				asl_msg_set_key_val(msg, ASL_KEY_SENDER, "kernel");
				break;
			}
			case SOURCE_INTERNAL:
			{
				asl_msg_set_key_val(msg, ASL_KEY_SENDER, "syslogd");
				break;
			}
			default:
			{
				asl_msg_set_key_val(msg, ASL_KEY_SENDER, "Unknown");
			}
		}
	}
	else if ((source != SOURCE_KERN) && (uid != 0) && (!strcmp(val, "kernel")))
	{
		/* allow UID 0 to send messages with "Sender kernel", but nobody else */
		asl_msg_set_key_val(msg, ASL_KEY_SENDER, "Unknown");
	}

	/* Facility */
	fac = asl_msg_get_val_for_key(msg, ASL_KEY_FACILITY);
	if (fac == NULL)
	{
		if (source == SOURCE_KERN) fac = "kern";
		else fac = "user";
		asl_msg_set_key_val(msg, ASL_KEY_FACILITY, fac);
	}
	else if (fac[0] == '#')
	{
		fnum = LOG_USER;
		if ((fac[1] >= '0') && (fac[1] <= '9'))
		{
			fnum = atoi(fac + 1) << 3;
			if ((fnum == 0) && (strcmp(fac + 1, "0"))) fnum = LOG_USER;
		}

		fac = asl_syslog_faciliy_num_to_name(fnum);
		asl_msg_set_key_val(msg, ASL_KEY_FACILITY, fac);
	}
	else if (!strncmp(fac, SYSTEM_RESERVED, SYSTEM_RESERVED_LEN))
	{
		/* only UID 0 may use "com.apple.system" */
		if (uid != 0) asl_msg_set_key_val(msg, ASL_KEY_FACILITY, FACILITY_USER);
	}

	/*
	 * kernel messages are only readable by root and admin group.
	 * all other messages are admin-only readable unless they already
	 * have specific read access controls set.
	 */
	if (source == SOURCE_KERN)
	{
		asl_msg_set_key_val(msg, ASL_KEY_READ_UID, "0");
		asl_msg_set_key_val(msg, ASL_KEY_READ_GID, "80");
	}
	else
	{
		ruval = asl_msg_get_val_for_key(msg, ASL_KEY_READ_UID);
		rgval = asl_msg_get_val_for_key(msg, ASL_KEY_READ_GID);

		if ((ruval == NULL) && (rgval == NULL))
		{
			asl_msg_set_key_val(msg, ASL_KEY_READ_GID, "80");
		}
	}

	/* Set DB Expire Time for com.apple.system.utmpx and lastlog */
	if ((!strcmp(fac, "com.apple.system.utmpx")) || (!strcmp(fac, "com.apple.system.lastlog")))
	{
		snprintf(buf, sizeof(buf), "%lu", tick + global.utmp_ttl);
		asl_msg_set_key_val(msg, ASL_KEY_EXPIRE_TIME, buf);
	}

#if 0
	/* Set DB Expire Time for Filesystem errors */
	if (!strcmp(fac, FSLOG_VAL_FACILITY))
	{
		snprintf(buf, sizeof(buf), "%lu", tick + FS_TTL_SEC);
		asl_msg_set_key_val(msg, ASL_KEY_EXPIRE_TIME, buf);
	}
#endif

	/*
	 * special case handling of kernel disaster messages
	 */
	if ((source == SOURCE_KERN) && (level <= KERN_DISASTER_LEVEL))
	{
		if (kern_post_level != NULL) *kern_post_level = level;
		disaster_message(msg);
	}

	return VERIFY_STATUS_OK;
}

void
list_append_msg(asl_msg_list_t *list, asl_msg_t *msg)
{
	if (list == NULL) return;
	if (msg == NULL) return;

	/*
	 * NB: curr is the list size
	 * grow list if necessary
	 */
	if (list->count == list->curr)
	{
		if (list->curr == 0)
		{
			list->msg = (asl_msg_t **)calloc(LIST_SIZE_DELTA, sizeof(asl_msg_t *));
		}
		else
		{
			list->msg = (asl_msg_t **)reallocf(list->msg, (list->curr + LIST_SIZE_DELTA) * sizeof(asl_msg_t *));
		}

		if (list->msg == NULL)
		{
			list->curr = 0;
			list->count = 0;
			return;
		}

		list->curr += LIST_SIZE_DELTA;
	}

	list->msg[list->count] = (asl_msg_t *)msg;
	list->count++;
}

void
init_globals(void)
{
	asl_out_rule_t *r;

	OSSpinLockLock(&global.lock);

	global.pid = getpid();
	global.debug = 0;
	free(global.debug_file);
	global.debug_file = NULL;
	global.launchd_enabled = 1;

#if TARGET_OS_EMBEDDED
	global.dbtype = DB_TYPE_MEMORY;
#else
	global.dbtype = DB_TYPE_FILE;
#endif
	global.db_file_max = DEFAULT_DB_FILE_MAX;
	global.db_memory_max = DEFAULT_DB_MEMORY_MAX;
	global.db_memory_str_max = DEFAULT_DB_MEMORY_STR_MAX;
	global.mps_limit = DEFAULT_MPS_LIMIT;
	global.remote_delay_time = DEFAULT_REMOTE_DELAY;
	global.bsd_max_dup_time = DEFAULT_BSD_MAX_DUP_SEC;
	global.mark_time = DEFAULT_MARK_SEC;
	global.utmp_ttl = DEFAULT_UTMP_TTL_SEC;
	global.max_work_queue_size = DEFAULT_WORK_QUEUE_SIZE_MAX;

	global.asl_out_module = asl_out_module_init();
	OSSpinLockUnlock(&global.lock);

	if (global.asl_out_module != NULL)
	{
		for (r = global.asl_out_module->ruleset; r != NULL; r = r->next)
		{
			if ((r->action == ACTION_SET_PARAM) && (r->query == NULL) && (!strncmp(r->options, "debug", 5))) control_set_param(r->options, true);
		}
	}
}

/*
 * Used to set config parameters.
 * Line format "= name value"
 */
int
control_set_param(const char *s, bool eval)
{
	char **l;
	uint32_t intval, count, v32a, v32b, v32c;

	if (s == NULL) return -1;
	if (s[0] == '\0') return 0;

	/* skip '=' and whitespace */
	if (*s == '=') s++;
	while ((*s == ' ') || (*s == '\t')) s++;

	l = explode(s, " \t");
	if (l == NULL) return -1;

	for (count = 0; l[count] != NULL; count++);

	/* name is required */
	if (count == 0)
	{
		free_string_list(l);
		return -1;
	}

	/* Check variables that allow 0 or 1 / boolean */
	if (!strcasecmp(l[0], "debug"))
	{
		/* = debug [0|1] [file] */
		if (count == 1)
		{
			intval = (eval) ? 1 : 0;
			config_debug(intval, NULL);
		}
		else if (!strcmp(l[1], "0"))
		{
			config_debug(0, l[2]);
		}
		else if (!strcmp(l[1], "1"))
		{
			config_debug(1, l[2]);
		}
		else
		{
			intval = (eval) ? 1 : 0;
			config_debug(intval, l[1]);
		}

		free_string_list(l);
		return 0;
	}

	/* value is required */
	if (count == 1)
	{
		free_string_list(l);
		return -1;
	}

	if (!strcasecmp(l[0], "mark_time"))
	{
		/* = mark_time seconds */
		OSSpinLockLock(&global.lock);
		if (eval) global.mark_time = atoll(l[1]);
		else global.mark_time = 0;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "dup_delay"))
	{
		/* = bsd_max_dup_time seconds */
		OSSpinLockLock(&global.lock);
		if (eval) global.bsd_max_dup_time = atoll(l[1]);
		else global.bsd_max_dup_time = DEFAULT_BSD_MAX_DUP_SEC;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "remote_delay"))
	{
		/* = remote_delay microseconds */
		OSSpinLockLock(&global.lock);
		if (eval) global.remote_delay_time = atol(l[1]);
		else global.remote_delay_time = DEFAULT_REMOTE_DELAY;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "utmp_ttl"))
	{
		/* = utmp_ttl seconds */
		OSSpinLockLock(&global.lock);
		if (eval) global.utmp_ttl = (time_t)atoll(l[1]);
		else global.utmp_ttl = DEFAULT_UTMP_TTL_SEC;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "mps_limit"))
	{
		/* = mps_limit number */
		OSSpinLockLock(&global.lock);
		if (eval) global.mps_limit = (uint32_t)atol(l[1]);
		else global.mps_limit = DEFAULT_MPS_LIMIT;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "max_work_queue_size"))
	{
		/* = max_work_queue_size number */
		OSSpinLockLock(&global.lock);
		if (eval) global.max_work_queue_size = (int64_t)atoll(l[1]);
		else global.max_work_queue_size = DEFAULT_WORK_QUEUE_SIZE_MAX;
		OSSpinLockUnlock(&global.lock);
	}
	else if (!strcasecmp(l[0], "max_file_size"))
	{
		/* = max_file_size bytes */
		pthread_mutex_lock(global.db_lock);

		if (global.dbtype & DB_TYPE_FILE)
		{
			asl_store_close(global.file_db);
			global.file_db = NULL;
			if (eval) global.db_file_max = atoi(l[1]);
			else global.db_file_max = DEFAULT_DB_FILE_MAX;
		}

		pthread_mutex_unlock(global.db_lock);
	}
	else if ((!strcasecmp(l[0], "db")) || (!strcasecmp(l[0], "database")) || (!strcasecmp(l[0], "datastore")))
	{
		/* NB this is private / unpublished */
		/* = db type [max]... */
		if (eval)
		{
			v32a = 0;
			v32b = 0;
			v32c = 0;

			if ((l[1][0] >= '0') && (l[1][0] <= '9'))
			{
				intval = atoi(l[1]);
				if ((count >= 3) && (strcmp(l[2], "-"))) v32a = atoi(l[2]);
				if ((count >= 4) && (strcmp(l[3], "-"))) v32b = atoi(l[3]);
				if ((count >= 5) && (strcmp(l[4], "-"))) v32c = atoi(l[4]);
			}
			else if (!strcasecmp(l[1], "file"))
			{
				intval = DB_TYPE_FILE;
				if ((count >= 3) && (strcmp(l[2], "-"))) v32a = atoi(l[2]);
			}
			else if (!strncasecmp(l[1], "mem", 3))
			{
				intval = DB_TYPE_MEMORY;
				if ((count >= 3) && (strcmp(l[2], "-"))) v32b = atoi(l[2]);
			}
			else
			{
				free_string_list(l);
				return -1;
			}

			if (v32a == 0) v32a = global.db_file_max;
			if (v32b == 0) v32b = global.db_memory_max;
			if (v32c == 0) v32c = global.db_memory_str_max;

			config_data_store(intval, v32a, v32b, v32c);
		}
		else
		{
#if TARGET_OS_EMBEDDED
			intval = DB_TYPE_MEMORY;
#else
			intval = DB_TYPE_FILE;
#endif
			config_data_store(intval, DEFAULT_DB_FILE_MAX, DEFAULT_DB_MEMORY_MAX, DEFAULT_DB_MEMORY_STR_MAX);
		}
	}

	free_string_list(l);
	return 0;
}

static int
control_message(asl_msg_t *msg)
{
	const char *str = asl_msg_get_val_for_key(msg, ASL_KEY_MSG);

	if (str == NULL) return 0;

	if (!strncmp(str, "= reset", 7))
	{
		init_globals();
		return asl_action_reset();
	}
	else if (!strncmp(str, "= crash", 7))
	{
		abort();
	}
	else if (!strncmp(str, "@ ", 2))
	{
		return asl_action_control_set_param(str);
	}
	else if (!strncmp(str, "= ", 2))
	{
		return control_set_param(str, true);
	}

	return 0;
}

void
process_message(asl_msg_t *msg, uint32_t source)
{
	int64_t msize = 0;
	static bool wq_draining = false;
	bool is_control = false;
	asl_msg_t *x;

	if (msg == NULL) return;

	is_control = asl_check_option(msg, ASL_OPT_CONTROL) != 0;

	if ((!is_control) && wq_draining)
	{
		if (global.work_queue_size >= (global.max_work_queue_size / 2))
		{
			asldebug("Work queue draining: dropped message.\n");
			asl_msg_release(msg);
			return;
		}
		else
		{
			asldebug("Work queue re-enabled at 1/2 max.  size %llu  max %llu\n", global.work_queue_size, global.max_work_queue_size);
			wq_draining = false;
		}
	}

	for (x = msg; x != NULL; x = x->next) msize += x->mem_size;

	if ((global.work_queue_size + msize) >= global.max_work_queue_size)
	{
		char *str = NULL;

		wq_draining = true;
		asl_msg_release(msg);

		asldebug("Work queue disabled.  msize %llu  size %llu  max %llu\n", msize, global.work_queue_size + msize, global.max_work_queue_size);
		asprintf(&str, "[Sender syslogd] [Level 2] [PID %u] [Message Internal work queue size limit exceeded - dropping messages] [UID 0] [UID 0] [Facility syslog]", global.pid);
		msg = asl_msg_from_string(str);
		free(str);
	}

	OSAtomicAdd64(msize, &global.work_queue_size);
	OSAtomicIncrement32(&global.work_queue_count);
	dispatch_async(global.work_queue, ^{
		int32_t kplevel;
		uint32_t status;
		uid_t uid;

		kplevel = -1;
		uid = -2;

		status = aslmsg_verify(msg, source, &kplevel, &uid);
		if (status == VERIFY_STATUS_OK)
		{
			if ((source == SOURCE_KERN) && (kplevel >= 0))
			{
				if (kplevel > 7) kplevel = 7;
				if (kern_notify_token[kplevel] < 0)
				{
					status = notify_register_plain(kern_notify_key[kplevel], &(kern_notify_token[kplevel]));
					if (status != 0) asldebug("notify_register_plain(%s) failed status %u\n", status);
				}

				notify_post(kern_notify_key[kplevel]);
			}

			if ((uid == 0) && is_control) control_message(msg);

			/* send message to output modules */
			asl_out_message(msg);
#if !TARGET_IPHONE_SIMULATOR
			if (global.bsd_out_enabled) bsd_out_message(msg);
#endif
		}

		asl_msg_release(msg);

		OSAtomicAdd64(-1ll * msize, &global.work_queue_size);
		OSAtomicDecrement32(&global.work_queue_count);
	});
}

int
internal_log_message(const char *str)
{
	asl_msg_t *msg;

	if (str == NULL) return 1;

	msg = asl_msg_from_string(str);
	if (msg == NULL) return 1;

	process_message(msg, SOURCE_INTERNAL);

	return 0;
}

void
trigger_aslmanager()
{
#if 0
	dispatch_async(dispatch_get_main_queue(), ^{
		if (aslmanager_triggered == 0)
		{
			aslmanager_triggered = 1;

			time_t now = time(0);
			if ((now - global.aslmanager_last_trigger) >= ASLMANAGER_DELAY)
			{
				global.aslmanager_last_trigger = now;
			//	asl_trigger_aslmanager();
				aslmanager_triggered = 0;
			}
			else
			{
				uint64_t delta = ASLMANAGER_DELAY - (now - global.aslmanager_last_trigger);
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delta * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
					global.aslmanager_last_trigger = time(0);
			// XXX		asl_trigger_aslmanager();
					aslmanager_triggered = 0;
				});
			}
		}
	});
#endif
}

int
asldebug(const char *str, ...)
{
	va_list v;
	FILE *dfp = NULL;

	if (global.debug == 0) return 0;

	if (global.debug_file == NULL) dfp = fopen(_PATH_SYSLOGD_LOG, "a");
	else dfp = fopen(global.debug_file, "a");
	if (dfp == NULL) return 0;

	va_start(v, str);
	vfprintf(dfp, str, v);
	va_end(v);

	fclose(dfp);

	return 0;
}

void
asl_mark(void)
{
	char *str = NULL;

	asprintf(&str, "[Sender syslogd] [Level 6] [PID %u] [Message -- MARK --] [UID 0] [UID 0] [Facility syslog]", global.pid);
	internal_log_message(str);
	free(str);
}

asl_msg_t *
asl_syslog_input_convert(const char *in, int len, char *rhost, uint32_t source)
{
	int pf, pri, index, n;
	char *p, *colon, *brace, *space, *tmp, *tval, *hval, *sval, *pval, *mval;
	char prival[8];
	const char *fval;
	asl_msg_t *msg;
	struct tm time;
	time_t tick;

	if (in == NULL) return NULL;
	if (len <= 0) return NULL;

	pri = LOG_DEBUG;
	if (source == SOURCE_KERN) pri = LOG_NOTICE;

	tval = NULL;
	hval = NULL;
	sval = NULL;
	pval = NULL;
	mval = NULL;
	fval = NULL;

	index = 0;
	p = (char *)in;

	/* skip leading whitespace */
	while ((index < len) && ((*p == ' ') || (*p == '\t')))
	{
		p++;
		index++;
	}

	if (index >= len) return NULL;

	/* parse "<NN>" priority (level and facility) */
	if (*p == '<')
	{
		p++;
		index++;

		n = sscanf(p, "%d", &pf);
		if (n == 1)
		{
			pri = pf & 0x7;
			if (pf > 0x7) fval = asl_syslog_faciliy_num_to_name(pf & LOG_FACMASK);
		}

		while ((index < len) && (*p != '>'))
		{
			p++;
			index++;
		}

		if (index < len)
		{
			p++;
			index++;
		}
	}

	snprintf(prival, sizeof(prival), "%d", pri);

	/* check if a timestamp is included */
	if (((len - index) > 15) && (p[9] == ':') && (p[12] == ':') && (p[15] == ' '))
	{
		tmp = malloc(16);
		if (tmp == NULL) return NULL;

		memcpy(tmp, p, 15);
		tmp[15] = '\0';

		tick = asl_core_parse_time(tmp, NULL);
		if (tick == (time_t)-1)
		{
			tval = tmp;
		}
		else
		{
			free(tmp);
			gmtime_r(&tick, &time);
			asprintf(&tval, "%d.%02d.%02d %02d:%02d:%02d UTC", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
		}

		p += 16;
		index += 16;
	}

	/* stop here for kernel messages */
	if (source == SOURCE_KERN)
	{
		msg = asl_msg_new(ASL_TYPE_MSG);
		if (msg == NULL) return NULL;

		asl_msg_set_key_val(msg, ASL_KEY_MSG, p);
		asl_msg_set_key_val(msg, ASL_KEY_LEVEL, prival);
		asl_msg_set_key_val(msg, ASL_KEY_PID, "0");

		return msg;
	}

	/* if message is from a network socket, hostname follows */
	if (source == SOURCE_UDP_SOCKET)
	{
		space = strchr(p, ' ');
		if (space != NULL)
		{
			n = space - p;
			hval = malloc(n + 1);
			if (hval == NULL) return NULL;

			memcpy(hval, p, n);
			hval[n] = '\0';

			p = space + 1;
			index += (n + 1);
		}
	}

	colon = strchr(p, ':');
	brace = strchr(p, '[');

	/* check for "sender:" or sender[pid]:"  */
	if (colon != NULL)
	{
		if ((brace != NULL) && (brace < colon))
		{
			n = brace - p;
			sval = malloc(n + 1);
			if (sval == NULL) return NULL;

			memcpy(sval, p, n);
			sval[n] = '\0';

			n = colon - (brace + 1) - 1;
			pval = malloc(n + 1);
			if (pval == NULL) return NULL;

			memcpy(pval, (brace + 1), n);
			pval[n] = '\0';
		}
		else
		{
			n = colon - p;
			sval = malloc(n + 1);
			if (sval == NULL) return NULL;

			memcpy(sval, p, n);
			sval[n] = '\0';
		}

		n = colon - p;
		p = colon + 1;
		index += (n + 1);
	}

	if (*p == ' ')
	{
		p++;
		index++;
	}

	n = len - index;
	if (n > 0)
	{
		mval = malloc(n + 1);
		if (mval == NULL) return NULL;

		memcpy(mval, p, n);
		mval[n] = '\0';
	}

	if (fval == NULL) fval = asl_syslog_faciliy_num_to_name(LOG_USER);

	msg = asl_msg_new(ASL_TYPE_MSG);
	if (msg == NULL) return NULL;

	if (tval != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_TIME, tval);
		free(tval);
	}

	if (fval != NULL) asl_msg_set_key_val(msg, "Facility", fval);
	else asl_msg_set_key_val(msg, "Facility", "user");

	if (sval != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_SENDER, sval);
		free(sval);
	}

	if (pval != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_PID, pval);
		free(pval);
	}
	else
	{
		asl_msg_set_key_val(msg, ASL_KEY_PID, "-1");
	}

	if (mval != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_MSG, mval);
		free(mval);
	}

	asl_msg_set_key_val(msg, ASL_KEY_LEVEL, prival);
	asl_msg_set_key_val(msg, ASL_KEY_UID, "-2");
	asl_msg_set_key_val(msg, ASL_KEY_GID, "-2");

	if (hval != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_HOST, hval);
		free(hval);
	}
	else if (rhost != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_HOST, rhost);
	}

	return msg;
}

asl_msg_t *
asl_input_parse(const char *in, int len, char *rhost, uint32_t source)
{
	asl_msg_t *msg;
	int status, x, legacy, off;

	asldebug("asl_input_parse: %s\n", (in == NULL) ? "NULL" : in);

	if (in == NULL) return NULL;

	legacy = 1;
	msg = NULL;

	/* calculate length if not provided */
	if (len == 0) len = strlen(in);

	/*
	 * Determine if the input is "old" syslog format or new ASL format.
	 * Old format lines should start with "<", but they can just be straight text.
	 * ASL input may start with a length (10 bytes) followed by a space and a '['.
	 * The length is optional, so ASL messages may also just start with '['.
	 */
	if ((in[0] != '<') && (len > 11))
	{
		status = sscanf(in, "%d ", &x);
		if ((status == 1) && (in[10] == ' ') && (in[11] == '[')) legacy = 0;
	}

	if (legacy == 1) return asl_syslog_input_convert(in, len, rhost, source);

	off = 11;
	if (in[0] == '[') off = 0;

	msg = asl_msg_from_string(in + off);
	if (msg == NULL) return NULL;

	if (rhost != NULL) asl_msg_set_key_val(msg, ASL_KEY_HOST, rhost);

	return msg;
}

#if !TARGET_IPHONE_SIMULATOR
void
launchd_callback(struct timeval *when, pid_t from_pid, pid_t about_pid, uid_t sender_uid, gid_t sender_gid, int priority, const char *from_name, const char *about_name, const char *session_name, const char *msg)
{
	asl_msg_t *m;
	char str[256];
	time_t now;

	if (global.launchd_enabled == 0) return;

/*
	asldebug("launchd_callback Time %lu %lu PID %u RefPID %u UID %d GID %d PRI %d Sender %s Ref %s Session %s Message %s\n",
	when->tv_sec, when->tv_usec, from_pid, about_pid, sender_uid, sender_gid, priority, from_name, about_name, session_name, msg);
*/

	m = asl_msg_new(ASL_TYPE_MSG);
	if (m == NULL) return;

	/* Level */
	if (priority < ASL_LEVEL_EMERG) priority = ASL_LEVEL_EMERG;
	if (priority > ASL_LEVEL_DEBUG) priority = ASL_LEVEL_DEBUG;
	snprintf(str, sizeof(str), "%d", priority);

	asl_msg_set_key_val(m, ASL_KEY_LEVEL, str);

	/* Time */
	if (when != NULL)
	{
		snprintf(str, sizeof(str), "%lu", when->tv_sec);
		asl_msg_set_key_val(m, ASL_KEY_TIME, str);

		snprintf(str, sizeof(str), "%lu", 1000 * (unsigned long int)when->tv_usec);
		asl_msg_set_key_val(m, ASL_KEY_TIME_NSEC, str);
	}
	else
	{
		now = time(NULL);
		snprintf(str, sizeof(str), "%lu", now);
		asl_msg_set_key_val(m, ASL_KEY_TIME, str);
	}

	/* Facility */
	asl_msg_set_key_val(m, ASL_KEY_FACILITY, FACILITY_CONSOLE);

	/* UID */
	snprintf(str, sizeof(str), "%u", (unsigned int)sender_uid);
	asl_msg_set_key_val(m, ASL_KEY_UID, str);

	/* GID */
	snprintf(str, sizeof(str), "%u", (unsigned int)sender_gid);
	asl_msg_set_key_val(m, ASL_KEY_GID, str);

	/* PID */
	if (from_pid != 0)
	{
		snprintf(str, sizeof(str), "%u", (unsigned int)from_pid);
		asl_msg_set_key_val(m, ASL_KEY_PID, str);
	}

	/* Reference PID */
	if ((about_pid > 0) && (about_pid != from_pid))
	{
		snprintf(str, sizeof(str), "%u", (unsigned int)about_pid);
		asl_msg_set_key_val(m, ASL_KEY_REF_PID, str);
	}

	/* Sender */
	if (from_name != NULL)
	{
		asl_msg_set_key_val(m, ASL_KEY_SENDER, from_name);
	}

	/* ReadUID */
	if (sender_uid != 0)
	{
		snprintf(str, sizeof(str), "%d", (int)sender_uid);
		asl_msg_set_key_val(m, ASL_KEY_READ_UID, str);
	}

	/* Reference Process */
	if (about_name != NULL)
	{
		if ((from_name != NULL) && (strcmp(from_name, about_name) != 0))
		{
			asl_msg_set_key_val(m, ASL_KEY_REF_PROC, about_name);
		}
	}

	/* Session */
	if (session_name != NULL)
	{
		asl_msg_set_key_val(m, ASL_KEY_SESSION, session_name);
	}

	/* Message */
	if (msg != NULL)
	{
		asl_msg_set_key_val(m, ASL_KEY_MSG, msg);
	}

	process_message(m, SOURCE_LAUNCHD);
}

#endif
