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

#if TARGET_IPHONE_SIMULATOR
struct _not_empty;
#else

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <notify.h>
#include "daemon.h"

#define forever for(;;)

#define MY_ID "remote"
#define MAXLINE 4096
#define LOCKDOWN_PATH "/var/run/lockdown"
#define SYSLOG_SOCK_PATH "/var/run/lockdown/syslog.sock"
#define ASL_REMOTE_PORT 203

#define PRINT_STD 0
#define PRINT_RAW 1

#define WATCH_OFF 0
#define WATCH_LOCKDOWN_START 1
#define WATCH_RUN 2

#define SESSION_FLAGS_LOCKDOWN 0x00000001

#define MAXSOCK 1

static int rfd4 = -1;
static int rfd6 = -1;
static int rfdl = -1;

static dispatch_source_t in_src_local;
static dispatch_source_t in_src_tcp;
static dispatch_queue_t in_queue;

#ifdef NSS32 
typedef uint32_t notify_state_t;
extern int notify_set_state(int, notify_state_t);
#else
typedef uint64_t notify_state_t;
#endif

extern size_t asl_memory_size(asl_memory_t *s);
extern uint32_t db_query(asl_msg_list_t *query, asl_msg_list_t **res, uint64_t startid, int count, uint32_t duration, int direction, uint64_t *lastid, int32_t ruid, int32_t rgid, int raccess);

extern void add_lockdown_session(int fd);
extern void remove_lockdown_session(int fd);

#define SESSION_WRITE(f,x) if (write(f, x, strlen(x)) < 0) goto exit_session

typedef struct
{
	int sock;
	uint32_t flags;
} session_args_t;

uint32_t
remote_db_size(uint32_t sel)
{
	if (sel == DB_TYPE_FILE) return global.db_file_max;
	if (sel == DB_TYPE_MEMORY) return global.db_memory_max;
	return 0;
}

uint32_t
remote_db_set_size(uint32_t sel, uint32_t size)
{
	if (sel == DB_TYPE_FILE) global.db_file_max = size;
	if (sel == DB_TYPE_MEMORY) global.db_memory_max = size;
	return 0;
}

asl_msg_t *
remote_db_stats(uint32_t sel)
{
	asl_msg_t *m;
	m = NULL;

	if (sel == DB_TYPE_FILE) asl_store_statistics(global.file_db, &m);
	if (sel == DB_TYPE_MEMORY) asl_memory_statistics(global.memory_db, &m);
	return m;
}

