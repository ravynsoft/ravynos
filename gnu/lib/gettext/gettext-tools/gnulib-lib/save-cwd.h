/* Save and restore current working directory.

   Copyright (C) 1995, 1997-1998, 2003, 2009-2023 Free Software Foundation,
   Inc.

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

/* Written by Jim Meyering.  */

#ifndef SAVE_CWD_H
# define SAVE_CWD_H 1

struct saved_cwd
  {
    int desc;
    char *name;
  };

int save_cwd (struct saved_cwd *cwd);
int restore_cwd (const struct saved_cwd *cwd);
void free_cwd (struct saved_cwd *cwd);

#endif /* SAVE_CWD_H */
