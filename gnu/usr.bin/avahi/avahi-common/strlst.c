/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "strlst.h"
#include "malloc.h"
#include "defs.h"

AvahiStringList*avahi_string_list_add_anonymous(AvahiStringList *l, size_t size) {
    AvahiStringList *n;

    if (!(n = avahi_malloc(sizeof(AvahiStringList) + size)))
        return NULL;

    n->next = l;
    n->size = size;

    /* NUL terminate strings, just to make sure */
    n->text[size] = 0;

    return n;
}

AvahiStringList *avahi_string_list_add_arbitrary(AvahiStringList *l, const uint8_t*text, size_t size) {
    AvahiStringList *n;

    assert(size == 0 || text);

    if (!(n = avahi_string_list_add_anonymous(l, size)))
        return NULL;

    if (size > 0)
        memcpy(n->text, text, size);

    return n;
}

AvahiStringList *avahi_string_list_add(AvahiStringList *l, const char *text) {
    assert(text);

    return avahi_string_list_add_arbitrary(l, (const uint8_t*) text, strlen(text));
}

int avahi_string_list_parse(const void* data, size_t size, AvahiStringList **ret) {
    const uint8_t *c;
    AvahiStringList *r = NULL;

    assert(data);
    assert(ret);

    c = data;
    while (size > 0) {
        size_t k;

        k = *(c++);
        size--;

        if (k > size)
            goto fail; /* Overflow */

        if (k > 0) { /* Ignore empty strings */
            AvahiStringList *n;

            if (!(n = avahi_string_list_add_arbitrary(r, c, k)))
                goto fail; /* OOM */

            r = n;
        }

        c += k;
        size -= k;
    }

    *ret = r;

    return 0;

fail:
    avahi_string_list_free(r);
    return -1;
}

void avahi_string_list_free(AvahiStringList *l) {
    AvahiStringList *n;

    while (l) {
        n = l->next;
        avahi_free(l);
        l = n;
    }
}

AvahiStringList* avahi_string_list_reverse(AvahiStringList *l) {
    AvahiStringList *r = NULL, *n;

    while (l) {
        n = l->next;
        l->next = r;
        r = l;
        l = n;
    }

    return r;
}

/**
 * This routine is used for both human- and machine-readable output of
 * TXT records. As such it must cope with escaping, in order to allow
 * machines to reconstruct the original data.
 *
 * AFAIK no RFC specifies syntax for TXT data other than raw binary,
 * though presumably zonefile syntax would make sense:
 *
 *   - RFC 1035 says that TXT records contain `<character-string>`s, and section
 *     5 says:
 *
 *       <character-string> is expressed in one or two ways: as a contiguous set
 *       of characters without interior spaces, or as a string beginning with a "
 *       and ending with a ".  Inside a " delimited string any character can
 *       occur, except for a " itself, which must be quoted using \ (back slash).
 *
 *     This omits escaping of backslashes (!).
 *
 *   - RFC 1034 doesn't say anything relevant.
 *
 *   - RFC 1464 suggests a specific encoding of information within a TXT
 *     record but does not discuss formatting of TXT records in
 *     general.
 *
 * In order to also escape newlines, which interfere with line-by-line
 * machine processing of records, this routine:
 *
 *   - escapes >>> " <<< to >>> \" <<<
 *   - escapes >>> \ <<< to >>> \\ <<<
 *   - escapes bytes less than 32 to backslash-prefixed 3-digit DECIMAL form
 */
