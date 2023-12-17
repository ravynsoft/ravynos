/* Opening PO files.
   Copyright (C) 1995-1997, 2000-2003, 2006 Free Software Foundation, Inc.

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

#ifndef _OPEN_CATALOG_H
#define _OPEN_CATALOG_H

#include <stdbool.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Open the input file with the name INPUT_NAME.  The ending .po is added
   if necessary.  If INPUT_NAME is not an absolute file name and the file is
   not found, the list of directories in "dir-list.h" is searched.  The
   file's pathname is returned in *REAL_FILE_NAME_P, for error message
   purposes.  */
extern FILE *open_catalog_file (const char *input_name,
                                char **real_file_name_p, bool exit_on_error);


#ifdef __cplusplus
}
#endif


#endif /* _OPEN_CATALOG_H */
