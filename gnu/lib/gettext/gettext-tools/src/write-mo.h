/* Writing binary .mo files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2023 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _WRITE_MO_H
#define _WRITE_MO_H

#include <stddef.h>
#include <stdbool.h>

#include "message.h"

/* True if no conversion to UTF-8 is desired.  */
extern bool no_convert_to_utf8;

/* True if the redundant storage of instantiations of system-dependent strings
   shall be avoided.  */
extern bool no_redundancy;

/* Alignment of strings in resulting .mo file.  */
extern size_t alignment;

/* True if writing a .mo file in opposite endianness than the host.  */
extern bool byteswap;

/* True if no hash table in .mo is wanted.  */
extern bool no_hash_table;

/* Write a GNU mo file.  mlp is a list containing the messages to be output.
   domain_name is the domain name, file_name is the desired file name.
   input_file is the name of the input file.
   Return 0 if ok, nonzero on error.  */
extern int
       msgdomain_write_mo (message_list_ty *mlp,
                           const char *domain_name,
                           const char *file_name,
                           const char *input_file);

#endif /* _WRITE_MO_H */
