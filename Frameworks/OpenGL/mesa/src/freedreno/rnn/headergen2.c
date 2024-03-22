/*
 * Copyright (C) 2013 Rob Clark <robdclark@gmail.com>
 * Copyright (C) 2010-2011 Marcin Kościelnicki <koriakin@0x04.net>
 * Copyright (C) 2010 Luca Barbieri <luca@luca-barbieri.com>
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

/* modified version of headergen which uses enums and inline fxns for
 * type safety.. based on original headergen
 */

#include "rnn.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>

struct rnndelem **elems = NULL;
int elemsnum = 0;
int elemsmax = 0;

char **offsetfns = NULL;
int offsetfnsnum = 0;
int offsetfnsmax = 0;

int startcol = 64;

struct fout {
	char *name;
	FILE *file;
	char *guard;
};

struct fout *fouts = 0;
int foutsnum = 0;
int foutsmax = 0;

static bool no_asserts = false;

static void seekcol (FILE *f, int src, int dst) {
	if (dst <= src)
		fprintf (f, "\t");
	else {
		int n = dst/8 - src/8;
		if (n) {
			while (n--)
				fprintf (f, "\t");
			n = dst&7;
		} else
			n = dst-src;
		while (n--)
			fprintf (f, " ");
	}
}

static FILE *findfout (char *file) {
	int i;
	for (i = 0; i < foutsnum; i++)
		if (!strcmp(fouts[i].name, file))
			break;
	if (i == foutsnum) {
		fprintf (stderr, "AIII, didn't open file %s.\n", file);
		exit(1);
	}
	return fouts[i].file;
}

static void printdef (char *name, char *suf, int type, uint64_t val, char *file) {
	FILE *dst = findfout(file);
	int len;
	if (suf)
		len = fprintf (dst, "#define %s__%s", name, suf);
	else
		len = fprintf (dst, "#define %s", name);
	if (type == 0 && val > 0xffffffffull)
		seekcol (dst, len, startcol-8);
	else
		seekcol (dst, len, startcol);
	switch (type) {
		case 0:
			if (val > 0xffffffffull)
				fprintf (dst, "0x%016"PRIx64"ULL\n", val);
			else
				fprintf (dst, "0x%08"PRIx64"\n", val);
			break;
		case 1:
			fprintf (dst, "%"PRIu64"\n", val);
			break;
	}
}

static void printvalue (struct rnnvalue *val, int shift) {
	if (val->varinfo.dead)
		return;
	if (val->valvalid)
		printdef (val->fullname, 0, 0, val->value << shift, val->file);
}

static void printbitfield (struct rnnbitfield *bf, int shift);

