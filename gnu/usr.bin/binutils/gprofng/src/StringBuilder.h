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

/*
 *	java/lang/StringBuilder
 *
 *	Based on JavaTM 2 Platform Standard Ed. 5.0
 */

#ifndef _StringBuilder_h
#define _StringBuilder_h

class StringBuilder
{
public:
  StringBuilder ();
  StringBuilder (int capacity);
  virtual ~StringBuilder ();

  int
  length ()
  {
    return count;
  }

  int
  capacity ()
  {
    return maxCapacity;
  }

  bool endsWith (const char str[]);
  void ensureCapacity (int minimumCapacity);
  void expandCapacity (int minimumCapacity);
  void trimToSize ();
  void trim ();
  void setLength (int newLength);
  char charAt (int index);
  void getChars (int srcBegin, int srcEnd, char dst[], int dstBegin);
  void setCharAt (int index, char ch);
  StringBuilder *append (StringBuilder *sb);
  StringBuilder *append (const char str[]);
  StringBuilder *append (const char str[], int offset, int len);
  StringBuilder *append (bool b);
  StringBuilder *append (char c);
  StringBuilder *append (int i);
  StringBuilder *append (unsigned int i);
  StringBuilder *append (long lng);
  StringBuilder *append (unsigned long i);
  StringBuilder *append (long long lng);
  StringBuilder *append (unsigned long long lng);
  StringBuilder *append (float f);
  StringBuilder *append (double d);
  StringBuilder *_delete (int start, int end);
  StringBuilder *deleteCharAt (int index);
  StringBuilder *insert (int index, const char str[], int offset, int len);
  StringBuilder *insert (int offset, const char str[]);
  StringBuilder *insert (int offset, bool b);
  StringBuilder *insert (int offset, char c);
  StringBuilder *insert (int offset, int i);
  StringBuilder *insert (int offset, long l);
  StringBuilder *insert (int offset, float f);
  StringBuilder *insert (int offset, double d);
  StringBuilder *reverse ();
  char *toString ();
  void toFile (FILE *fp);
  void toFileLn (FILE *fp);
  void write (int fd);

  // Not in Java
  StringBuilder *appendf (const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  StringBuilder *sprintf (const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

  int indexOf (const char str[]);
  int indexOf (const char str[], int fromIndex);
  int lastIndexOf (const char str[]);
  int lastIndexOf (const char str[], int fromIndex);

private:
  char *value;
  int count;
  int maxCapacity;
};

#endif /* _StringBuilder_h */
