/*
 * Copyright (c) 2019 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _ARRAY_H
#define	_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct array array_t;

int array_count(const array_t *);
void *array_get(const array_t *, int);

void array_add(array_t **, void *);
void array_concat(array_t **, array_t **);
void array_clear(array_t *, void (*)(void *, void *), void *);
void array_free(array_t **, void (*)(void *, void *), void *);

int array_iter(const array_t *, int (*)(void *, void *), void *);
#define ARRAY_ABORT  -1
#define ARRAY_KEEP   0
#define ARRAY_REMOVE 1
int array_filter(array_t *, int (*)(void *, void *), void *);
void array_sort(array_t *, int (*)(void *, void *));

#ifdef __cplusplus
}
#endif

#endif /* _ARRAY_H */
