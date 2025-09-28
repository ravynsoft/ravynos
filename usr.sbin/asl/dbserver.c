/*
 * Copyright (c) 2007-2012 Apple Inc. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/fileport.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <bsm/libbsm.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/event.h>
#include <servers/bootstrap.h>
#include <pthread.h>
#include <notify.h>
#include <sys/time.h>
#include <xpc/xpc.h>
//#include <xpc/private.h>
#include <libproc.h>
#include <uuid/uuid.h>
#include "daemon.h"
#include "asl_ipc.h"

#define forever for(;;)

#define LIST_SIZE_DELTA 256

#define SEND_NOTIFICATION 0xfadefade

#define QUERY_FLAG_SEARCH_REVERSE 0x00000001
#define QUERY_DURATION_UNLIMITED 0

#define SEARCH_FORWARD 1
#define SEARCH_BACKWARD -1

#define MAX_AGAIN 100

#define ASL_ENTITLEMENT_KEY "com.apple.asl.access_as_root"
#define ASL_ENTITLEMENT_UID_KEY "com.apple.asl.access_as_uid"
#define ASL_ENTITLEMENT_GID_KEY "com.apple.asl.access_as_gid"

static dispatch_queue_t asl_server_queue;
static dispatch_queue_t watch_queue;
static dispatch_once_t watch_init_once;

extern boolean_t asl_ipc_server(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

static task_name_t *client_tasks = NULL;
static uint32_t client_tasks_count = 0;

static int *direct_watch = NULL;
/* N.B. ports are in network byte order */
static uint16_t *direct_watch_port = NULL;
static uint32_t direct_watch_count = 0;

typedef union
{
	mach_msg_header_t head;
	union __RequestUnion__asl_ipc_subsystem request;
} asl_request_msg;

typedef union
{
	mach_msg_header_t head;
	union __ReplyUnion__asl_ipc_subsystem reply;
} asl_reply_msg;

static void
db_asl_open(uint32_t dbtype)
{
	uint32_t status;
	struct stat sb;

	if ((dbtype & DB_TYPE_FILE) && (global.file_db == NULL))
	{
		memset(&sb, 0, sizeof(struct stat));
		if (stat(PATH_ASL_STORE, &sb) == 0)
		{
			/* must be a directory */
			if (!S_ISDIR(sb.st_mode))
			{
				asldebug("error: %s is not a directory", PATH_ASL_STORE);
				return;
			}
		}
		else
		{
			if (errno == ENOENT)
			{
				/* /var/log/asl doesn't exist - create it */
				if (mkdir(PATH_ASL_STORE, 0755) != 0)
				{
					asldebug("error: can't create data store %s: %s\n", PATH_ASL_STORE, strerror(errno));
					return;
				}
			}
			else
			{
				/* stat failed for some other reason */
				asldebug("error: can't stat data store %s: %s\n", PATH_ASL_STORE, strerror(errno));
				return;
			}
		}

		status = asl_store_open_write(NULL, &(global.file_db));
		if (status != ASL_STATUS_OK)
		{
			asldebug("asl_store_open_write: %s\n", asl_core_error(status));
		}
		else
		{
			if (global.db_file_max != 0) asl_store_max_file_size(global.file_db, global.db_file_max);
			trigger_aslmanager();
		}
	}

	if ((dbtype & DB_TYPE_MEMORY) && (global.memory_db == NULL))
	{
		status = asl_memory_open(global.db_memory_max, global.db_memory_str_max, &(global.memory_db));
		if (status != ASL_STATUS_OK)
		{
			asldebug("asl_memory_open: %s\n", asl_core_error(status));
		}
	}
}

void
add_lockdown_session(int fd)
{
	dispatch_once(&watch_init_once, ^{
		watch_queue = dispatch_queue_create("Direct Watch Queue", NULL);
	});

	dispatch_async(watch_queue, ^{
		if (global.lockdown_session_count == 0) global.lockdown_session_fds = NULL;

		global.lockdown_session_fds = reallocf(global.lockdown_session_fds, global.lockdown_session_count + 1 * sizeof(int));

		if (global.lockdown_session_fds == NULL)
		{
			asldebug("add_lockdown_session: realloc failed\n");
			global.lockdown_session_count = 0;
		}
		else
		{
			global.lockdown_session_fds[global.lockdown_session_count++] = fd;
		}

		global.watchers_active = direct_watch_count + global.lockdown_session_count;
	});
}

void
remove_lockdown_session(int fd)
{
	dispatch_once(&watch_init_once, ^{
		watch_queue = dispatch_queue_create("Direct Watch Queue", NULL);
	});

	dispatch_async(watch_queue, ^{
		int i, n;

		for (i = 0, n = 0; i < global.lockdown_session_count; i++)
		{
			if (global.lockdown_session_fds[i] == fd)
			{
			}
			else
			{
				if (i != n) global.lockdown_session_fds[n] = global.lockdown_session_fds[i];
				n++;
			}
		}

		if (n == 0)
		{
			free(global.lockdown_session_fds);
			global.lockdown_session_fds = NULL;
			global.lockdown_session_count = 0;
		}
		else
		{
			global.lockdown_session_fds = reallocf(global.lockdown_session_fds, n * sizeof(int));
			if (global.lockdown_session_fds == NULL)
			{
				asldebug("remove_lockdown_session: realloc failed\n");
				global.lockdown_session_count = 0;
			}
			else
			{
				global.lockdown_session_count = n;
			}
		}

		global.watchers_active = direct_watch_count + global.lockdown_session_count;
	});
}

