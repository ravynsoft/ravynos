/* Routines for locating data files
   Copyright (C) 2016, 2019 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2016.

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

#ifndef _SEARCH_PATH_H
#define _SEARCH_PATH_H

#ifdef __cplusplus
extern "C" {
#endif


/* Find the standard search path for data files.  If SUB is not NULL, append it
   to each directory.
   Returns a freshly allocated NULL terminated list of freshly allocated
   strings.  */
extern char **get_search_path (const char *sub);


#ifdef __cplusplus
}
#endif

#endif /* _SEARCH_PATH_H */
