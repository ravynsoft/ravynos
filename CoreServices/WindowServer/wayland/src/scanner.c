/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
 * Copyright © 2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "wayland-version.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>

#if HAVE_LIBXML
#include <libxml/parser.h>

/* Embedded wayland.dtd file */
/* static const char wayland_dtd[]; wayland.dtd */
#include "wayland.dtd.h"
#endif

/* Expat must be included after libxml as both want to declare XMLCALL; see
 * the Git commit that 'git blame' for this comment points to for more. */
#include <expat.h>

#include "wayland-util.h"

#define PROGRAM_NAME "wayland-scanner"

enum side {
	CLIENT,
	SERVER,
};

enum visibility {
	PRIVATE,
	PUBLIC,
};

static int
usage(int ret)
{
	fprintf(stderr, "usage: %s [OPTION] [client-header|server-header|private-code|public-code]"
		" [input_file output_file]\n", PROGRAM_NAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Converts XML protocol descriptions supplied on "
			"stdin or input file to client\n"
			"headers, server headers, or protocol marshalling code.\n\n"
			"Use \"public-code\" only if the marshalling code will be public - "
			"aka DSO will export it while other components will be using it.\n"
			"Using \"private-code\" is strongly recommended.\n\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "    -h,  --help                  display this help and exit.\n"
			"    -v,  --version               print the wayland library version that\n"
			"                                 the scanner was built against.\n"
			"    -c,  --include-core-only     include the core version of the headers,\n"
			"                                 that is e.g. wayland-client-core.h instead\n"
			"                                 of wayland-client.h.\n"
			"    -s,  --strict                exit immediately with an error if DTD\n"
			"                                 verification fails.\n");
	exit(ret);
}

static int
scanner_version(int ret)
{
	fprintf(stderr, "%s %s\n", PROGRAM_NAME, WAYLAND_VERSION);
	exit(ret);
}

