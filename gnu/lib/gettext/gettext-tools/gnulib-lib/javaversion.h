/* Determine the Java version supported by javaexec.
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _JAVAVERSION_H
#define _JAVAVERSION_H

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Return information about the Java version used by execute_java_class().
   This is the value of System.getProperty("java.specification.version").
   Some possible values are: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 9, 10, 11,
   12, 13, 14, 15, 16, 17, 18, 19, 20.
   Return NULL if the Java version cannot be determined.  */
extern char * javaexec_version (void)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;


#ifdef __cplusplus
}
#endif


#endif /* _JAVAVERSION_H */
