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

#ifndef _ATOM_H
#define	_ATOM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ATOM_NULL ((atom_t *)NULL)

typedef const struct atom atom_t;

atom_t *atom_get(const char *s);
atom_t *atom_get_consume(char *s);

unsigned atom_hash(atom_t *atom);

#ifndef __cplusplus
struct atom {
	char value[0];
};

static inline const char *
atom_pretty(atom_t *atom, const char *nullstr)
{
	return atom ? atom->value : nullstr;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ATOM_H */
