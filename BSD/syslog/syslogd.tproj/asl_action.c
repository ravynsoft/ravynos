/*
 * Copyright (c) 2004-2013 Apple Inc. All rights reserved.
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
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <notify.h>
#include <pthread.h>
#include <sys/acl.h>
#include <dirent.h>
#include <time.h>
#include <membership.h>
#include <configuration_profile.h>
#include "daemon.h"
#include <xpc/private.h>

#define _PATH_WALL "/usr/bin/wall"

#define MY_ID "asl_action"

#define MAX_FAILURES 5

#define ACTION_STATUS_ERROR  -1
#define ACTION_STATUS_OK      0

#define IDLE_CLOSE 300

#define PRIVATE_FLAG_NO_CLOSE    0x00000001 /* File or Store is closed by a cancellation handler */

#define DST_CLOSE_CHECKPOINT 0
#define DST_CLOSE_DELETED 1
#define DST_CLOSE_ERROR 2
#define DST_CLOSE_IDLE 3
#define DST_CLOSE_SHUTDOWN 4
static const char *why_str[] =
{
	"checkpoint",
	"deleted",
	"error",
	"idle",
	"shutdown"
};

#define forever for(;;)

static dispatch_queue_t asl_action_queue;
static dispatch_source_t checkpoint_timer;
static time_t sweep_time = 0;

#if TARGET_OS_EMBEDDED
#ifndef CRASH_MOVER_SERVICE
#define CRASH_MOVER_SERVICE "com.apple.crash_mover"
#endif
static dispatch_queue_t crashlog_queue;
static time_t crashmover_state = 0;
static int crashmover_token = -1;
#endif

typedef struct asl_file_data
{
	uint64_t next_id;
	asl_file_t *aslfile;
	time_t last_time;
	uint32_t flags;
	uint32_t pending;
	dispatch_source_t monitor;
} asl_action_asl_file_data_t;

typedef struct asl_store_data
{
	FILE *storedata;
	asl_file_t *aslfile;
	uint64_t next_id;
	time_t last_time;
	uint32_t flags;
	uint32_t pending;
	uint32_t p_year;
	uint32_t p_month;
	uint32_t p_day;
	dispatch_source_t storedata_monitor;
	dispatch_source_t aslfile_monitor;
} asl_action_asl_store_data_t;

typedef struct file_data
{
	int fd;
	uint32_t flags;
	time_t last_time;
	uint32_t last_count;
	uint32_t last_hash;
	uint32_t pending;
	char *last_msg;
	dispatch_source_t dup_timer;
	dispatch_source_t monitor;
} asl_action_file_data_t;

typedef struct set_param_data
{
	int token;
} asl_action_set_param_data_t;

static int action_asl_store_count;
static bool store_has_logged;

extern void db_save_message(asl_msg_t *m);

/* forward */
static int _act_file_checkpoint_all(uint32_t force);
static void _asl_action_post_process_rule(asl_out_module_t *m, asl_out_rule_t *r);
static void _asl_action_close_idle_files(time_t idle_time);
static void _act_dst_close(asl_out_rule_t *r, int why);

static void
_act_out_set_param(asl_out_module_t *m, char *x, bool eval)
{
	char *s = x;
	char **l;
	uint32_t count, intval;

	l = explode(s, " \t");
	if (l == NULL) return;

	for (count = 0; l[count] != NULL; count++);
	if (count == 0)
	{
		free_string_list(l);
		return;
	}

	if (!strcasecmp(l[0], "enable"))
	{
		/* = enable [1|0] */
		if (count < 2) intval = 1;
		else intval = atoi(l[1]);

		if (!eval) intval = (intval == 0) ? 1 : 0;

		if (intval == 0) m->flags &= ~MODULE_FLAG_ENABLED;
		else m->flags|= MODULE_FLAG_ENABLED;
		free_string_list(l);
		return;
	}
	else if (!strcasecmp(l[0], "disable"))
	{
		/* = disable [1|0] */
		if (count < 2) intval = 1;
		else intval = atoi(l[1]);

		if (!eval) intval = (intval == 0) ? 1 : 0;

		if (intval != 0) m->flags &= ~MODULE_FLAG_ENABLED;
		else m->flags|= MODULE_FLAG_ENABLED;
		free_string_list(l);
		return;
	}

	free_string_list(l);

	if (!strcmp(m->name, ASL_MODULE_NAME))
	{
		/* Other parameters may be set by com.apple.asl module */
		control_set_param(x, eval);
	}
}

static void
_act_notify(asl_out_module_t *m, asl_out_rule_t *r)
{
	if (m == NULL) return;
	if ((m->flags & MODULE_FLAG_ENABLED) == 0) return;

	if (r == NULL) return;
	if (r->options == NULL) return;

	notify_post(r->options);
}

static void
_act_broadcast(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
#if !TARGET_OS_EMBEDDED
	FILE *pw;
	const char *val;

	if (m == NULL) return;
	if ((m->flags & MODULE_FLAG_ENABLED) == 0) return;

	if (m->name == NULL) return;
	if (r == NULL) return;
	if (msg == NULL) return;

	/* only base module (asl.conf) may broadcast */
	if (strcmp(m->name, ASL_MODULE_NAME)) return;

	val = r->options;
	if (val == NULL) val = asl_msg_get_val_for_key(msg, ASL_KEY_MSG);
	if (val == NULL) return;

	pw = popen(_PATH_WALL, "w");
	if (pw == NULL)
	{
		asldebug("%s: error sending wall message: %s\n", MY_ID, strerror(errno));
		return;
	}

	fprintf(pw, "%s", val);
	pclose(pw);
#endif
}

static void
_act_set_key(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	/* Placeholder */
}

static void
_act_unset_key(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	/* Placeholder */
}

static void
_act_access_control(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	int32_t ruid, rgid;
	char *p;

	if (m == NULL) return;
	if (m->name == NULL) return;
	if (r == NULL) return;
	if (msg == NULL) return;

	/* only base module (asl.conf) may set access controls */
	if (strcmp(m->name, ASL_MODULE_NAME)) return;

	ruid = atoi(r->options);
	rgid = -1;
	p = strchr(r->options, ' ');
	if (p == NULL) p = strchr(r->options, '\t');
	if (p != NULL)
	{
		*p = '\0';
		p++;
		rgid = atoi(p);
	}

	if (ruid != -1) asl_msg_set_key_val(msg, ASL_KEY_READ_UID, r->options);
	if (p != NULL)
	{
		if (rgid != -1) asl_msg_set_key_val(msg, ASL_KEY_READ_GID, p);
		p--;
		*p = ' ';
	}
}

#if TARGET_OS_EMBEDDED
static void
_crashlog_queue_check(void)
{
	/*
	 * Check whether the crashlog queue has been suspended for too long.
	 * We allow the crashlog quque to be suspended for 60 seconds.
	 * After that, we start logging again.  This prevents syslogd from
	 * filling memory due to a suspended queue.  CrashMover really shoud
	 * take no more than a second or two to finish.
	 */
	if (crashmover_state == 0) return;
	if ((time(NULL) - crashmover_state) <= 60) return;

	asldebug("CrashMover timeout: resuming crashlog queue\n");
	dispatch_resume(crashlog_queue);
	crashmover_state = 0;
}
#endif

/*
 * cover routine for asl_out_dst_file_create_open()
 */
static int
_act_file_create_open(asl_out_dst_data_t *dst)
{
	return asl_out_dst_file_create_open(dst, NULL);
}

/*
 * Checks and creates (if required) a directory for an ASL data store.
 */
