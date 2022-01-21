/*
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#ifndef __ASL_STORE_H__
#define __ASL_STORE_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <asl.h>
#include <asl_file.h>
#include <asl_core.h>
#include <Availability.h>
#include <os/object.h>
#include <os/object_private.h>
#include <TargetConditionals.h>

#define PATH_ASL_STORE (asl_filesystem_path(ASL_PLACE_DATABASE))
#define PATH_ASL_ARCHIVE (asl_filesystem_path(ASL_PLACE_ARCHIVE))

#define FILE_ASL_STORE_DATA "StoreData"

#define ASL_STORE_FLAG_NO_ACLS 0x00000001
#define ASL_STORE_FLAG_NO_TTL  0x00000002

#define FILE_CACHE_SIZE 64
#define FILE_CACHE_TTL 300

typedef struct
{
	time_t ts;
	uid_t u;
	gid_t g;
	time_t bb;
	char *path;
	asl_file_t *f;
} asl_cached_file_t;

typedef struct asl_store_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint64_t curr;
	char *base_dir;
	FILE *storedata;
	uint64_t next_id;
	asl_cached_file_t file_cache[FILE_CACHE_SIZE];
	void *work;
	time_t start_today;
	time_t start_tomorrow;
	time_t last_write;
	size_t max_file_size;
	uint32_t flags;
} asl_store_t;

__BEGIN_DECLS

const asl_jump_table_t *asl_store_jump_table(void);

uint32_t asl_store_open_write(const char *basedir, asl_store_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_open_read(const char *basedir, asl_store_t **s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_close(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
asl_store_t *asl_store_retain(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
void asl_store_release(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
uint32_t asl_store_statistics(asl_store_t *s, asl_msg_t **msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_set_flags(asl_store_t *s, uint32_t flags) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

uint32_t asl_store_save(asl_store_t *s, asl_msg_t *msg) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

asl_msg_list_t *asl_store_match(asl_store_t *s, asl_msg_list_t *query, uint64_t *last, uint64_t start, uint32_t count, uint32_t duration, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_match_start(asl_store_t *s, uint64_t start_id, int32_t direction) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_match_next(asl_store_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint32_t count) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

uint32_t asl_store_max_file_size(asl_store_t *s, size_t max) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
uint32_t asl_store_sweep_file_cache(asl_store_t *s) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_3_2);

uint32_t asl_store_open_aux(asl_store_t *s, asl_msg_t *msg, int *fd, char **url) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);

__END_DECLS

#endif /* __ASL_STORE_H__ */
