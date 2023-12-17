/* Message list concatenation and duplicate handling.
   Copyright (C) 2001-2003, 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _MSGL_CAT_H
#define _MSGL_CAT_H

#include <stdbool.h>

#include "message.h"
#include "str-list.h"
#include "read-catalog-abstract.h"


#ifdef __cplusplus
extern "C" {
#endif


/* These variables control which messages are selected.  */
extern DLL_VARIABLE int more_than;
extern DLL_VARIABLE int less_than;

/* If true, use the first available translation.
   If false, merge all available translations into one and fuzzy it.  */
extern DLL_VARIABLE bool use_first;

/* If true, merge like msgcomm.
   If false, merge like msgcat and msguniq.  */
extern DLL_VARIABLE bool msgcomm_mode;

/* If true, omit the header entry.
   If false, keep the header entry present in the input.  */
extern DLL_VARIABLE bool omit_header;

extern msgdomain_list_ty *
       catenate_msgdomain_list (string_list_ty *file_list,
                                catalog_input_format_ty input_syntax,
                                const char *to_code);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_CAT_H */