static int
_asl_dir_create(asl_out_rule_t *r)
{
	struct stat sb;
	int status;

	memset(&sb, 0, sizeof(struct stat));
	status = stat(r->dst->path, &sb);
	if (status == 0)
	{
		/* Store path must be a directory */
		if (!S_ISDIR(sb.st_mode))
		{
			asldebug("_asl_dir_create: expected a directory at path %s\n", r->dst->path);
			return -1;
		}
	}
	else if (errno == ENOENT)
	{
		/* Directory doesn't exists - try to create it */
		status = asl_out_mkpath(global.asl_out_module, r);
		if (status != 0)
		{
			asldebug("_asl_dir_create: asl_out_mkpath failed: %s\n", r->dst->path);
			return -1;
		}
	}
	else
	{
		/* Unexpected stat error */
		asldebug("_asl_dir_create: stat error %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * Close an ASL Directory StoreData file
 * - cancel the dispatch source for the file
 */
static void
_asl_dir_storedata_close(asl_out_rule_t *r)
{
	asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;
	if (as_data->storedata == NULL) return;

	if (as_data->storedata_monitor == NULL)
	{
		/*
		 * This should never happen, but _asl_dir_storedata_open allows
		 * dispatch_source_create to fail silently.  If there is no dispatch
		 * source, we just close the file.
		 */
		asldebug("close ASL storedata fd %d\n", fileno(as_data->storedata));
		fclose(as_data->storedata);
	}
	else
	{
		/*
		 * The storedata_monitor cancel handler will close the file.
		 */
		dispatch_source_cancel(as_data->storedata_monitor);
		dispatch_release(as_data->storedata_monitor);

	}

	asldebug("_asl_dir_storedata_close %p\n", as_data->storedata);
	as_data->storedata = NULL;
}

/*
 * Open an ASL Directory StoreData file
 * - check directory existance and create it if necessary
 * - check for StoreData file and create it (with given xid) if necessary.
 * - create a dispatch source to watch for StoreData file deletion
 */
static int
_asl_dir_storedata_open(asl_out_rule_t *r, uint64_t xid)
{
	struct stat sb;
	int status;
	char dstpath[MAXPATHLEN];

	asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;
	if (as_data->storedata != NULL) return 0;

	status = _asl_dir_create(r);
	if (status != 0)
	{
		asldebug("_asl_dir_storedata_open: No directory at path %s\n", r->dst->path);
		return -1;
	}

	/* StoreData file is not open */
	snprintf(dstpath, sizeof(dstpath), "%s/%s", r->dst->path, FILE_ASL_STORE_DATA);

	memset(&sb, 0, sizeof(struct stat));
	status = stat(dstpath, &sb);
	if (status == 0)
	{
		/* StoreData file exists */
		as_data->storedata = fopen(dstpath, "r+");
		if (as_data->storedata == NULL)
		{
			asldebug("_asl_dir_storedata_open: fopen existing %s: %s\n", dstpath, strerror(errno));
			return -1;
		}
	}
	else if (errno == ENOENT)
	{
		/*
		 * StoreData file does not exist.
		 * Create a new StoreData with a given xid.
		 */
		//TODO: This should return a failure if there are any ASL files.
		/* that would require reading the directory, thus a performance hit */
		as_data->storedata = fopen(dstpath, "w+");
		if (as_data->storedata == NULL)
		{
			asldebug("_asl_dir_storedata_open: fopen new %s: %s\n", dstpath, strerror(errno));
			return -1;
		}

		uint64_t xout = asl_core_htonq(xid);
		status = fwrite(&xout, sizeof(uint64_t), 1, as_data->storedata);
		if (status != 1)
		{
			asldebug("_asl_dir_storedata_open: storedata write failed %d %s\n", errno, strerror(errno));
			return -1;
		}

#if !TARGET_OS_SIMULATOR
		if (chown(dstpath, r->dst->uid[0], r->dst->gid[0]) != 0)
		{
			asldebug("_asl_dir_storedata_open: chown %d %d new %s: %s\n", dstpath, r->dst->uid[0], r->dst->gid[0], strerror(errno));
			return -1;
		}
#endif
	}
	else
	{
		/* Unexpected stat error */
		asldebug("_asl_dir_storedata_open: stat error %s\n", strerror(errno));
		return -1;
	}

	/* create storedata_monitor */
	int fd = fileno(as_data->storedata);
	FILE *sdfp = as_data->storedata;

	as_data->storedata_monitor = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd, DISPATCH_VNODE_DELETE, asl_action_queue);
	if (as_data->storedata_monitor != NULL)
	{
		dispatch_source_set_event_handler(as_data->storedata_monitor, ^{
			_act_dst_close(r, DST_CLOSE_DELETED);
		});

		dispatch_source_set_cancel_handler(as_data->storedata_monitor, ^{
			asldebug("cancel/close ASL storedata fd %d\n", fd);
			fclose(sdfp);
		});

		dispatch_resume(as_data->storedata_monitor);
	}

	asldebug("_asl_dir_storedata_open ASL storedata %s fd %d\n", dstpath, fd);
	return 0;
}

/*
 * Close an ASL Directory ASL file
 * - cancel the dispatch source for the file
 * - clears file name and timestamp in asl_action_asl_store_data_t structue
 */
static void
_asl_dir_today_close(asl_out_rule_t *r)
{
	asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;
	if (as_data->aslfile == NULL) return;

	if (as_data->pending != 0)
	{
		char *str = NULL;
		asprintf(&str, "[Sender syslogd] [Level 4] [PID %u] [Facility syslog] [Message ASL Store %s was closed with %d pending messages]", global.pid, r->dst->current_name, as_data->pending);
		internal_log_message(str);
		free(str);
	}

	if (as_data->aslfile_monitor == NULL)
	{
		/*
		 * This should never happen, but _asl_dir_today_open allows
		 * dispatch_source_create to fail silently.  If there is no dispatch
		 * source, we just close the file.
		 */
		asldebug("close ASL fd %d\n", fileno(as_data->aslfile->store));
		asl_file_close(as_data->aslfile);
	}
	else
	{
		/*
		 * The aslfile_monitor cancel handler will close the file.
		 */
		dispatch_source_cancel(as_data->aslfile_monitor);
		dispatch_release(as_data->aslfile_monitor);
		as_data->aslfile_monitor = NULL;
	}

	as_data->p_year = 0;
	as_data->p_month = 0;
	as_data->p_day = 0;

	free(r->dst->current_name);
	r->dst->current_name = NULL;

	as_data->aslfile = NULL;
}

/*
 * Check file size.
 */
static int
_act_checkpoint(asl_out_rule_t *r, uint32_t force)
{
	char tmpcurrent_name[MAXPATHLEN], *fn;
	bool size_only = false;

	if (r == NULL) return 0;
	if (r->dst == NULL) return 0;

	fn = r->dst->current_name;
	if (fn == NULL)
	{
		if (r->dst->path == NULL) return 0;
		asl_dst_make_current_name(r->dst, 0, tmpcurrent_name, sizeof(tmpcurrent_name));
		fn = tmpcurrent_name;
	}

	if ((force == CHECKPOINT_TEST) || (r->dst->flags & MODULE_FLAG_SIZE_ONLY))
	{
		size_only = true;
	}

	if (size_only && (r->dst->file_max == 0)) return 0;

	if ((r->dst->size == 0) || (r->dst->timestamp == 0))
	{
		struct stat sb;

		memset(&sb, 0, sizeof(struct stat));

		if (stat(fn, &sb) < 0)
		{
			if (errno == ENOENT) return 0;
			return -1;
		}

		if (r->dst->timestamp == 0) r->dst->timestamp = sb.st_birthtimespec.tv_sec;
		if (r->dst->timestamp == 0) r->dst->timestamp = sb.st_mtimespec.tv_sec;
		r->dst->size = sb.st_size;
	}
	
	if (size_only && (r->dst->size < r->dst->file_max)) return 0;

	if (r->dst->flags & MODULE_FLAG_BASESTAMP)
	{
		_act_dst_close(r, DST_CLOSE_CHECKPOINT);
	}
	else
	{
		char srcpath[MAXPATHLEN];
		char dstpath[MAXPATHLEN];

		snprintf(srcpath, sizeof(srcpath), "%s", fn);

		r->dst->timestamp = time(NULL);
		asl_dst_make_current_name(r->dst, MODULE_FLAG_BASESTAMP, dstpath, sizeof(dstpath));

		_act_dst_close(r, DST_CLOSE_CHECKPOINT);
		if (strneq(srcpath, dstpath))
		{
			rename(srcpath, dstpath);
			asldebug("CHECKPOINT RENAME %s %s\n", srcpath, dstpath);
		}
	}

	r->dst->timestamp = 0;
	r->dst->size = 0;
	return 1;
}

/*
 * Open today's ASL file in an ASL directory
 * - Checks date and closes a currently open file if it has the wrong date
 * - Opens today's file
 */
static int
_asl_dir_today_open(asl_out_rule_t *r, const time_t *tick)
{
	int status;
	mode_t mask;
	struct tm ctm;
	time_t now;

	if (r == NULL) return -1;
	if (r->dst == NULL) return -1;

	status = _asl_dir_create(r);
	if (status != 0)
	{
		asldebug("_asl_dir_today_open: No directory at path %s\n", r->dst->path);
		return -1;
	}

	asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;

	memset(&ctm, 0, sizeof(struct tm));
	if (tick == NULL)
	{
		now = time(NULL);
		tick = (const time_t *)&now;
	}

	if (localtime_r(tick, &ctm) == NULL)
	{
		asldebug("_asl_dir_today_open: localtime_r error %s\n", strerror(errno));
		return -1;
	}

	/* checks file_max and closes if required */
	status = _act_checkpoint(r, CHECKPOINT_TEST);
	if (status == 1) asl_trigger_aslmanager();

	if (as_data->aslfile != NULL)
	{
		/* Check the date */
		if ((as_data->p_year == ctm.tm_year) && (as_data->p_month == ctm.tm_mon) && (as_data->p_day == ctm.tm_mday)) return 0;

		/* Wrong date, close the current file */
		_asl_dir_today_close(r);
	}

	/* Open data file */

	if (r->dst->flags & MODULE_FLAG_BASESTAMP)
	{
		char tstamp[32];

		if (tick == NULL)
		{
			now = time(NULL);
			tick = (const time_t *)&now;
		}

		asl_make_timestamp(now, r->dst->style_flags, tstamp, sizeof(tstamp));
		asprintf(&(r->dst->current_name), "%s/%s.asl", r->dst->path, tstamp);
	}
	else
	{
		asprintf(&(r->dst->current_name), "%s/%d.%02d.%02d.asl", r->dst->path, ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}


	if (r->dst->current_name == NULL)
	{
		asldebug("_asl_dir_today_open: asprintf error %s\n", strerror(errno));
		return -1;
	}

#if TARGET_OS_SIMULATOR
	uid_t uid = -1;
	gid_t gid = -1;
#else
	uid_t uid = r->dst->uid[0];
	gid_t gid = r->dst->gid[0];
#endif

	mask = umask(0);
	status = asl_file_open_write(r->dst->current_name, (r->dst->mode & 00666), uid, gid, &(as_data->aslfile));
	umask(mask);

	if (status != ASL_STATUS_OK)
	{
		asldebug("_asl_dir_today_open: asl_file_open_write %s error %s\n", r->dst->current_name, asl_core_error(status));
		free(r->dst->current_name);
		r->dst->current_name = NULL;
		return -1;
	}

	if (fseek(as_data->aslfile->store, 0, SEEK_END) != 0)
	{
		asldebug("_asl_dir_today_open: fseek %s error %s\n", r->dst->current_name, strerror(errno));
		free(r->dst->current_name);
		r->dst->current_name = NULL;
		return -1;
	}

	as_data->p_year = ctm.tm_year;
	as_data->p_month = ctm.tm_mon;
	as_data->p_day = ctm.tm_mday;

	/* create aslfile_monitor */
	int fd = fileno(as_data->aslfile->store);
	asl_file_t *aslf = as_data->aslfile;

	as_data->aslfile_monitor = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd, DISPATCH_VNODE_DELETE, asl_action_queue);
	if (as_data->aslfile_monitor != NULL)
	{
		dispatch_source_set_event_handler(as_data->aslfile_monitor, ^{
			_act_dst_close(r, DST_CLOSE_DELETED);
		});

		dispatch_source_set_cancel_handler(as_data->aslfile_monitor, ^{
			asldebug("cancel/close ASL file fd %d\n", fd);
			asl_file_close(aslf);
		});

		dispatch_resume(as_data->aslfile_monitor);
	}

	asldebug("_asl_dir_today_open ASL file %s fd %d\n", r->dst->current_name, fd);

	return 0;
}

static void
_asl_file_close(asl_out_rule_t *r)
{
	if (r == NULL) return;
	if (r->dst == NULL) return;

	asl_action_asl_file_data_t *af_data = (asl_action_asl_file_data_t *)r->dst->private;
	if (af_data->aslfile == NULL) return;

	if (af_data->pending != 0)
	{
		char *str = NULL;
		asprintf(&str, "[Sender syslogd] [Level 4] [PID %u] [Facility syslog] [Message ASL File %s was closed with %d pending messages]", global.pid, r->dst->current_name, af_data->pending);
		internal_log_message(str);
		free(str);
	}

	if (af_data->monitor == NULL)
	{
		/*
		 * This should never happen, but _asl_file_open allows
		 * dispatch_source_create to fail silently.  If there is no dispatch
		 * source, we just close the file.
		 */
		asldebug("close ASL fd %d\n", fileno(af_data->aslfile->store));
		asl_file_close(af_data->aslfile);
	}
	else
	{
		/*
		 * The monitor cancel handler will close the file.
		 */
		dispatch_source_cancel(af_data->monitor);
		dispatch_release(af_data->monitor);
		af_data->monitor = NULL;
	}

	af_data->aslfile = NULL;
}

static int
_asl_file_open(asl_out_rule_t *r)
{
	int fd, status;

	if (r == NULL) return -1;
	if (r->dst == NULL) return -1;

	asl_action_asl_file_data_t *af_data = (asl_action_asl_file_data_t *)r->dst->private;
	if (af_data->aslfile != NULL) return 0;

	/* create path if necessary */
	status = asl_out_mkpath(global.asl_out_module, r);
	if (status != 0)
	{
		asldebug("_asl_file_open: asl_out_mkpath %s failed\n", r->dst->path);
		return -1;
	}

	fd = _act_file_create_open(r->dst);
	if (fd < 0)
	{
		asldebug("_asl_file_open: _act_file_create_open %s failed %d %s\n", r->dst->current_name, errno, strerror(errno));
		return -1;
	}

	close(fd);

	if (r->dst->current_name == NULL) return -1;

	status = asl_file_open_write(r->dst->current_name, 0, -1, -1, &(af_data->aslfile));
	if (status != ASL_STATUS_OK)
	{
		asldebug("_asl_file_open: asl_file_open_write %s failed %d %s\n", r->dst->current_name, errno, strerror(errno));
		return -1;
	}

	/* create monitor */
	fd = fileno(af_data->aslfile->store);
	asl_file_t *aslf = af_data->aslfile;

	af_data->monitor = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd, DISPATCH_VNODE_DELETE, asl_action_queue);
	if (af_data->monitor != NULL)
	{
		dispatch_source_set_event_handler(af_data->monitor, ^{
			_act_dst_close(r, DST_CLOSE_DELETED);
		});

		dispatch_source_set_cancel_handler(af_data->monitor, ^{
			asldebug("cancel/close ASL file fd %d\n", fd);
			asl_file_close(aslf);
		});

		dispatch_resume(af_data->monitor);
	}

	asldebug("_asl_file_open ASL file %s fd %d\n", r->dst->current_name, fd);
	return 0;
}

static void
_text_file_close(asl_out_rule_t *r)
{
	asl_action_file_data_t *f_data = (asl_action_file_data_t *)r->dst->private;
	if (f_data->fd < 0) return;

	if (f_data->pending != 0)
	{
		char *str = NULL;
		asprintf(&str, "[Sender syslogd] [Level 4] [PID %u] [Facility syslog] [Message File %s was closed with %d pending messages]", global.pid, r->dst->current_name, f_data->pending);
		internal_log_message(str);
		free(str);
	}

	if (f_data->monitor == NULL)
	{
		/*
		 * This should never happen, but _text_file_open allows
		 * dispatch_source_create to fail silently.  If there is no dispatch
		 * source, we just close the file.
		 */
		asldebug("close fd %d\n", f_data->fd);
		close(f_data->fd);
	}
	else
	{
		/*
		 * The monitor cancel handler will close the file.
		 */
		dispatch_source_cancel(f_data->monitor);
		dispatch_release(f_data->monitor);
		f_data->monitor = NULL;
	}

	f_data->fd = -1;
}

static int
_text_file_open(asl_out_rule_t *r)
{
	asl_action_file_data_t *f_data = (asl_action_file_data_t *)r->dst->private;

	if (f_data->fd < 0)
	{
		f_data->fd = _act_file_create_open(r->dst);
		if (f_data->fd < 0)
		{
			/*
			 * lazy path creation: create path and retry
			 * _act_file_create_open does not create the path
			 * so we do it here.
			 */
			int status = asl_out_mkpath(global.asl_out_module, r);
			if (status != 0) return -1;

			f_data->fd = _act_file_create_open(r->dst);
		}

		if (f_data->fd < 0) return -1;

		f_data->monitor = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, f_data->fd, DISPATCH_VNODE_DELETE, asl_action_queue);
		if (f_data->monitor != NULL)
		{
			int ffd = f_data->fd;

			dispatch_source_set_event_handler(f_data->monitor, ^{
				asldebug("dispatch_source DISPATCH_VNODE_DELETE fd %d\n", ffd);
				_act_dst_close(r, DST_CLOSE_DELETED);
			});

			dispatch_source_set_cancel_handler(f_data->monitor, ^{
				asldebug("cancel/close file fd %d\n", ffd);
				close(ffd);
			});

			dispatch_resume(f_data->monitor);
		}
	}

	return 0;
}

