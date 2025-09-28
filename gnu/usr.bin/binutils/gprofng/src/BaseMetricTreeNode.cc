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
#include <stdio.h>
#include <strings.h>
#include <limits.h>
#include <sys/param.h>

#include "hwcentry.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "Expression.h"
#include "Metric.h"
#include "Table.h"
#include "i18n.h"
#include "debug.h"

BaseMetricTreeNode::BaseMetricTreeNode ()
{
  init_vars ();
  build_basic_tree ();
}

BaseMetricTreeNode::BaseMetricTreeNode (BaseMetric *item)
{
  init_vars ();
  bm = item;
  name = dbe_strdup (bm->get_cmd ());
  uname = dbe_strdup (bm->get_username ());
  unit = NULL; //YXXX populate from base_metric (requires updating base_metric)
  unit_uname = NULL;
}

BaseMetricTreeNode::BaseMetricTreeNode (const char *_name, const char *_uname,
				    const char *_unit, const char *_unit_uname)
{
  init_vars ();
  name = dbe_strdup (_name);
  uname = dbe_strdup (_uname);
  unit = dbe_strdup (_unit);
  unit_uname = dbe_strdup (_unit_uname);
}

void
BaseMetricTreeNode::init_vars ()
{
  name = NULL;
  uname = NULL;
  unit = NULL;
  unit_uname = NULL;
  root = this;
  parent = NULL;
  children = new Vector<BaseMetricTreeNode*>;
  isCompositeMetric = false;
  bm = NULL;
  registered = false;
  num_registered_descendents = 0;
}

BaseMetricTreeNode::~BaseMetricTreeNode ()
{
  children->destroy ();
  delete children;
  free (name);
  free (uname);
  free (unit);
  free (unit_uname);
}

BaseMetricTreeNode *
BaseMetricTreeNode::register_metric (BaseMetric *item)
{
  BaseMetricTreeNode *found = root->find (item->get_cmd ());
  if (!found)
    {
      switch (item->get_type ())
	{
	case BaseMetric::CP_TOTAL:
	  found = root->find (L_CP_TOTAL);
	  break;
	case BaseMetric::CP_TOTAL_CPU:
	  found = root->find (L_CP_TOTAL_CPU);
	  break;
	}
      if (found && found->bm == NULL)
	found->bm = item;
    }
  if (!found)
    {
      switch (item->get_type ())
	{
	case BaseMetric::HEAP_ALLOC_BYTES:
	case BaseMetric::HEAP_ALLOC_CNT:
	case BaseMetric::HEAP_LEAK_BYTES:
	case BaseMetric::HEAP_LEAK_CNT:
	  found = root->find (get_prof_data_type_name (DATA_HEAP));
	  break;
	case BaseMetric::CP_KERNEL_CPU:
	case BaseMetric::CP_TOTAL:
	  found = root->find (get_prof_data_type_name (DATA_CLOCK));
	  break;
	case BaseMetric::CP_LMS_DFAULT:
	case BaseMetric::CP_LMS_TFAULT:
	case BaseMetric::CP_LMS_KFAULT:
	case BaseMetric::CP_LMS_STOPPED:
	case BaseMetric::CP_LMS_WAIT_CPU:
	case BaseMetric::CP_LMS_SLEEP:
	case BaseMetric::CP_LMS_USER_LOCK:
	case BaseMetric::CP_TOTAL_CPU:
	  found = root->find (L_CP_TOTAL);
	  break;
	case BaseMetric::CP_LMS_USER:
	case BaseMetric::CP_LMS_SYSTEM:
	case BaseMetric::CP_LMS_TRAP:
	  found = root->find (L_CP_TOTAL_CPU);
	  break;
	case BaseMetric::HWCNTR:
	  found = root->find ((item->get_flavors () & BaseMetric::DATASPACE) != 0 ?
			      L2_HWC_DSPACE : L2_HWC_GENERAL);
	  break;
	case BaseMetric::SYNC_WAIT_TIME:
	case BaseMetric::SYNC_WAIT_COUNT:
	  found = root->find (get_prof_data_type_name (DATA_SYNCH));
	  break;
	case BaseMetric::OMP_WORK:
	case BaseMetric::OMP_WAIT:
	case BaseMetric::OMP_OVHD:
	  found = root->find (get_prof_data_type_name (DATA_OMP));
	  break;
	case BaseMetric::IO_READ_TIME:
	case BaseMetric::IO_READ_BYTES:
	case BaseMetric::IO_READ_CNT:
	case BaseMetric::IO_WRITE_TIME:
	case BaseMetric::IO_WRITE_BYTES:
	case BaseMetric::IO_WRITE_CNT:
	case BaseMetric::IO_OTHER_TIME:
	case BaseMetric::IO_OTHER_CNT:
	case BaseMetric::IO_ERROR_TIME:
	case BaseMetric::IO_ERROR_CNT:
	  found = root->find (get_prof_data_type_name (DATA_IOTRACE));
	  break;
	case BaseMetric::ONAME:
	case BaseMetric::SIZES:
	case BaseMetric::ADDRESS:
	  found = root->find (L1_STATIC);
	  break;
	default:
	  found = root->find (L1_OTHER);
	  break;
	}
      assert (found != NULL);
      switch (item->get_type ())
	{
	case BaseMetric::CP_TOTAL:
	case BaseMetric::CP_TOTAL_CPU:
	  found->isCompositeMetric = true;
	  break;
	}
      found = found->add_child (item);
    }
  register_node (found);
  return found;
}

