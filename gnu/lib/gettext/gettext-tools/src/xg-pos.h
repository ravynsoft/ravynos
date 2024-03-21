/* The source file position.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_POS_H
#define _XGETTEXT_POS_H


#ifdef __cplusplus
extern "C" {
#endif

/* These variables are set by the extractor and used by the extractor and
   its auxiliary functions.  They are *not* meant to be used by xgettext.c.  */


/* Real filename, used in error messages about the input file.  */
extern const char *real_file_name;

/* Logical filename and line number, used to label the extracted messages.  */
extern char *logical_file_name;
extern int line_number;



#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_POS_H */
