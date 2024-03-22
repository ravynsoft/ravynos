/*
 * Copyright (C) 2010-2011 Marcin Kościelnicki <koriakin@0x04.net>
 * Copyright (C) 2010 Francisco Jerez <currojerez@riseup.net>
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

#include "rnndec.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "util.h"
#include "util/compiler.h"

struct rnndeccontext *rnndec_newcontext(struct rnndb *db) {
	struct rnndeccontext *res = calloc (sizeof *res, 1);
	res->db = db;
	res->colors = &envy_null_colors;
	return res;
}

int rnndec_varadd(struct rnndeccontext *ctx, char *varset, const char *variant) {
	struct rnnenum *en = rnn_findenum(ctx->db, varset);
	if (!en) {
		fprintf (stderr, "Enum %s doesn't exist in database!\n", varset);
		return 0;
	}
	int i, j;
	for (i = 0; i < en->valsnum; i++)
		if (!strcasecmp(en->vals[i]->name, variant)) {
			break;
		}

	if (i == en->valsnum) {
		fprintf (stderr, "Variant %s doesn't exist in enum %s!\n", variant, varset);
		return 0;
	}

	for (j = 0; j < ctx->varsnum; j++) {
		if (ctx->vars[j]->en == en) {
			ctx->vars[j]->variant = i;
			break;
		}
	}

	if (j == ctx->varsnum) {
		struct rnndecvariant *ci = calloc (sizeof *ci, 1);
		ci->en = en;
		ci->variant = i;
		ADDARRAY(ctx->vars, ci);
	}

	return 1;
}

int rnndec_varmatch(struct rnndeccontext *ctx, struct rnnvarinfo *vi) {
	if (vi->dead)
		return 0;
	int i;
	for (i = 0; i < vi->varsetsnum; i++) {
		int j;
		for (j = 0; j < ctx->varsnum; j++)
			if (vi->varsets[i]->venum == ctx->vars[j]->en)
				break;
		if (j == ctx->varsnum) {
			fprintf (stderr, "I don't know which %s variant to use!\n", vi->varsets[i]->venum->name);
		} else {
			if (!vi->varsets[i]->variants[ctx->vars[j]->variant])
				return 0;
		}
	}
	return 1;
}

/* see https://en.wikipedia.org/wiki/Half-precision_floating-point_format */
static uint32_t float16i(uint16_t val)
{
	uint32_t sign = ((uint32_t)(val & 0x8000)) << 16;
	uint32_t frac = val & 0x3ff;
	int32_t  expn = (val >> 10) & 0x1f;

	if (expn == 0) {
		if (frac) {
			/* denormalized number: */
			int shift = __builtin_clz(frac) - 21;
			frac <<= shift;
			expn = -shift;
		} else {
			/* +/- zero: */
			return sign;
		}
	} else if (expn == 0x1f) {
		/* Inf/NaN: */
		return sign | 0x7f800000 | (frac << 13);
	}

	return sign | ((expn + 127 - 15) << 23) | (frac << 13);
}
static float float16(uint16_t val)
{
	union { uint32_t i; float f; } u;
	u.i = float16i(val);
	return u.f;
}

static const char *rnndec_decode_enum_val(struct rnndeccontext *ctx,
		struct rnnvalue **vals, int valsnum, uint64_t value)
{
	int i;
	for (i = 0; i < valsnum; i++)
		if (rnndec_varmatch(ctx, &vals[i]->varinfo) &&
				vals[i]->valvalid && vals[i]->value == value)
			return vals[i]->name;
	return NULL;
}

const char *rnndec_decode_enum(struct rnndeccontext *ctx, const char *enumname, uint64_t enumval)
{
	struct rnnenum *en = rnn_findenum (ctx->db, enumname);
	if (en) {
		return rnndec_decode_enum_val(ctx, en->vals, en->valsnum, enumval);
	}
	return NULL;
}

/* The name UNK%u is used as a placeholder for bitfields that exist but
 * have an unknown function.
 */
