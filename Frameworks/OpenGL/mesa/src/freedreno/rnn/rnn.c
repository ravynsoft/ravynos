/*
 * Copyright (C) 2010-2011 Marcin Kościelnicki <koriakin@0x04.net>
 * Copyright (C) 2010 Luca Barbieri <luca@luca-barbieri.com>
 * Copyright (C) 2010 Francisco Jerez <currojerez@riseup.net>
 * Copyright (C) 2010 Martin Peres <martin.peres@ensi-bourges.fr>
 * Copyright (C) 2010 Marcin Slusarz <marcin.slusarz@gmail.com>
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

/* workaround libxml2 silliness: */
#pragma GCC diagnostic ignored "-Wpointer-sign"

#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlreader.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include "rnn.h"
#include "util.h"

#include "util/u_debug.h"

static char *catstr (char *a, char *b) {
	if (!a)
		return b;
	return aprintf("%s_%s", a, b);
}

static int strdiff (const char *a, const char *b) {
	if (!a && !b)
		return 0;
	if (!a || !b)
		return 1;
	return strcmp (a, b);
}

static void rnn_err(struct rnndb *db, const char *format, ...) _util_printf_format(2, 3);

static void rnn_err(struct rnndb *db, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	db->estatus = 1;
}

void rnn_init(void) {
	LIBXML_TEST_VERSION
	xmlInitParser();
}

struct rnndb *rnn_newdb(void) {
	struct rnndb *db = calloc(sizeof *db, 1);
	return db;
}

static char *getcontent (xmlNode *attr) {
	xmlNode *chain = attr->children;
	size_t size = 0;
	char *content, *p;
	while (chain) {
		if (chain->type == XML_TEXT_NODE)
			size += strlen(chain->content);
		chain = chain->next;
	}
	p = content = malloc(size + 1);
	chain = attr->children;
	while (chain) {
		if (chain->type == XML_TEXT_NODE) {
			char* sp = chain->content;
			if(p == content) {
				while(isspace(*sp))
					++sp;
			}
			size_t len = strlen(sp);
			memcpy(p, sp, len);
			p += len;
		}
		chain = chain->next;
	}
	while(p != content && isspace(p[-1]))
		--p;
	*p = 0;
	return content;
}

static char *getattrib (struct rnndb *db, char *file, int line, xmlAttr *attr) {
	xmlNode *chain = attr->children;
	while (chain) {
		if (chain->type != XML_TEXT_NODE) {
			rnn_err(db, "%s:%d: unknown attribute child \"%s\" in attribute \"%s\"\n", file, line, chain->name, attr->name);
		} else {
			return chain->content;
		}
		chain = chain->next;
	}
	return "";
}

static int getboolattrib (struct rnndb *db, char *file, int line, xmlAttr *attr) {
	char *c = getattrib(db, file, line, attr);
	if (!strcmp(c, "yes") || !strcmp(c, "1") || !strcmp(c, "true"))
		return 1;
	if (!strcmp(c, "no") || !strcmp(c, "0") || !strcmp(c, "false"))
		return 0;
	rnn_err(db, "%s:%d: invalid boolean value \"%s\" in attribute \"%s\"\n", file, line, c, attr->name);
	return 0;
}

static uint64_t getnum(struct rnndb *db, char *file, int line, xmlAttr *attr, char *c)
{
	char *cc;
	uint64_t res;
	if (strchr(c, 'x') || strchr(c, 'X'))
		res = strtoull(c, &cc, 16);
	else
		res = strtoull(c, &cc, 10);
	if (*cc)  {
		rnn_err(db, "%s:%d: invalid numeric value \"%s\" in attribute \"%s\"\n", file, line, c, attr->name);
	}
	return res;
}

static uint64_t getnumattrib (struct rnndb *db, char *file, int line, xmlAttr *attr) {
	char *c = getattrib(db, file, line, attr);
	return getnum(db, file, line, attr, c);
}

static int trytop (struct rnndb *db, char *file, xmlNode *node);

static int trydoc (struct rnndb *db, char *file, xmlNode *node) {
	if (!strcmp(node->name, "brief")) {
		return 1;
	} else if (!strcmp(node->name, "doc")) {
		return 1;
	}
	return 0;
}

static struct rnnvalue *parsevalue(struct rnndb *db, char *file, xmlNode *node);
static struct rnnbitfield *parsebitfield(struct rnndb *db, char *file, xmlNode *node);

static int trytypetag (struct rnndb *db, char *file, xmlNode *node, struct rnntypeinfo *ti) {
	if (!strcmp(node->name, "value")) {
		struct rnnvalue *val = parsevalue(db, file, node);
		if (val)
			ADDARRAY(ti->vals, val);
		return 1;
	} else if (!strcmp(node->name, "bitfield")) {
		struct rnnbitfield *bf = parsebitfield(db, file, node);
		if (bf)
			ADDARRAY(ti->bitfields, bf);
		return 1;
	}
	return 0;
}
static int trytypeattr (struct rnndb *db, char *file, xmlNode *node, xmlAttr *attr, struct rnntypeinfo *ti) {
	if (!strcmp(attr->name, "shr")) {
		ti->shr = getnumattrib(db, file, node->line, attr);
		return 1;
	} else if (!strcmp(attr->name, "min")) {
		ti->min = getnumattrib(db, file, node->line, attr);
		ti->minvalid = 1;
		return 1;
	} else if (!strcmp(attr->name, "max")) {
		ti->max = getnumattrib(db, file, node->line, attr);
		ti->maxvalid = 1;
		return 1;
	} else if (!strcmp(attr->name, "align")) {
		ti->align = getnumattrib(db, file, node->line, attr);
		ti->alignvalid = 1;
		return 1;
	} else if (!strcmp(attr->name, "type")) {
		ti->name = strdup(getattrib(db, file, node->line, attr));;
		return 1;
	} else if (!strcmp(attr->name, "radix")) {
		ti->radix = getnumattrib(db, file, node->line, attr);
		ti->radixvalid = 1;
		return 1;
	} else if (!strcmp(attr->name, "pos")) {
		ti->high = ti->low = getnumattrib(db, file, node->line, attr);
		return 1;
	} else if (!strcmp(attr->name, "low")) {
		ti->low = getnumattrib(db, file, node->line, attr);
		return 1;
	} else if (!strcmp(attr->name, "high")) {
		ti->high = getnumattrib(db, file, node->line, attr);
		return 1;
	} else if (!strcmp(attr->name, "addvariant")) {
		ti->addvariant = getboolattrib(db, file, node->line, attr);
		return 1;
	}
	return 0;
}

