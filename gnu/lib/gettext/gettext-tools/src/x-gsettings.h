/* xgettext GSettings schema file backend.
   Copyright (C) 2002-2003, 2006, 2013-2015, 2018, 2020 Free Software Foundation, Inc.
   Written by Daiki Ueno <ueno@gnu.org>, 2013.

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


#include <stdio.h>

#include "message.h"
#include "xgettext.h"


#ifdef __cplusplus
extern "C" {
#endif


/* The scanner is implemented as ITS rules, in its/gsettings.its.  */

#define EXTENSIONS_GSETTINGS                    \
  { "gschema.xml", NULL }, \

#define SCANNERS_GSETTINGS \
  { "gsettings", NULL, NULL, NULL, NULL, NULL }, \


#ifdef __cplusplus
}
#endif
