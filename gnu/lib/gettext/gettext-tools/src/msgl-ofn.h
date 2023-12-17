/* Message list test for ordinary file names.
   Copyright (C) 2021 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2021.  */

#ifndef _MSGL_OFN_H
#define _MSGL_OFN_H

#include "message.h"

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Tests whether some of the file names in the message locations has spaces.  */

extern bool
       message_has_filenames_with_spaces (message_ty *mp);
extern bool
       message_list_has_filenames_with_spaces (message_list_ty *mlp);
extern bool
       msgdomain_list_has_filenames_with_spaces (msgdomain_list_ty *mdlp);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_OFN_H */
