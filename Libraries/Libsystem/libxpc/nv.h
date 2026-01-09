/*-
 * Copyright (c) 2009-2013 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software was developed by Pawel Jakub Dawidek under sponsorship from
 * the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef	_NV_H_
#define	_NV_H_

#include <sys/cdefs.h>

#ifndef _KERNEL
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <uuid.h>
#else
#include <sys/uuid.h>
#endif



#ifndef	_NVLIST_T_DECLARED
#define	_NVLIST_T_DECLARED
struct nvlist;

typedef struct nvlist nvlist_t;
#endif

#define	NV_NAME_MAX	2048

#define	NV_TYPE_NONE			0

#define	NV_TYPE_NULL			1
#define	NV_TYPE_BOOL			2
#define	NV_TYPE_STRING			3
#define	NV_TYPE_DESCRIPTOR		4
#define	NV_TYPE_BINARY			5

#define	NV_TYPE_NUMBER			6
#define	NV_TYPE_PTR			7
#define	NV_TYPE_UINT64			8
#define	NV_TYPE_INT64			9
#define	NV_TYPE_ENDPOINT		10
#define	NV_TYPE_DATE			11
#define	NV_TYPE_UUID			12

#define	NV_TYPE_NVLIST			13
#define	NV_TYPE_NVLIST_ARRAY		14
#define	NV_TYPE_NVLIST_DICTIONARY	15


/*
 * Perform case-insensitive lookups of provided names.
 */
#define	NV_FLAG_IGNORE_CASE		0x01
/*
 * Names don't have to be unique.
 */
#define	NV_FLAG_NO_UNIQUE		0x02

#if defined(_KERNEL) && defined(MALLOC_DECLARE)
MALLOC_DECLARE(M_NVLIST);
#endif

__BEGIN_DECLS

nvlist_t	*nvlist_create(int flags);
nvlist_t	*nvlist_create_array(int flags);
nvlist_t	*nvlist_create_dictionary(int flags);
void		 nvlist_destroy(nvlist_t *nvl);
int		 nvlist_error(const nvlist_t *nvl);
bool		 nvlist_empty(const nvlist_t *nvl);
int		 nvlist_flags(const nvlist_t *nvl);
void		 nvlist_set_error(nvlist_t *nvl, int error);

nvlist_t *nvlist_clone(const nvlist_t *nvl);

#ifndef _KERNEL
void nvlist_dump(const nvlist_t *nvl, int fd);
void nvlist_fdump(const nvlist_t *nvl, FILE *fp);
#endif

size_t		 nvlist_size(const nvlist_t *nvl);
void		*nvlist_pack(const nvlist_t *nvl, size_t *sizep);
void		*nvlist_pack_buffer(const nvlist_t *nvl, void *buf, size_t *sizep);
nvlist_t	*nvlist_unpack(const void *buf, size_t size);

int nvlist_send(int sock, const nvlist_t *nvl);
nvlist_t *nvlist_recv(int sock);
nvlist_t *nvlist_xfer(int sock, nvlist_t *nvl);

const char *nvlist_next(const nvlist_t *nvl, int *typep, void **cookiep);

const nvlist_t *nvlist_get_parent(const nvlist_t *nvl, void **cookiep);

/*
 * The nvlist_exists functions check if the given name (optionally of the given
 * type) exists on nvlist.
 */

bool nvlist_exists(const nvlist_t *nvl, const char *name);
bool nvlist_exists_type(const nvlist_t *nvl, const char *name, int type);

bool nvlist_exists_null(const nvlist_t *nvl, const char *name);
bool nvlist_exists_bool(const nvlist_t *nvl, const char *name);
bool nvlist_exists_number(const nvlist_t *nvl, const char *name);
bool nvlist_exists_ptr(const nvlist_t *nvl, const char *name);
bool nvlist_exists_uint64(const nvlist_t *nvl, const char *name);
bool nvlist_exists_int64(const nvlist_t *nvl, const char *name);
bool nvlist_exists_endpoint(const nvlist_t *nvl, const char *name);
bool nvlist_exists_date(const nvlist_t *nvl, const char *name);
bool nvlist_exists_string(const nvlist_t *nvl, const char *name);
bool nvlist_exists_nvlist(const nvlist_t *nvl, const char *name);
bool nvlist_exists_nvlist_dictionary(const nvlist_t *nvl, const char *name);
bool nvlist_exists_nvlist_array(const nvlist_t *nvl, const char *name);
#ifndef _KERNEL
bool nvlist_exists_descriptor(const nvlist_t *nvl, const char *name);
#endif
bool nvlist_exists_binary(const nvlist_t *nvl, const char *name);
bool nvlist_exists_uuid(const nvlist_t *nvl, const char *name);