#ifdef LOCKDOWN
static void
sweep_lockdown_session_fds()
{
	int i, n;

	for (i = 0, n = 0; i < global.lockdown_session_count; i++)
	{
		if (global.lockdown_session_fds[i] >= 0)
		{
			if (i != n) global.lockdown_session_fds[n] = global.lockdown_session_fds[i];
 			n++;
		}
	}

	if (n == 0)
	{
		free(global.lockdown_session_fds);
		global.lockdown_session_fds = NULL;
		global.lockdown_session_count = 0;
	}
	else
	{
		global.lockdown_session_fds = reallocf(global.lockdown_session_fds, n * sizeof(int));
		if (global.lockdown_session_fds == NULL)
		{
			asldebug("sweep_lockdown_session_fds: realloc failed\n");
			global.lockdown_session_count = 0;
		}
		else
		{
			global.lockdown_session_count = n;
		}
	}

	global.watchers_active = direct_watch_count + global.lockdown_session_count;
}
#endif

static void
_internal_send_to_direct_watchers(asl_msg_t *msg)
{
	uint32_t i, j, nlen, outlen, cleanup, total_sent, again;
	ssize_t sent;
	char *out;

#ifdef LOCKDOWN
	static struct timeval last_time;

	cleanup = 0;

	if (global.lockdown_session_count > 0)
	{
		if (global.remote_delay_time > 0)
		{
			struct timeval now;
			uint64_t delta;

			if (gettimeofday(&now, NULL) == 0)
			{
				if (last_time.tv_sec != 0)
				{
					if (now.tv_sec > last_time.tv_sec)
					{
						now.tv_sec -= 1;
						now.tv_usec += 1000000;
					}

					delta = now.tv_sec - last_time.tv_sec;
					delta *= 1000000;
					delta += (now.tv_usec - last_time.tv_usec);
					if (delta < global.remote_delay_time)
					{
						usleep(delta);
					}
				}

				if (now.tv_usec >= 1000000)
				{
					now.tv_sec += 1;
					now.tv_usec -= 1000000;
				}

				last_time = now;
			}
		}

		out = asl_format_message(msg, ASL_MSG_FMT_STD, ASL_TIME_FMT_LCL, ASL_ENCODE_SAFE, &outlen);

		for (i = 0; i < global.lockdown_session_count; i++)
		{
			if (write(global.lockdown_session_fds[i], out, outlen) < 0)
			{
				asldebug("send_to_direct_watchers: lockdown %d write error: %d %s\n", global.lockdown_session_fds[i], errno, strerror(errno));
				close(global.lockdown_session_fds[i]);
				global.lockdown_session_fds[i] = -1;
				cleanup = 1;
			}
		}

		free(out);
	}

	if (cleanup != 0) sweep_lockdown_session_fds();
#endif

	if (direct_watch_count == 0)
	{
		direct_watch = NULL;
		return;
	}

	if (direct_watch == NULL)
	{
		direct_watch_count = 0;
		return;
	}

	cleanup = 0;
	out = asl_msg_to_string(msg, &outlen);

	if (out == NULL) return;

	nlen = htonl(outlen);
	for (i = 0; i < direct_watch_count; i++)
	{
		sent = send(direct_watch[i], &nlen, sizeof(nlen), 0);
		if (sent < sizeof(nlen))
		{
			/* bail out if we can't send 4 bytes */
			close(direct_watch[i]);
			direct_watch[i] = -1;
			cleanup = 1;
		}
		else
		{
			total_sent = 0;
			again = 0;

			while (total_sent < outlen)
			{
				errno = 0;
				sent = send(direct_watch[i], out + total_sent, outlen - total_sent, 0);
				if (sent <= 0)
				{
					asldebug("send_to_direct_watchers: send returned %d (errno %d)\n", sent, errno);
					if (errno == EAGAIN)
					{
						if (again > MAX_AGAIN)
						{
							asldebug("send_to_direct_watchers: exceeded EAGAIN limit - closing connection\n");
							break;
						}
						else
						{
							again++;
							errno = 0;
							continue;
						}
					}

					close(direct_watch[i]);
					direct_watch[i] = -1;
					cleanup = 1;
					break;
				}

				total_sent += sent;
			}
		}
	}

	free(out);

	if (cleanup == 0) return;

	j = 0;
	for (i = 0; i < direct_watch_count; i++)
	{
		if (direct_watch[i] >= 0)
		{
			if (j != i)
			{
				direct_watch[j] = direct_watch[i];
				direct_watch_port[j] = direct_watch_port[i];
				j++;
			}
		}
	}

	direct_watch_count = j;
	if (direct_watch_count == 0)
	{
		free(direct_watch);
		direct_watch = NULL;

		free(direct_watch_port);
		direct_watch_port = NULL;
	}
	else
	{
		direct_watch = reallocf(direct_watch, direct_watch_count * sizeof(int));
		direct_watch_port = reallocf(direct_watch_port, direct_watch_count * sizeof(uint16_t));
		if ((direct_watch == NULL) || (direct_watch_port == NULL))
		{
			free(direct_watch);
			direct_watch = NULL;

			free(direct_watch_port);
			direct_watch_port = NULL;

			direct_watch_count = 0;
		}
	}
}

void
send_to_direct_watchers(asl_msg_t *msg)
{
	dispatch_once(&watch_init_once, ^{
		watch_queue = dispatch_queue_create("Direct Watch Queue", NULL);
	});

	asl_msg_retain(msg);

	dispatch_async(watch_queue, ^{
		_internal_send_to_direct_watchers(msg);
		asl_msg_release(msg);
	});
}

/*
 * Called from asl_action.c to save messgaes to the ASL data store
 */
