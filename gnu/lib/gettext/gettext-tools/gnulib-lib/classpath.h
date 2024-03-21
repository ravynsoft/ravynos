/* Java CLASSPATH handling.
   Copyright (C) 2003, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2003.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Return the new CLASSPATH value.  The given classpaths are prepended to
   the current CLASSPATH value.   If use_minimal_classpath, the current
   CLASSPATH is ignored.  */
extern char * new_classpath (const char * const *classpaths,
                             unsigned int classpaths_count,
                             bool use_minimal_classpath);

/* Set CLASSPATH and returns a safe copy of its old value.  */
extern char * set_classpath (const char * const *classpaths,
                             unsigned int classpaths_count,
                             bool use_minimal_classpath, bool verbose)
     _GL_ATTRIBUTE_MALLOC;

/* Restore CLASSPATH to its previous value.  */
extern void reset_classpath (char *old_classpath);
