#ifndef foohashmaphfoo
#define foohashmaphfoo

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

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

typedef struct AvahiHashmap AvahiHashmap;

typedef unsigned (*AvahiHashFunc)(const void *data);
typedef int (*AvahiEqualFunc)(const void *a, const void *b);
typedef void (*AvahiFreeFunc)(void *p);

AvahiHashmap* avahi_hashmap_new(AvahiHashFunc hash_func, AvahiEqualFunc equal_func, AvahiFreeFunc key_free_func, AvahiFreeFunc value_free_func);

void avahi_hashmap_free(AvahiHashmap *m);
void* avahi_hashmap_lookup(AvahiHashmap *m, const void *key);
int avahi_hashmap_insert(AvahiHashmap *m, void *key, void *value);
int avahi_hashmap_replace(AvahiHashmap *m, void *key, void *value);
void avahi_hashmap_remove(AvahiHashmap *m, const void *key);

typedef void (*AvahiHashmapForeachCallback)(void *key, void *value, void *userdata);

void avahi_hashmap_foreach(AvahiHashmap *m, AvahiHashmapForeachCallback callback, void *userdata);

unsigned avahi_string_hash(const void *data);
int avahi_string_equal(const void *a, const void *b);

unsigned avahi_int_hash(const void *data);
int avahi_int_equal(const void *a, const void *b);

AVAHI_C_DECL_END

#endif