void
BaseMetricTreeNode::register_node (BaseMetricTreeNode *node)
{
  if (!node->registered)
    {
      node->registered = true;
      BaseMetricTreeNode *tmp = node->parent;
      while (tmp)
	{
	  tmp->num_registered_descendents++;
	  tmp = tmp->parent;
	}
    }
}

BaseMetricTreeNode *
BaseMetricTreeNode::find (const char *_name)
{
  BaseMetricTreeNode *found = NULL;
  if (dbe_strcmp (get_name (), _name) == 0)
    return this;
  if (bm && dbe_strcmp (bm->get_cmd (), _name) == 0)
    return this;
  BaseMetricTreeNode *child;
  int index;

  Vec_loop (BaseMetricTreeNode*, children, index, child)
  {
    found = child->find (_name);
    if (found)
      return found;
  }
  return NULL;
}

static void
int_get_registered_descendents (BaseMetricTreeNode* curr,
			  Vector<BaseMetricTreeNode*> *dest, bool nearest_only)
{
  if (!curr)
    return;
  if (curr->is_registered ())
    {
      dest->append (curr);
      if (nearest_only)
	return; // soon as we hit a live node, stop following branch
    }
  int index;
  BaseMetricTreeNode *child;

  Vec_loop (BaseMetricTreeNode*, curr->get_children (), index, child)
  {
    int_get_registered_descendents (child, dest, nearest_only);
  }
}

void
BaseMetricTreeNode::get_nearest_registered_descendents (Vector<BaseMetricTreeNode*> *dest)
{
  if (!dest || dest->size () != 0)
    abort ();
  bool nearest_only = true;
  int_get_registered_descendents (this, dest, nearest_only);
}

void
BaseMetricTreeNode::get_all_registered_descendents (Vector<BaseMetricTreeNode*> *dest)
{
  if (!dest || dest->size () != 0)
    abort ();
  bool nearest_only = false;
  int_get_registered_descendents (this, dest, nearest_only);
}

char *
BaseMetricTreeNode::get_description ()
{
  if (bm)
    {
      Hwcentry* hw_ctr = bm->get_hw_ctr ();
      if (hw_ctr)
	return hw_ctr->short_desc;
    }
  return NULL;
}

void
BaseMetricTreeNode::build_basic_tree ()
{
#define TREE_INSERT_DATA_TYPE(t) add_child(get_prof_data_type_name (t), get_prof_data_type_uname (t))
  BaseMetricTreeNode *level1, *level2;
  // register L1_DURATION here because it has a value but is not a true metric
  register_node (add_child (L1_DURATION, L1_DURATION_UNAME, UNIT_SECONDS,
			    UNIT_SECONDS_UNAME));
  register_node (add_child (L1_GCDURATION, L1_GCDURATION_UNAME, UNIT_SECONDS,
			    UNIT_SECONDS_UNAME));
  TREE_INSERT_DATA_TYPE (DATA_HEAP);
  level1 = TREE_INSERT_DATA_TYPE (DATA_CLOCK);
  level1 = level1->add_child (L_CP_TOTAL, GTXT ("XXX Total Thread Time"));
  level1->isCompositeMetric = true;
  level2 = level1->add_child (L_CP_TOTAL_CPU, GTXT ("XXX Total CPU Time"));
  level2->isCompositeMetric = true;

  add_child (L1_OTHER, L1_OTHER_UNAME);
  level1 = TREE_INSERT_DATA_TYPE (DATA_HWC);
  level1->add_child (L2_HWC_DSPACE, L2_HWC_DSPACE_UNAME);
  level1->add_child (L2_HWC_GENERAL, L2_HWC_GENERAL_UNAME);
  TREE_INSERT_DATA_TYPE (DATA_SYNCH);
  TREE_INSERT_DATA_TYPE (DATA_OMP);
  TREE_INSERT_DATA_TYPE (DATA_IOTRACE);
  add_child (L1_STATIC, L1_STATIC_UNAME);
}

BaseMetricTreeNode *
BaseMetricTreeNode::add_child (BaseMetric *item)
{
  return add_child (new BaseMetricTreeNode (item));
}

BaseMetricTreeNode *
BaseMetricTreeNode::add_child (const char * _name, const char *_uname,
			       const char * _unit, const char * _unit_uname)
{
  return add_child (new BaseMetricTreeNode (_name, _uname, _unit, _unit_uname));
}

BaseMetricTreeNode *
BaseMetricTreeNode::add_child (BaseMetricTreeNode *new_node)
{
  new_node->parent = this;
  new_node->root = root;
  children->append (new_node);
  return new_node;
}

char *
BaseMetricTreeNode::dump ()
{
  int len = 4;
  char *s = bm ? bm->dump () : dbe_strdup ("<no base metric>");
  char *msg = dbe_sprintf ("%s\n%*c %*c unit='%s' unit_uname='%s' uname='%s' name='%s'\n",
			   STR (s), len, ' ', len, ' ',
			   STR (get_unit_uname ()), STR (get_unit ()),
			   STR (get_user_name ()), STR (get_name ()));
  free (s);
  return msg;
}
