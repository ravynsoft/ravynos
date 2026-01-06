/*
 * Copyright (c) 2004-2015 Apple Inc. All rights reserved.
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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/fileport.h>
#include <crt_externs.h>
#include <asl.h>
#include <regex.h>
#include <notify.h>
#include <mach/mach.h>
#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mach_types.h>
#include <sys/types.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <libkern/OSAtomic.h>
#include <os/activity.h>
#include <os/log.h>
#include <os/log_private.h>
#include <asl_ipc.h>
#include <asl_client.h>
#include <asl_core.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <asl_store.h>
#include <asl_private.h>

#define forever for(;;)

#define FETCH_BATCH	256

#define EVAL_DEFAULT_ACTION (EVAL_SEND_ASL | EVAL_SEND_TRACE)
#define EVAL_ASL (EVAL_SEND_ASL | EVAL_TEXT_FILE | EVAL_ASL_FILE)

/*
 * Clients get a max of 36000 messages per hour.
 * Their quota gets refilled at a rate of 10 messages per second.
 */
#define QUOTA_MPH 36000
#define QUOTA_MPS 10
#define QUOTA_MSG_INTERVAL 60
#define NOQUOTA_ENV "ASL_QUOTA_DISABLED"
#define QUOTA_DISABLED_MSG "*** MESSAGE QUOTA DISABLED FOR THIS PROCESS ***"
#define QUOTA_MSG "*** LOG MESSAGE QUOTA EXCEEDED - SOME MESSAGES FROM THIS PROCESS HAVE BEEN DISCARDED ***"
#define QUOTA_LEVEL ASL_LEVEL_CRIT
#define QUOTA_LEVEL_STR ASL_STRING_CRIT

/*
 * Limit the size of messages sent to syslogd.
 */
#define SIZE_LIMIT_MSG "*** ASL MESSAGE SIZE (%u bytes) EXCEEDED MAXIMIMUM SIZE (%u bytes) ***"

static const os_log_type_t shim_asl_to_log_type[8] = {
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_EMERG
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_ALERT
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_CRIT
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_ERR
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_WARNING
	OS_LOG_TYPE_DEFAULT,    // ASL_LEVEL_NOTICE
	OS_LOG_TYPE_INFO,       // ASL_LEVEL_INFO
	OS_LOG_TYPE_DEBUG       // ASL_LEVEL_DEBUG
};

/* forward */
static ASL_STATUS _asl_send_message(asl_object_t obj, uint32_t eval, asl_msg_t *msg, const char *mstring);
static ASL_STATUS _asl_send_message_text(asl_client_t *asl, asl_msg_t *sendmsg, asl_object_t obj, uint32_t eval, asl_msg_t *msg, const char *mstring, bool shimmed);
__private_extern__ asl_client_t *_asl_open_default();

/* notify SPI */
uint32_t notify_register_plain(const char *name, int *out_token);

/* fork handling in asl_fd.c */
extern void _asl_redirect_fork_child(void);

#if TARGET_OS_OSX
extern void _asl_mt_shim_fork_child(void);
extern void _asl_mt_shim_send_message(asl_msg_t *msg);
#endif

typedef struct
{
	int fd;
	asl_msg_t *msg;
	dispatch_semaphore_t sem;
} asl_aux_context_t;

typedef struct
{
	int notify_count;
	int rc_change_token;
	int notify_token;
	int master_token;
	uint64_t proc_filter;
	uint64_t master_filter;
	time_t last_send;
	time_t last_oq_msg;
	uint32_t quota;
	dispatch_once_t port_lookup_once;
	mach_port_t server_port;
	char *sender;
	pthread_mutex_t lock;
	int aux_count;
	asl_aux_context_t **aux_ctx;
	asl_client_t *asl;
} _asl_global_t;

__private_extern__ _asl_global_t _asl_global = {0, -1, -1, -1, 0LL, 0LL, 0LL, 0LL, 0, 0, MACH_PORT_NULL, NULL, PTHREAD_MUTEX_INITIALIZER, 0, NULL, NULL};

static const char *level_to_number_string[] = {"0", "1", "2", "3", "4", "5", "6", "7"};

#define ASL_SERVICE_NAME "com.apple.system.logger"

/*
 * Called from the child process inside fork() to clean up
 * inherited state from the parent process.
 *
 * NB. A lock isn't required, since we're single threaded in this call.
 */
void
_asl_fork_child()
{
	_asl_global.notify_count = 0;
	_asl_global.rc_change_token = -1;
	_asl_global.master_token = -1;
	_asl_global.notify_token = -1;
	_asl_global.quota = 0;
	_asl_global.last_send = 0;
	_asl_global.last_oq_msg = 0;

	_asl_global.port_lookup_once = 0;
	_asl_global.server_port = MACH_PORT_NULL;

	pthread_mutex_init(&(_asl_global.lock), NULL);

	_asl_redirect_fork_child();
#if TARGET_OS_OSX
	_asl_mt_shim_fork_child();
#endif
}

/*
 * asl_remote_notify_name: returns the notification key for remote-control filter
 * changes for this process.
 */
char *
asl_remote_notify_name()
{
	pid_t pid = getpid();
	uid_t euid = geteuid();
	char *str = NULL;

	if (euid == 0) asprintf(&str, "%s.%d", NOTIFY_PREFIX_SYSTEM, pid);
	else asprintf(&str, "user.uid.%d.syslog.%d", euid, pid);

	return str;
}

static ASL_STATUS
_asl_notify_open(int do_lock)
{
	char *notify_name;
	uint32_t status;

	if (do_lock != 0) pthread_mutex_lock(&_asl_global.lock);

	_asl_global.notify_count++;

	if (_asl_global.notify_token != -1)
	{
		if (do_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
		return ASL_STATUS_OK;
	}

	if (_asl_global.rc_change_token == -1)
	{
		status = notify_register_check(NOTIFY_RC, &_asl_global.rc_change_token);
		if (status != NOTIFY_STATUS_OK) _asl_global.rc_change_token = -1;
	}

	if (_asl_global.master_token == -1)
	{
		status = notify_register_plain(NOTIFY_SYSTEM_MASTER, &_asl_global.master_token);
		if (status != NOTIFY_STATUS_OK) _asl_global.master_token = -1;
	}

	notify_name = asl_remote_notify_name();
	if (notify_name != NULL)
	{
		status = notify_register_plain(notify_name, &_asl_global.notify_token);
		free(notify_name);
		if (status != NOTIFY_STATUS_OK) _asl_global.notify_token = -1;
	}

	if (do_lock != 0) pthread_mutex_unlock(&_asl_global.lock);

	if (_asl_global.notify_token == -1) return ASL_STATUS_FAILED;
	return ASL_STATUS_OK;
}

#ifdef UNDEF
static void
_asl_notify_close()
{
	pthread_mutex_lock(&_asl_global.lock);

	if (_asl_global.notify_count > 0) _asl_global.notify_count--;

	if (_asl_global.notify_count > 0)
	{
		pthread_mutex_unlock(&_asl_global.lock);
		return;
	}

	if (_asl_global.rc_change_token >= 0) notify_cancel(_asl_global.rc_change_token);
	_asl_global.rc_change_token = -1;

	if (_asl_global.master_token >= 0) notify_cancel(_asl_global.master_token);
	_asl_global.master_token = -1;

	if (_asl_global.notify_token >= 0) notify_cancel(_asl_global.notify_token);
	_asl_global.notify_token = -1;

	pthread_mutex_unlock(&_asl_global.lock);
}
#endif

static void
_asl_global_init()
{
	dispatch_once(&_asl_global.port_lookup_once, ^{
		char *str = getenv("ASL_DISABLE");
		if ((str == NULL) || strcmp(str, "1"))
		{
			bootstrap_look_up2(bootstrap_port, ASL_SERVICE_NAME, &_asl_global.server_port, 0, BOOTSTRAP_PRIVILEGED_SERVER);
		}
	});
}

mach_port_t
asl_core_get_service_port(__unused int reset)
{
	_asl_global_init();
	return _asl_global.server_port;
}

#pragma mark -
#pragma mark asl_client

asl_object_t
asl_open(const char *ident, const char *facility, uint32_t opts)
{
	asl_client_t *asl = asl_client_open(ident, facility, opts);
	if (asl == NULL) return NULL;

	if (!(opts & ASL_OPT_NO_REMOTE)) _asl_notify_open(1);

	return (asl_object_t)asl;
}

asl_object_t
asl_open_from_file(int fd, const char *ident, const char *facility)
{
	return (asl_object_t)asl_client_open_from_file(fd, ident, facility);
}

void
asl_close(asl_object_t obj)
{
	asl_release(obj);
}

__private_extern__ asl_client_t *
_asl_open_default()
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		/*
		 * Do a sleight-of-hand with ASL_OPT_NO_REMOTE to avoid a deadlock
		 * since asl_open(xxx, yyy, 0) calls _asl_notify_open(1)
		 * which locks _asl_global.lock.
		 */
		_asl_global.asl = (asl_client_t *)asl_open(NULL, NULL, ASL_OPT_NO_REMOTE);

		/* Reset options to clear ASL_OPT_NO_REMOTE bit */
		if (_asl_global.asl != NULL) _asl_global.asl->options = 0;

		/* Now call _asl_notify_open(0) to finish the work */
		_asl_notify_open(0);
	});

	return _asl_global.asl;
}

