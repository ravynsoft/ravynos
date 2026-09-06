/* msgfmt utility (C) 2012 rofl0r
 * released under the MIT license, see LICENSE for details */
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "poparser.h"

enum mode {
	m_normal = 0,
	m_java,
	m_java2,
	m_csharp,
	m_csharp_resources,
	m_tcl,
	m_qt,
	m_desktop,
	m_xml
};

static void syntax(void) {
	fprintf(stdout, "Usage: msgfmt [OPTION] filename.po ...\n");
}

static void version(void) {
	fprintf(stdout, "msgfmt (GNU gettext-tools compatible) 99.9999.9999\n");
}

#define streq(A, B) (!strcmp(A, B))
#define strstarts(S, W) (memcmp(S, W, sizeof(W) - 1) ? NULL : (S + (sizeof(W) - 1)))

struct mo_hdr {
	unsigned magic;
	int rev;
	unsigned numstring;
	unsigned off_tbl_org;
	unsigned off_tbl_trans;
	unsigned hash_tbl_size;
	unsigned off_tbl_hash;
};

/* file layout:
	header
	strtable (lengths/offsets)
	transtable (lengths/offsets)
	[hashtable]
	strings section
	translations section */

const struct mo_hdr def_hdr = {
	0x950412de,
	0,
	0,
	sizeof(struct mo_hdr),
	0,
	0,
	0,
};

struct strtbl {
	uint32_t len, off;
};

struct strmap {
	struct strtbl str;
	struct strtbl trans;
};

struct callbackdata {
	FILE* out;
	enum po_stage stage;
	size_t cnt;
	size_t len[2];
	char* buf[2];
	struct strmap *list;
};

static struct callbackdata *cb_for_qsort;
int strtbl_cmp(const void *a_, const void *b_) {
	const struct strmap *a = a_, *b = b_;
	return strcmp(cb_for_qsort->buf[0] + a->str.off, cb_for_qsort->buf[0] + b->str.off);
}

int process_line_callback(po_message_t msg, void* user) {
	struct callbackdata *d = (struct callbackdata *) user;
	struct strtbl *str, *trans;
	size_t m;
	int sysdep_cases = 1;
	int i, j[MAX_SYSDEP+1] = {0};

	// compute sysdep cases
	for (i=0; i<MAX_SYSDEP; i++)
		if (msg->sysdep[i] != 0)
			sysdep_cases *= msg->sysdep[i];

	if (msg->flags & PO_FUZZY) return 0;
	if (msg->strlen[0] == 0) return 0;

	switch(d->stage) {
	case ps_size:
		m = 0;
		m += msg->id_len + 1;

		if (msg->plural_len)
			m += msg->plural_len + 1;

		if (msg->ctxt_len)
			m += msg->ctxt_len + 1;

		d->len[0] += m * sysdep_cases;

		m = 0;
		for (i=0; msg->strlen[i]; i++) {
			m += msg->strlen[i] + 1;
		}
		d->len[1] += m * sysdep_cases;

		d->cnt += sysdep_cases;
		break;
	case ps_parse:
		while (true) {
			str = &d->list[d->cnt].str;
			trans = &d->list[d->cnt].trans;

			str->off = d->len[0];
			str->len = 0;

			if (msg->ctxt_len) {
				m = poparser_sysdep(msg->ctxt, &d->buf[0][d->len[0]], j);
				str->len += m + 1;
				d->buf[0][d->len[0]+m] = 0x4;
				d->len[0] += m + 1;
			}

			m = poparser_sysdep(msg->id, &d->buf[0][d->len[0]], j);
			str->len += m;
			d->len[0] += m + 1;

			if (msg->plural_len) {
				m = poparser_sysdep(msg->plural, &d->buf[0][d->len[0]], j);
				str->len += m + 1;
				d->len[0] += m + 1;
			}

			trans->off = d->len[1];
			trans->len = -1;
			for (i=0; msg->strlen[i]; i++) {
				m = poparser_sysdep(msg->str[i], &d->buf[1][d->len[1]], j);
				trans->len += m + 1;
				d->len[1] += m + 1;
			}

			d->cnt++;

			// carry over the iter otherwise
			for (i=0; i<MAX_SYSDEP+1; i++) {
				// skip if it is not present
				if (i < MAX_SYSDEP && msg->sysdep[i] == 0) continue;
				j[i]++;
				if (i >= MAX_SYSDEP || j[i] < msg->sysdep[i])
					break;
				j[i] = 0;
			}

			// break if all combs iterated
			if (j[MAX_SYSDEP] == 1) break;
		}
		break;
	default:
		abort();
	}
	return 0;
}

