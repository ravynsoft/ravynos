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

#ifndef _GP_ARCHIVE_H_
#define _GP_ARCHIVE_H_

#include <regex.h>
#include "DbeApplication.h"

class ArchiveExp;
class LoadObject;
template <class ITEM> class Vector;

enum
{
  ARCH_NOTHING       = 0,
  ARCH_EXE_ONLY      = 1,
  ARCH_USED_EXE_ONLY = 2,
  ARCH_USED_SRC_ONLY = 4,
  ARCH_ALL           = 8
};

class er_archive : public DbeApplication
{
public:
  er_archive (int argc, char *argv[]);
  ~er_archive ();
  void start (int argc, char *argv[]);

private:
  void usage ();
  int check_args (int argc, char *argv[]);
  int clean_old_archive (char *expname, ArchiveExp *founder_exp);
  int mask_is_on (const char *str);
  void check_env_var ();
  Vector <LoadObject*> *get_loadObjs ();

  Vector<regex_t *> *mask;  // -m <regexp>
  int s_option;             // -s NO|ALL|USED
  char *common_archive_dir; // -d // absolute path to common archive
  int force;                // -F
  int quiet;                // -q
  int descendant;           // -n
  int use_relative_path;    // -r
};

#endif