/* Target signal numbers for GDB and the GDB remote protocol.
   Copyright (C) 1986-2023 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GDB_SIGNALS_H
#define GDB_SIGNALS_H

/* The numbering of these signals is chosen to match traditional unix
   signals (insofar as various unices use the same numbers, anyway).
   It is also the numbering of the GDB remote protocol.  Other remote
   protocols, if they use a different numbering, should make sure to
   translate appropriately.

   Since these numbers have actually made it out into other software
   (stubs, etc.), you mustn't disturb the assigned numbering.  If you
   need to add new signals here, add them to the end of the explicitly
   numbered signals, at the comment marker.  Add them unconditionally,
   not within any #if or #ifdef.

   This is based strongly on Unix/POSIX signals for several reasons:
   (1) This set of signals represents a widely-accepted attempt to
   represent events of this sort in a portable fashion, (2) we want a
   signal to make it from wait to child_wait to the user intact, (3) many
   remote protocols use a similar encoding.  However, it is
   recognized that this set of signals has limitations (such as not
   distinguishing between various kinds of SIGSEGV, or not
   distinguishing hitting a breakpoint from finishing a single step).
   So in the future we may get around this either by adding additional
   signals for breakpoint, single-step, etc., or by adding signal
   codes; the latter seems more in the spirit of what BSD, System V,
   etc. are doing to address these issues.  */

/* For an explanation of what each signal means, see
   gdb_signal_to_string.  */

enum gdb_signal
  {
#define SET(symbol, constant, name, string) \
    symbol = constant,
#include "gdb/signals.def"
#undef SET
  };

#endif /* #ifndef GDB_SIGNALS_H */
