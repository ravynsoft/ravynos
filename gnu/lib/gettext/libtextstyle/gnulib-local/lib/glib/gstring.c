/* GLIB - Library of useful routines for C programming
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * Modified by Bruno Haible for use as a gnulib module.
 */

/* 
 * MT safe
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "glib.h"
#if 0
#include "gprintf.h"

#include "galias.h"

struct _GStringChunk
{
  GHashTable *const_table;
  GSList     *storage_list;
  gsize       storage_next;    
  gsize       this_size;       
  gsize       default_size;    
};
#endif

/* Hash Functions.
 */

/**
 * g_str_equal:
 * @v1: a key. 
 * @v2: a key to compare with @v1.
 * 
 * Compares two strings and returns %TRUE if they are equal.
 * It can be passed to g_hash_table_new() as the @key_equal_func
 * parameter, when using strings as keys in a #GHashTable.
 *
 * Returns: %TRUE if the two keys match.
 */
gboolean
g_str_equal (gconstpointer v1,
	     gconstpointer v2)
{
  const gchar *string1 = v1;
  const gchar *string2 = v2;
  
  return strcmp (string1, string2) == 0;
}

/**
 * g_str_hash:
 * @v: a string key.
 *
 * Converts a string to a hash value.
 * It can be passed to g_hash_table_new() as the @hash_func parameter, 
 * when using strings as keys in a #GHashTable.
 *
 * Returns: a hash value corresponding to the key.
 */
guint
g_str_hash (gconstpointer v)
{
  /* 31 bit hash function */
  const signed char *p = v;
  guint32 h = *p;

  if (h)
    for (p += 1; *p != '\0'; p++)
      h = (h << 5) - h + *p;

  return h;
}

#define MY_MAXSIZE ((gsize)-1)

static inline gsize
nearest_power (gsize base, gsize num)    
{
  if (num > MY_MAXSIZE / 2)
    {
      return MY_MAXSIZE;
    }
  else
    {
      gsize n = base;

      while (n < num)
	n <<= 1;
      
      return n;
    }
}

#if 0

/* String Chunks.
 */

GStringChunk*
g_string_chunk_new (gsize default_size)    
{
  GStringChunk *new_chunk = g_new (GStringChunk, 1);
  gsize size = 1;    

  size = nearest_power (1, default_size);

  new_chunk->const_table       = NULL;
  new_chunk->storage_list      = NULL;
  new_chunk->storage_next      = size;
  new_chunk->default_size      = size;
  new_chunk->this_size         = size;

  return new_chunk;
}

void
g_string_chunk_free (GStringChunk *chunk)
{
  GSList *tmp_list;

  g_return_if_fail (chunk != NULL);

  if (chunk->storage_list)
    {
      for (tmp_list = chunk->storage_list; tmp_list; tmp_list = tmp_list->next)
	g_free (tmp_list->data);

      g_slist_free (chunk->storage_list);
    }

  if (chunk->const_table)
    g_hash_table_destroy (chunk->const_table);

  g_free (chunk);
}

gchar*
g_string_chunk_insert (GStringChunk *chunk,
		       const gchar  *string)
{
  g_return_val_if_fail (chunk != NULL, NULL);

  return g_string_chunk_insert_len (chunk, string, -1);
}

gchar*
g_string_chunk_insert_const (GStringChunk *chunk,
			     const gchar  *string)
{
  char* lookup;

  g_return_val_if_fail (chunk != NULL, NULL);

  if (!chunk->const_table)
    chunk->const_table = g_hash_table_new (g_str_hash, g_str_equal);

  lookup = (char*) g_hash_table_lookup (chunk->const_table, (gchar *)string);

  if (!lookup)
    {
      lookup = g_string_chunk_insert (chunk, string);
      g_hash_table_insert (chunk->const_table, lookup, lookup);
    }

  return lookup;
}

/**
 * g_string_chunk_insert_len:
 * @chunk: a #GStringChunk
 * @string: bytes to insert
 * @len: number of bytes of @string to insert, or -1 to insert a 
 *     nul-terminated string. 
 * 
 * Adds a copy of the first @len bytes of @string to the #GStringChunk. The
 * copy is nul-terminated.
 * 
 * The characters in the string can be changed, if necessary, though you
 * should not change anything after the end of the string.
 * 
 * Return value: a pointer to the copy of @string within the #GStringChunk
 * 
 * Since: 2.4
 **/