static int is_unknown(const char *name)
{
	unsigned u;
	return sscanf(name, "UNK%u", &u) == 1;
}

char *rnndec_decodeval(struct rnndeccontext *ctx, struct rnntypeinfo *ti, uint64_t value) {
	int width = ti->high - ti->low + 1;
	char *res = 0;
	int i;
	struct rnnvalue **vals;
	int valsnum;
	struct rnnbitfield **bitfields;
	int bitfieldsnum;
	char *tmp;
	const char *ctmp;
	uint64_t mask;

	uint64_t value_orig = value;
	if (!ti)
		goto failhex;
	value = (value & typeinfo_mask(ti)) >> ti->low;
	value <<= ti->shr;

	switch (ti->type) {
		case RNN_TTYPE_ENUM:
			vals = ti->eenum->vals;
			valsnum = ti->eenum->valsnum;
			goto doenum;
		case RNN_TTYPE_INLINE_ENUM:
			vals = ti->vals;
			valsnum = ti->valsnum;
			goto doenum;
		doenum:
			ctmp = rnndec_decode_enum_val(ctx, vals, valsnum, value);
			if (ctmp) {
				asprintf (&res, "%s%s%s", ctx->colors->eval, ctmp, ctx->colors->reset);
				if (ti->addvariant) {
					rnndec_varadd(ctx, ti->eenum->name, ctmp);
				}
				break;
			}
			goto failhex;
		case RNN_TTYPE_BITSET:
			bitfields = ti->ebitset->bitfields;
			bitfieldsnum = ti->ebitset->bitfieldsnum;
			goto dobitset;
		case RNN_TTYPE_INLINE_BITSET:
			bitfields = ti->bitfields;
			bitfieldsnum = ti->bitfieldsnum;
			goto dobitset;
		dobitset:
			mask = 0;
			for (i = 0; i < bitfieldsnum; i++) {
				if (!rnndec_varmatch(ctx, &bitfields[i]->varinfo))
					continue;
				uint64_t type_mask = typeinfo_mask(&bitfields[i]->typeinfo);
				if (((value & type_mask) == 0) && is_unknown(bitfields[i]->name))
					continue;
				mask |= type_mask;
				if (bitfields[i]->typeinfo.type == RNN_TTYPE_BOOLEAN) {
					const char *color = is_unknown(bitfields[i]->name) ?
							ctx->colors->err : ctx->colors->mod;
					if (value & type_mask) {
						if (!res)
							asprintf (&res, "%s%s%s", color, bitfields[i]->name, ctx->colors->reset);
						else {
							asprintf (&tmp, "%s | %s%s%s", res, color, bitfields[i]->name, ctx->colors->reset);
							free(res);
							res = tmp;
						}
					}
					continue;
				}
				char *subval;
				if (is_unknown(bitfields[i]->name) && (bitfields[i]->typeinfo.type != RNN_TTYPE_A3XX_REGID)) {
					uint64_t field_val = value & type_mask;
					field_val = (field_val & typeinfo_mask(&bitfields[i]->typeinfo)) >> bitfields[i]->typeinfo.low;
					field_val <<= bitfields[i]->typeinfo.shr;
					asprintf (&subval, "%s%#"PRIx64"%s", ctx->colors->err, field_val, ctx->colors->reset);
				} else {
					subval = rnndec_decodeval(ctx, &bitfields[i]->typeinfo, value & type_mask);
				}
				if (!res)
					asprintf (&res, "%s%s%s = %s", ctx->colors->rname, bitfields[i]->name, ctx->colors->reset, subval);
				else {
					asprintf (&tmp, "%s | %s%s%s = %s", res, ctx->colors->rname, bitfields[i]->name, ctx->colors->reset, subval);
					free(res);
					res = tmp;
				}
				free(subval);
			}
			if (value & ~mask) {
				if (!res)
					asprintf (&res, "%s%#"PRIx64"%s", ctx->colors->err, value & ~mask, ctx->colors->reset);
				else {
					asprintf (&tmp, "%s | %s%#"PRIx64"%s", res, ctx->colors->err, value & ~mask, ctx->colors->reset);
					free(res);
					res = tmp;
				}
			}
			if (!res)
				asprintf (&res, "%s0%s", ctx->colors->num, ctx->colors->reset);
			asprintf (&tmp, "{ %s }", res);
			free(res);
			return tmp;
		case RNN_TTYPE_SPECTYPE:
			return rnndec_decodeval(ctx, &ti->spectype->typeinfo, value);
		case RNN_TTYPE_HEX:
			asprintf (&res, "%s%#"PRIx64"%s", ctx->colors->num, value, ctx->colors->reset);
			break;
		case RNN_TTYPE_FIXED:
			if (value & UINT64_C(1) << (width-1)) {
				asprintf (&res, "%s-%lf%s", ctx->colors->num,
						((double)((UINT64_C(1) << width) - value)) / ((double)(1 << ti->radix)),
						ctx->colors->reset);
				break;
			}
			FALLTHROUGH;
		case RNN_TTYPE_UFIXED:
			asprintf (&res, "%s%lf%s", ctx->colors->num,
					((double)value) / ((double)(1LL << ti->radix)),
					ctx->colors->reset);
			break;
		case RNN_TTYPE_A3XX_REGID:
			asprintf (&res, "%sr%"PRIu64".%c%s", ctx->colors->num, (value >> 2), "xyzw"[value & 0x3], ctx->colors->reset);
			break;
		case RNN_TTYPE_UINT:
			asprintf (&res, "%s%"PRIu64"%s", ctx->colors->num, value, ctx->colors->reset);
			break;
		case RNN_TTYPE_INT:
			if (value & UINT64_C(1) << (width-1))
				asprintf (&res, "%s-%"PRIi64"%s", ctx->colors->num, (UINT64_C(1) << width) - value, ctx->colors->reset);
			else
				asprintf (&res, "%s%"PRIi64"%s", ctx->colors->num, value, ctx->colors->reset);
			break;
		case RNN_TTYPE_BOOLEAN:
			if (value == 0) {
				asprintf (&res, "%sFALSE%s", ctx->colors->eval, ctx->colors->reset);
			} else if (value == 1) {
				asprintf (&res, "%sTRUE%s", ctx->colors->eval, ctx->colors->reset);
			}
			break;
		case RNN_TTYPE_FLOAT: {
			union { uint64_t i; float f; double d; } val;
			val.i = value;
			if (width == 64)
				asprintf(&res, "%s%f%s", ctx->colors->num,
					val.d, ctx->colors->reset);
			else if (width == 32)
				asprintf(&res, "%s%f%s", ctx->colors->num,
					val.f, ctx->colors->reset);
			else if (width == 16)
				asprintf(&res, "%s%f%s", ctx->colors->num,
					float16(value), ctx->colors->reset);
			else
				goto failhex;

			break;
		}
		failhex:
		default:
			asprintf (&res, "%s%#"PRIx64"%s", ctx->colors->num, value, ctx->colors->reset);
			break;
	}
	if (value_orig & ~typeinfo_mask(ti)) {
		asprintf (&tmp, "%s | %s%#"PRIx64"%s", res, ctx->colors->err, value_orig & ~typeinfo_mask(ti), ctx->colors->reset);
		free(res);
		res = tmp;
	}
	return res;
}

