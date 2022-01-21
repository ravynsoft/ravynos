/*
 * Copyright (c) 2007-2013 Apple Inc.  All rights reserved.
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
#include <asl_object.h>
#include <asl_core.h>
#include <asl_private.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <asl_client.h>
#include <asl_store.h>
#include <asl_file.h>
#include <dispatch/dispatch.h>
#ifdef __FreeBSD__
#include <atomic_compat.h>
#else
#include <libkern/OSAtomic.h>
#endif

static const asl_jump_table_t *asl_jump[ASL_TYPE_COUNT];
static dispatch_once_t asl_object_once;

static void
_asl_object_init(void)
{
	asl_jump[ASL_TYPE_MSG] = asl_msg_jump_table();
	asl_jump[ASL_TYPE_QUERY] = asl_msg_jump_table();
	asl_jump[ASL_TYPE_LIST] = asl_msg_list_jump_table();
	asl_jump[ASL_TYPE_FILE] = asl_file_jump_table();
	asl_jump[ASL_TYPE_STORE] = asl_store_jump_table();
	asl_jump[ASL_TYPE_CLIENT] = asl_client_jump_table();
}

#pragma mark -
#pragma mark asl_object

int
asl_object_set_key_val_op(asl_object_private_t *obj, const char *key, const char *val, uint16_t op)
{
	if (obj == NULL) return -1;
	if (obj->asl_type >= ASL_TYPE_COUNT) return -1;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->set_key_val_op == NULL) return -1;
	return asl_jump[obj->asl_type]->set_key_val_op(obj, key, val, op);
}

void
asl_object_unset_key(asl_object_private_t *obj, const char *key)
{
	if (obj == NULL) return;
	if (obj->asl_type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->unset_key == NULL) return;
	asl_jump[obj->asl_type]->unset_key(obj, key);
}

int
asl_object_get_val_op_for_key(asl_object_private_t *obj, const char *key, const char **val, uint16_t *op)
{
	if (obj == NULL) return -1;
	if (obj->asl_type >= ASL_TYPE_COUNT) return -1;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->get_val_op_for_key == NULL) return -1;
	return asl_jump[obj->asl_type]->get_val_op_for_key(obj, key, val, op);
}

int
asl_object_get_key_val_op_at_index(asl_object_private_t *obj, size_t n, const char **key, const char **val, uint16_t *op)
{
	if (obj == NULL) return -1;
	if (obj->asl_type >= ASL_TYPE_COUNT) return -1;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->get_key_val_op_at_index == NULL) return -1;
	return asl_jump[obj->asl_type]->get_key_val_op_at_index(obj, n, key, val, op);
}

size_t
asl_object_count(asl_object_private_t *obj)
{
	if (obj == NULL) return 0;
	if (obj->asl_type >= ASL_TYPE_COUNT) return 0;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->count == NULL) return 0;
	return asl_jump[obj->asl_type]->count(obj);
}

asl_object_private_t *
asl_object_next(asl_object_private_t *obj)
{
	if (obj == NULL) return NULL;
	if (obj->asl_type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->next == NULL) return NULL;
	return asl_jump[obj->asl_type]->next(obj);
}

asl_object_private_t *
asl_object_prev(asl_object_private_t *obj)
{
	if (obj == NULL) return NULL;
	if (obj->asl_type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->prev == NULL) return NULL;
	return asl_jump[obj->asl_type]->prev(obj);
}

asl_object_private_t *
asl_object_get_object_at_index(asl_object_private_t *obj, size_t n)
{
	if (obj == NULL) return NULL;
	if (obj->asl_type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->get_object_at_index == NULL) return NULL;
	return asl_jump[obj->asl_type]->get_object_at_index(obj, n);
}

void
asl_object_set_iteration_index(asl_object_private_t *obj, size_t n)
{
	if (obj == NULL) return;
	if (obj->asl_type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->set_iteration_index == NULL) return;
	return asl_jump[obj->asl_type]->set_iteration_index(obj, n);
}

void
asl_object_remove_object_at_index(asl_object_private_t *obj, size_t n)
{
	if (obj == NULL) return;
	if (obj->asl_type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->remove_object_at_index == NULL) return;
	return asl_jump[obj->asl_type]->remove_object_at_index(obj, n);
}

void
asl_object_append(asl_object_private_t *obj, asl_object_private_t *newobj)
{
	int type = ASL_TYPE_CLIENT;

	if (obj != NULL) type = obj->asl_type;
	if (type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[type]->append == NULL) return;
	return asl_jump[type]->append(obj, newobj);
}

void
asl_object_prepend(asl_object_private_t *obj, asl_object_private_t *newobj)
{
	if (obj == NULL) return;
	if (obj->asl_type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->prepend == NULL) return;
	return asl_jump[obj->asl_type]->prepend(obj, newobj);
}

asl_object_private_t *
asl_object_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	/* default to asl_client_search for obj == NULL */
	if (obj == NULL) return (asl_object_private_t *)asl_client_search(NULL, (asl_msg_t *)query);
	if (obj->asl_type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->search == NULL) return NULL;
	return asl_jump[obj->asl_type]->search(obj, query);
}

