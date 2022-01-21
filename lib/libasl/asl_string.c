/*
 * Copyright (c) 2007-2013 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <asl.h>
#include <string.h>
#include <mach/kern_return.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <vm/vm_param.h>
#ifdef __FreeBSD__
#include <atomic_compat.h>
#else
#include <libkern/OSAtomic.h>
#endif
#include <asl_string.h>
#include <asl_private.h>

#define ASL_STRING_QUANTUM 256
static const char *cvis_7_13 = "abtnvfr";

asl_string_t *
asl_string_new(uint32_t encoding)
{
	asl_string_t *str = (asl_string_t *)calloc(1, sizeof(asl_string_t));
	if (str == NULL) return NULL;

	str->asl_type = ASL_TYPE_STRING;
	str->refcount = 1;

	str->encoding = encoding;
	str->delta = ASL_STRING_QUANTUM;
	if (encoding & ASL_STRING_VM) str->delta = PAGE_SIZE;
	str->bufsize = 0;
	str->cursor = 0;

	if (encoding & ASL_STRING_LEN) asl_string_append_no_encoding(str, "         0 ");
	return str;
}

asl_string_t *
asl_string_retain(asl_string_t *str)
{
	if (str == NULL) return NULL;

	OSAtomicIncrement32Barrier(&(str->refcount));
	return str;
}

void
asl_string_release(asl_string_t *str)
{
	if (str == NULL) return;
	if (OSAtomicDecrement32Barrier(&(str->refcount)) != 0) return;

	if (str->encoding & ASL_STRING_VM)
	{
		vm_deallocate(mach_task_self(), (vm_address_t)str->buf, str->bufsize);
	}
	else
	{
		free(str->buf);
	}

	free(str);
}

char *
asl_string_release_return_bytes(asl_string_t *str)
{
	char *out;
	if (str == NULL) return NULL;

	if (OSAtomicDecrement32Barrier(&(str->refcount)) != 0)
	{
		/* string is still retained - copy buf */
		if (str->encoding & ASL_STRING_VM)
		{
			if (str->bufsize == 0) return NULL;

			vm_address_t new = 0;
			kern_return_t kstatus = vm_allocate(mach_task_self(), &new, str->bufsize, TRUE);
			if (kstatus != KERN_SUCCESS) return NULL;

			memcpy((void *)new, str->buf, str->bufsize);
			return (char *)new;
		}
		else
		{
			if (str->cursor == 0) return NULL;
			return strdup(str->buf);
		}
	}

	out = str->buf;
	free(str);
	return out;
}

char *
asl_string_bytes(asl_string_t *str)
{
	if (str == NULL) return NULL;
	return str->buf;
}

/* length includes trailing nul */
size_t
asl_string_length(asl_string_t *str)
{
	if (str == NULL) return 0;
	if (str->cursor == 0) return 0;

	return str->cursor + 1;
}

size_t
asl_string_allocated_size(asl_string_t *str)
{
	if (str == NULL) return 0;
	return str->bufsize;
}

static int
_asl_string_grow(asl_string_t *str, size_t len)
{
	size_t newlen = 0;

	if (str == NULL) return -1;
	if (len == 0) return 0;

	if (str->bufsize == 0)
	{
		newlen = ((len + str->delta - 1) / str->delta) * str->delta;
	}
	else
	{
		/* used size is (str->cursor + 1) including tailiing nul */
		if (len <= (str->bufsize - (str->cursor + 1))) return 0;

		/* really this is ((str->cursor + 1) + len + (str->delta - 1)) */
		newlen = ((str->cursor + len + str->delta) / str->delta) * str->delta;
	}

	if (str->encoding & ASL_STRING_VM)
	{
		kern_return_t kstatus;
		vm_address_t new = 0;

		kstatus = vm_allocate(mach_task_self(), &new, newlen, TRUE);
		if (kstatus != KERN_SUCCESS)
		{
			new = 0;
			newlen = 0;
			return -1;
		}

		if (str->buf != NULL)
		{
			memcpy((void *)new, str->buf, str->bufsize);
			vm_deallocate(mach_task_self(), (vm_address_t)str->buf, str->bufsize);
		}

		str->buf = (char *)new;
		str->bufsize = newlen;
	}
	else
	{
		str->buf = reallocf(str->buf, newlen);
		if (str->buf == NULL)
		{
			str->cursor = 0;
			str->bufsize = 0;
			return -1;
		}

		str->bufsize = newlen;
	}

	return 0;
}

