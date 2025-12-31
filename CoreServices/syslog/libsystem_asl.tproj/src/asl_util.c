/*
 * Copyright (c) 2006-2013 Apple Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <Block.h>
#include <dispatch/dispatch.h>
#include <os/assumes.h>
#include <xpc/xpc.h>
#include <syslog.h>
#include <asl_core.h>
#include <asl_private.h>

static uint8_t *b64charset = (uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int
asl_is_utf8_char(const unsigned char *p, int *state, int *ctype)
{
	switch (*state)
	{
		case 0:
		{
			*ctype = 0;

			if (*p >= 0x80)
			{
				*state = 1;
				if ((*p >= 0xc2) && (*p <= 0xdf)) *ctype = 1;
				else if (*p == 0xe0) *ctype = 2;
				else if ((*p >= 0xe1) && (*p <= 0xef)) *ctype = 3;
				else if (*p == 0xf0) *ctype = 4;
				else if ((*p >= 0xf1) && (*p <= 0xf3)) *ctype = 5;
				else if (*p == 0xf4) *ctype = 6;
				else return 0;
			}

			break;
		}

		case 1:
		{
			switch (*ctype)
			{
				case 1:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
					else return 0;
					break;
				}

				case 2:
				{
					if ((*p >= 0xa0) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 3:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 4:
				{
					if ((*p >= 0x90) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 5:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 6:
				{
					if ((*p >= 0x80) && (*p <= 0x8f)) *state = 2;
					else return 0;
					break;
				}

				default: return 0;
			}

			break;
		}

		case 2:
		{
			if ((*ctype >= 2) && (*ctype <= 3))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
				else return 0;
			}
			else if ((*ctype >= 4) && (*ctype <= 6))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 3;
				else return 0;
			}
			else
			{
				return 0;
			}

			break;
		}

		case 3:
		{
			if ((*ctype >= 4) && (*ctype <= 6))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
				else return 0;
			}
			else
			{
				return 0;
			}

			break;
		}

		default: return 0;
	}

	return 1;
}

__private_extern__ int
asl_is_utf8(const char *str)
{
	const unsigned char *p;
	int flag = 1;
	int state = 0;
	int ctype = 0;

	if (str == NULL) return flag;

	for (p = (const unsigned char *)str; (*p != '\0') && (flag == 1); p++)
	{
		flag = asl_is_utf8_char(p, &state, &ctype);
	}

	return flag;
}

__private_extern__ uint8_t *
asl_b64_encode(const uint8_t *buf, size_t len)
{
	uint8_t *out;
	uint8_t b;
	size_t i0, i1, i2, j, outlen;

	if (buf == NULL) return NULL;
	if (len == 0) return NULL;

	outlen = ((len + 2) / 3) * 4;
	out = (uint8_t *)malloc(outlen + 1);
	if (out == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	out[outlen] = 0;

	i0 = 0;
	i1 = 1;
	i2 = 2;
	j = 0;

	while (i2 < len)
	{
		b = buf[i0] >> 2;
		out[j++] = b64charset[b];

		b = ((buf[i0] & 0x03) << 4) | (buf[i1] >> 4);
		out[j++] = b64charset[b];

		b = ((buf[i1] & 0x0f) << 2) | ((buf[i2] & 0xc0) >> 6);
		out[j++] = b64charset[b];

		b = buf[i2] & 0x3f;
		out[j++] = b64charset[b];

		i0 += 3;
		i1 = i0 + 1;
		i2 = i1 + 1;
	}

	if (i0 < len)
	{
		b = buf[i0] >> 2;
		out[j++] = b64charset[b];

		b = (buf[i0] & 0x03) << 4;

		if (i1 < len) b |= (buf[i1] >> 4);
		out[j++] = b64charset[b];

		if (i1 >= len)
		{
			out[j++] = '=';
			out[j++] = '=';
			return out;
		}

		b = (buf[i1] & 0x0f) << 2;
		out[j++] = b64charset[b];
		out[j++] = '=';
	}

	return out;
}

int
asl_syslog_faciliy_name_to_num(const char *name)
{
	if (name == NULL) return -1;

	if (strcaseeq(name, "auth")) return LOG_AUTH;
	if (strcaseeq(name, "authpriv")) return LOG_AUTHPRIV;
	if (strcaseeq(name, "cron")) return LOG_CRON;
	if (strcaseeq(name, "daemon")) return LOG_DAEMON;
	if (strcaseeq(name, "ftp")) return LOG_FTP;
	if (strcaseeq(name, "install")) return LOG_INSTALL;
	if (strcaseeq(name, "kern")) return LOG_KERN;
	if (strcaseeq(name, "lpr")) return LOG_LPR;
	if (strcaseeq(name, "mail")) return LOG_MAIL;
	if (strcaseeq(name, "netinfo")) return LOG_NETINFO;
	if (strcaseeq(name, "remoteauth")) return LOG_REMOTEAUTH;
	if (strcaseeq(name, "news")) return LOG_NEWS;
	if (strcaseeq(name, "security")) return LOG_AUTH;
	if (strcaseeq(name, "syslog")) return LOG_SYSLOG;
	if (strcaseeq(name, "user")) return LOG_USER;
	if (strcaseeq(name, "uucp")) return LOG_UUCP;
	if (strcaseeq(name, "local0")) return LOG_LOCAL0;
	if (strcaseeq(name, "local1")) return LOG_LOCAL1;
	if (strcaseeq(name, "local2")) return LOG_LOCAL2;
	if (strcaseeq(name, "local3")) return LOG_LOCAL3;
	if (strcaseeq(name, "local4")) return LOG_LOCAL4;
	if (strcaseeq(name, "local5")) return LOG_LOCAL5;
	if (strcaseeq(name, "local6")) return LOG_LOCAL6;
	if (strcaseeq(name, "local7")) return LOG_LOCAL7;
	if (strcaseeq(name, "launchd")) return LOG_LAUNCHD;

	return -1;
}

const char *
asl_syslog_faciliy_num_to_name(int n)
{
	if (n < 0) return NULL;

	if (n == LOG_AUTH) return "auth";
	if (n == LOG_AUTHPRIV) return "authpriv";
	if (n == LOG_CRON) return "cron";
	if (n == LOG_DAEMON) return "daemon";
	if (n == LOG_FTP) return "ftp";
	if (n == LOG_INSTALL) return "install";
	if (n == LOG_KERN) return "kern";
	if (n == LOG_LPR) return "lpr";
	if (n == LOG_MAIL) return "mail";
	if (n == LOG_NETINFO) return "netinfo";
	if (n == LOG_REMOTEAUTH) return "remoteauth";
	if (n == LOG_NEWS) return "news";
	if (n == LOG_AUTH) return "security";
	if (n == LOG_SYSLOG) return "syslog";
	if (n == LOG_USER) return "user";
	if (n == LOG_UUCP) return "uucp";
	if (n == LOG_LOCAL0) return "local0";
	if (n == LOG_LOCAL1) return "local1";
	if (n == LOG_LOCAL2) return "local2";
	if (n == LOG_LOCAL3) return "local3";
	if (n == LOG_LOCAL4) return "local4";
	if (n == LOG_LOCAL5) return "local5";
	if (n == LOG_LOCAL6) return "local6";
	if (n == LOG_LOCAL7) return "local7";
	if (n == LOG_LAUNCHD) return "launchd";

	return NULL;
}

static xpc_connection_t
_create_aslmanager_connection(void)
{
	xpc_connection_t connection;

	connection = xpc_connection_create_mach_service(ASLMANAGER_SERVICE_NAME, NULL, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
	xpc_connection_set_event_handler(connection, ^(xpc_object_t xobj) { if (xobj != NULL) {}; });
	xpc_connection_resume(connection);

	return connection;
}

int
asl_trigger_aslmanager(void)
{
	xpc_connection_t connection = _create_aslmanager_connection();
	if (connection == NULL) return -1;

	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	if (request == NULL) return -1;

	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, request);

	if (reply != NULL) xpc_release(reply);
	xpc_release(request);
	xpc_release(connection);
	return 0;
}
