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

#include "config.h"
#include <strings.h>
#include "DerivedMetrics.h"
#include "util.h"

enum opType
{
  opNULL,
  opPrimitive,
  opDivide
};

class definition
{
public:
  definition();
  ~definition();
  char *name;
  char *def;
  opType op;
  definition *arg1;
  definition *arg2;
  int index;
};

definition::definition ()
{
  name = def = NULL;
  arg1 = arg2 = NULL;
}

definition::~definition ()
{
  free (name);
  free (def);
}

DerivedMetrics::DerivedMetrics ()
{
  items = new Vector<definition*>;
}

DerivedMetrics::~DerivedMetrics ()
{
  Destroy (items);
}

definition *
DerivedMetrics::add_definition (char *_name, char *_username, char *_def)
{
  definition *p;

  // if the name doesn't matter, maybe there is a duplicate we can use
  if (_name == NULL)
    {
      int i;
      Vec_loop (definition*, items, i, p)
      {
	if (strcmp (p->def, _def) == 0)
	  return p;
      }
    }

  p = new definition;
  p->name = dbe_strdup (_name);
  p->def = dbe_strdup (_def);

  // parse the definition
  if (strchr (_def, '/') == NULL)
    {
      // it's a primitive metric
      p->op = opPrimitive;
      p->arg1 = p->arg2 = NULL;

    }
  else
    {
      // it's some operation on arguments
      p->op = opDivide;
      char *op_ptr = strchr (p->def, '/');
      *op_ptr = 0;
      p->arg1 = add_definition (NULL, NULL, p->def);
      *op_ptr = '/';
      p->arg2 = add_definition (NULL, NULL, op_ptr + 1);
    }
  p->index = items->size ();
  items->append (p);
  return p;
}

int *
DerivedMetrics::construct_map (Vector<Metric*> *mitems, BaseMetric::SubType st, char *expr_spec)
{
  if (items == NULL)
    return NULL;
  int ndm = items->size ();
  if (ndm == 0)
    return NULL;
  int nmetrics = mitems->size ();

  // allocate arrays for the mapping between derived metrics and requested values
  int *map = (int *) malloc (ndm * sizeof (int));

  // map derived metrics to requested metrics    // EUGENE explain this more clearly
  //   0  means not mapped
  //  >0  means primitive metric maps to map-1
  //  <0  means  derived  metric maps to 1-map
  int ndm_requested = 0;
  for (int idm = 0; idm < ndm; idm++)
    {
      definition *defdm = items->fetch (idm);
      map[idm] = 0;

      // figure out what name to use for this derived metric
      char *dname;
      if (defdm->op == opPrimitive)
	dname = defdm->def;
      else
	{
	  dname = defdm->name;
	  if (dname == NULL) break;
	}

      // look for this name among metrics
      int im;
      for (im = 0; im < nmetrics; im++)
	{
	  Metric *m = mitems->fetch (im);
	  if (strcmp (dname, m->get_cmd ()) == 0 && m->get_subtype () == st)
	    // apparent match, but let's check comparison mode
	    if (dbe_strcmp (expr_spec, m->get_expr_spec ()) == 0)
	      break;
	}

      // encode the mapping
      if (im >= nmetrics)
	map[idm] = 0; // does not map to requested metrics
      else if (defdm->op == opPrimitive)
	map[idm] = +1 + im; // encode as a positive index
      else
	{
	  map[idm] = -1 - im; // encode as a negative index
	  ndm_requested++;
	}
    }
  if (ndm_requested == 0)
    {
      free (map);
      map = NULL;
    }
  return map;
}

void
DerivedMetrics::fill_dependencies (definition *def, int *vec)
{
  switch (def->op)
    {
    case opPrimitive:
      vec[def->index] = 1;
      break;
    case opDivide:
      fill_dependencies (def->arg1, vec);
      fill_dependencies (def->arg2, vec);
      break;
    default:
      break;
    }
}

Vector<definition*> *
DerivedMetrics::get_dependencies (definition *def)
{
  int n = items->size ();

  // zero out a vector representing definitions
  int *vec = (int *) malloc (n * sizeof (int));
  for (int i = 0; i < n; i++)
    vec[i] = 0;
  fill_dependencies (def, vec);

  // construct the dependency vector
  Vector<definition*> *dependencies = new Vector<definition*>;
  for (int i = 0; i < n; i++)
    if (vec[i] == 1)
      dependencies->append (items->fetch (i));
  free (vec);
  return dependencies;
}

void
DerivedMetrics::dump (FILE *dis_file, int verbosity)
{
  int i;
  definition *item;

  // deal with the possibility that names might be NULL
  const char *UNNAMED = "(unnamed)";
#define NAME(x) ( (x) ? (x) : UNNAMED)

  Vec_loop (definition*, items, i, item)
  {
    // at low verbosity, skip over some items
    if (verbosity == 0)
      {
	if (item->name == NULL)
	  continue;
	if (strcmp (item->name, item->def) && item->op == opPrimitive)
	  continue;
      }

    // dump the definition
    switch (item->op)
      {
      case opPrimitive:
	fprintf (dis_file, "%s [%s] is a primitive metric\n", NAME (item->name),
		 item->def);
	break;
      case opDivide:
	fprintf (dis_file, "%s [%s] = %s [%s] / %s [%s]\n", NAME (item->name),
		 item->def, NAME (item->arg1->name), item->arg1->def,
		 NAME (item->arg2->name), item->arg2->def);
	break;
      default:
	fprintf (dis_file, "%s [%s] has an unrecognized op %d\n",
		 NAME (item->name), item->def, item->op);
	break;
      }
  }
}

double
DerivedMetrics::eval_one_item (definition *def, int *map, double *values)
{
  switch (def->op)
    {
    case opNULL:
      fprintf (stderr, GTXT ("cannot eval NULL expression\n"));
      return 0.;
    case opPrimitive:
      {
	int ival = map[def->index];
	if (ival <= 0) return 0.;
	ival--;
	return values[ival];
      }
    case opDivide:
      {
	double x1 = eval_one_item (def->arg1, map, values);
	double x2 = eval_one_item (def->arg2, map, values);
	if (x2 == 0) return 0.;
	return (x1 / x2);
      }
    default:
      fprintf (stderr, GTXT ("unknown expression\n"));
      return 0.;
    }
}

int
DerivedMetrics::eval (int *map, double *values)
{
  for (int i = 0, n = items->size (); i < n; i++)
    {
      if (map[i] < 0)
	{
	  int ival = -1 - map[i];
	  values[ival] = eval_one_item (items->fetch (i), map, values);
	}
    }
  return 0;
}