asl_object_private_t *
asl_object_match(asl_object_private_t *obj, asl_object_private_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	/* default to asl_client_match for obj == NULL */
	if (obj == NULL) return (asl_object_private_t *)asl_client_match(NULL, (asl_msg_list_t *)qlist, last, start, count, duration, dir);
	if (obj->asl_type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[obj->asl_type]->match == NULL) return NULL;
	return asl_jump[obj->asl_type]->match(obj, qlist, last, start, count, duration, dir);
}

asl_object_t
asl_retain(asl_object_t obj)
{
	asl_object_private_t *oo = (asl_object_private_t *)obj;
	if (oo == NULL) return NULL;

	OSAtomicIncrement32Barrier(&(oo->refcount));
	return obj;
}

void
asl_release(asl_object_t obj)
{
	asl_object_private_t *oo = (asl_object_private_t *)obj;
	if (oo == NULL) return;
	if (oo->asl_type >= ASL_TYPE_COUNT) return;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (OSAtomicDecrement32Barrier(&(oo->refcount)) != 0) return;
	if (asl_jump[oo->asl_type]->dealloc != NULL) asl_jump[oo->asl_type]->dealloc(oo);
}

asl_object_t
asl_new(uint32_t type)
{
	if (type >= ASL_TYPE_COUNT) return NULL;

	dispatch_once(&asl_object_once, ^{ _asl_object_init(); });
	if (asl_jump[type]->alloc == NULL) return NULL;
	asl_object_t out = (asl_object_t)asl_jump[type]->alloc(type);
	return out;
}

#pragma mark -
#pragma mark utilities

uint32_t
asl_get_type(asl_object_t obj)
{
	asl_object_private_t *oo = (asl_object_private_t *)obj;

	if (oo == NULL) return ASL_TYPE_UNDEF;
	return (int)oo->asl_type;
}

const char *
asl_get_value_for_key(asl_object_t obj, const char *key)
{
	const char *val = NULL;
	uint16_t op;

	asl_object_get_val_op_for_key((asl_object_private_t *)obj, key, &val, &op);
	return val;
}

int
asl_set(asl_object_t obj, const char *key, const char *val)
{
	asl_object_private_t *oo = (asl_object_private_t *)obj;
	uint16_t op = 0;

	if (oo == NULL) return -1;
	if (oo->asl_type == ASL_TYPE_QUERY) op = (uint32_t)-1;

	return asl_object_set_key_val_op(oo, key, val, op);
}

int
asl_unset_key(asl_object_t obj, const char *key)
{
	asl_object_unset_key((asl_object_private_t *)obj, key);
	return 0;
}