/*
 * asl_add_file: write log messages to the given file descriptor
 * Log messages will be written to this file as well as to the server.
 */
int
asl_add_output_file(asl_object_t client, int fd, const char *mfmt, const char *tfmt, int filter, int text_encoding)
{
	int status, use_global_lock = 0;
	asl_client_t *asl;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT)) return -1;

	asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return -1;
		pthread_mutex_lock(&_asl_global.lock);
		use_global_lock = 1;
	}

	status = asl_client_add_output_file(asl, fd, mfmt, tfmt, filter, text_encoding);

	if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
	return (status == ASL_STATUS_OK) ? 0 : -1;
}

/* returns previous filter value or -1 on error */
int
asl_set_output_file_filter(asl_object_t client, int fd, int filter)
{
	uint32_t last;
	int use_global_lock = 0;
	asl_client_t *asl;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT)) return -1;

	asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return -1;
		pthread_mutex_lock(&_asl_global.lock);
		use_global_lock = 1;
	}

	last = asl_client_set_output_file_filter(asl, fd, filter);

	if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
	return last;
}

/* SPI - Deprecated */
int
asl_add_output(asl_object_t client, int fd, const char *mfmt, const char *tfmt, uint32_t text_encoding)
{
	return asl_add_output_file(client, fd, mfmt, tfmt, ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG), text_encoding);
}

/* SPI - Deprecated */
int
asl_add_log_file(asl_object_t client, int fd)
{
	return asl_add_output_file(client, fd, ASL_MSG_FMT_STD, ASL_TIME_FMT_LCL, ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG), ASL_ENCODE_SAFE);
}

/*
 * asl_remove_output: stop writing log messages to the given file descriptor
 */
int
asl_remove_output_file(asl_object_t client, int fd)
{
	int status, use_global_lock = 0;
	asl_client_t *asl;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT)) return -1;

	asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return -1;
		pthread_mutex_lock(&_asl_global.lock);
		use_global_lock = 1;
	}

	status = asl_client_remove_output_file(asl, fd);

	if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
	return (status == ASL_STATUS_OK) ? 0 : -1;
}

int
asl_remove_output(asl_object_t client, int fd)
{
	return asl_remove_output_file(client, fd);
}

int
asl_remove_log_file(asl_object_t client, int fd)
{
	return asl_remove_output_file(client, fd);
}

/* returns previous filter value or -1 on error */
int
asl_set_filter(asl_object_t client, int f)
{
	int last, use_global_lock = 0;
	asl_client_t *asl;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT)) return -1;

	asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return -1;
		pthread_mutex_lock(&_asl_global.lock);
		use_global_lock = 1;
	}

	last = asl_client_set_filter(asl, f);

	if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
	return last;
}


#pragma mark -
#pragma mark message sending

/*
 * Evaluate client / message / level to determine what to do with a message.
 * Checks filters, tunneling, and log files.
 * Returns the bits below, ORed with the message level.
 *
 * EVAL_SEND_ASL	- send this message to syslogd
 * EVAL_SEND_TRACE	- log this message with Activity Tracing
 * EVAL_ASL_FILE	- write to a standalone ASL file (see asl_open_from_file)
 * EVAL_TEXT_FILE	- write this message to a text file / stderr
 * EVAL_TUNNEL		- tunneling enabled when sending to syslogd
 */
uint32_t
_asl_evaluate_send(asl_object_t client, asl_object_t m, int slevel)
{
	asl_client_t *asl;
	asl_msg_t *msg = (asl_msg_t *)m;
	uint32_t level, lmask, filter, status, tunnel;
	int check;
	uint64_t v64;
	const char *val;
	uint32_t eval;

	level = ASL_LEVEL_DEBUG;
	if (slevel >= 0) level = slevel;

	val = NULL;
	if ((asl_msg_lookup(msg, ASL_KEY_LEVEL, &val, NULL) == 0) && (val != NULL)) level = atoi(val);

	if (level < ASL_LEVEL_EMERG) level = ASL_LEVEL_EMERG;
	else if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT))
	{
		/* sending to something other than a client */
		return EVAL_ACTIVE | EVAL_SEND_ASL | level;
	}

	asl = (asl_client_t *)client;
	if ((asl != NULL) && (asl->aslfile != NULL)) return (EVAL_ASL_FILE | level);

	if (asl == NULL) asl = _asl_open_default();
	if (asl == NULL)
	{
		eval = EVAL_ACTIVE | EVAL_DEFAULT_ACTION | level;
		eval &= ~EVAL_SEND_ASL;
		return eval;
	}

	eval = (asl_client_get_control(asl) & EVAL_ACTION_MASK) | level;

	filter = asl->filter & 0xff;
	tunnel = (asl->filter & ASL_FILTER_MASK_TUNNEL) >> 8;
	if (tunnel != 0) eval |= EVAL_TUNNEL;

	if ((asl->options & ASL_OPT_NO_REMOTE) == 0)
	{
		pthread_mutex_lock(&_asl_global.lock);

		if (_asl_global.rc_change_token >= 0)
		{
			/* initialize or re-check process-specific and master filters  */
			check = 0;
			status = notify_check(_asl_global.rc_change_token, &check);
			if ((status == NOTIFY_STATUS_OK) && (check != 0))
			{
				if (_asl_global.master_token >= 0)
				{
					v64 = 0;
					status = notify_get_state(_asl_global.master_token, &v64);
					if (status == NOTIFY_STATUS_OK) _asl_global.master_filter = v64;
				}

				if (_asl_global.notify_token >= 0)
				{
					v64 = 0;
					status = notify_get_state(_asl_global.notify_token, &v64);
					if (status == NOTIFY_STATUS_OK) _asl_global.proc_filter = v64;
				}
			}
		}

		if (_asl_global.master_filter & EVAL_ACTIVE)
		{
			/* clear bits and set according to master */
			eval &= ~(EVAL_SEND_ASL | EVAL_SEND_TRACE);
			eval |= (_asl_global.master_filter & (EVAL_SEND_ASL | EVAL_SEND_TRACE));
			eval |= EVAL_TUNNEL;

			if ((_asl_global.master_filter & EVAL_LEVEL_MASK) != 0) filter = _asl_global.proc_filter & EVAL_LEVEL_MASK;
		}

		if (_asl_global.proc_filter & EVAL_ACTIVE)
		{
			/* clear bits and set according to proc */
			eval &= ~(EVAL_SEND_ASL | EVAL_SEND_TRACE | EVAL_TEXT_FILE);
			eval |= (_asl_global.proc_filter & (EVAL_SEND_ASL | EVAL_SEND_TRACE | EVAL_TEXT_FILE));
			eval |= EVAL_TUNNEL;

			if ((_asl_global.proc_filter & EVAL_LEVEL_MASK) != 0) filter = _asl_global.proc_filter & EVAL_LEVEL_MASK;
		}

		pthread_mutex_unlock(&_asl_global.lock);
	}

	lmask = ASL_FILTER_MASK(level);
	if ((filter != 0) && ((filter & lmask) == 0)) eval &= ~EVAL_SEND_ASL;
	if (asl->out_count > 0) eval |= EVAL_TEXT_FILE;

	/* don't send MessageTracer messages to Activity Tracing */
	val = NULL;
	if ((asl_msg_lookup(msg, ASL_KEY_MESSAGETRACER, &val, NULL) == 0) && (val != NULL))
	{
		eval &= ~EVAL_SEND_TRACE;
		eval |= EVAL_MT_SHIM;
	}

	/* don't send PowerManagement messages to Activity Tracing */
	val = NULL;
	if ((asl_msg_lookup(msg, ASL_KEY_POWERMANAGEMENT, &val, NULL) == 0) && (val != NULL)) eval &= ~EVAL_SEND_TRACE;

	/* don't send control messages to Activity Tracing */
	val = NULL;
	if ((asl_msg_lookup(msg, ASL_KEY_OPTION, &val, NULL) == 0) && (val != NULL)) eval &= ~EVAL_SEND_TRACE;

	/* don't send CFLog messages to Activity Tracing */
	val = NULL;
	if ((asl_msg_lookup(msg, ASL_KEY_CFLOG_LOCAL_TIME, &val, NULL) == 0) && (val != NULL)) eval &= ~EVAL_SEND_TRACE;

	val = NULL;
	if (((asl_msg_lookup(msg, ASL_KEY_FACILITY, &val, NULL) == 0) && (val != NULL)) ||
		((asl_msg_lookup(asl->kvdict, ASL_KEY_FACILITY, &val, NULL) == 0) && (val != NULL)))
	{
		/* don't send lastlog/utmp messages to Activity Tracing */
		if (!strcmp(val, FACILITY_LASTLOG) || !strcmp(val, FACILITY_UTMPX)) eval &= ~EVAL_SEND_TRACE;

		/* don't send LOG_INSTALL messages to Activity Tracing */
		if (!strcmp(val, asl_syslog_faciliy_num_to_name(LOG_INSTALL))) eval &= ~EVAL_SEND_TRACE;
	}

	return eval;
}

