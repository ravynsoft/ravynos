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

#ifndef __ASL_FILE_H__
#define __ASL_FILE_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <asl.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <Availability.h>
#include <os/object.h>
#include <os/object_private.h>

#define DB_HEADER_LEN 80
#define DB_HEADER_COOKIE_OFFSET 0
#define DB_HEADER_VERS_OFFSET 12
#define DB_HEADER_FIRST_OFFSET 16
#define DB_HEADER_TIME_OFFSET 24
#define DB_HEADER_CSIZE_OFFSET 32
#define DB_HEADER_FILTER_MASK_OFFSET 36
#define DB_HEADER_LAST_OFFSET 37

/*
 * Magic Cookie for database files.
 * MAXIMUM 12 CHARS! (DB_HEADER_VERS_OFFSET)
 */
#define ASL_DB_COOKIE "ASL DB"
#define ASL_DB_COOKIE_LEN 6
#define DB_VERSION 2
#define DB_VERSION_LEGACY_1 1

#define ASL_FILE_FLAG_READ            0x00000001
#define ASL_FILE_FLAG_WRITE           0x00000002
#define ASL_FILE_FLAG_UNLIMITED_CACHE 0x00000004
#define ASL_FILE_FLAG_PRESERVE_MSG_ID 0x00000008
#define ASL_FILE_FLAG_LEGACY_STORE    0x00000010

#define ASL_FILE_TYPE_MSG 0
#define ASL_FILE_TYPE_STR 1

#define ASL_FILE_POSITION_FIRST 0
#define ASL_FILE_POSITION_PREVIOUS 1
#define ASL_FILE_POSITION_NEXT 2
#define ASL_FILE_POSITION_LAST 3

/* flags for asl_file_filter */
#define ASL_FILE_FILTER_FLAG_KEEP_MATCHES 0x00000001

/* NB CACHE_SIZE must be > 1 */
#define CACHE_SIZE 128

/* This makes the maximum size of a file_string_t 128 bytes */
#define CACHE_MAX_STRING_LEN 108

/* Size of the fixed-length part of a MSG record */
#define MSG_RECORD_FIXED_LENGTH 122

/*
 * The first record (header) in the database has the format:
 *
 * | 12     | 4    | 8      | 8    | 4                 | 8    | 1    | 35   | (80 bytes)
 * | Cookie | Vers | First  | Time | String cache size | Last | Mask | Zero |
 * 
 * MSG records have the format:
 *
 * | 2  | 4   | 8    | 8  | 8    | 4    | 2     | 2     | 4   | 4   | 4   | 4    | 4    | 4      | 4
 * | 00 | Len | Next | ID | Time | Nano | Level | Flags | PID | UID | GID | RUID | RGID | RefPID | KV count ...
 *
 * | 8    | 8      | 8        | 8       | 8       | 8       | 8    | 8    |     | 8
 * | Host | Sender | Facility | Message | RefProc | Session | Key0 | Val0 | ... | Previous |
 * 
 * STR records have the format:
 *
 * | 2  | 4   | Len      | (Len + 6 bytes)
 * | 01 | Len | Data+NUL | 
 * 
 */

typedef struct file_string_s
{
	uint64_t where;
	uint32_t hash;
	struct file_string_s *next;
	char str[CACHE_MAX_STRING_LEN];
} file_string_t;

typedef struct asl_file_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint32_t flags;
	uint32_t version;
	uint32_t string_cache_count;
	uint32_t msg_count;
	file_string_t *string_list;
	file_string_t *string_spare;
	uint64_t first;
	uint64_t last;
	uint64_t last_mid;
	uint64_t prev;
	uint64_t cursor;
	uint64_t cursor_xid;
	uint64_t dob;
	size_t file_size;
	FILE *store;
	void *legacy;
	char *scratch;
} asl_file_t;

typedef struct asl_file_list_s
{
	asl_file_t *file;
	struct asl_file_list_s *next;
} asl_file_list_t;

__BEGIN_DECLS

const asl_jump_table_t *asl_file_jump_table(void);

asl_file_list_t *asl_file_list_add(asl_file_list_t *list, asl_file_t *f) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_file_list_close(asl_file_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_file_t *asl_file_retain(asl_file_t *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_file_release(asl_file_t *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_file_open_write(const char *path, mode_t mode, uid_t uid, gid_t gid, asl_file_t **s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_close(asl_file_t *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_file_save(asl_file_t *s, asl_msg_t *msg, uint64_t *mid) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_file_open_read(const char *path, asl_file_t **s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_fetch(asl_file_t *s, uint64_t mid, asl_msg_t **msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_file_read_set_position(asl_file_t *s, uint32_t pos) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_fetch_next(asl_file_t *s, asl_msg_t **msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_fetch_previous(asl_file_t *s, asl_msg_t **msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_msg_list_t *asl_file_match(asl_file_t *s, asl_msg_list_t *query, uint64_t *last, uint64_t start, uint32_t count, uint32_t duration, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_list_t *asl_file_list_match(asl_file_list_t *list, asl_msg_list_t *query, uint64_t *last, uint64_t start, uint32_t count, uint32_t duration, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

void *asl_file_list_match_start(asl_file_list_t *list, uint64_t start_id, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_list_match_next(void *token, asl_msg_list_t *query, asl_msg_list_t **res, uint32_t count) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_file_list_match_end(void *token) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

size_t asl_file_size(asl_file_t *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint64_t asl_file_ctime(asl_file_t *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_file_compact(asl_file_t *s, const char *path, mode_t mode, uid_t uid, gid_t gid) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_filter(asl_file_t *s, const char *path, asl_msg_list_t *filter, uint32_t flags, mode_t mode, uid_t uid, gid_t gid, uint32_t *dstcount, void (*aux_callback)(const char *auxfile)) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_file_filter_level(asl_file_t *s, const char *path, uint32_t keep_mask, mode_t mode, uid_t uid, gid_t gid, uint32_t *dstcount, void (*aux_callback)(const char *auxfile)) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

__END_DECLS

#endif /* __ASL_FILE_H__ */
