/* Message list header manipulation.
   Copyright (C) 2007, 2016 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2007.

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

#ifndef _MSGL_HEADER_H
#define _MSGL_HEADER_H

#include "message.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Set the given field to the given value.
   The FIELD name ends in a colon.
   The VALUE will have a space prepended and a newline appended by this
   function.  */
extern void
       msgdomain_list_set_header_field (msgdomain_list_ty *mdlp,
                                        const char *field, const char *value);

/* Remove the given field from the header.
   The FIELD name ends in a colon.  */
extern void
       message_list_delete_header_field (message_list_ty *mlp,
                                         const char *field);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_HEADER_H */