/*
 * _asl_lib_vlog_text
 * Internal routine used by asl_vlog.
 * msg:  an asl messsage
 * eval: log level and send flags for the message
 * format: A formating string
 * ap: va_list for the format
 * returns 0 for success, non-zero for failure
 */

__private_extern__ ASL_STATUS
_asl_lib_vlog_text(asl_object_t obj, uint32_t eval, asl_object_t msg, const char *format, va_list ap)
{
    int saved_errno = errno;
    int status;
    char *str, *fmt, estr[NL_TEXTMAX];
    uint32_t i, len, elen, expand;

    if (format == NULL) return ASL_STATUS_INVALID_ARG;

    /* insert strerror for %m */
    len = 0;
    elen = 0;

    expand = 0;
    for (i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            if (format[i+1] == '\0') len++;
            else if (format[i+1] == 'm')
            {
                expand = 1;
                strerror_r(saved_errno, estr, sizeof(estr));
                elen = strlen(estr);
                len += elen;
                i++;
            }
            else
            {
                len += 2;
                i++;
            }
        }
        else len++;
    }

    fmt = (char *)format;

    if (expand != 0)
    {
        fmt = malloc(len + 1);
        if (fmt == NULL) return ASL_STATUS_NO_MEMORY;

        len = 0;

        for (i = 0; format[i] != '\0'; i++)
        {
            if (format[i] == '%')
            {
                if (format[i+1] == '\0')
                {
                }
                else if ((format[i+1] == 'm') && (elen != 0))
                {
                    memcpy(fmt+len, estr, elen);
                    len += elen;
                    i++;
                }
                else
                {
                    fmt[len++] = format[i++];
                    fmt[len++] = format[i];
                }
            }
            else fmt[len++] = format[i];
        }
        
        fmt[len] = '\0';
    }
    
    str = NULL;
    vasprintf(&str, fmt, ap);
    if (expand != 0) free(fmt);
    
    if (str == NULL) return ASL_STATUS_NO_MEMORY;
    
    status = _asl_send_message_text(NULL, NULL, obj, eval, (asl_msg_t *)msg, str, true);
    free(str);
    
    return status;
}

/*
 * _asl_lib_vlog
 * Internal routine used by asl_vlog.
 * msg:  an asl messsage
 * eval: log level and send flags for the message
 * format: A formating string
 * ap: va_list for the format
 * returns 0 for success, non-zero for failure
 */
__private_extern__ ASL_STATUS
_asl_lib_vlog(asl_object_t obj, uint32_t eval, asl_object_t msg, const char *format, va_list ap)
{
	int saved_errno = errno;
	int status;
	char *str, *fmt, estr[NL_TEXTMAX];
	uint32_t i, len, elen, expand;

	if (format == NULL) return ASL_STATUS_INVALID_ARG;

	/* insert strerror for %m */
	len = 0;
	elen = 0;

	expand = 0;
	for (i = 0; format[i] != '\0'; i++)
	{
		if (format[i] == '%')
		{
			if (format[i+1] == '\0') len++;
			else if (format[i+1] == 'm')
			{
				expand = 1;
				strerror_r(saved_errno, estr, sizeof(estr));
				elen = strlen(estr);
				len += elen;
				i++;
			}
			else
			{
				len += 2;
				i++;
			}
		}
		else len++;
	}

	fmt = (char *)format;

	if (expand != 0)
	{
		fmt = malloc(len + 1);
		if (fmt == NULL) return ASL_STATUS_NO_MEMORY;

		len = 0;

		for (i = 0; format[i] != '\0'; i++)
		{
			if (format[i] == '%')
			{
				if (format[i+1] == '\0')
				{
				}
				else if ((format[i+1] == 'm') && (elen != 0))
				{
					memcpy(fmt+len, estr, elen);
					len += elen;
					i++;
				}
				else
				{
					fmt[len++] = format[i++];
					fmt[len++] = format[i];
				}
			}
			else fmt[len++] = format[i];
		}

		fmt[len] = '\0';
	}

	str = NULL;
	vasprintf(&str, fmt, ap);
	if (expand != 0) free(fmt);

	if (str == NULL) return ASL_STATUS_NO_MEMORY;

	status = _asl_send_message(obj, eval, (asl_msg_t *)msg, str);
	free(str);

	return status;
}

/*
 * asl_vlog
 * Similar to asl_log, but take a va_list instead of a list of arguments.
 * msg:  an asl message
 * level: the log level of the associated message
 * format: A formating string
 * ap: va_list for the format
 * returns 0 for success, non-zero for failure
 */
int
asl_vlog(asl_object_t client, asl_object_t msg, int level, const char *format, va_list ap)
{
	ASL_STATUS status = ASL_STATUS_OK;
	uint32_t eval = _asl_evaluate_send(client, msg, level);
	void *addr = __builtin_return_address(0);

	if ((eval & EVAL_SEND_TRACE) && os_log_shim_enabled(addr))
	{
		va_list ap_copy;
		if (level < ASL_LEVEL_EMERG) level = ASL_LEVEL_EMERG;
		if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;
		os_log_type_t type = shim_asl_to_log_type[level];

		va_copy(ap_copy, ap);
		os_log_with_args(OS_LOG_DEFAULT, type, format, ap_copy, addr);
		va_end(ap_copy);

		if (eval & EVAL_TEXT_FILE)
		{
			status = _asl_lib_vlog_text(client, eval, msg, format, ap);
		}
	}
	else if (eval & EVAL_ASL)
	{
		status = _asl_lib_vlog(client, eval, msg, format, ap);
	}

	return (status == ASL_STATUS_OK) ? 0 : -1;
}

