/* Message list test for equality.
   Copyright (C) 2001-2003 Free Software Foundation, Inc.
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

#ifndef _MSGL_EQUAL_H
#define _MSGL_EQUAL_H

#include <stdbool.h>

#include "message.h"


#ifdef __cplusplus
extern "C" {
#endif


extern bool
       string_list_equal (const string_list_ty *slp1,
                          const string_list_ty *slp2);

/* Test whether the written representation of two messages / message lists
   would be the same.  */

extern bool
       message_equal (const message_ty *mp1, const message_ty *mp2,
                      bool ignore_potcdate);
extern bool
       message_list_equal (const message_list_ty *mlp1,
                           const message_list_ty *mlp2,
                           bool ignore_potcdate);
extern bool
       msgdomain_list_equal (const msgdomain_list_ty *mdlp1,
                             const msgdomain_list_ty *mdlp2,
                             bool ignore_potcdate);


#ifdef __cplusplus
}
#endif


#endif /* _MSGL_EQUAL_H */