void
db_save_message(asl_msg_t *msg)
{
	uint64_t msgid;
	uint32_t status, dbtype;
	static int armed;
	static dispatch_source_t timer_src;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		timer_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
		dispatch_source_set_event_handler(timer_src, ^{
			// XXX notify_post(kNotifyASLDBUpdate);
			dispatch_suspend(timer_src);
			armed = 0;
		});
		armed = 0;
	});

	send_to_direct_watchers((asl_msg_t *)msg);

	dbtype = global.dbtype;

	if (asl_check_option(msg, ASL_OPT_DB_FILE))   dbtype |= DB_TYPE_FILE;
	if (asl_check_option(msg, ASL_OPT_DB_MEMORY)) dbtype |= DB_TYPE_MEMORY;

	db_asl_open(dbtype);

	if (dbtype & DB_TYPE_FILE)
	{
		status = asl_store_save(global.file_db, msg);
		if (status != ASL_STATUS_OK)
		{
			/* write failed - reopen & retry */
			asldebug("asl_store_save: %s\n", asl_core_error(status));
			asl_store_release(global.file_db);
			global.file_db = NULL;

			db_asl_open(dbtype);
			status = asl_store_save(global.file_db, msg);
			if (status != ASL_STATUS_OK)
			{
				asldebug("(retry) asl_store_save: %s\n", asl_core_error(status));
				asl_store_release(global.file_db);
				global.file_db = NULL;

				global.dbtype |= DB_TYPE_MEMORY;
				dbtype |= DB_TYPE_MEMORY;
				if (global.memory_db == NULL)
				{
					status = asl_memory_open(global.db_memory_max, global.db_memory_str_max, &(global.memory_db));
					if (status != ASL_STATUS_OK)
					{
						asldebug("asl_memory_open: %s\n", asl_core_error(status));
					}
				}
			}
		}
	}

	if (dbtype & DB_TYPE_MEMORY)
	{
		msgid = 0;
		status = asl_memory_save(global.memory_db, msg, &msgid);
		if (status != ASL_STATUS_OK)
		{
			/* save failed - reopen & retry*/
			asldebug("asl_memory_save: %s\n", asl_core_error(status));
			asl_memory_close(global.memory_db);
			global.memory_db = NULL;

			db_asl_open(dbtype);
			msgid = 0;
			status = asl_memory_save(global.memory_db, msg, &msgid);
			if (status != ASL_STATUS_OK)
			{
				asldebug("(retry) asl_memory_save: %s\n", asl_core_error(status));
				asl_memory_close(global.memory_db);
				global.memory_db = NULL;
			}
		}
	}

	if (armed == 0)
	{
		armed = 1;
		dispatch_source_set_timer(timer_src, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2), DISPATCH_TIME_FOREVER, 0);
		dispatch_resume(timer_src);
	}
}

void
disaster_message(asl_msg_t *msg)
{
	uint64_t msgid;
	uint32_t status;

	msgid = 0;

	if ((global.dbtype & DB_TYPE_MEMORY) == 0)
	{
		if (global.memory_db == NULL)
		{
			status = asl_memory_open(global.db_memory_max, global.db_memory_str_max, &(global.memory_db));
			if (status != ASL_STATUS_OK) asldebug("asl_memory_open: %s\n", asl_core_error(status));
			else asl_memory_save(global.memory_db, msg, &msgid);
		}
	}
}

/*
 * Do a database search.
 */
uint32_t
db_query(asl_msg_list_t *query, asl_msg_list_t **res, uint64_t startid, int count, uint32_t duration, int direction, uint64_t *lastid, int32_t ruid, int32_t rgid, int raccess)
{
	uint32_t status, ucount;
	uuid_string_t ustr;
	struct proc_uniqidentifierinfo pinfo;
	const char *str = NULL;

	/*
	 * Special case: if count is -1, we return ASL_STATUS_OK to indicate that the store is
	 * in memory, and ASL_STATUS_INVALID_STORE to indicate that the file store is in use.
	 */
	if (count == -1)
	{
		if (global.dbtype & DB_TYPE_FILE) return ASL_STATUS_INVALID_STORE;
		return ASL_STATUS_OK;
	}

	if (raccess != 0)
	{
		str = "NO ACCESS";
		uuid_clear(pinfo.p_uuid);
		if (proc_pidinfo(raccess, PROC_PIDUNIQIDENTIFIERINFO, 1, &pinfo, sizeof(pinfo)) == sizeof(pinfo))
		{
			uuid_unparse(pinfo.p_uuid, ustr);
			str = (const char *)ustr;
		}
	}

	ucount = count;

	status = ASL_STATUS_FAILED;

	if ((global.dbtype & DB_TYPE_MEMORY) || (global.disaster_occurred != 0))
	{
		status = asl_memory_match_restricted_uuid(global.memory_db, query, res, lastid, startid, ucount, duration, direction, ruid, rgid, str);
	}

	return status;
}

