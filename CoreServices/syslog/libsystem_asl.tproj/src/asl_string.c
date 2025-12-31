/*
 * Copyright (c) 2007-2015 Apple Inc.  All rights reserved.
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
#include <mach/vm_param.h>
#include <libkern/OSAtomic.h>
#include <asl_string.h>
#include <asl_private.h>

#define ASL_STRING_QUANTUM 256
static const char *cvis_7_13 = "abtnvfr";

/* Forward */
asl_string_t *asl_string_append_no_encoding_len(asl_string_t *str, const char *app, size_t copylen);

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

	if (encoding & ASL_STRING_LEN) asl_string_append_no_encoding_len(str, "         0 ", 11);
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

	if (str->encoding & ASL_STRING_LEN)
	{
		char tmp[11];
		snprintf(tmp, sizeof(tmp), "%10lu", str->cursor - 10);
		memcpy(str->buf, tmp, 10);
	}

	if (OSAtomicDecrement32Barrier(&(str->refcount)) != 0)
	{
		/* string is still retained - copy buf */
		if (str->encoding & ASL_STRING_VM)
		{
			if (str->bufsize == 0) return NULL;

			vm_address_t new = 0;
			kern_return_t kstatus = vm_allocate(mach_task_self(), &new, str->bufsize, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ASL));
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

	if (str->encoding & ASL_STRING_LEN)
	{
		char tmp[11];
		snprintf(tmp, sizeof(tmp), "%10lu", str->cursor - 10);
		memcpy(str->buf, tmp, 10);
	}

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

		kstatus = vm_allocate(mach_task_self(), &new, newlen, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ASL));
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

	return str;
}

asl_string_t *
asl_string_append_no_encoding_len(asl_string_t *str, const char *app, size_t copylen)
{
	size_t len, applen;

	if (str == NULL) return NULL;
	if (app == NULL) return str;

	applen = copylen;
	if (applen == 0) applen = strlen(app);

	len = applen;
	if (str->bufsize == 0) len++;

	if (_asl_string_grow(str, len) < 0) return str;

	memcpy(str->buf + str->cursor, app, applen);

	str->cursor += applen;
	str->buf[str->cursor] = '\0';

	return str;
}

asl_string_t *
asl_string_append_no_encoding(asl_string_t *str, const char *app)
{
	return asl_string_append_no_encoding_len(str, app, 0);
}