static void printtypeinfo (struct rnntypeinfo *ti, struct rnnbitfield *bf,
		char *prefix, char *file) {
	FILE *dst = findfout(file);
	enum rnnttype intype = ti->type;
	char *typename = NULL;
	uint32_t mask = typeinfo_mask(ti);
	uint32_t width = 1 + ti->high - ti->low;

	/* for fixed point, input type (arg to fxn) is float: */
	if ((ti->type == RNN_TTYPE_FIXED) || (ti->type == RNN_TTYPE_UFIXED))
		intype = RNN_TTYPE_FLOAT;

	/* for toplevel register (ie. not bitfield), only generate accessor
	 * fxn for special cases (float, shr, min/max, etc):
	 */
	if (bf || ti->shr || ti->minvalid || ti->maxvalid || ti->alignvalid ||
			ti->radixvalid || (intype == RNN_TTYPE_FLOAT)) {
		switch (intype) {
		case RNN_TTYPE_HEX:
		case RNN_TTYPE_UINT:
		case RNN_TTYPE_A3XX_REGID:
			typename = "uint32_t";
			break;
		case RNN_TTYPE_INT:
			typename = "int32_t";
			break;
		case RNN_TTYPE_FLOAT:
			typename = "float";
			break;
		case RNN_TTYPE_ENUM:
			asprintf(&typename, "enum %s", ti->name);
			break;
		default:
			break;
		}
	}

	/* for boolean, just generate a #define flag.. rather than inline fxn */
	if (bf && (intype == RNN_TTYPE_BOOLEAN)) {
		printdef(bf->fullname, 0, 0, mask, file);
		return;
	}

	if (typename) {
		printdef(prefix, "MASK", 0, mask, file);
		printdef(prefix, "SHIFT", 1, ti->low, file);

		fprintf(dst, "static inline uint32_t %s(%s val)\n", prefix, typename);
		fprintf(dst, "{\n");

		if ((ti->minvalid || ti->maxvalid || ti->alignvalid) && !no_asserts) {
			fprintf(dst, "\tassert(1");
			if (ti->minvalid)
				fprintf(dst, " && (val >= %"PRIu64")", ti->min);
			if (ti->maxvalid)
				fprintf(dst, " && (val <= %"PRIu64")", ti->max);
			if (ti->alignvalid)
				fprintf(dst, " && !(val %% %"PRIu64")", ti->align);
			fprintf(dst, ");\n");
		}

		if (ti->shr && !no_asserts) {
			fprintf(dst, "\tassert(!(val & 0x%x));\n", (1 << ti->shr) - 1);
		}

		fprintf(dst, "\treturn ((");

		if (ti->type == RNN_TTYPE_FIXED) {
			fprintf(dst, "((int32_t)(val * %d.0))", (1 << ti->radix));
		} else if (ti->type == RNN_TTYPE_UFIXED) {
			fprintf(dst, "((uint32_t)(val * %d.0))", (1 << ti->radix));
		} else if (ti->type == RNN_TTYPE_FLOAT) {
			if (width == 32)
				fprintf(dst, "fui(val)");
			else if (width == 16)
				fprintf(dst, "_mesa_float_to_half(val)");
			else
				assert(!"invalid float size");
		} else {
			fprintf(dst, "val");
		}

		if (ti->shr)
			fprintf(dst, " >> %d", ti->shr);

		fprintf(dst, ") << %s__SHIFT) & %s__MASK;\n", prefix, prefix);
		fprintf(dst, "}\n");

		if (intype == RNN_TTYPE_ENUM)
			free(typename);
	}

	int i;
	for (i = 0; i < ti->valsnum; i++)
		printvalue(ti->vals[i], ti->low);
	for (i = 0; i < ti->bitfieldsnum; i++)
		printbitfield(ti->bitfields[i], ti->low);
}

static void printbitfield (struct rnnbitfield *bf, int shift) {
	if (bf->varinfo.dead)
		return;
	printtypeinfo (&bf->typeinfo, bf, bf->fullname, bf->file);
}