char* avahi_string_list_to_string(AvahiStringList *l) {
    AvahiStringList *n;
    size_t s = 0;
    char *p, *t, *e;

    for (n = l; n; n = n->next) {
        if (n != l)
            s ++; /* for the inter-string separating space */

        for (p = (char*) n->text; ((size_t) (p - (char*) n->text) < n->size); p++) {
            switch (*p) {
              case '"':
              case '\\':
                  s += 2;
                  break;
              default:
                  if (*p < 32) {
                      s += 4;
                  } else {
                      s ++;
                      break;
                  }
            }
        }
        s += 2; /* for the leading and trailing double-quotes */
    }

    if (!(t = e = avahi_new(char, s+1))) /* plus one for the trailing NUL */
        return NULL;

    l = avahi_string_list_reverse(l);

    for (n = l; n; n = n->next) {
        if (n != l)
            *(e++) = ' ';

        *(e++) = '"';
        for (p = (char*) n->text; ((size_t) (p - (char*) n->text) < n->size); p++) {
            switch (*p) {
              case '"':
              case '\\':
                  *(e++) = '\\';
                  /* FALL THROUGH */
              default:
                  if (*p < 32) {
                      *(e++) = '\\';
                      *(e++) = '0' + (char)  ((uint8_t) *p / 100);
                      *(e++) = '0' + (char) (((uint8_t) *p / 10) % 10);
                      *(e++) = '0' + (char)  ((uint8_t) *p % 10);
                  } else {
                      *(e++) = *p;
                  }
            }
        }
        *(e++) = '"';

        assert(e);
    }

    l = avahi_string_list_reverse(l);

    *e = 0;

    return t;
}

size_t avahi_string_list_serialize(AvahiStringList *l, void *data, size_t size) {
    size_t used = 0;

    if (data) {
        AvahiStringList *n;
        uint8_t *c;

        l = avahi_string_list_reverse(l);
        c = data;

        for (n = l; size > 1 && n; n = n->next) {
            size_t k;

            if ((k = n->size) == 0)
                /* Skip empty strings */
                continue;

            if (k > 255)
                /* Truncate strings at 255 characters */
                k = 255;

            if (k > size-1)
                /* Make sure this string fits in */
                k = size-1;

            *(c++) = (uint8_t) k;
            memcpy(c, n->text, k);
            c += k;

            used += 1 + k;
            size -= 1 + k;
        }

        l = avahi_string_list_reverse(l);

        if (used == 0 && size > 0) {

            /* Empty lists are treated specially. To comply with
             * section 6.1 of the DNS-SD spec, we return a single
             * empty string (i.e. a NUL byte)*/

            *(uint8_t*) data = 0;
            used = 1;
        }

    } else {
        AvahiStringList *n;

        for (n = l; n; n = n->next) {
            size_t k;

            if ((k = n->size) == 0)
                continue;

            if (k > 255)
                k = 255;

            used += 1+k;
        }

        if (used == 0)
            used = 1;
    }

    return used;
}

int avahi_string_list_equal(const AvahiStringList *a, const AvahiStringList *b) {

    for (;;) {
        if (!a && !b)
            return 1;

        if (!a || !b)
            return 0;

        if (a->size != b->size)
            return 0;

        if (a->size != 0 && memcmp(a->text, b->text, a->size) != 0)
            return 0;

        a = a->next;
        b = b->next;
    }
}

AvahiStringList *avahi_string_list_add_many(AvahiStringList *r, ...) {
    va_list va;

    va_start(va, r);
    r = avahi_string_list_add_many_va(r, va);
    va_end(va);

    return r;
}

AvahiStringList *avahi_string_list_add_many_va(AvahiStringList *r, va_list va) {
    const char *txt;

    while ((txt = va_arg(va, const char*)))
        r = avahi_string_list_add(r, txt);

    return r;
}

AvahiStringList *avahi_string_list_new(const char *txt, ...) {
    va_list va;
    AvahiStringList *r = NULL;

    if (txt) {
        r = avahi_string_list_add(r, txt);

        va_start(va, txt);
        r = avahi_string_list_add_many_va(r, va);
        va_end(va);
    }

    return r;
}

AvahiStringList *avahi_string_list_new_va(va_list va) {
    return avahi_string_list_add_many_va(NULL, va);
}

AvahiStringList *avahi_string_list_copy(const AvahiStringList *l) {
    AvahiStringList *r = NULL;

    for (; l; l = l->next)
        if (!(r = avahi_string_list_add_arbitrary(r, l->text, l->size))) {
            avahi_string_list_free(r);
            return NULL;
        }

    return avahi_string_list_reverse(r);
}

AvahiStringList *avahi_string_list_new_from_array(const char *array[], int length) {
    AvahiStringList *r = NULL;
    int i;

    assert(array);

    for (i = 0; length >= 0 ? i < length : !!array[i]; i++)
        r = avahi_string_list_add(r, array[i]);

    return r;
}

unsigned avahi_string_list_length(const AvahiStringList *l) {
    unsigned n = 0;

    for (; l; l = l->next)
        n++;

    return n;
}

