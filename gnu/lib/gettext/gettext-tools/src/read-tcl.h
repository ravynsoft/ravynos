/* Reading tcl/msgcat .msg files.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

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

#ifndef _READ_TCL_H
#define _READ_TCL_H

#include "message.h"

/* Read the Tcl msg file given by locale_name and directory.
   Returns a list of messages.  */
extern msgdomain_list_ty *
       msgdomain_read_tcl (const char *locale_name, const char *directory);

#endif /* _READ_TCL_H */