/*
 * _asl_lib_log
 * SPI used by legacy (non-shim) ASL_PREFILTER_LOG. Converts format arguments to a va_list and
 * forwards the call to _asl_lib_vlog.
 * msg:  an asl message
 * eval: log level and send flags for the message
 * format: A formating string
 * ... args for format
 * returns 0 for success, non-zero for failure
 */
int
_asl_lib_log(asl_object_t client, uint32_t eval, asl_object_t msg, const char *format, ...)
{
	int status = 0;

	if (eval & EVAL_ASL)
	{
		va_list ap;

		va_start(ap, format);
		status = _asl_lib_vlog(client, eval, msg, format, ap);
		va_end(ap);
	}

	return status;
}

/*
 * asl_log
 * Processes an ASL log message.
 * msg:  an asl message
 * level: the log level of the associated message
 * format: A formating string
 * ... args for format
 * returns 0 for success, non-zero for failure
 */
int
asl_log(asl_object_t client, asl_object_t msg, int level, const char *format, ...)
{
	ASL_STATUS status = ASL_STATUS_OK;
	uint32_t eval = _asl_evaluate_send(client, msg, level);
	void *addr = __builtin_return_address(0);

	if ((eval & EVAL_SEND_TRACE) && os_log_shim_enabled(addr))
	{
		va_list ap;
		if (level < ASL_LEVEL_EMERG) level = ASL_LEVEL_EMERG;
		if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;
		os_log_type_t type = shim_asl_to_log_type[level];

		va_start(ap, format);
		os_log_with_args(OS_LOG_DEFAULT, type, format, ap, addr);
		va_end(ap);

		if (eval & EVAL_TEXT_FILE)
		{
			va_list ap;
			va_start(ap, format);
			status = _asl_lib_vlog_text(client, eval, msg, format, ap);
			va_end(ap);
		}
	}
	else if (eval & EVAL_ASL)
	{
		va_list ap;
		va_start(ap, format);
		status = _asl_lib_vlog(client, eval, msg, format, ap);
		va_end(ap);
	}

	return (status == ASL_STATUS_OK) ? 0 : -1;
}

/*
 * asl_log_message
 * Like asl_log, supplies NULL client and msg.
 * level: the log level of the associated message
 * format: A formating string
 * ... args for format
 * returns 0 for success, non-zero for failure
 */
int
asl_log_message(int level, const char *format, ...)
{
	ASL_STATUS status = ASL_STATUS_OK;
	uint32_t eval = _asl_evaluate_send(NULL, NULL, level);
	void *addr = __builtin_return_address(0);

	if ((eval & EVAL_SEND_TRACE) && os_log_shim_enabled(addr))
	{
		va_list ap;
		if (level < ASL_LEVEL_EMERG) level = ASL_LEVEL_EMERG;
		if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;
		os_log_type_t type = shim_asl_to_log_type[level];

		va_start(ap, format);
		os_log_with_args(OS_LOG_DEFAULT, type, format, ap, addr);
		va_end(ap);

		if (eval & EVAL_TEXT_FILE)
		{
			va_list ap;
			va_start(ap, format);
			status = _asl_lib_vlog_text(NULL, eval, NULL, format, ap);
			va_end(ap);
		}
	}
	else if (eval & EVAL_ASL)
	{
		va_list ap;
		va_start(ap, format);
		status = _asl_lib_vlog(NULL, eval, NULL, format, ap);
		va_end(ap);
	}

	return (status == ASL_STATUS_OK) ? 0 : -1;
}

/*
 * asl_get_filter: gets the values for the local, master, and remote filters, 
 * and indicates which one is active.
 */
int
asl_get_filter(asl_object_t client, int *local, int *master, int *remote, int *active)
{
	asl_client_t *asl, *asl_default;
	int l, m, r, x;
	int status, check;
	uint64_t v64;

	if ((client != NULL) && (asl_get_type(client) != ASL_TYPE_CLIENT)) return -1;

	l = 0;
	m = 0;
	r = 0;
	x = 0;

	asl_default = _asl_open_default();

	asl = (asl_client_t *)client;
	if (asl == NULL) asl = asl_default;
	if (asl != NULL) l = asl->filter & EVAL_LEVEL_MASK;

	if ((asl_default != NULL) && (!(asl_default->options & ASL_OPT_NO_REMOTE)))
	{
		pthread_mutex_lock(&_asl_global.lock);

		if (_asl_global.rc_change_token >= 0)
		{
			/* initialize or re-check process-specific and master filters  */
			check = 0;
			status = notify_check(_asl_global.rc_change_token, &check);
			if ((status == NOTIFY_STATUS_OK) && (check != 0))
			{
				if (_asl_global.master_token >= 0)
				{
					v64 = 0;
					status = notify_get_state(_asl_global.master_token, &v64);
					if (status == NOTIFY_STATUS_OK) _asl_global.master_filter = v64;
				}

				if (_asl_global.notify_token >= 0)
				{
					v64 = 0;
					status = notify_get_state(_asl_global.notify_token, &v64);
					if (status == NOTIFY_STATUS_OK) _asl_global.proc_filter = v64;
				}
			}
		}

		m = _asl_global.master_filter;
		if (m != 0) x = 1;

		r = _asl_global.proc_filter;
		if (r != 0) x = 2;

		pthread_mutex_unlock(&_asl_global.lock);
	}

	if (local != NULL) *local = l;
	if (master != NULL) *master = m;
	if (remote != NULL) *remote = r;
	if (active != NULL) *active = x;

	return 0;
}

/* SPI for SHIM control */
uint32_t
asl_set_local_control(asl_object_t client, uint32_t filter)
{
	asl_client_t *asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return UINT32_MAX;
	}
	else if (asl_get_type(client) != ASL_TYPE_CLIENT) return UINT32_MAX;

	return asl_client_set_control(asl, filter);
}

uint32_t
asl_get_local_control(asl_object_t client)
{
	asl_client_t *asl = (asl_client_t *)client;
	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return UINT32_MAX;
	}
	else if (asl_get_type(client) != ASL_TYPE_CLIENT) return UINT32_MAX;

	return asl_client_get_control(asl);
}

/*
 * Sets PID and OSActivityID values in a new message.
 * Also sets Level, Time, TimeNanoSec, Sender, Facility and Message if provided.
 */
