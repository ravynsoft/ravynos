/* backupfile.h -- declarations for making Emacs style backup file names
   Copyright (C) 1990-1992, 1997-1999, 2001-2003 Free Software Foundation, Inc.

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

#ifndef BACKUPFILE_H_
# define BACKUPFILE_H_

#ifdef __cplusplus
extern "C" {
#endif


/* When to make backup files. */
enum backup_type
{
  /* Never make backups. */
  none,

  /* Make simple backups of every file. */
  simple,

  /* Make numbered backups of files that already have numbered backups,
     and simple backups of the others. */
  numbered_existing,

  /* Make numbered backups of every file. */
  numbered
};

# define VALID_BACKUP_TYPE(Type)        \
  ((Type) == none                       \
   || (Type) == simple                  \
   || (Type) == numbered_existing       \
   || (Type) == numbered)

extern DLL_VARIABLE char const *simple_backup_suffix;

extern char *find_backup_file_name (char const *file,
                                    enum backup_type backup_type);
extern enum backup_type get_version (char const *context, char const *arg);
extern enum backup_type xget_version (char const *context, char const *arg);
extern void addext (char *filename, char const *ext, char e);


#ifdef __cplusplus
}
#endif

#endif /* ! BACKUPFILE_H_ */
