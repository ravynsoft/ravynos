/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2001-2003 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Create, manage, and destroy association lists.  alists are arrays with
 * arbitrary index types, and are also commonly known as associative arrays.
 */

#ifndef max_align_t
namespace {
	typedef long double max_align_t;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include "llvm/ADT/DenseMap.h"

#include "alist.h"
#include "memory.h"
#include "hash.h"
#include "ctftools.h"

struct alistDenseMapInfo {
	static inline void *getEmptyKey() {
		return reinterpret_cast<void *>(-1);
	}
	static inline void *getTombstoneKey() {
		return reinterpret_cast<void *>(-2);
	}
	static unsigned getHashValue(const void *v) {
		uintptr_t key = reinterpret_cast<uintptr_t>(v);
		return key * 37U;
	}
	static bool isEqual(const void *a, const void *b) {
		return a == b;
	}
};

extern "C" {

struct alist : public llvm::DenseMap<void *, void *, alistDenseMapInfo> {
	alist(unsigned size)
	: llvm::DenseMap<void *, void *, alistDenseMapInfo>(size)
	{
	}

	void
	stats(int verbose __unused)
	{
		printf("Alist statistics\n");
		printf(" Items  : %d\n", size());
	}
};

alist_t *
alist_new(unsigned size)
{
	return new alist{size};
}

void
alist_clear(alist_t *alist)
{
	alist->clear();
}

void
alist_free(alist_t *alist)
{
	delete alist;
}

void
alist_add(alist_t *alist, void *name, void *value)
{
	if (name == alistDenseMapInfo::getEmptyKey())
		terminate("Trying to insert the empty key (%p)", name);
	if (name == alistDenseMapInfo::getTombstoneKey())
		terminate("Trying to insert the tombstone key (%p)", name);
	alist->try_emplace(name, value);
}

int
alist_find(alist_t *alist, void *name, void **value)
{
	auto it = alist->find(name);

	if (it == alist->end())
		return (0);

	if (value)
		*value = it->second;

	return (1);
}

int
alist_iter(alist_t *alist, int (*func)(void *, void *, void *), void *priv)
{
	int cumrc = 0;
	int cbrc;

	for (auto it: *alist) {
		if ((cbrc = func(it.first, it.second, priv)) < 0)
			return (cbrc);
		cumrc += cbrc;
	}

	return (cumrc);
}

/*
 * Debugging support.  Used to print the contents of an alist.
 */

void
alist_stats(alist_t *alist, int verbose)
{
	printf("Alist statistics\n");
	alist->stats(verbose);
}

} // extern "C"