gchar*
g_string_chunk_insert_len (GStringChunk *chunk,
			   const gchar  *string, 
			   gssize        len)
{
  gssize size;
  gchar* pos;

  g_return_val_if_fail (chunk != NULL, NULL);

  if (len < 0)
    size = strlen (string);
  else
    size = len;
  
  if ((chunk->storage_next + size + 1) > chunk->this_size)
    {
      gsize new_size = nearest_power (chunk->default_size, size + 1);

      chunk->storage_list = g_slist_prepend (chunk->storage_list,
					     g_new (gchar, new_size));

      chunk->this_size = new_size;
      chunk->storage_next = 0;
    }

  pos = ((gchar *) chunk->storage_list->data) + chunk->storage_next;

  *(pos + size) = '\0';

  strncpy (pos, string, size);
  if (len > 0)
    size = strlen (pos);

  chunk->storage_next += size + 1;

  return pos;
}

#endif

/* Strings.
 */
static void
g_string_maybe_expand (GString* string,
		       gsize    len) 
{
  if (string->len + len >= string->allocated_len)
    {
      string->allocated_len = nearest_power (1, string->len + len + 1);
      string->str = g_realloc (string->str, string->allocated_len);
    }
}

GString*
g_string_sized_new (gsize dfl_size)    
{
  GString *string = g_slice_new (GString);

  string->allocated_len = 0;
  string->len   = 0;
  string->str   = NULL;

  g_string_maybe_expand (string, MAX (dfl_size, 2));
  string->str[0] = 0;

  return string;
}

GString*
g_string_new (const gchar *init)
{
  GString *string;

  if (init == NULL || *init == '\0')
    string = g_string_sized_new (2);
  else 
    {
      gint len;

      len = strlen (init);
      string = g_string_sized_new (len + 2);

      g_string_append_len (string, init, len);
    }

  return string;
}

GString*
g_string_new_len (const gchar *init,
                  gssize       len)    
{
  GString *string;

  if (len < 0)
    return g_string_new (init);
  else
    {
      string = g_string_sized_new (len);
      
      if (init)
        g_string_append_len (string, init, len);
      
      return string;
    }
}

gchar*
g_string_free (GString *string,
	       gboolean free_segment)
{
  gchar *segment;

  g_return_val_if_fail (string != NULL, NULL);

  if (free_segment)
    {
      g_free (string->str);
      segment = NULL;
    }
  else
    segment = string->str;

  g_slice_free (GString, string);

  return segment;
}

#if 0

gboolean
g_string_equal (const GString *v,
                const GString *v2)
{
  gchar *p, *q;
  GString *string1 = (GString *) v;
  GString *string2 = (GString *) v2;
  gsize i = string1->len;    

  if (i != string2->len)
    return FALSE;

  p = string1->str;
  q = string2->str;
  while (i)
    {
      if (*p != *q)
	return FALSE;
      p++;
      q++;
      i--;
    }
  return TRUE;
}

/* 31 bit hash function */
guint
g_string_hash (const GString *str)
{
  const gchar *p = str->str;
  gsize n = str->len;    
  guint h = 0;

  while (n--)
    {
      h = (h << 5) - h + *p;
      p++;
    }

  return h;
}

GString*
g_string_assign (GString     *string,
		 const gchar *rval)
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (rval != NULL, string);

  /* Make sure assigning to itself doesn't corrupt the string.  */
  if (string->str != rval)
    {
      /* Assigning from substring should be ok since g_string_truncate
	 does not realloc.  */
      g_string_truncate (string, 0);
      g_string_append (string, rval);
    }

  return string;
}

GString*
g_string_truncate (GString *string,
		   gsize    len)    
{
  g_return_val_if_fail (string != NULL, NULL);

  string->len = MIN (len, string->len);
  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_set_size:
 * @string: a #GString
 * @len: the new length
 * 
 * Sets the length of a #GString. If the length is less than
 * the current length, the string will be truncated. If the
 * length is greater than the current length, the contents
 * of the newly added area are undefined. (However, as
 * always, string->str[string->len] will be a nul byte.) 
 * 
 * Return value: @string
 **/
GString*
g_string_set_size (GString *string,
		   gsize    len)    
{
  g_return_val_if_fail (string != NULL, NULL);

  if (len >= string->allocated_len)
    g_string_maybe_expand (string, len - string->len);
  
  string->len = len;
  string->str[len] = 0;

  return string;
}

#endif

GString*
g_string_insert_len (GString     *string,
		     gssize       pos,    
		     const gchar *val,
		     gssize       len)    
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);

  if (len < 0)
    len = strlen (val);

  if (pos < 0)
    pos = string->len;
  else
    g_return_val_if_fail (pos <= string->len, string);

  /* Check whether val represents a substring of string.  This test
     probably violates chapter and verse of the C standards, since
     ">=" and "<=" are only valid when val really is a substring.
     In practice, it will work on modern archs.  */
  if (val >= string->str && val <= string->str + string->len)
    {
      gsize offset = val - string->str;
      gsize precount = 0;

      g_string_maybe_expand (string, len);
      val = string->str + offset;
      /* At this point, val is valid again.  */

      /* Open up space where we are going to insert.  */
      if (pos < string->len)
	g_memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* Move the source part before the gap, if any.  */
      if (offset < pos)
	{
	  precount = MIN (len, pos - offset);
	  memcpy (string->str + pos, val, precount);
	}

      /* Move the source part after the gap, if any.  */
      if (len > precount)
	memcpy (string->str + pos + precount,
		val + /* Already moved: */ precount + /* Space opened up: */ len,
		len - precount);
    }
  else
    {
      g_string_maybe_expand (string, len);

      /* If we aren't appending at the end, move a hunk
       * of the old string to the end, opening up space
       */
      if (pos < string->len)
	g_memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* insert the new string */
      if (len == 1)
        string->str[pos] = *val;
      else
        memcpy (string->str + pos, val, len);
    }

  string->len += len;

  string->str[string->len] = 0;

  return string;
}

