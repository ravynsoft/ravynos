/* Output stream referring to an stdio FILE.
   Copyright (C) 2006, 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _FILE_OSTREAM_H
#define _FILE_OSTREAM_H

#include <stdbool.h>
#include <stdio.h>

#include "ostream.h"


struct file_ostream : struct ostream
{
methods:
  /* Accessors.  */
  FILE * get_stdio_stream (file_ostream_t stream);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to FP.
   Note that the resulting stream must be closed before FP can be closed.  */
extern file_ostream_t file_ostream_create (FILE *fp);


/* Test whether a given output stream is a file_ostream.  */
extern bool is_instance_of_file_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _FILE_OSTREAM_H */
