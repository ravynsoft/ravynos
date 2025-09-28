/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef __FreeBSD__
#include <atomic_compat.h>
#else
#include <libkern/OSAtomic.h>
#endif
//#include <crt_externs.h>
#include <asl.h>
#include <asl_private.h>
#include <asl_ipc.h>
#include <asl_core.h>
#include <asl_client.h>

#define PUBLIC_OPT_MASK 0x000000ff

/* private asl_file SPI */
__private_extern__ ASL_STATUS asl_file_open_write_fd(int descriptor, asl_file_t **s);

/* private asl SPI */
__private_extern__ ASL_STATUS asl_client_internal_send(asl_object_t client, asl_object_t msg);
__private_extern__ char *get_argv0();

#pragma mark -
#pragma mark asl_client_t

static void
_asl_client_free_internal(asl_client_t *client)
{
	uint32_t i;

	if (client == NULL) return;

	if (client->kvdict != NULL) asl_msg_release(client->kvdict);
	client->kvdict = NULL;

	if (client->aslfile != NULL) asl_file_close(client->aslfile);
	client->aslfile = NULL;

	for (i = 0; i < client->out_count; i++)
	{
		free(client->out_list[i].mfmt);
		free(client->out_list[i].tfmt);
	}

	free(client->out_list);
	client->out_list = NULL;

	free(client);
}

asl_client_t *
asl_client_open(const char *ident, const char *facility, uint32_t opts)
{
	asl_client_t *client = (asl_client_t *)calloc(1, sizeof(asl_client_t));
	if (client == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	client->asl_type = ASL_TYPE_CLIENT;
	client->refcount = 1;

	client->kvdict = asl_msg_new(ASL_TYPE_MSG);
	if (client->kvdict == NULL)
	{
		asl_client_release(client);
		errno = ENOMEM;
		return NULL;
	}

	client->options = opts & PUBLIC_OPT_MASK;

	client->pid = getpid();
	client->uid = getuid();
	client->gid = getgid();

	if (ident != NULL)
	{
		asl_msg_set_key_val(client->kvdict, ASL_KEY_SENDER, ident);
	}
	else
	{
		char *name = get_argv0();
		if (name != NULL)
		{
			char *x = strrchr(name, '/');
			if (x != NULL) x++;
			else x = name;
			asl_msg_set_key_val(client->kvdict, ASL_KEY_SENDER, x);
		}
	}

	if (facility != NULL)
	{
		asl_msg_set_key_val(client->kvdict, ASL_KEY_FACILITY, facility);
	}
	else if (client->uid == 0)
	{
		asl_msg_set_key_val(client->kvdict, ASL_KEY_FACILITY, asl_syslog_faciliy_num_to_name(LOG_DAEMON));
	}
	else
	{
		asl_msg_set_key_val(client->kvdict, ASL_KEY_FACILITY, asl_syslog_faciliy_num_to_name(LOG_USER));
	}

	client->filter = ASL_FILTER_MASK_UPTO(ASL_LEVEL_NOTICE);

	if (client->options & ASL_OPT_STDERR)
	{
		/* only add stderr if it is valid */
		if (fcntl(STDERR_FILENO, F_GETFD) >= 0)
		{
			asl_client_add_output_file(client, fileno(stderr), ASL_MSG_FMT_STD, ASL_TIME_FMT_LCL, ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG), ASL_ENCODE_SAFE);
		}
		else
		{
			/* stderr has been closed, ignore ASL_OPT_STDERR flag */
			client->options &= ~ASL_OPT_STDERR;
		}
	}

	return client;
}

asl_client_t *
asl_client_open_from_file(int descriptor, const char *ident, const char *facility)
{
	uint32_t status;
	asl_client_t *client = asl_client_open(ident, facility, 0);
	if (client == NULL) return NULL;

	client->filter = ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG);

	status = asl_file_open_write_fd(descriptor, &(client->aslfile));
	if (status != ASL_STATUS_OK)
	{
		_asl_client_free_internal(client);
		return NULL;
	}

	client->aslfileid = 1;

	return client;
}

