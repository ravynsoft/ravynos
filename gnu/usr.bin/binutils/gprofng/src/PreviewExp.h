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

#ifndef _PREVIEW_EXP_H
#define _PREVIEW_EXP_H

#include "Experiment.h"
#include "vec.h"

class PreviewExp : public Experiment
{
public:
  PreviewExp ();
  ~PreviewExp ();

  virtual Exp_status experiment_open (char *path);

  Vector<char*> *preview_info ();

  char *
  getArgList ()
  {
    return uarglist;
  }

private:
  char *mqueue_str (Emsgqueue *msgqueue, char *null_str);

  int is_group;
};

#endif /* ! _PREVIEW_EXP_H */
