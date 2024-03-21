/* Customization of the cvs command.
   Copyright (C) 2002, 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

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

/* Enable a variable CVSUSER for cvs.  */
/* See cvs/subr.c: getcaller().  */

#include <stdlib.h>
#include <string.h>
#include <pwd.h>

int getuid (void)
{
  return 0;
}

char * getlogin (void)
{
  char *s;

  s = getenv ("CVSUSER");
  if (s && *s)
    return s;
  s = getenv ("USER");
  if (s && *s)
    return s;
  return NULL;
}

struct passwd * getpwnam (const char *name)
{
  static struct passwd pw;
  static char namebuf[100];

  pw.pw_name = strcpy (namebuf, name);
  pw.pw_passwd = "*";
  pw.pw_uid = 100;
  pw.pw_gid = 100;
  pw.pw_gecos = "";
  pw.pw_dir = "/";
  pw.pw_shell = "/bin/sh";

  return &pw;
}