asl_client_t *
asl_client_retain(asl_client_t *client)
{
	if (client == NULL) return NULL;
	asl_retain((asl_object_t)client);
	return client;
}

void
asl_client_release(asl_client_t *client)
{
	if (client == NULL) return;
	asl_release((asl_object_t)client);
}

#pragma mark -
#pragma mark database access

ASL_STATUS
asl_client_send(asl_client_t *client, asl_msg_t *msg)
{
	return asl_client_internal_send((asl_object_t)client, (asl_object_t)msg);
}

static asl_msg_list_t *
_do_server_match(asl_msg_list_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int dir)
{
	char *str, *res = NULL;
	uint32_t len, reslen, status;
	uint64_t last64, start64, count64;
	kern_return_t kstatus;
	asl_msg_list_t *out;
	caddr_t vmstr;
	mach_port_t asl_server_port = asl_core_get_service_port(0);

	if (asl_server_port == MACH_PORT_NULL) return NULL;

	str = NULL;
	if (qlist == NULL)
	{
		asprintf(&str, "0\n");
		len = 3;
	}
	else
	{
		str = asl_msg_list_to_string(qlist, &len);
	}

	if (str == NULL) return NULL;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmstr, len, TRUE);
	if (kstatus != KERN_SUCCESS) return NULL;

	memmove(vmstr, str, len);
	free(str);

	last64 = 0;
	start64 = start;
	count64 = count;

	kstatus = _asl_server_match(asl_server_port, vmstr, len, start64, count64, duration, dir, (caddr_t *)&res, &reslen, &last64, (int *)&status);
	*last = last64;

	out = asl_msg_list_from_string(res);
	vm_deallocate(mach_task_self(), (vm_address_t)res, reslen);

	return out;
}

static asl_msg_list_t *
_do_server_search(asl_msg_t *q)
{
	asl_msg_list_t *out;
	char *qstr, *str, *res = NULL;
	uint32_t len, reslen = 0, status = ASL_STATUS_OK;
	uint64_t cmax = 0;
	kern_return_t kstatus;
	caddr_t vmstr;
	mach_port_t asl_server_port = asl_core_get_service_port(0);

	if (asl_server_port == MACH_PORT_NULL) return NULL;

	len = 0;
	qstr = asl_msg_to_string(q, &len);

	str = NULL;
	if (qstr == NULL)
	{
		asprintf(&str, "0\n");
		len = 3;
	}
	else
	{
		asprintf(&str, "1\n%s\n", qstr);
		len += 3;
		free(qstr);
	}

	if (str == NULL) return NULL;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmstr, len, TRUE);
	if (kstatus != KERN_SUCCESS) return NULL;

	memmove(vmstr, str, len);
	free(str);

	kstatus = _asl_server_query_2(asl_server_port, vmstr, len, 0, 0, 0, (caddr_t *)&res, &reslen, &cmax, (int *)&status);
	if (kstatus != KERN_SUCCESS) return NULL;

	out = asl_msg_list_from_string(res);
	vm_deallocate(mach_task_self(), (vm_address_t)res, reslen);

	return out;
}

static asl_msg_list_t *
_do_store_match(asl_msg_list_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction)
{
	asl_msg_list_t *out;
	uint32_t status;
	uint64_t l64 = 0, s64;
	asl_store_t *store = NULL;

	uint32_t len;
	char *str = asl_msg_list_to_string(qlist, &len);
	free(str);

	status = asl_store_open_read(NULL, &store);
	if (status != 0) return NULL;
	if (store == NULL) return NULL;

	s64 = start;
	out = asl_store_match(store, qlist, &l64, s64, count, duration, direction);
	*last = l64;

	asl_store_close(store);

	return out;
}

