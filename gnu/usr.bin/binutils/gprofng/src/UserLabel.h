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

#ifndef _USER_LABEL_H
#define _USER_LABEL_H

#include <time.h>
#include "vec.h"

class Expression;
class StringBuilder;

class UserLabel
{
public:

  enum
  {
    REL_TIME = 0,
    ABS_TIME = 1,
    CUR_TIME = 2
  };

  UserLabel (char *_name);
  ~UserLabel ();
  void register_user_label (int groupId);
  void gen_expr ();
  char *dump ();
  static void dump (const char *msg, Vector<UserLabel*> *labels);

  char *name, *comment, *str_expr, *all_times, *hostname;
  bool start_f, stop_f;
  Expression *expr;
  timeval start_tv;
  long long atime, timeStart, timeStop, start_sec, start_hrtime;
  int id, relative;

private:
  void gen_time_expr (StringBuilder *sb, long long hrtime, char *op);

  static int last_id;
};

#endif
