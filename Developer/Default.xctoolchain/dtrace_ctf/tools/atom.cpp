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

#ifndef max_align_t
namespace {
	typedef long double max_align_t;
}
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "memory.h"
#include "atom.h"
#include "llvm/ADT/DenseSet.h"

llvm::DenseSet<const char *> atoms{4096};

extern "C" {

atom_t *
atom_get(const char *s)
{
	auto it = atoms.insert(s);
	if (it.second) {
		*it.first = xstrdup(s);
	}
	return reinterpret_cast<atom_t *>(*it.first);
}

atom_t *
atom_get_consume(char *s)
{
	auto it = atoms.insert(s);
	if (!it.second) {
		free(s);
	}
	return reinterpret_cast<atom_t *>(*it.first);
}

__attribute__((always_inline)) // let LTO know
unsigned
atom_hash(atom_t *atom)
{
	unsigned long key = reinterpret_cast<unsigned long>(atom);
	key ^= key >> 4;
#if __LP64__
	key *= 0x8a970be7488fda55;
	key ^= __builtin_bswap64(key);
#else
	key *= 0x5052acdb;
	key ^= __builtin_bswap32(key);
#endif
	return static_cast<unsigned>(key);
}

} // extern "C"