static asl_msg_list_t *
_do_store_search(asl_msg_t *query)
{
	asl_msg_list_t *out, *qlist = NULL;
	uint32_t status;
	uint16_t op;
	uint64_t last = 0, start = 0;
	asl_store_t *store = NULL;
	const char *val = NULL;

	/* check for "ASLMessageId >[=] n" and set start_id */
	status = asl_msg_lookup(query, ASL_KEY_MSG_ID, &val, &op);
	if ((status == 0) && (val != NULL) && (op & ASL_QUERY_OP_GREATER))
	{
		if (op & ASL_QUERY_OP_EQUAL) start = atoll(val);
		else start = atoll(val) + 1;
	}

	status = asl_store_open_read(NULL, &store);
	if (status != 0) return NULL;
	if (store == NULL) return NULL;

	if (query != NULL)
	{
		qlist = asl_msg_list_new();
		asl_msg_list_append(qlist, query);
	}

	out = asl_store_match(store, qlist, &last, start, 0, 0, 1);
	asl_store_close(store);

	asl_msg_list_release(qlist);
	return out;
}

asl_msg_list_t *
asl_client_match(asl_client_t *client, asl_msg_list_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction)
{
	if (asl_store_location() == ASL_STORE_LOCATION_FILE) return _do_store_match(qlist, last, start, count, duration, direction);
	return _do_server_match(qlist, last, start, count, duration, direction);
}

asl_msg_list_t *
asl_client_search(asl_client_t *client, asl_msg_t *query)
{
	if (asl_store_location() == ASL_STORE_LOCATION_FILE) return _do_store_search(query);
	return _do_server_search(query);
}


#pragma mark -
#pragma mark output control

/* returns last filter value, or -1 on error */
int
asl_client_set_filter(asl_client_t *client, int filter)
{
	int last;

	if (client == NULL) return -1;
	last = client->filter;
	client->filter = filter;
	return last;
}

ASL_STATUS
asl_client_add_output_file(asl_client_t *client, int descriptor, const char *mfmt, const char *tfmt, int filter, int text_encoding)
{
	uint32_t i;

	if (client == NULL) return ASL_STATUS_FAILED;

	for (i = 0; i < client->out_count; i++)
	{
		if (client->out_list[i].fd == descriptor)
		{
			/* update message format, time format, filter, and text encoding */
			free(client->out_list[i].mfmt);
			client->out_list[i].mfmt = NULL;
			if (mfmt != NULL) client->out_list[i].mfmt = strdup(mfmt);

			free(client->out_list[i].tfmt);
			client->out_list[i].tfmt = NULL;
			if (tfmt != NULL) client->out_list[i].tfmt = strdup(tfmt);

			client->out_list[i].encoding = text_encoding;
			client->out_list[i].filter = filter;

			return ASL_STATUS_OK;
		}
	}

	if (client->out_count == 0) client->out_list = NULL;
	client->out_list = (asl_out_file_t *)reallocf(client->out_list, (1 + client->out_count) * sizeof(asl_out_file_t));

	if (client->out_list == NULL) return ASL_STATUS_FAILED;

	client->out_list[client->out_count].fd = descriptor;
	client->out_list[client->out_count].encoding = text_encoding;
	client->out_list[client->out_count].filter = filter;
	if (mfmt != NULL) client->out_list[client->out_count].mfmt = strdup(mfmt);
	if (tfmt != NULL) client->out_list[client->out_count].tfmt = strdup(tfmt);

	client->out_count++;

	return ASL_STATUS_OK;
}

/* returns last filter value, or -1 on error */
int
asl_client_set_output_file_filter(asl_client_t *client, int descriptor, int filter)
{
	uint32_t i;
	int last = 0;

	if (client == NULL) return -1;

	for (i = 0; i < client->out_count; i++)
	{
		if (client->out_list[i].fd == descriptor)
		{
			/* update filter */
			last = client->out_list[i].filter;
			client->out_list[i].filter = filter;
			break;
		}
	}

	return last;
}

