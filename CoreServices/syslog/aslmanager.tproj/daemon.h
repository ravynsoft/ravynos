/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <asl_msg_list.h>
#include "asl_common.h"

#define DEFAULT_MAX_SIZE 150000000
#define IOBUFSIZE 4096

#define DO_ASLDB	0x00000001
#define DO_MODULE	0x00000002
#define DO_CHECKPT	0x00000004

#define DEBUG_FLAG_MASK  0xfffffff0
#define DEBUG_LEVEL_MASK 0x0000000f
#define DEBUG_FILE       0x00000010
#define DEBUG_STDERR     0x00000020
#define DEBUG_ASL        0x00000040

#define AUX_URL_MINE "file:///var/log/asl/"
#define AUX_URL_MINE_LEN 20

#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_MINUTE 60

/* length of "file://" */
#define AUX_URL_PATH_OFFSET 7

#define NAME_LIST_FLAG_COMPRESSED 0x00000001

#define DAEMON_STATE_IDLE  0x00000000
#define DAEMON_STATE_MAIN  0x00000001
#define DAEMON_STATE_CACHE 0x00000002

typedef struct name_list_s
{
	char *name;
	size_t size;
	uint32_t flags;
	struct name_list_s *next;
} name_list_t;

const char *keep_str(uint8_t mask);
void set_debug(int flag, const char *str);
void debug_log(int level, char *str, ...);
void debug_close();

name_list_t *add_to_name_list(name_list_t *l, const char *name, size_t size, uint32_t flags);
void free_name_list(name_list_t *l);

int copy_compress_file(asl_out_dst_data_t *asldst, const char *src, const char *dst);

void filesystem_rename(const char *src, const char *dst);
void filesystem_unlink(const char *path);
void filesystem_truncate(const char *path);
void filesystem_rmdir(const char *path);
int32_t filesystem_copy(asl_out_dst_data_t *asldst, const char *src, const char *dst, uint32_t flags);
int32_t filesystem_reset_ctime(const char *path);

int remove_directory(const char *path);
size_t directory_size(const char *path);

time_t parse_ymd_name(const char *name);
uint32_t ymd_file_age(const char *name, time_t now, uid_t *u, gid_t *g);
uint32_t ymd_file_filter(const char *name, const char *path, uint32_t keep_mask, mode_t ymd_mode, uid_t ymd_uid, gid_t ymd_gid);

int process_asl_data_store(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts);
int module_copy_rename(asl_out_dst_data_t *dst);
int module_expire(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts);
int module_check_size(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts, bool check, size_t *msize);
int process_module(asl_out_module_t *mod, asl_out_dst_data_t *opts);
int process_dst(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts);

asl_msg_list_t * control_query(asl_msg_t *a);
int checkpoint(const char *name);

void main_task(void);

