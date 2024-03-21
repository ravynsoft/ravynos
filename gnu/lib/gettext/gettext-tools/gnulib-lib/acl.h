/* acl.c - access control lists

   Copyright (C) 2002, 2008-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Written by Paul Eggert.  */

#ifndef _GL_ACL_H
#define _GL_ACL_H 1

/* This file uses _GL_ATTRIBUTE_CONST.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <sys/types.h>
#include <sys/stat.h>

bool acl_errno_valid (int) _GL_ATTRIBUTE_CONST;
int file_has_acl (char const *, struct stat const *);
int qset_acl (char const *, int, mode_t);
int set_acl (char const *, int, mode_t);
int qcopy_acl (char const *, int, char const *, int, mode_t);
int copy_acl (char const *, int, char const *, int, mode_t);
int chmod_or_fchmod (char const *, int, mode_t);

#endif