static int
_act_dst_open(asl_out_rule_t *r, const time_t *tick, uint64_t xid)
{
	if (r == NULL) return -1;
	if (r->dst == NULL) return -1;
	if (r->dst->private == NULL) return -1;

	if (r->action == ACTION_ASL_DIR)
	{
		if (_asl_dir_today_open(r, tick) != 0)
		{
			asldebug("_act_dst_open:_asl_dir_today_open %s FAILED!\n", r->dst->path);
			return -1;
		}

		if (_asl_dir_storedata_open(r, xid) != 0)
		{
			asldebug("_act_dst_open:_asl_dir_storedata_open %s FAILED!  Closing today file\n", r->dst->path);
			_asl_dir_today_close(r);
			return -1;
		}

		return 0;
	}

	if (r->action == ACTION_ASL_FILE)
	{
		return _asl_file_open(r);
	}

	if (r->action == ACTION_FILE)
	{
		return _text_file_open(r);
	}

	return -1;
}

static void
_act_dst_close(asl_out_rule_t *r, int why)
{
	if (r == NULL) return;
	if (r->dst == NULL) return;
	if (r->dst->private == NULL) return;

	if (r->action == ACTION_ASL_DIR)
	{
		asldebug("_act_dst_close: %s ASL DIR %s\n", why_str[why], r->dst->path);
		if (why != DST_CLOSE_CHECKPOINT) _asl_dir_storedata_close(r);
		_asl_dir_today_close(r);
	}
	else if (r->action == ACTION_ASL_FILE)
	{
		asldebug("_act_dst_close: %s ASL FILE %s\n", why_str[why], (r->dst->current_name == NULL) ? r->dst->path : r->dst->current_name);
		_asl_file_close(r);
	}
	else if (r->action == ACTION_FILE)
	{
		asldebug("_act_dst_close: %s FILE %s\n", why_str[why], (r->dst->current_name == NULL) ? r->dst->path : r->dst->current_name);
		_text_file_close(r);
	}
}

