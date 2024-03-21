/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <stdarg.h>
#include <unistd.h>

#include "gp-defs.h"
#include "StringBuilder.h"
#include "i18n.h"

StringBuilder::StringBuilder ()
{
  count = 0;
  maxCapacity = 16;
  value = (char *) malloc (maxCapacity);
  memset (value, 0, maxCapacity);
}

StringBuilder::StringBuilder (int capacity)
{
  count = 0;
  maxCapacity = capacity;
  value = (char *) malloc (maxCapacity);
  memset (value, 0, maxCapacity);
}

StringBuilder::~StringBuilder ()
{
  free (value);
}

void
StringBuilder::ensureCapacity (int minimumCapacity)
{
  if (minimumCapacity > maxCapacity)
    expandCapacity (minimumCapacity);
}

void
StringBuilder::expandCapacity (int minimumCapacity)
{
  int newCapacity = (maxCapacity + 1) * 2;
  if (newCapacity < 0)
    newCapacity = MAXINT;
  else if (minimumCapacity > newCapacity)
    newCapacity = minimumCapacity;
  char *newValue = (char *) malloc (newCapacity);
  maxCapacity = newCapacity;
  memcpy (newValue, value, count);
  memset (newValue + count, 0, maxCapacity - count);
  free (value);
  value = newValue;
}

void
StringBuilder::trimToSize ()
{
  if (count < maxCapacity)
    {
      char *newValue = (char *) malloc (count);
      maxCapacity = count;
      memcpy (newValue, value, count);
      free (value);
      value = newValue;
    }
}

void
StringBuilder::trim ()
{
  while (count > 0)
    {
      if (value[count - 1] != ' ')
	break;
      count--;
    }
}

void
StringBuilder::setLength (int newLength)
{
  if (newLength < 0)
    return;
  if (newLength > maxCapacity)
    expandCapacity (newLength);
  if (count < newLength)
    {
      for (; count < newLength; count++)
	value[count] = '\0';
    }
  else
    count = newLength;
}

char
StringBuilder::charAt (int index)
{
  if (index < 0 || index >= count)
    return 0;
  return value[index];
}

void
StringBuilder::getChars (int srcBegin, int srcEnd, char dst[], int dstBegin)
{
  if (srcBegin < 0)
    return;
  if (srcEnd < 0 || srcEnd > count)
    return;
  if (srcBegin > srcEnd)
    return;
  memcpy (dst + dstBegin, value + srcBegin, srcEnd - srcBegin);
}

void
StringBuilder::setCharAt (int index, char ch)
{
  if (index < 0 || index >= count)
    return;
  value[index] = ch;
}

StringBuilder *
StringBuilder::append (StringBuilder *sb)
{
  if (sb == NULL)
    return append (NTXT ("null"));
  int len = sb->count;
  int newcount = count + len;
  if (newcount > maxCapacity)
    expandCapacity (newcount);
  sb->getChars (0, len, value, count);
  count = newcount;
  return this;
}

StringBuilder *
StringBuilder::append (const char str[])
{
  int len = (int) strlen (str);
  int newCount = count + len;
  if (newCount > maxCapacity)
    expandCapacity (newCount);
  memcpy (value + count, str, len);
  count = newCount;
  return this;
}

StringBuilder *
StringBuilder::append (const char str[], int offset, int len)
{
  int newCount = count + len;
  if (newCount > maxCapacity)
    expandCapacity (newCount);
  memcpy (value + count, str + offset, len);
  count = newCount;
  return this;
}

StringBuilder *
StringBuilder::append (bool b)
{
  if (b)
    append (NTXT ("true"));
  else
    append (NTXT ("false"));
  return this;
}

StringBuilder *
StringBuilder::append (char c)
{
  int newCount = count + 1;
  if (newCount > maxCapacity)
    {
      expandCapacity (newCount);
    }
  value[count++] = c;
  return this;
}

StringBuilder *
StringBuilder::append (int i)
{
  char buf[16];
  snprintf (buf, sizeof (buf), NTXT ("%d"), i);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (unsigned int i)
{
  char buf[16];
  snprintf (buf, sizeof (buf), NTXT ("%u"), i);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (long lng)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%ld"), lng);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (unsigned long lng)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%lu"), lng);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (long long lng)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%lld"), lng);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (unsigned long long lng)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%llu"), lng);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (float f)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%f"), (double) f);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::append (double d)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%f"), d);
  append (buf);
  return this;
}

StringBuilder *
StringBuilder::_delete (int start, int end)
{
  if (start < 0)
    return this;
  if (end > count)
    end = count;
  if (start > end)
    return this;
  int len = end - start;
  if (len > 0)
    {
      memcpy (value + start, value + start + len, count - end);
      count -= len;
    }
  return this;
}

StringBuilder *
StringBuilder::deleteCharAt (int index)
{
  if (index < 0 || index >= count)
    return this;
  memcpy (value + index, value + index + 1, count - index - 1);
  count--;
  return this;
}

bool
StringBuilder::endsWith (const char str[])
{
  if (str == NULL)
    {
      if (count == 0)
	return true;
      return false;
    }
  int len = (int) strlen (str);
  if (len == 0)
    return true;
  int start = count - len;
  if (start < 0)
    return false;
  int res = strncmp ((const char *) (value + start), str, len);
  if (res != 0)
    return false;
  return true;
}

