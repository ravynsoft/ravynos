/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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

#ifndef __ASL_MSG_H__
#define __ASL_MSG_H__

#include <stdint.h>
#include <xpc/xpc.h>
#include <asl.h>
#include <asl_string.h>
#include <asl_core.h>
#include <asl_object.h>

#define IndexNull ((uint32_t)-1)

#define ASL_MSG_PAGE_DATA_SIZE 220

#define ASL_MSG_KVO_COUNT 30
// ASL_MSG_KVO_QUERY_SLOTS = ASL_MSG_KVO_COUNT / 3;
#define ASL_MSG_KVO_QUERY_SLOTS 10
// ASL_MSG_KVO_MSG_SLOTS = ASL_MSG_KVO_COUNT / 2;
#define ASL_MSG_KVO_MSG_SLOTS 15

#define ASL_MSG_OFFSET_MASK   0x3fff
#define ASL_MSG_KV_MASK       0xc000
#define ASL_MSG_KV_INLINE     0x0000
#define ASL_MSG_KV_DICT       0x8000
#define ASL_MSG_KV_EXTERN     0x4000

#define ASL_MSG_SLOT_FREE     0xffff

#define ASL_STD_KEY_BASE      0x8000
#define ASL_STD_KEY_TIME      0x8001
#define ASL_STD_KEY_NANO      0x8002
#define ASL_STD_KEY_HOST      0x8003
#define ASL_STD_KEY_SENDER    0x8004
#define ASL_STD_KEY_FACILITY  0x8005
#define ASL_STD_KEY_PID       0x8006
#define ASL_STD_KEY_UID       0x8007
#define ASL_STD_KEY_GID       0x8008
#define ASL_STD_KEY_LEVEL     0x8009
#define ASL_STD_KEY_MESSAGE   0x800a
#define ASL_STD_KEY_READ_UID  0x800b
#define ASL_STD_KEY_READ_GID  0x800c
#define ASL_STD_KEY_SESSION   0x800d
#define ASL_STD_KEY_REF_PID   0x800e
#define ASL_STD_KEY_REF_PROC  0x800f
#define ASL_STD_KEY_MSG_ID    0x8010
#define ASL_STD_KEY_EXPIRE    0x8011
#define ASL_STD_KEY_OPTION    0x8012
#define ASL_STD_KEY_FREE_NOTE 0x8013
#define ASL_STD_KEY_LAST ASL_STD_KEY_FREE_NOTE

#define ASL_MT_KEY_BASE       0x8100
#define ASL_MT_KEY_DOMAIN     0x8101
#define ASL_MT_KEY_SCOPE      0x8102
#define ASL_MT_KEY_RESULT     0x8103
#define ASL_MT_KEY_SIG        0x8104
#define ASL_MT_KEY_SIG2       0x8105
#define ASL_MT_KEY_SIG3       0x8106
#define ASL_MT_KEY_SUCCESS    0x8107
#define ASL_MT_KEY_UUID       0x8108
#define ASL_MT_KEY_VAL        0x8109
#define ASL_MT_KEY_VAL2       0x810a
#define ASL_MT_KEY_VAL3       0x810b
#define ASL_MT_KEY_VAL4       0x810c
#define ASL_MT_KEY_VAL5       0x810d
#define ASL_MT_KEY_LAST ASL_MT_KEY_VAL5

#define ASL_PRIVATE_KEY_BASE  0x8200

typedef struct asl_msg_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint32_t count;
	uint32_t data_size;
	uint64_t mem_size;
	struct asl_msg_s *next;
#ifndef __LP64__
	uint32_t pad;
#endif
	uint16_t kvo[ASL_MSG_KVO_COUNT];
	char data[ASL_MSG_PAGE_DATA_SIZE];
} asl_msg_t;

__BEGIN_DECLS

const asl_jump_table_t *asl_msg_jump_table(void);

/* new/retain/release */
asl_msg_t *asl_msg_new(uint32_t type) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_retain(asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_release(asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

int asl_msg_set_key_val(asl_msg_t *msg, const char *key, const char *val) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
int asl_msg_set_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint32_t op) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_unset(asl_msg_t *msg, const char *key) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_unset_index(asl_msg_t *msg, uint32_t n) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_msg_t *asl_msg_copy(asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_merge(asl_msg_t *target, asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

int asl_msg_lookup(asl_msg_t *msg, const char *key, const char **valout, uint16_t *opout) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_msg_fetch(asl_msg_t *msg, uint32_t n, const char **keyout, const char **valout, uint16_t *opout) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
const char *asl_msg_get_val_for_key(asl_msg_t *msg, const char *key) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

uint32_t asl_msg_type(asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_msg_count(asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

char *asl_msg_to_string(asl_msg_t *in, uint32_t *len) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_from_string(const char *buf) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

char *asl_format_message(asl_msg_t *msg, const char *msg_fmt, const char *time_fmt, uint32_t text_encoding, uint32_t *outlen) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_string_t *asl_msg_to_string_raw(uint32_t encoding, asl_msg_t *msg, const char *tfmt) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t * asl_string_append_asl_msg(asl_string_t *str, asl_msg_t *msg) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

int asl_msg_cmp(asl_msg_t *a, asl_msg_t *b) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

void _asl_log_args_to_xpc(asl_object_t client, asl_object_t msg, xpc_object_t dict); //TODO: ADD AVAILABLITY INFO

__END_DECLS

#endif /* __ASL_MSG_H__ */