static void printdelem (struct rnndelem *elem, uint64_t offset, const char *str) {
	int use_offset_fxn;
	char *offsetfn = NULL;

	if (elem->varinfo.dead)
		return;

	use_offset_fxn = elem->offsets || elem->doffset || elem->doffsets;
	assert((!!elem->offsets + !!elem->doffset + !!elem->doffsets) <= 1);

	if (use_offset_fxn)
		asprintf(&offsetfn, "__offset_%s", elem->name);

	if (elem->length != 1) {
		ADDARRAY(elems, elem);
		ADDARRAY(offsetfns, offsetfn);
	}

	if (elem->name) {
		char *regname;
		if (str) {
			asprintf(&regname, "REG_%s_%s", elem->fullname, str);
		} else {
			asprintf(&regname, "REG_%s", elem->fullname);
		}
		if (elemsnum) {
			FILE *dst = findfout(elem->file);
			int i;

			if (use_offset_fxn) {
				fprintf(dst, "static inline uint32_t %s(", offsetfn);
				if (elem->index)
					fprintf(dst, "enum %s", elem->index->name);
				else
					fprintf(dst, "uint32_t");
				fprintf(dst, " idx)\n");
				fprintf(dst, "{\n");
				if (elem->doffset) {
					fprintf(dst, "\treturn (%s) + (%#" PRIx64 "*idx);\n", elem->doffset, elem->stride);
				} else {
					int valuesnum = elem->doffsets ? elem->doffsetsnum : elem->offsetsnum;

					fprintf(dst, "\tswitch (idx) {\n");
					for (i = 0; i < valuesnum; i++) {
						struct rnnvalue *val = NULL;
						fprintf(dst, "\t\tcase ");
						if (elem->index) {
							int j;
							for (j = 0; j < elem->index->valsnum; j++) {
								if (elem->index->vals[j]->value == i) {
									val = elem->index->vals[j];
									break;
								}
							}
						}
						if (val) {
							fprintf(dst, "%s", val->name);
						} else {
							fprintf(dst, "%d", i);
						}
						if (elem->offsets) {
							fprintf(dst, ": return 0x%08"PRIx64";\n", elem->offsets[i]);
						} else {
							fprintf(dst, ": return (%s);\n", elem->doffsets[i]);
						}
					}
					fprintf(dst, "\t\tdefault: return INVALID_IDX(idx);\n");
					fprintf(dst, "\t}\n");
				}
				fprintf(dst, "}\n");
			}
			fprintf (dst, "static inline uint32_t %s(", regname);
			for (i = 0; i < elemsnum; i++) {
				if (i)
					fprintf(dst, ", ");
				if (elems[i]->index)
					fprintf(dst, "enum %s ", elems[i]->index->name);
				else
					fprintf(dst, "uint32_t ");
				fprintf (dst, "i%d", i);
			}
			fprintf (dst, ") { return ");
			fprintf (dst, "0x%08"PRIx64"", offset + elem->offset);
			for (i = 0; i < elemsnum; i++) {
				if (offsetfns[i])
					fprintf(dst, " + %s(i%d)", offsetfns[i], i);
				else
					fprintf (dst, " + %#" PRIx64 "*i%d", elems[i]->stride, i);
			}
			fprintf (dst, "; }\n");
		} else
			printdef (regname, 0, 0, offset + elem->offset, elem->file);

		free(regname);
/*
		if (elem->stride)
			printdef (elem->fullname, "ESIZE", 0, elem->stride, elem->file);
		if (elem->length != 1)
			printdef (elem->fullname, "LEN", 0, elem->length, elem->file);
*/
		printtypeinfo (&elem->typeinfo, NULL, elem->fullname, elem->file);
	}
	fprintf (findfout(elem->file), "\n");
	int j;
	for (j = 0; j < elem->subelemsnum; j++) {
		printdelem(elem->subelems[j], offset + elem->offset, elem->varinfo.prefixstr);
	}
	if (elem->length != 1) {
		elemsnum--;
		offsetfnsnum--;
	}
	free(offsetfn);
}

static void print_file_info_(FILE *dst, struct stat* sb, struct tm* tm)
{
	char timestr[64];
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm);
	fprintf(dst, "(%7Lu bytes, from %s)\n", (unsigned long long)sb->st_size, timestr);
}

static void print_file_info(FILE *dst, const char* file)
{
	struct stat sb;
	struct tm tm;
	stat(file, &sb);
	gmtime_r(&sb.st_mtime, &tm);
	print_file_info_(dst, &sb, &tm);
}

static void printhead(struct fout f, struct rnndb *db) {
	int i, j;
	struct stat sb;
	struct tm tm;
	stat(f.name, &sb);
	gmtime_r(&sb.st_mtime, &tm);
	fprintf (f.file, "#ifndef %s\n", f.guard);
	fprintf (f.file, "#define %s\n", f.guard);
	fprintf (f.file, "\n");
	fprintf(f.file,
		"/* Autogenerated file, DO NOT EDIT manually!\n"
		"\n"
		"This file was generated by the rules-ng-ng headergen tool in this git repository:\n"
		"http://github.com/freedreno/envytools/\n"
		"git clone https://github.com/freedreno/envytools.git\n"
		"\n"
		"The rules-ng-ng source files this header was generated from are:\n");
	unsigned maxlen = 0;
	for(i = 0; i < db->filesnum; ++i) {
		unsigned len = strlen(db->files[i]);
		if(len > maxlen)
			maxlen = len;
	}
	for(i = 0; i < db->filesnum; ++i) {
		unsigned len = strlen(db->files[i]);
		fprintf(f.file, "- %s%*s ", db->files[i], maxlen - len, "");
		print_file_info(f.file, db->files[i]);
	}
	fprintf(f.file,
		"\n"
		"Copyright (C) ");
	if(db->copyright.firstyear && db->copyright.firstyear < (1900 + tm.tm_year))
		fprintf(f.file, "%u-", db->copyright.firstyear);
	fprintf(f.file, "%u", 1900 + tm.tm_year);
	if(db->copyright.authorsnum) {
		fprintf(f.file, " by the following authors:");
		for(i = 0; i < db->copyright.authorsnum; ++i) {
			fprintf(f.file, "\n- ");
			if(db->copyright.authors[i]->name)
				fprintf(f.file, "%s", db->copyright.authors[i]->name);
			if(db->copyright.authors[i]->email)
				fprintf(f.file, " <%s>", db->copyright.authors[i]->email);
			if(db->copyright.authors[i]->nicknamesnum) {
				for(j = 0; j < db->copyright.authors[i]->nicknamesnum; ++j) {
					fprintf(f.file, "%s%s", (j ? ", " : " ("), db->copyright.authors[i]->nicknames[j]);
				}
				fprintf(f.file, ")");
			}
		}
	}
	fprintf(f.file, "\n");
	if(db->copyright.license)
		fprintf(f.file, "\n%s\n", db->copyright.license);
	fprintf(f.file, "*/\n\n\n");
}

