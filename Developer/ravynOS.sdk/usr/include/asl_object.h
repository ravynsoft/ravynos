#ifndef __ASL_OBJECT_H__
#define __ASL_OBJECT_H__

/*
 * Copyright (c) 2007-2012 Apple Inc.  All rights reserved.
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

/* generic object pointer */
typedef struct _asl_object_s
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	char asl_data[];
} asl_object_private_t;

#define ASL_OBJECT_HEADER_SIZE 8

typedef struct asl_jump_table_s
{
	asl_object_private_t * (*alloc)(uint32_t type);
	void (*dealloc)(asl_object_private_t *obj);
	int (*set_key_val_op)(asl_object_private_t *obj, const char *key, const char *val, uint16_t op);
	void (*unset_key)(asl_object_private_t *obj, const char *key);
	int (*get_val_op_for_key)(asl_object_private_t *obj, const char *key, const char **val, uint16_t *op);
	int (*get_key_val_op_at_index)(asl_object_private_t *obj, size_t n, const char **key, const char **val, uint16_t *op);
	size_t (*count)(asl_object_private_t *obj);
	asl_object_private_t *(*next)(asl_object_private_t *obj);
	asl_object_private_t *(*prev)(asl_object_private_t *obj);
	asl_object_private_t *(*get_object_at_index)(asl_object_private_t *obj, size_t n);
	void (*set_iteration_index)(asl_object_private_t *obj, size_t n);
	void (*remove_object_at_index)(asl_object_private_t *obj, size_t n);
	void (*append)(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr);
	void (*prepend)(asl_object_private_t *obj, asl_object_private_t *newobj);
	asl_object_private_t *(*search)(asl_object_private_t *obj, asl_object_private_t *query);
	asl_object_private_t *(*match)(asl_object_private_t *obj, asl_object_private_t *querylist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir);
} asl_jump_table_t;

__BEGIN_DECLS

int asl_object_set_key_val_op(asl_object_private_t *obj, const char *key, const char *val, uint16_t op);
void asl_object_unset_key(asl_object_private_t *obj, const char *key);
int asl_object_get_key_value_op(asl_object_private_t *obj, const char *key, const char **val, uint16_t *op);
size_t asl_object_count(asl_object_private_t *obj);
asl_object_private_t *asl_object_next(asl_object_private_t *obj);
asl_object_private_t *asl_object_prev(asl_object_private_t *obj);
asl_object_private_t *asl_object_get_object_at_index(asl_object_private_t *obj, size_t n);
void asl_object_set_iteration_index(asl_object_private_t *obj, size_t n);
void asl_object_remove_object_at_index(asl_object_private_t *obj, size_t n);
void asl_object_append(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr);
void asl_object_prepend(asl_object_private_t *obj, asl_object_private_t *newobj);
asl_object_private_t *asl_object_search(asl_object_private_t *obj, asl_object_private_t *query);
asl_object_private_t *asl_object_match(asl_object_private_t *obj, asl_object_private_t *querylist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir);

__END_DECLS

#endif /* __ASL_OBJECT_H__ */