int process(FILE *in, FILE *out, bool strict) {
	struct mo_hdr mohdr = def_hdr;
	char line[8192]; char *lp;
	size_t off, i;
	enum po_error t;
	size_t convbuf_sz = 32768;
	char *convbuf = malloc(convbuf_sz);

	struct callbackdata d = {
		.len = {0, 0},
		.cnt = 0,
		.out = out,
	};

	struct po_parser pb, *p = &pb;

	mohdr.off_tbl_trans = mohdr.off_tbl_org;

	poparser_init(p, convbuf, convbuf_sz, process_line_callback, &d);
	p->strict = strict;
	d.stage = p->stage;

	while((lp = fgets(line, sizeof(line), in))) {
		if ((t = poparser_feed_line(p, lp, strlen(line))) != po_success)
			return t;
	}
	if ((t = poparser_finish(p)) != po_success)
		return t;

	if (strict && d.cnt == 0) return -(po_error_last+1);

	d.list = (struct strmap*)malloc(sizeof(struct strmap)*d.cnt);
	if (!d.list)
		return -po_fail_mem;
	d.buf[0] = (char*)malloc(d.len[0]);
	if (!d.buf[0])
		return -po_fail_mem;
	d.buf[1] = (char*)malloc(d.len[1]);
	if (!d.buf[1])
		return -po_fail_mem;
	d.len[0] = 0;
	d.len[1] = 0;
	d.cnt = 0;
	d.stage = p->stage;

	fseek(in, 0, SEEK_SET);
	while ((lp = fgets(line, sizeof(line), in))) {
		if ((t = poparser_feed_line(p, lp, strlen(line))) != po_success) {
			free(d.list);
			free(d.buf[0]);
			free(d.buf[1]);
			return t;
		}
	}
	if ((t = poparser_finish(p)) != po_success) {
		free(d.list);
		free(d.buf[0]);
		free(d.buf[1]);
		return t;
	}

	cb_for_qsort = &d;
	qsort(d.list, d.cnt, sizeof(struct strmap), strtbl_cmp);
	cb_for_qsort = NULL;

	// print header
	mohdr.numstring = d.cnt;
	mohdr.off_tbl_org = sizeof(struct mo_hdr);
	mohdr.off_tbl_trans = mohdr.off_tbl_org + d.cnt * sizeof(struct strtbl);
	fwrite(&mohdr, sizeof(mohdr), 1, out);

	off = mohdr.off_tbl_trans + d.cnt * sizeof(struct strtbl);
	for (i = 0; i < d.cnt; i++) {
		d.list[i].str.off += off;
		fwrite(&d.list[i].str, sizeof(struct strtbl), 1, d.out);
	}

	off += d.len[0];
	for (i = 0; i < d.cnt; i++) {
		d.list[i].trans.off += off;
		fwrite(&d.list[i].trans, sizeof(struct strtbl), 1, d.out);
	}

	fwrite(d.buf[0], d.len[0], 1, d.out);
	fwrite(d.buf[1], d.len[1], 1, d.out);

	free(d.list);
	free(d.buf[0]);
	free(d.buf[1]);

	free(convbuf);

	return 0;
}

void set_file(int out, char* fn, FILE** dest) {
	char b[4096];
	size_t n;
	FILE *tmpf = NULL;

	if(streq(fn, "-")) {
		if(out) {
			*dest = stdout;
		} else {
			tmpf = tmpfile();
			if(!tmpf) {
				perror("tmpfile");
				exit(1);
			}
			*dest = tmpf;

			while((n=fread(b, sizeof(*b), sizeof(b), stdin)) > 0)
				fwrite(b, sizeof(*b), n, tmpf);

			fseek(tmpf, 0, SEEK_SET);
		}
	} else {
		*dest = fopen(fn, out ? "w" : "r");
	}

	if(!*dest) {
		perror("fopen");
		exit(1);
	}
}

