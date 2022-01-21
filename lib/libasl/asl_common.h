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

#ifndef __ASL_COMMON_H__
#define __ASL_COMMON_H__

#include <stdio.h>
#include <xpc/xpc.h>

#define ASL_MODULE_NAME "com.apple.asl"
#define _PATH_CRASHREPORTER "/Library/Logs/CrashReporter"
#define _PATH_CRASHREPORTER_MOBILE "/var/mobile/Library/Logs/CrashReporter"

#define ASL_SERVICE_NAME "com.apple.system.logger"

#define CRASH_MOVER_WILL_START_NOTIFICATION  "CrashMoverWillStart"

#define DEFAULT_TTL 7 /* days */
#define SECONDS_PER_DAY 86400

#define ACTION_NONE       0
#define ACTION_SET_PARAM  1
#define ACTION_OUT_DEST   2
#define ACTION_IGNORE     3
#define ACTION_SKIP       4
#define ACTION_CLAIM      5
#define ACTION_NOTIFY     6
#define ACTION_BROADCAST  7
#define ACTION_ACCESS     8
#define ACTION_SET_KEY    9
#define ACTION_UNSET_KEY 10
#define ACTION_ASL_STORE 11 /* Save in main ASL Database */
#define ACTION_ASL_FILE  12 /* Save in an ASL format data file */
#define ACTION_ASL_DIR   13 /* Save in an ASL directory */
#define ACTION_FILE      14 /* Save in a text file */
#define ACTION_FORWARD   15
#define ACTION_CONTROL   16
#define ACTION_SET_FILE  17 /* = foo [File /a/b/c] */
#define ACTION_SET_PLIST 18 /* = foo [Plist /a/b/c] ... */
#define ACTION_SET_PROF  19 /* = foo [Profile abc] ... */

#define STYLE_SEC_PREFIX_CHAR 'T'

#define MODULE_FLAG_HAS_LOGGED   0x80000000
#define MODULE_FLAG_CLEAR_LOGGED 0x7fffffff
#define MODULE_FLAG_ENABLED      0x00000001
#define MODULE_FLAG_LOCAL        0x00000002
#define MODULE_FLAG_ROTATE       0x00000004
#define MODULE_FLAG_COALESCE     0x00000008
#define MODULE_FLAG_COMPRESS     0x00000010
#define MODULE_FLAG_NONSTD_DIR   0x00000020
#define MODULE_FLAG_EXTERNAL     0x00000040
#define MODULE_FLAG_TRUNCATE     0x00000080
#define MODULE_FLAG_STYLE_SEC    0x00000100 /* foo.T1332799722 (note STYLE_SEC_PREFIX_CHAR) */
#define MODULE_FLAG_STYLE_SEQ    0x00000200 /* foo.0 */
#define MODULE_FLAG_STYLE_UTC    0x00000400 /* foo.2012-04-06T13:45:00Z */
#define MODULE_FLAG_STYLE_UTC_B  0x00000800 /* ("basic utc") foo.20120406T134500Z */
#define MODULE_FLAG_STYLE_LCL    0x00001000 /* foo.2012-04-06T13:45:00-7 */
#define MODULE_FLAG_STYLE_LCL_B  0x00002000 /* ("basic local") foo.20120406T134500-7 */
#define MODULE_FLAG_BASESTAMP    0x00004000 /* base file has timestamp */
#define MODULE_FLAG_CRASHLOG     0x00008000 /* checkpoint on CrashMoverWillStart notification */
#define MODULE_FLAG_SOFT_WRITE   0x00010000 /* ignore write failures */
#define MODULE_FLAG_TYPE_ASL     0x00020000 /* asl format file */
#define MODULE_FLAG_TYPE_ASL_DIR 0x00040000 /* asl format directory */
#define MODULE_FLAG_STD_BSD_MSG  0x00080000 /* print format is std, bsd, or msg */

