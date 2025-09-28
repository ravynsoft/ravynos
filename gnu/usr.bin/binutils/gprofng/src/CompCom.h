/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _COMPCOM_H
#define _COMPCOM_H

#include <sys/types.h>
#include "comp_com.h"

class Elf;
typedef int (*CheckSrcName) (char *);

class CompComment
{
public:
  CompComment (Elf *_elf, int _compcom);
  ~CompComment ();
  int compcom_open (CheckSrcName check_src);
  char *compcom_format (int index, compmsg *msg, int &visible);

private:
  int get_align (int64_t, int align);
  char *get_demangle_name (char *fname);

  Elf *elf;
  int compcom, elf_cls;
  compmsg *msgs;        /* the array of messages */
  int32_t *params;      /* the parameters used in the messages parameters are
			 *  either integers or string-indices */
  char *strs; /* the strings used in the messages */
};

class ComC
{
public:
  ComC ()       { com_str = NULL; };
  ~ComC ()      { free (com_str); };

  int sec;
  int type;
  int visible;
  int line;
  char *com_str;
};

#endif /* _COMPCOM_H */
