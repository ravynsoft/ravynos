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

#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include <avahi-common/malloc.h>

#include "dns_sd.h"
#include "warn.h"

typedef struct TXTRecordInternal {
    uint8_t *buffer, *malloc_buffer;
    size_t size, max_size;
} TXTRecordInternal;

#define INTERNAL_PTR(txtref) (* (TXTRecordInternal**) (txtref))
#define INTERNAL_PTR_CONST(txtref) (* (const TXTRecordInternal* const *) (txtref))

void DNSSD_API TXTRecordCreate(
    TXTRecordRef *txtref,
    uint16_t length,
    void *buffer) {

    TXTRecordInternal *t;

    AVAHI_WARN_LINKAGE;

    assert(txtref);

    /* Apple's API design is flawed in so many ways, including the
     * fact that it isn't compatible with 64 bit processors. To work
     * around this we need some magic here which involves allocating
     * our own memory. Please, Apple, do your homework next time
     * before designing an API! */

    if ((t = avahi_new(TXTRecordInternal, 1))) {
        t->buffer = buffer;
        t->max_size = buffer ? length : (size_t)0;
        t->size = 0;
        t->malloc_buffer = NULL;
    }

    /* If we were unable to allocate memory, we store a NULL pointer
     * and return a NoMemory error later, is somewhat unclean, but
     * should work. */
    INTERNAL_PTR(txtref) = t;
}

void DNSSD_API TXTRecordDeallocate(TXTRecordRef *txtref) {
    TXTRecordInternal *t;

    AVAHI_WARN_LINKAGE;

    assert(txtref);
    t = INTERNAL_PTR(txtref);
    if (!t)
        return;

    avahi_free(t->malloc_buffer);
    avahi_free(t);

    /* Just in case ... */
    INTERNAL_PTR(txtref) = NULL;
}

static int make_sure_fits_in(TXTRecordInternal *t, size_t size) {
    uint8_t *n;
    size_t nsize;

    assert(t);

    if (t->size + size <= t->max_size)
        return 0;

    nsize = t->size + size + 100;

    if (nsize > 0xFFFF)
        return -1;

    if (!(n = avahi_realloc(t->malloc_buffer, nsize)))
        return -1;

    if (!t->malloc_buffer && t->size)
        memcpy(n, t->buffer, t->size);

    t->buffer = t->malloc_buffer = n;
    t->max_size = nsize;

    return 0;
}

static int remove_key(TXTRecordInternal *t, const char *key) {
    size_t i;
    uint8_t *p;
    size_t key_len;
    int found = 0;

    key_len = strlen(key);
    assert(key_len <= 0xFF);

    p = t->buffer;
    i = 0;

    while (i < t->size) {

        /* Does the item fit in? */
        assert(*p <= t->size - i - 1);

        /* Key longer than buffer */
        if (key_len > t->size - i - 1)
            break;

        if (key_len <= *p &&
            strncmp(key, (char*) p+1, key_len) == 0 &&
            (key_len == *p || p[1+key_len] == '=')) {

            uint8_t s;

            /* Key matches, so let's remove it */

            s = *p;
            memmove(p, p + 1 + *p, t->size - i - *p -1);
            t->size -= s + 1;

            found = 1;
        } else {
            /* Skip to next */

            i += *p +1;
            p += *p +1;
        }
    }

    return found;
}

DNSServiceErrorType DNSSD_API TXTRecordSetValue(
    TXTRecordRef *txtref,
    const char *key,
    uint8_t length,
    const void *value) {

    TXTRecordInternal *t;
    uint8_t *p;
    size_t l, n;

    AVAHI_WARN_LINKAGE;

    assert(key);
    assert(txtref);

    l = strlen(key);

    if (*key == 0 || strchr(key, '=') || l > 0xFF) /* Empty or invalid key */
        return kDNSServiceErr_Invalid;

    if (!(t = INTERNAL_PTR(txtref)))
        return kDNSServiceErr_NoMemory;

    n = l + (value ? length + 1 : 0);

    if (n > 0xFF)
        return kDNSServiceErr_Invalid;

    if (make_sure_fits_in(t, 1 + n) < 0)
        return kDNSServiceErr_NoMemory;

    remove_key(t, key);

    p = t->buffer + t->size;

    *(p++) = (uint8_t) n;
    t->size ++;

    memcpy(p, key, l);
    p += l;
    t->size += l;

    if (value) {
        *(p++) = '=';
        memcpy(p, value, length);
        t->size += length + 1;
    }

    assert(t->size <= t->max_size);

    return kDNSServiceErr_NoError;
}

DNSServiceErrorType DNSSD_API TXTRecordRemoveValue(TXTRecordRef *txtref, const char *key) {
    TXTRecordInternal *t;
    int found;

    AVAHI_WARN_LINKAGE;

    assert(key);
    assert(txtref);

    if (*key == 0 || strchr(key, '=') || strlen(key) > 0xFF) /* Empty or invalid key */
        return kDNSServiceErr_Invalid;

    if (!(t = INTERNAL_PTR(txtref)))
        return kDNSServiceErr_NoError;

    found = remove_key(t, key);

    return found ? kDNSServiceErr_NoError : kDNSServiceErr_NoSuchKey;
}