static char *appendidx (struct rnndeccontext *ctx, char *name, uint64_t idx, struct rnnenum *index) {
	char *res;
	const char *index_name = NULL;

	if (index)
		index_name = rnndec_decode_enum_val(ctx, index->vals, index->valsnum, idx);

	if (index_name)
		asprintf (&res, "%s[%s%s%s]", name, ctx->colors->eval, index_name, ctx->colors->reset);
	else
		asprintf (&res, "%s[%s%#"PRIx64"%s]", name, ctx->colors->num, idx, ctx->colors->reset);

	free (name);
	return res;
}

/* This could probably be made to work for stripes too.. */
static int get_array_idx_offset(struct rnndelem *elem, uint64_t addr, uint64_t *idx, uint64_t *offset)
{
	if (elem->offsets) {
		int i;
		for (i = 0; i < elem->offsetsnum; i++) {
			uint64_t o = elem->offsets[i];
			if ((o <= addr) && (addr < (o + elem->stride))) {
				*idx = i;
				*offset = addr - o;
				return 0;
			}
		}
		return -1;
	} else {
		if (addr < elem->offset)
			return -1;

		*idx = (addr - elem->offset) / elem->stride;
		*offset = (addr - elem->offset) % elem->stride;

		if (elem->length && (*idx >= elem->length))
			return -1;

		return 0;
	}
}

