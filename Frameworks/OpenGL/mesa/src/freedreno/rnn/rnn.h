/*
 * Copyright (C) 2010 Marcin Kościelnicki <koriakin@0x04.net>
 * Copyright (C) 2010 Luca Barbieri <luca@luca-barbieri.com>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef RNN_H
#define RNN_H

#include <stdint.h>
#include <stdlib.h>

struct rnnauthor {
	char* name;
	char* email;
	char* contributions;
	char* license;
	char** nicknames;
	int nicknamesnum;
	int nicknamesmax;
};

struct rnncopyright {
	unsigned firstyear;
	char* license;
	struct rnnauthor **authors;
	int authorsnum;
	int authorsmax;
};

struct rnndb {
	struct rnncopyright copyright;
	struct rnnenum **enums;
	int enumsnum;
	int enumsmax;
	struct rnnbitset **bitsets;
	int bitsetsnum;
	int bitsetsmax;
	struct rnndomain **domains;
	int domainsnum;
	int domainsmax;
	struct rnngroup **groups;
	int groupsnum;
	int groupsmax;
	struct rnnspectype **spectypes;
	int spectypesnum;
	int spectypesmax;
	char **files;
	int filesnum;
	int filesmax;
	int estatus;
};

struct rnnvarset {
	struct rnnenum *venum;
	int *variants;
};

struct rnnvarinfo {
	char *prefixstr;
	char *varsetstr;
	char *variantsstr;
	int dead;
	struct rnnenum *prefenum;
	char *prefix;
	struct rnnvarset **varsets;
	int varsetsnum;
	int varsetsmax;
};

struct rnnenum {
	char *name;
	int bare;
	int isinline;
	struct rnnvarinfo varinfo;
	struct rnnvalue **vals;
	int valsnum;
	int valsmax;
	char *fullname;
	int prepared;
	char *file;
};

struct rnnvalue {
	char *name;
	int valvalid;
	uint64_t value;
	struct rnnvarinfo varinfo;
	char *fullname;
	char *file;
};

struct rnntypeinfo {
	char *name;
	enum rnnttype {
		RNN_TTYPE_INVALID,
		RNN_TTYPE_INLINE_ENUM,
		RNN_TTYPE_INLINE_BITSET,
		RNN_TTYPE_ENUM,
		RNN_TTYPE_BITSET,
		RNN_TTYPE_SPECTYPE,
		RNN_TTYPE_HEX,
		RNN_TTYPE_INT,
		RNN_TTYPE_UINT,
		RNN_TTYPE_FLOAT,
		RNN_TTYPE_BOOLEAN,
		RNN_TTYPE_FIXED,
		RNN_TTYPE_UFIXED,
		RNN_TTYPE_A3XX_REGID,
	} type;
	struct rnnenum *eenum;
	struct rnnbitset *ebitset;
	struct rnnspectype *spectype;
	struct rnnbitfield **bitfields;
	int bitfieldsnum;
	int bitfieldsmax;
	struct rnnvalue **vals;
	int valsnum;
	int valsmax;
	int shr, low, high;
	uint64_t min, max, align, radix;
	int addvariant;
	int minvalid, maxvalid, alignvalid, radixvalid;
};

static inline uint64_t typeinfo_mask(struct rnntypeinfo *ti)
{
	if (ti->high == 63)
		return -(1ULL << ti->low);
	else
		return (1ULL << (ti->high + 1)) - (1ULL << ti->low);
}

struct rnnbitset {
	char *name;
	int bare;
	int isinline;
	struct rnnvarinfo varinfo;
	struct rnnbitfield **bitfields;
	int bitfieldsnum;
	int bitfieldsmax;
	char *fullname;
	char *file;
};

struct rnnbitfield {
	char *name;
	struct rnnvarinfo varinfo;
	struct rnntypeinfo typeinfo;
	char *fullname;
	char *file;
};

struct rnndomain {
	char *name;
	int bare;
	int width;
	uint64_t size;
	int sizevalid;
	struct rnnvarinfo varinfo;
	struct rnndelem **subelems;
	int subelemsnum;
	int subelemsmax;
	char *fullname;
	char *file;
};

struct rnngroup {
	char *name;
	struct rnndelem **subelems;
	int subelemsnum;
	int subelemsmax;
};

struct rnndelem {
	enum rnnetype {
		RNN_ETYPE_REG,
		RNN_ETYPE_ARRAY,
		RNN_ETYPE_STRIPE,
		RNN_ETYPE_USE_GROUP,
	} type;
	char *name;
	int width;
	enum rnnaccess {
		RNN_ACCESS_R,
		RNN_ACCESS_W,
		RNN_ACCESS_RW,
	} access;
	uint64_t offset;
	uint64_t *offsets;       /* for "array" with irregular offsets */
	int offsetsnum;
	int offsetsmax;
	char *doffset;
	char **doffsets;
	int doffsetsnum;
	int doffsetsmax;
	uint64_t length;
	uint64_t stride;
	struct rnndelem **subelems;
	int subelemsnum;
	int subelemsmax;
	struct rnnvarinfo varinfo;
	struct rnntypeinfo typeinfo;
	struct rnnenum *index;   /* for arrays, for symbolic idx values */
	char *fullname;
	char *file;
};

struct rnnspectype {
	char *name;
	struct rnntypeinfo typeinfo;
	char *file;
};

void rnn_init(void);
struct rnndb *rnn_newdb(void);
void rnn_parsefile (struct rnndb *db, char *file);
void rnn_prepdb (struct rnndb *db);
struct rnnenum *rnn_findenum (struct rnndb *db, const char *name);
struct rnnbitset *rnn_findbitset (struct rnndb *db, const char *name);
struct rnndomain *rnn_finddomain (struct rnndb *db, const char *name);
struct rnnspectype *rnn_findspectype (struct rnndb *db, const char *name);

#endif
