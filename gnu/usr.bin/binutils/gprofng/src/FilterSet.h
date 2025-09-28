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

#ifndef _FILTERSET_H
#define _FILTERSET_H

#include "dbe_types.h"
#include "vec.h"

class Experiment;
class FilterNumeric;
class DbeView;

#define SAMPLE_FILTER_IDX 0
#define THREAD_FILTER_IDX 1
#define LWP_FILTER_IDX 2
#define CPU_FILTER_IDX 3

class FilterSet
{
public:

  FilterSet (DbeView *_dbev, Experiment *_exp);
  ~FilterSet ();
  char *get_advanced_filter ();
  FilterNumeric *get_filter (int);

  bool
  get_enabled ()
  {
    return enbl;
  }

  void
  set_enabled (bool b)
  {
    enbl = b;
  }

  Vector<FilterNumeric*> *
  get_all_filters ()
  {
    return dfilter;
  }

private:

  DbeView *dbev;
  Experiment *exp;
  bool enbl;
  Vector<FilterNumeric*> *dfilter;
};

#endif /* _FILTERSET_H */