int main(int argc, char **argv) {
	char *file;
	struct rnndb *db;
	int i, j;

	if (argc < 2) {
		fprintf(stderr, "Usage:\n\theadergen database-file\n");
		exit(1);
	}

	if ((argc >= 3) && !strcmp(argv[1], "--no-asserts")) {
		no_asserts = true;
		file = argv[2];
	} else {
		file = argv[1];
	}

	rnn_init();
	db = rnn_newdb();
	rnn_parsefile (db, file);
	rnn_prepdb (db);
	for(i = 0; i < db->filesnum; ++i) {
		char *dstname = malloc(strlen(db->files[i]) + 3);
		char *pretty;
		strcpy(dstname, db->files[i]);
		strcat(dstname, ".h");
		struct fout f = { db->files[i], fopen(dstname, "w") };
		if (!f.file) {
			perror(dstname);
			exit(1);
		}
		free(dstname);
		pretty = strrchr(f.name, '/');
		if (pretty)
			pretty += 1;
		else
			pretty = f.name;
		f.guard = strdup(pretty);
		for (j = 0; j < strlen(f.guard); j++)
			if (isalnum(f.guard[j]))
				f.guard[j] = toupper(f.guard[j]);
			else
				f.guard[j] = '_';
		ADDARRAY(fouts, f);
		printhead(f, db);
	}

	for (i = 0; i < db->enumsnum; i++) {
		FILE *dst = NULL;
		int j;
		for (j = 0; j < db->enums[i]->valsnum; j++) {
			if (!dst) {
				dst = findfout(db->enums[i]->vals[j]->file);
				fprintf(dst, "enum %s {\n", db->enums[i]->name);
			}
			if (0xffff0000 & db->enums[i]->vals[j]->value)
				fprintf(dst, "\t%s = 0x%08"PRIx64",\n", db->enums[i]->vals[j]->name,
						db->enums[i]->vals[j]->value);
			else
				fprintf(dst, "\t%s = %"PRIu64",\n", db->enums[i]->vals[j]->name,
						db->enums[i]->vals[j]->value);
		}
		if (dst) {
			fprintf(dst, "};\n\n");
		}
	}
	for (i = 0; i < db->bitsetsnum; i++) {
		if (db->bitsets[i]->isinline)
			continue;
		int j;
		for (j = 0; j < db->bitsets[i]->bitfieldsnum; j++)
			printbitfield (db->bitsets[i]->bitfields[j], 0);
	}
	for (i = 0; i < db->domainsnum; i++) {
		if (db->domains[i]->size)
			printdef (db->domains[i]->fullname, "SIZE", 0, db->domains[i]->size, db->domains[i]->file);
		int j;
		for (j = 0; j < db->domains[i]->subelemsnum; j++) {
			printdelem(db->domains[i]->subelems[j], 0, NULL);
		}
	}
	for(i = 0; i < foutsnum; ++i) {
		fprintf (fouts[i].file, "\n#endif /* %s */\n", fouts[i].guard);
	}
	return db->estatus;
}
