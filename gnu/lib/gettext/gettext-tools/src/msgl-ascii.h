/* Message list test for ASCII character set.
   Copyright (C) 2001-2003, 2005, 2023 Free Software Foundation, Inc.
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

#ifndef _MSGL_ASCII_H
#define _MSGL_ASCII_H

#include "string-desc.h"
#include "message.h"

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


extern bool
       is_ascii_string (const char *string);
extern bool
       is_ascii_string_desc (string_desc_t string);
extern bool
       is_ascii_string_list (string_list_ty *slp);
extern bool
       is_ascii_message (message_ty *mp);
extern bool
       is_ascii_message_list (message_list_ty *mlp);
extern bool
       is_ascii_msgdomain_list (msgdomain_list_ty *mdlp);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_ASCII_H */
