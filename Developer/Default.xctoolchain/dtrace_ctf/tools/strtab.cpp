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
 * Copyright (c) 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef max_align_t
namespace {
	typedef long double max_align_t;
}
#endif

#if !defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "strtab.h"
#include "memory.h"
#else
#include <sys/types.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "darwin_shim.h"
#include "strtab.h"
#include "memory.h"

#endif /* __APPLE__ */

#undef MIN
#define	MIN(a, b) 		((a) > (b) ? (b) : (a))

#include "llvm/ADT/DenseMap.h"

template<>
struct llvm::DenseMapInfo<atom_t *> {
	static inline atom_t *getEmptyKey() {
		return reinterpret_cast<atom_t *>(-1);
	}
	static inline atom_t *getTombstoneKey() {
		return reinterpret_cast<atom_t *>(-2);
	}
	static unsigned getHashValue(const atom_t *atom) {
		return atom_hash(atom);
	}
	static bool isEqual(const atom_t *LHS, const atom_t *RHS) {
		return LHS == RHS;
	}
};

typedef struct strinfo {
	ulong_t str_buf;		/* index of string data buffer */
	size_t str_off;			/* offset in bytes of this string */
	size_t str_len;			/* length in bytes of this string */
} strinfo_t;

#define	STRTAB_HASHSZ	211		/* use a prime number of hash buckets */
#define	STRTAB_BUFSZ	(64 * 1024)	/* use 64K data buffers by default */

extern "C" {

struct strhash : public llvm::DenseMap<atom_t *, strinfo_t> {
	strhash()
	: llvm::DenseMap<atom_t *, strinfo_t>(STRTAB_HASHSZ)
	{
	}
};


static void
strtab_grow(strtab_t *sp)
{
	sp->str_nbufs++;
	sp->str_bufs = (char **)xrealloc(sp->str_bufs, sp->str_nbufs * sizeof (char *));
	sp->str_ptr = (char *)xmalloc(sp->str_bufsz);
	sp->str_bufs[sp->str_nbufs - 1] = sp->str_ptr;
}

void
strtab_create(strtab_t *sp)
{
	sp->str_hash = new strhash();
	sp->str_bufs = NULL;
	sp->str_ptr = NULL;
	sp->str_nbufs = 0;
	sp->str_bufsz = STRTAB_BUFSZ;
	sp->str_size = 1;

	strtab_grow(sp);
	*sp->str_ptr++ = '\0';
}

void
strtab_destroy(strtab_t *sp)
{
	ulong_t i;

	for (i = 0; i < sp->str_nbufs; i++)
		free(sp->str_bufs[i]);

	delete sp->str_hash;
	free(sp->str_bufs);
}

static void
strtab_copyin(strtab_t *sp, const char *str, size_t len)
{
	ulong_t b = sp->str_nbufs - 1;
	size_t resid, n;

	while (len != 0) {
		if (sp->str_ptr == sp->str_bufs[b] + sp->str_bufsz) {
			strtab_grow(sp);
			b++;
		}

		resid = sp->str_bufs[b] + sp->str_bufsz - sp->str_ptr;
		n = MIN(resid, len);
		bcopy(str, sp->str_ptr, n);

		sp->str_ptr += n;
		str += n;
		len -= n;
	}
}

size_t
strtab_insert(strtab_t *sp, atom_t *atom)
{
	const char *str = (const char *)atom;

	if (str == NULL || str[0] == '\0')
		return (0); /* we keep a \0 at offset 0 to simplify things */

	size_t len = strlen((const char *)atom);
	strinfo_t info = {
		.str_buf = sp->str_nbufs - 1,
		.str_off = sp->str_size,
		.str_len = len,
	};

	/*
	 * If the string is already in our hash table, just return the offset
	 * of the existing string element and do not add a duplicate string.
	 */
	auto it = sp->str_hash->try_emplace(atom, info);
	if (!it.second) {
		return it.first->second.str_off;
	}

	/*
	 * Now copy the string data into our buffer list, and then update
	 * the global counts of strings and bytes.  Return str's byte offset.
	 */
	strtab_copyin(sp, str, len + 1);
	sp->str_size += len + 1;
	return (info.str_off);
}

size_t
strtab_size(const strtab_t *sp)
{
	return (sp->str_size);
}

ssize_t
strtab_write(const strtab_t *sp,
    ssize_t (*func)(const void *, size_t, void *), void *priv)
{
	ssize_t res, total = 0;
	ulong_t i;
	size_t n;

	for (i = 0; i < sp->str_nbufs; i++, total += res) {
		if (i == sp->str_nbufs - 1)
			n = sp->str_ptr - sp->str_bufs[i];
		else
			n = sp->str_bufsz;

		if ((res = func(sp->str_bufs[i], n, priv)) <= 0)
			break;
	}

	if (total == 0 && sp->str_size != 0)
		return (-1);

	return (total);
}

} // extern "C"