static uint32_t
_act_store_file_setup(asl_out_module_t *m, asl_out_rule_t *r)
{
	uint32_t status;
	asl_action_asl_file_data_t *af_data;

	if (r == NULL) return ASL_STATUS_INVALID_STORE;
	if (r->dst == NULL) return ASL_STATUS_INVALID_STORE;
	if (r->dst->private == NULL) return ASL_STATUS_INVALID_STORE;

	af_data = (asl_action_asl_file_data_t *)r->dst->private;
	if (af_data->aslfile != NULL)
	{
		af_data->next_id++;
		return ASL_STATUS_OK;
	}

	if (_act_dst_open(r, NULL, 0) != 0) return ASL_STATUS_WRITE_FAILED;

	status = asl_file_read_set_position(af_data->aslfile, ASL_FILE_POSITION_LAST);
	if (status != ASL_STATUS_OK)
	{
		asldebug("_act_store_file_setup: asl_file_read_set_position failed %d %s\n", status, asl_core_error(status));
		_act_dst_close(r, DST_CLOSE_ERROR);
		return status;
	}

	af_data->next_id = af_data->aslfile->cursor_xid + 1;
	if (fseek(af_data->aslfile->store, 0, SEEK_END) != 0)
	{
		asldebug("_act_store_file_setup: fseek failed %d %s\n", errno, strerror(errno));
		_act_dst_close(r, DST_CLOSE_ERROR);
		return ASL_STATUS_WRITE_FAILED;
	}

	return ASL_STATUS_OK;
}

/*
 * _act_store_dir_setup
 *
 * Opens StoreData file and today's file
 * Reads ASL Message ID from StoreData file
 * Writes ASL Message ID + 1 to StoreData file
 */
static uint32_t
_act_store_dir_setup(asl_out_module_t *m, asl_out_rule_t *r, time_t tick)
{
	uint64_t xid;
	int status;
	asl_action_asl_store_data_t *as_data;

	if (r == NULL) return ASL_STATUS_INVALID_STORE;
	if (r->dst == NULL) return ASL_STATUS_INVALID_STORE;
	if (r->dst->private == NULL) return ASL_STATUS_INVALID_STORE;
	if (r->dst->path == NULL) return ASL_STATUS_INVALID_STORE;

	as_data = (asl_action_asl_store_data_t *)r->dst->private;

	if (_act_dst_open(r, NULL, as_data->next_id) != 0)
	{
		asldebug("_act_store_dir_setup: _act_dst_open %s failed\n", r->dst->path);
		return ASL_STATUS_WRITE_FAILED;
	}

	/* get / set message id from StoreData file */
	xid = 0;
	rewind(as_data->storedata);
	if (fread(&xid, sizeof(uint64_t), 1, as_data->storedata) != 1)
	{
		asldebug("_act_store_dir_setup: storedata read failed %d %s\n", errno, strerror(errno));
		_act_dst_close(r, DST_CLOSE_ERROR);
		return ASL_STATUS_READ_FAILED;
	}

	xid = asl_core_ntohq(xid);
	xid++;
	as_data->next_id = xid;

	xid = asl_core_htonq(xid);
	rewind(as_data->storedata);
	status = fwrite(&xid, sizeof(uint64_t), 1, as_data->storedata);
	if (status != 1)
	{
		asldebug("_act_store_dir_setup: storedata write failed %d %s\n", errno, strerror(errno));
		_act_dst_close(r, DST_CLOSE_ERROR);
		return ASL_STATUS_WRITE_FAILED;
	}

	fflush(as_data->storedata);

	if (fseek(as_data->aslfile->store, 0, SEEK_END) != 0)
	{
		asldebug("_act_store_dir_setup: aslfile fseek failed %d %s\n", errno, strerror(errno));
		_act_dst_close(r, DST_CLOSE_ERROR);
		return ASL_STATUS_FAILED;
	}

	return ASL_STATUS_OK;
}

static void
_asl_action_asl_store_data_free(asl_action_asl_store_data_t *as_data)
{
	if (as_data == NULL) return;
	free(as_data);
}

static void
_asl_action_asl_file_data_free(asl_action_asl_file_data_t *af_data)
{
	if (af_data == NULL) return;
	free(af_data);
}

static void
_asl_action_file_data_free(asl_action_file_data_t *f_data)
{
	if (f_data == NULL) return;

	if (f_data->dup_timer != NULL)
	{
		if (f_data->last_count == 0)
		{
			/*
			 * The timer exists, but last_count is zero, so the timer is suspended.
			 * Sources must not be released in when suspended.
			 * So we resume it so that we can release it.
			 */
			dispatch_resume(f_data->dup_timer);
		}

		dispatch_release(f_data->dup_timer);
	}

	free(f_data->last_msg);
	free(f_data);
}

static void
_asl_action_set_param_data_free(asl_action_set_param_data_t *spdata)
{
	if (spdata != NULL) notify_cancel(spdata->token);
	free(spdata);
}

static void
_asl_action_save_failed(const char *where, asl_out_module_t *m, asl_out_rule_t *r, uint32_t status)
{
	if (r->dst->flags & MODULE_FLAG_SOFT_WRITE) return;

	r->dst->fails++;
	asldebug("%s: %s save to %s failed: %s\n", where, m->name, r->dst->path, asl_core_error(status));

	/* disable further activity after multiple failures */
	if (r->dst->fails > MAX_FAILURES)
	{
		char *str = NULL;
		asprintf(&str, "[Sender syslogd] [Level 3] [PID %u] [Facility syslog] [Message Disabling module %s writes to %s following %u failures (%s)]", global.pid, m->name, r->dst->path, r->dst->fails, asl_core_error(status));
		internal_log_message(str);
		free(str);

		if (r->action == ACTION_ASL_DIR) _asl_action_asl_store_data_free((asl_action_asl_store_data_t *)r->dst->private);
		else if (r->action == ACTION_ASL_FILE) _asl_action_asl_file_data_free((asl_action_asl_file_data_t *)r->dst->private);
		else if (r->action == ACTION_FILE) _asl_action_file_data_free((asl_action_file_data_t *)r->dst->private);

		r->dst->private = NULL;
		r->action = ACTION_NONE;
	}
}

/*
 * Save a message in an ASL file.
 */
static int
_act_store_file(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	asl_action_asl_file_data_t *af_data;
	uint32_t status;
	uint64_t mid;

	if (r == NULL) return ACTION_STATUS_ERROR;
	if (r->dst == NULL) return ACTION_STATUS_ERROR;
	if (r->dst->private == NULL) return ACTION_STATUS_ERROR;

	af_data = (asl_action_asl_file_data_t *)r->dst->private;
	if (af_data->pending > 0) af_data->pending--;

	status = _act_store_file_setup(m, r);
	if (status == ASL_STATUS_OK)
	{
		af_data->last_time = time(NULL);

		r->dst->fails = 0;
		mid = af_data->next_id;

		/* save message to file and update dst size */
		status = asl_file_save(af_data->aslfile, msg, &mid);
		if (status == ASL_STATUS_OK)
		{
			r->dst->size = af_data->aslfile->file_size;

			if (_act_checkpoint(r, CHECKPOINT_TEST) == 1) asl_trigger_aslmanager();
		}
	}

	if (status != ASL_STATUS_OK)
	{
		_asl_action_save_failed("_act_store_file", m, r, status);
		return ACTION_STATUS_ERROR;
	}

	return ACTION_STATUS_OK;
}

/*
 * Save a message in an ASL data store.
 */
static int
_act_store_dir(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	asl_action_asl_store_data_t *as_data;
	uint32_t status;
	uint64_t mid;
	const char *val;
	time_t tick;

	if (r == NULL) return ACTION_STATUS_ERROR;
	if (r->dst == NULL) return ACTION_STATUS_ERROR;
	if (r->dst->private == NULL) return ACTION_STATUS_ERROR;

	as_data = (asl_action_asl_store_data_t *)r->dst->private;
	if (as_data->pending > 0) as_data->pending--;

	val = asl_msg_get_val_for_key(msg, ASL_KEY_TIME);
	if (val == NULL) return ACTION_STATUS_ERROR;

	tick = atol(val);

	status = _act_store_dir_setup(m, r, tick);
	if (status == ASL_STATUS_OK)
	{
		as_data->last_time = time(NULL);

		r->dst->fails = 0;
		mid = as_data->next_id;
		status = asl_file_save(as_data->aslfile, msg, &mid);
		if (status == ASL_STATUS_OK) r->dst->size = as_data->aslfile->file_size;
		else asldebug("_act_store_dir asl_file_save FAILED %s\n", asl_core_error(status));
		//TODO: checkpoint here?
		/*
		 * Currently, the checkpoint is in _asl_dir_today_open().
		 * Moving it here would be in keeping with the way that
		 * _act_store_file() and _act_file_final() do checkpoints.
		 */
	}

	if (status != ASL_STATUS_OK)
	{
		_asl_action_save_failed("_act_store_dir", m, r, status);
		return ACTION_STATUS_ERROR;
	}

	return ACTION_STATUS_OK;
}

