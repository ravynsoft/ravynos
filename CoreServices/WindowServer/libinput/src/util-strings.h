/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013-2015 Red Hat, Inc.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_XLOCALE_H
#include <xlocale.h>
#endif

#include "util-macros.h"

static inline bool
streq(const char *str1, const char *str2)
{
	/* one NULL, one not NULL is always false */
	if (str1 && str2)
		return strcmp(str1, str2) == 0;
	return str1 == str2;
}

static inline bool
strneq(const char *str1, const char *str2, int n)
{
	/* one NULL, one not NULL is always false */
	if (str1 && str2)
		return strncmp(str1, str2, n) == 0;
	return str1 == str2;
}

static inline void *
zalloc(size_t size)
{
	void *p;

	/* We never need to alloc anything more than 1,5 MB so we can assume
	 * if we ever get above that something's going wrong */
	if (size > 1536 * 1024)
		assert(!"bug: internal malloc size limit exceeded");

	p = calloc(1, size);
	if (!p)
		abort();

	return p;
}

/**
 * strdup guaranteed to succeed. If the input string is NULL, the output
 * string is NULL. If the input string is a string pointer, we strdup or
 * abort on failure.
 */
static inline char*
safe_strdup(const char *str)
{
	char *s;

	if (!str)
		return NULL;

	s = strdup(str);
	if (!s)
		abort();
	return s;
}

/**
 * Simple wrapper for asprintf that ensures the passed in-pointer is set
 * to NULL upon error.
 * The standard asprintf() call does not guarantee the passed in pointer
 * will be NULL'ed upon failure, whereas this wrapper does.
 *
 * @param strp pointer to set to newly allocated string.
 * This pointer should be passed to free() to release when done.
 * @param fmt the format string to use for printing.
 * @return The number of bytes printed (excluding the null byte terminator)
 * upon success or -1 upon failure. In the case of failure the pointer is set
 * to NULL.
 */
__attribute__ ((format (printf, 2, 3)))
static inline int
xasprintf(char **strp, const char *fmt, ...)
{
	int rc = 0;
	va_list args;

	va_start(args, fmt);
	rc = vasprintf(strp, fmt, args);
	va_end(args);
	if ((rc == -1) && strp)
		*strp = NULL;

	return rc;
}

__attribute__ ((format (printf, 2, 0)))
static inline int
xvasprintf(char **strp, const char *fmt, va_list args)
{
	int rc = 0;
	rc = vasprintf(strp, fmt, args);
	if ((rc == -1) && strp)
		*strp = NULL;

	return rc;
}

static inline bool
safe_atoi_base(const char *str, int *val, int base)
{
	assert(str != NULL);

	char *endptr;
	long v;

	assert(base == 10 || base == 16 || base == 8);

	errno = 0;
	v = strtol(str, &endptr, base);
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;

	if (v > INT_MAX || v < INT_MIN)
		return false;

	*val = v;
	return true;
}

static inline bool
safe_atoi(const char *str, int *val)
{
	assert(str != NULL);
	return safe_atoi_base(str, val, 10);
}

static inline bool
safe_atou_base(const char *str, unsigned int *val, int base)
{
	assert(str != NULL);

	char *endptr;
	unsigned long v;

	assert(base == 10 || base == 16 || base == 8);

	errno = 0;
	v = strtoul(str, &endptr, base);
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;

	if ((long)v < 0)
		return false;

	*val = v;
	return true;
}

static inline bool
safe_atou(const char *str, unsigned int *val)
{
	assert(str != NULL);
	return safe_atou_base(str, val, 10);
}

static inline bool
safe_atod(const char *str, double *val)
{
	assert(str != NULL);

	char *endptr;
	double v;
#ifdef HAVE_LOCALE_H
	locale_t c_locale;
#endif
	size_t slen = strlen(str);

	/* We don't have a use-case where we want to accept hex for a double
	 * or any of the other values strtod can parse */
	for (size_t i = 0; i < slen; i++) {
		char c = str[i];

		if (isdigit(c))
		       continue;
		switch(c) {
		case '+':
		case '-':
		case '.':
			break;
		default:
			return false;
		}
	}

#ifdef HAVE_LOCALE_H
	/* Create a "C" locale to force strtod to use '.' as separator */
	c_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
	if (c_locale == (locale_t)0)
		return false;

	errno = 0;
	v = strtod_l(str, &endptr, c_locale);
	freelocale(c_locale);
#else
	/* No locale support in provided libc, assume it already uses '.' */
	errno = 0;
	v = strtod(str, &endptr);
#endif
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;
	if (v != 0.0 && !isnormal(v))
		return false;

	*val = v;
	return true;
}

