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

#ifndef _DERIVEDMETRICS_H
#define _DERIVEDMETRICS_H

#include <stdio.h>
#include "BaseMetric.h"
#include "Metric.h"

class definition;

class DerivedMetrics
{
public:
  DerivedMetrics ();
  ~DerivedMetrics ();
  definition *add_definition (char *_name, char *_username, char *_def);
  int *construct_map (Vector<Metric*> *mitems, BaseMetric::SubType st,
		      char *expr_spec);
  void dump (FILE *dis_file, int verbosity);
  double eval_one_item (definition *def, int *map, double *values);
  int eval (int *map, double *values);
  void fill_dependencies (definition *def, int *vec);
  Vector<definition*> *get_dependencies (definition *def);

  Vector<definition*> *
  get_items ()
  {
    return items;
  }

private:
  Vector<definition*> *items;
};

#endif