void
session(void *x)
{
	int i, s, wfd, status, pfmt, watch, wtoken, nfd, do_prompt;
	asl_msg_list_t *res;
	asl_msg_list_t ql;
	uint32_t outlen;
	asl_msg_t *stats;
	asl_msg_t *query;
	asl_msg_t *qlq[1];
	char str[1024], *p, *qs, *out;
	ssize_t len;
	fd_set readfds, errfds;
	uint64_t low_id, high_id;
	uint32_t dbselect, flags;
	session_args_t *sp;

	if (x == NULL) pthread_exit(NULL);

	sp = (session_args_t *)x;
	s = sp->sock;
	flags = sp->flags;
	free(x);

	asldebug("%s %d: starting interactive session for %ssocket %d\n", MY_ID, s, (flags & SESSION_FLAGS_LOCKDOWN) ? "lockdown " : "", s);

	do_prompt = 1;
	watch = WATCH_OFF;
	wfd = -1;
	wtoken = -1;

	dbselect = 0;
	if (global.dbtype & DB_TYPE_MEMORY) dbselect = DB_TYPE_MEMORY;
	else if (global.dbtype & DB_TYPE_FILE) dbselect = DB_TYPE_FILE;

	low_id = 0;
	high_id = 0;

	pfmt = PRINT_STD;
	query = NULL;
	memset(&ql, 0, sizeof(asl_msg_list_t));

	if (flags & SESSION_FLAGS_LOCKDOWN) sleep(1);

	snprintf(str, sizeof(str), "\n========================\nASL is here to serve you\n");
	if (write(s, str, strlen(str)) < 0)
	{
		close(s);
		pthread_exit(NULL);
		return;
	}

	if (flags & SESSION_FLAGS_LOCKDOWN)
	{
		snprintf(str, sizeof(str), "> ");
		SESSION_WRITE(s, str);
	}

	forever
	{
		if (((flags & SESSION_FLAGS_LOCKDOWN) == 0) && (do_prompt > 0))
		{
			snprintf(str, sizeof(str), "> ");
			SESSION_WRITE(s, str);
		}

		do_prompt = 1;

		memset(str, 0, sizeof(str));

		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		FD_ZERO(&errfds);
		FD_SET(s, &errfds);
		nfd = s;

		if (wfd != -1)
		{
			FD_SET(wfd, &readfds);
			if (wfd > nfd) nfd = wfd;
		}

		status = select(nfd + 1, &readfds, NULL, &errfds, NULL);
		if (status == 0) continue;
		if (status < 0)
		{
			asldebug("%s %d: select %d %s\n", MY_ID, s, errno, strerror(errno));
			goto exit_session;
		}

		if (FD_ISSET(s, &errfds))
		{
			asldebug("%s %d: error on socket %d\n", MY_ID, s, s);
			goto exit_session;
		}

		if ((wfd != -1) && (FD_ISSET(wfd, &readfds)))
		{
			(void)read(wfd, &i, sizeof(int));
		}

		if (FD_ISSET(s, &errfds))
		{
			asldebug("%s %d: socket %d reported error\n", MY_ID, s, s);
			goto exit_session;
		}

		if (FD_ISSET(s, &readfds))
		{
			len = read(s, str, sizeof(str) - 1);
			if (len <= 0)
			{
				asldebug("%s %d: read error on socket %d: %d %s\n", MY_ID, s, s, errno, strerror(errno));
				goto exit_session;
			}

			while ((len > 1) && ((str[len - 1] == '\n') || (str[len - 1] == '\r')))
			{
				str[len - 1] = '\0';
				len--;
			}

			if ((!strcmp(str, "q")) || (!strcmp(str, "quit")) || (!strcmp(str, "exit")))
			{
				snprintf(str, sizeof(str), "Goodbye\n");
				write(s, str, strlen(str));
				close(s);
				s = -1;
				break;
			}

			if ((!strcmp(str, "?")) || (!strcmp(str, "help")))
			{
				snprintf(str, sizeof(str), "Commands\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    quit                 exit session\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    select [val]         get [set] current database\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "                         val must be \"file\" or \"mem\"\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    file [on/off]        enable / disable file store\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    memory [on/off]      enable / disable memory store\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    stats                database statistics\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    flush                flush database\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    dbsize [val]         get [set] database size (# of records)\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    watch                print new messages as they arrive\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    stop                 stop watching for new messages\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    raw                  use raw format for printing messages\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    std                  use standard format for printing messages\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    *                    show all log messages\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    * key val            equality search for messages (single key/value pair)\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    * op key val         search for matching messages (single key/value pair)\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "    * [op key val] ...   search for matching messages (multiple key/value pairs)\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "                         operators:  =  <  >  ! (not equal)  T (key exists)  R (regex)\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "                         modifiers (must follow operator):\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "                                 C=casefold  N=numeric  S=substring  A=prefix  Z=suffix\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strcmp(str, "stats"))
			{
				stats = remote_db_stats(dbselect);
				out = asl_format_message((asl_msg_t *)stats, ASL_MSG_FMT_RAW, ASL_TIME_FMT_SEC, ASL_ENCODE_NONE, &outlen);
				write(s, out, outlen);
				free(out);
				asl_msg_release(stats);
				continue;
			}
			else if (!strcmp(str, "flush"))
			{}
			else if (!strncmp(str, "select", 6))
			{
				p = str + 6;
				while ((*p == ' ') || (*p == '\t')) p++;
				if (*p == '\0')
				{
					if (dbselect == 0) snprintf(str, sizeof(str), "no store\n");
					else if (dbselect == DB_TYPE_FILE) snprintf(str, sizeof(str), "file store\n");
					else if (dbselect == DB_TYPE_MEMORY) snprintf(str, sizeof(str), "memory store\n");
					SESSION_WRITE(s, str);
					continue;
				}

				if (!strncmp(p, "file", 4))
				{
					if ((global.dbtype & DB_TYPE_FILE) == 0)
					{
						snprintf(str, sizeof(str), "file database is not enabled\n");
						SESSION_WRITE(s, str);
						continue;
					}

					dbselect = DB_TYPE_FILE;
				}
				else if (!strncmp(p, "mem", 3))
				{
					if ((global.dbtype & DB_TYPE_MEMORY) == 0)
					{
						snprintf(str, sizeof(str), "memory database is not enabled\n");
						SESSION_WRITE(s, str);
						continue;
					}

					dbselect = DB_TYPE_MEMORY;
				}
				else
				{
					snprintf(str, sizeof(str), "unknown database type\n");
					SESSION_WRITE(s, str);
					continue;
				}

				snprintf(str, sizeof(str), "OK\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strncmp(str, "file", 4))
			{
				p = str + 4;
				while ((*p == ' ') || (*p == '\t')) p++;
				if (*p == '\0')
				{
					snprintf(str, sizeof(str), "file database is %senabled\n", (global.dbtype & DB_TYPE_FILE) ? "" : "not ");
					SESSION_WRITE(s, str);
					if ((global.dbtype & DB_TYPE_FILE) != 0) dbselect = DB_TYPE_FILE;
					continue;
				}

				if (!strcmp(p, "on")) global.dbtype |= DB_TYPE_FILE;
				else if (!strcmp(p, "off")) global.dbtype &= ~ DB_TYPE_FILE;

				snprintf(str, sizeof(str), "OK\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strncmp(str, "memory", 6))
			{
				p = str + 6;
				while ((*p == ' ') || (*p == '\t')) p++;
				if (*p == '\0')
				{
					snprintf(str, sizeof(str), "memory database is %senabled\n", (global.dbtype & DB_TYPE_MEMORY) ? "" : "not ");
					SESSION_WRITE(s, str);
					if ((global.dbtype & DB_TYPE_MEMORY) != 0) dbselect = DB_TYPE_MEMORY;
					continue;
				}

				if (!strcmp(p, "on")) global.dbtype |= DB_TYPE_MEMORY;
				else if (!strcmp(p, "off")) global.dbtype &= ~ DB_TYPE_MEMORY;

				snprintf(str, sizeof(str), "OK\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strncmp(str, "dbsize", 6))
			{
				if (dbselect == 0)
				{
					snprintf(str, sizeof(str), "no store\n");
					SESSION_WRITE(s, str);
					continue;
				}

				p = str + 6;
				while ((*p == ' ') || (*p == '\t')) p++;
				if (*p == '\0')
				{
					snprintf(str, sizeof(str), "DB size %u\n", remote_db_size(dbselect));
					SESSION_WRITE(s, str);
					continue;
				}

				i = atoi(p);
				remote_db_set_size(dbselect, i);

				snprintf(str, sizeof(str), "OK\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strcmp(str, "stop"))
			{
				if (watch != WATCH_OFF)
				{
					watch = WATCH_OFF;
					notify_cancel(wtoken);
					wfd = -1;
					wtoken = -1;

					low_id = 0;
					high_id = 0;

					if (query != NULL) free(query);
					query = NULL;

					snprintf(str, sizeof(str), "OK\n");
					SESSION_WRITE(s, str);
					continue;
				}

				snprintf(str, sizeof(str), "not watching!\n");
				SESSION_WRITE(s, str);
				continue;
			}
			else if (!strcmp(str, "raw"))
			{
				pfmt = PRINT_RAW;
				continue;
			}
			else if (!strcmp(str, "std"))
			{
				pfmt = PRINT_STD;
				continue;
			}
			else if (!strcmp(str, "watch"))
			{
				if (((flags & SESSION_FLAGS_LOCKDOWN) == 0) && (watch != WATCH_OFF))
				{
					snprintf(str, sizeof(str), "already watching!\n");
					SESSION_WRITE(s, str);
					continue;
				}

				if (flags & SESSION_FLAGS_LOCKDOWN)
				{
					/*
					 * If this session is PurpleConsole or Xcode watching for log messages,
					 * we pass through the bottom of the loop (below) once to pick up
					 * existing messages already in memory.  After that, dbserver will
					 * send new messages in send_to_direct_watchers().  We wait until
					 * the initial messages are sent before adding the connection to
					 * global.lockdown_session_fds to allow this query to complete before
					 * dbserver starts sending.  To prevent a race between this query and
					 * when messages are sent by send_to_direct_watchers, we suspend the
					 * work queue and resume it when lockdown_session_fds has been updated.
					 */
					watch = WATCH_LOCKDOWN_START;
					dispatch_suspend(global.work_queue);
				}
				else
				{
					#if 0
					status = notify_register_file_descriptor(kNotifyASLDBUpdate, &wfd, 0, &wtoken);
					if (status != 0)
					{
						snprintf(str, sizeof(str), "notify_register_file_descriptor failed: %d\n", status);
						SESSION_WRITE(s, str);
						continue;
					}
					#endif

					watch = WATCH_RUN;
				}

				snprintf(str, sizeof(str), "OK\n");
				SESSION_WRITE(s, str);
				do_prompt = 2;
			}
			else if ((str[0] == '*') || (str[0] == 'T') || (str[0] == '=') || (str[0] == '!') || (str[0] == '<') || (str[0] == '>'))
			{
				memset(&ql, 0, sizeof(asl_msg_list_t));
				if (query != NULL) free(query);
				query = NULL;

				p = str;
				if (*p == '*') p++;
				while ((*p == ' ') || (*p == '\t')) p++;

				if (*p == '\0')
				{
					/* NULL query */
				}
				else if (*p == '[')
				{
					qs = NULL;
					asprintf(&qs, "Q %s", p);
					query = asl_msg_from_string(qs);
					free(qs);
				}
				else if ((*p == 'T') || (*p == '=') || (*p == '!') || (*p == '<') || (*p == '>') || (*p == 'R'))
				{
					qs = NULL;
					asprintf(&qs, "Q [%s]", p);
					query = asl_msg_from_string(qs);
					free(qs);
				}
				else
				{
					qs = NULL;
					asprintf(&qs, "Q [= %s]", p);
					query = asl_msg_from_string(qs);
					free(qs);
				}
			}
			else
			{
				snprintf(str, sizeof(str), "unrecognized command\n");
				SESSION_WRITE(s, str);
				snprintf(str, sizeof(str), "enter \"help\" for help\n");
				SESSION_WRITE(s, str);
				continue;
			}
		}

		if ((flags & SESSION_FLAGS_LOCKDOWN) && (watch == WATCH_RUN)) continue;

		/* Bottom of the loop: do a database query and print the results */

		if (query != NULL)
		{
			ql.count = 1;
			qlq[0] = query;
			ql.msg = qlq;
		}

		if (watch == WATCH_OFF) low_id = 0;

		res = NULL;
		high_id = 0;
		(void)db_query(&ql, &res, low_id, 0, 0, 0, &high_id, 0, 0, 0);

		if ((watch == WATCH_RUN) && (high_id >= low_id)) low_id = high_id + 1;

		if (res == NULL)
		{
			if (watch == WATCH_OFF)
			{
				snprintf(str, sizeof(str), "-nil-\n");
				SESSION_WRITE(s, str);
			}
			else
			{
				if (do_prompt != 2) do_prompt = 0;
			}
		}
		else if (pfmt == PRINT_RAW)
		{
			if (watch == WATCH_RUN)
			{
				snprintf(str, sizeof(str), "\n");
				SESSION_WRITE(s, str);
			}

			outlen = 0;
			out = asl_msg_list_to_string(res, &outlen);
			write(s, out, outlen);
			free(out);

			snprintf(str, sizeof(str), "\n");
			SESSION_WRITE(s, str);
		}
		else
		{
			if ((watch == WATCH_RUN) || (watch == WATCH_LOCKDOWN_START))
			{
				snprintf(str, sizeof(str), "\n");
				SESSION_WRITE(s, str);
			}

			snprintf(str, sizeof(str), "\n");
			for (i = 0; i < res->count; i++)
			{
				int wstatus;

				out = asl_format_message(res->msg[i], ASL_MSG_FMT_STD, ASL_TIME_FMT_LCL, ASL_ENCODE_SAFE, &outlen);

				do
				{
					int n = 0;

					errno = 0;
					wstatus = write(s, out, outlen);
					if (wstatus < 0)
					{
						asldebug("%s %d: %d/%d write data failed: %d %s\n", MY_ID, s, i, res->count, errno, strerror(errno));
						if (errno == EAGAIN)
						{
							n++;
							if (n > 10) break;
							usleep(10000);
						}
						else
						{
							goto exit_session;
						}
					}
				} while (errno == EAGAIN);

				free(out);
				if (global.remote_delay_time > 0) usleep(global.remote_delay_time);
			}
		}

		asl_msg_list_release(res);

		if (watch == WATCH_LOCKDOWN_START)
		{
			add_lockdown_session(s);
			watch = WATCH_RUN;
			dispatch_resume(global.work_queue);
		}
	}

exit_session:

	asldebug("%s %d: terminating session for %ssocket %d\n", MY_ID, s, (flags & SESSION_FLAGS_LOCKDOWN) ? "lockdown " : "", s);

	if (s >= 0)
	{
		if (flags & SESSION_FLAGS_LOCKDOWN) remove_lockdown_session(s);
		close(s);
	}

	if (watch == WATCH_LOCKDOWN_START) dispatch_resume(global.work_queue);
	if (wtoken >= 0) notify_cancel(wtoken);
	if (query != NULL) asl_msg_release(query);
	pthread_exit(NULL);
}

asl_msg_t *
remote_acceptmsg(int fd, int tcp)
{
	socklen_t fromlen;
	int s, flags, status, v;
	pthread_attr_t attr;
	pthread_t t;
	struct sockaddr_storage from;
	session_args_t *sp;

	fromlen = sizeof(struct sockaddr_un);
	if (tcp == 1) fromlen = sizeof(struct sockaddr_storage);

	memset(&from, 0, sizeof(from));

	s = accept(fd, (struct sockaddr *)&from, &fromlen);
	if (s == -1)
	{
		asldebug("%s: accept: %s\n", MY_ID, strerror(errno));
		return NULL;
	}

	flags = fcntl(s, F_GETFL, 0);
	flags &= ~ O_NONBLOCK;
	status = fcntl(s, F_SETFL, flags);
	if (status < 0)
	{
		asldebug("%s: fcntl: %s\n", MY_ID, strerror(errno));
		close(s);
		return NULL;
	}

	v = 1;
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &v, sizeof(v));

	if (tcp == 1)
	{
		flags = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(int));
	}

	sp = (session_args_t *)calloc(1, sizeof(session_args_t));
	if (sp == NULL)
	{
		asldebug("%s: malloc: %s\n", MY_ID, strerror(errno));
		close(s);
		return NULL;
	}

	sp->sock = s;
	if (tcp == 0) sp->flags |= SESSION_FLAGS_LOCKDOWN;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&t, &attr, (void *(*)(void *))session, (void *)sp);
	pthread_attr_destroy(&attr);

	return NULL;
}

asl_msg_t *
remote_acceptmsg_local(int fd)
{
	return remote_acceptmsg(fd, 0);
}

asl_msg_t *
remote_acceptmsg_tcp(int fd)
{
	return remote_acceptmsg(fd, 1);
}

int
remote_init_lockdown(void)
{
	int status, reuse, fd;
	struct sockaddr_un local;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
	{
		asldebug("%s: socket: %s\n", MY_ID, strerror(errno));
		return -1;
	}

	reuse = 1;
	status = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int));
	if (status < 0)
	{
		asldebug("%s: setsockopt: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	/* make sure the lockdown directory exists */
	mkdir(LOCKDOWN_PATH, 0777);

	memset(&local, 0, sizeof(local));
	local.sun_family = AF_UNIX;
	strlcpy(local.sun_path, SYSLOG_SOCK_PATH, sizeof(local.sun_path));
	unlink(local.sun_path);

	status = bind(fd, (struct sockaddr *)&local, sizeof(local.sun_family) + sizeof(local.sun_path));

	if (status < 0)
	{
		asldebug("%s: bind: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	status = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (status < 0)
	{
		asldebug("%s: fcntl: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	status = listen(fd, 5);
	if (status < 0)
	{
		asldebug("%s: listen: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	chmod(SYSLOG_SOCK_PATH, 0666);

	in_src_local = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)fd, 0, in_queue);
	dispatch_source_set_event_handler(in_src_local, ^{ remote_acceptmsg_local(fd); });
	dispatch_resume(in_src_local);

	return fd;
}

int
remote_init_tcp(int family)
{
	int status, reuse, fd;
	struct sockaddr_in a4;
	struct sockaddr_in6 a6;
	struct sockaddr *s;
	socklen_t len;

	fd = socket(family, SOCK_STREAM, 0);
	if (fd < 0)
	{
		asldebug("%s: socket: %s\n", MY_ID, strerror(errno));
		return -1;
	}

	reuse = 1;
	status = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int));
	if (status < 0)
	{
		asldebug("%s: setsockopt: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	memset(&(a4.sin_addr), 0, sizeof(struct in_addr));
	a4.sin_family = AF_INET;
	a4.sin_port = htons(ASL_REMOTE_PORT);

	memset(&(a6.sin6_addr), 0, sizeof(struct in6_addr));
	a6.sin6_family = AF_INET6;
	a6.sin6_port = htons(ASL_REMOTE_PORT);

	s = (struct sockaddr *)&a4;
	len = sizeof(struct sockaddr_in);

	if (family == AF_INET6)
	{
		s = (struct sockaddr *)&a6;
		len = sizeof(struct sockaddr_in6);
	}

	status = bind(fd, s, len);
	if (status < 0)
	{
		asldebug("%s: bind: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	status = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (status < 0)
	{
		asldebug("%s: fcntl: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	status = listen(fd, 5);
	if (status < 0)
	{
		asldebug("%s: listen: %s\n", MY_ID, strerror(errno));
		close(fd);
		return -1;
	}

	in_src_tcp = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)fd, 0, in_queue);
	dispatch_source_set_event_handler(in_src_tcp, ^{ remote_acceptmsg_tcp(fd); });
	dispatch_resume(in_src_tcp);

	return fd;
}

int
remote_init(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		in_queue = dispatch_queue_create(MY_ID, NULL);
	});

	asldebug("%s: init\n", MY_ID);

#ifdef LOCKDOWN
	rfdl = remote_init_lockdown();
#endif

#ifdef REMOTE_IPV4
	rfd4 = remote_init_tcp(AF_INET);
#endif

#ifdef REMOTE_IPV6
	rfd6 = remote_init_tcp(AF_INET6);
#endif

	return 0;
}

int
remote_close(void)
{
	if (rfdl >= 0)
	{
		close(rfdl);
	}

	rfdl = -1;

	if (rfd4 >= 0) 
	{
		close(rfd4);
	}

	rfd4 = -1;

	if (rfd6 >= 0)
	{
		close(rfd6);
	}

	rfd6 = -1;

	return 0;
}

int
remote_reset(void)
{
	return 0;

	remote_close();
	return remote_init();
}

#endif /* !TARGET_IPHONE_SIMULATOR */