static asl_string_t *
asl_string_append_internal(asl_string_t *str, const char *app, int encode_space)
{
	uint8_t x, y, z;
	const uint8_t *s, *p;
	size_t copylen;

	if (str == NULL) return NULL;
	if (app == NULL) return str;

	switch (str->encoding & ASL_ENCODE_MASK)
	{
		case ASL_ENCODE_NONE:
		{
			return asl_string_append_no_encoding_len(str, app, 0);
		}
		case ASL_ENCODE_SAFE:
		{
			/* minor encoding to reduce the likelyhood of spoof attacks */
			const char *p;

			s = NULL;
			copylen = 0;

			for (p = app; *p != '\0'; p++)
			{
				x = p[0];
				y = 0;
				z = 0;
				
				if (x != 0) y = p[1];
				if (y != 0) z = p[2];
				
				if ((x == 10) || (x == 13))
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "\n\t", 2);
				}
				else if (x == 8)
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "^H", 2);
				}
				else if ((x == 0xc2) && (y == 0x85))
				{
					p++;
					
					/* next line - format like newline */
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}
					
					asl_string_append_no_encoding_len(str, "\n\t", 2);
				}
				else if ((x == 0xe2) && (y == 0x80) && ((z == 0xa8) || (z == 0xa9)))
				{
					p += 3;
					
					/* line separator or paragraph separator - format like newline */
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}
					
					asl_string_append_no_encoding_len(str, "\n\t", 2);
				}
				else
				{
					if (s == NULL) s = p;
					copylen++;
				}
			}

			if (copylen > 0) asl_string_append_no_encoding_len(str, s, copylen);

			return str;
		}
		case ASL_ENCODE_ASL:
		{
			s = NULL;
			copylen = 0;

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
						if (copylen > 0)
						{
							asl_string_append_no_encoding_len(str, s, copylen);
							s = NULL;
							copylen = 0;
						}

						asl_string_append_no_encoding_len(str, "\\240", 4);
						continue;
					}

					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "\\M", 2);
					x &= 0x7f;
					meta = 1;
				}

				/* space is either ' ' or \s */
				if (x == 32)
				{
					if (encode_space == 0)
					{
						if (s == NULL) s = p;
						copylen++;
						continue;
					}

					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "\\s", 2);
					continue;
				}

				/* \ is escaped */
				if ((meta == 0) && (x == 92))
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "\\\\", 2);
					continue;
				}

				/* [ and ] are escaped in ASL encoding */
				if ((str->encoding & ASL_ENCODE_ASL) && (meta == 0) && ((*p == 91) || (*p == 93)))
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					if (*p == '[') asl_string_append_no_encoding_len(str, "\\[", 2);
					else asl_string_append_no_encoding_len(str, "\\]", 2);
					continue;
				}

				/* DEL is \^? */
				if (x == 127)
				{
					if (meta == 0)
					{
						if (copylen > 0)
						{
							asl_string_append_no_encoding_len(str, s, copylen);
							s = NULL;
							copylen = 0;
						}

						asl_string_append_char_no_encoding(str, '\\');
					}

					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "^?", 2);
					continue;
				}

				/* 33-126 are printable (add a '-' prefix for meta) */
				if ((x >= 33) && (x <= 126))
				{
					if (meta == 1)
					{
						if (copylen > 0)
						{
							asl_string_append_no_encoding_len(str, s, copylen);
							s = NULL;
							copylen = 0;
						}

						asl_string_append_char_no_encoding(str, '-');
						asl_string_append_char_no_encoding(str, x);
						continue;
					}

					if (s == NULL) s = p;
					copylen++;

					continue;
				}

				/* non-meta BEL, BS, HT, NL, VT, NP, CR (7-13) are \a, \b, \t, \n, \v, \f, and \r */
				if ((meta == 0) && (x >= 7) && (x <= 13))
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_char_no_encoding(str, '\\');
					asl_string_append_char_no_encoding(str, cvis_7_13[x - 7]);
					continue;
				}

				/* 0 - 31 are ^@ - ^_ (non-meta get a leading \) */
				if (x <= 31)
				{
					if (meta == 0)
					{
						if (copylen > 0)
						{
							asl_string_append_no_encoding_len(str, s, copylen);
							s = NULL;
							copylen = 0;
						}

						asl_string_append_char_no_encoding(str, '\\');
					}

					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_char_no_encoding(str, '^');
					asl_string_append_char_no_encoding(str, 64 + x);
					continue;
				}

				if (s == NULL) s = p;
				copylen++;
			}

			if (copylen > 0)
			{
				asl_string_append_no_encoding_len(str, s, copylen);
				s = NULL;
				copylen = 0;
			}

			return str;
		}
		case ASL_ENCODE_XML:
		{
			s = NULL;
			copylen = 0;

			for (p = app; *p != '\0'; p++)
			{
				x = *p;

				if (x == '&')
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "&amp;", 5);
				}
				else if (x == '<')
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "&lt;", 4);
				}
				else if (x == '>')
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "&gt;", 4);
				}
				else if (x == '"')
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "&quot;", 6);
				}
				else if (x == '\'')
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					asl_string_append_no_encoding_len(str, "&apos;", 6);
				}
				else if (iscntrl(x))
				{
					if (copylen > 0)
					{
						asl_string_append_no_encoding_len(str, s, copylen);
						s = NULL;
						copylen = 0;
					}

					char tmp[8];
					snprintf(tmp, sizeof(tmp), "&#x%02hhx;", x);
					asl_string_append_no_encoding_len(str, tmp, 6);
				}
				else
				{
					if (s == NULL) s = p;
					copylen++;
				}
			}

			if (copylen > 0)
			{
				asl_string_append_no_encoding_len(str, s, copylen);
				s = NULL;
				copylen = 0;
			}

			return str;
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
	return asl_string_append_no_encoding_len(str, opstr, 0);
}

asl_string_t *
asl_string_append_xml_tag(asl_string_t *str, const char *tag, const char *s)
{
	asl_string_append_no_encoding_len(str, "\t\t<", 3);
	asl_string_append_no_encoding_len(str, tag, 0);
	asl_string_append_char_no_encoding(str, '>');
	asl_string_append_internal(str, s, 0);
	asl_string_append_no_encoding_len(str, "</", 2);
	asl_string_append_no_encoding_len(str, tag, 0);
	asl_string_append_no_encoding_len(str, ">\n", 2);
	return str;
}

