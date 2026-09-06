#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iconv.h>
#include "poparser.h"
#include "StringEscape.h"

#define strstarts(S, W) (memcmp(S, W, sizeof(W) - 1) ? NULL : (S + (sizeof(W) - 1)))

static void poparser_populate_msg_sysdeps(const char *x, po_message_t msg) {
	int i;
	for (i = 0; i < MAX_SYSDEP; i++) {
		if (msg->sysdep[i] == 0 && strstr(x, sysdep_cases[i].format))
			msg->sysdep[i] = sysdep_cases[i].cnt;
	}
}

void poparser_init(struct po_parser *p, char* workbuf, size_t bufsize, poparser_callback cb, void* cbdata) {
	int cnt;
	memset(p, 0, sizeof(struct po_parser));
	p->buf = workbuf;
	p->bufsize = bufsize;
	p->cb = cb;
	p->cbdata = cbdata;
	p->hdr.nplurals = MAX_NPLURALS;
	p->max_ctxt_len = 1;
	p->max_id_len = 1;
	p->max_plural_len = 1;
	for (cnt = 0; cnt < MAX_NPLURALS; cnt++)
		p->max_strlen[cnt] = 1;
	p->strcnt = 0;
	p->first = true;
}

static inline enum po_error poparser_feed_hdr(struct po_parser *p, char* msg) {
	char *x, *y;
	if (p->first && msg) {
		if ((x = strstr(msg, "charset="))) {
			for (y = x; *y && *y != '\\' && !isspace(*y); y++);

			if ((uintptr_t)(y-x-7) > sizeof(p->hdr.charset))
				return -po_unsupported_charset;

			memcpy(p->hdr.charset, x+8, y-x-8);
			p->hdr.charset[y-x-8] = 0;

			p->cd = iconv_open("UTF-8", p->hdr.charset);
			if (p->cd == (iconv_t)-1) {
				p->cd = 0;
				if (p->strict) return -po_unsupported_charset;
			}
		}

		if ((x = strstr(msg, "nplurals="))) {
			p->hdr.nplurals = *(x+9) - '0';
		}
	}

	return po_success;
}

static inline enum po_error poparser_clean(struct po_parser *p, po_message_t msg) {
	if (p->strcnt) {
		if (p->first) p->first = false;

		msg->strlen[p->strcnt] = 0;

		// met a new block starting with msgid
		if (p->cb)
			p->cb(msg, p->cbdata);

		memset(msg->sysdep , 0, sizeof(msg->sysdep));
		msg->ctxt_len = 0;
		msg->id_len = 0;
		msg->plural_len = 0;
		msg->flags = 0;
		p->strcnt = 0;
	}

	return po_success;
}

enum po_error poparser_feed_line(struct po_parser *p, char* in, size_t in_len) {
	char *line = in;
	size_t line_len = in_len;
	po_message_t msg = &p->msg;
	int cnt = 0;
	enum po_error t;
	size_t len;
	char *x, *y, *z;