static struct rnnvalue *parsevalue(struct rnndb *db, char *file, xmlNode *node) {
	struct rnnvalue *val = calloc(sizeof *val, 1);
	val->file = file;
	xmlAttr *attr = node->properties;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			val->name = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "value")) {
			val->value = getnumattrib(db, file, node->line, attr);
			val->valvalid = 1;
		} else if (!strcmp(attr->name, "varset")) {
			val->varinfo.varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			val->varinfo.variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for value\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in %s: <%s>\n", file, chain->line, node->name, chain->name);
		}
		chain = chain->next;
	}
	if (!val->name) {
		rnn_err(db, "%s:%d: nameless value\n", file, node->line);
		return 0;
	} else {
		return val;
	}
}

static void parsespectype(struct rnndb *db, char *file, xmlNode *node) {
	struct rnnspectype *res = calloc (sizeof *res, 1);
	res->file = file;
	xmlAttr *attr = node->properties;
	int i;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			res->name = strdup(getattrib(db, file, node->line, attr));
		} else if (!trytypeattr(db, file, node, attr, &res->typeinfo)) {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for spectype\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	if (!res->name) {
		rnn_err(db, "%s:%d: nameless spectype\n", file, node->line);
		return;
	}
	for (i = 0; i < db->spectypesnum; i++)
		if (!strcmp(db->spectypes[i]->name, res->name)) {
			rnn_err(db, "%s:%d: duplicated spectype name %s\n", file, node->line, res->name);
			return;
		}
	ADDARRAY(db->spectypes, res);
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!trytypetag(db, file, chain, &res->typeinfo) && !trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in spectype: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static void parseenum(struct rnndb *db, char *file, xmlNode *node) {
	xmlAttr *attr = node->properties;
	char *name = 0;
	int isinline = 0;
	int bare = 0;
	char *prefixstr = 0;
	char *varsetstr = 0;
	char *variantsstr = 0;
	int i;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			name = getattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "bare")) {
			bare = getboolattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "inline")) {
			isinline = getboolattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "prefix")) {
			prefixstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "varset")) {
			varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for enum\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	if (!name) {
		rnn_err(db, "%s:%d: nameless enum\n", file, node->line);
		return;
	}
	struct rnnenum *cur = 0;
	for (i = 0; i < db->enumsnum; i++)
		if (!strcmp(db->enums[i]->name, name)) {
			cur = db->enums[i];
			break;
		}
	if (cur) {
		if (strdiff(cur->varinfo.prefixstr, prefixstr) ||
				strdiff(cur->varinfo.varsetstr, varsetstr) ||
				strdiff(cur->varinfo.variantsstr, variantsstr) ||
				cur->isinline != isinline || cur->bare != bare) {
			rnn_err(db, "%s:%d: merge fail for enum %s\n", file, node->line, node->name);
		}
	} else {
		cur = calloc(sizeof *cur, 1);
		cur->name = strdup(name);
		cur->isinline = isinline;
		cur->bare = bare;
		cur->varinfo.prefixstr = prefixstr;
		cur->varinfo.varsetstr = varsetstr;
		cur->varinfo.variantsstr = variantsstr;
		cur->file = file;
		ADDARRAY(db->enums, cur);
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!strcmp(chain->name, "value")) {
			struct rnnvalue *val = parsevalue(db, file, chain);
			if (val)
				ADDARRAY(cur->vals, val);
		} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in enum: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static struct rnnbitfield *parsebitfield(struct rnndb *db, char *file, xmlNode *node) {
	struct rnnbitfield *bf = calloc(sizeof *bf, 1);
	bf->file = file;
	xmlAttr *attr = node->properties;
	bf->typeinfo.low = bf->typeinfo.high = -1;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			bf->name = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "varset")) {
			bf->varinfo.varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			bf->varinfo.variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!trytypeattr(db, file, node, attr, &bf->typeinfo)) {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for bitfield\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!trytypetag(db, file, chain, &bf->typeinfo) && !trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in %s: <%s>\n", file, chain->line, node->name, chain->name);
		}
		chain = chain->next;
	}
	if (!bf->name) {
		rnn_err(db, "%s:%d: nameless bitfield\n", file, node->line);
		return 0;
	} else if (bf->typeinfo.low < 0|| bf->typeinfo.high < 0 || bf->typeinfo.high < bf->typeinfo.low) {
		rnn_err(db, "%s:%d: bitfield has wrong placement\n", file, node->line);
		return 0;
	} else {
		return bf;
	}
}

