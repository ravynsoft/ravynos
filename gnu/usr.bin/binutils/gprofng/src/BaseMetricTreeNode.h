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

#ifndef _BASEMETRICTREENODE_H
#define _BASEMETRICTREENODE_H

#include "BaseMetric.h"

// Unit values
#define UNIT_SECONDS            "SECONDS"
#define UNIT_SECONDS_UNAME      GTXT("secs.")
#define UNIT_BYTES              "BYTES"
#define UNIT_BYTES_UNAME        GTXT("bytes")

// Name values for intermediate parent nodes that aren't defined elsewhere
#define L1_DURATION             "PROFDATA_TYPE_DURATION"
#define L1_DURATION_UNAME       GTXT("Experiment Duration")
#define L1_GCDURATION           "PROFDATA_TYPE_GCDURATION"
#define L1_GCDURATION_UNAME     GTXT("Java Garbage Collection Duration")
#define L2_HWC_DSPACE           "PROFDATA_TYPE_HWC_DSPACE"
#define L2_HWC_DSPACE_UNAME     GTXT("Memoryspace Hardware Counters")
#define L2_HWC_GENERAL          "PROFDATA_TYPE_HWC_GENERAL"
#define L2_HWC_GENERAL_UNAME    GTXT("General Hardware Counters")
#define L1_MPI_STATES           "PROFDATA_TYPE_MPI_STATES"
#define L1_MPI_STATES_UNAME     GTXT("MPI States")
#define L1_OTHER                "PROFDATA_TYPE_OTHER"
#define L1_OTHER_UNAME          GTXT("Derived and Other Metrics")
#define L1_STATIC               "PROFDATA_TYPE_STATIC"
#define L1_STATIC_UNAME         GTXT("Static")
#define L_CP_TOTAL              "L_CP_TOTAL"
#define L_CP_TOTAL_CPU          "L_CP_TOTAL_CPU"

class BaseMetricTreeNode
{
public:
  BaseMetricTreeNode (); // builds basic metric tree (not including HWCs)
  virtual ~BaseMetricTreeNode ();
  BaseMetricTreeNode *register_metric (BaseMetric *item);
  BaseMetricTreeNode *find (const char *name);
  void get_nearest_registered_descendents (Vector<BaseMetricTreeNode*> *new_vec);
  void get_all_registered_descendents (Vector<BaseMetricTreeNode*> *new_vec);
  char *get_description();
  char *dump();

  BaseMetricTreeNode *get_root ()       { return root; }
  BaseMetricTreeNode *get_parent ()     { return parent; }
  Vector<BaseMetricTreeNode*> *get_children () { return children; }
  bool is_registered ()                 { return registered; }
  int get_num_registered_descendents () { return num_registered_descendents; }
  bool is_composite_metric ()           { return isCompositeMetric; }
  BaseMetric *get_BaseMetric ()         { return bm; }
  char *get_name ()                     { return name; }
  char *get_user_name ()                { return uname; }
  char *get_unit ()                     { return unit; }
  char *get_unit_uname ()               { return unit_uname; }

private:
  BaseMetricTreeNode (BaseMetric *item);
  BaseMetricTreeNode (const char *name, const char *uname,
		      const char *_unit, const char *_unit_uname);
  void init_vars ();
  void build_basic_tree ();
  BaseMetricTreeNode *add_child (BaseMetric *item);
  BaseMetricTreeNode *add_child (const char *name, const char *uname,
				  const char *unit = NULL, const char *unit_uname = NULL);
  BaseMetricTreeNode *add_child (BaseMetricTreeNode *new_node);
  void register_node (BaseMetricTreeNode *);

  BaseMetricTreeNode *root;     // root of tree
  BaseMetricTreeNode *parent;   // my parent
  bool aggregation;             // value is based on children's values
  char *name;           // bm->get_cmd() for metrics, unique string otherwise
  char *uname;                  // user-visible text
  char *unit;                   // see UNIT_* defines
  char *unit_uname;             // see UNIT_*_UNAME defines
  Vector<BaseMetricTreeNode*> *children;    // my children
  bool isCompositeMetric;       // value is sum of children
  BaseMetric *bm;               // metric for this node, or null
  bool registered;              // metric has been officially registered
  int num_registered_descendents;   // does not include self
};

#endif  /* _BASEMETRICTREENODE_H */