AvahiStringList *avahi_string_list_add_vprintf(AvahiStringList *l, const char *format, va_list va) {
    size_t len = 80;
    AvahiStringList *r;

    assert(format);

    if (!(r = avahi_malloc(sizeof(AvahiStringList) + len)))
        return NULL;

    for (;;) {
        int n;
        AvahiStringList *nr;
        va_list va2;

        va_copy(va2, va);
        n = vsnprintf((char*) r->text, len, format, va2);
        va_end(va2);

        if (n >= 0 && n < (int) len)
            break;

        if (n >= 0)
            len = n+1;
        else
            len *= 2;

        if (!(nr = avahi_realloc(r, sizeof(AvahiStringList) + len))) {
            avahi_free(r);
            return NULL;
        }

        r = nr;
    }

    r->next = l;
    r->size = strlen((char*) r->text);

    return r;
}

AvahiStringList *avahi_string_list_add_printf(AvahiStringList *l, const char *format, ...) {
    va_list va;

    assert(format);

    va_start(va, format);
    l  = avahi_string_list_add_vprintf(l, format, va);
    va_end(va);

    return l;
}

AvahiStringList *avahi_string_list_find(AvahiStringList *l, const char *key) {
    size_t n;

    assert(key);
    n = strlen(key);

    for (; l; l = l->next) {
        if (strcasecmp((char*) l->text, key) == 0)
            return l;

        if (strncasecmp((char*) l->text, key, n) == 0 && l->text[n] == '=')
            return l;
    }

    return NULL;
}

AvahiStringList *avahi_string_list_add_pair(AvahiStringList *l, const char *key, const char *value) {
    assert(key);

    if (value)
        return avahi_string_list_add_printf(l, "%s=%s", key, value);
    else
        return avahi_string_list_add(l, key);
}

AvahiStringList *avahi_string_list_add_pair_arbitrary(AvahiStringList *l, const char *key, const uint8_t *value, size_t size) {
    size_t n;
    assert(key);

    if (!value)
        return avahi_string_list_add(l, key);

    n = strlen(key);

    if (!(l = avahi_string_list_add_anonymous(l, n + 1 + size)))
        return NULL;

    memcpy(l->text, key, n);
    l->text[n] = '=';
    memcpy(l->text + n + 1, value, size);

    return l;
}

int avahi_string_list_get_pair(AvahiStringList *l, char **key, char **value, size_t *size) {
    char *e;

    assert(l);

    if (!(e = memchr(l->text, '=', l->size))) {

        if (key)
            if (!(*key = avahi_strdup((char*) l->text)))
                return -1;

        if (value)
            *value = NULL;

        if (size)
            *size = 0;

    } else {
        size_t n;

        if (key)
            if (!(*key = avahi_strndup((char*) l->text, e - (char *) l->text)))
                return -1;

        e++; /* Advance after '=' */

        n = l->size - (e - (char*) l->text);

        if (value) {

            if (!(*value = avahi_memdup(e, n+1))) {
                if (key)
                    avahi_free(*key);
                return -1;
            }

            (*value)[n] = 0;
        }

        if (size)
            *size = n;
    }

    return 0;
}

AvahiStringList *avahi_string_list_get_next(AvahiStringList *l) {
    assert(l);
    return l->next;
}

uint8_t *avahi_string_list_get_text(AvahiStringList *l) {
    assert(l);
    return l->text;
}

size_t avahi_string_list_get_size(AvahiStringList *l) {
    assert(l);
    return l->size;
}

uint32_t avahi_string_list_get_service_cookie(AvahiStringList *l) {
    AvahiStringList *f;
    char *value = NULL, *end = NULL;
    uint32_t ret;

    if (!(f = avahi_string_list_find(l, AVAHI_SERVICE_COOKIE)))
        return AVAHI_SERVICE_COOKIE_INVALID;

    if (avahi_string_list_get_pair(f, NULL, &value, NULL) < 0 || !value)
        return AVAHI_SERVICE_COOKIE_INVALID;

    ret = (uint32_t) strtoll(value, &end, 0);

    if (*value && end && *end != 0) {
        avahi_free(value);
        return AVAHI_SERVICE_COOKIE_INVALID;
    }

    avahi_free(value);

    return ret;
}