static void parsebitset(struct rnndb *db, char *file, xmlNode *node) {
	xmlAttr *attr = node->properties;
	char *name = 0;
	int isinline = 0;
	int bare = 0;
	char *prefixstr = 0;
	char *varsetstr = 0;
	char *variantsstr = 0;
	int i;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			name = getattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "bare")) {
			bare = getboolattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "inline")) {
			isinline = getboolattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "prefix")) {
			prefixstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "varset")) {
			varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for bitset\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	if (!name) {
		rnn_err(db, "%s:%d: nameless bitset\n", file, node->line);
		return;
	}
	struct rnnbitset *cur = 0;
	for (i = 0; i < db->bitsetsnum; i++)
		if (!strcmp(db->bitsets[i]->name, name)) {
			cur = db->bitsets[i];
			break;
		}
	if (cur) {
		if (strdiff(cur->varinfo.prefixstr, prefixstr) ||
				strdiff(cur->varinfo.varsetstr, varsetstr) ||
				strdiff(cur->varinfo.variantsstr, variantsstr) ||
				cur->isinline != isinline || cur->bare != bare) {
			rnn_err(db, "%s:%d: merge fail for bitset %s\n", file, node->line, node->name);
		}
	} else {
		cur = calloc(sizeof *cur, 1);
		cur->name = strdup(name);
		cur->isinline = isinline;
		cur->bare = bare;
		cur->varinfo.prefixstr = prefixstr;
		cur->varinfo.varsetstr = varsetstr;
		cur->varinfo.variantsstr = variantsstr;
		cur->file = file;
		ADDARRAY(db->bitsets, cur);
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!strcmp(chain->name, "bitfield")) {
			struct rnnbitfield *bf = parsebitfield(db, file, chain);
			if (bf)
				ADDARRAY(cur->bitfields, bf);
		} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in bitset: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static struct rnndelem *trydelem(struct rnndb *db, char *file, xmlNode *node) {
	if (!strcmp(node->name, "use-group")) {
		struct rnndelem *res = calloc(sizeof *res, 1);
		res->file = file;
		res->type = RNN_ETYPE_USE_GROUP;
		xmlAttr *attr = node->properties;
		while (attr) {
			if (!strcmp(attr->name, "ref")) {
				res->name = strdup(getattrib(db, file, node->line, attr));
			} else {
				rnn_err(db, "%s:%d: wrong attribute \"%s\" for %s\n", file, node->line, attr->name, node->name);
			}
			attr = attr->next;
		}
		if (!res->name) {
			rnn_err(db, "%s:%d: nameless use-group\n", file, node->line);
			return 0;
		}
		return res;
	} else if (!strcmp(node->name, "stripe") || !strcmp(node->name, "array")) {
		struct rnndelem *res = calloc(sizeof *res, 1);
		if (!strcmp(node->name, "array"))
			res->name = "";
		res->type = (strcmp(node->name, "stripe")?RNN_ETYPE_ARRAY:RNN_ETYPE_STRIPE);
		res->length = 1;
		res->file = file;
		xmlAttr *attr = node->properties;
		while (attr) {
			if (!strcmp(attr->name, "name")) {
				res->name = strdup(getattrib(db, file, node->line, attr));
			} else if (!strcmp(attr->name, "offset")) {
				res->offset = getnumattrib(db, file, node->line, attr);
			} else if (!strcmp(attr->name, "offsets")) {
				char *str = strdup(getattrib(db, file, node->line, attr));
				char *tok, *save, *tmp = str;
				while ((tok = strtok_r(str, ",", &save))) {
					uint64_t offset = getnum(db, file, node->line, attr, tok);
					ADDARRAY(res->offsets, offset);
					str = NULL;
				}
				if (str)
					fprintf(stderr, "%s:%d: invalid offsets: %s\n", file, node->line, str);
				free(tmp);
			} else if (!strcmp(attr->name, "doffset")) {
				/* dynamic runtime determined offset: */
				res->doffset = strdup(getattrib(db, file, node->line, attr));
			} else if (!strcmp(attr->name, "doffsets")) {
				/* dynamic runtime determined offsets: */
				char *str = strdup(getattrib(db, file, node->line, attr));
				char *tok, *save, *tmp = str;
				while ((tok = strtok_r(str, ",", &save))) {
					char *doffset = strdup(tok);
					ADDARRAY(res->doffsets, doffset);
					str = NULL;
				}
				if (str)
					fprintf(stderr, "%s:%d: invalid offsets: %s\n", file, node->line, str);
				free(tmp);
			} else if (!strcmp(attr->name, "length")) {
				res->length = getnumattrib(db, file, node->line, attr);
			} else if (!strcmp(attr->name, "stride")) {
				res->stride = getnumattrib(db, file, node->line, attr);
			} else if (!strcmp(attr->name, "prefix")) {
				res->varinfo.prefixstr = strdup(getattrib(db, file, node->line, attr));
			} else if (!strcmp(attr->name, "varset")) {
				res->varinfo.varsetstr = strdup(getattrib(db, file, node->line, attr));
			} else if (!strcmp(attr->name, "variants")) {
				res->varinfo.variantsstr = strdup(getattrib(db, file, node->line, attr));
			} else if (!strcmp(attr->name, "index")) {
				const char *enumname = getattrib(db, file, node->line, attr);
				res->index = rnn_findenum(db, enumname);
				if (!res->index) {
					rnn_err(db, "%s:%d: invalid enum name \"%s\"\n", file, node->line, enumname);
				}
			} else if (!strcmp(attr->name, "usage")) {
				// no-op
			} else {
				rnn_err(db, "%s:%d: wrong attribute \"%s\" for %s\n", file, node->line, attr->name, node->name);
			}
			attr = attr->next;
		}
		xmlNode *chain = node->children;
		while (chain) {
			struct rnndelem *delem;
			if (chain->type != XML_ELEMENT_NODE) {
			} else if ((delem = trydelem(db, file, chain))) {
				ADDARRAY(res->subelems, delem);
			} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
				rnn_err(db, "%s:%d: wrong tag in %s: <%s>\n", file, chain->line, node->name, chain->name);
			}
			chain = chain->next;
		}

		/* Sanity checking */
		if (res->type == RNN_ETYPE_ARRAY && res->stride == 0) {
			fprintf(stderr, "%s: Array %s's stride is undefined. Aborting.\n", file, res->name);
			exit(-1);
		}
		return res;

	}
	int width;
	if (!strcmp(node->name, "reg8"))
		width = 8;
	else if (!strcmp(node->name, "reg16"))
		width = 16;
	else if (!strcmp(node->name, "reg32"))
		width = 32;
	else if (!strcmp(node->name, "reg64"))
		width = 64;
	else
		return 0;
	struct rnndelem *res = calloc(sizeof *res, 1);
	res->file = file;
	res->type = RNN_ETYPE_REG;
	res->width = width;
	res->length = 1;
	res->access = RNN_ACCESS_RW;
	xmlAttr *attr = node->properties;
	res->typeinfo.low = 0;
	res->typeinfo.high = width - 1;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			res->name = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "offset")) {
			res->offset = getnumattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "length")) {
			res->length = getnumattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "stride")) {
			res->stride = getnumattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "varset")) {
			res->varinfo.varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			res->varinfo.variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "access")) {
			char *str = getattrib(db, file, node->line, attr);
			if (!strcmp(str, "r"))
				res->access = RNN_ACCESS_R;
			else if (!strcmp(str, "w"))
				res->access = RNN_ACCESS_W;
			else if (!strcmp(str, "rw"))
				res->access = RNN_ACCESS_RW;
			else
				fprintf (stderr, "%s:%d: wrong access type \"%s\" for register\n", file, node->line, str);
		} else if (!strcmp(attr->name, "usage")) {
			// no-op
		} else if (!trytypeattr(db, file, node, attr, &res->typeinfo)) {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for register\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!trytypetag(db, file, chain, &res->typeinfo) && !trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in %s: <%s>\n", file, chain->line, node->name, chain->name);
		}
		chain = chain->next;
	}
	if (!res->name) {
		rnn_err(db, "%s:%d: nameless register\n", file, node->line);
		return 0;
	} else {
	}
	return res;
}