/*
 * The nvlist_add functions add the given name/value pair.
 * If a pointer is provided, nvlist_add will internally allocate memory for the
 * given data (in other words it won't consume provided buffer).
 */

void nvlist_add_null(nvlist_t *nvl, const char *name);
void nvlist_add_bool(nvlist_t *nvl, const char *name, bool value);
void nvlist_add_number(nvlist_t *nvl, const char *name, uint64_t value);
void nvlist_add_ptr(nvlist_t *nvl, const char *name, uintptr_t value);
void nvlist_add_int64(nvlist_t *nvl, const char *name, int64_t value);
void nvlist_add_uint64(nvlist_t *nvl, const char *name, uint64_t value);
void nvlist_add_endpoint(nvlist_t *nvl, const char *name, int value);
void nvlist_add_date(nvlist_t *nvl, const char *name, uint64_t value);
void nvlist_add_string(nvlist_t *nvl, const char *name, const char *value);
void nvlist_add_stringf(nvlist_t *nvl, const char *name, const char *valuefmt, ...) __printflike(3, 4);
#ifdef _VA_LIST_DECLARED
void nvlist_add_stringv(nvlist_t *nvl, const char *name, const char *valuefmt, va_list valueap) __printflike(3, 0);
#endif
void nvlist_add_nvlist(nvlist_t *nvl, const char *name, const nvlist_t *value);
void nvlist_add_nvlist_dictionary(nvlist_t *nvl, const char *name, const nvlist_t *value);
void nvlist_add_nvlist_array(nvlist_t *nvl, const char *name, const nvlist_t *value);
#ifndef _KERNEL
void nvlist_add_descriptor(nvlist_t *nvl, const char *name, int value);
#endif
void nvlist_add_binary(nvlist_t *nvl, const char *name, const void *value, size_t size);
void nvlist_add_uuid(nvlist_t *nvl, const char *name, const uuid_t *value);

/*
 * The nvlist_move functions add the given name/value pair.
 * The functions consumes provided buffer.
 */

void nvlist_move_string(nvlist_t *nvl, const char *name, char *value);
void nvlist_move_nvlist(nvlist_t *nvl, const char *name, nvlist_t *value);
void nvlist_move_nvlist_array(nvlist_t *nvl, const char *name, nvlist_t *value);
void nvlist_move_nvlist_dictionary(nvlist_t *nvl, const char *name, nvlist_t *value);
#ifndef _KERNEL
void nvlist_move_descriptor(nvlist_t *nvl, const char *name, int value);
#endif
void nvlist_move_binary(nvlist_t *nvl, const char *name, void *value, size_t size);
void nvlist_move_uuid(nvlist_t *nvl, const char *name, uuid_t *value);

/*
 * The nvlist_get functions returns value associated with the given name.
 * If it returns a pointer, the pointer represents internal buffer and should
 * not be freed by the caller.
 */

bool		 nvlist_get_bool(const nvlist_t *nvl, const char *name);
uint64_t	 nvlist_get_number(const nvlist_t *nvl, const char *name);
uintptr_t	 nvlist_get_ptr(const nvlist_t *nvl, const char *name);
uint64_t	 nvlist_get_uint64(const nvlist_t *nvl, const char *name);
int64_t		 nvlist_get_int64(const nvlist_t *nvl, const char *name);
int		 nvlist_get_endpoint(const nvlist_t *nvl, const char *name);
uint64_t	 nvlist_get_date(const nvlist_t *nvl, const char *name);