asl_string_t *
asl_string_append_char_no_encoding(asl_string_t *str, const char c)
{
	size_t len;

	if (str == NULL) return NULL;

	len = 1;
	if (str->bufsize == 0) len++;

	if (_asl_string_grow(str, len) < 0) return str;

	str->buf[str->cursor] = c;
	str->cursor++;
	str->buf[str->cursor] = '\0';

	if (str->encoding & ASL_STRING_LEN)
	{
		char tmp[11];
		snprintf(tmp, sizeof(tmp), "%10lu", str->cursor - 10);
		memcpy(str->buf, tmp, 10);
	}

	return str;
}

asl_string_t *
asl_string_append_no_encoding(asl_string_t *str, const char *app)
{
	size_t len, applen;

	if (str == NULL) return NULL;
	if (app == NULL) return str;

	applen = strlen(app);
	len = applen;
	if (str->bufsize == 0) len++;

	if (_asl_string_grow(str, len) < 0) return str;

	memcpy(str->buf + str->cursor, app, applen);

	str->cursor += applen;
	str->buf[str->cursor] = '\0';

	if (str->encoding & ASL_STRING_LEN)
	{
		char tmp[11];
		snprintf(tmp, sizeof(tmp), "%10lu", str->cursor - 10);
		memcpy(str->buf, tmp, 10);
	}

	return str;
}

static asl_string_t *
asl_string_append_internal(asl_string_t *str, const char *app, int encode_space)
{
	uint8_t x;
	const char *p;

	if (str == NULL) return NULL;
	if (app == NULL) return str;

	switch (str->encoding & ASL_ENCODE_MASK)
	{
		case ASL_ENCODE_NONE:
		{
			return asl_string_append_no_encoding(str, app);
		}
		case ASL_ENCODE_SAFE:
		{
			/* minor encoding to reduce the likelyhood of spoof attacks */
			const char *p;

			for (p = app; *p != '\0'; p++)
			{
				if ((*p == 10) || (*p == 13))
				{
					asl_string_append_no_encoding(str, "\n\t");
				}
				else if (*p == 8)
				{
					asl_string_append_no_encoding(str, "^H");
				}
				else
				{
					asl_string_append_char_no_encoding(str, *p);
				}
			}

			return str;
		}
		case ASL_ENCODE_ASL:
		{
			for (p = app; *p != '\0'; p++)
			{
				int meta = 0;

				x = *p;

				/* Meta chars get \M prefix */
				if (x >= 128)
				{
					/* except meta-space, which is \240 */
					if (x == 160)
					{
						asl_string_append_no_encoding(str, "\\240");
						continue;
					}

					asl_string_append_no_encoding(str, "\\M");
					x &= 0x7f;
					meta = 1;
				}

				/* space is either ' ' or \s */
				if (x == 32)
				{
					if (encode_space == 0)
					{
						asl_string_append_char_no_encoding(str, ' ');
						continue;
					}

					asl_string_append_no_encoding(str, "\\s");
					continue;
				}

				/* \ is escaped */
				if ((meta == 0) && (x == 92))
				{
					asl_string_append_no_encoding(str, "\\\\");
					continue;
				}

				/* [ and ] are escaped in ASL encoding */
				if ((str->encoding & ASL_ENCODE_ASL) && (meta == 0) && ((*p == 91) || (*p == 93)))
				{
					if (*p == '[') asl_string_append_no_encoding(str, "\\[");
					else asl_string_append_no_encoding(str, "\\]");
					continue;
				}

				/* DEL is \^? */
				if (x == 127)
				{
					if (meta == 0)
					{
						asl_string_append_char_no_encoding(str, '\\');
					}

					asl_string_append_no_encoding(str, "^?");
					continue;
				}

				/* 33-126 are printable (add a '-' prefix for meta) */
				if ((x >= 33) && (x <= 126))
				{
					if (meta == 1)
					{
						asl_string_append_char_no_encoding(str, '-');
					}

					asl_string_append_char_no_encoding(str, x);
					continue;
				}

				/* non-meta BEL, BS, HT, NL, VT, NP, CR (7-13) are \a, \b, \t, \n, \v, \f, and \r */
				if ((meta == 0) && (x >= 7) && (x <= 13))
				{
					asl_string_append_char_no_encoding(str, '\\');
					asl_string_append_char_no_encoding(str, cvis_7_13[x - 7]);
					continue;
				}

				/* 0 - 31 are ^@ - ^_ (non-meta get a leading \) */
				if (x <= 31)
				{
					if (meta == 0)
					{
						asl_string_append_char_no_encoding(str, '\\');
					}

					asl_string_append_char_no_encoding(str, '^');
					asl_string_append_char_no_encoding(str, 64 + x);
					continue;
				}

				asl_string_append_char_no_encoding(str, x);
			}

			return str;
		}
		case ASL_ENCODE_XML:
		{
			for (p = app; *p != '\0'; p++)
			{
				x = *p;

				if (x == '&')
				{
					asl_string_append_no_encoding(str, "&amp;");
				}
				else if (x == '<')
				{
					asl_string_append_no_encoding(str, "&lt;");
				}
				else if (x == '>')
				{
					asl_string_append_no_encoding(str, "&gt;");
				}
				else if (x == '"')
				{
					asl_string_append_no_encoding(str, "&quot;");
				}
				else if (x == '\'')
				{
					asl_string_append_no_encoding(str, "&apos;");
				}
				else if (iscntrl(x))
				{
					char tmp[8];
					snprintf(tmp, sizeof(tmp), "&#x%02hhx;", x);
					asl_string_append_no_encoding(str, tmp);
				}
				else
				{
					asl_string_append_char_no_encoding(str, x);
				}
			}
		}
		default:
		{
			return str;
		}
	}

	return str;
}