static void
_act_store_final(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	if (r->action == ACTION_ASL_DIR) _act_store_dir(m, r, msg);
	else _act_store_file(m, r, msg);
}

/*
 * Save a message to an ASL format file (ACTION_ASL_FILE)
 * or to an ASL directory (ACTION_ASL_DIR).
 */
static void
_act_store(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	if (r == NULL) return;
	if (r->dst == NULL) return;
	if (msg == NULL) return;
	if (m == NULL) return;
	if ((m->flags & MODULE_FLAG_ENABLED) == 0) return;

	if (r->dst->flags & MODULE_FLAG_HAS_LOGGED) return;

	r->dst->flags |= MODULE_FLAG_HAS_LOGGED;
	if (r->action == ACTION_ASL_DIR)
	{
		asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;
		if (as_data != NULL) as_data->pending++;
	}
	else if (r->action == ACTION_ASL_FILE)
	{
		asl_action_asl_file_data_t *af_data = (asl_action_asl_file_data_t *)r->dst->private;
		if (af_data != NULL) af_data->pending++;
	}

#if TARGET_OS_EMBEDDED
	if (r->dst->flags & MODULE_FLAG_CRASHLOG)
	{
		_crashlog_queue_check();
		asl_msg_retain(msg);
		dispatch_async(crashlog_queue, ^{
			dispatch_async(asl_action_queue, ^{
				_act_store_final(m, r, msg);
				asl_msg_release(msg);
			});
		});
		return;
	}
#endif

	_act_store_final(m, r, msg);
}

static int
_send_repeat_msg(asl_out_rule_t *r)
{
	asl_action_file_data_t *f_data;
	char vt[32], *msg;
	int len, status;
	time_t now = time(NULL);

	if (r == NULL) return -1;
	if (r->dst == NULL) return -1;
	if (r->dst->private == NULL) return -1;

	f_data = (asl_action_file_data_t *)r->dst->private;

	free(f_data->last_msg);
	f_data->last_msg = NULL;

	if (f_data->last_count == 0) return 0;

	/* stop the timer */
	dispatch_suspend(f_data->dup_timer);

	memset(vt, 0, sizeof(vt));
	ctime_r(&now, vt);
	vt[19] = '\0';

	msg = NULL;
	asprintf(&msg, "%s --- last message repeated %u time%s ---\n", vt + 4, f_data->last_count, (f_data->last_count == 1) ? "" : "s");
	f_data->last_count = 0;
	f_data->last_time = now;
	if (msg == NULL) return -1;

	if (f_data->fd < 0) f_data->fd = _act_file_create_open(r->dst);

	len = strlen(msg);
	status = write(f_data->fd, msg, len);
	free(msg);

	if ((status < 0) || (status < len))
	{
		asldebug("%s: error writing repeat message (%s): %s\n", MY_ID, r->dst->current_name, strerror(errno));
		return -1;
	}

	return 0;
}

static void
_start_cycling()
{
	struct timespec midnight;
	struct tm t;
	time_t x;

	x = time(NULL);

	if (checkpoint_timer != NULL) return;

	localtime_r(&x, &t);

	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;
	t.tm_mday++;

	x = mktime(&t);
	midnight.tv_sec = x;
	midnight.tv_nsec = 0;

	checkpoint_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, asl_action_queue);
	dispatch_source_set_timer(checkpoint_timer, dispatch_walltime(&midnight, 0), NSEC_PER_SEC * SEC_PER_DAY, 0);
	dispatch_source_set_event_handler(checkpoint_timer, ^{ _act_file_checkpoint_all(CHECKPOINT_FORCE); });
	dispatch_resume(checkpoint_timer);
}

/* check if a module path (mpath) matches a user path (upath) */
static bool
_act_file_equal(const char *mpath, const char *upath)
{
	const char *slash;

	/* NULL upath means user wants to match all files */
	if (upath == NULL) return true;

	if (mpath == NULL) return false;

	/* check for exact match */
	if (!strcmp(mpath, upath)) return true;

	/* upath may be the last component of mpath */
	slash = strrchr(mpath, '/');
	if (slash == NULL) return false;

	if (!strcmp(slash + 1, upath)) return true;
	return false;
}

static int
_act_file_checkpoint(asl_out_module_t *m, const char *path, uint32_t force)
{
	asl_out_rule_t *r;
	int did_checkpoint = 0;

	if (m == NULL) return 0;

	for (r = m->ruleset; r != NULL; r = r->next)
	{
		if ((r->action == ACTION_FILE) || (r->action == ACTION_ASL_FILE))
		{
			if (r->dst->flags & MODULE_FLAG_ROTATE)
			{
				if (_act_file_equal(r->dst->path, path))
				{
					if (_act_checkpoint(r, force) > 0)
					{
						did_checkpoint = 1;
						_act_dst_close(r, DST_CLOSE_CHECKPOINT);
					}
				}
			}
		}
	}

	return did_checkpoint;
}

static int
_act_file_checkpoint_all(uint32_t force)
{
	asl_out_module_t *m;
	int did_checkpoint = 0;

	for (m = global.asl_out_module; m != NULL; m = m->next)
	{
		if (_act_file_checkpoint(m, NULL, force) > 0) did_checkpoint = 1;
	}

	asl_trigger_aslmanager();

	return did_checkpoint;
}

/*
 * Save a message in a plain text file.
 */
static void
_act_file_final(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	asl_action_file_data_t *f_data;
	int is_dup;
	uint32_t len, msg_hash = 0;
	char *str;
	time_t now;

	if (r == NULL) return;
	if (r->dst == NULL) return;
	if (r->dst->private == NULL) return;

	f_data = (asl_action_file_data_t *)r->dst->private;
	if (f_data->pending > 0) f_data->pending--;

	/*
	 * If print format is std, bsd, or msg, then skip messages with
	 * no ASL_KEY_MSG, or without a value for it.
	 */
	if (r->dst->flags & MODULE_FLAG_STD_BSD_MSG)
	{
		const char *msgval = NULL;
		if (asl_msg_lookup(msg, ASL_KEY_MSG, &msgval, NULL) != 0) return;
		if (msgval == NULL) return;
	}

	now = time(NULL);

	is_dup = 0;

	str = asl_format_message(msg, r->dst->fmt, r->dst->tfmt, ASL_ENCODE_SAFE, &len);

	if (r->dst->flags & MODULE_FLAG_COALESCE)
	{
		if (f_data->dup_timer == NULL)
		{
			/* create a timer to flush dups on this file */
			f_data->dup_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, asl_action_queue);
			dispatch_source_set_event_handler(f_data->dup_timer, ^{ _send_repeat_msg(r); });
		}

		if ((global.bsd_max_dup_time > 0) && (str != NULL) && (f_data->last_msg != NULL))
		{
			msg_hash = asl_core_string_hash(str + 16, len - 16);
			if ((f_data->last_hash == msg_hash) && (!strcmp(f_data->last_msg, str + 16)))
			{
				if ((now - f_data->last_time) < global.bsd_max_dup_time) is_dup = 1;
			}
		}
	}

	if (is_dup == 1)
	{
		if (f_data->last_count == 0)
		{
			/* start the timer */
			dispatch_source_set_timer(f_data->dup_timer, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * global.bsd_max_dup_time), DISPATCH_TIME_FOREVER, 0);
			dispatch_resume(f_data->dup_timer);
		}

		f_data->last_count++;
	}
	else
	{
		if (_act_dst_open(r, NULL, 0) != 0)
		{
			_asl_action_save_failed("_act_file", m, r, ASL_STATUS_FAILED);
			free(str);
			return;
		}
		else
		{
			r->dst->fails = 0;
		}

		/*
		 * The current message is not a duplicate.  If f_data->last_count > 0
		 * we need to write a "last message repeated N times" log entry.
		 * _send_repeat_msg will free last_msg and do nothing if
		 * last_count == 0, but we test and free here to avoid a function call.
		 */
		if (f_data->last_count > 0)
		{
			_send_repeat_msg(r);
		}
		else
		{
			free(f_data->last_msg);
			f_data->last_msg = NULL;
		}

		if (str != NULL) f_data->last_msg = strdup(str + 16);

		f_data->last_hash = msg_hash;
		f_data->last_count = 0;
		f_data->last_time = now;

		if ((str != NULL) && (len > 1))
		{
			/* write line to file and update dst size */
			size_t bytes = write(f_data->fd, str, len - 1);
			if (bytes > 0) r->dst->size += bytes;

			if (_act_checkpoint(r, CHECKPOINT_TEST) == 1) asl_trigger_aslmanager();
		}
	}

	free(str);
}