static bool
is_dtd_valid(FILE *input, const char *filename)
{
	bool rc = true;
#if HAVE_LIBXML
	xmlParserCtxtPtr ctx = NULL;
	xmlDocPtr doc = NULL;
	xmlDtdPtr dtd = NULL;
	xmlValidCtxtPtr	dtdctx;
	xmlParserInputBufferPtr	buffer;
	int fd = fileno(input);

	dtdctx = xmlNewValidCtxt();
	ctx = xmlNewParserCtxt();
	if (!ctx || !dtdctx)
		abort();

	buffer = xmlParserInputBufferCreateMem(wayland_dtd,
					       sizeof(wayland_dtd),
					       XML_CHAR_ENCODING_UTF8);
	if (!buffer) {
		fprintf(stderr, "Failed to init buffer for DTD.\n");
		abort();
	}

	dtd = xmlIOParseDTD(NULL, buffer, XML_CHAR_ENCODING_UTF8);
	if (!dtd) {
		fprintf(stderr, "Failed to parse DTD.\n");
		abort();
	}

	doc = xmlCtxtReadFd(ctx, fd, filename, NULL, 0);
	if (!doc) {
		fprintf(stderr, "Failed to read XML\n");
		abort();
	}

	rc = xmlValidateDtd(dtdctx, doc, dtd);
	xmlFreeDoc(doc);
	xmlFreeParserCtxt(ctx);
	xmlFreeDtd(dtd);
	xmlFreeValidCtxt(dtdctx);
	/* xmlIOParseDTD consumes buffer */

	if (lseek(fd, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to reset fd, output would be garbage.\n");
		abort();
	}
#endif
	return rc;
}

#define XML_BUFFER_SIZE 4096

struct location {
	const char *filename;
	int line_number;
};

struct description {
	char *summary;
	char *text;
};

struct protocol {
	char *name;
	char *uppercase_name;
	struct wl_list interface_list;
	int type_index;
	int null_run_length;
	char *copyright;
	struct description *description;
	bool core_headers;
};

struct interface {
	struct location loc;
	char *name;
	char *uppercase_name;
	int version;
	int since;
	struct wl_list request_list;
	struct wl_list event_list;
	struct wl_list enumeration_list;
	struct wl_list link;
	struct description *description;
};

struct message {
	struct location loc;
	char *name;
	char *uppercase_name;
	struct wl_list arg_list;
	struct wl_list link;
	int arg_count;
	int new_id_count;
	int type_index;
	int all_null;
	int destructor;
	int since;
	struct description *description;
};

enum arg_type {
	NEW_ID,
	INT,
	UNSIGNED,
	FIXED,
	STRING,
	OBJECT,
	ARRAY,
	FD
};

struct arg {
	char *name;
	enum arg_type type;
	int nullable;
	char *interface_name;
	struct wl_list link;
	char *summary;
	char *enumeration_name;
};

struct enumeration {
	char *name;
	char *uppercase_name;
	struct wl_list entry_list;
	struct wl_list link;
	struct description *description;
	bool bitfield;
	int since;
};

struct entry {
	char *name;
	char *uppercase_name;
	char *value;
	char *summary;
	int since;
	struct wl_list link;
	struct description *description;
};

struct parse_context {
	struct location loc;
	XML_Parser parser;
	struct protocol *protocol;
	struct interface *interface;
	struct message *message;
	struct enumeration *enumeration;
	struct entry *entry;
	struct description *description;
	char character_data[8192];
	unsigned int character_data_length;
};

enum identifier_role {
	STANDALONE_IDENT,
	TRAILING_IDENT
};

static void *
fail_on_null(void *p)
{
	if (p == NULL) {
		fprintf(stderr, "%s: out of memory\n", PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}

	return p;
}

static void *
zalloc(size_t s)
{
	return calloc(s, 1);
}

static void *
xzalloc(size_t s)
{
	return fail_on_null(zalloc(s));
}

static char *
xstrdup(const char *s)
{
	return fail_on_null(strdup(s));
}

static char *
uppercase_dup(const char *src)
{
	char *u;
	int i;

	u = xstrdup(src);
	for (i = 0; u[i]; i++)
		u[i] = toupper(u[i]);
	u[i] = '\0';

	return u;
}

static const char *indent(int n)
{
	const char *whitespace[] = {
		"\t\t\t\t\t\t\t\t\t\t\t\t",
		"\t\t\t\t\t\t\t\t\t\t\t\t ",
		"\t\t\t\t\t\t\t\t\t\t\t\t  ",
		"\t\t\t\t\t\t\t\t\t\t\t\t   ",
		"\t\t\t\t\t\t\t\t\t\t\t\t    ",
		"\t\t\t\t\t\t\t\t\t\t\t\t     ",
		"\t\t\t\t\t\t\t\t\t\t\t\t      ",
		"\t\t\t\t\t\t\t\t\t\t\t\t       "
	};

	return whitespace[n % 8] + 12 - n / 8;
}

static void
desc_dump(char *desc, const char *fmt, ...) WL_PRINTF(2, 3);

static void
desc_dump(char *desc, const char *fmt, ...)
{
	va_list ap;
	char buf[128], hang;
	int col, i, j, k, startcol, newlines;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	for (i = 0, col = 0; buf[i] != '*'; i++) {
		if (buf[i] == '\t')
			col = (col + 8) & ~7;
		else
			col++;
	}

	printf("%s", buf);

	if (!desc) {
		printf("(none)\n");
		return;
	}

	startcol = col;
	col += strlen(&buf[i]);
	if (col - startcol > 2)
		hang = '\t';
	else
		hang = ' ';

	for (i = 0; desc[i]; ) {
		k = i;
		newlines = 0;
		while (desc[i] && isspace(desc[i])) {
			if (desc[i] == '\n')
				newlines++;
			i++;
		}
		if (!desc[i])
			break;

		j = i;
		while (desc[i] && !isspace(desc[i]))
			i++;

		if (newlines > 1)
			printf("\n%s*", indent(startcol));
		if (newlines > 1 || col + i - j > 72) {
			printf("\n%s*%c", indent(startcol), hang);
			col = startcol;
		}

		if (col > startcol && k > 0)
			col += printf(" ");
		col += printf("%.*s", i - j, &desc[j]);
	}
	putchar('\n');
}

static void __attribute__ ((noreturn))
fail(struct location *loc, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	fprintf(stderr, "%s:%d: error: ",
		loc->filename, loc->line_number);
	vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void
warn(struct location *loc, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	fprintf(stderr, "%s:%d: warning: ",
		loc->filename, loc->line_number);
	vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static bool
is_nullable_type(struct arg *arg)
{
	switch (arg->type) {
	/* Strings and objects are possibly nullable */
	case STRING:
	case OBJECT:
		return true;
	default:
		return false;
	}
}

static struct message *
create_message(struct location loc, const char *name)
{
	struct message *message;

	message = xzalloc(sizeof *message);
	message->loc = loc;
	message->name = xstrdup(name);
	message->uppercase_name = uppercase_dup(name);
	wl_list_init(&message->arg_list);

	return message;
}

static void
free_arg(struct arg *arg)
{
	free(arg->name);
	free(arg->interface_name);
	free(arg->summary);
	free(arg->enumeration_name);
	free(arg);
}

static struct arg *
create_arg(const char *name)
{
	struct arg *arg;

	arg = xzalloc(sizeof *arg);
	arg->name = xstrdup(name);

	return arg;
}

static bool
set_arg_type(struct arg *arg, const char *type)
{
	if (strcmp(type, "int") == 0)
		arg->type = INT;
	else if (strcmp(type, "uint") == 0)
		arg->type = UNSIGNED;
	else if (strcmp(type, "fixed") == 0)
		arg->type = FIXED;
	else if (strcmp(type, "string") == 0)
		arg->type = STRING;
	else if (strcmp(type, "array") == 0)
		arg->type = ARRAY;
	else if (strcmp(type, "fd") == 0)
		arg->type = FD;
	else if (strcmp(type, "new_id") == 0)
		arg->type = NEW_ID;
	else if (strcmp(type, "object") == 0)
		arg->type = OBJECT;
	else
		return false;

	return true;
}

static void
free_description(struct description *desc)
{
	if (!desc)
		return;

	free(desc->summary);
	free(desc->text);

	free(desc);
}

static void
free_message(struct message *message)
{
	struct arg *a, *a_next;

	free(message->name);
	free(message->uppercase_name);
	free_description(message->description);

	wl_list_for_each_safe(a, a_next, &message->arg_list, link)
		free_arg(a);

	free(message);
}

static struct enumeration *
create_enumeration(const char *name)
{
	struct enumeration *enumeration;

	enumeration = xzalloc(sizeof *enumeration);
	enumeration->name = xstrdup(name);
	enumeration->uppercase_name = uppercase_dup(name);
	enumeration->since = 1;

	wl_list_init(&enumeration->entry_list);

	return enumeration;
}

static struct entry *
create_entry(const char *name, const char *value)
{
	struct entry *entry;

	entry = xzalloc(sizeof *entry);
	entry->name = xstrdup(name);
	entry->uppercase_name = uppercase_dup(name);
	entry->value = xstrdup(value);

	return entry;
}

static void
free_entry(struct entry *entry)
{
	free(entry->name);
	free(entry->uppercase_name);
	free(entry->value);
	free(entry->summary);
	free_description(entry->description);

	free(entry);
}

static void
free_enumeration(struct enumeration *enumeration)
{
	struct entry *e, *e_next;

	free(enumeration->name);
	free(enumeration->uppercase_name);
	free_description(enumeration->description);

	wl_list_for_each_safe(e, e_next, &enumeration->entry_list, link)
		free_entry(e);

	free(enumeration);
}

static struct interface *
create_interface(struct location loc, const char *name, int version)
{
	struct interface *interface;

	interface = xzalloc(sizeof *interface);
	interface->loc = loc;
	interface->name = xstrdup(name);
	interface->uppercase_name = uppercase_dup(name);
	interface->version = version;
	interface->since = 1;
	wl_list_init(&interface->request_list);
	wl_list_init(&interface->event_list);
	wl_list_init(&interface->enumeration_list);

	return interface;
}

static void
free_interface(struct interface *interface)
{
	struct message *m, *next_m;
	struct enumeration *e, *next_e;

	free(interface->name);
	free(interface->uppercase_name);
	free_description(interface->description);

	wl_list_for_each_safe(m, next_m, &interface->request_list, link)
		free_message(m);
	wl_list_for_each_safe(m, next_m, &interface->event_list, link)
		free_message(m);
	wl_list_for_each_safe(e, next_e, &interface->enumeration_list, link)
		free_enumeration(e);

	free(interface);
}

/* Convert string to unsigned integer
 *
 * Parses a non-negative base-10 number from the given string.  If the
 * specified string is blank, contains non-numerical characters, is out
 * of range, or results in a negative number, -1 is returned to indicate
 * an error.
 *
 * Upon error, this routine does not modify or set errno.
 *
 * Returns -1 on error, or a non-negative integer on success
 */
static int
strtouint(const char *str)
{
	long int ret;
	char *end;
	int prev_errno = errno;

	errno = 0;
	ret = strtol(str, &end, 10);
	if (errno != 0 || end == str || *end != '\0')
		return -1;

	/* check range */
	if (ret < 0 || ret > INT_MAX) {
		return -1;
	}

	errno = prev_errno;
	return (int)ret;
}

/* Check that the provided string will produce valid "C" identifiers.
 *
 * If the string will form the prefix of an identifier in the
 * generated C code, then it must match [_a-zA-Z][_0-9a-zA-Z]*.
 *
 * If the string will form the suffix of an identifier, then
 * it must match [_0-9a-zA-Z]+.
 *
 * Unicode characters or escape sequences are not permitted,
 * since not all C compilers support them.
 *
 * If the above conditions are not met, then fail()
 */
static void
validate_identifier(struct location *loc,
		const char *str,
		enum identifier_role role)
{
	const char *scan;

	if (!*str) {
		fail(loc, "element name is empty");
	}

	for (scan = str; *scan; scan++) {
		char c = *scan;

		/* we do not use the locale-dependent `isalpha` */
		bool is_alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
		bool is_digit = c >= '0' && c <= '9';
		bool leading_char = (scan == str) && role == STANDALONE_IDENT;

		if (is_alpha || c == '_' || (!leading_char && is_digit))
			continue;

		if (role == TRAILING_IDENT)
			fail(loc,
			     "'%s' is not a valid trailing identifier part", str);
		else
			fail(loc,
			     "'%s' is not a valid standalone identifier", str);
	}
}

static int
version_from_since(struct parse_context *ctx, const char *since)
{
	int version;

	if (since != NULL) {
		version = strtouint(since);
		if (version == -1) {
			fail(&ctx->loc, "invalid integer (%s)\n", since);
		} else if (version > ctx->interface->version) {
			fail(&ctx->loc, "since (%u) larger than version (%u)\n",
			     version, ctx->interface->version);
		}
	} else {
		version = 1;
	}


	return version;
}

static void
start_element(void *data, const char *element_name, const char **atts)
{
	struct parse_context *ctx = data;
	struct interface *interface;
	struct message *message;
	struct arg *arg;
	struct enumeration *enumeration;
	struct entry *entry;
	struct description *description = NULL;
	const char *name = NULL;
	const char *type = NULL;
	const char *interface_name = NULL;
	const char *value = NULL;
	const char *summary = NULL;
	const char *since = NULL;
	const char *allow_null = NULL;
	const char *enumeration_name = NULL;
	const char *bitfield = NULL;
	int i, version = 0;

	ctx->loc.line_number = XML_GetCurrentLineNumber(ctx->parser);
	for (i = 0; atts[i]; i += 2) {
		if (strcmp(atts[i], "name") == 0)
			name = atts[i + 1];
		if (strcmp(atts[i], "version") == 0) {
			version = strtouint(atts[i + 1]);
			if (version == -1)
				fail(&ctx->loc, "wrong version (%s)", atts[i + 1]);
		}
		if (strcmp(atts[i], "type") == 0)
			type = atts[i + 1];
		if (strcmp(atts[i], "value") == 0)
			value = atts[i + 1];
		if (strcmp(atts[i], "interface") == 0)
			interface_name = atts[i + 1];
		if (strcmp(atts[i], "summary") == 0)
			summary = atts[i + 1];
		if (strcmp(atts[i], "since") == 0)
			since = atts[i + 1];
		if (strcmp(atts[i], "allow-null") == 0)
			allow_null = atts[i + 1];
		if (strcmp(atts[i], "enum") == 0)
			enumeration_name = atts[i + 1];
		if (strcmp(atts[i], "bitfield") == 0)
			bitfield = atts[i + 1];
	}

	ctx->character_data_length = 0;
	if (strcmp(element_name, "protocol") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no protocol name given");

		validate_identifier(&ctx->loc, name, STANDALONE_IDENT);
		ctx->protocol->name = xstrdup(name);
		ctx->protocol->uppercase_name = uppercase_dup(name);
	} else if (strcmp(element_name, "copyright") == 0) {

	} else if (strcmp(element_name, "interface") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no interface name given");

		if (version == 0)
			fail(&ctx->loc, "no interface version given");

		validate_identifier(&ctx->loc, name, STANDALONE_IDENT);
		interface = create_interface(ctx->loc, name, version);
		ctx->interface = interface;
		wl_list_insert(ctx->protocol->interface_list.prev,
			       &interface->link);
	} else if (strcmp(element_name, "request") == 0 ||
		   strcmp(element_name, "event") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no request name given");

		validate_identifier(&ctx->loc, name, STANDALONE_IDENT);
		message = create_message(ctx->loc, name);

		if (strcmp(element_name, "request") == 0)
			wl_list_insert(ctx->interface->request_list.prev,
				       &message->link);
		else
			wl_list_insert(ctx->interface->event_list.prev,
				       &message->link);

		if (type != NULL && strcmp(type, "destructor") == 0)
			message->destructor = 1;

		version = version_from_since(ctx, since);

		if (version < ctx->interface->since)
			warn(&ctx->loc, "since version not increasing\n");
		ctx->interface->since = version;
		message->since = version;

		if (strcmp(name, "destroy") == 0 && !message->destructor)
			fail(&ctx->loc, "destroy request should be destructor type");

		ctx->message = message;
	} else if (strcmp(element_name, "arg") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no argument name given");

		validate_identifier(&ctx->loc, name, STANDALONE_IDENT);
		arg = create_arg(name);
		if (!set_arg_type(arg, type))
			fail(&ctx->loc, "unknown type (%s)", type);

		switch (arg->type) {
		case NEW_ID:
			ctx->message->new_id_count++;
			/* fallthrough */
		case OBJECT:
			if (interface_name) {
				validate_identifier(&ctx->loc,
						    interface_name,
						    STANDALONE_IDENT);
				arg->interface_name = xstrdup(interface_name);
			}
			break;
		default:
			if (interface_name != NULL)
				fail(&ctx->loc, "interface attribute not allowed for type %s", type);
			break;
		}

		if (allow_null) {
			if (strcmp(allow_null, "true") == 0)
				arg->nullable = 1;
			else if (strcmp(allow_null, "false") != 0)
				fail(&ctx->loc,
				     "invalid value for allow-null attribute (%s)",
				     allow_null);

			if (!is_nullable_type(arg))
				fail(&ctx->loc,
				     "allow-null is only valid for objects, strings, and arrays");
		}

		if (enumeration_name == NULL || strcmp(enumeration_name, "") == 0)
			arg->enumeration_name = NULL;
		else
			arg->enumeration_name = xstrdup(enumeration_name);

		if (summary)
			arg->summary = xstrdup(summary);

		wl_list_insert(ctx->message->arg_list.prev, &arg->link);
		ctx->message->arg_count++;
	} else if (strcmp(element_name, "enum") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no enum name given");

		validate_identifier(&ctx->loc, name, TRAILING_IDENT);
		enumeration = create_enumeration(name);

		if (bitfield == NULL || strcmp(bitfield, "false") == 0)
			enumeration->bitfield = false;
		else if (strcmp(bitfield, "true") == 0)
			enumeration->bitfield = true;
		else
			fail(&ctx->loc,
			     "invalid value (%s) for bitfield attribute (only true/false are accepted)",
			     bitfield);

		wl_list_insert(ctx->interface->enumeration_list.prev,
			       &enumeration->link);

		ctx->enumeration = enumeration;
	} else if (strcmp(element_name, "entry") == 0) {
		if (name == NULL)
			fail(&ctx->loc, "no entry name given");

		validate_identifier(&ctx->loc, name, TRAILING_IDENT);
		entry = create_entry(name, value);
		version = version_from_since(ctx, since);

		if (version < ctx->enumeration->since)
			warn(&ctx->loc, "since version not increasing\n");
		ctx->enumeration->since = version;
		entry->since = version;

		if (summary)
			entry->summary = xstrdup(summary);
		else
			entry->summary = NULL;
		wl_list_insert(ctx->enumeration->entry_list.prev,
			       &entry->link);
		ctx->entry = entry;
	} else if (strcmp(element_name, "description") == 0) {
		if (summary == NULL)
			fail(&ctx->loc, "description without summary");

		description = xzalloc(sizeof *description);
		description->summary = xstrdup(summary);

		if (ctx->message)
			ctx->message->description = description;
		else if (ctx->entry)
			ctx->entry->description = description;
		else if (ctx->enumeration)
			ctx->enumeration->description = description;
		else if (ctx->interface)
			ctx->interface->description = description;
		else
			ctx->protocol->description = description;
		ctx->description = description;
	}
}

static struct enumeration *
find_enumeration(struct protocol *protocol,
		 struct interface *interface,
		 char *enum_attribute)
{
	struct interface *i;
	struct enumeration *e;
	char *enum_name;
	uint32_t idx = 0, j;

	for (j = 0; j + 1 < strlen(enum_attribute); j++) {
		if (enum_attribute[j] == '.') {
			idx = j;
		}
	}

	if (idx > 0) {
		enum_name = enum_attribute + idx + 1;

		wl_list_for_each(i, &protocol->interface_list, link)
			if (strncmp(i->name, enum_attribute, idx) == 0)
				wl_list_for_each(e, &i->enumeration_list, link)
					if (strcmp(e->name, enum_name) == 0)
						return e;
	} else if (interface) {
		enum_name = enum_attribute;

		wl_list_for_each(e, &interface->enumeration_list, link)
			if (strcmp(e->name, enum_name) == 0)
				return e;
	}

	return NULL;
}

static void
verify_arguments(struct parse_context *ctx,
		 struct interface *interface,
		 struct wl_list *messages,
		 struct wl_list *enumerations)
{
	struct message *m;
	wl_list_for_each(m, messages, link) {
		struct arg *a;
		wl_list_for_each(a, &m->arg_list, link) {
			struct enumeration *e;

			if (!a->enumeration_name)
				continue;


			e = find_enumeration(ctx->protocol, interface,
					     a->enumeration_name);

			switch (a->type) {
			case INT:
				if (e && e->bitfield)
					fail(&ctx->loc,
					     "bitfield-style enum must only be referenced by uint");
				break;
			case UNSIGNED:
				break;
			default:
				fail(&ctx->loc,
				     "enumeration-style argument has wrong type");
			}
		}
	}

}

#ifndef HAVE_STRNDUP
char *
strndup(const char *s, size_t size)
{
	char *r = malloc(size + 1);
	strncpy(r, s, size);
	r[size] = '\0';
	return r;
}
#endif

static void
end_element(void *data, const XML_Char *name)
{
	struct parse_context *ctx = data;

	if (strcmp(name, "copyright") == 0) {
		ctx->protocol->copyright =
			strndup(ctx->character_data,
				ctx->character_data_length);
	} else if (strcmp(name, "description") == 0) {
		ctx->description->text =
			strndup(ctx->character_data,
				ctx->character_data_length);
		ctx->description = NULL;
	} else if (strcmp(name, "request") == 0 ||
		   strcmp(name, "event") == 0) {
		ctx->message = NULL;
	} else if (strcmp(name, "enum") == 0) {
		if (wl_list_empty(&ctx->enumeration->entry_list)) {
			fail(&ctx->loc, "enumeration %s was empty",
			     ctx->enumeration->name);
		}
		ctx->enumeration = NULL;
	} else if (strcmp(name, "entry") == 0) {
		ctx->entry = NULL;
	} else if (strcmp(name, "protocol") == 0) {
		struct interface *i;

		wl_list_for_each(i, &ctx->protocol->interface_list, link) {
			verify_arguments(ctx, i, &i->request_list, &i->enumeration_list);
			verify_arguments(ctx, i, &i->event_list, &i->enumeration_list);
		}
	}
}

static void
character_data(void *data, const XML_Char *s, int len)
{
	struct parse_context *ctx = data;

	if (ctx->character_data_length + len > sizeof (ctx->character_data)) {
		fprintf(stderr, "too much character data");
		exit(EXIT_FAILURE);
	    }

	memcpy(ctx->character_data + ctx->character_data_length, s, len);
	ctx->character_data_length += len;
}

static void
format_text_to_comment(const char *text, bool standalone_comment)
{
	int bol = 1, start = 0, i, length;
	bool comment_started = !standalone_comment;

	length = strlen(text);
	for (i = 0; i <= length; i++) {
		if (bol && (text[i] == ' ' || text[i] == '\t')) {
			continue;
		} else if (bol) {
			bol = 0;
			start = i;
		}
		if (text[i] == '\n' ||
		    (text[i] == '\0' && !(start == i))) {
			printf("%s%s%.*s\n",
			       comment_started ? " *" : "/*",
			       i > start ? " " : "",
			       i - start, text + start);
			bol = 1;
			comment_started = true;
		}
	}
	if (comment_started && standalone_comment)
		printf(" */\n\n");
}

static void
emit_opcodes(struct wl_list *message_list, struct interface *interface)
{
	struct message *m;
	int opcode;

	if (wl_list_empty(message_list))
		return;

	opcode = 0;
	wl_list_for_each(m, message_list, link)
		printf("#define %s_%s %d\n",
		       interface->uppercase_name, m->uppercase_name, opcode++);

	printf("\n");
}

static void
emit_opcode_versions(struct wl_list *message_list, struct interface *interface)
{
	struct message *m;

	wl_list_for_each(m, message_list, link) {
		printf("/**\n * @ingroup iface_%s\n */\n", interface->name);
		printf("#define %s_%s_SINCE_VERSION %d\n",
		       interface->uppercase_name, m->uppercase_name, m->since);
	}

	printf("\n");
}

static void
emit_type(struct arg *a)
{
	switch (a->type) {
	default:
	case INT:
	case FD:
		printf("int32_t ");
		break;
	case NEW_ID:
	case UNSIGNED:
		printf("uint32_t ");
		break;
	case FIXED:
		printf("wl_fixed_t ");
		break;
	case STRING:
		printf("const char *");
		break;
	case OBJECT:
		printf("struct %s *", a->interface_name);
		break;
	case ARRAY:
		printf("struct wl_array *");
		break;
	}
}

static void
emit_stubs(struct wl_list *message_list, struct interface *interface)
{
	struct message *m;
	struct arg *a, *ret;
	int has_destructor, has_destroy;

	printf("/** @ingroup iface_%s */\n", interface->name);
	printf("static inline void\n"
	       "%s_set_user_data(struct %s *%s, void *user_data)\n"
	       "{\n"
	       "\twl_proxy_set_user_data((struct wl_proxy *) %s, user_data);\n"
	       "}\n\n",
	       interface->name, interface->name, interface->name,
	       interface->name);

	printf("/** @ingroup iface_%s */\n", interface->name);
	printf("static inline void *\n"
	       "%s_get_user_data(struct %s *%s)\n"
	       "{\n"
	       "\treturn wl_proxy_get_user_data((struct wl_proxy *) %s);\n"
	       "}\n\n",
	       interface->name, interface->name, interface->name,
	       interface->name);

	printf("static inline uint32_t\n"
	       "%s_get_version(struct %s *%s)\n"
	       "{\n"
	       "\treturn wl_proxy_get_version((struct wl_proxy *) %s);\n"
	       "}\n\n",
	       interface->name, interface->name, interface->name,
	       interface->name);

	has_destructor = 0;
	has_destroy = 0;
	wl_list_for_each(m, message_list, link) {
		if (m->destructor)
			has_destructor = 1;
		if (strcmp(m->name, "destroy") == 0)
			has_destroy = 1;
	}

	if (!has_destructor && has_destroy) {
		fail(&interface->loc,
		     "interface '%s' has method named destroy "
		     "but no destructor",
		     interface->name);
		exit(EXIT_FAILURE);
	}

	if (!has_destroy && strcmp(interface->name, "wl_display") != 0) {
		printf("/** @ingroup iface_%s */\n", interface->name);
		printf("static inline void\n"
		       "%s_destroy(struct %s *%s)\n"
		       "{\n"
		       "\twl_proxy_destroy("
		       "(struct wl_proxy *) %s);\n"
		       "}\n\n",
		       interface->name, interface->name, interface->name,
		       interface->name);
	}

	if (wl_list_empty(message_list))
		return;

	wl_list_for_each(m, message_list, link) {
		if (m->new_id_count > 1) {
			warn(&m->loc,
			     "request '%s::%s' has more than "
			     "one new_id arg, not emitting stub\n",
			     interface->name, m->name);
			continue;
		}

		ret = NULL;
		wl_list_for_each(a, &m->arg_list, link) {
			if (a->type == NEW_ID)
				ret = a;
		}

		printf("/**\n"
		       " * @ingroup iface_%s\n", interface->name);
		if (m->description && m->description->text)
			format_text_to_comment(m->description->text, false);
		printf(" */\n");
		if (ret && ret->interface_name == NULL)
			printf("static inline void *\n");
		else if (ret)
			printf("static inline struct %s *\n",
			       ret->interface_name);
		else
			printf("static inline void\n");

		printf("%s_%s(struct %s *%s",
		       interface->name, m->name,
		       interface->name, interface->name);

		wl_list_for_each(a, &m->arg_list, link) {
			if (a->type == NEW_ID && a->interface_name == NULL) {
				printf(", const struct wl_interface *interface"
				       ", uint32_t version");
				continue;
			} else if (a->type == NEW_ID)
				continue;
			printf(", ");
			emit_type(a);
			printf("%s", a->name);
		}

		printf(")\n"
		       "{\n");
		printf("\t");
		if (ret) {
			printf("struct wl_proxy *%s;\n\n"
			       "\t%s = ", ret->name, ret->name);
		}
		printf("wl_proxy_marshal_flags("
		       "(struct wl_proxy *) %s,\n"
		       "\t\t\t %s_%s",
		       interface->name,
		       interface->uppercase_name,
		       m->uppercase_name);

		if (ret) {
			if (ret->interface_name) {
				/* Normal factory case, an arg has type="new_id" and
				 * an interface is provided */
				printf(", &%s_interface", ret->interface_name);
			} else {
				/* an arg has type ="new_id" but interface is not
				 * provided, such as in wl_registry.bind */
				printf(", interface");
			}
		} else {
			/* No args have type="new_id" */
			printf(", NULL");
		}

		if (ret && ret->interface_name == NULL)
			printf(", version");
		else
			printf(", wl_proxy_get_version((struct wl_proxy *) %s)",
			       interface->name);
		printf(", %s", m->destructor ? "WL_MARSHAL_FLAG_DESTROY" : "0");

		wl_list_for_each(a, &m->arg_list, link) {
			if (a->type == NEW_ID) {
				if (a->interface_name == NULL)
					printf(", interface->name, version");
				printf(", NULL");
			} else {
				printf(", %s", a->name);
			}
		}
		printf(");\n");

		if (ret && ret->interface_name == NULL)
			printf("\n\treturn (void *) %s;\n", ret->name);
		else if (ret)
			printf("\n\treturn (struct %s *) %s;\n",
			       ret->interface_name, ret->name);

		printf("}\n\n");
	}
}

static void
emit_event_wrappers(struct wl_list *message_list, struct interface *interface)
{
	struct message *m;
	struct arg *a;

	/* We provide hand written functions for the display object */
	if (strcmp(interface->name, "wl_display") == 0)
		return;

	wl_list_for_each(m, message_list, link) {
		printf("/**\n"
		       " * @ingroup iface_%s\n"
		       " * Sends an %s event to the client owning the resource.\n",
		       interface->name,
		       m->name);
		printf(" * @param resource_ The client's resource\n");
		wl_list_for_each(a, &m->arg_list, link) {
			if (a->summary)
				printf(" * @param %s %s\n", a->name, a->summary);
		}
		printf(" */\n");
		printf("static inline void\n"
		       "%s_send_%s(struct wl_resource *resource_",
		       interface->name, m->name);

		wl_list_for_each(a, &m->arg_list, link) {
			printf(", ");
			switch (a->type) {
			case NEW_ID:
			case OBJECT:
				printf("struct wl_resource *");
				break;
			default:
				emit_type(a);
			}
			printf("%s", a->name);
		}

		printf(")\n"
		       "{\n"
		       "\twl_resource_post_event(resource_, %s_%s",
		       interface->uppercase_name, m->uppercase_name);

		wl_list_for_each(a, &m->arg_list, link)
			printf(", %s", a->name);

		printf(");\n");
		printf("}\n\n");
	}
}

static void
emit_enumerations(struct interface *interface)
{
	struct enumeration *e;
	struct entry *entry;

	wl_list_for_each(e, &interface->enumeration_list, link) {
		struct description *desc = e->description;

		printf("#ifndef %s_%s_ENUM\n",
		       interface->uppercase_name, e->uppercase_name);
		printf("#define %s_%s_ENUM\n",
		       interface->uppercase_name, e->uppercase_name);

		if (desc) {
			printf("/**\n");
			printf(" * @ingroup iface_%s\n", interface->name);
			format_text_to_comment(desc->summary, false);
			if (desc->text)
				format_text_to_comment(desc->text, false);
			printf(" */\n");
		}
		printf("enum %s_%s {\n", interface->name, e->name);
		wl_list_for_each(entry, &e->entry_list, link) {
			desc = entry->description;
			if (entry->summary || entry->since > 1 || desc) {
				printf("\t/**\n");
				if (entry->summary)
					printf("\t * %s\n", entry->summary);
				if (desc) {
					printf("\t * %s\n", desc->summary);
					printf("\t *\n");
					if (desc->text)
						desc_dump(desc->text, "\t * ");
				}
				if (entry->since > 1)
					printf("\t * @since %d\n", entry->since);
				printf("\t */\n");
			}
			printf("\t%s_%s_%s = %s,\n",
			       interface->uppercase_name,
			       e->uppercase_name,
			       entry->uppercase_name, entry->value);
		}
		printf("};\n");

		wl_list_for_each(entry, &e->entry_list, link) {
			if (entry->since == 1)
                            continue;

                        printf("/**\n * @ingroup iface_%s\n */\n", interface->name);
                        printf("#define %s_%s_%s_SINCE_VERSION %d\n",
                               interface->uppercase_name,
                               e->uppercase_name, entry->uppercase_name,
                               entry->since);

		}

		printf("#endif /* %s_%s_ENUM */\n\n",
		       interface->uppercase_name, e->uppercase_name);
	}
}

static void
emit_structs(struct wl_list *message_list, struct interface *interface, enum side side)
{
	struct message *m;
	struct arg *a;
	int n;

	if (wl_list_empty(message_list))
		return;

	printf("/**\n");
	printf(" * @ingroup iface_%s\n", interface->name);
	printf(" * @struct %s_%s\n", interface->name,
	       (side == SERVER) ? "interface" : "listener");
	printf(" */\n");
	printf("struct %s_%s {\n", interface->name,
	       (side == SERVER) ? "interface" : "listener");

	wl_list_for_each(m, message_list, link) {
		struct description *mdesc = m->description;

		printf("\t/**\n");
		if (mdesc) {
			if (mdesc->summary)
				printf("\t * %s\n", mdesc->summary);
			printf("\t *\n");
			desc_dump(mdesc->text, "\t * ");
		}
		wl_list_for_each(a, &m->arg_list, link) {
			if (side == SERVER && a->type == NEW_ID &&
			    a->interface_name == NULL)
				printf("\t * @param interface name of the objects interface\n"
				       "\t * @param version version of the objects interface\n");

			if (a->summary)
				printf("\t * @param %s %s\n", a->name,
				       a->summary);
		}
		if (m->since > 1) {
			printf("\t * @since %d\n", m->since);
		}
		printf("\t */\n");
		printf("\tvoid (*%s)(", m->name);

		n = strlen(m->name) + 17;
		if (side == SERVER) {
			printf("struct wl_client *client,\n"
			       "%sstruct wl_resource *resource",
			       indent(n));
		} else {
			printf("void *data,\n"),
			printf("%sstruct %s *%s",
			       indent(n), interface->name, interface->name);
		}

		wl_list_for_each(a, &m->arg_list, link) {
			printf(",\n%s", indent(n));

			if (side == SERVER && a->type == OBJECT)
				printf("struct wl_resource *");
			else if (side == SERVER && a->type == NEW_ID && a->interface_name == NULL)
				printf("const char *interface, uint32_t version, uint32_t ");
			else if (side == CLIENT && a->type == OBJECT && a->interface_name == NULL)
				printf("void *");

			else if (side == CLIENT && a->type == NEW_ID)
				printf("struct %s *", a->interface_name);
			else
				emit_type(a);

			printf("%s", a->name);
		}

		printf(");\n");
	}

	printf("};\n\n");

	if (side == CLIENT) {
	    printf("/**\n"
		   " * @ingroup iface_%s\n"
		   " */\n", interface->name);
	    printf("static inline int\n"
		   "%s_add_listener(struct %s *%s,\n"
		   "%sconst struct %s_listener *listener, void *data)\n"
		   "{\n"
		   "\treturn wl_proxy_add_listener((struct wl_proxy *) %s,\n"
		   "%s(void (**)(void)) listener, data);\n"
		   "}\n\n",
		   interface->name, interface->name, interface->name,
		   indent(14 + strlen(interface->name)),
		   interface->name,
		   interface->name,
		   indent(37));
	}
}

static void
emit_types_forward_declarations(struct protocol *protocol,
				struct wl_list *message_list,
				struct wl_array *types)
{
	struct message *m;
	struct arg *a;
	int length;
	char **p;

	wl_list_for_each(m, message_list, link) {
		length = 0;
		m->all_null = 1;
		wl_list_for_each(a, &m->arg_list, link) {
			length++;
			switch (a->type) {
			case NEW_ID:
			case OBJECT:
				if (!a->interface_name)
					continue;

				m->all_null = 0;
				p = fail_on_null(wl_array_add(types, sizeof *p));
				*p = a->interface_name;
				break;
			default:
				break;
			}
		}

		if (m->all_null && length > protocol->null_run_length)
			protocol->null_run_length = length;
	}
}

static int
cmp_names(const void *p1, const void *p2)
{
	const char * const *s1 = p1, * const *s2 = p2;

	return strcmp(*s1, *s2);
}

static const char *
get_include_name(bool core, enum side side)
{
	if (side == SERVER)
		return core ? "wayland-server-core.h" : "wayland-server.h";
	else
		return core ? "wayland-client-core.h" : "wayland-client.h";
}

static void
emit_mainpage_blurb(const struct protocol *protocol, enum side side)
{
	struct interface *i;

	printf("/**\n"
	       " * @page page_%s The %s protocol\n",
	       protocol->name, protocol->name);

	if (protocol->description) {
		if (protocol->description->summary) {
			printf(" * %s\n"
			       " *\n", protocol->description->summary);
		}

		if (protocol->description->text) {
			printf(" * @section page_desc_%s Description\n", protocol->name);
			format_text_to_comment(protocol->description->text, false);
			printf(" *\n");
		}
	}

	printf(" * @section page_ifaces_%s Interfaces\n", protocol->name);
	wl_list_for_each(i, &protocol->interface_list, link) {
		printf(" * - @subpage page_iface_%s - %s\n",
		       i->name,
		       i->description && i->description->summary ?  i->description->summary : "");
	}

	if (protocol->copyright) {
		printf(" * @section page_copyright_%s Copyright\n",
		       protocol->name);
		printf(" * <pre>\n");
		format_text_to_comment(protocol->copyright, false);
		printf(" * </pre>\n");
	}

	printf(" */\n");
}

static void
emit_header(struct protocol *protocol, enum side side)
{
	struct interface *i, *i_next;
	struct wl_array types;
	const char *s = (side == SERVER) ? "SERVER" : "CLIENT";
	char **p, *prev;

	printf("/* Generated by %s %s */\n\n", PROGRAM_NAME, WAYLAND_VERSION);

	printf("#ifndef %s_%s_PROTOCOL_H\n"
	       "#define %s_%s_PROTOCOL_H\n"
	       "\n"
	       "#include <stdint.h>\n"
	       "#include <stddef.h>\n"
	       "#include \"%s\"\n\n"
	       "#ifdef  __cplusplus\n"
	       "extern \"C\" {\n"
	       "#endif\n\n",
	       protocol->uppercase_name, s,
	       protocol->uppercase_name, s,
	       get_include_name(protocol->core_headers, side));
	if (side == SERVER)
		printf("struct wl_client;\n"
		       "struct wl_resource;\n\n");

	emit_mainpage_blurb(protocol, side);

	wl_array_init(&types);
	wl_list_for_each(i, &protocol->interface_list, link) {
		emit_types_forward_declarations(protocol, &i->request_list, &types);
		emit_types_forward_declarations(protocol, &i->event_list, &types);
	}

	wl_list_for_each(i, &protocol->interface_list, link) {
		p = fail_on_null(wl_array_add(&types, sizeof *p));
		*p = i->name;
	}

	if (types.size > 0)
		qsort(types.data, types.size / sizeof *p, sizeof *p, cmp_names);

	prev = NULL;
	wl_array_for_each(p, &types) {
		if (prev && strcmp(*p, prev) == 0)
			continue;
		printf("struct %s;\n", *p);
		prev = *p;
	}
	wl_array_release(&types);
	printf("\n");

	wl_list_for_each(i, &protocol->interface_list, link) {
		printf("#ifndef %s_INTERFACE\n", i->uppercase_name);
		printf("#define %s_INTERFACE\n", i->uppercase_name);
		printf("/**\n"
		       " * @page page_iface_%s %s\n",
		       i->name, i->name);
		if (i->description && i->description->text) {
			printf(" * @section page_iface_%s_desc Description\n",
			       i->name);
			format_text_to_comment(i->description->text, false);
		}
		printf(" * @section page_iface_%s_api API\n"
		       " * See @ref iface_%s.\n"
		       " */\n",
		       i->name, i->name);
		printf("/**\n"
		       " * @defgroup iface_%s The %s interface\n",
		       i->name, i->name);
		if (i->description && i->description->text)
			format_text_to_comment(i->description->text, false);
		printf(" */\n");
		printf("extern const struct wl_interface "
		       "%s_interface;\n", i->name);
		printf("#endif\n");
	}

	printf("\n");

	wl_list_for_each_safe(i, i_next, &protocol->interface_list, link) {

		emit_enumerations(i);

		if (side == SERVER) {
			emit_structs(&i->request_list, i, side);
			emit_opcodes(&i->event_list, i);
			emit_opcode_versions(&i->event_list, i);
			emit_opcode_versions(&i->request_list, i);
			emit_event_wrappers(&i->event_list, i);
		} else {
			emit_structs(&i->event_list, i, side);
			emit_opcodes(&i->request_list, i);
			emit_opcode_versions(&i->event_list, i);
			emit_opcode_versions(&i->request_list, i);
			emit_stubs(&i->request_list, i);
		}

		free_interface(i);
	}

	printf("#ifdef  __cplusplus\n"
	       "}\n"
	       "#endif\n"
	       "\n"
	       "#endif\n");
}

static void
emit_null_run(struct protocol *protocol)
{
	int i;

	for (i = 0; i < protocol->null_run_length; i++)
		printf("\tNULL,\n");
}

static void
emit_types(struct protocol *protocol, struct wl_list *message_list)
{
	struct message *m;
	struct arg *a;

	wl_list_for_each(m, message_list, link) {
		if (m->all_null) {
			m->type_index = 0;
			continue;
		}

		m->type_index =
			protocol->null_run_length + protocol->type_index;
		protocol->type_index += m->arg_count;

		wl_list_for_each(a, &m->arg_list, link) {
			switch (a->type) {
			case NEW_ID:
			case OBJECT:
				if (a->interface_name)
					printf("\t&%s_interface,\n",
					       a->interface_name);
				else
					printf("\tNULL,\n");
				break;
			default:
				printf("\tNULL,\n");
				break;
			}
		}
	}
}

static void
emit_messages(const char *name, struct wl_list *message_list,
	      struct interface *interface, const char *suffix)
{
	struct message *m;
	struct arg *a;

	if (wl_list_empty(message_list))
		return;

	printf("static const struct wl_message "
	       "%s_%s[] = {\n",
	       interface->name, suffix);

	wl_list_for_each(m, message_list, link) {
		printf("\t{ \"%s\", \"", m->name);

		if (m->since > 1)
			printf("%d", m->since);

		wl_list_for_each(a, &m->arg_list, link) {
			if (is_nullable_type(a) && a->nullable)
				printf("?");

			switch (a->type) {
			default:
			case INT:
				printf("i");
				break;
			case NEW_ID:
				if (a->interface_name == NULL)
					printf("su");
				printf("n");
				break;
			case UNSIGNED:
				printf("u");
				break;
			case FIXED:
				printf("f");
				break;
			case STRING:
				printf("s");
				break;
			case OBJECT:
				printf("o");
				break;
			case ARRAY:
				printf("a");
				break;
			case FD:
				printf("h");
				break;
			}
		}
		printf("\", %s_types + %d },\n", name, m->type_index);
	}

	printf("};\n\n");
}


static void
emit_code(struct protocol *protocol, enum visibility vis)
{
	const char *symbol_visibility;
	struct interface *i, *next;
	struct wl_array types;
	char **p, *prev;

	printf("/* Generated by %s %s */\n\n", PROGRAM_NAME, WAYLAND_VERSION);

	if (protocol->copyright)
		format_text_to_comment(protocol->copyright, true);

	printf("#include <stdlib.h>\n"
	       "#include <stdint.h>\n"
	       "#include \"wayland-util.h\"\n\n");

	/* When building a shared library symbols must be exported, otherwise
	 * we want to have the symbols hidden. */
	if (vis == PRIVATE) {
		symbol_visibility = "WL_PRIVATE";
		printf("#ifndef __has_attribute\n"
		       "# define __has_attribute(x) 0  /* Compatibility with non-clang compilers. */\n"
		       "#endif\n\n");

		printf("#if (__has_attribute(visibility) || defined(__GNUC__) && __GNUC__ >= 4)\n"
		       "#define WL_PRIVATE __attribute__ ((visibility(\"hidden\")))\n"
		       "#else\n"
		       "#define WL_PRIVATE\n"
		       "#endif\n\n");
	} else {
		symbol_visibility = "WL_EXPORT";
	}

	wl_array_init(&types);
	wl_list_for_each(i, &protocol->interface_list, link) {
		emit_types_forward_declarations(protocol, &i->request_list, &types);
		emit_types_forward_declarations(protocol, &i->event_list, &types);
	}

	if (types.size > 0)
		qsort(types.data, types.size / sizeof *p, sizeof *p, cmp_names);

	prev = NULL;
	wl_array_for_each(p, &types) {
		if (prev && strcmp(*p, prev) == 0)
			continue;
		printf("extern const struct wl_interface %s_interface;\n", *p);
		prev = *p;
	}
	wl_array_release(&types);
	printf("\n");

	printf("static const struct wl_interface *%s_types[] = {\n", protocol->name);
	emit_null_run(protocol);
	wl_list_for_each(i, &protocol->interface_list, link) {
		emit_types(protocol, &i->request_list);
		emit_types(protocol, &i->event_list);
	}
	printf("};\n\n");

	wl_list_for_each_safe(i, next, &protocol->interface_list, link) {

		emit_messages(protocol->name, &i->request_list, i, "requests");
		emit_messages(protocol->name, &i->event_list, i, "events");

		printf("%s const struct wl_interface "
		       "%s_interface = {\n"
		       "\t\"%s\", %d,\n",
		       symbol_visibility, i->name, i->name, i->version);

		if (!wl_list_empty(&i->request_list))
			printf("\t%d, %s_requests,\n",
			       wl_list_length(&i->request_list), i->name);
		else
			printf("\t0, NULL,\n");

		if (!wl_list_empty(&i->event_list))
			printf("\t%d, %s_events,\n",
			       wl_list_length(&i->event_list), i->name);
		else
			printf("\t0, NULL,\n");

		printf("};\n\n");

		/* we won't need it any further */
		free_interface(i);
	}
}

static void
free_protocol(struct protocol *protocol)
{
	free(protocol->name);
	free(protocol->uppercase_name);
	free(protocol->copyright);
	free_description(protocol->description);
}

int main(int argc, char *argv[])
{
	struct parse_context ctx;
	struct protocol protocol;
	FILE *input = stdin;
	char *input_filename = NULL;
	int len;
	void *buf;
	bool help = false;
	bool core_headers = false;
	bool version = false;
	bool strict = false;
	bool fail = false;
	int opt;
	enum {
		CLIENT_HEADER,
		SERVER_HEADER,
		PRIVATE_CODE,
		PUBLIC_CODE,
		CODE,
	} mode;

	static const struct option options[] = {
		{ "help",              no_argument, NULL, 'h' },
		{ "version",           no_argument, NULL, 'v' },
		{ "include-core-only", no_argument, NULL, 'c' },
		{ "strict",            no_argument, NULL, 's' },
		{ 0,                   0,           NULL, 0 }
	};

	while (1) {
		opt = getopt_long(argc, argv, "hvcs", options, NULL);

		if (opt == -1)
			break;

		switch (opt) {
		case 'h':
			help = true;
			break;
		case 'v':
			version = true;
			break;
		case 'c':
			core_headers = true;
			break;
		case 's':
			strict = true;
			break;
		default:
			fail = true;
			break;
		}
	}

	argv += optind;
	argc -= optind;

	if (help)
		usage(EXIT_SUCCESS);
	else if (version)
		scanner_version(EXIT_SUCCESS);
	else if ((argc != 1 && argc != 3) || fail)
		usage(EXIT_FAILURE);
	else if (strcmp(argv[0], "help") == 0)
		usage(EXIT_SUCCESS);
	else if (strcmp(argv[0], "client-header") == 0)
		mode = CLIENT_HEADER;
	else if (strcmp(argv[0], "server-header") == 0)
		mode = SERVER_HEADER;
	else if (strcmp(argv[0], "private-code") == 0)
		mode = PRIVATE_CODE;
	else if (strcmp(argv[0], "public-code") == 0)
		mode = PUBLIC_CODE;
	else if (strcmp(argv[0], "code") == 0)
		mode = CODE;
	else
		usage(EXIT_FAILURE);

	if (argc == 3) {
		input_filename = argv[1];
		input = fopen(input_filename, "r");
		if (input == NULL) {
			fprintf(stderr, "Could not open input file: %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (freopen(argv[2], "w", stdout) == NULL) {
			fprintf(stderr, "Could not open output file: %s\n",
				strerror(errno));
			fclose(input);
			exit(EXIT_FAILURE);
		}
	}

	/* initialize protocol structure */
	memset(&protocol, 0, sizeof protocol);
	wl_list_init(&protocol.interface_list);
	protocol.core_headers = core_headers;

	/* initialize context */
	memset(&ctx, 0, sizeof ctx);
	ctx.protocol = &protocol;
	if (input == stdin)
		ctx.loc.filename = "<stdin>";
	else
		ctx.loc.filename = input_filename;

	if (!is_dtd_valid(input, ctx.loc.filename)) {
		fprintf(stderr,
		"*******************************************************\n"
		"*                                                     *\n"
		"* WARNING: XML failed validation against built-in DTD *\n"
		"*                                                     *\n"
		"*******************************************************\n");
		if (strict) {
			fclose(input);
			exit(EXIT_FAILURE);
		}
	}

	/* create XML parser */
	ctx.parser = XML_ParserCreate(NULL);
	XML_SetUserData(ctx.parser, &ctx);
	if (ctx.parser == NULL) {
		fprintf(stderr, "failed to create parser\n");
		fclose(input);
		exit(EXIT_FAILURE);
	}

	XML_SetElementHandler(ctx.parser, start_element, end_element);
	XML_SetCharacterDataHandler(ctx.parser, character_data);

	do {
		buf = XML_GetBuffer(ctx.parser, XML_BUFFER_SIZE);
		len = fread(buf, 1, XML_BUFFER_SIZE, input);
		if (len < 0) {
			fprintf(stderr, "fread: %s\n", strerror(errno));
			fclose(input);
			exit(EXIT_FAILURE);
		}
		if (XML_ParseBuffer(ctx.parser, len, len == 0) == 0) {
			fprintf(stderr,
				"Error parsing XML at line %ld col %ld: %s\n",
				XML_GetCurrentLineNumber(ctx.parser),
				XML_GetCurrentColumnNumber(ctx.parser),
				XML_ErrorString(XML_GetErrorCode(ctx.parser)));
			fclose(input);
			exit(EXIT_FAILURE);
		}
	} while (len > 0);

	XML_ParserFree(ctx.parser);

	switch (mode) {
		case CLIENT_HEADER:
			emit_header(&protocol, CLIENT);
			break;
		case SERVER_HEADER:
			emit_header(&protocol, SERVER);
			break;
		case PRIVATE_CODE:
			emit_code(&protocol, PRIVATE);
			break;
		case CODE:
			fprintf(stderr,
				"Using \"code\" is deprecated - use "
				"private-code or public-code.\n"
				"See the help page for details.\n");
			/* fallthrough */
		case PUBLIC_CODE:
			emit_code(&protocol, PUBLIC);
			break;
	}

	free_protocol(&protocol);
	fclose(input);

	return 0;
}