#define MODULE_FLAG_STYLE_BITS   (MODULE_FLAG_STYLE_SEC | MODULE_FLAG_STYLE_SEQ | MODULE_FLAG_STYLE_UTC | MODULE_FLAG_STYLE_UTC_B | MODULE_FLAG_STYLE_LCL | MODULE_FLAG_STYLE_LCL_B)
#define CHECKPOINT_TEST   0x00000000
#define CHECKPOINT_FORCE  0x00000001
#define CHECKPOINT_CRASH  0x00000002

#define LEVEL_ALL 8

typedef struct
{
	char *path;
	char *fname;
	char *fmt;
	const char *tfmt;
	char *rotate_dir;
	uint32_t pvt_flags;
	uint32_t flags;
	uint32_t fails;
	uint32_t ttl[9];
	mode_t mode;
#if !TARGET_IPHONE_SIMULATOR
	uid_t *uid;
	uint32_t nuid;
	gid_t *gid;
	uint32_t ngid;
#endif
	size_t file_max;
	size_t all_max;
	uint32_t refcount;
	time_t stamp;
	size_t size;
	void *private;
} asl_out_dst_data_t;

typedef struct asl_out_rule_s
{
	asl_msg_t *query;
	uint32_t action;
	char *options;
	asl_out_dst_data_t *dst;
	void *private;
	struct asl_out_rule_s *next;
} asl_out_rule_t;

typedef struct asl_out_module_s
{
	char *name;
	uint32_t flags;
	asl_out_rule_t *ruleset;
	struct asl_out_module_s *next;
} asl_out_module_t;

typedef struct asl_out_file_list_s
{
	char *name;
	time_t ftime;
	uint32_t seq;
	size_t size;
	struct asl_out_file_list_s *next;
	struct asl_out_file_list_s *prev;
} asl_out_file_list_t;

char **explode(const char *s, const char *delim);
void free_string_list(char **l);
char *get_line_from_file(FILE *f);
char *next_word_from_string(char **s);
size_t asl_str_to_size(char *s);
asl_msg_t *xpc_object_to_asl_msg(xpc_object_t xobj);

int asl_check_option(asl_msg_t *msg, const char *opt);

/* ASL OUT MODULES */
asl_out_module_t *asl_out_module_new(const char *name);
void asl_out_module_free(asl_out_module_t *m);
asl_out_module_t *asl_out_module_init(void);
asl_out_module_t *asl_out_module_init_from_file(const char *name, FILE *f);
asl_out_rule_t *asl_out_module_parse_line(asl_out_module_t *m, char *s);

void asl_out_module_print(FILE *f, asl_out_module_t *m);
char *asl_out_module_rule_to_string(asl_out_rule_t *r);

int asl_out_mkpath(asl_out_module_t *mlist, asl_out_rule_t *r);
int asl_out_dst_checkpoint(asl_out_dst_data_t *dst, uint32_t force);
int asl_out_dst_file_create_open(asl_out_dst_data_t *dst, char **pathp);
int asl_out_dst_set_access(int fd, asl_out_dst_data_t *dst);
void asl_make_timestamp(time_t stamp, uint32_t flags, char *buf, size_t len);
void asl_make_dst_filename(asl_out_dst_data_t *dst, char *buf, size_t len);

asl_out_dst_data_t *asl_out_dst_data_retain(asl_out_dst_data_t *dst);
void asl_out_dst_data_release(asl_out_dst_data_t *dst);

/* rotated log files */
asl_out_file_list_t *asl_list_log_files(const char *dir, const char *base, bool src, uint32_t flags);
asl_out_file_list_t * asl_list_src_files(asl_out_dst_data_t *dst);
asl_out_file_list_t * asl_list_dst_files(asl_out_dst_data_t *dst);
void asl_out_file_list_free(asl_out_file_list_t *l);

asl_msg_t *configuration_profile_to_asl_msg(const char *ident);

int asl_secure_chown_chmod_dir(const char *path, uid_t uid, gid_t gid, mode_t mode);

#endif /* __ASL_COMMON_H__ */