int main(int argc, char**argv) {
	if (argc == 1) {
		syntax();
		return 1;
	}

	int arg = 1;
	enum mode mode = m_normal;
	bool strict = false;
	FILE *out = NULL;
	FILE *in = NULL;
	int expect_in_fn = 0;
	char path[PATH_MAX];
	char buf[4096];
	size_t n;
	int ret = 0;
	char* locale = NULL;
	char* dest = NULL;

#define A argv[arg]
	for(; arg < argc; arg++) {
		if(A[0] == '-') {
			if(A[1] == '-') {
				if(
					streq(A+2, "template") ||
					streq(A+2, "strict") ||
					streq(A+2, "properties-input") ||
					streq(A+2, "stringtable-input") ||
					streq(A+2, "use-fuzzy") ||
					strstarts(A+2, "alignment=") ||
					streq(A+2, "check-format") ||
					streq(A+2, "check-header") ||
					streq(A+2, "check-domain") ||
					streq(A+2, "check-compatibility") ||
					streq(A+2, "check-accelerators") ||
					streq(A+2, "no-hash") ||
					streq(A+2, "verbose") ||
					streq(A+2, "statistics") ||
					strstarts(A+2, "keyword=") ||
					strstarts(A+2, "check-accelerators=") ||
					strstarts(A+2, "resource=")
					) {
					} else if(streq(A+2, "java")) {
						mode = m_java;
					} else if(streq(A+2, "java2")) {
						mode = m_java2;
					} else if(streq(A+2, "csharp")) {
						mode = m_csharp;
					} else if(streq(A+2, "csharp-resources")) {
						mode = m_csharp_resources;
					} else if(streq(A+2, "tcl")) {
						mode = m_tcl;
					} else if(streq(A+2, "qt")) {
						mode = m_qt;
					} else if(streq(A+2, "desktop")) {
						mode = m_desktop;
					} else if(streq(A+2, "xml")) {
						mode = m_xml;
					} else if(streq(A+2, "keyword")) {
						arg++;
					} else if((locale = strstarts(A+2, "locale="))) {
					} else if(streq(A+2, "check")) {
						strict = true;
					} else if(strstarts(A+2, "template=")) {
						set_file(0, A+11, &in);
						expect_in_fn = 0;
					} else if(strstarts(A+2, "output-file=")) {
						set_file(1, A+14, &out);
					} else if(strstarts(A+2, "output=")) {
						set_file(1, A+9, &out);
					} else if(streq(A+2, "version")) {
						version();
						return 0;
					} else if(streq(A+2, "help")) {
						syntax();
						return 0;
					} else expect_in_fn = 1;
			} else if(streq(A + 1, "o")) {
				arg++;
				set_file(1, A, &out);
			} else if(
				streq(A+1, "j") ||
				streq(A+1, "r") ||
				streq(A+1, "P") ||
				streq(A+1, "f") ||
				streq(A+1, "a") ||
				streq(A+1, "v") ||
				streq(A+1, "C")
			) {
			} else if (streq(A+1, "c")) {
				strict = true;
			} else if (streq(A+1, "V")) {
				version();
				return 0;
			} else if (
				streq(A+1, "h") ||
				// D means find input files in the specific directory
				// we should fail this option explicitly
				streq(A+1, "D")
			) {
				syntax();
				return 1;
			} else if (streq(A+1, "l")) {
				arg++;
				locale = A;
			} else if (streq(A+1, "d")) {
				arg++;
				dest = A;
			} else expect_in_fn = 1;
		} else expect_in_fn = 1;

		if (expect_in_fn) {
			set_file(0, A, &in);
			expect_in_fn = 0;
		}
	}

	switch (mode) {
	case m_tcl:
		if (locale == NULL || dest == NULL) {
			return -1;
		}

		snprintf(path, sizeof(path), "%s/%s.msg", dest, locale);
		out = fopen(path, "w");

		if (out == NULL) {
			perror("fopen");
			ret = -1;
		}
		break;
	case m_desktop:
	case m_xml:
		while((n = fread(buf, sizeof(buf[0]), sizeof(buf), in)) > 0)
			fwrite(buf, sizeof(buf[0]), n, out);

		break;
	case m_normal:
		if(out == NULL) {
			set_file(1, "messages.mo", &out);
		}

		if(in == NULL || out == NULL) {
			return -1;
		}

		ret = process(in, out, strict);
		break;
	default:
		fprintf(stderr, "unsupported mode %d\n", mode);
		ret = -1;
	}

	if(in == stdin)
		fflush(in);
	else
		fclose(in);

	if(out == stdout)
		fflush(out);
	else
		fclose(out);

	if (ret < 0) remove(dest);
	return ret;
}