static struct rnndecaddrinfo *trymatch (struct rnndeccontext *ctx, struct rnndelem **elems, int elemsnum, uint64_t addr, int write, int dwidth, uint64_t *indices, int indicesnum) {
	struct rnndecaddrinfo *res;
	int i, j;
	for (i = 0; i < elemsnum; i++) {
		if (!rnndec_varmatch(ctx, &elems[i]->varinfo))
			continue;
		uint64_t offset, idx;
		char *tmp, *name;
		switch (elems[i]->type) {
			case RNN_ETYPE_REG:
				if (addr < elems[i]->offset)
					break;
				if (elems[i]->stride) {
					idx = (addr-elems[i]->offset)/elems[i]->stride;
					offset = (addr-elems[i]->offset)%elems[i]->stride;
				} else {
					idx = 0;
					offset = addr-elems[i]->offset;
				}
				if (offset >= elems[i]->width/dwidth)
					break;
				if (elems[i]->length && idx >= elems[i]->length)
					break;
				res = calloc (sizeof *res, 1);
				res->typeinfo = &elems[i]->typeinfo;
				res->width = elems[i]->width;
				asprintf (&res->name, "%s%s%s", ctx->colors->rname, elems[i]->name, ctx->colors->reset);
				for (j = 0; j < indicesnum; j++)
					res->name = appendidx(ctx, res->name, indices[j], NULL);
				if (elems[i]->length != 1)
					res->name = appendidx(ctx, res->name, idx, elems[i]->index);
				if (offset) {
					/* use _HI suffix for addresses */
					if (offset == 1 &&
						(!strcmp(res->typeinfo->name, "address") ||
						 !strcmp(res->typeinfo->name, "waddress")))  {
						asprintf (&tmp, "%s_HI", res->name);
					} else {
						asprintf (&tmp, "%s+%s%#"PRIx64"%s", res->name, ctx->colors->err, offset, ctx->colors->reset);
					}
					free(res->name);
					res->name = tmp;
				}
				return res;
			case RNN_ETYPE_STRIPE:
				for (idx = 0; idx < elems[i]->length || !elems[i]->length; idx++) {
					if (addr < elems[i]->offset + elems[i]->stride * idx)
						break;
					offset = addr - (elems[i]->offset + elems[i]->stride * idx);
					int extraidx = (elems[i]->length != 1);
					int nindnum = (elems[i]->name ? 0 : indicesnum + extraidx);
					uint64_t nind[MAX2(nindnum, 1)];
					if (!elems[i]->name) {
						for (j = 0; j < indicesnum; j++)
							nind[j] = indices[j];
						if (extraidx)
							nind[indicesnum] = idx;
					}
					res = trymatch (ctx, elems[i]->subelems, elems[i]->subelemsnum, offset, write, dwidth, nind, nindnum);
					if (!res)
						continue;
					if (!elems[i]->name)
						return res;
					asprintf (&name, "%s%s%s", ctx->colors->rname, elems[i]->name, ctx->colors->reset);
					for (j = 0; j < indicesnum; j++)
						name = appendidx(ctx, name, indices[j], NULL);
					if (elems[i]->length != 1)
						name = appendidx(ctx, name, idx, elems[i]->index);
					asprintf (&tmp, "%s.%s", name, res->name);
					free(name);
					free(res->name);
					res->name = tmp;
					return res;
				}
				break;
			case RNN_ETYPE_ARRAY:
				if (get_array_idx_offset(elems[i], addr, &idx, &offset))
					break;
				asprintf (&name, "%s%s%s", ctx->colors->rname, elems[i]->name, ctx->colors->reset);
				for (j = 0; j < indicesnum; j++)
					name = appendidx(ctx, name, indices[j], NULL);
				if (elems[i]->length != 1)
					name = appendidx(ctx, name, idx, elems[i]->index);
				if ((res = trymatch (ctx, elems[i]->subelems, elems[i]->subelemsnum, offset, write, dwidth, 0, 0))) {
					asprintf (&tmp, "%s.%s", name, res->name);
					free(name);
					free(res->name);
					res->name = tmp;
					return res;
				}
				res = calloc (sizeof *res, 1);
				asprintf (&tmp, "%s+%s%#"PRIx64"%s", name, ctx->colors->err, offset, ctx->colors->reset);
				free(name);
				res->name = tmp;
				return res;
			default:
				break;
		}
	}
	return 0;
}

