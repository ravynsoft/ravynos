/* Copying of files.
   Copyright (C) 2001-2003, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#ifdef __cplusplus
extern "C" {
#endif


/* Error codes returned by qcopy_file_preserving.  */
enum
{
  GL_COPY_ERR_OPEN_READ = -1,
  GL_COPY_ERR_OPEN_BACKUP_WRITE = -2,
  GL_COPY_ERR_READ = -3,
  GL_COPY_ERR_WRITE = -4,
  GL_COPY_ERR_AFTER_READ = -5,
  GL_COPY_ERR_GET_ACL = -6,
  GL_COPY_ERR_SET_ACL = -7
};

/* Copy a regular file: from src_filename to dest_filename.
   The destination file is assumed to be a backup file.
   Modification times, owner, group and access permissions are preserved as
   far as possible.
   Return 0 if successful, otherwise set errno and return one of the error
   codes above.  */
extern int qcopy_file_preserving (const char *src_filename, const char *dest_filename);

/* Copy a regular file: from src_filename to dest_filename.
   The destination file is assumed to be a backup file.
   Modification times, owner, group and access permissions are preserved as
   far as possible.
   Exit upon failure.  */
extern void copy_file_preserving (const char *src_filename, const char *dest_filename);


#ifdef __cplusplus
}
#endif