GString*
g_string_append (GString     *string,
		 const gchar *val)
{  
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);

  return g_string_insert_len (string, -1, val, -1);
}

GString*
g_string_append_len (GString	 *string,
                     const gchar *val,
                     gssize       len)    
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);

  return g_string_insert_len (string, -1, val, len);
}

#if !defined IN_LIBTEXTSTYLE
#undef g_string_append_c
#endif
GString*
g_string_append_c (GString *string,
		   gchar    c)
{
  g_return_val_if_fail (string != NULL, NULL);

  return g_string_insert_c (string, -1, c);
}

/**
 * g_string_append_unichar:
 * @string: a #GString
 * @wc: a Unicode character
 * 
 * Converts a Unicode character into UTF-8, and appends it
 * to the string.
 * 
 * Return value: @string
 **/
GString*
g_string_append_unichar (GString  *string,
			 gunichar  wc)
{  
  g_return_val_if_fail (string != NULL, NULL);
  
  return g_string_insert_unichar (string, -1, wc);
}

#if 0

GString*
g_string_prepend (GString     *string,
		  const gchar *val)
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);
  
  return g_string_insert_len (string, 0, val, -1);
}

GString*
g_string_prepend_len (GString	  *string,
                      const gchar *val,
                      gssize       len)    
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);

  return g_string_insert_len (string, 0, val, len);
}

GString*
g_string_prepend_c (GString *string,
		    gchar    c)
{  
  g_return_val_if_fail (string != NULL, NULL);
  
  return g_string_insert_c (string, 0, c);
}

/**
 * g_string_prepend_unichar:
 * @string: a #GString.
 * @wc: a Unicode character.
 * 
 * Converts a Unicode character into UTF-8, and prepends it
 * to the string.
 * 
 * Return value: @string.
 **/
GString*
g_string_prepend_unichar (GString  *string,
			  gunichar  wc)
{  
  g_return_val_if_fail (string != NULL, NULL);
  
  return g_string_insert_unichar (string, 0, wc);
}

GString*
g_string_insert (GString     *string,
		 gssize       pos,    
		 const gchar *val)
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (val != NULL, string);
  if (pos >= 0)
    g_return_val_if_fail (pos <= string->len, string);
  
  return g_string_insert_len (string, pos, val, -1);
}

#endif