static void
register_session(task_name_t task_name, pid_t pid)
{
	mach_port_t previous;
	uint32_t i;

	if (task_name == MACH_PORT_NULL) return;

	if (global.dead_session_port == MACH_PORT_NULL)
	{
		mach_port_deallocate(mach_task_self(), task_name);
		return;
	}

	for (i = 0; i < client_tasks_count; i++) if (task_name == client_tasks[i])
	{
		mach_port_deallocate(mach_task_self(), task_name);
		return;
	}

	if (client_tasks_count == 0) client_tasks = (task_name_t *)calloc(1, sizeof(task_name_t));
	else client_tasks = (task_name_t *)reallocf(client_tasks, (client_tasks_count + 1) * sizeof(task_name_t));

	if (client_tasks == NULL)
	{
		mach_port_deallocate(mach_task_self(), task_name);
		return;
	}

	client_tasks[client_tasks_count] = task_name;
	client_tasks_count++;

	asldebug("register_session: %u   PID %d\n", (unsigned int)task_name, (int)pid);

	/* register for port death notification */
	mach_port_request_notification(mach_task_self(), task_name, MACH_NOTIFY_DEAD_NAME, 0, global.dead_session_port, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
	mach_port_deallocate(mach_task_self(), previous);

	asl_client_count_increment();
}

static void
cancel_session(task_name_t task_name)
{
	uint32_t i;

	for (i = 0; (i < client_tasks_count) && (task_name != client_tasks[i]); i++);

	if (i >= client_tasks_count) return;

	if (client_tasks_count == 1)
	{
		free(client_tasks);
		client_tasks = NULL;
		client_tasks_count = 0;
	}
	else
	{
		for (i++; i < client_tasks_count; i++) client_tasks[i-1] = client_tasks[i];
		client_tasks_count--;
		client_tasks = (task_name_t *)reallocf(client_tasks, client_tasks_count * sizeof(task_name_t));
	}

	asldebug("cancel_session: %u\n", (unsigned int)task_name);

	/* we hold a send right or dead name right for the task name port */
	mach_port_deallocate(mach_task_self(), task_name);
	asl_client_count_decrement();
}

static uint32_t
register_direct_watch(uint16_t port)
{
#if TARGET_OS_EMBEDDED
	uint32_t i;
	int sock, flags;
	struct sockaddr_in address;

	if (port == 0) return ASL_STATUS_FAILED;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) return ASL_STATUS_FAILED;

	address.sin_family = AF_INET;
	/* port must be sent in network byte order */
	address.sin_port = port;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) != 0) return ASL_STATUS_FAILED;

	i = 1;
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &i, sizeof(i));

	i = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &i, sizeof(i));

	/* make socket non-blocking */
	flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) flags = 0;
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	if (direct_watch_count == 0)
	{
		direct_watch = (int *)calloc(1, sizeof(int));
		direct_watch_port = (uint16_t *)calloc(1, sizeof(uint16_t));
	}
	else
	{
		direct_watch = (int *)reallocf(direct_watch, (direct_watch_count + 1) * sizeof(int));
		direct_watch_port = (uint16_t *)reallocf(direct_watch_port, (direct_watch_count + 1) * sizeof(uint16_t));
	}

	if ((direct_watch == NULL) || (direct_watch_port == NULL))
	{
		close(sock);

		free(direct_watch);
		direct_watch = NULL;

		free(direct_watch_port);
		direct_watch_port = NULL;

		direct_watch_count = 0;
		global.watchers_active = 0;
		if (global.lockdown_session_count > 0) global.watchers_active = 1;

		return ASL_STATUS_FAILED;
	}

	direct_watch[direct_watch_count] = sock;
	direct_watch_port[direct_watch_count] = port;
	direct_watch_count++;
	global.watchers_active = direct_watch_count + global.lockdown_session_count;

	return ASL_STATUS_OK;
#else
	return ASL_STATUS_FAILED;
#endif
}

static void
cancel_direct_watch(uint16_t port)
{
#if TARGET_OS_EMBEDDED
	uint32_t i;

	for (i = 0; (i < direct_watch_count) && (port != direct_watch_port[i]); i++);

	if (i >= direct_watch_count) return;

	if (direct_watch_count == 1)
	{
		free(direct_watch);
		direct_watch = NULL;

		free(direct_watch_port);
		direct_watch_port = NULL;

		direct_watch_count = 0;
		global.watchers_active = 0;
		if (global.lockdown_session_count > 0) global.watchers_active = 1;
	}
	else
	{
		for (i++; i < direct_watch_count; i++)
		{
			direct_watch[i-1] = direct_watch[i];
			direct_watch_port[i-1] = direct_watch_port[i];
		}

		direct_watch_count--;
		global.watchers_active = direct_watch_count + global.lockdown_session_count;

		direct_watch = (int *)reallocf(direct_watch, direct_watch_count * sizeof(int));
		direct_watch_port = (uint16_t *)reallocf(direct_watch_port, direct_watch_count * sizeof(uint16_t));

		if ((direct_watch == NULL) || (direct_watch_port == NULL))
		{
			free(direct_watch);
			direct_watch = NULL;

			free(direct_watch_port);
			direct_watch_port = NULL;

			direct_watch_count = 0;
			global.watchers_active = 0;
			if (global.lockdown_session_count > 0) global.watchers_active = 1;
		}
	}
#endif
}

