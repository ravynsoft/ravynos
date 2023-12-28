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

#ifndef _EXPGROUP_H
#define _EXPGROUP_H

#include "vec.h"
#include "Map.h"

class Experiment;
class LoadObject;

class ExpGroup
{
public:
  ExpGroup (char *nm);
  ~ExpGroup ();
  void append (Experiment *exp);
  void drop_experiment (Experiment *exp);
  Vector<Experiment*> *get_founders ();
  void create_list_of_loadObjects ();
  LoadObject *get_comparable_loadObject (LoadObject *lo);

  Vector<Experiment*> *exps;
  Vector<LoadObject*> *loadObjs;
  Map <LoadObject*, int> *loadObjsMap;
  Experiment *founder;
  char *name;
  int groupId;
  static int phaseCompareIdx;
};

#endif  /* _EXPGROUP_H */
