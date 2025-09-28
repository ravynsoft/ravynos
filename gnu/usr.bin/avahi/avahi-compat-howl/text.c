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

#include <assert.h>

#include <avahi-common/strlst.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>

#include "howl.h"
#include "warn.h"

struct _sw_text_record {
    AvahiStringList *strlst;
    uint8_t *buffer;
    size_t buffer_size;
    int buffer_valid;
};

#ifndef HAVE_STRLCPY

static size_t strlcpy(char *dest, const char *src, size_t n) {
    assert(dest);
    assert(src);

    if (n > 0) {
        strncpy(dest, src, n-1);
        dest[n-1] = 0;
    }

    return strlen(src);
}

#endif

sw_result sw_text_record_init(sw_text_record *self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    if (!(*self = avahi_new(struct _sw_text_record, 1))) {
        *self = NULL;
        return SW_E_UNKNOWN;
    }

    (*self)->strlst = NULL;
    (*self)->buffer = NULL;
    (*self)->buffer_size = 0;
    (*self)->buffer_valid = 0;

    return SW_OKAY;
}

sw_result sw_text_record_fina(sw_text_record self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    avahi_string_list_free(self->strlst);
    avahi_free(self->buffer);
    avahi_free(self);
    return SW_OKAY;
}

sw_result sw_text_record_add_string(
    sw_text_record self,
    sw_const_string string) {

    AvahiStringList *n;

    assert(self);
    assert(string);

    AVAHI_WARN_LINKAGE;

    if (!(n = avahi_string_list_add(self->strlst, string)))
        return SW_E_UNKNOWN;

    self->strlst = n;
    self->buffer_valid = 0;
    return SW_OKAY;
}

sw_result sw_text_record_add_key_and_string_value(
    sw_text_record self,
    sw_const_string key,
    sw_const_string val) {

    AvahiStringList *n;

    assert(self);
    assert(key);

    AVAHI_WARN_LINKAGE;

    if (!(n = avahi_string_list_add_pair(self->strlst, key, val)))
        return SW_E_UNKNOWN;

    self->strlst = n;
    self->buffer_valid = 0;
    return SW_OKAY;
}

sw_result sw_text_record_add_key_and_binary_value(
    sw_text_record self,
    sw_const_string key,
    sw_octets val,
    sw_uint32 len) {

    AvahiStringList *n;

    assert(self);
    assert(key);
    assert(len || !val);

    AVAHI_WARN_LINKAGE;

    if (!(n = avahi_string_list_add_pair_arbitrary(self->strlst, key, val, len)))
        return SW_E_UNKNOWN;

    self->strlst = n;
    self->buffer_valid = 0;
    return SW_OKAY;
}

static int rebuild(sw_text_record self) {
    assert(self);

    if (self->buffer_valid)
        return 0;

    self->buffer_size = avahi_string_list_serialize(self->strlst, NULL, 0);

    if (!(self->buffer = avahi_realloc(self->buffer, self->buffer_size + 1)))
        return -1;

    avahi_string_list_serialize(self->strlst, self->buffer, self->buffer_size);
    self->buffer_valid = 1;

    return 0;
}

sw_octets sw_text_record_bytes(sw_text_record self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    if (rebuild(self) < 0)
        return NULL;

    return self->buffer;
}

sw_uint32 sw_text_record_len(sw_text_record self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    if (rebuild(self) < 0)
        return (uint32_t) -1;

    return self->buffer_size;
}

struct _sw_text_record_iterator {
    AvahiStringList *strlst, *index;

};

sw_result sw_text_record_iterator_init(
    sw_text_record_iterator * self,
    sw_octets text_record,
    sw_uint32 text_record_len) {

    AvahiStringList *txt;
    assert(self);

    AVAHI_WARN_LINKAGE;

    if (!(*self = avahi_new(struct _sw_text_record_iterator, 1))) {
        *self = NULL;
        return SW_E_UNKNOWN;
    }

    if (avahi_string_list_parse(text_record, text_record_len, &txt) < 0) {
        avahi_free(*self);
        *self = NULL;
        return SW_E_UNKNOWN;
    }

    (*self)->index = (*self)->strlst = avahi_string_list_reverse(txt);

    return SW_OKAY;
}

sw_result sw_text_record_iterator_fina(sw_text_record_iterator self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    avahi_string_list_free(self->strlst);
    avahi_free(self);

    return SW_OKAY;
}

sw_result sw_text_record_iterator_next(
    sw_text_record_iterator self,
    char key[SW_TEXT_RECORD_MAX_LEN],
    sw_uint8 val[SW_TEXT_RECORD_MAX_LEN],
    sw_uint32 * val_len) {

    char *mkey = NULL, *mvalue = NULL;
    size_t msize = 0;

    assert(self);
    assert(key);

    AVAHI_WARN_LINKAGE;

    if (!self->index)
        return SW_E_UNKNOWN;

    if (avahi_string_list_get_pair(self->index, &mkey, &mvalue, &msize) < 0)
        return SW_E_UNKNOWN;

    strlcpy(key, mkey, SW_TEXT_RECORD_MAX_LEN);
    memset(val, 0, SW_TEXT_RECORD_MAX_LEN);
    memcpy(val, mvalue, msize);
    *val_len = msize;

    avahi_free(mkey);
    avahi_free(mvalue);

    self->index = self->index->next;

    return SW_OKAY;
}