ASL_STATUS
asl_client_remove_output_file(asl_client_t *client, int descriptor)
{
	uint32_t i;
	int x;

	if (client == NULL) return ASL_STATUS_INVALID_ARG;

	if (client->out_count == 0) return ASL_STATUS_OK;

	x = -1;
	for (i = 0; i < client->out_count; i++)
	{
		if (client->out_list[i].fd == descriptor)
		{
			x = i;
			break;
		}
	}

	if (x == -1) return ASL_STATUS_OK;

	free(client->out_list[x].mfmt);
	free(client->out_list[x].tfmt);

	for (i = x + 1; i < client->out_count; i++, x++)
	{
		client->out_list[x] = client->out_list[i];
	}

	client->out_count--;

	if (client->out_count == 0)
	{
		free(client->out_list);
		client->out_list = NULL;
	}
	else
	{
		client->out_list = (asl_out_file_t *)reallocf(client->out_list, client->out_count * sizeof(asl_out_file_t));

		if (client->out_list == NULL)
		{
			client->out_count = 0;
			return ASL_STATUS_FAILED;
		}
	}

	return ASL_STATUS_OK;
}

#pragma mark -
#pragma mark dictionary access

asl_msg_t *
asl_client_kvdict(asl_client_t *client)
{
	if (client == NULL) return NULL;
	return client->kvdict;
}

#pragma mark -
#pragma mark asl_object support

static void
_jump_dealloc(asl_object_private_t *obj)
{
	_asl_client_free_internal((asl_client_t *)obj);
}

static void
_jump_append(asl_object_private_t *obj, asl_object_private_t *newobj)
{
	int type = asl_get_type((asl_object_t)newobj);

	if (type == ASL_TYPE_LIST)
	{
		asl_msg_t *msg;
		asl_msg_list_reset_iteration((asl_msg_list_t *)newobj, 0);
		while (NULL != (msg = asl_msg_list_next((asl_msg_list_t *)newobj)))
		{
			if (asl_client_internal_send((asl_object_t)obj, (asl_object_t)msg) != ASL_STATUS_OK) return;
		}
	}
	else if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		asl_client_internal_send((asl_object_t)obj, (asl_object_t)newobj);
	}
}

static asl_object_private_t *
_jump_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	int type = asl_get_type((asl_object_t)query);
	if ((query != NULL) && (type != ASL_TYPE_MSG) && (type != ASL_TYPE_QUERY)) return NULL;

	return (asl_object_private_t *)asl_client_search((asl_client_t *)obj, (asl_msg_t *)query);
}

static asl_object_private_t *
_jump_match(asl_object_private_t *obj, asl_object_private_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	asl_msg_list_t *out = NULL;
	int type = asl_get_type((asl_object_t)qlist);

	if ((qlist != NULL) && (type != ASL_TYPE_LIST)) return NULL;

	out = asl_client_match((asl_client_t *)obj, (asl_msg_list_t *)qlist, last, start, count, duration, dir);
	return (asl_object_private_t *)out;
}

__private_extern__ const asl_jump_table_t *
asl_client_jump_table()
{
	static const asl_jump_table_t jump =
	{
		.alloc = NULL,
		.dealloc = &_jump_dealloc,
		.set_key_val_op = NULL,
		.unset_key = NULL,
		.get_val_op_for_key = NULL,
		.get_key_val_op_at_index = NULL,
		.count = NULL,
		.next = NULL,
		.prev = NULL,
		.get_object_at_index = NULL,
		.set_iteration_index = NULL,
		.remove_object_at_index = NULL,
		.append = &_jump_append,
		.prepend = NULL,
		.search = &_jump_search,
		.match = &_jump_match
	};

	return &jump;
}