static void
_act_file(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	asl_action_file_data_t *f_data;

	if (r == NULL) return;
	if (msg == NULL) return;
	if (m == NULL) return;
	if ((m->flags & MODULE_FLAG_ENABLED) == 0) return;
	if (r->dst == NULL) return;
	if (r->dst->private == NULL) return;

	if (r->dst->flags & MODULE_FLAG_HAS_LOGGED) return;

	r->dst->flags |= MODULE_FLAG_HAS_LOGGED;
	f_data = (asl_action_file_data_t *)r->dst->private;
	if (f_data != NULL) f_data->pending++;

#if TARGET_OS_EMBEDDED
	if (r->dst->flags & MODULE_FLAG_CRASHLOG)
	{
		_crashlog_queue_check();
		asl_msg_retain(msg);
		dispatch_async(crashlog_queue, ^{
			dispatch_async(asl_action_queue, ^{
				_act_file_final(m, r, msg);
				asl_msg_release(msg);
			});
		});
		return;
	}
#endif

	_act_file_final(m, r, msg);
}

static void
_act_forward(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	/* To do: <rdar://problem/6130747> Add a "forward" action to asl.conf */
}

static void
_act_control(asl_out_module_t *m, asl_out_rule_t *r, asl_msg_t *msg)
{
	const char *p;

	if (m == NULL) return;
	if (r == NULL) return;

	p = asl_msg_get_val_for_key(msg, ASL_KEY_MODULE);

	if (r->options == NULL) return;

	if (!strcmp(r->options, "enable"))
	{
		m->flags |= MODULE_FLAG_ENABLED;
	}
	else if (!strcmp(r->options, "disable"))
	{
		m->flags &= ~MODULE_FLAG_ENABLED;
	}
	else if ((!strcmp(r->options, "checkpoint")) || (!strcmp(r->options, "rotate")))
	{
		_act_file_checkpoint(m, NULL, CHECKPOINT_FORCE);
	}
}

static void
_send_to_asl_store(asl_msg_t *msg)
{
	if ((global.asl_out_module != NULL) && ((global.asl_out_module->flags & MODULE_FLAG_ENABLED) == 0)) return;

	if (store_has_logged) return;
	store_has_logged = true;

	db_save_message(msg);
}

static int
_asl_out_process_message(asl_out_module_t *m, asl_msg_t *msg)
{
	asl_out_rule_t *r;

	if (m == NULL) return 1;
	if (msg == NULL) return 1;

	/* reset flag bit used for duplicate avoidance */
	for (r = m->ruleset; r != NULL; r = r->next)
	{
		if ((r->action == ACTION_FILE) || (r->action == ACTION_ASL_DIR) || (r->action == ACTION_ASL_FILE))
		{
			if (r->dst != NULL) r->dst->flags &= MODULE_FLAG_CLEAR_LOGGED;
		}
	}

	for (r = m->ruleset; r != NULL; r = r->next)
	{
		if (r->query == NULL) continue;

		/* ACTION_SET_FILE, ACTION_SET_PLIST, and ACTION_SET_PROF are handled independently  */
		if ((r->action == ACTION_SET_FILE) || (r->action == ACTION_SET_PLIST) || (r->action == ACTION_SET_PROF)) continue;

		/*
		 * ACTION_CLAIM during processing is a filter.  It will only be here if the option "only"
		 * was supplied.  In this case we test the message against the query.  If it does not
		 * match, we skip the message.
		 */
		if (r->action == ACTION_CLAIM)
		{
			if ((asl_msg_cmp(r->query, msg) != 1)) return 0;
		}

		if ((asl_msg_cmp(r->query, msg) == 1))
		{
			if (r->action == ACTION_NONE) continue;
			else if (r->action == ACTION_IGNORE) return 1;
			else if (r->action == ACTION_SKIP) return 0;
			else if (r->action == ACTION_ASL_STORE) _send_to_asl_store(msg);
			else if (r->action == ACTION_ACCESS) _act_access_control(m, r, msg);
			else if (r->action == ACTION_SET_KEY) _act_set_key(m, r, msg);
			else if (r->action == ACTION_UNSET_KEY) _act_unset_key(m, r, msg);
			else if (r->action == ACTION_NOTIFY) _act_notify(m, r);
			else if (r->action == ACTION_BROADCAST) _act_broadcast(m, r, msg);
			else if (r->action == ACTION_FORWARD) _act_forward(m, r, msg);
			else if (r->action == ACTION_CONTROL) _act_control(m, r, msg);
			else if (r->action == ACTION_SET_PARAM) _act_out_set_param(m, r->options, true);
			else if ((r->action == ACTION_ASL_FILE) || (r->action == ACTION_ASL_DIR)) _act_store(m, r, msg);
			else if (r->action == ACTION_FILE) _act_file(m, r, msg);
		}
	}

	return 0;
}

void
asl_out_message(asl_msg_t *msg, int64_t msize)
{
	OSAtomicIncrement32(&global.asl_queue_count);
	asl_msg_retain(msg);

	dispatch_async(asl_action_queue, ^{
		int ignore = 0;
		const char *p;
		time_t now = time(NULL);
		asl_out_module_t *m = global.asl_out_module;

		store_has_logged = false;

		p = asl_msg_get_val_for_key(msg, ASL_KEY_MODULE);
		if (p == NULL)
		{
			if ((action_asl_store_count == 0) || (asl_check_option(msg, ASL_OPT_STORE) == 1)) _send_to_asl_store(msg);

			ignore = _asl_out_process_message(m, msg);
			if (ignore == 0)
			{
				if (m != NULL) m = m->next;
				while (m != NULL)
				{
					_asl_out_process_message(m, msg);
					m = m->next;
				}
			}
		}
		else 
		{
			if (m != NULL) m = m->next;
			while (m != NULL)
			{
				if (!strcmp(p, m->name)) _asl_out_process_message(m, msg);
				m = m->next;
			}
		}

		p = asl_msg_get_val_for_key(msg, ASL_KEY_FINAL_NOTIFICATION);
		if (p != NULL) asl_msg_set_key_val(msg, ASL_KEY_FREE_NOTE, p);

		/* chain to the next output module (done this way to make queue size accounting easier */
#if !TARGET_OS_SIMULATOR
		if (global.bsd_out_enabled) bsd_out_message(msg, msize);
		else OSAtomicAdd64(-1ll * msize, &global.memory_size);
#else
		OSAtomicAdd64(-1ll * msize, &global.memory_size);
#endif

		asl_msg_release(msg);
		OSAtomicDecrement32(&global.asl_queue_count);

		if ((now - sweep_time) >= IDLE_CLOSE)
		{
			_asl_action_close_idle_files(IDLE_CLOSE);
			sweep_time = now;
		}
	});
}

static char *
_asl_action_profile_test(asl_out_module_t *m, asl_out_rule_t *r)
{
	const char *ident;
	asl_msg_t *profile;
	bool eval;

	/* ident is first message key */
	asl_msg_fetch(r->query, 0, &ident, NULL, NULL);
	if (ident == NULL)
	{
		r->action = ACTION_NONE;
		return NULL;
	}

	profile = configuration_profile_to_asl_msg(ident);
	eval = (asl_msg_cmp(r->query, profile) == 1);
	_act_out_set_param(m, r->options, eval);
	asl_msg_release(profile);

	return strdup(ident);
}

static const char *
_asl_action_file_test(asl_out_module_t *m, asl_out_rule_t *r)
{
	const char *path;
	struct stat sb;
	int status;
	bool eval;

	/* path is first message key */
	asl_msg_fetch(r->query, 0, &path, NULL, NULL);
	if (path == NULL)
	{
		r->action = ACTION_NONE;
		return NULL;
	}

	memset(&sb, 0, sizeof(struct stat));
	status = stat(path, &sb);
	eval = (status == 0);
	_act_out_set_param(m, r->options, eval);

	return path;
}

static void
_asl_action_handle_file_change_notification(int t)
{
	asl_out_module_t *m;
	asl_out_rule_t *r;

	for (m = global.asl_out_module; m != NULL; m = m->next)
	{
		for (r = m->ruleset; r != NULL; r = r->next)
		{
			if (r->action == ACTION_SET_FILE)
			{
				asl_action_set_param_data_t *spdata = (asl_action_set_param_data_t *)r->private;
				if ((spdata != NULL) && (spdata->token == t))
				{
					_asl_action_file_test(m, r);
					return;
				}
			}
			else if (r->action == ACTION_SET_PLIST)
			{
				asl_action_set_param_data_t *spdata = (asl_action_set_param_data_t *)r->private;
				if ((spdata != NULL) && (spdata->token == t))
				{
					char *str = _asl_action_profile_test(m, r);
					free(str);
					return;
				}
			}
			else if (r->action == ACTION_SET_PROF)
			{
				asl_action_set_param_data_t *spdata = (asl_action_set_param_data_t *)r->private;
				if ((spdata != NULL) && (spdata->token == t))
				{
					char *str = _asl_action_profile_test(m, r);
					free(str);
					return;
				}
			}
		}
	}

	asl_out_module_free(m);
}

