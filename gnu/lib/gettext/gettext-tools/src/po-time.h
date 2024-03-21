/* PO/POT file timestamps.
   Copyright (C) 2001-2003, 2023 Free Software Foundation, Inc.
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

#ifndef _PO_TIME_H
#define _PO_TIME_H

#include <time.h>

#include "attribute.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Return a freshly allocated string containing the given time in the
   format YYYY-MM-DD HH:MM+TZOFF.  */
extern char *po_strftime (const time_t *tp)
       ATTRIBUTE_MALLOC;


#ifdef __cplusplus
}
#endif

#endif /* _PO_TIME_H */
