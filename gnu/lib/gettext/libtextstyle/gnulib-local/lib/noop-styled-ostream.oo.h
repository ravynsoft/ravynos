/* Output stream with no-op styling.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

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

#ifndef _NOOP_STYLED_OSTREAM_H
#define _NOOP_STYLED_OSTREAM_H

#include <stdbool.h>

#include "styled-ostream.h"


struct noop_styled_ostream : struct styled_ostream
{
methods:
  /* Accessors.  */
  ostream_t get_destination (noop_styled_ostream_t stream);
  bool      is_owning_destination (noop_styled_ostream_t stream);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that delegates to DESTINATION and that supports
   the styling operations as no-ops.
   If PASS_OWNERSHIP is true, closing the resulting stream will automatically
   close the DESTINATION.
   Note that if PASS_OWNERSHIP is false, the resulting stream must be closed
   before DESTINATION can be closed.  */
extern noop_styled_ostream_t
       noop_styled_ostream_create (ostream_t destination, bool pass_ownership);


/* Test whether a given output stream is a noop_styled_ostream.  */
extern bool is_instance_of_noop_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _NOOP_STYLED_OSTREAM_H */
