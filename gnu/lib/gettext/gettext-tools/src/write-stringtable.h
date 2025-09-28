/* Writing NeXTstep/GNUstep .strings files.
   Copyright (C) 2003, 2006, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifndef _WRITE_STRINGTABLE_H
#define _WRITE_STRINGTABLE_H

#include "write-catalog.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Describes a PO file in .strings syntax.  */
extern DLL_VARIABLE const struct catalog_output_format output_format_stringtable;


#ifdef __cplusplus
}
#endif


#endif /* _WRITE_STRINGTABLE_H */