static void parsegroup(struct rnndb *db, char *file, xmlNode *node) {
	xmlAttr *attr = node->properties;
	char *name = 0;
	int i;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			name = getattrib(db, file, node->line, attr);
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for group\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	if (!name) {
		rnn_err(db, "%s:%d: nameless group\n", file, node->line);
		return;
	}
	struct rnngroup *cur = 0;
	for (i = 0; i < db->groupsnum; i++)
		if (!strcmp(db->groups[i]->name, name)) {
			cur = db->groups[i];
			break;
		}
	if (!cur) {
		cur = calloc(sizeof *cur, 1);
		cur->name = strdup(name);
		ADDARRAY(db->groups, cur);
	}
	xmlNode *chain = node->children;
	while (chain) {
		struct rnndelem *delem;
		if (chain->type != XML_ELEMENT_NODE) {
		} else if ((delem = trydelem(db, file, chain))) {
			ADDARRAY(cur->subelems, delem);
		} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in group: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static void parsedomain(struct rnndb *db, char *file, xmlNode *node) {
	xmlAttr *attr = node->properties;
	char *name = 0;
	uint64_t size = 0; int width = 8;
	int bare = 0;
	char *prefixstr = 0;
	char *varsetstr = 0;
	char *variantsstr = 0;
	int i;
	while (attr) {
		if (!strcmp(attr->name, "name")) {
			name = getattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "bare")) {
			bare = getboolattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "size")) {
			size = getnumattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "width")) {
			width = getnumattrib(db, file, node->line, attr);
		} else if (!strcmp(attr->name, "prefix")) {
			prefixstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "varset")) {
			varsetstr = strdup(getattrib(db, file, node->line, attr));
		} else if (!strcmp(attr->name, "variants")) {
			variantsstr = strdup(getattrib(db, file, node->line, attr));
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for domain\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	if (!name) {
		rnn_err(db, "%s:%d: nameless domain\n", file, node->line);
		return;
	}
	struct rnndomain *cur = 0;
	for (i = 0; i < db->domainsnum; i++)
		if (!strcmp(db->domains[i]->name, name)) {
			cur = db->domains[i];
			break;
		}
	if (cur) {
		if (strdiff(cur->varinfo.prefixstr, prefixstr) ||
				strdiff(cur->varinfo.varsetstr, varsetstr) ||
				strdiff(cur->varinfo.variantsstr, variantsstr) ||
				cur->width != width ||
				cur->bare != bare ||
				(size && cur->size && size != cur->size)) {
			rnn_err(db, "%s:%d: merge fail for domain %s\n", file, node->line, node->name);
		} else {
			if (size)
				cur->size = size;
		}
	} else {
		cur = calloc(sizeof *cur, 1);
		cur->name = strdup(name);
		cur->bare = bare;
		cur->width = width;
		cur->size = size;
		cur->varinfo.prefixstr = prefixstr;
		cur->varinfo.varsetstr = varsetstr;
		cur->varinfo.variantsstr = variantsstr;
		cur->file = file;
		ADDARRAY(db->domains, cur);
	}
	xmlNode *chain = node->children;
	while (chain) {
		struct rnndelem *delem;
		if (chain->type != XML_ELEMENT_NODE) {
		} else if ((delem = trydelem(db, file, chain))) {
			ADDARRAY(cur->subelems, delem);
		} else if (!trytop(db, file, chain) && !trydoc(db, file, chain)) {
			rnn_err(db, "%s:%d: wrong tag in domain: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static void parsecopyright(struct rnndb *db, char *file, xmlNode *node) {
	struct rnncopyright* copyright = &db->copyright;
	xmlAttr *attr = node->properties;
	while (attr) {
		if (!strcmp(attr->name, "year")) {
			unsigned firstyear = getnumattrib(db, file, node->line, attr);
			if(!copyright->firstyear || firstyear < copyright->firstyear)
				copyright->firstyear = firstyear;
		} else {
			rnn_err(db, "%s:%d: wrong attribute \"%s\" for copyright\n", file, node->line, attr->name);
		}
		attr = attr->next;
	}
	xmlNode *chain = node->children;
	while (chain) {
		if (chain->type != XML_ELEMENT_NODE) {
		} else if (!strcmp(chain->name, "license"))
			if(copyright->license) {
				if(strcmp(copyright->license, node->content)) {
					fprintf(stderr, "fatal error: multiple different licenses specified!\n");
					abort(); /* TODO: do something better here, but headergen, xml2html, etc. should not produce anything in this case */
				}
			} else
				copyright->license = getcontent(chain);
		else if (!strcmp(chain->name, "author")) {
			struct rnnauthor* author = calloc(sizeof *author, 1);
			xmlAttr* authorattr = chain->properties;
			xmlNode *authorchild = chain->children;
			author->contributions = getcontent(chain);
			while (authorattr) {
				if (!strcmp(authorattr->name, "name"))
					author->name = strdup(getattrib(db, file, chain->line, authorattr));
				else if (!strcmp(authorattr->name, "email"))
					author->email = strdup(getattrib(db, file, chain->line, authorattr));
				else {
					rnn_err(db, "%s:%d: wrong attribute \"%s\" for author\n", file, chain->line, authorattr->name);
				}
				authorattr = authorattr->next;
			}
			while(authorchild)  {
				if (authorchild->type != XML_ELEMENT_NODE) {
				} else if (!strcmp(authorchild->name, "nick")) {
					xmlAttr* nickattr = authorchild->properties;
					char* nickname = 0;
					while(nickattr) {
						if (!strcmp(nickattr->name, "name"))
							nickname = strdup(getattrib(db, file, authorchild->line, nickattr));
						else {
							rnn_err(db, "%s:%d: wrong attribute \"%s\" for nick\n", file, authorchild->line, nickattr->name);
						}
						nickattr = nickattr->next;
					}
					if(!nickname) {
						rnn_err(db, "%s:%d: missing \"name\" attribute for nick\n", file, authorchild->line);
					} else
						ADDARRAY(author->nicknames, nickname);
				} else {
					rnn_err(db, "%s:%d: wrong tag in author: <%s>\n", file, authorchild->line, authorchild->name);
				}
				authorchild = authorchild->next;
			}
			ADDARRAY(copyright->authors, author);
		} else {
			rnn_err(db, "%s:%d: wrong tag in copyright: <%s>\n", file, chain->line, chain->name);
		}
		chain = chain->next;
	}
}

static int trytop (struct rnndb *db, char *file, xmlNode *node) {
	if (!strcmp(node->name, "enum")) {
		parseenum(db, file, node);
		return 1;
	} else if (!strcmp(node->name, "bitset")) {
		parsebitset(db, file, node);
		return 1;
	} else if (!strcmp(node->name, "group")) {
		parsegroup(db, file, node);
		return 1;
	} else if (!strcmp(node->name, "domain")) {
		parsedomain(db, file, node);
		return 1;
	} else if (!strcmp(node->name, "spectype")) {
		parsespectype(db, file, node);
		return 1;
	} else if (!strcmp(node->name, "import")) {
		xmlAttr *attr = node->properties;
		char *subfile = 0;
		while (attr) {
			if (!strcmp(attr->name, "file")) {
				subfile = getattrib(db, file, node->line, attr);
			} else {
				rnn_err(db, "%s:%d: wrong attribute \"%s\" for import\n", file, node->line, attr->name);
			}
			attr = attr->next;
		}
		if (!subfile) {
			rnn_err(db, "%s:%d: missing \"file\" attribute for import\n", file, node->line);
		} else {
			rnn_parsefile(db, subfile);
		}
		return 1;
	} else if (!strcmp(node->name, "copyright")) {
		parsecopyright(db, file, node);
		return 1;
	}
	return 0;
}

static char * find_file(const char *file_orig)
{
	const char *rnn_path = getenv("RNN_PATH");
	char *fname;

	if (!rnn_path)
		rnn_path = RNN_DEF_PATH;

	FILE *file = find_in_path(file_orig, rnn_path, &fname);
	if (!file) {
		fprintf (stderr, "%s: couldn't find database file. Please set the env var RNN_PATH.\n", file_orig);
		return NULL;
	}
	fclose(file);

	return fname;
}

static int validate_doc(struct rnndb *db, xmlDocPtr doc, xmlNodePtr database)
{
	/* find the schemaLocation property: */
	xmlAttrPtr attr = database->properties;
	const char *schema_name = NULL;
	char *schema_path;

	while (attr) {
		if (!strcmp(attr->name, "schemaLocation")) {
			xmlNodePtr data = attr->children;
			schema_name = data->content;
			/* we expect this to look like <namespace url> schema.xsd.. I think
			 * technically it is supposed to be just a URL, but that doesn't
			 * quite match up to what we do.. Just skip over everything up to
			 * and including the first whitespace character:
			 */
			while (schema_name && (schema_name[0] != ' '))
				schema_name++;
			schema_name++;
			break;
		}
	}

	if (!schema_name) {
		rnn_err(db, "could not find schema.  Missing schemaLocation?");
		return 0;
	}

	schema_path = find_file(schema_name);
	if (!schema_path) {
		rnn_err(db, "%s: couldn't find database file. Please set the env var RNN_PATH.\n", schema_name);
		return 0;
	}

	xmlSchemaParserCtxtPtr parser = xmlSchemaNewParserCtxt(schema_path);
	xmlSchemaPtr schema = xmlSchemaParse(parser);
	xmlSchemaValidCtxtPtr validCtxt = xmlSchemaNewValidCtxt(schema);
	int ret = xmlSchemaValidateDoc(validCtxt, doc);

	xmlSchemaFreeValidCtxt(validCtxt);
	xmlSchemaFree(schema);
	xmlSchemaFreeParserCtxt(parser);

	free(schema_path);

	return ret;
}

void rnn_parsefile (struct rnndb *db, char *file_orig) {
	int i;
	char *fname;

	fname = find_file(file_orig);
	if (!fname) {
		db->estatus = 1;
		return;
	}

	for (i = 0; i < db->filesnum; i++)
		if (!strcmp(db->files[i], fname))
			return;
		
	ADDARRAY(db->files, fname);
	xmlDocPtr doc = xmlParseFile(fname);
	if (!doc) {
		rnn_err(db, "%s: couldn't open database file. Please set the env var RNN_PATH.\n", fname);
		return;
	}
	xmlNode *root = doc->children;
	while (root) {
		if (root->type != XML_ELEMENT_NODE) {
		} else if (strcmp(root->name, "database")) {
			rnn_err(db, "%s:%d: wrong top-level tag <%s>\n", fname, root->line, root->name);
		} else {
			xmlNode *chain = root->children;
			if (validate_doc(db, doc, root)) {
				rnn_err(db, "%s: database file has errors\n", fname);
				return;
			}
			while (chain) {
				if (chain->type != XML_ELEMENT_NODE) {
				} else if (!trytop(db, fname, chain) && !trydoc(db, fname, chain)) {
					rnn_err(db, "%s:%d: wrong tag in database: <%s>\n", fname, chain->line, chain->name);
				}
				chain = chain->next;
			}
		}
		root = root->next;
	}
	xmlFreeDoc(doc);
}

static struct rnnvalue *copyvalue (struct rnnvalue *val, char *file) {
	struct rnnvalue *res = calloc (sizeof *res, 1);
	res->name = val->name;
	res->valvalid = val->valvalid;
	res->value = val->value;
	res->varinfo = val->varinfo;
	res->file = file;
	return res;
}

static struct rnnbitfield *copybitfield (struct rnnbitfield *bf, char *file);


static void copytypeinfo (struct rnntypeinfo *dst, struct rnntypeinfo *src, char *file) {
	int i;
	dst->name = src->name;
	dst->shr = src->shr;
	dst->low = src->low;
	dst->high = src->high;
	dst->min = src->min;
	dst->max = src->max;
	dst->align = src->align;
	dst->addvariant = src->addvariant;
	for (i = 0; i < src->valsnum; i++)
		ADDARRAY(dst->vals, copyvalue(src->vals[i], file));
	for (i = 0; i < src->bitfieldsnum; i++)
		ADDARRAY(dst->bitfields, copybitfield(src->bitfields[i], file));
}

static struct rnnbitfield *copybitfield (struct rnnbitfield *bf, char *file) {
	struct rnnbitfield *res = calloc (sizeof *res, 1);
	res->name = bf->name;
	res->varinfo = bf->varinfo;
	res->file = file;
	copytypeinfo(&res->typeinfo, &bf->typeinfo, file);
	return res;
}

static struct rnndelem *copydelem (struct rnndelem *elem, char *file) {
	struct rnndelem *res = calloc (sizeof *res, 1);
	res->type = elem->type;
	res->name = elem->name;
	res->width = elem->width;
	res->access = elem->access;
	res->offset = elem->offset;
	res->length = elem->length;
	res->stride = elem->stride;
	res->varinfo = elem->varinfo;
	res->file = file;
	copytypeinfo(&res->typeinfo, &elem->typeinfo, file);
	int i;
	for (i = 0; i < elem->subelemsnum; i++)
		ADDARRAY(res->subelems, copydelem(elem->subelems[i], file));
	for (i = 0; i < elem->offsetsnum; i++)
		ADDARRAY(res->offsets, elem->offsets[i]);
	return res;
}

static struct rnnvarset *copyvarset (struct rnnvarset *varset) {
	struct rnnvarset *res = calloc(sizeof *res, 1);
	res->venum = varset->venum;
	res->variants = calloc(sizeof *res->variants, res->venum->valsnum);
	int i;
	for (i = 0; i < res->venum->valsnum; i++)
		res->variants[i] = varset->variants[i];
	return res;
}

static void prepenum(struct rnndb *db, struct rnnenum *en);

static int findvidx (struct rnndb *db, struct rnnenum *en, char *name) {
	int i;
	for (i = 0; i < en->valsnum; i++)
		if (!strcmp(en->vals[i]->name, name))
			return i;
	rnn_err(db, "Cannot find variant %s in enum %s!\n", name, en->name);
	return -1;
}

static void prepvarinfo (struct rnndb *db, char *what, struct rnnvarinfo *vi, struct rnnvarinfo *parent) {
	if (parent)
		vi->prefenum = parent->prefenum;
	if (vi->prefixstr) {
		if (!strcmp(vi->prefixstr, "none"))
			vi->prefenum = 0;
		else
			vi->prefenum = rnn_findenum(db, vi->prefixstr); // XXX
	}
	int i;
	if (parent)
		for (i = 0; i < parent->varsetsnum; i++)
			ADDARRAY(vi->varsets, copyvarset(parent->varsets[i]));
	struct rnnenum *varset = vi->prefenum;
	if (!varset && !vi->varsetstr && parent)
		vi->varsetstr = parent->varsetstr;
	if (vi->varsetstr)
		varset = rnn_findenum(db, vi->varsetstr);
	if (vi->variantsstr) {
		char *vars = vi->variantsstr;
		if (!varset) {
			rnn_err(db, "%s: tried to use variants without active varset!\n", what);
			return;
		}
		struct rnnvarset *vs = 0;
		int nvars = varset->valsnum;
		for (i = 0; i < vi->varsetsnum; i++)
			if (vi->varsets[i]->venum == varset) {
				vs = vi->varsets[i];
				break;
			}
		if (!vs) {
			vs = calloc (sizeof *vs, 1);
			vs->venum = varset;
			vs->variants = calloc(sizeof *vs->variants, nvars);
			for (i = 0; i < nvars; i++)
				vs->variants[i] = 1;
			ADDARRAY(vi->varsets, vs);
		}
		while (1) {
			while (*vars == ' ') vars++;
			if (*vars == 0)
				break;
			char *split = vars;
			while (*split != ':' && *split != '-' && *split != ' '  && *split != 0)
				split++;
			char *first = 0;
			if (split != vars)
				first = strndup(vars, split-vars);
			if (*split == ' ' || *split == 0) {
				int idx = findvidx(db, varset, first);
				if (idx != -1)
					vs->variants[idx] |= 2;
				vars = split;
			} else {
				char *end = split+1;
				while (*end != ' '  && *end != 0)
					end++;
				char *second = 0;
				if (end != split+1)
					second = strndup(split+1, end-split-1);
				int idx1 = 0;
				if (first)
					idx1 = findvidx(db, varset, first);
				int idx2 = nvars;
				if (second) {
					idx2 = findvidx(db, varset, second);
					if (*split == '-')
						idx2++;
				}
				if (idx1 != -1 && idx2 != -1)
					for (i = idx1; i < idx2; i++)
						vs->variants[i] |= 2;
				vars = end;
				free(second);
			}
			free(first);
		}
		vi->dead = 1;
		for (i = 0; i < nvars; i++) {
			vs->variants[i] = (vs->variants[i] == 3);
			if (vs->variants[i])
				vi->dead = 0;
		}
	}
	if (vi->dead)
		return;
	if (vi->prefenum) {
		struct rnnvarset *vs = 0;
		for (i = 0; i < vi->varsetsnum; i++)
			if (vi->varsets[i]->venum == vi->prefenum) {
				vs = vi->varsets[i];
				break;
			}
		if (vs) {
			for (i = 0; i < vi->prefenum->valsnum; i++)
				if (vs->variants[i]) {
					vi->prefix = vi->prefenum->vals[i]->name;
					return;
				}
		} else {
			vi->prefix = vi->prefenum->vals[0]->name;
		}
	}
}

static void prepvalue(struct rnndb *db, struct rnnvalue *val, char *prefix, struct rnnvarinfo *parvi) {
	val->fullname = catstr(prefix, val->name);
	prepvarinfo (db, val->fullname, &val->varinfo, parvi);
	if (val->varinfo.dead)
		return;
	if (val->varinfo.prefix)
		val->fullname = catstr(val->varinfo.prefix, val->fullname);
}

static void prepbitfield(struct rnndb *db, struct rnnbitfield *bf, char *prefix, struct rnnvarinfo *parvi);

static void preptypeinfo(struct rnndb *db, struct rnntypeinfo *ti, char *prefix, struct rnnvarinfo *vi, char *file) {
	int i;
	if (ti->name) {
		struct rnnenum *en = rnn_findenum (db, ti->name);
		struct rnnbitset *bs = rnn_findbitset (db, ti->name);
		struct rnnspectype *st = rnn_findspectype (db, ti->name);
		if (en) {
			if (en->isinline) {
				ti->type = RNN_TTYPE_INLINE_ENUM;
				int j;
				for (j = 0; j < en->valsnum; j++)
					ADDARRAY(ti->vals, copyvalue(en->vals[j], file));
			} else {
				ti->type = RNN_TTYPE_ENUM;
				ti->eenum = en;
			}
		} else if (bs) {
			if (bs->isinline) {
				ti->type = RNN_TTYPE_INLINE_BITSET;
				int j;
				for (j = 0; j < bs->bitfieldsnum; j++)
					ADDARRAY(ti->bitfields, copybitfield(bs->bitfields[j], file));
			} else {
				ti->type = RNN_TTYPE_BITSET;
				ti->ebitset = bs;
			}
		} else if (st) {
			ti->type = RNN_TTYPE_SPECTYPE;
			ti->spectype = st;
		} else if (!strcmp(ti->name, "hex")) {
			ti->type = RNN_TTYPE_HEX;
		} else if (!strcmp(ti->name, "float")) {
			ti->type = RNN_TTYPE_FLOAT;
		} else if (!strcmp(ti->name, "uint")) {
			ti->type = RNN_TTYPE_UINT;
		} else if (!strcmp(ti->name, "int")) {
			ti->type = RNN_TTYPE_INT;
		} else if (!strcmp(ti->name, "boolean")) {
			ti->type = RNN_TTYPE_BOOLEAN;
		} else if (!strcmp(ti->name, "bitfield")) {
			ti->type = RNN_TTYPE_INLINE_BITSET;
		} else if (!strcmp(ti->name, "enum")) {
			ti->type = RNN_TTYPE_INLINE_ENUM;
		} else if (!strcmp(ti->name, "fixed")) {
			ti->type = RNN_TTYPE_FIXED;
		} else if (!strcmp(ti->name, "ufixed")) {
			ti->type = RNN_TTYPE_UFIXED;
		} else if (!strcmp(ti->name, "a3xx_regid")) {
			ti->type = RNN_TTYPE_A3XX_REGID;
		} else if (!strcmp(ti->name, "waddress") || !strcmp(ti->name, "address")) {
			ti->type = RNN_TTYPE_HEX;
		} else {
			ti->type = RNN_TTYPE_HEX;
			rnn_err(db, "%s: unknown type %s\n", prefix, ti->name);
		}
	} else if (ti->bitfieldsnum) {
		ti->name = "bitfield";
		ti->type = RNN_TTYPE_INLINE_BITSET;
	} else if (ti->valsnum) {
		ti->name = "enum";
		ti->type = RNN_TTYPE_INLINE_ENUM;
	} else if (ti->low == 0 && ti->high == 0) {
		ti->name = "boolean";
		ti->type = RNN_TTYPE_BOOLEAN;
	} else {
		ti->name = "hex";
		ti->type = RNN_TTYPE_HEX;
	}
	if (ti->addvariant && ti->type != RNN_TTYPE_ENUM) {
		rnn_err(db, "%s: addvariant specified on non-enum type %s\n", prefix, ti->name);
	}
	for (i = 0; i < ti->bitfieldsnum; i++) {
		prepbitfield(db,  ti->bitfields[i], prefix, vi);
		if (ti->bitfields[i]->typeinfo.addvariant) {
			for (int j = 0; j < i; j++) {
				if (!ti->bitfields[j]->typeinfo.addvariant) {
					struct rnnbitfield *t = ti->bitfields[j];
					ti->bitfields[j] = ti->bitfields[i];
					ti->bitfields[i] = t;
					break;
				}
			}
		}
	}
	for (i = 0; i < ti->valsnum; i++)
		prepvalue(db, ti->vals[i], prefix, vi);
}

static void prepbitfield(struct rnndb *db, struct rnnbitfield *bf, char *prefix, struct rnnvarinfo *parvi) {
	bf->fullname = catstr(prefix, bf->name);
	prepvarinfo (db, bf->fullname, &bf->varinfo, parvi);
	if (bf->varinfo.dead)
		return;
	preptypeinfo(db, &bf->typeinfo, bf->fullname, &bf->varinfo, bf->file);
	if (bf->varinfo.prefix)
		bf->fullname = catstr(bf->varinfo.prefix, bf->fullname);
}

static void prepdelem(struct rnndb *db, struct rnndelem *elem, char *prefix, struct rnnvarinfo *parvi, int width) {
	if (elem->type == RNN_ETYPE_USE_GROUP) {
		int i;
		struct rnngroup *gr = 0;
		for (i = 0; i < db->groupsnum; i++)
			if (!strcmp(db->groups[i]->name, elem->name)) {
				gr = db->groups[i];
				break;
			}
		if (gr) {
			for (i = 0; i < gr->subelemsnum; i++)
				ADDARRAY(elem->subelems, copydelem(gr->subelems[i], elem->file));
		} else {
			rnn_err(db, "group %s not found!\n", elem->name);
		}
		elem->type = RNN_ETYPE_STRIPE;
		elem->length = 1;
		elem->name = 0;
	}
	if (elem->name) {
		if (elem->varinfo.variantsstr && !strstr(elem->varinfo.variantsstr, "-")) {
			/* Special hack for headergen2 to deal with variant regs (like a6xx vs
			 * a7xx).. gen_header.py handles this differently by generating C++
			 * template based reg builder to handle variants.  But for now we still
			 * need something that can be used for kernel headers.
			 */
			elem->fullname = catstr(elem->varinfo.variantsstr, elem->name);
		} else {
			elem->fullname = catstr(prefix, elem->name);
		}
	}
	prepvarinfo (db, elem->fullname?elem->fullname:prefix, &elem->varinfo, parvi);
	if (elem->varinfo.dead)
		return;
	if (elem->length != 1 && !elem->stride) {
		if (elem->type != RNN_ETYPE_REG) {
			rnn_err(db, "%s has non-1 length, but no stride!\n", elem->fullname);
		} else {
			elem->stride = elem->width/width;
		}
	}
	preptypeinfo(db, &elem->typeinfo, elem->name?elem->fullname:prefix, &elem->varinfo, elem->file);

	int i;
	for (i = 0; i < elem->subelemsnum; i++)
		prepdelem(db,  elem->subelems[i], elem->name?elem->fullname:prefix, &elem->varinfo, width);
	if (elem->varinfo.prefix && elem->name)
		elem->fullname = catstr(elem->varinfo.prefix, elem->fullname);
}

static void prepdomain(struct rnndb *db, struct rnndomain *dom) {
	prepvarinfo (db, dom->name, &dom->varinfo, 0);
	int i;
	for (i = 0; i < dom->subelemsnum; i++)
		prepdelem(db, dom->subelems[i], dom->bare?0:dom->name, &dom->varinfo, dom->width);
	dom->fullname = catstr(dom->varinfo.prefix, dom->name);
}

static void prepenum(struct rnndb *db, struct rnnenum *en) {
	if (en->prepared)
		return;
	prepvarinfo (db, en->name, &en->varinfo, 0);
	int i;
	if (en->isinline)
		return;
	for (i = 0; i < en->valsnum; i++)
		prepvalue(db, en->vals[i], en->bare?0:en->name, &en->varinfo);
	en->fullname = catstr(en->varinfo.prefix, en->name);
	en->prepared = 1;
}

static void prepbitset(struct rnndb *db, struct rnnbitset *bs) {
	prepvarinfo (db, bs->name, &bs->varinfo, 0);
	int i;
	if (bs->isinline)
		return;
	for (i = 0; i < bs->bitfieldsnum; i++)
		prepbitfield(db, bs->bitfields[i], bs->bare?0:bs->name, &bs->varinfo);
	bs->fullname = catstr(bs->varinfo.prefix, bs->name);
}

static void prepspectype(struct rnndb *db, struct rnnspectype *st) {
	preptypeinfo(db, &st->typeinfo, st->name, 0, st->file); // XXX doesn't exactly make sense...
}

void rnn_prepdb (struct rnndb *db) {
	int i;
	for (i = 0; i < db->enumsnum; i++)
		prepenum(db, db->enums[i]);
	for (i = 0; i < db->bitsetsnum; i++)
		prepbitset(db, db->bitsets[i]);
	for (i = 0; i < db->domainsnum; i++)
		prepdomain(db, db->domains[i]);
	for (i = 0; i < db->spectypesnum; i++)
		prepspectype(db, db->spectypes[i]);
}

struct rnnenum *rnn_findenum (struct rnndb *db, const char *name) {
	int i;
	for (i = 0; i < db->enumsnum; i++)
		if (!strcmp(db->enums[i]->name, name))
			return db->enums[i];
	return 0;
}

struct rnnbitset *rnn_findbitset (struct rnndb *db, const char *name) {
	int i;
	for (i = 0; i < db->bitsetsnum; i++)
		if (!strcmp(db->bitsets[i]->name, name))
			return db->bitsets[i];
	return 0;
}

struct rnndomain *rnn_finddomain (struct rnndb *db, const char *name) {
	int i;
	for (i = 0; i < db->domainsnum; i++)
		if (!strcmp(db->domains[i]->name, name))
			return db->domains[i];
	return 0;
}

struct rnnspectype *rnn_findspectype (struct rnndb *db, const char *name) {
	int i;
	for (i = 0; i < db->spectypesnum; i++)
		if (!strcmp(db->spectypes[i]->name, name))
			return db->spectypes[i];
	return 0;
}
