/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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

#ifndef __ASL_MEMORY_H__
#define __ASL_MEMORY_H__
#include <stdint.h>
#include <asl.h>

typedef struct
{
	uint32_t hash;
	uint32_t refcount;
	char str[];
} mem_string_t;

typedef struct
{
	uint64_t mid;
	uint64_t time;
	uint32_t nano;
	uint8_t unused_0;
	uint8_t level;
	uint16_t flags;
	uint32_t pid;
	uint32_t uid;
	uint32_t gid;
	uint32_t ruid;
	uint32_t rgid;
	uint32_t refpid;
	uint64_t os_activity_id;
	uint32_t kvcount;
	mem_string_t *host;
	mem_string_t *sender;
	mem_string_t *sender_mach_uuid;
	mem_string_t *facility;
	mem_string_t *message;
	mem_string_t *refproc;
	mem_string_t *session;
	mem_string_t **kvlist;
} mem_record_t;

typedef struct
{
	uint32_t string_count;
	void **string_cache;
	uint32_t record_count;
	uint32_t record_first;
	mem_record_t **record;
	mem_record_t *buffer_record;
	uint32_t max_string_mem;
	uint32_t curr_string_mem;
} asl_memory_t;

uint32_t asl_memory_open(uint32_t max_records, size_t max_str_mem, asl_memory_t **s);
uint32_t asl_memory_close(asl_memory_t *s);
uint32_t asl_memory_statistics(asl_memory_t *s, asl_msg_t **msg);

uint32_t asl_memory_save(asl_memory_t *s, asl_msg_t *msg, uint64_t *mid);
uint32_t asl_memory_fetch(asl_memory_t *s, uint64_t mid, asl_msg_t **msg, int32_t ruid, int32_t rgid);

uint32_t asl_memory_match(asl_memory_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, int32_t ruid, int32_t rgid);
uint32_t asl_memory_match_restricted_uuid(asl_memory_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, uint32_t duration, int32_t direction, int32_t ruid, int32_t rgid, const char *uuid_str);

#endif /* __ASL_MEMORY_H__ */