const char	*nvlist_get_string(const nvlist_t *nvl, const char *name);
const nvlist_t	*nvlist_get_nvlist(const nvlist_t *nvl, const char *name);
const nvlist_t	*nvlist_get_nvlist_array(const nvlist_t *nvl, const char *name);
const nvlist_t	*nvlist_get_nvlist_dictionary(const nvlist_t *nvl, const char *name);
#ifndef _KERNEL
int		 nvlist_get_descriptor(const nvlist_t *nvl, const char *name);
#endif
const void	*nvlist_get_binary(const nvlist_t *nvl, const char *name, size_t *sizep);
const uuid_t	*nvlist_get_uuid(const nvlist_t *nvl, const char *name);

/*
 * The nvlist_take functions returns value associated with the given name and
 * remove the given entry from the nvlist.
 * The caller is responsible for freeing received data.
 */

bool		 nvlist_take_bool(nvlist_t *nvl, const char *name);
uint64_t	 nvlist_take_number(nvlist_t *nvl, const char *name);
uintptr_t	 nvlist_take_ptr(nvlist_t *nvl, const char *name);
uint64_t	 nvlist_take_uint64(nvlist_t *nvl, const char *name);
int64_t		 nvlist_take_int64(nvlist_t *nvl, const char *name);
int		 nvlist_take_endpoint(nvlist_t *nvl, const char *name);
uint64_t	 nvlist_take_date(nvlist_t *nvl, const char *name);
char		*nvlist_take_string(nvlist_t *nvl, const char *name);
nvlist_t	*nvlist_take_nvlist(nvlist_t *nvl, const char *name);
nvlist_t	*nvlist_take_nvlist_array(nvlist_t *nvl, const char *name);
nvlist_t	*nvlist_take_nvlist_dictionary(nvlist_t *nvl, const char *name);

#ifndef _KERNEL
int		 nvlist_take_descriptor(nvlist_t *nvl, const char *name);
#endif
void		*nvlist_take_binary(nvlist_t *nvl, const char *name, size_t *sizep);
uuid_t		*nvlist_take_uuid(nvlist_t *nvl, const char *name);

/*
 * The nvlist_free functions removes the given name/value pair from the nvlist
 * and frees memory associated with it.
 */

void nvlist_free(nvlist_t *nvl, const char *name);
void nvlist_free_type(nvlist_t *nvl, const char *name, int type);

void nvlist_free_null(nvlist_t *nvl, const char *name);
void nvlist_free_bool(nvlist_t *nvl, const char *name);
void nvlist_free_number(nvlist_t *nvl, const char *name);
void nvlist_free_ptr(nvlist_t *nvl, const char *name);
void nvlist_free_uint64(nvlist_t *nvl, const char *name);
void nvlist_free_int64(nvlist_t *nvl, const char *name);
void nvlist_free_endpoint(nvlist_t *nvl, const char *name);
void nvlist_free_date(nvlist_t *nvl, const char *name);

void nvlist_free_string(nvlist_t *nvl, const char *name);
void nvlist_free_nvlist(nvlist_t *nvl, const char *name);
void nvlist_free_nvlist_array(nvlist_t *nvl, const char *name);
void nvlist_free_nvlist_dictionary(nvlist_t *nvl, const char *name);
#ifndef _KERNEL
void nvlist_free_descriptor(nvlist_t *nvl, const char *name);
#endif
void nvlist_free_binary(nvlist_t *nvl, const char *name);
void nvlist_free_uuid(nvlist_t *nvl, const char *name);

/*
 * Below are the same functions, but which operate on format strings and
 * variable argument lists.
 *
 * Functions that are not inserting a new pair into the nvlist cannot handle
 * a failure to allocate the memory to hold the new name.  Therefore these
 * functions are not provided in the kernel.
 */