int
asl_set_key_val_op(asl_object_t obj, const char *key, const char *val, uint16_t op)
{
	return asl_object_set_key_val_op((asl_object_private_t *)obj, key, val, op);
}

size_t
asl_count(asl_object_t obj)
{
	return asl_object_count((asl_object_private_t *)obj);
}

asl_object_t
asl_get_index(asl_object_t list, size_t index)
{
	return (asl_object_t)asl_object_get_object_at_index((asl_object_private_t *)list, index);
}

asl_object_t
asl_next(asl_object_t obj)
{
	return (asl_object_t)asl_object_next((asl_object_private_t *)obj);
}

asl_object_t
asl_prev(asl_object_t obj)
{
	return (asl_object_t)asl_object_prev((asl_object_private_t *)obj);
}

void
asl_append(asl_object_t a, asl_object_t b)
{
	asl_object_append((asl_object_private_t *)a, (asl_object_private_t *)b);
}

void
asl_prepend(asl_object_t a, asl_object_t b)
{
	asl_object_prepend((asl_object_private_t *)a, (asl_object_private_t *)b);
}

/* asl_send is implemented as asl_append */
int
asl_send(asl_object_t a, asl_object_t b)
{
	asl_object_append((asl_object_private_t *)a, (asl_object_private_t *)b);
	return 0;
}

const char *
asl_key(asl_object_t obj, uint32_t n)
{
	const char *key = NULL;
	size_t sn = n;

	if (asl_object_get_key_val_op_at_index((asl_object_private_t *)obj, sn, &key, NULL, NULL) != 0) return NULL;
	return key;
}

const char *
asl_get(asl_object_t obj, const char *key)
{
	const char *val = NULL;

	if (asl_object_get_val_op_for_key((asl_object_private_t *)obj, key, &val, NULL) != 0) return NULL;
	return val;
}

int
asl_fetch_key_val_op(asl_object_t obj, uint32_t n, const char **key, const char **val, uint32_t *op)
{
	uint16_t op16;
	size_t sn = n;
	int status = asl_object_get_key_val_op_at_index((asl_object_private_t *)obj, sn, key, val, &op16);
	if (status != 0) return status;
	if (op != NULL) *op = op16;
	return 0;
}

int
asl_set_query(asl_object_t obj, const char *key, const char *val, uint32_t op)
{
	uint16_t op16 = op;
	return asl_object_set_key_val_op((asl_object_private_t *)obj, key, val, op16);
}

int
asl_unset(asl_object_t obj, const char *key)
{
	asl_object_unset_key((asl_object_private_t *)obj, key);
	return 0;
}

void
asl_reset_iteration(asl_object_t obj, size_t position)
{
	asl_object_set_iteration_index((asl_object_private_t *)obj, position);
}

asl_object_t
asl_search(asl_object_t data, asl_object_t query)
{
	return (asl_object_t)asl_object_search((asl_object_private_t *)data, (asl_object_private_t *)query);
}

asl_object_t
asl_match(asl_object_t data, asl_object_t qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction)
{
	return (asl_object_t)asl_object_match((asl_object_private_t *)data, (asl_object_private_t *)qlist, last, start, count, duration, direction);
}

void
asl_free(asl_object_t obj)
{
	asl_release(obj);
}

void
aslresponse_free(asl_object_t obj)
{
	asl_release(obj);
}

asl_object_t
aslresponse_next(asl_object_t obj)
{
	return (asl_object_t)asl_object_next((asl_object_private_t *)obj);
}

asl_object_t
asl_list_from_string(const char *buf)
{
	return (asl_object_t)asl_msg_list_from_string(buf);
}

char *
asl_format(asl_object_t obj, const char *msg_fmt, const char *time_fmt, uint32_t text_encoding)
{
	uint32_t len;
	uint32_t type = asl_get_type(obj);
	if (type != ASL_TYPE_MSG) return NULL;
	return asl_format_message((asl_msg_t *)obj, msg_fmt, time_fmt, text_encoding, &len);
}