asl_msg_t *
asl_base_msg(asl_client_t *asl, uint32_t level, const struct timeval *tv, const char *sstr, const char *fstr, const char *mstr)
{
	char aux_val[64];
	asl_msg_t *aux;
	int status;
	os_activity_id_t osaid;

	aux = asl_msg_new(ASL_TYPE_MSG);
	if (aux == NULL) return NULL;

	/* Level */
	if (level <= 7) asl_msg_set_key_val(aux, ASL_KEY_LEVEL, level_to_number_string[level]);

	/* Time and TimeNanoSec */
	if (tv != NULL)
	{
		snprintf(aux_val, sizeof(aux_val), "%llu", (unsigned long long) tv->tv_sec);
		asl_msg_set_key_val(aux, ASL_KEY_TIME, aux_val);

		snprintf(aux_val, sizeof(aux_val), "%d", tv->tv_usec * 1000);
		asl_msg_set_key_val(aux, ASL_KEY_TIME_NSEC, aux_val);
	}

	/* Message */
	if (mstr != NULL) asl_msg_set_key_val(aux, ASL_KEY_MSG, mstr);

	/* PID */
	snprintf(aux_val, sizeof(aux_val), "%u", getpid());
	asl_msg_set_key_val(aux, ASL_KEY_PID, aux_val);

	/* OSActivityID */
	osaid = os_activity_get_identifier(OS_ACTIVITY_CURRENT, NULL);
	if (osaid)
	{
		snprintf(aux_val, sizeof(aux_val), "0x%016llx", osaid);
		asl_msg_set_key_val(aux, ASL_KEY_OS_ACTIVITY_ID, aux_val);
	}

	/* Sender */
	if ((sstr == NULL) && (asl != NULL))
	{
		/* See if the client has a value for ASL_KEY_SENDER */
		status = asl_msg_lookup((asl_msg_t *)asl->kvdict, ASL_KEY_SENDER, &sstr, NULL);
		if ((status != 0) || (sstr == NULL))
		{
			sstr = NULL;

			/* See if the global cache has a value for ASL_KEY_SENDER */
			if (_asl_global.sender == NULL)
			{
				/* Get the process name with _NSGetArgv */
				char *name = *(*_NSGetArgv());
				if (name != NULL)
				{
					char *x = strrchr(name, '/');
					if (x != NULL) x++;
					else x = name;

					/* Set the cache value */
					pthread_mutex_lock(&_asl_global.lock);
					if (_asl_global.sender == NULL) _asl_global.sender = strdup(x);
					pthread_mutex_unlock(&_asl_global.lock);
				}
			}

			if (_asl_global.sender != NULL) asl_msg_set_key_val(aux, ASL_KEY_SENDER, _asl_global.sender);
			else asl_msg_set_key_val(aux, ASL_KEY_SENDER, "Unknown");
		}
	}

	if (sstr != NULL) asl_msg_set_key_val(aux, ASL_KEY_SENDER, sstr);

	/* Facility */
	if ((fstr == NULL) && (asl != NULL))
	{
		status = asl_msg_lookup((asl_msg_t *)asl->kvdict, ASL_KEY_FACILITY, &fstr, NULL);
		if (status != 0) fstr = NULL;
	}

	if (fstr != NULL) asl_msg_set_key_val(aux, ASL_KEY_FACILITY, fstr);

	return aux;
}

#ifdef NOTDEF
/*
 * Possibly useful someday...
 */
asl_msg_t *
asl_prepared_message(asl_client_t *asl, asl_msg_t *msg)
{
	uint32_t i, len, level, outstatus;
	const char *val, *sstr, *fstr;
	struct timeval tval = {0, 0};
	int status;
	asl_msg_t *out;

	if (asl == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return NULL;
	}

	status = gettimeofday(&tval, NULL);
	if (status != 0)
	{
		time_t tick = time(NULL);
		tval.tv_sec = tick;
		tval.tv_usec = 0;
	}

	val = NULL;
	status = asl_msg_lookup(msg, ASL_KEY_LEVEL, &val, NULL);
	if (status != 0) val = NULL;

	level = ASL_LEVEL_DEBUG;
	if (val != NULL) level = atoi(val);
	if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;

	sstr = NULL;
	status = asl_msg_lookup(msg, ASL_KEY_SENDER, &sstr, NULL);
	if (status != 0) sstr = NULL;

	fstr = NULL;
	status = asl_msg_lookup(msg, ASL_KEY_FACILITY, &fstr, NULL);
	if (status != 0) fstr = NULL;

	out = asl_base_msg(asl, level, &tval, sstr, fstr, NULL);
	out = asl_msg_merge(out, msg);

	return out;
}
#endif

static void
_asl_set_option(asl_msg_t *msg, const char *opt)
{
	const char *val = NULL;
	uint32_t status;

	if (msg == NULL) return;
	if (opt == NULL) return;

	status = asl_msg_lookup(msg, ASL_KEY_OPTION, &val, NULL);
	if ((status != 0) || (val == NULL))
	{
		asl_msg_set_key_val(msg, ASL_KEY_OPTION, opt);
	}
	else
	{
		char *option = NULL;
		asprintf(&option, "%s %s", opt, val);
		asl_msg_set_key_val(msg, ASL_KEY_OPTION, option);
		free(option);
	}
}

static ASL_STATUS
_asl_send_message_text(asl_client_t *asl, asl_msg_t *sendmsg, asl_object_t obj, uint32_t eval, asl_msg_t *msg, const char *mstr, bool shimmed)
{
    int status;
    uint32_t level, lmask;
    asl_msg_t *release_sendmsg = NULL;

    if (asl == NULL) {
        if (obj == NULL) {
            asl = _asl_open_default();
            if (asl == NULL) return ASL_STATUS_FAILED;
         } else {
            uint32_t objtype = asl_get_type(obj);
            if (objtype == ASL_TYPE_CLIENT) asl = (asl_client_t *)obj;
        }
    }

    level = eval & EVAL_LEVEL_MASK;
    if (level > 7) level = 7;
    lmask = ASL_FILTER_MASK(level);

    if (sendmsg == NULL) {
        struct timeval tval = {0, 0};
        const char *sstr, *fstr;

        status = gettimeofday(&tval, NULL);
        if (status != 0) {
            time_t tick = time(NULL);
            tval.tv_sec = tick;
            tval.tv_usec = 0;
        }

        sstr = NULL;
        status = asl_msg_lookup(msg, ASL_KEY_SENDER, &sstr, NULL);
        if (status != 0) {
            sstr = NULL;
        }

        fstr = NULL;
        status = asl_msg_lookup(msg, ASL_KEY_FACILITY, &fstr, NULL);
        if (status != 0) {
            fstr = NULL;
        }
        sendmsg = asl_base_msg(asl, level, &tval, sstr, fstr, mstr);
        if (sendmsg == NULL) {
            return ASL_STATUS_FAILED;
        }

        release_sendmsg = sendmsg = asl_msg_merge(sendmsg, msg);
    }

    /* write to file descriptors */
    for (uint32_t i = 0; i < asl->out_count; i++) {
        if (shimmed) {
            if ((asl->out_list[i].fd != STDOUT_FILENO) && (asl->out_list[i].fd != STDERR_FILENO)) {
                continue; // new logging only support stdout/stderr
            }
        }

        if ((asl->out_list[i].fd >= 0) && (asl->out_list[i].filter != 0) && ((asl->out_list[i].filter & lmask) != 0)) {
            char *str;

            uint32_t len = 0;
            str = asl_format_message(sendmsg, asl->out_list[i].mfmt, asl->out_list[i].tfmt, asl->out_list[i].encoding, &len);
            if (str == NULL) continue;

            status = write(asl->out_list[i].fd, str, len - 1);
            if (status < 0)
            {
                /* soft error for fd 2 (stderr) */
                if (asl->out_list[i].fd == 2) status = 0;
                asl->out_list[i].fd = -1;
            } else {
                status = 0;
            }
            
            free(str);
        }
    }
    if (release_sendmsg) {
        asl_msg_release(release_sendmsg);
    }

    return status;
}