	if (line_len == 0 || line[0] == '\n') {
		// ignore blank lines
		return po_success;
	} else if (line[0] == '#') {
		if (p->previous == po_str) {
			if ( (t = poparser_clean(p, msg)) != po_success)
				return t;
		}

		switch (line[1]) {
		case ',':
			x = &line[2];
			while (*x && (y = strpbrk(x, " ,\n"))) {
				if (y != x && !strncmp(x, "fuzzy", 5)) {
					msg->flags |= PO_FUZZY;
				}
				x = y + strspn(y, " ,\n");
			}
			break;
		case '.':
			// extracted comments for translators, ignore
		case ':':
			// reference comments for translators, ignore
		case '|':
			// previous untranslated strings for translators, ignore
		default:
			// ignore normal comments
			return po_success;
		}
	} else if (line[0] == '"') {
		if ( (y = strrchr(x = &line[1], '"')) == NULL)
			return -po_excepted_token;

		len = y - x;
		*y = 0;

		if (p->cd) {
			line = x;
			line_len = len + 1;
			x = p->buf;
			len = p->bufsize;
			if (iconv(p->cd, &line, &line_len, &x, &len) == (size_t)-1)
				return -po_failed_iconv;

			if (line_len != 0)
				return -po_failed_iconv;

			len = x - p->buf;
			x = p->buf;
		}

		poparser_populate_msg_sysdeps(x, msg);

		switch (p->previous) {
		case po_str:
			if ((t = poparser_feed_hdr(p, x)) != po_success) {
				return t;
			}

			cnt = p->strcnt - 1;
			if (p->stage == ps_parse) {
				len = unescape(x, &msg->str[cnt][msg->strlen[cnt]], p->max_strlen[cnt]);
			}

			msg->strlen[cnt] += len;
			break;
		case po_plural:
			if (p->stage == ps_parse) {
				len = unescape(x, &msg->plural[msg->plural_len], p->max_plural_len);
			}

			msg->plural_len += len;
			break;
		case po_id:
			if (p->stage == ps_parse) {
				len = unescape(x, &msg->id[msg->id_len], p->max_id_len);
			}

			msg->id_len += len;
			break;
		case po_ctxt:
			if (p->stage == ps_parse) {
				len = unescape(x, &msg->ctxt[msg->ctxt_len], p->max_ctxt_len);
			}

			msg->ctxt_len += len;
			break;
		default:
			return -po_invalid_entry;
		}
	} else if ((z = strstarts(line, "msg"))) {
		if ( (x = strchr(z, '"')) == NULL)
			return -po_excepted_token;

		if ( (y = strrchr(x+1, '"')) == NULL)
			return -po_excepted_token;

		len = y - ++x;
		*y = 0;

		if (p->cd) {
			line = x;
			line_len = len + 1;
			x = p->buf;
			len = p->bufsize;

			if (iconv(p->cd, &line, &line_len, &x, &len) == (size_t)-1)
				return -po_failed_iconv;

			if (line_len != 0)
				return -po_failed_iconv;

			len = x - p->buf;
			x = p->buf;
		}

		if ((y = strstarts(z, "ctxt")) && isspace(*y)) {
			if ( (t = poparser_clean(p, msg)) != po_success)
				return t;

			if (msg->id_len || msg->plural_len)
				return -po_invalid_entry;

			poparser_populate_msg_sysdeps(x, msg);

			if (p->stage == ps_parse) {
				if (msg->ctxt == NULL) {
					return -po_internal;
				}

				len = unescape(x, msg->ctxt, p->max_ctxt_len);
			}

			msg->ctxt_len = len;
			p->previous = po_ctxt;
		} else if ((y = strstarts(z, "id")) && isspace(*y)) {
			if ( (t = poparser_clean(p, msg)) != po_success)
				return t;

			if (msg->plural_len)
				return -po_invalid_entry;

			poparser_populate_msg_sysdeps(x, msg);

			if (p->stage == ps_parse) {
				if (msg->id == NULL) {
					return -po_internal;
				}

				len = unescape(x, msg->id, p->max_id_len);
			}

			msg->id_len = len;
			p->previous = po_id;
		} else if ((y = strstarts(z, "id_plural")) && isspace(*y)) {
			if (!msg->id_len || p->strcnt)
				return -po_invalid_entry;

			poparser_populate_msg_sysdeps(x, msg);

			if (p->stage == ps_parse) {
				if (msg->plural == NULL) {
					return -po_internal;
				}

				len = unescape(x, msg->plural, p->max_plural_len);
			}

			msg->plural_len = len;
			p->previous = po_plural;
		} else if ((y = strstarts(z, "str"))) {
			if (!msg->id_len && !p->first)
				return -po_invalid_entry;

			if (isspace(*y)) {
				if (p->strcnt || msg->plural_len)
					return -po_invalid_entry;

				cnt = (p->strcnt = 1) - 1;
			} else if (*y == '[') {
				if (!msg->plural_len)
					return -po_invalid_entry;

				if (y[2] != ']' || !isspace(y[3])) return -po_excepted_token;

				p->strcnt = (cnt = y[1] - '0') + 1;

				if (p->strict && p->strcnt > p->hdr.nplurals) {
					return -po_plurals_overflow;
				}
			} else {
				return -po_excepted_token;
			}

			if ((t = poparser_feed_hdr(p, x)) != po_success) {
				return t;
			}

			poparser_populate_msg_sysdeps(x, msg);

			if (p->stage == ps_parse) {
				if (msg->str[cnt] == NULL) {
					return -po_internal;
				}

				len = unescape(x, msg->str[cnt], p->max_strlen[cnt]);
			}

			msg->strlen[cnt] = len;
			p->previous = po_str;
		} else {
			return -po_invalid_entry;
		}
	}

	if (p->stage == ps_size) {
		if (p->max_strlen[cnt] < msg->strlen[cnt])
			p->max_strlen[cnt] = msg->strlen[cnt] + 1;
		if (p->max_plural_len < msg->plural_len)
			p->max_plural_len = msg->plural_len + 1;
		if (p->max_id_len < msg->id_len)
			p->max_id_len = msg->id_len + 1;
		if (p->max_ctxt_len < msg->ctxt_len)
			p->max_ctxt_len = msg->ctxt_len + 1;
	}

	return po_success;
}

enum po_error poparser_finish(struct po_parser *p) {
	size_t len;
	int cnt;
	enum po_error t;
	po_message_t msg = &p->msg;

	if (p->stage == ps_size) {
		if ( (t = poparser_clean(p, msg)) != po_success)
			return t;

		len = p->max_ctxt_len;
		len += p->max_id_len;
		len += p->max_plural_len;
		for (cnt = 0; cnt < MAX_NPLURALS; cnt++)
			len += p->max_strlen[cnt];

		memset(msg, 0, sizeof(struct po_message));
		msg->ctxt = (char*)malloc(len);
		msg->id = msg->ctxt + p->max_ctxt_len;
		msg->plural = msg->id + p->max_id_len;
		msg->str[0] = msg->plural + p->max_plural_len;
		for (cnt = 1; cnt < MAX_NPLURALS; cnt++)
			msg->str[cnt] = msg->str[cnt-1] + p->max_strlen[cnt-1];

		p->hdr.nplurals = 2;
		p->first = true;
	} else {
		if ( (t = poparser_clean(p, msg)) != po_success)
			return t;
		if (msg->ctxt) free(msg->ctxt);
		if (p->cd) iconv_close(p->cd);
	}

	if (p->stage < ps_parse) p->stage++;

	return po_success;
}

size_t poparser_sysdep(const char *in, char *out, int *sysdep_repidx) {
	const char *x, *y, *outs;
	size_t m;
	int n;
	outs = out;
	x = in;

	while ((y = strchr(x, '%'))) {
		y++;
		if (outs)
			memcpy(out, x, y-x);
		out += y-x;
		x = y;

		for (n=0; n < MAX_SYSDEP; n++) {
			m = strlen(sysdep_cases[n].format);
			if (!strncmp(y, sysdep_cases[n].format, m)) {
				x = y + m;

				y = sysdep_cases[n].repl[sysdep_repidx[n]];
				m = strlen(y);
				if (outs)
					memcpy(out, y, m);
				out += m;

				break;
			}
		}
	}

	m = strlen(x);
	if (outs)
		memcpy(out, x, m+1);
	out += m;
	return out - outs;
}
