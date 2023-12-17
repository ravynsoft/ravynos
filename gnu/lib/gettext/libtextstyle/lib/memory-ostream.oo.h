/* Output stream that accumulates the output in memory.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
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

#ifndef _MEMORY_OSTREAM_H
#define _MEMORY_OSTREAM_H

#include <stdbool.h>
#include <stddef.h>

#include "ostream.h"

struct memory_ostream : struct ostream
{
methods:
  /* Return a pointer to the output accumulated so far and its size:
     Store them in *BUFP and *BUFLENP.
     Note: These two return values become invalid when more output is done to
     the stream or when the stream is freed.  */
  void contents (memory_ostream_t stream, const void **bufp, size_t *buflenp);
};

#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that accumulates the output in a memory buffer.  */
extern memory_ostream_t memory_ostream_create (void);


/* Test whether a given output stream is a memory_ostream.  */
extern bool is_instance_of_memory_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _MEMORY_OSTREAM_H */