static void
_asl_action_post_process_rule(asl_out_module_t *m, asl_out_rule_t *r)
{
	if ((m == NULL) || (r == NULL)) return;

	if (m != global.asl_out_module)
	{
		/* check if any previous module has used this destination */
		asl_out_module_t *n;
		bool search = true;

		if ((r->dst != NULL) && (r->dst->path != NULL))
		{
			for (n = global.asl_out_module; search && (n != NULL) && (n != m); n = n->next)
			{
				asl_out_rule_t *s;
				for (s = n->ruleset; search && (s != NULL); s = s->next)
				{
					if (s->action == ACTION_OUT_DEST)
					{
						if ((s->dst != NULL) && (s->dst->path != NULL) && (!strcmp(r->dst->path, s->dst->path)))
						{
							/* rule r of module m is using previously used dst of rule s of module n */
							asl_out_dst_data_release(r->dst);
							r->dst = NULL;

							if (r->action == ACTION_OUT_DEST)
							{
								char *str = NULL;
								asprintf(&str, "[Sender syslogd] [Level 5] [PID %u] [Message Configuration Notice:\nASL Module \"%s\" sharing output destination \"%s\" with ASL Module \"%s\".\nOutput parameters from ASL Module \"%s\" override any specified in ASL Module \"%s\".] [UID 0] [GID 0] [Facility syslog]", global.pid, m->name, s->dst->path, n->name, n->name, m->name);
								internal_log_message(str);
								free(str);
							}
							else
							{
								r->dst = asl_out_dst_data_retain(s->dst);
							}

							search = false;
						}
					}
				}
			}
		}
	}

	if (r->action == ACTION_SET_PARAM)
	{
		if (r->query == NULL) _act_out_set_param(m, r->options, true);
	}
	else if (r->action == ACTION_CLAIM)
	{
		/* becomes ACTION_SKIP in com.apple.asl config */
		if (m != global.asl_out_module)
		{
			asl_out_rule_t *rule = (asl_out_rule_t *)calloc(1, sizeof(asl_out_rule_t));
			if (rule != NULL)
			{
				char *str = NULL;
				asprintf(&str, "[Sender syslogd] [Level 5] [PID %u] [Message Configuration Notice:\nASL Module \"%s\" claims selected messages.\nThose messages may not appear in standard system log files or in the ASL database.] [UID 0] [GID 0] [Facility syslog]", global.pid, m->name);
				internal_log_message(str);
				free(str);

				rule->query = asl_msg_copy(r->query);
				rule->action = ACTION_SKIP;
				rule->next = global.asl_out_module->ruleset;
				global.asl_out_module->ruleset = rule;
			}

			/*
			 * After adding ACTION_SKIP to com.apple.asl module, the claim becomes a no-op in this module
			 * UNLESS the claim includes the option "only".  In that case, the claim becomes a filter:
			 * any messages that DO NOT match the claim are skipped by this module.
			 */
			if (r->options == NULL) r->action = ACTION_NONE;
			else if (strcmp(r->options, "only") != 0) r->action = ACTION_NONE;
		}
	}
	else if (r->action == ACTION_ASL_STORE)
	{
		action_asl_store_count++;
	}
	else if (r->action == ACTION_ASL_DIR)
	{
		if (r->dst->private == NULL) r->dst->private = (asl_action_asl_store_data_t *)calloc(1, sizeof(asl_action_asl_store_data_t));
	}
	else if (r->action == ACTION_ASL_FILE)
	{
		if (r->dst->private == NULL)r->dst->private = (asl_action_asl_file_data_t *)calloc(1, sizeof(asl_action_asl_file_data_t));
	}
	else if (r->action == ACTION_FILE)
	{
		if (r->dst->private == NULL) r->dst->private = (asl_action_file_data_t *)calloc(1, sizeof(asl_action_file_data_t));
		if (r->dst->private != NULL) ((asl_action_file_data_t *)(r->dst->private))->fd = -1;
	}
	else if (r->action == ACTION_SET_PLIST)
	{
		char *ident =_asl_action_profile_test(m, r);
		char *notify_key = configuration_profile_create_notification_key(ident);
		free(ident);

		if (notify_key != NULL)
		{
			int status, token;
			asl_action_set_param_data_t *spdata;

			status = notify_register_dispatch(notify_key, &token, asl_action_queue, ^(int t){
				_asl_action_handle_file_change_notification(t);
			});

			free(notify_key);

			spdata = (asl_action_set_param_data_t *)calloc(1, sizeof(asl_action_set_param_data_t));
			if (spdata == NULL)
			{
				notify_cancel(token);
			}
			else
			{
				spdata->token = token;
				r->private = spdata;
			}
		}
	}
	else if (r->action == ACTION_SET_PROF)
	{
		char *ident =_asl_action_profile_test(m, r);
		char *notify_key = configuration_profile_create_notification_key(ident);
		free(ident);

		if (notify_key != NULL)
		{
			int status, token;
			asl_action_set_param_data_t *spdata;

			status = notify_register_dispatch(notify_key, &token, asl_action_queue, ^(int t){
				_asl_action_handle_file_change_notification(t);
			});

			free(notify_key);

			spdata = (asl_action_set_param_data_t *)calloc(1, sizeof(asl_action_set_param_data_t));
			if (spdata == NULL)
			{
				notify_cancel(token);
			}
			else
			{
				spdata->token = token;
				r->private = spdata;
			}
		}
	}
	else if (r->action == ACTION_SET_FILE)
	{
		char *notify_key;
		const char *path =_asl_action_file_test(m, r);

		if (path != NULL)
		{
			asprintf(&notify_key, "%s%s", NOTIFY_PATH_SERVICE, path);
			if (notify_key != NULL)
			{
				int status, token;
				asl_action_set_param_data_t *spdata;

				status = notify_register_dispatch(notify_key, &token, asl_action_queue, ^(int t){
					_asl_action_handle_file_change_notification(t);
				});

				free(notify_key);

				spdata = (asl_action_set_param_data_t *)calloc(1, sizeof(asl_action_set_param_data_t));
				if (spdata == NULL)
				{
					notify_cancel(token);
				}
				else
				{
					spdata->token = token;
					r->private = spdata;
				}
			}
		}
	}
}

static void
_asl_action_configure()
{
	asl_out_rule_t *r;
	asl_out_module_t *m;
	uint32_t flags = 0;

	if (global.asl_out_module == NULL) global.asl_out_module = asl_out_module_init();
	if (global.asl_out_module == NULL) return;

	asldebug("%s: init\n", MY_ID);

	action_asl_store_count = 0;

	for (m = global.asl_out_module; m != NULL; m = m->next)
	{
		for (r = m->ruleset; r != NULL; r = r->next)
		{
			_asl_action_post_process_rule(m, r);
			if (r->dst != NULL) flags |= (r->dst->flags & (MODULE_FLAG_ROTATE | MODULE_FLAG_CRASHLOG));
		}
	}

	if (global.debug != 0)
	{
		FILE *dfp;
		if (global.debug_file == NULL) dfp = fopen(_PATH_SYSLOGD_LOG, "a");
		else dfp = fopen(global.debug_file, "a");
		if (dfp != NULL)
		{
			for (m = global.asl_out_module; m != NULL; m = m->next)
			{
				fprintf(dfp, "module: %s%s\n", (m->name == NULL) ? "<unknown>" : m->name, (m->flags & MODULE_FLAG_LOCAL) ? " (local)" : "");
				asl_out_module_print(dfp, m);
				fprintf(dfp, "\n");
			}
			fclose(dfp);
		}
	}

	sweep_time = time(NULL);

	if (flags & MODULE_FLAG_ROTATE)
	{
		_act_file_checkpoint_all(CHECKPOINT_TEST);
		if (checkpoint_timer == NULL) _start_cycling();
	}
}

int
asl_action_init(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		asl_action_queue = dispatch_queue_create("ASL Action Queue", NULL);
#if TARGET_OS_EMBEDDED
		crashlog_queue = dispatch_queue_create("iOS CrashLog Queue", NULL);
		notify_register_dispatch(CRASH_MOVER_SERVICE, &crashmover_token, asl_action_queue, ^(int unused) {
			uint64_t cmstate = 0;
			uint64_t oldstate = (crashmover_state == 0) ? 0llu : 1llu;

			uint32_t status = notify_get_state(crashmover_token, &cmstate);
			if (status == 0)
			{
				if (cmstate != oldstate)
				{
					crashmover_state = 0;
					if (cmstate == 1) crashmover_state = time(NULL);

					if (crashmover_state == 0)
					{
						asldebug("CrashMover finished\n");
						dispatch_resume(crashlog_queue);
					}
					else
					{
						asldebug("CrashMover active: suspending crashlog queue and closing files\n");
						dispatch_suspend(crashlog_queue);
						_asl_action_close_idle_files(0);
					}
				}
			}
		});
#endif
	});

	_asl_action_configure();

	return 0;
}

/*
 * Close outputs and free modules.
 */