static ASL_STATUS
_asl_send_message(asl_object_t obj, uint32_t eval, asl_msg_t *msg, const char *mstr)
{
	uint32_t len, level, lmask, outstatus, objtype;
	const char *sstr, *fstr;
	struct timeval tval = {0, 0};
	int status;
	int use_global_lock = 0;
	kern_return_t kstatus;
	asl_msg_t *sendmsg;
	asl_msg_t *qd_msg = NULL;
	asl_client_t *asl = NULL;
	static dispatch_once_t noquota_once;
	__block int log_quota_msg = 0;

	if ((eval & EVAL_ASL) == 0) return ASL_STATUS_OK;

	if (obj == NULL)
	{
		asl = _asl_open_default();
		if (asl == NULL) return ASL_STATUS_FAILED;
		use_global_lock = 1;
		objtype = ASL_TYPE_CLIENT;
	}
	else
	{
		objtype = asl_get_type(obj);
		if (objtype == ASL_TYPE_CLIENT) asl = (asl_client_t *)obj;
	}

	level = eval & EVAL_LEVEL_MASK;
	if (level > 7) level = 7;
	lmask = ASL_FILTER_MASK(level);

	if ((objtype == ASL_TYPE_CLIENT) && (asl->aslfile != NULL)) use_global_lock = 1;

	status = gettimeofday(&tval, NULL);
	if (status != 0)
	{
		time_t tick = time(NULL);
		tval.tv_sec = tick;
		tval.tv_usec = 0;
	}

	sstr = NULL;
	status = asl_msg_lookup(msg, ASL_KEY_SENDER, &sstr, NULL);
	if (status != 0) sstr = NULL;

	fstr = NULL;
	status = asl_msg_lookup(msg, ASL_KEY_FACILITY, &fstr, NULL);
	if (status != 0) fstr = NULL;

	sendmsg = asl_base_msg(asl, level, &tval, sstr, fstr, mstr);
	if (sendmsg == NULL) return ASL_STATUS_FAILED;

	/* Set "ASLOption store" if tunneling */
	if (eval & EVAL_TUNNEL) _asl_set_option(sendmsg, ASL_OPT_STORE);

	outstatus = -1;

	if (use_global_lock != 0) pthread_mutex_lock(&_asl_global.lock);

	sendmsg = asl_msg_merge(sendmsg, msg);

	if (objtype != ASL_TYPE_CLIENT)
	{
		asl_append(obj, (asl_object_t)sendmsg);
		asl_msg_release(sendmsg);
		if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
		return ASL_STATUS_OK;
	}

	/*
	 * If there is an aslfile this is a stand-alone file client.
	 * Just save to the file.
	 */
	if (asl->aslfile != NULL)
	{
		outstatus = ASL_STATUS_FAILED;

		if (sendmsg != NULL)
		{
			outstatus = asl_file_save(asl->aslfile, sendmsg, &(asl->aslfileid));
			asl->aslfileid++;
		}

		asl_msg_release(sendmsg);

		if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);
		return outstatus;
	}

	outstatus = 0;

	/*
	 * ASL message quota
	 * Quotas are disabled if:
	 * - a remote control filter is in place (EVAL_TUNNEL)
	 * - Environment variable ASL_QUOTA_DISABLED == 1
	 * - /etc/asl/.noquota existed at the time that the process started
	 *
	 * We just check /etc/asl/.noquota once, since it would be
	 * expensive to stat() for every log message.
	 *
	 * We only check the Environment variable once, since getenv() is
	 * not thread safe.  If someone is changing the environment,
	 * this can crash.
	 */

	dispatch_once(&noquota_once, ^{
		struct stat sb;
		memset(&sb, 0, sizeof(struct stat));

		int save_errno = errno;

		if (stat(NOQUOTA_FILE_PATH, &sb) == 0)
		{
			_asl_global.quota = UINT32_MAX;
		}
		else
		{
			const char *qtest = getenv(NOQUOTA_ENV);
			if ((qtest != NULL) && (!strcmp(qtest, "1")))
			{
				_asl_global.quota = UINT32_MAX;
				log_quota_msg = 1;
			}
		}

		/* reset errno since we want stat() to fail silently */
		errno = save_errno;
	});

	if (log_quota_msg != 0)
	{
		qd_msg = asl_base_msg(asl, QUOTA_LEVEL, &tval, sstr, fstr, QUOTA_DISABLED_MSG);
		asl_msg_set_key_val(qd_msg, ASL_KEY_OPTION, ASL_OPT_STORE);
	}

	if (((eval & EVAL_TUNNEL) == 0) && (_asl_global.quota != UINT32_MAX))
	{
		time_t last_send = _asl_global.last_send;
		time_t last_oq = _asl_global.last_oq_msg;
		uint32_t qcurr = _asl_global.quota;
		time_t delta;
		uint32_t qinc, qnew;

		qnew = qcurr;

		/* add QUOTA_MPS to quota for each second we've been idle */
		if (tval.tv_sec > last_send)
		{
			delta = tval.tv_sec - last_send;

			qinc = QUOTA_MPH;
			if (delta < (QUOTA_MPH / QUOTA_MPS)) qinc = delta * QUOTA_MPS;

			qnew = MIN(QUOTA_MPH, qcurr + qinc);
			OSAtomicCompareAndSwapLongBarrier(last_send, tval.tv_sec, (long *)&_asl_global.last_send);
		}

		if (qnew == 0)
		{
			if ((tval.tv_sec - last_oq) > QUOTA_MSG_INTERVAL)
			{
				eval |= EVAL_QUOTA;
				OSAtomicCompareAndSwapLongBarrier(last_oq, tval.tv_sec, (long *)&_asl_global.last_oq_msg);
			}
			else
			{
				eval &= ~EVAL_SEND_ASL;
			}
		}
		else
		{
			OSAtomicCompareAndSwap32Barrier(qcurr, qnew - 1, (int32_t *)&_asl_global.quota);
		}
	}

#if TARGET_OS_OSX
	if (eval & EVAL_MT_SHIM)
	{
		_asl_mt_shim_send_message(sendmsg);
	}
#endif

	_asl_global_init();
	if ((_asl_global.server_port != MACH_PORT_NULL) && (eval & EVAL_SEND_ASL))
	{
		asl_string_t *send_str;
		const char *str;
		size_t vmsize;

		if (eval & EVAL_QUOTA)
		{
			asl_msg_set_key_val(sendmsg, ASL_KEY_LEVEL, QUOTA_LEVEL_STR);
			asl_msg_set_key_val(sendmsg, ASL_KEY_MSG, QUOTA_MSG);
		}

		if (qd_msg != NULL)
		{
			send_str = asl_msg_to_string_raw(ASL_STRING_MIG, qd_msg, "raw");
			len = asl_string_length(send_str);
			vmsize = asl_string_allocated_size(send_str);
			str = asl_string_release_return_bytes(send_str);
			if (len != 0)
			{
				kstatus = _asl_server_message(_asl_global.server_port, (caddr_t)str, len);
				if (kstatus != KERN_SUCCESS)
				{
					vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
				}
			}
			else if (vmsize != 0)
			{
				vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
			}
			asl_msg_release(qd_msg);
		}

		send_str = asl_msg_to_string_raw(ASL_STRING_MIG, sendmsg, "raw");
		len = asl_string_length(send_str);

		if (len > LIBASL_MAX_MSG_SIZE)
		{
			char tmp[256];

			snprintf(tmp, sizeof(tmp), SIZE_LIMIT_MSG, len, LIBASL_MAX_MSG_SIZE);
			asl_msg_t *limitmsg = asl_base_msg(asl, ASL_LEVEL_CRIT, &tval, sstr, fstr, tmp);

			asl_string_release(send_str);
			len = 0;

			if (limitmsg != NULL)
			{
				/* Set "ASLOption store" if tunneling */
				if (eval & EVAL_TUNNEL) _asl_set_option(limitmsg, ASL_OPT_STORE);

				send_str = asl_msg_to_string_raw(ASL_STRING_MIG, limitmsg, "raw");
				len = asl_string_length(send_str);
				asl_msg_release(limitmsg);
			}
		}

		vmsize = asl_string_allocated_size(send_str);
		str = asl_string_release_return_bytes(send_str);

		if (len != 0)
		{
			/* send a mach message to syslogd */
			kstatus = _asl_server_message(_asl_global.server_port, (caddr_t)str, len);
			if (kstatus != KERN_SUCCESS)
			{
				vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
				outstatus = -1;
			}
		}
		else if (vmsize >0) vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
	}

	if ((sendmsg != NULL) && (asl->out_count > 0))
	{
		status = _asl_send_message_text(asl, sendmsg, obj, eval, msg, mstr, false);
	}

	asl_msg_release(sendmsg);

	if (use_global_lock != 0) pthread_mutex_unlock(&_asl_global.lock);

	return outstatus;
}

static void
os_log_with_args_wrapper(void *addr, int level, const char *format, ...)
{
	va_list ap;
	if (level < ASL_LEVEL_EMERG) level = ASL_LEVEL_EMERG;
	if (level > ASL_LEVEL_DEBUG) level = ASL_LEVEL_DEBUG;
	os_log_type_t type = shim_asl_to_log_type[level];

	va_start(ap, format);
	os_log_with_args(OS_LOG_DEFAULT, type, format, ap, addr);
	va_end(ap);
}