asl_string_t *
asl_string_append(asl_string_t *str, const char *app)
{
	return asl_string_append_internal(str, app, 0);
}

asl_string_t *
asl_string_append_asl_key(asl_string_t *str, const char *app)
{
	return asl_string_append_internal(str, app, 1);
}

asl_string_t *
asl_string_append_op(asl_string_t *str, uint32_t op)
{
	char opstr[8];
	uint32_t i;

	if (str == NULL) return NULL;

	if (op == ASL_QUERY_OP_NULL)
	{
		return asl_string_append_char_no_encoding(str, '.');
	}

	i = 0;
	if (op & ASL_QUERY_OP_CASEFOLD) opstr[i++] = 'C';

	if (op & ASL_QUERY_OP_REGEX) opstr[i++] = 'R';

	if (op & ASL_QUERY_OP_NUMERIC) opstr[i++] = 'N';

	if (op & ASL_QUERY_OP_PREFIX)
	{
		if (op & ASL_QUERY_OP_SUFFIX) opstr[i++] = 'S';
		else opstr[i++] = 'A';
	}
	if (op & ASL_QUERY_OP_SUFFIX) opstr[i++] = 'Z';

	switch (op & ASL_QUERY_OP_TRUE)
	{
		case ASL_QUERY_OP_EQUAL:
			opstr[i++] = '=';
			break;
		case ASL_QUERY_OP_GREATER:
			opstr[i++] = '>';
			break;
		case ASL_QUERY_OP_GREATER_EQUAL:
			opstr[i++] = '>';
			opstr[i++] = '=';
			break;
		case ASL_QUERY_OP_LESS:
			opstr[i++] = '<';
			break;
		case ASL_QUERY_OP_LESS_EQUAL:
			opstr[i++] = '<';
			opstr[i++] = '=';
			break;
		case ASL_QUERY_OP_NOT_EQUAL:
			opstr[i++] = '!';
			break;
		case ASL_QUERY_OP_TRUE:
			opstr[i++] = 'T';
			break;
		default:
			break;
	}

	if (i == 0)
	{
		return asl_string_append_char_no_encoding(str, '.');
	}

	opstr[i] = '\0';
	return asl_string_append_no_encoding(str, opstr);
}

asl_string_t *
asl_string_append_xml_tag(asl_string_t *str, const char *tag, const char *s)
{
	asl_string_append_no_encoding(str, "\t\t<");
	asl_string_append_no_encoding(str, tag);
	asl_string_append_no_encoding(str, ">");
	asl_string_append_internal(str, s, 0);
	asl_string_append_no_encoding(str, "</");
	asl_string_append_no_encoding(str, tag);
	asl_string_append_no_encoding(str, ">\n");
	return str;
}