uint16_t DNSSD_API TXTRecordGetLength(const TXTRecordRef *txtref) {
    const TXTRecordInternal *t;

    AVAHI_WARN_LINKAGE;

    assert(txtref);

    if (!(t = INTERNAL_PTR_CONST(txtref)))
        return 0;

    assert(t->size <= 0xFFFF);
    return (uint16_t) t->size;
}

const void * DNSSD_API TXTRecordGetBytesPtr(const TXTRecordRef *txtref) {
    const TXTRecordInternal *t;

    AVAHI_WARN_LINKAGE;

    assert(txtref);

    if (!(t = INTERNAL_PTR_CONST(txtref)) || !t->buffer)
        return "";

    return t->buffer;
}

static const uint8_t *find_key(const uint8_t *buffer, size_t size, const char *key) {
    size_t i;
    const uint8_t *p;
    size_t key_len;

    key_len = strlen(key);

    assert(key_len <= 0xFF);

    p = buffer;
    i = 0;

    while (i < size) {

        /* Does the item fit in? */
        if (*p > size - i - 1)
            return NULL;

        /* Key longer than buffer */
        if (key_len > size - i - 1)
            return NULL;

        if (key_len <= *p &&
            strncmp(key, (const char*) p+1, key_len) == 0 &&
            (key_len == *p || p[1+key_len] == '=')) {

            /* Key matches, so let's return it */

            return p;
        }

        /* Skip to next */
        i += *p +1;
        p += *p +1;
    }

    return NULL;
}

int DNSSD_API TXTRecordContainsKey (
    uint16_t size,
    const void *buffer,
    const char *key) {

    AVAHI_WARN_LINKAGE;

    assert(key);

    if (!size)
        return 0;

    assert(buffer);

    if (!(find_key(buffer, size, key)))
        return 0;

    return 1;
}

const void * DNSSD_API TXTRecordGetValuePtr(
    uint16_t size,
    const void *buffer,
    const char *key,
    uint8_t *value_len) {

    const uint8_t *p;
    size_t n, l;

    AVAHI_WARN_LINKAGE;

    assert(key);

    if (!size)
        goto fail;

    if (*key == 0 || strchr(key, '=') || strlen(key) > 0xFF) /* Empty or invalid key */
        return NULL;

    assert(buffer);

    if (!(p = find_key(buffer, size, key)))
        goto fail;

    n = *p;
    l = strlen(key);

    assert(n >= l);
    p += 1 + l;
    n -= l;

    if (n <= 0)
        goto fail;

    assert(*p == '=');
    p++;
    n--;

    if (value_len)
        *value_len = n;

    return p;

fail:
    if (value_len)
        *value_len = 0;

    return NULL;
}


uint16_t DNSSD_API TXTRecordGetCount(
    uint16_t size,
    const void *buffer) {

    const uint8_t *p;
    unsigned n = 0;
    size_t i;

    AVAHI_WARN_LINKAGE;

    if (!size)
        return 0;

    assert(buffer);

    p = buffer;
    i = 0;

    while (i < size) {

        /* Does the item fit in? */
        if (*p > size - i - 1)
            break;

        n++;

        /* Skip to next */
        i += *p +1;
        p += *p +1;
    }

    assert(n <= 0xFFFF);

    return (uint16_t) n;
}

DNSServiceErrorType DNSSD_API TXTRecordGetItemAtIndex(
    uint16_t size,
    const void *buffer,
    uint16_t idx,
    uint16_t key_len,
    char *key,
    uint8_t *value_len,
    const void **value) {

    const uint8_t *p;
    size_t i;
    unsigned n = 0;
    DNSServiceErrorType ret = kDNSServiceErr_Invalid;

    AVAHI_WARN_LINKAGE;

    if (!size)
        goto fail;

    assert(buffer);

    p = buffer;
    i = 0;

    while (i < size) {

        /* Does the item fit in? */
        if (*p > size - i - 1)
            goto fail;

        if (n >= idx) {
            size_t l;
            const uint8_t *d;

            d = memchr(p+1, '=', *p);

            /* Length of key */
            l = d ? d - p - 1 : *p;

            if (key_len < l+1) {
                ret = kDNSServiceErr_NoMemory;
                goto fail;
            }

            strncpy(key, (const char*) p + 1, l);
            key[l] = 0;

            if (d) {
                if (value_len)
                    *value_len = *p - l - 1;

                if (value)
                    *value = d + 1;
            } else {

                if (value_len)
                    *value_len  = 0;

                if (value)
                    *value = NULL;
            }

            return kDNSServiceErr_NoError;
        }

        n++;

        /* Skip to next */
        i += *p +1;
        p += *p +1;
    }


fail:

    if (value)
        *value = NULL;

    if (value_len)
        *value_len = 0;

    return ret;

}