static int
syslogd_state_query(asl_msg_t *q, asl_msg_list_t **res, uid_t uid)
{
	asl_msg_list_t *out;
	uint32_t i, n;
	bool all = false;
	asl_msg_t *m;
	char val[256];
	const char *mval;
	asl_out_module_t *om;

	if (res == NULL) return ASL_STATUS_INVALID_ARG;
	*res = NULL;

	out = asl_msg_list_new();
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	m = asl_msg_new(ASL_TYPE_MSG);
	if (m == NULL)
	{
		asl_msg_list_release(out);
		return ASL_STATUS_NO_MEMORY;
	}

	asl_msg_list_append(out, m);

	/* q must have [ASLOption control], so a "null" query really has count == 1 */
	if (asl_msg_count(q) == 1) all = true;

	if (all || (0 == asl_msg_lookup(q, "debug", NULL, NULL)))
	{
		if (global.debug == 0) snprintf(val, sizeof(val), "0");
		else snprintf(val, sizeof(val), "1 %s", global.debug_file);
		asl_msg_set_key_val(m, "debug", val);
	}

	if (all || (0 == asl_msg_lookup(q, "dbtype", NULL, NULL)))
	{
		n = 0;
		if (global.dbtype & DB_TYPE_FILE) n++;
		if (global.dbtype & DB_TYPE_MEMORY) n++;

		if (n == 0)
		{
			asl_msg_set_key_val(m, "dbtype", "unknown");
		}
		else
		{
			i = 0;
			memset(val, 0, sizeof(val));

			if (global.dbtype & DB_TYPE_FILE)
			{
				i++;
				strncat(val, "file", 4);
				if (i < n) strncat(val, " ", 1);
			}

			if (global.dbtype & DB_TYPE_MEMORY)
			{
				i++;
				strncat(val, "memory", 6);
				if (i < n) strncat(val, " ", 1);
			}

			asl_msg_set_key_val(m, "dbtype", val);
		}
	}

	if (all || (0 == asl_msg_lookup(q, "db_file_max", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%u", global.db_file_max);
		asl_msg_set_key_val(m, "db_file_max", val);
	}

	if (all || (0 == asl_msg_lookup(q, "db_memory_max", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%u", global.db_memory_max);
		asl_msg_set_key_val(m, "db_memory_max", val);
	}
	
	if (all || (0 == asl_msg_lookup(q, "db_memory_str_max", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%u", global.db_memory_str_max);
		asl_msg_set_key_val(m, "db_memory_str_max", val);
	}
	
	if (all || (0 == asl_msg_lookup(q, "mps_limit", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%u", global.mps_limit);
		asl_msg_set_key_val(m, "mps_limit", val);
	}

	if (all || (0 == asl_msg_lookup(q, "bsd_max_dup_time", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%llu", global.bsd_max_dup_time);
		asl_msg_set_key_val(m, "bsd_max_dup_time", val);
	}

	if (all || (0 == asl_msg_lookup(q, "mark_time", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%llu", global.mark_time);
		asl_msg_set_key_val(m, "mark_time", val);
	}

	if (all || (0 == asl_msg_lookup(q, "utmp_ttl", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%lu", global.utmp_ttl);
		asl_msg_set_key_val(m, "utmp_ttl", val);
	}

	if (all || (0 == asl_msg_lookup(q, "max_work_queue_size", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%lld", global.max_work_queue_size);
		asl_msg_set_key_val(m, "max_work_queue_size", val);
	}

	if (all || (0 == asl_msg_lookup(q, "work_queue_count", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.work_queue_count);
		asl_msg_set_key_val(m, "work_queue_count", val);
	}

	if (all || (0 == asl_msg_lookup(q, "asl_queue_count", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.asl_queue_count);
		asl_msg_set_key_val(m, "asl_queue_count", val);
	}

	if (all || (0 == asl_msg_lookup(q, "bsd_queue_count", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.bsd_queue_count);
		asl_msg_set_key_val(m, "bsd_queue_count", val);
	}

	if (all || (0 == asl_msg_lookup(q, "client_count", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.client_count);
		asl_msg_set_key_val(m, "client_count", val);
	}

	if (all || (0 == asl_msg_lookup(q, "disaster_occurred", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.disaster_occurred);
		asl_msg_set_key_val(m, "disaster_occurred", val);
	}

#ifdef LOCKDOWN
	if (all || (0 == asl_msg_lookup(q, "lockdown_session_count", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.lockdown_session_count);
		asl_msg_set_key_val(m, "lockdown_session_count", val);
	}

	if (all || (0 == asl_msg_lookup(q, "remote_delay_time", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%u", global.remote_delay_time);
		asl_msg_set_key_val(m, "remote_delay_time", val);
	}

#endif

	if (all || (0 == asl_msg_lookup(q, "watchers_active", NULL, NULL)))
	{
		snprintf(val, sizeof(val), "%d", global.watchers_active);
		asl_msg_set_key_val(m, "watchers_active", val);
	}

	for (i = 0; i < global.module_count; i++)
	{
		if (all || (0 == asl_msg_lookup(q, global.module[i]->name, NULL, NULL)))
		{
			snprintf(val, sizeof(val), "%s", global.module[i]->enabled ? "enabled" : "disabled");
			asl_msg_set_key_val(m, global.module[i]->name, val);
		}
	}

	for (om = global.asl_out_module; om != NULL; om = om->next)
	{
		if (all || (0 == asl_msg_lookup(q, om->name, NULL, NULL)))
		{
			snprintf(val, sizeof(val), "%s", om->flags & MODULE_FLAG_ENABLED ? "enabled" : "disabled");
			if (om->name == NULL) asl_msg_set_key_val(m, "asl.conf", val);
			else asl_msg_set_key_val(m, om->name, val);
		}
	}

	/* synchronous actions use queries, since messages are simpleroutines */
	if (0 == asl_msg_lookup(q, "action", &mval, NULL))
	{
		int res = -1;
		if (uid == 0) res = asl_action_control_set_param(mval);
		snprintf(val, sizeof(val), "%d", res);
		asl_msg_set_key_val(m, "action", val);
	}

	asl_msg_release(m);
	*res = out;
	return ASL_STATUS_OK;
}

/*
 * Receives messages on the "com.apple.system.logger" mach port.
 * Services database search requests.
 * Runs in it's own thread.
 */
void
database_server()
{
	asl_request_msg *request;
	uint32_t rqs;
	struct timeval now, send_time;
	mach_dead_name_notification_t *deadname;
	const uint32_t rbits = MACH_RCV_MSG | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT) | MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0); // XXX | MACH_RCV_VOUCHER;

	send_time.tv_sec = 0;
	send_time.tv_usec = 0;

	rqs = sizeof(asl_request_msg) + MAX_TRAILER_SIZE;

	asl_server_queue = dispatch_queue_create("ASL Server Queue", NULL);

	forever
	{
		kern_return_t ks;

		now.tv_sec = 0;
		now.tv_usec = 0;

		request = (asl_request_msg *)calloc(1, rqs);
		if (request == NULL) continue;

		request->head.msgh_local_port = global.server_port;
		request->head.msgh_size = rqs;

		ks = mach_msg(&(request->head), rbits, 0, rqs, global.listen_set, 0, MACH_PORT_NULL);
		if (ks != KERN_SUCCESS)
		{
			/*
			 * This shouldn't happen, but if we get a failure the best thing to do is to crash.
			 */
			char str[256];
			asldebug("FATAL ERROR: mach_msg() receive failed with status 0x%08x\n", ks);
			snprintf(str, sizeof(str), "[Sender syslogd] [Level 1] [PID %u] [Facility syslog] [Message FATAL ERROR: mach_msg() receive failed with status 0x%08x]", global.pid, ks);
			internal_log_message(str);
			sleep(1);
			abort();
		}

		if (request->head.msgh_id == MACH_NOTIFY_DEAD_NAME)
		{
			deadname = (mach_dead_name_notification_t *)request;
			dispatch_async(asl_server_queue, ^{
				cancel_session(deadname->not_port);
				/* dead name notification includes a dead name right */
				mach_port_deallocate(mach_task_self(), deadname->not_port);
				free(request);
			});

			continue;
		}

		dispatch_async(asl_server_queue, ^{
			const uint32_t sbits = MACH_SEND_MSG | MACH_SEND_TIMEOUT;;
			kern_return_t ks;
			asl_reply_msg *reply = calloc(1, sizeof(asl_reply_msg) + MAX_TRAILER_SIZE);

			//voucher_mach_msg_state_t voucher = voucher_mach_msg_adopt(&(request->head));

			/* MIG server routine */
			asl_ipc_server(&(request->head), &(reply->head));

			if (!(reply->head.msgh_bits & MACH_MSGH_BITS_COMPLEX))
			{
				if (reply->reply.Reply__asl_server_message.RetCode == MIG_NO_REPLY)
				{
					reply->head.msgh_remote_port = MACH_PORT_NULL;
				}
				else if ((reply->reply.Reply__asl_server_message.RetCode != KERN_SUCCESS) && (request->head.msgh_bits & MACH_MSGH_BITS_COMPLEX))
				{
					/* destroy the request - but not the reply port */
					request->head.msgh_remote_port = MACH_PORT_NULL;
					mach_msg_destroy(&(request->head));
				}
			}

			if (reply->head.msgh_remote_port != MACH_PORT_NULL)
			{
				ks = mach_msg(&(reply->head), sbits, reply->head.msgh_size, 0, MACH_PORT_NULL, 10, MACH_PORT_NULL);
				if ((ks == MACH_SEND_INVALID_DEST) || (ks == MACH_SEND_TIMED_OUT))
				{
					/* clean up */
					mach_msg_destroy(&(reply->head));
				}
				else if (ks == MACH_SEND_INVALID_HEADER)
				{
					/*
					 * This should never happen, but we can continue running.
					 */
					char str[256];
					asldebug("ERROR: mach_msg() send failed with MACH_SEND_INVALID_HEADER 0x%08x\n", ks);
					snprintf(str, sizeof(str), "[Sender syslogd] [Level 3] [PID %u] [Facility syslog] [Message mach_msg() send failed with status 0x%08x (MACH_SEND_INVALID_HEADER)]", global.pid, ks);
					internal_log_message(str);
					mach_msg_destroy(&(reply->head));
				}
				else if (ks == MACH_SEND_NO_BUFFER)
				{
					/*
					 * This should never happen, but the kernel can run out of memory.
					 * We clean up and continue running.
					 */
					char str[256];
					asldebug("ERROR: mach_msg() send failed with MACH_SEND_NO_BUFFER 0x%08x\n", ks);
					snprintf(str, sizeof(str), "[Sender syslogd] [Level 3] [PID %u] [Facility syslog] [Message mach_msg() send failed with status 0x%08x (MACH_SEND_NO_BUFFER)]", global.pid, ks);
					internal_log_message(str);
					mach_msg_destroy(&(reply->head));
				}
				else if (ks != KERN_SUCCESS)
				{
					/*
					 * Failed to send a reply message.  This should never happen,
					 * but the best action is to crash.
					 */
					char str[256];
					asldebug("FATAL ERROR: mach_msg() send failed with status 0x%08x\n", ks);
					snprintf(str, sizeof(str), "[Sender syslogd] [Level 1] [PID %u] [Facility syslog] [Message FATAL ERROR: mach_msg() send failed with status 0x%08x]", global.pid, ks);
					internal_log_message(str);
					sleep(1);
					abort();
				}
			}
			else if (reply->head.msgh_bits & MACH_MSGH_BITS_COMPLEX)
			{
				mach_msg_destroy(&reply->head);
			}

			//voucher_mach_msg_revert(voucher);
			free(request);
			free(reply);
		});
	}
}

static void
caller_get_read_entitlement(pid_t pid, uid_t *uid, gid_t *gid)
{
#if TARGET_OS_EMBEDDED
	xpc_object_t edata, entitlements, val;
	bool bval = false;
	int64_t ival = -2;
	size_t len;
	const void *ptr;

	edata = xpc_copy_entitlements_for_pid(pid);
	if (edata == NULL) return;

	ptr = xpc_data_get_bytes_ptr(edata);
	len = xpc_data_get_length(edata);

	entitlements = xpc_create_from_plist(ptr, len);
	xpc_release(edata);
	if (entitlements == NULL) return;

	if (xpc_get_type(entitlements) != XPC_TYPE_DICTIONARY)
	{
		asldebug("xpc_copy_entitlements_for_pid has non-dictionary data for pid %d\n", pid);
		return;
	}

	bval = xpc_dictionary_get_bool(entitlements, ASL_ENTITLEMENT_KEY);
	if (bval && (uid != NULL))
	{
		*uid = 0;
		xpc_release(entitlements);
		return;
	}

	val = xpc_dictionary_get_value(entitlements, ASL_ENTITLEMENT_UID_KEY);
	if (val != NULL)
	{
		if ((xpc_get_type(val) == XPC_TYPE_INT64) && (uid != NULL))
		{
			ival = xpc_int64_get_value(val);
			*uid = ival;
		}
	}

	val = xpc_dictionary_get_value(entitlements, ASL_ENTITLEMENT_GID_KEY);
	if (val != NULL)
	{
		if ((xpc_get_type(val) == XPC_TYPE_INT64) && (gid != NULL))
		{
			ival = xpc_int64_get_value(val);
			*gid = ival;
		}
	}

	xpc_release(entitlements);
#endif
}

static kern_return_t
__asl_server_query_internal
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	uint32_t duration,
	int direction,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	uid_t uid,
	gid_t gid,
	pid_t pid
)
{
	asl_msg_list_t *query;
	asl_msg_list_t *res;
	char *out, *vmbuffer;
	uint32_t outlen;
	kern_return_t kstatus;

	*status = ASL_STATUS_OK;

	if ((request != NULL) && (request[requestCnt - 1] != '\0'))
	{
		*status = ASL_STATUS_INVALID_ARG;
		vm_deallocate(mach_task_self(), (vm_address_t)request, requestCnt);
		return KERN_SUCCESS;
	}

	query = asl_msg_list_from_string(request);
	if (request != NULL) vm_deallocate(mach_task_self(), (vm_address_t)request, requestCnt);
	res = NULL;

	/* A query list containing a single query, which itself contains
	 * [ASLOption control] is an internal state query */
	if ((query != NULL) && (query->count == 1) && asl_check_option(query->msg[0], ASL_OPT_CONTROL))
	{
		*status = syslogd_state_query(query->msg[0], &res, uid);
	}
	else
	{
		int x = 0;
#if TARGET_OS_EMBEDDED
		x = pid;
#endif

		if (pid > 0)
		{
			caller_get_read_entitlement(pid, &uid, &gid);
			if (uid == 0) x = 0;
		}

		*status = db_query(query, &res, startid, count, duration, direction, lastid, uid, gid, x);
	}

	asl_msg_list_release(query);
	if (*status != ASL_STATUS_INVALID_STORE)
	{
		/* ignore */
	}
	else if (*status != ASL_STATUS_OK)
	{
		if (res != NULL) asl_msg_list_release(res);
		return KERN_SUCCESS;
	}

	out = NULL;
	outlen = 0;
	out = asl_msg_list_to_string(res, &outlen);
	asl_msg_list_release(res);

	if ((out == NULL) || (outlen == 0)) return KERN_SUCCESS;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmbuffer, outlen, TRUE);
	if (kstatus != KERN_SUCCESS)
	{
		free(out);
		return kstatus;
	}

	memmove(vmbuffer, out, outlen);
	free(out);

	*reply = vmbuffer;
	*replyCnt = outlen;

	return KERN_SUCCESS;
}

kern_return_t
__asl_server_query_2
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	audit_token_t token
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	int direction = SEARCH_FORWARD;
	if (flags & QUERY_FLAG_SEARCH_REVERSE) direction = SEARCH_BACKWARD;

	audit_token_to_au32(token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);

	return __asl_server_query_internal(server, request, requestCnt, startid, count, QUERY_DURATION_UNLIMITED, direction, reply, replyCnt, lastid, status, uid, gid, pid);
}

kern_return_t
__asl_server_query
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	security_token_t *token
)
{
	int direction = SEARCH_FORWARD;
	if (flags & QUERY_FLAG_SEARCH_REVERSE) direction = SEARCH_BACKWARD;
	
	return __asl_server_query_internal(server, request, requestCnt, startid, count, QUERY_DURATION_UNLIMITED, direction, reply, replyCnt, lastid, status, (uid_t)token->val[0], (gid_t)token->val[1], (pid_t)-1);
}

kern_return_t
__asl_server_query_timeout
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	int count,
	int flags,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	audit_token_t token
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	int direction = SEARCH_FORWARD;
	if (flags & QUERY_FLAG_SEARCH_REVERSE) direction = SEARCH_BACKWARD;

	audit_token_to_au32(token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);

	return __asl_server_query_internal(server, request, requestCnt, startid, count, QUERY_DURATION_UNLIMITED, direction, reply, replyCnt, lastid, status, uid, gid, pid);
}

kern_return_t
__asl_server_match
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	uint64_t startid,
	uint64_t count,
	uint32_t duration,
	int direction,
	caddr_t *reply,
	mach_msg_type_number_t *replyCnt,
	uint64_t *lastid,
	int *status,
	audit_token_t token
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	
	audit_token_to_au32(token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);
	
	return __asl_server_query_internal(server, request, requestCnt, startid, count, duration, direction, reply, replyCnt, lastid, status, uid, gid, pid);
}

kern_return_t
__asl_server_prune
(
	mach_port_t server,
	caddr_t request,
	mach_msg_type_number_t requestCnt,
	int *status,
	security_token_t *token
)
{
	return KERN_SUCCESS;
}

kern_return_t
__asl_server_message
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	audit_token_t token
)
{
	asl_msg_t *msg;
	char tmp[64];
	uid_t uid;
	gid_t gid;
	pid_t pid;
	kern_return_t kstatus;
	mach_port_name_t client;

	if (message == NULL)
	{
		return KERN_SUCCESS;
	}

	if (message[messageCnt - 1] != '\0')
	{
		vm_deallocate(mach_task_self(), (vm_address_t)message, messageCnt);
		return KERN_SUCCESS;
	}

	asldebug("__asl_server_message: %s\n", (message == NULL) ? "NULL" : message);

	msg = asl_msg_from_string(message);
	vm_deallocate(mach_task_self(), (vm_address_t)message, messageCnt);

	if (msg == NULL) return KERN_SUCCESS;

	uid = (uid_t)-1;
	gid = (gid_t)-1;
	pid = (pid_t)-1;
	audit_token_to_au32(token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);

	client = MACH_PORT_NULL;
	kstatus = task_name_for_pid(mach_task_self(), pid, &client);
	if (kstatus == KERN_SUCCESS) register_session(client, pid);

	snprintf(tmp, sizeof(tmp), "%d", uid);
	asl_msg_set_key_val(msg, ASL_KEY_UID, tmp);

	snprintf(tmp, sizeof(tmp), "%d", gid);
	asl_msg_set_key_val(msg, ASL_KEY_GID, tmp);

	snprintf(tmp, sizeof(tmp), "%d", pid);
	asl_msg_set_key_val(msg, ASL_KEY_PID, tmp);

	process_message(msg, SOURCE_ASL_MESSAGE);

	return KERN_SUCCESS;
}

kern_return_t
__asl_server_create_aux_link
(
	mach_port_t server,
	caddr_t message,
	mach_msg_type_number_t messageCnt,
	mach_port_t *fileport,
	caddr_t *newurl,
	mach_msg_type_number_t *newurlCnt,
	int *status,
	audit_token_t token
)
{
	asl_msg_t *msg;
	char tmp[64];
	uid_t uid;
	gid_t gid;
	pid_t pid;
	kern_return_t kstatus;
	mach_port_name_t client;
	char *url, *vmbuffer;
	int fd;

	*status = ASL_STATUS_OK;
	*fileport = MACH_PORT_NULL;
	*newurl = 0;

	if (message == NULL)
	{
		*status = ASL_STATUS_INVALID_ARG;
		return KERN_SUCCESS;
	}

	if (message[messageCnt - 1] != '\0')
	{
		*status = ASL_STATUS_INVALID_ARG;
		vm_deallocate(mach_task_self(), (vm_address_t)message, messageCnt);
		return KERN_SUCCESS;
	}

	asldebug("__asl_server_create_aux_link: %s\n", (message == NULL) ? "NULL" : message);

	if ((global.dbtype & DB_TYPE_FILE) == 0)
	{
		*status = ASL_STATUS_INVALID_STORE;
		return KERN_SUCCESS;
	}

	*fileport = MACH_PORT_NULL;

	msg = asl_msg_from_string(message);
	vm_deallocate(mach_task_self(), (vm_address_t)message, messageCnt);

	if (msg == NULL) return KERN_SUCCESS;

	uid = (uid_t)-1;
	gid = (gid_t)-1;
	pid = (pid_t)-1;
	audit_token_to_au32(token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);

	client = MACH_PORT_NULL;
	kstatus = task_name_for_pid(mach_task_self(), pid, &client);
	if (kstatus == KERN_SUCCESS) register_session(client, pid);

	snprintf(tmp, sizeof(tmp), "%d", uid);
	asl_msg_set_key_val(msg, ASL_KEY_UID, tmp);

	snprintf(tmp, sizeof(tmp), "%d", gid);
	asl_msg_set_key_val(msg, ASL_KEY_GID, tmp);

	snprintf(tmp, sizeof(tmp), "%d", pid);
	asl_msg_set_key_val(msg, ASL_KEY_PID, tmp);

	/* create a file for the client */
	*status = asl_store_open_aux(global.file_db, msg, &fd, &url);
	asl_msg_release(msg);
	if (*status != ASL_STATUS_OK) return KERN_SUCCESS;
	if (url == NULL)
	{
		if (fd >= 0) close(fd);
		*status = ASL_STATUS_FAILED;
		return KERN_SUCCESS;
	}

#if 0
	if (fileport_makeport(fd, (fileport_t *)fileport) < 0)
	{
		close(fd);
		free(url);
		*status = ASL_STATUS_FAILED;
		return KERN_SUCCESS;
	}
#endif

	close(fd);

	*newurlCnt = strlen(url) + 1;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmbuffer, *newurlCnt, TRUE);
	if (kstatus != KERN_SUCCESS)
	{
		free(url);
		return kstatus;
	}

	memmove(vmbuffer, url, *newurlCnt);
	free(url);

	*newurl = vmbuffer;

	return KERN_SUCCESS;
}

kern_return_t
__asl_server_register_direct_watch
(
	mach_port_t server,
	int port,
	audit_token_t token
)
{
	uint16_t p16 = port;
	pid_t pid = (pid_t)-1;

	audit_token_to_au32(token, NULL, NULL, NULL, NULL, NULL, &pid, NULL, NULL);

	asldebug("__asl_server_register_direct_watch: pid %u port %hu\n", pid, ntohs(p16));

	dispatch_once(&watch_init_once, ^{
		watch_queue = dispatch_queue_create("Direct Watch Queue", NULL);
	});

	dispatch_async(watch_queue, ^{ register_direct_watch(p16); });

	return KERN_SUCCESS;
}

kern_return_t
__asl_server_cancel_direct_watch
(
	mach_port_t server,
	int port,
	audit_token_t token
)
{
	uint16_t p16 = port;

	asldebug("__asl_server_cancel_direct_watch: %hu\n", ntohs(p16));

	dispatch_once(&watch_init_once, ^{
		watch_queue = dispatch_queue_create("Direct Watch Queue", NULL);
	});

	dispatch_async(watch_queue, ^{ cancel_direct_watch(p16); });

	return KERN_SUCCESS;
}