char **strv_from_argv(int argc, char **argv);
char **strv_from_string(const char *in, const char *separator, size_t *num_elements);
char *strv_join(char **strv, const char *joiner);

static inline void
strv_free(char **strv) {
	char **s = strv;

	if (!strv)
		return;

	while (*s != NULL) {
		free(*s);
		*s = (char*)0x1; /* detect use-after-free */
		s++;
	}

	free (strv);
}

/**
 * parse a string containing a list of doubles into a double array.
 *
 * @param in string to parse
 * @param separator string used to separate double in list e.g. ","
 * @param result double array
 * @param length length of double array
 * @return true when parsed successfully otherwise false
 */
static inline double *
double_array_from_string(const char *in,
			 const char *separator,
			 size_t *length)
{
	assert(in != NULL);
	assert(separator != NULL);
	assert(length != NULL);

	double *result = NULL;
	*length = 0;

	size_t nelem;
	char **strv = strv_from_string(in, separator, &nelem);
	if (!strv)
		return result;

	double *numv = zalloc(sizeof(double) * nelem);
	for (size_t idx = 0; idx < nelem; idx++) {
		double val;
		if (!safe_atod(strv[idx], &val))
			goto out;

		numv[idx] = val;
	}

	result = numv;
	numv = NULL;
	*length = nelem;

out:
	strv_free(strv);
	free(numv);
	return result;
}

struct key_value_str{
	char *key;
	char *value;
};

struct key_value_double {
	double key;
	double value;
};

static inline ssize_t
kv_double_from_string(const char *string,
		      const char *pair_separator,
		      const char *kv_separator,
		      struct key_value_double **result_out)

{
	struct key_value_double *result = NULL;

	if (!pair_separator || pair_separator[0] == '\0' ||
	    !kv_separator || kv_separator[0] == '\0')
		return -1;

	size_t npairs;
	char **pairs = strv_from_string(string, pair_separator, &npairs);
	if (!pairs || npairs == 0)
		goto error;

	result = zalloc(npairs * sizeof *result);

	for (size_t idx = 0; idx < npairs; idx++) {
		char *pair = pairs[idx];
		size_t nelem;
		char **kv = strv_from_string(pair, kv_separator, &nelem);
		double k, v;

		if (!kv || nelem != 2 ||
		    !safe_atod(kv[0], &k) ||
		    !safe_atod(kv[1], &v)) {
			strv_free(kv);
			goto error;
		}

		result[idx].key = k;
		result[idx].value = v;

		strv_free(kv);
	}

	strv_free(pairs);

	*result_out = result;

	return npairs;

error:
	strv_free(pairs);
	free(result);
	return -1;
}

/**
 * Strip any of the characters in what from the beginning and end of the
 * input string.
 *
 * @return a newly allocated string with none of "what" at the beginning or
 * end of string
 */
static inline char *
strstrip(const char *input, const char *what)
{
	assert(input != NULL);

	char *str, *last;

	str = safe_strdup(&input[strspn(input, what)]);

	last = str;

	for (char *c = str; *c != '\0'; c++) {
		if (!strchr(what, *c))
			last = c + 1;
	}

	*last = '\0';

	return str;
}

/**
 * Return true if str ends in suffix, false otherwise. If the suffix is the
 * empty string, strendswith() always returns false.
 */
static inline bool
strendswith(const char *str, const char *suffix)
{
	if (str == NULL)
		return false;

	size_t slen = strlen(str);
	size_t suffixlen = strlen(suffix);
	size_t offset;

	if (slen == 0 || suffixlen == 0 || suffixlen > slen)
		return false;

	offset = slen - suffixlen;
	return strneq(&str[offset], suffix, suffixlen);
}

static inline bool
strstartswith(const char *str, const char *prefix)
{
	if (str == NULL)
		return false;

	size_t prefixlen = strlen(prefix);

	return prefixlen > 0 ? strneq(str, prefix, strlen(prefix)) : false;
}

const char *
safe_basename(const char *filename);

char *
trunkname(const char *filename);

/**
 * Return a copy of str with all % converted to %% to make the string
 * acceptable as printf format.
 */
static inline char *
str_sanitize(const char *str)
{
	if (!str)
		return NULL;

	if (!strchr(str, '%'))
		return strdup(str);

	size_t slen = min(strlen(str), 512);
	char *sanitized = zalloc(2 * slen + 1);
	const char *src = str;
	char *dst = sanitized;

	for (size_t i = 0; i < slen; i++) {
		if (*src == '%')
			*dst++ = '%';
		*dst++ = *src++;
	}
	*dst = '\0';

	return sanitized;
}
