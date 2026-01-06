/*
 * Copyright (c) 2004-2011 Apple Inc. All rights reserved.
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
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "daemon.h"

#define BSD_SOCKET_NAME "BSDSystemLogger"
#define MY_ID "bsd_in"
#define MAXLINE 4096

static int sock = -1;
static dispatch_source_t in_src;
static dispatch_queue_t in_queue;

void
bsd_in_acceptmsg(int fd)
{
	uint32_t len;
	int n;
	char line[MAXLINE];
	struct sockaddr_un sun;
	asl_msg_t *m;

	len = sizeof(struct sockaddr_un);
	n = recvfrom(fd, line, MAXLINE, 0, (struct sockaddr *)&sun, &len);

	if (n <= 0) return;

	line[n] = '\0';

	m = asl_input_parse(line, n, NULL, SOURCE_BSD_SOCKET);
	process_message(m, SOURCE_BSD_SOCKET);
}

int
bsd_in_init()
{
	int rbufsize;
	int len;
	launch_data_t sockets_dict, fd_array, fd_dict;
	static dispatch_once_t once;

#if TARGET_OS_SIMULATOR
	const char *_PATH_SYSLOG_IN = getenv("IOS_SIMULATOR_SYSLOG_SOCKET");
#endif

	dispatch_once(&once, ^{
		in_queue = dispatch_queue_create(MY_ID, NULL);
	});

	asldebug("%s: init\n", MY_ID);
	if (sock >= 0) return -1;

	if (global.launch_dict == NULL)
	{
		asldebug("%s: launchd dict is NULL\n", MY_ID);
		return -1;
	}

	sockets_dict = launch_data_dict_lookup(global.launch_dict, LAUNCH_JOBKEY_SOCKETS);
	if (sockets_dict == NULL)
	{
		asldebug("%s: launchd lookup of LAUNCH_JOBKEY_SOCKETS failed\n", MY_ID);
		return -1;
	}

	fd_array = launch_data_dict_lookup(sockets_dict, BSD_SOCKET_NAME);
	if (fd_array == NULL)
	{
		asldebug("%s: launchd lookup of BSD_SOCKET_NAME failed\n", MY_ID);
		return -1;
	}

	len = launch_data_array_get_count(fd_array);
	if (len <= 0)
	{
		asldebug("%s: launchd fd array is empty\n", MY_ID);
		return -1;
	}

	if (len > 1)
	{
		asldebug("%s: warning! launchd fd array has %d sockets\n", MY_ID, len);
	}

	fd_dict = launch_data_array_get_index(fd_array, 0);
	if (fd_dict == NULL)
	{
		asldebug("%s: launchd file discriptor array element 0 is NULL\n", MY_ID);
		return -1;
	}

	sock = launch_data_get_fd(fd_dict);

	rbufsize = 128 * 1024;
	len = sizeof(rbufsize);

	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rbufsize, len) < 0)
	{
		asldebug("%s: couldn't set receive buffer size for socket %d (%s): %s\n", MY_ID, sock, _PATH_SYSLOG_IN, strerror(errno));
		close(sock);
		sock = -1;
		return -1;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
	{
		asldebug("%s: couldn't set O_NONBLOCK for socket %d (%s): %s\n", MY_ID, sock, _PATH_SYSLOG_IN, strerror(errno));
		close(sock);
		sock = -1;
		return -1;
	}

	in_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)sock, 0, in_queue);
	dispatch_source_set_event_handler(in_src, ^{ bsd_in_acceptmsg(sock); });

	dispatch_resume(in_src);
	return 0;
}

int
bsd_in_close(void)
{
	if (sock < 0) return 1;

	dispatch_source_cancel(in_src);
	dispatch_release(in_src);
	in_src = NULL;

	close(sock);
	sock = -1;

	return 0;
}

int
bsd_in_reset(void)
{
	return 0;
}
