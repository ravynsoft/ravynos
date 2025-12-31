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

#ifndef __ASL_MSG_LIST_H__
#define __ASL_MSG_LIST_H__

#include <stdint.h>
#include <asl_core.h>
#include <asl_msg.h>
#include <os/object.h>
#include <os/object_private.h>

typedef struct asl_msg_list_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint32_t count;
	uint32_t curr;
	asl_msg_t **msg;
} asl_msg_list_t;

__BEGIN_DECLS

const asl_jump_table_t *asl_msg_list_jump_table(void);

char *asl_msg_list_to_string(asl_msg_list_t *list, uint32_t *len) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_msg_list_to_asl_string(asl_msg_list_t *list, uint32_t encoding) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_list_t *asl_msg_list_from_string(const char *buf) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_msg_list_t *asl_msg_list_new(void) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_list_t *asl_msg_list_retain(asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_list_release(asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_msg_list_t *asl_msg_list_new_count(uint32_t n) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_list_insert(asl_msg_list_t *list, uint32_t x, void *obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

void asl_msg_list_append(asl_msg_list_t *list, void *obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_list_prepend(asl_msg_list_t *list, void *obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
size_t asl_msg_list_count(asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_list_get_index(asl_msg_list_t *list, size_t index) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_list_remove_index(asl_msg_list_t *list, size_t index) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_list_next(asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_t *asl_msg_list_prev(asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_msg_list_reset_iteration(asl_msg_list_t *list, size_t position) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_list_t *asl_msg_list_search(asl_msg_list_t *list, asl_msg_t *query) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_msg_list_t *asl_msg_list_match(asl_msg_list_t *list, asl_msg_list_t *querylist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

asl_msg_list_t *asl_msg_search(asl_msg_t *msg, asl_msg_t *query) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_msg_match(asl_msg_t *msg, asl_msg_list_t *querylist, asl_msg_list_t **res, size_t *last, size_t start, size_t count, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

int asl_msg_cmp_list(asl_msg_t *msg, asl_msg_list_t *list) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

__END_DECLS

#endif /* __ASL_MSG_LIST_H__ */
