/* read-file.h -- read file contents into a string
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Simon Josefsson.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef READ_FILE_H
#define READ_FILE_H

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get size_t, free().  */
#include <stdlib.h>

/* Get FILE.  */
#include <stdio.h>

/* Indicate that the file is treated as binary.  */
#define RF_BINARY 0x1

/* Indicate that the file content contains sensitive information.  */
#define RF_SENSITIVE 0x2

extern char *fread_file (FILE * stream, int flags, size_t * length)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

extern char *read_file (const char *filename, int flags, size_t * length)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

#endif /* READ_FILE_H */