/*
 * asl_send: send a message 
 * This routine may be used instead of asl_log() or asl_vlog() if asl_set()
 * has been used to set all of a message's attributes.
 * eval:  hints about what to do with the message
 * msg:  an asl message
 * returns 0 for success, non-zero for failure
 */
__private_extern__ ASL_STATUS
asl_client_internal_send(asl_object_t obj, asl_object_t msg, void *addr)
{
	int status = ASL_STATUS_OK;
	uint32_t eval = _asl_evaluate_send(obj, msg, -1);

	const char *message = asl_msg_get_val_for_key((asl_msg_t *)msg, ASL_KEY_MSG);

	if ((eval & EVAL_SEND_TRACE) && message && message[0] && os_log_shim_enabled(addr))
	{
		int level = ASL_LEVEL_DEBUG;
		const char *lval = asl_msg_get_val_for_key((asl_msg_t *)msg, ASL_KEY_LEVEL);
		if (lval != NULL) level = atoi(lval);

		/*
		 * If the return address and the format string are from different
		 * binaries, os_log_with_args will not record the return address.
		 * Work around this by passing a dynamic format string.
		 */
		char dynamic_format[] = "%s";
		os_log_with_args_wrapper(addr, level, dynamic_format, message);

		if (eval & EVAL_TEXT_FILE)
		{
			status = _asl_send_message_text(NULL, NULL, obj, eval, (asl_msg_t *)msg, NULL, true);
		}
	}
	else if (eval & EVAL_ASL)
	{
		status = _asl_send_message(obj, eval, (asl_msg_t *)msg, NULL);
	}

	return status;
}

#pragma mark -
#pragma mark auxiliary files and URLs

static ASL_STATUS
_asl_aux_save_context(asl_aux_context_t *ctx)
{
	if (ctx == NULL) return ASL_STATUS_FAILED;

	pthread_mutex_lock(&_asl_global.lock);

	_asl_global.aux_ctx = (asl_aux_context_t **)reallocf(_asl_global.aux_ctx, (_asl_global.aux_count + 1) * sizeof(asl_aux_context_t *));
	if (_asl_global.aux_ctx == NULL)
	{
		_asl_global.aux_count = 0;
		pthread_mutex_unlock(&_asl_global.lock);
		return ASL_STATUS_FAILED;
	}

	_asl_global.aux_ctx[_asl_global.aux_count++] = ctx;

	pthread_mutex_unlock(&_asl_global.lock);

	return ASL_STATUS_OK;
}

/*
 * Creates an auxiliary file that may be used to save arbitrary data.  The ASL message msg
 * will be saved at the time that the auxiliary file is created.  The message will include
 * any keys and values found in msg, and it will include the title and Uniform Type
 * Identifier specified.  Output parameter out_fd will contain the file descriptor of the
 * new auxiliary file.
 */
static ASL_STATUS
_asl_auxiliary(asl_msg_t *msg, const char *title, const char *uti, const char *url, int *out_fd)
{
	asl_msg_t *aux;
	asl_string_t *send_str;
	const char *str;
	fileport_t fileport;
	kern_return_t kstatus;
	size_t len, vmsize;
	uint32_t newurllen, where;
	int status, fd, fdpair[2];
	caddr_t newurl;
	dispatch_queue_t pipe_q;
	dispatch_io_t pipe_channel;
	dispatch_semaphore_t sem;

	aux = asl_msg_new(ASL_TYPE_MSG);

	if (url != NULL) asl_msg_set_key_val(aux, ASL_KEY_AUX_URL, url);
	if (title != NULL) asl_msg_set_key_val(aux, ASL_KEY_AUX_TITLE, title);
	if (uti == NULL) asl_msg_set_key_val(aux, ASL_KEY_AUX_UTI, "public.data");
	else asl_msg_set_key_val(aux, ASL_KEY_AUX_UTI, uti);

	aux = asl_msg_merge(aux, msg);

	/* if (out_fd == NULL), this is from asl_log_auxiliary_location */
	if (out_fd == NULL)
	{
		uint32_t eval = _asl_evaluate_send(NULL, (asl_object_t)aux, -1);
		status = _asl_send_message(NULL, eval, aux, NULL);
		asl_msg_release(aux);
		return status;
	}

	where = asl_store_location();
	if (where == ASL_STORE_LOCATION_MEMORY)
	{
		/* create a pipe */
		asl_aux_context_t *ctx = (asl_aux_context_t *)calloc(1, sizeof(asl_aux_context_t));
		if (ctx == NULL) return ASL_STATUS_FAILED;

		status = pipe(fdpair);
		if (status < 0)
		{
			free(ctx);
			return ASL_STATUS_FAILED;
		}

		/* give read end to dispatch_io_read */
		fd = fdpair[0];
		sem = dispatch_semaphore_create(0);
		ctx->sem = sem;
		ctx->fd = fdpair[1];

		status = _asl_aux_save_context(ctx);
		if (status != ASL_STATUS_OK)
		{
			close(fdpair[0]);
			close(fdpair[1]);
			dispatch_release(sem);
			free(ctx);
			return ASL_STATUS_FAILED;
		}

		pipe_q = dispatch_queue_create("ASL_AUX_PIPE_Q", NULL);
		pipe_channel = dispatch_io_create(DISPATCH_IO_STREAM, fd, pipe_q, ^(int err){
			close(fd);
		});

		*out_fd = fdpair[1];

		dispatch_io_set_low_water(pipe_channel, SIZE_MAX);

		dispatch_io_read(pipe_channel, 0, SIZE_MAX, pipe_q, ^(bool done, dispatch_data_t pipedata, int err){
			if (err == 0)
			{
				size_t len = dispatch_data_get_size(pipedata);
				if (len > 0)
				{
					const char *bytes = NULL;
					char *encoded;
					uint32_t eval;

					dispatch_data_t md = dispatch_data_create_map(pipedata, (const void **)&bytes, &len);
					encoded = asl_core_encode_buffer(bytes, len);
					asl_msg_set_key_val(aux, ASL_KEY_AUX_DATA, encoded);
					free(encoded);
					eval = _asl_evaluate_send(NULL, (asl_object_t)aux, -1);
					_asl_send_message(NULL, eval, aux, NULL);
					asl_msg_release(aux);
					dispatch_release(md);
				}
			}

			if (done)
			{
				dispatch_semaphore_signal(sem);
				dispatch_release(pipe_channel);
				dispatch_release(pipe_q);
			}
		});

		return ASL_STATUS_OK;
	}

	_asl_global_init();
	if (_asl_global.server_port == MACH_PORT_NULL) return ASL_STATUS_FAILED;

	send_str = asl_msg_to_string_raw(ASL_STRING_MIG, aux, "raw");
	len = asl_string_length(send_str);
	vmsize = asl_string_allocated_size(send_str);
	str = asl_string_release_return_bytes(send_str);

	if (len == 0) 
	{
		asl_msg_release(aux);
		vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
		return ASL_STATUS_FAILED;
	}

	status = 0;
	fileport = MACH_PORT_NULL;
	status = KERN_SUCCESS;

	kstatus = _asl_server_create_aux_link(_asl_global.server_port, (caddr_t)str, len, &fileport, &newurl, &newurllen, &status);
	if (kstatus != KERN_SUCCESS)
	{
		vm_deallocate(mach_task_self(), (vm_address_t)str, vmsize);
		asl_msg_release(aux);
		return ASL_STATUS_FAILED;
	}

	if (status != 0)
	{
		asl_msg_release(aux);
		return status;
	}

	if (newurl != NULL)
	{
		asl_msg_set_key_val(aux, ASL_KEY_AUX_URL, newurl);
		vm_deallocate(mach_task_self(), (vm_address_t)newurl, newurllen);
	}

	if (fileport == MACH_PORT_NULL)
	{
		asl_msg_release(aux);
		return ASL_STATUS_FAILED;
	}

	fd = fileport_makefd(fileport);
	mach_port_deallocate(mach_task_self(), fileport);
	if (fd < 0)
	{
		asl_msg_release(aux);
		status = -1;
	}
	else
	{
		asl_aux_context_t *ctx = (asl_aux_context_t *)calloc(1, sizeof(asl_aux_context_t));
		if (ctx == NULL)
		{
			status = -1;
		}
		else
		{
			*out_fd = fd;

			ctx->fd = fd;
			ctx->msg = aux;

			status = _asl_aux_save_context(ctx);
		}
	}

	return status;
}