static void
_asl_action_free_modules(asl_out_module_t *m)
{
	asl_out_rule_t *r;
	asl_out_module_t *x;

	/*
	 * asl_common frees a list of modules with asl_out_module_free.
	 * This loop frees the private data attached some modules.
	 */
	for (x = m; x != NULL; x = x->next)
	{
		for (r = x->ruleset; r != NULL; r = r->next)
		{
			if (r->action == ACTION_ASL_DIR)
			{
				_act_dst_close(r, DST_CLOSE_SHUTDOWN);
				if (r->dst != NULL)
				{
					_asl_action_asl_store_data_free((asl_action_asl_store_data_t *)r->dst->private);
					r->dst->private = NULL;
				}
			}
			else if (r->action == ACTION_ASL_FILE)
			{
				_act_dst_close(r, DST_CLOSE_SHUTDOWN);
				if (r->dst != NULL)
				{
					_asl_action_asl_file_data_free((asl_action_asl_file_data_t *)r->dst->private);
					r->dst->private = NULL;
				}
			}
			else if (r->action == ACTION_FILE)
			{
				_act_dst_close(r, DST_CLOSE_SHUTDOWN);
				if (r->dst != NULL)
				{
					asl_action_file_data_t *f_data = (asl_action_file_data_t *)r->dst->private;
					if (f_data != NULL)
					{
						/* flush repeat message if necessary */
						if (f_data->last_count > 0) _send_repeat_msg(r);
						_asl_action_file_data_free(f_data);
						r->dst->private = NULL;
					}
				}
			}
			else if (r->action == ACTION_SET_PLIST)
			{
				_asl_action_set_param_data_free((asl_action_set_param_data_t *)r->private);
			}
			else if (r->action == ACTION_SET_PROF)
			{
				_asl_action_set_param_data_free((asl_action_set_param_data_t *)r->private);
			}
			else if (r->action == ACTION_SET_FILE)
			{
				_asl_action_set_param_data_free((asl_action_set_param_data_t *)r->private);
			}
		}
	}

	asl_out_module_free(m);
}

static int
_asl_action_close_internal(void)
{
#if TARGET_OS_EMBEDDED
	if (crashmover_state != 0)
	{
		dispatch_resume(crashlog_queue);
		crashmover_state = 0;
	}

	/* wait for the crashlog_queue to flush before _asl_action_free_modules() */
	dispatch_sync(crashlog_queue, ^{ int x = 0; if (x == 1) x = 2; });
#endif

	_asl_action_free_modules(global.asl_out_module);
	global.asl_out_module = NULL;
	sweep_time = time(NULL);

	return 0;
}

static void
_asl_action_close_idle_files(time_t idle_time)
{
	asl_out_module_t *m;
	time_t now = time(NULL);

	for (m = global.asl_out_module; m != NULL; m = m->next)
	{
		asl_out_rule_t *r;

		for (r = m->ruleset; r != NULL; r = r->next)
		{
			if (idle_time == 0)
			{
				if ((r->dst != NULL) && (r->dst->flags & MODULE_FLAG_CRASHLOG))
				{
					_act_dst_close(r, DST_CLOSE_IDLE);
					//TODO: can r->action even be ACTION_ASL_DIR?
					/* if not, we can avoid the extra check here */
					if (r->action != ACTION_ASL_DIR) _act_checkpoint(r, CHECKPOINT_FORCE);
				}
			}
			else if (r->action == ACTION_ASL_DIR)
			{
				if (r->dst != NULL)
				{
					asl_action_asl_store_data_t *as_data = (asl_action_asl_store_data_t *)r->dst->private;
					if ((as_data != NULL) && (as_data->aslfile != NULL) && (as_data->pending == 0) && ((now - as_data->last_time) >= idle_time)) _act_dst_close(r, DST_CLOSE_IDLE);
				}
			}
			else if (r->action == ACTION_ASL_FILE)
			{
				if (r->dst != NULL)
				{
					asl_action_asl_file_data_t *af_data = (asl_action_asl_file_data_t *)r->dst->private;
					if ((af_data != NULL) && (af_data->aslfile != NULL) && (af_data->pending == 0) && ((now - af_data->last_time) >= idle_time)) _act_dst_close(r, DST_CLOSE_IDLE);
				}
			}
			else if (r->action == ACTION_FILE)
			{
				if (r->dst != NULL)
				{
					asl_action_file_data_t *f_data = (asl_action_file_data_t *)r->dst->private;
					if ((f_data != NULL) && (f_data->fd >= 0) && (f_data->pending == 0) && ((now - f_data->last_time) >= idle_time)) _act_dst_close(r, DST_CLOSE_IDLE);
				}
			}
		}
	}
}

int
asl_action_close(void)
{
	dispatch_async(asl_action_queue, ^{
		_asl_action_close_internal();
	});

	return 0;
}

int
asl_action_reset(void)
{
	dispatch_async(asl_action_queue, ^{
		_asl_action_close_internal();
		asl_action_init();
	});

	return 0;
}

asl_out_module_t *
_asl_action_module_with_name(const char *name)
{
	asl_out_module_t *m;

	if (global.asl_out_module == NULL) return NULL;
	if (name == NULL) return global.asl_out_module;

	for (m = global.asl_out_module; m != NULL; m = m->next)
	{
		if ((m->name != NULL) && (!strcmp(m->name, name))) return m;
	}

	return NULL;
}

/*
 * called from control_message 
 * Used to control modules dynamically.
 * Line format "@ module param [value ...]"
 *
 * Note this is synchronous on asl_action queue.
 */
int
asl_action_control_set_param(const char *s)
{
	__block char **l;
	uint32_t count = 0;

	if (s == NULL) return -1;
	if (s[0] == '\0') return 0;

	/* skip '@' and whitespace */
	if (*s == '@') s++;
	while ((*s == ' ') || (*s == '\t')) s++;

	l = explode(s, " \t");
	if (l != NULL) for (count = 0; l[count] != NULL; count++);

	/* at least 2 parameters (l[0] = module, l[1] = param) required */
	if (count < 2)
	{
		free_string_list(l);
		return -1;
	}

	if (global.asl_out_module == NULL)
	{
		asldebug("asl_action_control_set_param: no modules loaded\n");
		free_string_list(l);
		return -1;
	}

	/* create / modify a module */
	if ((!strcasecmp(l[1], "define")) && (strcmp(l[0], "*")))
	{
		char *str = strdup(s);
		if (str == NULL)
		{
			asldebug("asl_action_control_set_param: memory allocation failed\n");
			free_string_list(l);
			return -1;
		}

		dispatch_sync(asl_action_queue, ^{
			asl_out_module_t *m;
			asl_out_rule_t *r;
			char *p = str;

			/* skip name, whitespace, "define" */
			while ((*p != ' ') && (*p != '\t')) p++;
			while ((*p == ' ') || (*p == '\t')) p++;
			while ((*p != ' ') && (*p != '\t')) p++;

			m = _asl_action_module_with_name(l[0]);
			if (m == NULL)
			{
				asl_out_module_t *x;

				m = asl_out_module_new(l[0]);
				for (x = global.asl_out_module; x->next != NULL; x = x->next);
				x->next = m;
			}

			r = asl_out_module_parse_line(m, p);
			if (r != NULL)
			{
				_asl_action_post_process_rule(m, r);
				if ((r->dst != NULL) && (r->dst->flags & MODULE_FLAG_ROTATE)) 
				{
					_act_file_checkpoint_all(CHECKPOINT_TEST);
					if (checkpoint_timer == NULL) _start_cycling();
				}
			}
		});

		free(str);
		free_string_list(l);
		return 0;
	}

	dispatch_sync(asl_action_queue, ^{
		uint32_t intval;
		int do_all = 0;
		asl_out_module_t *m;

		if (!strcmp(l[0], "*"))
		{
			do_all = 1;
			m = _asl_action_module_with_name(NULL);
		}
		else
		{
			m = _asl_action_module_with_name(l[0]);
		}

		while (m != NULL) 
		{
			if (!strcasecmp(l[1], "enable"))
			{
				intval = 1;

				/* don't do enable for ASL_MODULE_NAME if input name is "*" */
				if ((do_all == 0) || (strcmp(m->name, ASL_MODULE_NAME)))
				{
					/* @ module enable {0|1} */
					if (count > 2) intval = atoi(l[2]);

					if (intval == 0) m->flags &= ~MODULE_FLAG_ENABLED;
					else m->flags |= MODULE_FLAG_ENABLED;
				}
			}
			else if (!strcasecmp(l[1], "checkpoint"))
			{
				/* @ module checkpoint [file] */
				if (count > 2) _act_file_checkpoint(m, l[2], CHECKPOINT_FORCE);
				else _act_file_checkpoint(m, NULL, CHECKPOINT_FORCE);
			}

			if (do_all == 1) m = m->next;
			else m = NULL;
		}

	});

	free_string_list(l);
	return 0;
}

void
asl_action_out_module_query(asl_msg_t *q, asl_msg_t *m, bool all)
{
	dispatch_sync(asl_action_queue, ^{
		asl_out_module_t *om;
		const char *val;
		for (om = global.asl_out_module; om != NULL; om = om->next)
		{
			if (all || (0 == asl_msg_lookup(q, om->name, NULL, NULL)))
			{
				val = om->flags & MODULE_FLAG_ENABLED ? "enabled" : "disabled";
				if (om->name == NULL) asl_msg_set_key_val(m, "asl.conf", val);
				else asl_msg_set_key_val(m, om->name, val);
			}
		}
	});
}
