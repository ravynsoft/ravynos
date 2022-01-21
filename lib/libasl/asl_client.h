/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef __ASL_CLIENT_H__
#define __ASL_CLIENT_H__

#include <stdint.h>
#include <asl.h>
#include <asl_file.h>
#include <asl_store.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <Availability.h>
#include <asl_object.h>

#define CLIENT_FLAG_WRITE_SYS   0x00000001
#define CLIENT_FLAG_WRITE_STORE 0x00000002
#define CLIENT_FLAG_WRITE_FILE  0x00000004
#define CLIENT_FLAG_READ_SYS    0x00000100
#define CLIENT_FLAG_READ_STORE  0x00000200
#define CLIENT_FLAG_READ_FILE   0x00000400

typedef struct
{
	int fd;
	uint32_t encoding;
	uint32_t filter;
	char *mfmt;
	char *tfmt;
} asl_out_file_t;

typedef struct asl_client_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint32_t flags;
	uint32_t options;
	pid_t pid;
	uid_t uid;
	gid_t gid;
	asl_msg_t *kvdict;
	uint32_t filter;
	int notify_token;
	int notify_master_token;
	uint32_t out_count;
	asl_out_file_t *out_list;
	asl_file_t *aslfile;
	uint64_t aslfileid;
	asl_store_t *store;
	uint32_t reserved1;
	void *reserved2;
} asl_client_t;

__BEGIN_DECLS

const asl_jump_table_t *asl_client_jump_table(void);

asl_client_t *asl_client_open(const char *ident, const char *facility, uint32_t opts) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
asl_client_t *asl_client_open_from_file(int descriptor, const char *ident, const char *facility) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
asl_client_t *asl_client_retain(asl_client_t *client) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
void asl_client_release(asl_client_t *client) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

int asl_client_set_filter(asl_client_t *client, int filter) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
ASL_STATUS asl_client_add_output_file(asl_client_t *client, int descriptor, const char *mfmt, const char *tfmt, int filter, int text_encoding) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
int asl_client_set_output_file_filter(asl_client_t *client, int fd, int filter) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
ASL_STATUS asl_client_remove_output_file(asl_client_t *client, int descriptor) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

int asl_client_log_descriptor(asl_client_t *client, asl_msg_t *msg, int level, int descriptor, uint32_t fd_type) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

asl_msg_t *asl_client_kvdict(asl_client_t *client) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

int asl_client_log(asl_client_t *client, asl_msg_t *msg, int level, const char *format, ...) __printflike(4, 5) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
int asl_client_vlog(asl_client_t *client, asl_msg_t *msg, int level, const char *format, va_list ap) __printflike(4, 0) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
ASL_STATUS asl_client_send(asl_client_t *client, asl_msg_t *msg) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

asl_msg_list_t *asl_client_search(asl_client_t *client, asl_msg_t *query) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
asl_msg_list_t *asl_client_match(asl_client_t *client, asl_msg_list_t *querylist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

__END_DECLS

#endif /* __ASL_CLIENT_H__ */
