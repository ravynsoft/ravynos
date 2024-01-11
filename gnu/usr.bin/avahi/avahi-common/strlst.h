#ifndef footxtlisthfoo
#define footxtlisthfoo

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

/** \file strlst.h Implementation of a data type to store lists of strings */

#include <sys/types.h>
#include <inttypes.h>
#include <stdarg.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/gccmacro.h>

AVAHI_C_DECL_BEGIN

/** Linked list of strings that can contain any number of binary
 * characters, including NUL bytes. An empty list is created by
 * assigning a NULL to a pointer to AvahiStringList. The string list
 * is stored in reverse order, so that appending to the string list is
 * effectively a prepending to the linked list.  This object is used
 * primarily for storing DNS TXT record data. */
typedef struct AvahiStringList {
    struct AvahiStringList *next; /**< Pointer to the next linked list element */
    size_t size;  /**< Size of text[] */
    uint8_t text[1]; /**< Character data */
} AvahiStringList;

/** @{ \name Construction and destruction */

/** Create a new string list by taking a variable list of NUL
 * terminated strings. The strings are copied using g_strdup(). The
 * argument list must be terminated by a NULL pointer. */
AvahiStringList *avahi_string_list_new(const char *txt, ...) AVAHI_GCC_SENTINEL;

/** \cond fulldocs */
/** Same as avahi_string_list_new() but pass a va_list structure */
AvahiStringList *avahi_string_list_new_va(va_list va);
/** \endcond */

/** Create a new string list from a string array. The strings are
 * copied using g_strdup(). length should contain the length of the
 * array, or -1 if the array is NULL terminated*/
AvahiStringList *avahi_string_list_new_from_array(const char **array, int length);

/** Free a string list */
void avahi_string_list_free(AvahiStringList *l);

/** @} */

/** @{ \name Adding strings */

/** Append a NUL terminated string to the specified string list. The
 * passed string is copied using g_strdup(). Returns the new list
 * start. */
AvahiStringList *avahi_string_list_add(AvahiStringList *l, const char *text);

/** Append a new NUL terminated formatted string to the specified string list */
AvahiStringList *avahi_string_list_add_printf(AvahiStringList *l, const char *format, ...) AVAHI_GCC_PRINTF_ATTR23;

/** \cond fulldocs */
/** Append a new NUL terminated formatted string to the specified string list */
AvahiStringList *avahi_string_list_add_vprintf(AvahiStringList *l, const char *format, va_list va);
/** \endcond */

/** Append an arbitrary length byte string to the list. Returns the
 * new list start. */
AvahiStringList *avahi_string_list_add_arbitrary(AvahiStringList *l, const uint8_t *text, size_t size);

/** Append a new entry to the string list. The string is not filled
with data. The caller should fill in string data afterwards by writing
it to l->text, where l is the pointer returned by this function. This
function exists solely to optimize a few operations where otherwise
superfluous string copying would be necessary. */
AvahiStringList*avahi_string_list_add_anonymous(AvahiStringList *l, size_t size);

/** Same as avahi_string_list_add(), but takes a variable number of
 * NUL terminated strings. The argument list must be terminated by a
 * NULL pointer. Returns the new list start. */
AvahiStringList *avahi_string_list_add_many(AvahiStringList *r, ...) AVAHI_GCC_SENTINEL;

/** \cond fulldocs */
/** Same as avahi_string_list_add_many(), but use a va_list
 * structure. Returns the new list start. */
AvahiStringList *avahi_string_list_add_many_va(AvahiStringList *r, va_list va);
/** \endcond */

/** @} */

/** @{ \name String list operations */

/** Convert the string list object to a single character string,
 * seperated by spaces and enclosed in "". avahi_free() the result! This
 * function doesn't work well with strings that contain NUL bytes. */
char* avahi_string_list_to_string(AvahiStringList *l);

/** \cond fulldocs */
/** Serialize the string list object in a way that is compatible with
 * the storing of DNS TXT records. Strings longer than 255 bytes are truncated. */
size_t avahi_string_list_serialize(AvahiStringList *l, void * data, size_t size);

/** Inverse of avahi_string_list_serialize() */
int avahi_string_list_parse(const void *data, size_t size, AvahiStringList **ret);
/** \endcond */

/** Compare to string lists */
int avahi_string_list_equal(const AvahiStringList *a, const AvahiStringList *b);

/** Copy a string list */
AvahiStringList *avahi_string_list_copy(const AvahiStringList *l);

/** Reverse the string list. */
AvahiStringList* avahi_string_list_reverse(AvahiStringList *l);

/** Return the number of elements in the string list */
unsigned avahi_string_list_length(const AvahiStringList *l);

/** @} */

/** @{ \name Accessing items */

/** Returns the next item in the string list */
AvahiStringList *avahi_string_list_get_next(AvahiStringList *l);

/** Returns the text for the current item */
uint8_t *avahi_string_list_get_text(AvahiStringList *l);

/** Returns the size of the current text */
size_t avahi_string_list_get_size(AvahiStringList *l);

/** @} */

/** @{ \name DNS-SD TXT pair handling */

/** Find the string list entry for the given DNS-SD TXT key */
AvahiStringList *avahi_string_list_find(AvahiStringList *l, const char *key);

/** Return the DNS-SD TXT key and value for the specified string list
 * item. If size is not NULL it will be filled with the length of
 * value. (for strings containing NUL bytes). If the entry doesn't
 * contain a value *value will be set to NULL. You need to
 * avahi_free() the strings returned in *key and *value. */
int avahi_string_list_get_pair(AvahiStringList *l, char **key, char **value, size_t *size);

/** Add a new DNS-SD TXT key value pair to the string list. value may
 * be NULL in case you want to specify a key without a value */
AvahiStringList *avahi_string_list_add_pair(AvahiStringList *l, const char *key, const char *value);

/** Same as avahi_string_list_add_pair() but allow strings containing NUL bytes in *value. */
AvahiStringList *avahi_string_list_add_pair_arbitrary(AvahiStringList *l, const char *key, const uint8_t *value, size_t size);

/** @} */

/** \cond fulldocs */
/** Try to find a magic service cookie in the specified DNS-SD string
 * list. Or return AVAHI_SERVICE_COOKIE_INVALID if none is found. */
uint32_t avahi_string_list_get_service_cookie(AvahiStringList *l);
/** \endcond */

AVAHI_C_DECL_END

#endif