int rnndec_checkaddr(struct rnndeccontext *ctx, struct rnndomain *domain, uint64_t addr, int write) {
	struct rnndecaddrinfo *res = trymatch(ctx, domain->subelems, domain->subelemsnum, addr, write, domain->width, 0, 0);
	if (res) {
		free(res->name);
		free(res);
	}
	return res != NULL;
}

struct rnndecaddrinfo *rnndec_decodeaddr(struct rnndeccontext *ctx, struct rnndomain *domain, uint64_t addr, int write) {
	struct rnndecaddrinfo *res = trymatch(ctx, domain->subelems, domain->subelemsnum, addr, write, domain->width, 0, 0);
	if (res)
		return res;
	res = calloc (sizeof *res, 1);
	asprintf (&res->name, "%s%#"PRIx64"%s", ctx->colors->err, addr, ctx->colors->reset);
	return res;
}

static unsigned tryreg(struct rnndeccontext *ctx, struct rnndelem **elems, int elemsnum,
		int dwidth, const char *name, uint64_t *offset)
{
	int i;
	unsigned ret;
	const char *suffix = strchr(name, '[');
	unsigned n = suffix ? (suffix - name) : strlen(name);
	const char *dotsuffix = strchr(name, '.');
	unsigned dotn = dotsuffix ? (dotsuffix - name) : strlen(name);

	const char *child = NULL;
	unsigned idx = 0;

	if (suffix) {
		const char *tmp = strchr(suffix, ']');
		idx = strtol(suffix+1, NULL, 0);
		child = tmp+2;
	}

	for (i = 0; i < elemsnum; i++) {
		struct rnndelem *elem = elems[i];
		if (!rnndec_varmatch(ctx, &elem->varinfo))
			continue;
		int match = elem->name && (strlen(elem->name) == n) && !strncmp(elem->name, name, n);
		switch (elem->type) {
			case RNN_ETYPE_REG:
				if (match) {
					assert(!suffix);
					*offset = elem->offset;
					return 1;
				}
				break;
			case RNN_ETYPE_STRIPE:
				if (elem->name) {
					if (!dotsuffix)
						break;
					if (strlen(elem->name) != dotn || strncmp(elem->name, name, dotn))
						break;
				}
				ret = tryreg(ctx, elem->subelems, elem->subelemsnum, dwidth,
					elem->name ? dotsuffix : name, offset);
				if (ret)
					return 1;
				break;
			case RNN_ETYPE_ARRAY:
				if (match) {
					assert(suffix);
					ret = tryreg(ctx, elem->subelems, elem->subelemsnum, dwidth, child, offset);
					if (ret) {
						*offset += elem->offset + (idx * elem->stride);
						return 1;
					}
				}
				break;
			default:
				break;
		}
	}
	return 0;
}

uint64_t rnndec_decodereg(struct rnndeccontext *ctx, struct rnndomain *domain, const char *name)
{
	uint64_t offset;
	if (tryreg(ctx, domain->subelems, domain->subelemsnum, domain->width, name, &offset)) {
		return offset;
	} else {
		return 0;
	}
}
