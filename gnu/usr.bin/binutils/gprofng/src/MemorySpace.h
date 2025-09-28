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

#ifndef _MEMORYSPACE_H
#define	_MEMORYSPACE_H

#include <stdio.h>

#include "dbe_structs.h"
#include "vec.h"
#include "Exp_Layout.h"
#include "Histable.h"
#include "Hist_data.h"
#include "Metric.h"
#include "HashMap.h"

class Experiment;
class Expression;
class DataView;
class DbeView;
class MemObj;

class MemObjType_t
{
public:
  MemObjType_t ();
  ~MemObjType_t ();
  int type;
  char *name;
  char *index_expr;
  char *machmodel;
  char mnemonic;
  char *short_description;
  char *long_description;
};

class MemorySpace
{
public:

  MemorySpace (DbeView *_dbev, int subtype);
  ~MemorySpace ();

  void reset (void);

  int
  getMemObjType (void)
  {
    return mstype;
  }

  char *
  getMemObjTypeName (void)
  {
    return msname;
  }

  Expression *
  getMemObjDef (void)
  {
    return msindex_exp;
  }

  // static members, used to define or fetch the various MemorySpaces
  static void get_filter_keywords (Vector <void*> *res);
  static Vector<void*> *getMemObjects (void);
  static void set_MemTabOrder (Vector<int> *);
  static char *mobj_define (char *, char *, char *, char *, char *);
  static char *mobj_delete (char *);
  static MemObjType_t *findMemSpaceByName (const char *mname);
  static MemObjType_t *findMemSpaceByIndex (int index);
  static char pickMnemonic (char *name);
  static Vector<char *> *getMachineModelMemObjs (char *);

private:
  HashMap<uint64_t, MemObj*> *objs;
  int findMemObject (uint64_t indx);
  MemObj *lookupMemObject (Experiment *exp, DataView*, long);
  MemObj *createMemObject (uint64_t, char *moname);

  int mstype;               // type of this memory space
  char *msname;             // name of this memory space
  Expression *msindex_exp;  // index-expression for this memory space
  char *msindex_exp_str;    // string for index-expression
  Hist_data *hist_data_all; // the cached data for mode=Hist_Data::ALL
  uint64_t selected_mo_index; // which page, cacheline, etc.
  int sel_ind;              // index of selected object in list
  DbeView *dbev;
  int phaseIdx;
  uint64_t idx_min;
  uint64_t idx_max;
  MemObj *unk_memobj;
  MemObj *total_memobj;
};

#endif /* _MEMORYSPACE_H */