#ifndef _KERNEL
bool nvlist_existsf(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_type(const nvlist_t *nvl, int type, const char *namefmt, ...) __printflike(3, 4);

bool nvlist_existsf_null(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_bool(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_number(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_ptr(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_uint64(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_int64(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_endpoint(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_date(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_string(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_nvlist(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_nvlist_array(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_nvlist_dictionary(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_descriptor(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_binary(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
bool nvlist_existsf_uuid(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);

bool nvlist_existsv(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_type(const nvlist_t *nvl, int type, const char *namefmt, va_list nameap) __printflike(3, 0);

bool nvlist_existsv_null(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_bool(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_number(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_ptr(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_uint64(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_int64(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_endpoint(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_date(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_string(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_nvlist(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_nvlist_array(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_nvlist_dictionary(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_descriptor(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_binary(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
bool nvlist_existsv_uuid(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
#endif

void nvlist_addf_null(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_addf_bool(nvlist_t *nvl, bool value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_number(nvlist_t *nvl, uint64_t value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_ptr(nvlist_t *nvl, uintptr_t value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_uint64(nvlist_t *nvl, uint64_t value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_int64(nvlist_t *nvl, int64_t value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_endpoint(nvlist_t *nvl, int value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_date(nvlist_t *nvl, uint64_t value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_string(nvlist_t *nvl, const char *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_nvlist(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_nvlist_array(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_addf_nvlist_dictionary(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
#ifndef _KERNEL
void nvlist_addf_descriptor(nvlist_t *nvl, int value, const char *namefmt, ...) __printflike(3, 4);
#endif
void nvlist_addf_binary(nvlist_t *nvl, const void *value, size_t size, const char *namefmt, ...) __printflike(4, 5);
void nvlist_addf_uuid(nvlist_t *nvl, const uuid_t *value, const char *namefmt, ...) __printflike(3, 4);

#if !defined(_KERNEL) || defined(_VA_LIST_DECLARED)
void nvlist_addv_null(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_addv_bool(nvlist_t *nvl, bool value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_number(nvlist_t *nvl, uint64_t value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_ptr(nvlist_t *nvl, uintptr_t value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_uint64(nvlist_t *nvl, uint64_t value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_int64(nvlist_t *nvl, int64_t value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_endpoint(nvlist_t *nvl, int value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_date(nvlist_t *nvl, uint64_t value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_string(nvlist_t *nvl, const char *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_nvlist(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_nvlist_array(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_addv_nvlist_dictionary(nvlist_t *nvl, const nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
#ifndef _KERNEL
void nvlist_addv_descriptor(nvlist_t *nvl, int value, const char *namefmt, va_list nameap) __printflike(3, 0);
#endif
void nvlist_addv_binary(nvlist_t *nvl, const void *value, size_t size, const char *namefmt, va_list nameap) __printflike(4, 0);
void nvlist_addv_uuid(nvlist_t *nvl, const uuid_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
#endif

void nvlist_movef_string(nvlist_t *nvl, char *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_movef_nvlist(nvlist_t *nvl, nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_movef_nvlist_array(nvlist_t *nvl, nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
void nvlist_movef_nvlist_dictionary(nvlist_t *nvl, nvlist_t *value, const char *namefmt, ...) __printflike(3, 4);
#ifndef _KERNEL
void nvlist_movef_descriptor(nvlist_t *nvl, int value, const char *namefmt, ...) __printflike(3, 4);
#endif
void nvlist_movef_binary(nvlist_t *nvl, void *value, size_t size, const char *namefmt, ...) __printflike(4, 5);
void nvlist_movef_uuid(nvlist_t *nvl, uuid_t *value, const char *namefmt, ...) __printflike(3, 4);

#if !defined(_KERNEL) || defined(_VA_LIST_DECLARED)
void nvlist_movev_string(nvlist_t *nvl, char *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_movev_nvlist(nvlist_t *nvl, nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_movev_nvlist_array(nvlist_t *nvl, nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
void nvlist_movev_nvlist_dictionary(nvlist_t *nvl, nvlist_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
#ifndef _KERNEL
void nvlist_movev_descriptor(nvlist_t *nvl, int value, const char *namefmt, va_list nameap) __printflike(3, 0);
#endif
void nvlist_movev_binary(nvlist_t *nvl, void *value, size_t size, const char *namefmt, va_list nameap) __printflike(4, 0);
void nvlist_movev_uuid(nvlist_t *nvl, uuid_t *value, const char *namefmt, va_list nameap) __printflike(3, 0);
#endif

#ifndef _KERNEL
bool		 nvlist_getf_bool(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_getf_number(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uintptr_t	 nvlist_getf_ptr(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_getf_uint64(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int64_t		 nvlist_getf_int64(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int		 nvlist_getf_endpoint(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_getf_date(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
const char	*nvlist_getf_string(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
const nvlist_t	*nvlist_getf_nvlist(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
const nvlist_t	*nvlist_getf_nvlist_array(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
const nvlist_t	*nvlist_getf_nvlist_dictionary(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int		 nvlist_getf_descriptor(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
const void	*nvlist_getf_binary(const nvlist_t *nvl, size_t *sizep, const char *namefmt, ...) __printflike(3, 4);
const uuid_t	*nvlist_getf_uuid(const nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);

bool		 nvlist_getv_bool(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_getv_number(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uintptr_t	 nvlist_getv_ptr(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_getv_uint64(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int64_t	 	 nvlist_getv_int64(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int	 	 nvlist_getv_endpoint(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_getv_date(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
const char	*nvlist_getv_string(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
const nvlist_t	*nvlist_getv_nvlist(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
const nvlist_t	*nvlist_getv_nvlist_array(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
const nvlist_t	*nvlist_getv_nvlist_dictionary(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int		 nvlist_getv_descriptor(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
const void	*nvlist_getv_binary(const nvlist_t *nvl, size_t *sizep, const char *namefmt, va_list nameap) __printflike(3, 0);
const uuid_t	*nvlist_getv_uuid(const nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);

bool		 nvlist_takef_bool(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_takef_number(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uintptr_t	 nvlist_takef_ptr(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_takef_uint64(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int64_t	 	 nvlist_takef_int64(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int		 nvlist_takef_endpoint(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
uint64_t	 nvlist_takef_date(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
char		*nvlist_takef_string(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
nvlist_t	*nvlist_takef_nvlist(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
nvlist_t	*nvlist_takef_nvlist_array(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
nvlist_t	*nvlist_takef_nvlist_dictionary(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
int		 nvlist_takef_descriptor(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void		*nvlist_takef_binary(nvlist_t *nvl, size_t *sizep, const char *namefmt, ...) __printflike(3, 4);
uuid_t		*nvlist_takef_uuid(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);

bool		 nvlist_takev_bool(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_takev_number(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uintptr_t	 nvlist_takev_ptr(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_takev_uint64(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int64_t		 nvlist_takev_int64(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int		 nvlist_takev_endpoint(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
uint64_t	 nvlist_takev_date(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
char		*nvlist_takev_string(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
nvlist_t	*nvlist_takev_nvlist(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
nvlist_t	*nvlist_takev_nvlist_array(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
nvlist_t	*nvlist_takev_nvlist_dictionary(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
int		 nvlist_takev_descriptor(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void		*nvlist_takev_binary(nvlist_t *nvl, size_t *sizep, const char *namefmt, va_list nameap) __printflike(3, 0);
uuid_t		*nvlist_takev_uuid(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);

void nvlist_freef(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_type(nvlist_t *nvl, int type, const char *namefmt, ...) __printflike(3, 4);

void nvlist_freef_null(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_bool(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_number(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_ptr(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_uint64(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_int64(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_endpoint(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_date(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_string(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_nvlist(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_nvlist_array(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_nvlist_dictionary(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_descriptor(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_binary(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);
void nvlist_freef_uuid(nvlist_t *nvl, const char *namefmt, ...) __printflike(2, 3);

void nvlist_freev(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_type(nvlist_t *nvl, int type, const char *namefmt, va_list nameap) __printflike(3, 0);

void nvlist_freev_null(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_bool(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_number(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_ptr(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_uint64(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_int64(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_endpoint(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_date(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_string(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_nvlist(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_nvlist_array(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_nvlist_dictionary(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_descriptor(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_binary(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
void nvlist_freev_uuid(nvlist_t *nvl, const char *namefmt, va_list nameap) __printflike(2, 0);
#endif /* _KERNEL */

__END_DECLS

#endif	/* !_NV_H_ */
