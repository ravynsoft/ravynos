/* GNU gettext - internationalization aids
   Copyright (C) 1996, 1998, 2000-2003 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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

#ifndef _DIR_LIST_H
#define _DIR_LIST_H

/* Management of the list of directories where PO files are searched.
   It is an ordered list, without duplicates.  The default value of the
   list consists of the single directory ".".  */


#ifdef __cplusplus
extern "C" {
#endif


/* Append a directory to the end of the list of directories.  */
extern void dir_list_append (const char *directory);

/* Return the nth directory, or NULL of n is out of range.  */
extern const char *dir_list_nth (int n);

/* Return the current list of directories, for later use with dir_list_restore.
   Reset the list to empty.  */
extern void *dir_list_save_reset (void);

/* Restore a previously saved list of directories.  */
extern void dir_list_restore (void *saved_value);


#ifdef __cplusplus
}
#endif


#endif /* _DIR_LIST_H */