StringBuilder *
StringBuilder::insert (int index, const char str[], int offset, int len)
{
  if (index < 0 || index > count)
    return this;
  if (offset < 0 || len < 0 || offset > ((int) strlen (str)) - len)
    return this;
  int newCount = count + len;
  if (newCount > maxCapacity)
    expandCapacity (newCount);
  memcpy (value + index + len, value + index, count - index);
  memcpy (value + index, str + offset, len);
  count = newCount;
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, const char str[])
{
  if (offset < 0 || offset > count)
    return this;
  int len = (int) strlen (str);
  int newCount = count + len;
  if (newCount > maxCapacity)
    expandCapacity (newCount);
  memcpy (value + offset + len, value + offset, count - offset);
  memcpy (value + offset, str, len);
  count = newCount;
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, bool b)
{
  return insert (offset, b ? NTXT ("true") : NTXT ("false"));
}

StringBuilder *
StringBuilder::insert (int offset, char c)
{
  int newCount = count + 1;
  if (newCount > maxCapacity)
    expandCapacity (newCount);
  memcpy (value + offset + 1, value + offset, count - offset);
  value[offset] = c;
  count = newCount;
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, int i)
{
  char buf[16];
  snprintf (buf, sizeof (buf), NTXT ("%d"), i);
  insert (offset, buf);
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, long l)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%ld"), l);
  insert (offset, buf);
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, float f)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%f"), (double) f);
  insert (offset, buf);
  return this;
}

StringBuilder *
StringBuilder::insert (int offset, double d)
{
  char buf[32];
  snprintf (buf, sizeof (buf), NTXT ("%f"), d);
  insert (offset, buf);
  return this;
}

StringBuilder *
StringBuilder::reverse ()
{
  int n = count - 1;
  for (int j = (n - 1) >> 1; j >= 0; --j)
    {
      char temp = value[j];
      char temp2 = value[n - j];
      value[j] = temp2;
      value[n - j] = temp;
    }
  return this;
}

//String *StringBuilder::toString();
char *
StringBuilder::toString ()
{
  char *str = (char *) malloc (count + 1);
  memcpy (str, value, count);
  str[count] = '\0';
  return str;
}

void
StringBuilder::toFile (FILE *fp)
{
  append ('\0');
  count--;
  fprintf (fp, NTXT ("%s"), value);
}

void
StringBuilder::toFileLn (FILE *fp)
{
  trim ();
  append ('\0');
  count--;
  fprintf (fp, NTXT ("%s\n"), value);
}

void
StringBuilder::write (int fd)
{
  if (count > 0)
    ::write (fd, value, count);
}

StringBuilder *
StringBuilder::sprintf (const char *fmt, ...)
{
  int cnt;
  setLength (0);

  va_list vp;
  va_start (vp, fmt);
  cnt = vsnprintf (value, maxCapacity, fmt, vp);
  va_end (vp);
  if (cnt < maxCapacity)
    {
      count = cnt;
      return this;
    }

  // Have to count the trailing zero
  ensureCapacity (cnt + 1);
  va_start (vp, fmt);
  count = vsnprintf (value, maxCapacity, fmt, vp);
  va_end (vp);
  return this;
}

StringBuilder *
StringBuilder::appendf (const char *fmt, ...)
{
  va_list vp;
  va_start (vp, fmt);
  int cnt = vsnprintf (value + count, maxCapacity - count, fmt, vp);
  va_end (vp);
  if (cnt + count < maxCapacity)
    {
      count += cnt;
      return this;
    }

  // Have to count the trailing zero
  ensureCapacity (count + cnt + 1);
  va_start (vp, fmt);
  count += vsnprintf (value + count, maxCapacity - count, fmt, vp);
  va_end (vp);
  return this;
}

int
StringBuilder::indexOf (const char str[])
{
  return indexOf (str, 0);
}

int
StringBuilder::indexOf (const char str[], int fromIndex)
{
  int len = (int) strlen (str);
  if (fromIndex >= count)
    return len == 0 ? count : -1;
  if (fromIndex < 0)
    fromIndex = 0;
  if (len == 0)
    return fromIndex;

  char first = str[0];
  int max = (count - len);

  for (int i = fromIndex; i <= max; i++)
    {
      /* Look for first character. */
      if (value[i] != first)
	while (++i <= max && value[i] != first)
	  ;
      /* Found first character, now look at the rest of v2 */
      if (i <= max)
	{
	  int j = i + 1;
	  int end = j + len - 1;
	  for (int k = 1; j < end && value[j] == str[k]; j++, k++)
	    ;
	  if (j == end)     /* Found whole string. */
	    return i;
	}
    }
  return -1;
}

int
StringBuilder::lastIndexOf (const char str[])
{
  return lastIndexOf (str, count);
}

int
StringBuilder::lastIndexOf (const char str[], int fromIndex)
{
  /*
   * Check arguments; return immediately where possible. For
   * consistency, don't check for null str.
   */
  int len = (int) strlen (str);
  int rightIndex = count - len;
  if (fromIndex < 0)
    return -1;
  if (fromIndex > rightIndex)
    fromIndex = rightIndex;
  /* Empty string always matches. */
  if (len == 0)
    return fromIndex;

  int strLastIndex = len - 1;
  char strLastChar = str[strLastIndex];
  int min = len - 1;
  int i = min + fromIndex;

  while (true)
    {
      while (i >= min && value[i] != strLastChar)
	i--;
      if (i < min)
	return -1;

      int j = i - 1;
      int start = j - (len - 1);
      int k = strLastIndex - 1;
      while (j > start)
	{
	  if (value[j--] != str[k--])
	    {
	      i--;
	      break;
	    }
	}
      if (j == start)
	return start + 1;
    }
}

