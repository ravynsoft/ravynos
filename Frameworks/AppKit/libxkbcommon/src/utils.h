/*
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
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

#ifndef UTILS_H
#define UTILS_H 1

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#else
/* Required on Windows where unistd.h doesn't exist */
# define R_OK    4               /* Test for read permission.  */
# define W_OK    2               /* Test for write permission.  */
# define X_OK    1               /* Test for execute permission.  */
# define F_OK    0               /* Test for existence.  */
#endif

#ifdef _WIN32
# include <direct.h>
# include <io.h>
# ifndef S_ISDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# endif
#endif

#include "darray.h"

#define STATIC_ASSERT(expr, message) do { \
    switch (0) { case 0: case (expr): ; } \
} while (0)

#define ARRAY_SIZE(arr) ((sizeof(arr) / sizeof(*(arr))))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* Round up @a so it's divisible by @b. */
#define ROUNDUP(a, b) (((a) + (b) - 1) / (b) * (b))

#define STRINGIFY(x) #x
#define STRINGIFY2(x) STRINGIFY(x)

/* Check if a character is valid in a string literal */
static inline bool
is_valid_char(char c)
{
    /* Currently we only check for NULL character, but this could be extended
     * in the future to further ASCII control characters. */
    return c != 0;
}

char
to_lower(char c);

int
istrcmp(const char *a, const char *b);

int
istrncmp(const char *a, const char *b, size_t n);

static inline bool
streq(const char *s1, const char *s2)
{
    assert(s1 && s2);
    return strcmp(s1, s2) == 0;
}

static inline bool
streq_null(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL)
        return s1 == s2;
    return streq(s1, s2);
}

static inline bool
streq_not_null(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return false;
    return streq(s1, s2);
}

static inline bool
istreq(const char *s1, const char *s2)
{
    return istrcmp(s1, s2) == 0;
}

static inline bool
istreq_prefix(const char *s1, const char *s2)
{
    return istrncmp(s1, s2, strlen(s1)) == 0;
}

static inline char *
strdup_safe(const char *s)
{
    return s ? strdup(s) : NULL;
}

static inline size_t
strlen_safe(const char *s)
{
    return s ? strlen(s) : 0;
}

static inline bool
isempty(const char *s)
{
    return s == NULL || s[0] == '\0';
}

static inline const char *
strnull(const char *s)
{
    return s ? s : "(null)";
}

static inline const char *
strempty(const char *s)
{
    return s ? s : "";
}

static inline void *
memdup(const void *mem, size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if (p)
        memcpy(p, mem, nmemb * size);
    return p;
}

#if !(defined(HAVE_STRNDUP) && HAVE_STRNDUP)
static inline char *
strndup(const char *s, size_t n)
{
    size_t slen = strlen(s);
    size_t len = MIN(slen, n);
    char *p = malloc(len + 1);
    if (!p)
        return NULL;
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}
#endif

/* ctype.h is locale-dependent and has other oddities. */
static inline bool
is_space(char ch)
{
    return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

static inline bool
is_alpha(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static inline bool
is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static inline bool
is_alnum(char ch)
{
    return is_alpha(ch) || is_digit(ch);
}

static inline bool
is_xdigit(char ch)
{
    return
        (ch >= '0' && ch <= '9') ||
        (ch >= 'a' && ch <= 'f') ||
        (ch >= 'A' && ch <= 'F');
}

static inline bool
is_graph(char ch)
{
    /* See table in ascii(7). */
    return ch >= '!' && ch <= '~';
}

/*
 * Return the bit position of the most significant bit.
 * Note: this is 1-based! It's more useful this way, and returns 0 when
 * mask is all 0s.
 */
static inline unsigned
msb_pos(uint32_t mask)
{
    unsigned pos = 0;
    while (mask) {
        pos++;
        mask >>= 1u;
    }
    return pos;
}

static inline int
one_bit_set(uint32_t x)
{
    return x && (x & (x - 1)) == 0;
}

bool
map_file(FILE *file, char **string_out, size_t *size_out);

void
unmap_file(char *string, size_t size);

static inline bool
check_eaccess(const char *path, int mode)
{
#if defined(HAVE_EACCESS)
    if (eaccess(path, mode) != 0)
        return false;
#elif defined(HAVE_EUIDACCESS)
    if (euidaccess(path, mode) != 0)
        return false;
#endif

    return true;
}

#if defined(HAVE_SECURE_GETENV)
# define secure_getenv secure_getenv
#elif defined(HAVE___SECURE_GETENV)
# define secure_getenv __secure_getenv
#else
# define secure_getenv getenv
#endif

#if defined(HAVE___BUILTIN_EXPECT)
# define likely(x)   __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#else
# define likely(x)   (x)
# define unlikely(x) (x)
#endif

/* Compiler Attributes */

#if defined(__GNUC__) && (__GNUC__ >= 4) && !defined(__CYGWIN__)
# define XKB_EXPORT      __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define XKB_EXPORT      __global
#else /* not gcc >= 4 and not Sun Studio >= 8 */
# define XKB_EXPORT
#endif

#if defined(__MINGW32__)
# define ATTR_PRINTF(x,y) __attribute__((__format__(__MINGW_PRINTF_FORMAT, x, y)))
#elif defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 203)
# define ATTR_PRINTF(x,y) __attribute__((__format__(__printf__, x, y)))
#else /* not gcc >= 2.3 */
# define ATTR_PRINTF(x,y)
#endif

#if (defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 205)) \
    || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
# define ATTR_NORETURN __attribute__((__noreturn__))
#else
# define ATTR_NORETURN
#endif /* GNUC  */

#if (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 296)
#define ATTR_MALLOC  __attribute__((__malloc__))
#else
#define ATTR_MALLOC
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4)
# define ATTR_NULL_SENTINEL __attribute__((__sentinel__))
#else
# define ATTR_NULL_SENTINEL
#endif /* GNUC >= 4 */

#if (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 295)
#define ATTR_PACKED  __attribute__((__packed__))
#else
#define ATTR_PACKED
#endif

#if !(defined(HAVE_ASPRINTF) && HAVE_ASPRINTF)
int asprintf(char **strp, const char *fmt, ...) ATTR_PRINTF(2, 3);
# if !(defined(HAVE_VASPRINTF) && HAVE_VASPRINTF)
#  include <stdarg.h>
int vasprintf(char **strp, const char *fmt, va_list ap);
# endif /* !HAVE_VASPRINTF */
#endif /* !HAVE_ASPRINTF */

static inline bool
ATTR_PRINTF(3, 4)
snprintf_safe(char *buf, size_t sz, const char *format, ...)
{
    va_list ap;
    int rc;

    va_start(ap, format);
    rc = vsnprintf(buf, sz, format, ap);
    va_end(ap);

    return rc >= 0 && (size_t)rc < sz;
}

static inline char *
ATTR_PRINTF(1, 0)
vasprintf_safe(const char *fmt, va_list args)
{
    char *str;
    int len;

    len = vasprintf(&str, fmt, args);

    if (len == -1)
        return NULL;

    return str;
}

/**
 * A version of asprintf that returns the allocated string or NULL on error.
 */
static inline char *
ATTR_PRINTF(1, 2)
asprintf_safe(const char *fmt, ...)
{
    va_list args;
    char *str;

    va_start(args, fmt);
    str = vasprintf_safe(fmt, args);
    va_end(args);

    return str;
}

#endif /* UTILS_H */