GString*
g_string_insert_c (GString *string,
		   gssize   pos,    
		   gchar    c)
{
  g_return_val_if_fail (string != NULL, NULL);

  g_string_maybe_expand (string, 1);

  if (pos < 0)
    pos = string->len;
  else
    g_return_val_if_fail (pos <= string->len, string);
  
  /* If not just an append, move the old stuff */
  if (pos < string->len)
    g_memmove (string->str + pos + 1, string->str + pos, string->len - pos);

  string->str[pos] = c;

  string->len += 1;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_insert_unichar:
 * @string: a #GString
 * @pos: the position at which to insert character, or -1 to
 *       append at the end of the string.
 * @wc: a Unicode character
 * 
 * Converts a Unicode character into UTF-8, and insert it
 * into the string at the given position.
 * 
 * Return value: @string
 **/
GString*
g_string_insert_unichar (GString *string,
			 gssize   pos,    
			 gunichar wc)
{
  gint charlen, first, i;
  gchar *dest;

  g_return_val_if_fail (string != NULL, NULL);

  /* Code copied from g_unichar_to_utf() */
  if (wc < 0x80)
    {
      first = 0;
      charlen = 1;
    }
  else if (wc < 0x800)
    {
      first = 0xc0;
      charlen = 2;
    }
  else if (wc < 0x10000)
    {
      first = 0xe0;
      charlen = 3;
    }
   else if (wc < 0x200000)
    {
      first = 0xf0;
      charlen = 4;
    }
  else if (wc < 0x4000000)
    {
      first = 0xf8;
      charlen = 5;
    }
  else
    {
      first = 0xfc;
      charlen = 6;
    }
  /* End of copied code */

  g_string_maybe_expand (string, charlen);

  if (pos < 0)
    pos = string->len;
  else
    g_return_val_if_fail (pos <= string->len, string);

  /* If not just an append, move the old stuff */
  if (pos < string->len)
    g_memmove (string->str + pos + charlen, string->str + pos, string->len - pos);

  dest = string->str + pos;
  /* Code copied from g_unichar_to_utf() */
  for (i = charlen - 1; i > 0; --i)
    {
      dest[i] = (wc & 0x3f) | 0x80;
      wc >>= 6;
    }
  dest[0] = wc | first;
  /* End of copied code */
  
  string->len += charlen;

  string->str[string->len] = 0;

  return string;
}

#if 0

GString*
g_string_erase (GString *string,
		gssize   pos,
		gssize   len)
{
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (pos >= 0, string);
  g_return_val_if_fail (pos <= string->len, string);

  if (len < 0)
    len = string->len - pos;
  else
    {
      g_return_val_if_fail (pos + len <= string->len, string);

      if (pos + len < string->len)
	g_memmove (string->str + pos, string->str + pos + len, string->len - (pos + len));
    }

  string->len -= len;
  
  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_ascii_down:
 * @string: a GString
 * 
 * Converts all upper case ASCII letters to lower case ASCII letters.
 * 
 * Return value: passed-in @string pointer, with all the upper case
 *               characters converted to lower case in place, with
 *               semantics that exactly match g_ascii_tolower.
 **/
GString*
g_string_ascii_down (GString *string)
{
  gchar *s;
  gint n;

  g_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = string->str;

  while (n)
    {
      *s = g_ascii_tolower (*s);
      s++;
      n--;
    }

  return string;
}

/**
 * g_string_ascii_up:
 * @string: a GString
 * 
 * Converts all lower case ASCII letters to upper case ASCII letters.
 * 
 * Return value: passed-in @string pointer, with all the lower case
 *               characters converted to upper case in place, with
 *               semantics that exactly match g_ascii_toupper.
 **/
GString*
g_string_ascii_up (GString *string)
{
  gchar *s;
  gint n;

  g_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = string->str;

  while (n)
    {
      *s = g_ascii_toupper (*s);
      s++;
      n--;
    }

  return string;
}

/**
 * g_string_down:
 * @string: a #GString
 *  
 * Converts a #GString to lowercase.
 *
 * Returns: the #GString.
 *
 * Deprecated:2.2: This function uses the locale-specific tolower() function, 
 * which is almost never the right thing. Use g_string_ascii_down() or 
 * g_utf8_strdown() instead.
 */
GString*
g_string_down (GString *string)
{
  guchar *s;
  glong n;

  g_return_val_if_fail (string != NULL, NULL);

  n = string->len;    
  s = (guchar *) string->str;

  while (n)
    {
      if (isupper (*s))
	*s = tolower (*s);
      s++;
      n--;
    }

  return string;
}

/**
 * g_string_up:
 * @string: a #GString 
 * 
 * Converts a #GString to uppercase.
 * 
 * Return value: the #GString
 *
 * Deprecated:2.2: This function uses the locale-specific toupper() function, 
 * which is almost never the right thing. Use g_string_ascii_up() or 
 * g_utf8_strup() instead.
 **/
GString*
g_string_up (GString *string)
{
  guchar *s;
  glong n;

  g_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = (guchar *) string->str;

  while (n)
    {
      if (islower (*s))
	*s = toupper (*s);
      s++;
      n--;
    }

  return string;
}

#endif

static void
g_string_append_printf_internal (GString     *string,
				 const gchar *fmt,
				 va_list      args)
{
  gchar *buffer;
  gint length;
  
  length = g_vasprintf (&buffer, fmt, args);
  g_string_append_len (string, buffer, length);
  g_free (buffer);
}

#if 0

void
g_string_printf (GString *string,
		 const gchar *fmt,
		 ...)
{
  va_list args;

  g_string_truncate (string, 0);

  va_start (args, fmt);
  g_string_append_printf_internal (string, fmt, args);
  va_end (args);
}

#endif

void
g_string_append_printf (GString *string,
			const gchar *fmt,
			...)
{
  va_list args;

  va_start (args, fmt);
  g_string_append_printf_internal (string, fmt, args);
  va_end (args);
}

#if 0
#define __G_STRING_C__
#include "galiasdef.c"
#endif