int
asl_create_auxiliary_file(asl_object_t msg, const char *title, const char *uti, int *out_fd)
{
	if (out_fd == NULL) return -1;

	ASL_STATUS status = _asl_auxiliary((asl_msg_t *)msg, title, uti, NULL, out_fd);
	return (status == ASL_STATUS_OK) ? 0 : -1;
}

int
asl_log_auxiliary_location(asl_object_t msg, const char *title, const char *uti, const char *url)
{
	ASL_STATUS status = _asl_auxiliary((asl_msg_t *)msg, title, uti, url, NULL);
	return (status == ASL_STATUS_OK) ? 0 : -1;
}

/*
 * Close an auxiliary file.
 * Sends the cached auxiliary message to syslogd.
 * Returns 0 on success, -1 on error.
 */
int
asl_close_auxiliary_file(int fd)
{
	int i, j, status;
	asl_msg_t *aux_msg;
	dispatch_semaphore_t aux_sem = NULL;

	pthread_mutex_lock(&(_asl_global.lock));

	aux_msg = NULL;
	status = -1;

	for (i = 0; i < _asl_global.aux_count; i++)
	{
		if (_asl_global.aux_ctx[i]->fd == fd)
		{
			status = 0;

			aux_msg = _asl_global.aux_ctx[i]->msg;
			aux_sem = _asl_global.aux_ctx[i]->sem;

			free(_asl_global.aux_ctx[i]);

			for (j = i + 1; j < _asl_global.aux_count; i++, j++)
			{
				_asl_global.aux_ctx[i] = _asl_global.aux_ctx[j];
			}

			_asl_global.aux_count--;

			if (_asl_global.aux_count == 0)
			{
				free(_asl_global.aux_ctx);
				_asl_global.aux_ctx = NULL;
			}
			else
			{
				_asl_global.aux_ctx = (asl_aux_context_t **)reallocf(_asl_global.aux_ctx, _asl_global.aux_count * sizeof(asl_aux_context_t *));
				if (_asl_global.aux_ctx == NULL)
				{
					_asl_global.aux_count = 0;
					status = -1;
				}
			}

			break;
		}
	}

	pthread_mutex_unlock(&(_asl_global.lock));

	close(fd);

	if (aux_msg != NULL)
	{
		uint32_t eval = _asl_evaluate_send(NULL, (asl_object_t)aux_msg, -1);
		if (_asl_send_message(NULL, eval, aux_msg, NULL) != ASL_STATUS_OK) status = -1;
		asl_msg_release(aux_msg);
	}

	if (aux_sem != NULL)
	{
		dispatch_semaphore_wait(aux_sem, DISPATCH_TIME_FOREVER);
		dispatch_release(aux_sem);
	}

	return status;
}

#pragma mark -

asl_msg_t *
_asl_server_control_query(void)
{
	asl_msg_list_t *list = NULL;
	char *res;
	uint32_t len, reslen, status;
	uint64_t cmax, qmin;
	kern_return_t kstatus;
	caddr_t vmstr;
	asl_msg_t *m = NULL;
	static const char ctlstr[] = "1\nQ [= ASLOption control]\n";

	_asl_global_init();
	if (_asl_global.server_port == MACH_PORT_NULL) return NULL;

	len = strlen(ctlstr) + 1;

	qmin = 0;
	cmax = 0;
	res = NULL;
	reslen = 0;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmstr, len, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ASL));
	if (kstatus != KERN_SUCCESS) return NULL;

	memmove(vmstr, ctlstr, len);

	status = 0;
	kstatus = _asl_server_query_2(_asl_global.server_port, vmstr, len, qmin, FETCH_BATCH, 0, (caddr_t *)&res, &reslen, &cmax, (int *)&status);
	if (kstatus != KERN_SUCCESS) return NULL;

	list = asl_msg_list_from_string(res);
	vm_deallocate(mach_task_self(), (vm_address_t)res, reslen);

	if (list == NULL) return NULL;
	if (list->count > 0) m = asl_msg_retain(list->msg[0]);
	asl_msg_list_release(list);
	return m;
}

/*
 * Returns ASL_STORE_LOCATION_FILE or ASL_STORE_LOCATION_MEMORY
 */
int
asl_store_location()
{
	kern_return_t kstatus;
	char *res;
	uint32_t reslen, status;
	uint64_t cmax;

	_asl_global_init();
	if (_asl_global.server_port == MACH_PORT_NULL) return ASL_STORE_LOCATION_FILE;

	res = NULL;
	reslen = 0;
	cmax = 0;
	status = ASL_STATUS_OK;

	kstatus = _asl_server_query_2(_asl_global.server_port, NULL, 0, 0, -1, 0, (caddr_t *)&res, &reslen, &cmax, (int *)&status);
	if (kstatus != KERN_SUCCESS) return ASL_STORE_LOCATION_FILE;

	/* res should never be returned, but just to be certain we don't leak VM ... */
	if (res != NULL) vm_deallocate(mach_task_self(), (vm_address_t)res, reslen);

	if (status == ASL_STATUS_OK) return ASL_STORE_LOCATION_MEMORY;
	return ASL_STORE_LOCATION_FILE;
}

asl_object_t
asl_open_path(const char *path, uint32_t opts)
{
	struct stat sb;
	asl_file_t *fout = NULL;
	asl_store_t *sout = NULL;

	if (opts == 0) opts = ASL_OPT_OPEN_READ;

	if (opts & ASL_OPT_OPEN_READ)
	{
		if (path == NULL)
		{
			if (asl_store_open_read(ASL_PLACE_DATABASE_DEFAULT, &sout) != ASL_STATUS_OK) return NULL;
			return (asl_object_t)sout;
		}

		memset(&sb, 0, sizeof(struct stat));
		if (stat(path, &sb) < 0) return NULL;

		if (sb.st_mode & S_IFREG)
		{
			if (asl_file_open_read(path, &fout) != ASL_STATUS_OK) return NULL;
			return (asl_object_t)fout;
		}
		else if (sb.st_mode & S_IFDIR)
		{
			if (asl_store_open_read(path, &sout) != ASL_STATUS_OK) return NULL;
			return (asl_object_t)sout;
		}

		return NULL;
	}
	else if (opts & ASL_OPT_OPEN_WRITE)
	{
		if (path == NULL) return NULL;

		memset(&sb, 0, sizeof(struct stat));
		if (stat(path, &sb) < 0)
		{
			if (errno != ENOENT) return NULL;

			if (opts & ASL_OPT_CREATE_STORE)
			{
				if (asl_store_open_write(path, &sout) != ASL_STATUS_OK) return NULL;
				return (asl_object_t)sout;
			}
			else
			{
				if (asl_file_open_write(path, 0644, geteuid(), getegid(), &fout) != ASL_STATUS_OK) return NULL;
				return (asl_object_t)fout;
			}
		}
		else if (sb.st_mode & S_IFREG)
		{
			if (asl_file_open_write(path, 0644, geteuid(), getegid(), &fout) != ASL_STATUS_OK) return NULL;
			return (asl_object_t)fout;
		}
		else if (sb.st_mode & S_IFDIR)
		{
			if (asl_store_open_write(path, &sout) != ASL_STATUS_OK) return NULL;
			return (asl_object_t)sout;
		}
	}

	return NULL;
}
