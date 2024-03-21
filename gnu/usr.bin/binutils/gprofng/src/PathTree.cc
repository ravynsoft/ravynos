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
#include <stdlib.h>

#include "util.h"
#include "DefaultMap.h"
#include "CacheMap.h"

#include "DbeSession.h"
#include "Application.h"
#include "CallStack.h"
#include "Emsg.h"
#include "Experiment.h"
#include "Expression.h"
#include "Function.h"
#include "Histable.h"
#include "IndexObject.h"
#include "MetricList.h"
#include "Module.h"
#include "DbeView.h"
#include "Metric.h"
#include "PathTree.h"
#include "LoadObject.h"
#include "Sample.h"
#include "StringBuilder.h"
#include "Table.h"

// Define counts, rate for error warnings for statistical profiles
#define MIN_PROF_CNT    100
#define MAX_PROF_RATE   1000.

#define NUM_DESCENDANTS(nd) ((nd)->descendants ? (nd)->descendants->size() : 0)
#define IS_LEAF(nd)         ((nd)->descendants == NULL)

#ifdef DEBUG
#define DBG(__func) __func
#else
#define DBG(__func)
#endif

void
PathTree::construct (DbeView *_dbev, int _indxtype, PathTreeType _pathTreeType)
{
  dbev = _dbev;
  indxtype = _indxtype;
  pathTreeType = _pathTreeType;
  status = 0;
  nchunks = 0;
  chunks = NULL;
  nodes = 1; // don't use node 0
  nslots = 0;
  slots = NULL;
  root_idx = 0;
  root = NULL;
  depth = 1;
  dnodes = 0;
  phaseIdx = -1;
  nexps = 0;
  total_obj = NULL;
  indx_expr = NULL;
  statsq = NULL;
  warningq = NULL;
  cancel_ok = 1;
  ptree_internal = NULL;
  ftree_internal = NULL;
  ftree_needs_update = false;
  depth_map = NULL;
  init ();
}

PathTree::~PathTree ()
{
  fini ();
  for (long i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
}

void
PathTree::init ()
{
  fn_map = new DefaultMap<Function*, NodeIdx>;
  stack_prop = PROP_NONE;
  desc_htable_size = 511;
  desc_htable_nelem = 0;
  descHT = new hash_node_t*[desc_htable_size];
  for (int i = 0; i < desc_htable_size; i++)
    descHT[i] = NULL;
  pathMap = new CacheMap<uint64_t, NodeIdx>;
  statsq = new Emsgqueue (NTXT ("statsq"));
  warningq = new Emsgqueue (NTXT ("warningq"));
  if (indxtype < 0)
    {
      Function *ftotal = dbeSession->get_Total_Function ();
      if (pathTreeType == PATHTREE_INTERNAL_FUNCTREE)
	total_obj = ftotal;
      else
	total_obj = ftotal->find_dbeinstr (0, 0);
      VMode view_mode = dbev->get_view_mode ();
      if (view_mode == VMODE_MACHINE)
	stack_prop = PROP_MSTACK;
      else if (view_mode == VMODE_EXPERT)
	stack_prop = PROP_XSTACK;
      else if (view_mode == VMODE_USER)
	{
	  stack_prop = PROP_USTACK;
	  if (dbeSession->is_omp_available ()
	      && pathTreeType == PATHTREE_INTERNAL_OMP)
	    stack_prop = PROP_XSTACK;
	}
    }
  else
    {
      total_obj = new IndexObject (indxtype, (uint64_t) - 2);
      total_obj->set_name (dbe_strdup (NTXT ("<Total>")));
      char *idxname = dbeSession->getIndexSpaceName (indxtype);
      if (streq (idxname, NTXT ("OMP_preg")))
	stack_prop = PROP_CPRID;
      else if (streq (idxname, NTXT ("OMP_task")))
	stack_prop = PROP_TSKID;
      else
	indx_expr = dbeSession->getIndexSpaceExpr (indxtype);
    }
  root_idx = new_Node (0, total_obj, false);
  root = NODE_IDX (root_idx);
}

void
PathTree::fini ()
{
  // For each node free its descendants vector
  // and reset the node list of its function
  for (long i = 1; i < nodes; i++)
    {
      Node *node = NODE_IDX (i);
      if (node->descendants)
	delete node->descendants;
    }
  nodes = 1; // don't use node 0

  for (int i = 0; i < nslots; i++)
    {
      int **tmp = slots[i].mvals;
      for (long j = 0; j < nchunks; j++)
	delete[] tmp[j];
      delete[] tmp;
    }
  delete[] slots;
  slots = NULL;
  nslots = 0;
  delete fn_map;
  fn_map = NULL;
  delete pathMap;
  pathMap = NULL;
  destroy (depth_map);
  depth_map = NULL;
  if (indxtype >= 0)
    delete total_obj;

  for (int i = 0; i < desc_htable_size; i++)
    {
      hash_node_t *p = descHT[i];
      while (p)
	{
	  hash_node_t *p1 = p;
	  p = p->next;
	  delete p1;
	}
    }
  delete[] descHT;
  delete statsq;
  delete warningq;
  depth = 1;
  dnodes = 0;
  phaseIdx = -1;
  nexps = 0;
  status = 0;
}

PtreePhaseStatus
PathTree::reset ()
{
  if (pathTreeType == PATHTREE_INTERNAL_FUNCTREE)
    return NORMAL; // never process reset for ftree_internal.

  if (dbeSession->is_omp_available () && dbev->get_view_mode () == VMODE_USER
      && pathTreeType == PATHTREE_MAIN && ptree_internal == NULL)
    ptree_internal = new PathTree (dbev, indxtype, PATHTREE_INTERNAL_OMP);

  if (phaseIdx != dbev->getPhaseIdx ())
    {
      fini ();
      init ();
      phaseIdx = dbev->getPhaseIdx ();
      ftree_needs_update = true;
    }
  for (; nexps < dbeSession->nexps (); nexps++)
    {
      ftree_needs_update = true;
      if (add_experiment (nexps) == CANCELED)
	return CANCELED;
    }

  // LIBRARY_VISIBILITY
  if (dbev->isNewViewMode ())
    dbev->resetNewViewMode ();
  if (dbev->isShowHideChanged ())
    dbev->resetShowHideChanged ();
  return NORMAL;
}

int
PathTree::allocate_slot (int id, ValueTag vtype)
{

  int i;
  int slot_idx = find_slot (id);
  if (slot_idx >= 0)
    {
      DBG (assert (slots[slot_idx].vtype == vtype));
      return slot_idx;
    }
  slot_idx = nslots++;

  Slot *old_slots = slots;
  slots = new Slot[nslots];
  for (i = 0; i < slot_idx; i++)
    slots[i] = old_slots[i];
  delete[] old_slots;

  slots[slot_idx].id = id;
  slots[slot_idx].vtype = vtype;
  int **ip = new int*[nchunks];
  for (i = 0; i < nchunks; i++)
    ip[i] = NULL;
  slots[slot_idx].mvals = ip;

  return slot_idx;
}

void
PathTree::allocate_slots (Slot *new_slots, int new_nslots)
{
  // duplicates new_slots

  // if previously had more slots than currently requested, delete the data from those slots.
  for (int i = new_nslots; i < nslots; i++)
    {
      int **tmp = slots[i].mvals;
      for (long j = 0; j < nchunks; j++)
	delete tmp[j];
      delete tmp;
    }
  if (new_nslots == 0)
    {
      nslots = new_nslots;
      delete[] slots;
      slots = NULL;
      return;
    }

  Slot *old_slots = slots;
  slots = new Slot[new_nslots];
  for (int i = 0; i < new_nslots; i++)
    {
      slots[i] = new_slots[i]; // pick up id and vtype
      if (i < nslots)
	slots[i].mvals = old_slots[i].mvals;
      else
	{
	  if (nchunks == 0)
	    slots[i].mvals = NULL;
	  else
	    {
	      int **ip = new int*[nchunks];
	      for (long j = 0; j < nchunks; j++)
		ip[j] = NULL;
	      slots[i].mvals = ip;
	    }
	}
    }
  nslots = new_nslots;
  delete old_slots;
}

int
PathTree::find_slot (int id)
{
  for (int i = 0; i < nslots; i++)
    if (slots[i].id == id)
      return i;
  return -1;
}

PathTree::NodeIdx
PathTree::new_Node (NodeIdx anc, Histable *instr, bool leaf)
{
  if (nodes >= nchunks * CHUNKSZ)
    {
      long idx = nchunks++;

      // Reallocate Node chunk array
      Node **old_chunks = chunks;
      chunks = new Node*[nchunks];
      for (long k = 0; k < idx; k++)
	chunks[k] = old_chunks[k];
      delete[] old_chunks;

      // Reallocate metric value chunk arrays.
      for (int i = 0; i < nslots; i++)
	{
	  int **mvals = new int*[nchunks];
	  for (long k = 0; k < idx; k++)
	    {
	      mvals[k] = slots[i].mvals[k];
	    }
	  delete[] slots[i].mvals;
	  slots[i].mvals = mvals;
	  slots[i].mvals[idx] = NULL;
	}

      // Allocate new chunk for nodes.
      // Note that we don't need to allocate new chunks
      // for metric values at this point as we rely on
      // lazy allocation.
      //
      allocate_chunk (chunks, idx);
    }
  NodeIdx node_idx = nodes++;
  Node *node = NODE_IDX (node_idx);
  node->ancestor = anc;
  node->descendants = leaf ? (Vector<NodeIdx>*)NULL : new Vector<NodeIdx>(2);
  node->instr = instr;
  Function *func = (Function*) (instr->convertto (Histable::FUNCTION));
  node->funclist = fn_map->get (func);
  fn_map->put (func, node_idx);
  return node_idx;
}

PathTree::NodeIdx
PathTree::find_path (Experiment *exp, DataView *dview, long recIdx)
{
  if (indx_expr != NULL)
    {
      Expression::Context ctx (dbev, exp, dview, recIdx);
      uint64_t idx = indx_expr->eval (&ctx);
      Histable *cur_obj = dbeSession->createIndexObject (indxtype, idx);
      cur_obj->set_name_from_context (&ctx);
      NodeIdx dsc_idx = find_in_desc_htable (root_idx, cur_obj, true);
      depth = 2;
      return dsc_idx;
    }

  bool showAll = dbev->isShowAll ();
  int t_stack_prop = stack_prop;
  void *stackId = dview->getObjValue (t_stack_prop, recIdx);
  NodeIdx node_idx;
  if (stackId != NULL)
    {
      // pathMap does not work with NULL key
      node_idx = pathMap->get ((uint64_t) stackId);
      if (node_idx != 0)
	return node_idx;
    }
  Vector<Histable*> *stack = (Vector<Histable*>*)CallStack::getStackPCs (stackId, !showAll);
  int stack_size = stack->size ();
  if (stack_size == 0)
    return root_idx;

  node_idx = root_idx;
  int thisdepth = 1;

  for (int i = stack_size - 1; i >= 0; i--)
    {
      bool leaf = (i == 0);
      Histable *cur_addr = stack->fetch (i);

      // bail out of loop if load object API-only is set
      // and this is not the top frame
      // This is now done in HSTACK if hide is set

      Function *func = (Function*) cur_addr->convertto (Histable::FUNCTION);
      if (func != NULL)
	{
	  Module *mod = func->module;
	  LoadObject *lo = mod->loadobject;
	  int segx = lo->seg_idx;
	  if (showAll && dbev->get_lo_expand (segx) == LIBEX_API
	      && i != stack_size - 1)
	    leaf = true;
	}

      NodeIdx dsc_idx = find_desc_node (node_idx, cur_addr, leaf);
      thisdepth++;
      node_idx = dsc_idx;

      // LIBEX_API processing might have set leaf to true
      if (leaf)
	break;
    }
  if (thisdepth > depth)
    depth = thisdepth;
  delete stack;
  pathMap->put ((uint64_t) stackId, node_idx);
  return node_idx;
}

static int
desc_node_comp (const void *s1, const void *s2, const void *ptree)
{
  PathTree::NodeIdx t1, t2;
  t1 = *(PathTree::NodeIdx *)s1;
  t2 = *(PathTree::NodeIdx *)s2;
  PathTree* Ptree = (PathTree *) ptree;
  PathTree::Node *n1 = Ptree->NODE_IDX (t1);
  PathTree::Node *n2 = Ptree->NODE_IDX (t2);
  Histable *d1 = n1->instr;
  Histable *d2 = n2->instr;
  if (d1->id < d2->id)
    return -1;
  else if (d1->id > d2->id)
    return +1;
  else
    return 0;
}

PathTree::NodeIdx
PathTree::find_in_desc_htable (NodeIdx node_idx, Histable *instr, bool leaf)
{
  unsigned int hash_code = (unsigned int) instr->id % desc_htable_size;
  Node *node = NODE_IDX (node_idx);
  hash_node_t *p = NULL;
  for (p = descHT[hash_code]; p; p = p->next)
    {
      Node *dsc = NODE_IDX (p->nd);
      Histable *dinstr = dsc->instr;
      if (dinstr->id == instr->id && leaf == IS_LEAF (dsc))
	return p->nd;
    }
  // Not found
  NodeIdx dsc_idx = new_Node (node_idx, instr, leaf);
  node->descendants->append (dsc_idx);
  p = new hash_node_t ();
  p->nd = dsc_idx;
  p->next = descHT[hash_code];
  descHT[hash_code] = p;
  desc_htable_nelem++;

  // time to resize
  if (desc_htable_nelem == desc_htable_size)
    {
      int old_htable_size = desc_htable_size;
      desc_htable_size = old_htable_size * 2 + 1;
      hash_node_t **old_htable = descHT;
      descHT = new hash_node_t*[desc_htable_size];
      for (int i = 0; i < desc_htable_size; i++)
	descHT[i] = NULL;

      for (int i = 0; i < old_htable_size; i++)
	if (old_htable[i] != NULL)
	  {
	    hash_node *old_p;
	    hash_node_t *hash_p = old_htable[i];
	    while (hash_p != NULL)
	      {
		hash_node_t *new_p = new hash_node_t ();
		new_p->nd = hash_p->nd;
		Node *dnode = NODE_IDX (hash_p->nd);
		Histable *dnode_instr = dnode->instr;
		hash_code = (unsigned int) dnode_instr->id % desc_htable_size;
		new_p->next = descHT[hash_code];
		descHT[hash_code] = new_p;
		old_p = hash_p;
		hash_p = hash_p->next;
		delete old_p;
	      }
	  }
      delete[] old_htable;
    }
  return dsc_idx;
}

PathTree::NodeIdx
PathTree::find_desc_node (NodeIdx node_idx, Histable *instr, bool leaf)
{
  // Binary search. All nodes are ordered by Histable::id.

  // We have a special case when two nodes with the same
  //	id value may co-exist: one representing a leaf node and
  //	another one representing a call site.
  Node *node = NODE_IDX (node_idx);
  int left = 0;
  int right = NUM_DESCENDANTS (node) - 1;
  while (left <= right)
    {
      int index = (left + right) / 2;
      NodeIdx dsc_idx = node->descendants->fetch (index);
      Node *dsc = NODE_IDX (dsc_idx);
      Histable *dinstr = dsc->instr;
      if (instr->id < dinstr->id)
	right = index - 1;
      else if (instr->id > dinstr->id)
	left = index + 1;
      else if (leaf == IS_LEAF (dsc))
	return dsc_idx;
      else if (leaf)
	right = index - 1;
      else
	left = index + 1;
    }

  // None was found. Create one.
  NodeIdx dsc_idx = new_Node (node_idx, instr, leaf);
  node->descendants->insert (left, dsc_idx);
  return dsc_idx;
}

PtreePhaseStatus
PathTree::process_packets (Experiment *exp, DataView *packets, int data_type)
{
  Expression::Context ctx (dbev, exp);
  char *progress_bar_msg = NULL;
  int progress_bar_percent = -1;

  Vector<BaseMetric*> *mlist = dbev->get_all_reg_metrics ();
  Vector<BaseMetric*> mlist2;
  StringBuilder stb;
  for (int midx = 0, mlist_sz = mlist->size (); midx < mlist_sz; ++midx)
    {
      BaseMetric *mtr = mlist->fetch (midx);
      if (mtr->get_packet_type () == data_type &&
	  (mtr->get_expr () == NULL || mtr->get_expr ()->passes (&ctx)))
	{
	  Hwcentry *hwc = mtr->get_hw_ctr ();
	  if (hwc)
	    {
	      stb.setLength (0);
	      // XXX this should be done at metric registration
	      Collection_params *col_params = exp->get_params ();
	      for (int i = 0; i < MAX_HWCOUNT; i++)
		{
		  // We may have duplicate counters in col_params,
		  // check for all (see 5081284).
		  if (dbe_strcmp (hwc->name, col_params->hw_aux_name[i]) == 0)
		    {
		      if (stb.length () != 0)
			stb.append (NTXT ("||"));
		      stb.append (NTXT ("HWCTAG=="));
		      stb.append (i);
		    }
		}
	      if (stb.length () == 0)
		continue;
	      stb.append (NTXT ("&& ((HWCINT & "));
	      stb.append ((long long) HWCVAL_ERR_FLAG);
	      stb.append (NTXT (")==0)"));
	      char *s = stb.toString ();
	      mtr->set_cond_spec (s);
	      free (s);
	    }
	  ValueTag vtype = mtr->get_vtype ();
	  switch (vtype)
	    {
	    case VT_INT:
	    case VT_ULLONG:
	    case VT_LLONG:
	      break; // nothing to do
	    default:
	      vtype = VT_ULLONG; // ym: not sure when this would happen
	      break;
	    }
	  allocate_slot (mtr->get_id (), vtype);
	  mlist2.append (mtr);
	}
    }

  Slot **mslots = new Slot*[mlist2.size ()];
  for (int midx = 0, mlist_sz = mlist2.size (); midx < mlist_sz; ++midx)
    {
      BaseMetric *mtr = mlist2.fetch (midx);
      int id = mtr->get_id ();
      int slot_ind = find_slot (id);
      mslots[midx] = SLOT_IDX (slot_ind);
    }

  for (long i = 0, packets_sz = packets->getSize (); i < packets_sz; ++i)
    {
      if (dbeSession->is_interactive ())
	{
	  if (NULL == progress_bar_msg)
	    progress_bar_msg = dbe_sprintf (GTXT ("Processing Experiment: %s"),
					  get_basename (exp->get_expt_name ()));
	  int val = (int) (100 * i / packets_sz);
	  if (val > progress_bar_percent)
	    {
	      progress_bar_percent += 10;
	      if (theApplication->set_progress (val, progress_bar_msg)
		  && cancel_ok)
		{
		  delete[] mslots;
		  return CANCELED;
		}
	    }
	}

      NodeIdx path_idx = 0;
      ctx.put (packets, i);

      for (int midx = 0, mlist_sz = mlist2.size (); midx < mlist_sz; ++midx)
	{
	  BaseMetric *mtr = mlist2.fetch (midx);
	  if (mtr->get_cond () != NULL && !mtr->get_cond ()->passes (&ctx))
	    continue;

	  int64_t mval = mtr->get_val ()->eval (&ctx);
	  if (mval == 0)
	    continue;
	  if (path_idx == 0)
	    path_idx = find_path (exp, packets, i);
	  NodeIdx node_idx = path_idx;
	  Slot *mslot = mslots[midx];
	  while (node_idx)
	    {
	      INCREMENT_METRIC (mslot, node_idx, mval);
	      node_idx = NODE_IDX (node_idx)->ancestor;
	    }
	}
    }
  if (dbeSession->is_interactive ())
    free (progress_bar_msg);
  delete[] mslots;
  if (indx_expr != NULL)
    root->descendants->sort ((CompareFunc) desc_node_comp, this);
  return NORMAL;
}

DataView *
PathTree::get_filtered_events (int exp_index, int data_type)
{
  if (indx_expr != NULL)
    {
      IndexObjType_t *indexObj = dbeSession->getIndexSpace (indxtype);
      if (indexObj->memObj && data_type != DATA_HWC)
	return NULL;
    }
  return dbev->get_filtered_events (exp_index, data_type);
}

PtreePhaseStatus
PathTree::add_experiment (int exp_index)
{
  StringBuilder sb;
  char *expt_name;
  char *base_name;
  Emsg *m;
  Experiment *experiment = dbeSession->get_exp (exp_index);
  if (experiment->broken != 0)
    return NORMAL;
  status = 0;
  expt_name = experiment->get_expt_name ();
  base_name = get_basename (expt_name);

  hrtime_t starttime = gethrtime ();
  hrtime_t startvtime = gethrvtime ();

  // Experiment::getEndTime was initially implemented as
  // returning exp->last_event. To preserve the semantics
  // new Experiment::getLastEvent() is used here.
  hrtime_t tot_time = experiment->getLastEvent () - experiment->getStartTime ();

  if (!dbev->isShowAll () && (dbev->isShowHideChanged ()
			      || dbev->isNewViewMode ()))
    experiment->resetShowHideStack ();

  // To report experiment index to the user,
  // start numeration from 1, not 0
  sb.sprintf (GTXT ("PathTree processing experiment %d (`%s'); duration %lld.%06lld"),
	      exp_index + 1, base_name,
	      tot_time / NANOSEC, (tot_time % NANOSEC / 1000));
  m = new Emsg (CMSG_COMMENT, sb);
  statsq->append (m);

  DataView *prof_packet = get_filtered_events (exp_index, DATA_CLOCK);
  if (prof_packet && prof_packet->getSize () > 0)
    {
      if (process_packets (experiment, prof_packet, DATA_CLOCK) == CANCELED)
	return CANCELED;
      long clock_cnt = prof_packet->getSize ();
      double clock_rate;
      if (tot_time != 0)
	clock_rate = (double) clock_cnt / (double) tot_time * (double) NANOSEC;
      else
	clock_rate = (double) 0.;
      if (experiment->timelineavail)
	sb.sprintf (GTXT ("  Processed %ld clock-profile events (%3.2f/sec.)"),
		    clock_cnt, clock_rate);
      else
	sb.sprintf (GTXT ("  Processed %ld clock-profile events"), clock_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);

      // check for statistical validity
      if ((experiment->timelineavail == true)
	   && !dbev->get_filter_active () && (clock_cnt < MIN_PROF_CNT))
	{
	  sb.sprintf (GTXT ("WARNING: too few clock-profile events (%ld) in experiment %d (`%s') for statistical validity"),
		      clock_cnt, exp_index + 1, base_name);
	  m = new Emsg (CMSG_COMMENT, sb);
	  statsq->append (m);
	}
    }

  DataView *sync_packet = get_filtered_events (exp_index, DATA_SYNCH);
  if (sync_packet && sync_packet->getSize () > 0)
    {
      if (process_packets (experiment, sync_packet, DATA_SYNCH) == CANCELED)
	return CANCELED;
      long sync_cnt = sync_packet->getSize ();
      sb.sprintf (GTXT ("  Processed %ld synctrace events"), sync_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);
    }

  DataView *iotrace_packet = get_filtered_events (exp_index, DATA_IOTRACE);
  if (iotrace_packet && iotrace_packet->getSize () > 0)
    {
      if (process_packets (experiment, iotrace_packet, DATA_IOTRACE) == CANCELED)
	return CANCELED;
      long iotrace_cnt = iotrace_packet->getSize ();
      sb.sprintf (GTXT ("  Processed %ld IO trace events"), iotrace_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);
    }

  DataView *hwc_packet = get_filtered_events (exp_index, DATA_HWC);
  if (hwc_packet && hwc_packet->getSize () > 0)
    {
      if (process_packets (experiment, hwc_packet, DATA_HWC) == CANCELED)
	return CANCELED;
      long hwc_cnt = hwc_packet->getSize ();
      double hwc_rate = (double) hwc_cnt / (double) tot_time * (double) NANOSEC;
      if (experiment->timelineavail)
	sb.sprintf (GTXT ("  Processed %ld hwc-profile events (%3.2f/sec.)"),
		    hwc_cnt, hwc_rate);
      else
	sb.sprintf (GTXT ("  Processed %ld hwc-profile events"), hwc_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);

      // check for statistical validity
      if (experiment->timelineavail && !dbev->get_filter_active () && (hwc_cnt < MIN_PROF_CNT))
	{
	  sb.sprintf (GTXT ("WARNING: too few HW counter profile events (%ld) in experiment %d (`%s') for statistical validity"),
		      hwc_cnt, exp_index + 1, base_name);
	  m = new Emsg (CMSG_COMMENT, sb);
	  statsq->append (m);
	}
    }

  DataView *heap_packet = get_filtered_events (exp_index, DATA_HEAP);
  if (heap_packet && heap_packet->getSize () > 0)
    {
      if (process_packets (experiment, heap_packet, DATA_HEAP) == CANCELED)
	return CANCELED;
      long heap_cnt = heap_packet->getSize ();
      sb.sprintf (GTXT ("  Processed %ld heaptrace events"), heap_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);
    }

  DataView *race_packet = get_filtered_events (exp_index, DATA_RACE);
  if (race_packet && race_packet->getSize () > 0)
    {
      if (process_packets (experiment, race_packet, DATA_RACE) == CANCELED)
	return CANCELED;
      long race_cnt = race_packet->getSize ();
      sb.sprintf (GTXT ("  Processed %ld race access events"), race_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);
    }

  DataView *deadlock_packet = get_filtered_events (exp_index, DATA_DLCK);
  if (deadlock_packet && deadlock_packet->getSize () > 0)
    {
      if (process_packets (experiment, deadlock_packet, DATA_DLCK) == CANCELED)
	return CANCELED;
      long race_cnt = deadlock_packet->getSize ();
      sb.sprintf (GTXT ("  Processed %ld race access events"), race_cnt);
      m = new Emsg (CMSG_COMMENT, sb);
      statsq->append (m);
    }

  hrtime_t pathtime = gethrtime () - starttime;
  hrtime_t pathvtime = gethrvtime () - startvtime;
  sb.sprintf (GTXT ("PathTree time = %lld.%06lld CPU-time %lld.%06lld\n"),
	      pathtime / NANOSEC, (pathtime % NANOSEC) / 1000,
	      pathvtime / NANOSEC, (pathvtime % NANOSEC) / 1000);
  m = new Emsg (CMSG_COMMENT, sb);
  statsq->append (m);
  return NORMAL;
}

Hist_data *
PathTree::compute_metrics (MetricList *mlist, Histable::Type type,
			   Hist_data::Mode mode, Vector<Histable*> *objs,
			   Histable *context, Vector<Histable*> *sel_objs,
			   PtreeComputeOption computeOpt)
{
  VMode view_mode = dbev->get_view_mode ();

  // For displaying disassembly correctly in user mode with openmp
  if (ptree_internal != NULL &&
      (view_mode == VMODE_EXPERT ||
       (view_mode == VMODE_USER && (type == Histable::INSTR
				    || (dbev->isOmpDisMode ()
					&& type == Histable::FUNCTION
					&& mode == Hist_data::CALLEES
					&& computeOpt == COMPUTEOPT_OMP_CALLEE))
				    )))
    return ptree_internal->compute_metrics (mlist, type, mode, objs, context,
					    sel_objs);

  PtreePhaseStatus resetStatus = reset ();

  hist_data = new Hist_data (mlist, type, mode);
  int nmetrics = mlist->get_items ()->size ();
  int sort_ind = -1;
  Hist_data::HistItem *hi;
  int index;

  if (status != 0 || resetStatus == CANCELED)
    return hist_data;

  hist_data->set_status (Hist_data::SUCCESS);
  if (dbeSession->is_interactive () && mode != Hist_data::CALLEES)
    theApplication->set_progress (0, GTXT ("Constructing Metrics"));

  xlate = new int[nmetrics];
  for (int mind = 0; mind < nmetrics; mind++)
    {
      Metric *mtr = mlist->get (mind);
      xlate[mind] = find_slot (mtr->get_id ());
    }

  // Compute dynamic metrics
  obj_list = new Histable*[depth];
  if ((type == Histable::LINE || type == Histable::INSTR)
      && mode == Hist_data::CALLERS)
    node_list = new Node*[depth];
  percent = 0;
  ndone = 0;
  if (mode == Hist_data::MODL)
    {
      Histable *obj = objs && objs->size () > 0 ? objs->fetch (0) : NULL;
      if (obj != NULL)
	{
	  switch (obj->get_type ())
	    {
	    case Histable::FUNCTION:
	      {
		Vector<Function*> *funclist = new Vector<Function*>;
		funclist->append ((Function*) obj);
		get_metrics (funclist, context);
		delete funclist;
		break;
	      }
	    case Histable::MODULE:
	      {
		Vector<Histable*> *comparableModules = obj->get_comparable_objs ();
		if (comparableModules != NULL)
		  {
		    Vector<Function*> *functions = new Vector<Function*>;
		    for (int i = 0; i < comparableModules->size (); i++)
		      {
			Module *mod = (Module*) comparableModules->fetch (i);
			if (mod)
			  {
			    bool found = false;
			    for (int i1 = 0; i1 < i; i1++)
			      {
				if (mod == comparableModules->fetch (i1))
				  {
				    found = true;
				    break;
				  }
			      }
			    if (!found)
			      functions->addAll (mod->functions);
			  }
		      }
		    get_metrics (functions, context);
		    delete functions;
		  }
		else
		  get_metrics (((Module*) obj)->functions, context);
		break;
	      }
	    case Histable::SOURCEFILE:
	      get_metrics (((SourceFile *) obj)->get_functions (), context);
	      break;
	    default:
	      DBG (assert (0));
	    }
	}
    }
  else if (mode == Hist_data::CALLERS)
    {
      if (objs && objs->size () > 0)
	get_clr_metrics (objs);
    }
  else if (mode == Hist_data::CALLEES)
    {
      if (objs && objs->size () > 0)
	get_cle_metrics (objs);
      else   // Special case: get root
	get_cle_metrics (NULL);
    }
  else if (mode == Hist_data::SELF)
    {
      if (objs->size () == 1)
	{
	  Histable *obj = objs->fetch (0);
	  if (obj != NULL)
	    {
	      if (obj->get_type () == Histable::LINE)
		{
		  Vector<Function*> *funclist = new Vector<Function*>;
		  for (DbeLine *dl = (DbeLine*) obj->convertto (Histable::LINE);
			  dl; dl = dl->dbeline_func_next)
		    if (dl->func)
		      funclist->append (dl->func);

		  get_self_metrics (obj, funclist, sel_objs);
		  delete funclist;
		}
	      else if (obj->get_type () == Histable::FUNCTION
		       || obj->get_type () == Histable::INSTR)
		{
		  // Use shortcut for functions and oth.
		  if (context)
		    {
		      Vector<Function*> *funclist = NULL;
		      if (context->get_type () == Histable::MODULE)
			funclist = ((Module*) context)->functions->copy ();
		      else
			{
			  funclist = new Vector<Function*>;
			  funclist->append ((Function*) context);
			}
		      get_self_metrics (obj, funclist, sel_objs);
		      delete funclist;
		    }
		  else
		    get_self_metrics (objs);
		}
	      else
		get_self_metrics (objs);
	    }
	}
      else
	get_self_metrics (objs);
    }
  else   // Hist_data::ALL
    get_metrics (root_idx, 0);

  delete[] obj_list;
  if ((type == Histable::LINE || type == Histable::INSTR)
      && mode == Hist_data::CALLERS)
    delete[] node_list;

  // Postprocess; find total
  for (long mind = 0, sz = mlist->get_items ()->size (); mind < sz; mind++)
    {
      Metric *mtr = mlist->get_items ()->get (mind);
      Metric::SubType subtype = mtr->get_subtype ();
      ValueTag vtype = mtr->get_vtype ();
      hist_data->total->value[mind].tag = vtype;

      switch (vtype)
	{
	  // ignoring the following cases (why?)
	case VT_SHORT:
	case VT_FLOAT:
	case VT_HRTIME:
	case VT_LABEL:
	case VT_ADDRESS:
	case VT_OFFSET:
	  break;

	case VT_INT:
	  // Calculate total as the sum of all values in hist_data for
	  // ATTRIBUTED metrics only. For all others, use root node values.
	  //
	  if ((mode == Hist_data::CALLERS || mode == Hist_data::CALLEES)
	      && subtype == Metric::ATTRIBUTED)
	    {
	      hist_data->total->value[mind].i = 0;
	      Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	      {
		hist_data->total->value[mind].i += hi->value[mind].i;
	      }
	      if (mode == Hist_data::CALLEES)
		hist_data->total->value[mind].i += hist_data->gprof_item->value[mind].i;
	    }
	  else if (xlate[mind] != -1)
	    ASN_METRIC_VAL (hist_data->total->value[mind], slots[xlate[mind]],
			    root_idx);
	  break;

	case VT_LLONG:
	  Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	  {
	    hi->value[mind].tag = vtype;
	  }

	  if ((mode == Hist_data::CALLERS || mode == Hist_data::CALLEES)
	      && subtype == Metric::ATTRIBUTED)
	    {
	      hist_data->total->value[mind].ll = 0;
	      Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	      {
		hist_data->total->value[mind].ll += hi->value[mind].ll;
	      }
	      if (mode == Hist_data::CALLEES)
		hist_data->total->value[mind].ll += hist_data->gprof_item->value[mind].ll;
	    }
	  else if (xlate[mind] != -1)
	    ASN_METRIC_VAL (hist_data->total->value[mind], slots[xlate[mind]], root_idx);
	  break;

	case VT_ULLONG:
	  Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	  {
	    hi->value[mind].tag = vtype;
	  }
	  if ((mode == Hist_data::CALLERS || mode == Hist_data::CALLEES)
	      && subtype == Metric::ATTRIBUTED)
	    {
	      hist_data->total->value[mind].ull = 0;
	      Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	      {
		hist_data->total->value[mind].ull += hi->value[mind].ull;
	      }
	      if (mode == Hist_data::CALLEES)
		hist_data->total->value[mind].ull += hist_data->gprof_item->value[mind].ull;
	    }
	  else if (xlate[mind] != -1)
	    ASN_METRIC_VAL (hist_data->total->value[mind], slots[xlate[mind]], root_idx);
	  break;

	case VT_DOUBLE:
	  double prec = mtr->get_precision ();
	  ValueTag vt = (xlate[mind] != -1) ? slots[xlate[mind]].vtype : VT_INT;
	  Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	  {
	    double val = (vt == VT_LLONG ? hi->value[mind].ll :
			  (vt == VT_ULLONG ? hi->value[mind].ull
			   : hi->value[mind].i));
	    hi->value[mind].tag = vtype;
	    hi->value[mind].d = val / prec;
	  }

	  if ((mode == Hist_data::CALLERS || mode == Hist_data::CALLEES)
	       && subtype == Metric::ATTRIBUTED)
	    {
	      hist_data->total->value[mind].d = 0.0;
	      Vec_loop (Hist_data::HistItem*, hist_data->hist_items, index, hi)
	      {
		hist_data->total->value[mind].d += hi->value[mind].d;
	      }
	      if (mode == Hist_data::CALLEES)
		hist_data->total->value[mind].d +=
		      (double) (vt == VT_LLONG ? hist_data->gprof_item->value[mind].ll :
				(vt == VT_ULLONG ? hist_data->gprof_item->value[mind].ull :
				 hist_data->gprof_item->value[mind].i)) / prec;
	    }
	  else if (xlate[mind] != -1)
	    {
	      TValue& total = hist_data->total->value[mind];
	      ASN_METRIC_VAL (total, slots[xlate[mind]], root_idx);
	      double val = (vt == VT_LLONG ? total.ll :
			    (vt == VT_ULLONG ? total.ll : total.i));
	      total.d = val / prec;
	    }
	  break;
	}
    }
  delete[] xlate;

  // Determine by which metric to sort if any
  bool rev_sort = mlist->get_sort_rev ();
  for (long mind = 0, sz = mlist->get_items ()->size (); mind < sz; mind++)
    {
      Metric *mtr = mlist->get_items ()->get (mind);
      if (mlist->get_sort_ref_index () == mind)
	sort_ind = mind;

      switch (mtr->get_type ())
	{
	case BaseMetric::SIZES:
	  Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	  {
	    Histable *h = mtr->get_comparable_obj (hi->obj);
	    hi->value[mind].tag = VT_LLONG;
	    hi->value[mind].ll = h ? h->get_size () : 0;
	  }
	  break;
	case BaseMetric::ADDRESS:
	  Vec_loop (Hist_data::HistItem *, hist_data->hist_items, index, hi)
	  {
	    Histable *h = mtr->get_comparable_obj (hi->obj);
	    hi->value[mind].tag = VT_ADDRESS;
	    hi->value[mind].ll = h ? h->get_addr () : 0;
	  }
	  break;
	case BaseMetric::DERIVED:
	  {
	    Definition *def = mtr->get_definition ();
	    long *map = def->get_map ();
	    for (long i1 = 0, sz1 = hist_data->hist_items->size (); i1 < sz1; i1++)
	      {
		/* Hist_data::HistItem * */hi = hist_data->hist_items->get (i1);
		hi->value[mind].tag = VT_DOUBLE;
		hi->value[mind].d = def->eval (map, hi->value);
	      }
	    hist_data->total->value[mind].tag = VT_DOUBLE;
	    hist_data->total->value[mind].d = def->eval (map, hist_data->total->value);
	  }
	  break;
	default:
	  break;
	}
    }

  hist_data->sort (sort_ind, rev_sort);
  hist_data->compute_minmax ();
  if (dbeSession->is_interactive () && mode != Hist_data::CALLERS)
    theApplication->set_progress (0, GTXT (""));

#if DEBUG_FTREE
  if (ftree_hist_data)
    {
      bool matches = ftree_debug_match_hist_data (hist_data, ftree_hist_data);
      if (!matches)
	assert (false);
      delete hist_data;
      hist_data = ftree_hist_data; // return the debug version
    }
#endif
  return hist_data;
}

#if DEBUG_FTREE
bool
PathTree::ftree_debug_match_hist_data (Hist_data *data /* ref */,
				       Hist_data *data_tmp)
{
  if (data->get_status () != Hist_data::SUCCESS)
    {
      DBG (assert (false));
      return false;
    }
  if (data == NULL && data != data_tmp)
    {
      DBG (assert (false));
      return false;
    }

  MetricList *mlist;
  mlist = data->get_metric_list ();
  MetricList *mlist_tmp;
  mlist_tmp = data_tmp->get_metric_list ();
  if (mlist->size () != mlist_tmp->size ())
    {
      DBG (assert (false));
      return false;
    }

  // Get table size: count visible metrics
  int nitems = data->size ();
  if (data->size () != data_tmp->size ())
    {
      DBG (assert (false));
      return false;
    }

  for (int i = 0; i < nitems; ++i)
    {
      Hist_data::HistItem *item = data->fetch (i);
      Hist_data::HistItem *item_tmp = data_tmp->fetch (i);
      if (item->obj->id != item_tmp->obj->id)
	{
	  DBG (assert (false));
	  return false;
	}
    }

  for (long i = 0, sz = mlist->size (); i < sz; i++)
    {
      long met_ind = i;
      Metric *mitem = mlist->get (i);
      Metric *mitem_tmp = mlist_tmp->get (i);

      if (mitem->get_id () != mitem_tmp->get_id ())
	{
	  DBG (assert (false));
	  return false;
	}
      if (mitem->get_visbits () != mitem_tmp->get_visbits ())
	{
	  DBG (assert (false));
	  return false;
	}
      if (mitem->get_vtype () != mitem_tmp->get_vtype ())
	{
	  DBG (assert (false));
	  return false;
	}

      if (!mitem->is_visible () && !mitem->is_tvisible ()
	  && !mitem->is_pvisible ())
	continue;
      // table->append(dbeGetTableDataOneColumn(data, i));
      for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	{
	  Metric *m = mitem;
	  TValue res;
	  TValue res_tmp;
	  TValue *v = data->get_value (&res, met_ind, row);
	  TValue *v_tmp = data_tmp->get_value (&res_tmp, met_ind, row);
	  if ((m->get_visbits () & VAL_RATIO) != 0)
	    {
	      if (v->tag != VT_LABEL)
		{
		  if (v->to_double () != v_tmp->to_double ())
		    {
		      DBG (assert (false));
		      return false;
		    }
		}
	      continue;
	    }
	  switch (m->get_vtype ())
	    {
	    case VT_DOUBLE:
	      {
		double diff = v->d - v_tmp->d;
		if (diff < 0) diff = -diff;
		if (diff > 0.0001)
		  {
		    DBG (assert (false));
		    return false;
		  }
		else
		  DBG (assert (true));
		break;
	      }
	    case VT_INT:
	      if (v->i != v_tmp->i)
		{
		  DBG (assert (false));
		  return false;
		}
	      break;
	    case VT_ULLONG:
	    case VT_LLONG:
	    case VT_ADDRESS:
	      if (v->ll != v_tmp->ll)
		{
		  DBG (assert (false));
		  return false;
		}
	      break;

	    case VT_LABEL:
	      if (dbe_strcmp (v->l, v_tmp->l))
		{
		  DBG (assert (false));
		  return false;
		}
	      break;
	    default:
	      DBG (assert (false));
	      return false;
	    }
	}
    }
  return true;
}
#endif

Histable *
PathTree::get_hist_func_obj (Node *node)
{
  LoadObject *lo;
  Function *func;
  func = (Function*) (node->instr->convertto (Histable::FUNCTION));
  // LIBRARY VISIBILITY
  lo = func->module->loadobject;
  if (dbev->get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
    return lo->get_hide_function ();
  return get_compare_obj (func);
}

Histable *
PathTree::get_hist_obj (Node *node, Histable* context)
{
  LoadObject *lo;
  Function *func;
  switch (hist_data->type)
    {
    case Histable::INSTR:
      if (hist_data->mode == Hist_data::MODL)
	{
	  if (node->instr->get_type () != Histable::INSTR)
	    return NULL;
	}
      else
	{
	  // LIBRARY VISIBILITY
	  func = (Function*) (node->instr->convertto (Histable::FUNCTION));
	  lo = func->module->loadobject;
	  if (dbev->get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
	    return lo->get_hide_function ();
	}
      return node->instr;

    case Histable::LINE:
      if (hist_data->mode != Hist_data::MODL)
	{
	  func = (Function*) (node->instr->convertto (Histable::FUNCTION));
	  lo = func->module->loadobject;
	  // LIBRARY VISIBILITY
	  if (dbev->get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
	    return lo->get_hide_function ();
	}
      // For openmp user mode - the stack is already made with dbelines,
      // no need to convert it
      if (node->instr->get_type () == Histable::LINE)
	return node->instr;
      return node->instr->convertto (Histable::LINE, context);

    case Histable::FUNCTION:
      if (pathTreeType == PATHTREE_INTERNAL_FUNCTREE && node->ancestor != 0)
	func = (Function*) node->instr;
      else
	func = (Function*) (node->instr->convertto (Histable::FUNCTION));
      lo = func->module->loadobject;
      // LIBRARY VISIBILITY
      if (dbev->get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
	return lo->get_hide_function ();
      return get_compare_obj (func);
    case Histable::MODULE:
      func = (Function*) (node->instr->convertto (Histable::FUNCTION));
      return func->module;
    case Histable::LOADOBJECT:
      func = (Function*) (node->instr->convertto (Histable::FUNCTION));
      return func->module->loadobject;
    case Histable::INDEXOBJ:
    case Histable::MEMOBJ:
      return node->instr;
    default:
      DBG (assert (0));
    }
  return NULL;
}

Histable *
PathTree::get_compare_obj (Histable *obj)
{
  if (obj && dbev->comparingExperiments ())
    obj = dbev->get_compare_obj (obj);
  return obj;
}

void
PathTree::get_metrics (NodeIdx node_idx, int dpth)
{
  Node *node = NODE_IDX (node_idx);
  Histable *cur_obj = get_hist_obj (node);
  obj_list[dpth] = cur_obj;

  // Check for recursion (inclusive metrics)
  int incl_ok = 1;
  for (int i = dpth - 1; i >= 0; i--)
    if (cur_obj == obj_list[i])
      {
	incl_ok = 0;
	break;
      }

  // Check for leaf nodes (exclusive metrics)
  int excl_ok = 0;
  if (IS_LEAF (node) || node == NODE_IDX (root_idx))
    excl_ok = 1;

  // We shouldn't eliminate empty subtrees here because
  // we create the list of hist items dynamically and want
  // one for each object in the tree.
  cur_obj = get_compare_obj (cur_obj);
  Hist_data::HistItem *hi = hist_data->append_hist_item (cur_obj);
  DBG (assert (hi != NULL));

  MetricList *mlist = hist_data->get_metric_list ();
  for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
    {
      if (xlate[ind] == -1)
	continue;
      Metric *mtr = mlist->get (ind);
      Metric::SubType subtype = mtr->get_subtype ();
      if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	continue;

      switch (subtype)
	{
	case Metric::INCLUSIVE:
	  if (incl_ok && hi)
	    ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	  break;
	case Metric::EXCLUSIVE:
	  if (excl_ok && hi)
	    ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	  break;
	  // ignoring the following cases (why?)
	case Metric::STATIC:
	case Metric::ATTRIBUTED:
	  break;
	case Metric::DATASPACE:
	  if (hi)
	    ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	  break;
	}
    }

  if (dbeSession->is_interactive ())
    {
      ndone++;
      int new_percent = 95 * ndone / nodes;
      if (new_percent > percent)
	{
	  percent = new_percent;
	  theApplication->set_progress (percent, NULL);
	}
    }

  // Recursively process all descendants
  int index;
  int dsize = NUM_DESCENDANTS (node);
  for (index = 0; index < dsize; index++)
    get_metrics (node->descendants->fetch (index), dpth + 1);
}

void
PathTree::get_clr_metrics (Vector<Histable*> *objs, NodeIdx node_idx,
			   int pmatch, int dpth)
{
  Node *node = NODE_IDX (node_idx);
  Histable *cur_obj;
  if (hist_data->type == Histable::LINE || hist_data->type == Histable::INSTR)
    {
      cur_obj = get_hist_func_obj (node);
      node_list[dpth] = node;
    }
  else
    cur_obj = get_hist_obj (node);
  obj_list[dpth] = cur_obj;

  bool match = false;
  int nobj = objs->size ();
  if (dpth + 1 >= nobj)
    {
      match = true;
      for (int i = 0; i < nobj; ++i)
	{
	  if (objs->fetch (i) != obj_list[dpth - nobj + 1 + i])
	    {
	      match = false;
	      break;
	    }
	}
    }

  Hist_data::HistItem *hi = NULL;
  Hist_data::HistItem *hi_adj = NULL;
  if (match && dpth >= nobj)
    {
      if (hist_data->type == Histable::LINE
	  || hist_data->type == Histable::INSTR)
	hi = hist_data->append_hist_item (get_hist_obj (node_list[dpth - nobj]));
      else
	hi = hist_data->append_hist_item (obj_list[dpth - nobj]);

      if (pmatch >= 0 && pmatch >= nobj)
	{
	  if (hist_data->type == Histable::LINE
	      || hist_data->type == Histable::INSTR)
	    hi_adj = hist_data->append_hist_item (get_hist_obj (
						    node_list[pmatch - nobj]));
	  else
	    hi_adj = hist_data->append_hist_item (obj_list[pmatch - nobj]);
	}
    }

  if (hi != NULL)
    {
      MetricList *mlist = hist_data->get_metric_list ();
      for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
	{
	  if (xlate[ind] == -1)
	    continue;
	  if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	    continue;
	  Metric *mtr = mlist->get (ind);
	  Metric::SubType subtype = mtr->get_subtype ();

	  switch (subtype)
	    {
	    case Metric::ATTRIBUTED:
	      if (hi)
		ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	      if (hi_adj)
		SUB_METRIC_VAL (hi_adj->value[ind], slots[xlate[ind]], node_idx);
	      break;
	    case Metric::STATIC:
	    case Metric::EXCLUSIVE:
	    case Metric::INCLUSIVE:
	    case Metric::DATASPACE:
	      break;
	    }
	}
    }

  // Recursively process all descendants
  int dsize = NUM_DESCENDANTS (node);
  for (int index = 0; index < dsize; index++)
    get_clr_metrics (objs, node->descendants->fetch (index),
		     match ? dpth : pmatch, dpth + 1);
}

void
PathTree::get_clr_metrics (Vector<Histable*> *objs)
{
  get_clr_metrics (objs, root_idx, -1, 0);
}

void
PathTree::get_cle_metrics (Vector<Histable*> *objs, NodeIdx node_idx, int pcle,
			   int pmatch, int dpth)
{
  Node *node = NODE_IDX (node_idx);
  Histable *cur_obj = get_hist_obj (node);
  obj_list[dpth] = cur_obj;

  bool match = false;
  int nobj = objs->size ();
  if (dpth + 1 >= nobj)
    {
      match = true;
      for (int i = 0; i < nobj; ++i)
	if (objs->fetch (i) != obj_list[dpth - nobj + 1 + i])
	  {
	    match = false;
	    break;
	  }
    }

  Hist_data::HistItem *hi = NULL;
  Hist_data::HistItem *hi_adj = NULL;
  if (pmatch >= 0 && dpth == pmatch + 1)
    hi = hist_data->append_hist_item (cur_obj);
  if (match && IS_LEAF (node))
    hi = hist_data->gprof_item;
  if (pcle >= 0)
    hi_adj = hist_data->append_hist_item (obj_list[pcle]);

  if (hi != NULL)
    {
      MetricList *mlist = hist_data->get_metric_list ();
      for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
	{
	  if (xlate[ind] == -1)
	    continue;
	  if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	    continue;
	  Metric *mtr = mlist->get (ind);
	  Metric::SubType subtype = mtr->get_subtype ();
	  if (subtype == Metric::ATTRIBUTED)
	    {
	      ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	      if (hi_adj)
		SUB_METRIC_VAL (hi_adj->value[ind], slots[xlate[ind]], node_idx);
	    }
	}
    }

  // Recursively process all descendants
  int dsize = NUM_DESCENDANTS (node);
  for (int index = 0; index < dsize; index++)
    get_cle_metrics (objs, node->descendants->fetch (index),
		     pmatch >= 0 && dpth == pmatch + 1 ? dpth : pcle,
		     match ? dpth : pmatch, dpth + 1);
}

void
PathTree::get_cle_metrics (Vector<Histable*> *objs, NodeIdx node_idx, int dpth)
{
  Node *node = NODE_IDX (node_idx);
  Histable *cur_obj = get_hist_obj (node);
  Hist_data::HistItem *hi = NULL;
  if (NULL == objs)   // Special case: get root
    hi = hist_data->append_hist_item (cur_obj);
  else
    {
      if (dpth == objs->size ())
	hi = hist_data->append_hist_item (cur_obj);
      else if (cur_obj == objs->fetch (dpth))
	{
	  // Recursively process all descendants
	  int dsize = NUM_DESCENDANTS (node);
	  for (int index = 0; index < dsize; index++)
	    get_cle_metrics (objs, node->descendants->fetch (index), dpth + 1);
	  if (dpth == objs->size () - 1 && dsize == 0)
	    hi = hist_data->gprof_item;
	}
    }

  if (hi != NULL)
    {
      MetricList *mlist = hist_data->get_metric_list ();
      for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
	{
	  if (xlate[ind] == -1)
	    continue;
	  if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	    continue;
	  Metric *mtr = mlist->get (ind);
	  Metric::SubType subtype = mtr->get_subtype ();
	  if (subtype == Metric::ATTRIBUTED)
	    ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	}
    }
}

void
PathTree::ftree_reset ()
{
  if (pathTreeType == PATHTREE_MAIN && indxtype < 0)
    {
      reset ();
      if (ftree_needs_update)
	{
	  if (ftree_internal == NULL)
	    {
	      ftree_internal = new PathTree (dbev, indxtype,
					     PATHTREE_INTERNAL_FUNCTREE);
	      if (ftree_internal == NULL)
		return;
	    }
	  ftree_internal->ftree_build (this);
	  ftree_needs_update = false;
	}
    }
}

void
PathTree::ftree_build (PathTree * mstr)
{
  fini ();
  init ();
  allocate_slots (mstr->slots, mstr->nslots);
  ftree_build (mstr, mstr->root_idx, root_idx);
  depth = mstr->depth;
  depth_map_build ();
}

#if DEBUG_FTREE // possibly TBR
void
PathTree::ftree_dump ()
{
  hrtime_t starttime, endtime;
  int nmetrics = 1;
  // int nmetrics = nslots;
  for (int kk = 0; kk < nmetrics; kk++)
    {
      int id = slots[kk].id;
      starttime = gethrtime ();
      long nodecnt = 0;
      for (int ii = 0; ii < depth; ii++)
	{
	  Vector<Vector<void*>*> *tmp = (Vector<Vector<void*>*>*)get_ftree_level
		  (id, ii);
	  if (tmp == NULL)
	    continue;
	  long sz = tmp->get (0)->size ();
	  nodecnt += sz;
#if 1
	  //   fprintf(stderr, "... finished (%ld nodes)\n", sz);
#else
	  Vector<NodeIdx> *nodeIdxList = (Vector<NodeIdx> *)tmp->get (0);
	  Vector<NodeIdx> *ancestorNodeIdxList = (Vector<NodeIdx> *)tmp->get (1);
	  Vector<uint64_t> *idList = (Vector<uint64_t> *)tmp->get (2);
	  Vector<uint64_t> *vals = (Vector<uint64_t> *)tmp->get (3);
	  for (int jj = 0; jj < sz; jj++)
	    fprintf (stderr, " ...%d:%d node=%ld, anc=%ld, id=%llu, val=%llu\n",
		     sz, jj, nodeIdxList->get (jj),
		     ancestorNodeIdxList->get (jj),
		     idList->get (jj), vals->get (jj));
#endif
	  destroy (tmp);
	}
      endtime = gethrtime ();
      fprintf (stderr, "====================== %ld nodes time=%llu\n",
	       nodecnt, (endtime - starttime) / 1000 / 1000);
    }
}
#endif

// ftree: translate mstr Histable::INSTR to Histable::FUNCTION
void
PathTree::ftree_build (PathTree *mstr, NodeIdx mstr_node_idx,
		       NodeIdx local_node_idx)
{
  // requires: slots, nslots
  Node *mstr_node = mstr->NODE_IDX (mstr_node_idx);
  int dsize = NUM_DESCENDANTS (mstr_node);

  // Add metrics
  for (int i = 0; i < nslots; i++)
    {
      if (i >= mstr->nslots)
	continue; //weird
      if (slots[i].vtype != mstr->slots[i].vtype)
	continue; //weird
      TValue val;
      val.ll = 0;
      mstr->ASN_METRIC_VAL (val, mstr->slots[i], mstr_node_idx);
      int64_t mval;
      switch (slots[i].vtype)
	{
	case VT_ULLONG:
	case VT_LLONG:
	  mval = val.ll;
	  break;
	case VT_INT:
	  mval = val.i;
	  break;
	default:
	  mval = 0;
	  break;
	}
      if (mval)
	{
	  Slot * mslot = SLOT_IDX (i);
	  if (mslot)
	    INCREMENT_METRIC (mslot, local_node_idx, mval);
	}
    }

  // Recursively process all descendants
  for (int index = 0; index < dsize; index++)
    {
      NodeIdx mstr_desc_node_idx = mstr_node->descendants->fetch (index);
      Node *mstr_desc_node = mstr->NODE_IDX (mstr_desc_node_idx);
      Function *func = (Function*) mstr_desc_node->instr->convertto (Histable::FUNCTION);
      int mstr_desc_dsize = NUM_DESCENDANTS (mstr_desc_node);
      bool leaf = (mstr_desc_dsize == 0);
      NodeIdx local_desc_node_idx = find_desc_node (local_node_idx, func, leaf);
      ftree_build (mstr, mstr_desc_node_idx, local_desc_node_idx);
    }
}

void
PathTree::depth_map_build ()
{
  destroy (depth_map);
  depth_map = new Vector<Vector<NodeIdx>*>(depth);
  if (depth)
    {
      depth_map->put (depth - 1, 0); // fill vector with nulls
      depth_map_build (root_idx, 0);
    }
}

void
PathTree::depth_map_build (NodeIdx node_idx, int dpth)
{
  Node *node = NODE_IDX (node_idx);

  Vector<NodeIdx> *node_idxs = depth_map->get (dpth);
  if (node_idxs == NULL)
    {
      node_idxs = new Vector<NodeIdx>();
      depth_map->store (dpth, node_idxs);
    }
  node_idxs->append (node_idx);

  // Recursively process all descendants
  int dsize = NUM_DESCENDANTS (node);
  for (int index = 0; index < dsize; index++)
    {
      NodeIdx desc_node_idx = node->descendants->fetch (index);
      depth_map_build (desc_node_idx, dpth + 1);
    }
}

int
PathTree::get_ftree_depth ()
{ // external use only
  ftree_reset ();
  if (!ftree_internal)
    return 0;
  return ftree_internal->get_depth ();
}

Vector<Function*>*
PathTree::get_ftree_funcs ()
{ // external use only
  ftree_reset ();
  if (!ftree_internal)
    return NULL;
  return ftree_internal->get_funcs ();
}

Vector<Function*>*
PathTree::get_funcs ()
{
  // get unique functions
  if (fn_map == NULL)
    return NULL;
  return fn_map->keySet ();
}

Vector<void*>*
PathTree::get_ftree_level (BaseMetric *bm, int dpth)
{ // external use only
  ftree_reset ();
  if (!ftree_internal)
    return NULL;
  return ftree_internal->get_level (bm, dpth);
}

Vector<void*>*
PathTree::get_level (BaseMetric *bm, int dpth)
{
  // Nodes at tree depth dpth
  if (dpth < 0 || dpth >= depth)
    return NULL;
  if (depth_map == NULL)
    return NULL;
  Vector<NodeIdx> *node_idxs = depth_map->get (dpth);
  return get_nodes (bm, node_idxs);
}

Vector<void*>*
PathTree::get_ftree_node_children (BaseMetric *bm, NodeIdx node_idx)
{ // external use only
  ftree_reset ();
  if (!ftree_internal)
    return NULL;
  return ftree_internal->get_node_children (bm, node_idx);
}

Vector<void*>*
PathTree::get_node_children (BaseMetric *bm, NodeIdx node_idx)
{
  // Nodes that are children of node_idx
  if (depth_map == NULL)
    return NULL;
  if (node_idx == 0)  // special case for root
    return get_nodes (bm, depth_map->get (0));
  if (node_idx < 0 || node_idx >= nodes)
    return NULL;
  Node *node = NODE_IDX (node_idx);
  if (node == NULL)
    return NULL;
  Vector<NodeIdx> *node_idxs = node->descendants;
  return get_nodes (bm, node_idxs);
}

Vector<void*>*
PathTree::get_nodes (BaseMetric *bm, Vector<NodeIdx> *node_idxs)
{ // used for ftree
  // capture info for node_idxs:
  //   node's idx
  //   node->ancestor idx
  //   node->instr->id
  //   mind metric value // in the future, could instead accept vector of mind
  if (node_idxs == NULL)
    return NULL;
  long sz = node_idxs->size ();
  if (sz <= 0)
    return NULL;

  bool calculate_metric = false;
  ValueTag vtype;
  int slot_idx;
  double prec;
  if (bm != NULL)
    {
      int mind = bm->get_id ();
      slot_idx = find_slot (mind); // may be -1 (CPI and IPC)
      prec = bm->get_precision ();
      vtype = bm->get_vtype ();
    }
  else
    {
      slot_idx = -1;
      prec = 1.0;
      vtype = VT_INT;
    }

  if (slot_idx >= 0)
    {
      switch (vtype)
	{
	case VT_ULLONG:
	case VT_LLONG:
	case VT_INT:
	  if (slots[slot_idx].vtype == vtype)
	    calculate_metric = true;
	  else
	    DBG (assert (false));
	  break;
	case VT_DOUBLE:
	  calculate_metric = true;
	  break;
	default:
	  break;
	}
    }

  Vector<void*> *results = new Vector<void*>(4);
  if (!calculate_metric)
    results->store (3, NULL);
  else
    {
      // Code below cribbed from Dbe.cc:dbeGetTableDataV2Data.
      // TBD: possibly create an intermediate HistData and instead call that routine
      switch (vtype)
	{
	case VT_ULLONG:
	case VT_LLONG:
	  {
	    Vector<long long> *vals = new Vector<long long>(sz);
	    for (long i = 0; i < sz; i++)
	      {
		NodeIdx node_idx = node_idxs->get (i);
		TValue val;
		val.ll = 0;
		ASN_METRIC_VAL (val, slots[slot_idx], node_idx);
		vals->append (val.ll);
	      }
	    results->store (3, vals);
	    break;
	  }
	case VT_DOUBLE:
	  {
	    Vector<double> *vals = new Vector<double>(sz);
	    TValue val;
	    val.tag = slots[slot_idx].vtype; // required for to_double();
	    for (long i = 0; i < sz; i++)
	      {
		NodeIdx node_idx = node_idxs->get (i);
		val.ll = 0;
		ASN_METRIC_VAL (val, slots[slot_idx], node_idx);
		double dval = val.to_double ();
		dval /= prec;
		vals->append (dval);
	      }
	    results->store (3, vals);
	    break;
	  }
	case VT_INT:
	  {
	    Vector<int> *vals = new Vector<int>(sz);
	    for (long i = 0; i < sz; i++)
	      {
		NodeIdx node_idx = node_idxs->get (i);
		TValue val;
		val.i = 0;
		ASN_METRIC_VAL (val, slots[slot_idx], node_idx);
		vals->append (val.i);
	      }
	    results->store (3, vals);
	    break;
	  }
	default:
	  results->store (3, NULL);
	  break;
	}
    }

  Vector<int> *nodeIdxList = new Vector<int>(sz);
  Vector<int> *ancestorNodeIdxList = new Vector<int>(sz);
  Vector<uint64_t> *idList = new Vector<uint64_t>(sz);
  for (long i = 0; i < sz; i++)
    {
      NodeIdx node_idx = node_idxs->get (i);
      Node *node = NODE_IDX (node_idx);
      NodeIdx ancestor_idx = node->ancestor;
      Histable *func = node->instr;
      nodeIdxList->append (node_idx);
      ancestorNodeIdxList->append (ancestor_idx);
      idList->append (func->id);
    }

  results->store (0, nodeIdxList);
  results->store (1, ancestorNodeIdxList);
  results->store (2, idList);
  return results;
}

void
PathTree::get_cle_metrics (Vector<Histable*> *objs)
{
  if (NULL == objs || objs->fetch (0) == get_hist_obj (NODE_IDX (root_idx)))
    // Call Tree optimization
    get_cle_metrics (objs, root_idx, 0);
  else
    // General case
    get_cle_metrics (objs, root_idx, -1, -1, 0);
}

void
PathTree::get_metrics (Vector<Function*> *functions, Histable *context)
{
  Function *fitem;
  int excl_ok, incl_ok;
  NodeIdx node_idx;
  Node *node, *anc;
  int index;

  Vec_loop (Function*, functions, index, fitem)
  {
    node_idx = fn_map->get (fitem);
    for (; node_idx; node_idx = node->funclist)
      {
	node = NODE_IDX (node_idx);
	Histable *h_obj = get_hist_obj (node, context);
	if (h_obj == NULL)
	  continue;

	// Check for recursion (inclusive metrics)
	incl_ok = 1;
	for (anc = NODE_IDX (node->ancestor); anc;
		anc = NODE_IDX (anc->ancestor))
	  {
	    if (h_obj == get_hist_obj (anc, context))
	      {
		incl_ok = 0;
		break;
	      }
	  }

	// Check for leaf nodes (exclusive metrics)
	excl_ok = 0;
	if (IS_LEAF (node))
	  excl_ok = 1;

	h_obj = get_compare_obj (h_obj);
	Hist_data::HistItem *hi = hist_data->append_hist_item (h_obj);

	if (!excl_ok)
	  hist_data->get_callsite_mark ()->put (h_obj, 1);
	MetricList *mlist = hist_data->get_metric_list ();
	for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
	  {
	    if (xlate[ind] == -1)
	      continue;
	    Metric *mtr = mlist->get (ind);
	    Metric::SubType subtype = mtr->get_subtype ();
	    if (subtype == Metric::INCLUSIVE && !incl_ok)
	      continue;
	    if (subtype == Metric::EXCLUSIVE && !excl_ok)
	      continue;
	    if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	      continue;
	    ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	  }
      }
  }
}

void
PathTree::get_self_metrics (Vector<Histable*> *objs, NodeIdx node_idx,
			    bool seen, int dpth)
{
  Node *node = NODE_IDX (node_idx);
  Histable *cur_obj = get_hist_obj (node);
  obj_list[dpth] = cur_obj;

  bool match = false;
  int nobj = objs->size ();
  if (dpth + 1 >= nobj)
    {
      match = true;
      for (int i = 0; i < nobj; ++i)
	{
	  if (objs->fetch (i) != obj_list[dpth - nobj + 1 + i])
	    {
	      match = false;
	      break;
	    }
	}
    }

  if (match)
    {
      Hist_data::HistItem *hi = hist_data->append_hist_item (cur_obj);
      int incl_ok = !seen;
      int excl_ok = 0;
      if (IS_LEAF (node) || node == NODE_IDX (root_idx))
	excl_ok = 1;
      MetricList *mlist = hist_data->get_metric_list ();
      for (long ind = 0, sz = mlist->size (); ind < sz; ind++)
	{
	  if (xlate[ind] == -1)
	    continue;
	  if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
	    continue;
	  Metric *mtr = mlist->get (ind);
	  Metric::SubType subtype = mtr->get_subtype ();
	  switch (subtype)
	    {
	    case Metric::INCLUSIVE:
	      if (incl_ok && hi)
		ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	      break;
	    case Metric::EXCLUSIVE:
	    case Metric::ATTRIBUTED:
	      if (excl_ok && hi)
		ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	      break;
	    case Metric::DATASPACE:
	      if (hi)
		ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	      break;
	      // ignoring the following cases (why?)
	    case Metric::STATIC:
	      break;
	    }
	}
    }

  if (dbeSession->is_interactive ())
    {
      ndone++;
      int new_percent = 95 * ndone / nodes;
      if (new_percent > percent)
	{
	  percent = new_percent;
	  theApplication->set_progress (percent, NULL);
	}
    }

  // Recursively process all descendants
  int index;
  int dsize = NUM_DESCENDANTS (node);
  for (index = 0; index < dsize; index++)
    get_self_metrics (objs, node->descendants->fetch (index),
		      seen || match, dpth + 1);
}

void
PathTree::get_self_metrics (Vector<Histable*> *objs)
{
  get_self_metrics (objs, root_idx, false, 0);
}

void
PathTree::get_self_metrics (Histable *obj, Vector<Function*> *funclist,
			    Vector<Histable*>* sel_objs)
{
  int excl_ok, incl_ok;
  NodeIdx node_idx;
  Node *node, *anc;

  if (obj == NULL)
    return;

  SourceFile *src = NULL;
  if (obj && obj->get_type () == Histable::LINE)
    {
      DbeLine *dbeline = (DbeLine*) obj;
      src = dbeline->sourceFile;
    }

  Hist_data::HistItem *hi = hist_data->append_hist_item (obj);
  for (int i = 0, sz = funclist ? funclist->size () : 0; i < sz; i++)
    {
      Function *fitem = (Function*) get_compare_obj (funclist->fetch (i));
      node_idx = fn_map->get (fitem);
      for (; node_idx; node_idx = node->funclist)
	{
	  node = NODE_IDX (node_idx);
	  if (obj && obj->get_type () == Histable::LINE)
	    {
	      Histable *h = get_hist_obj (node, src);
	      if (h == NULL)
		continue;
	      if (h->convertto (Histable::LINE) != obj->convertto (Histable::LINE))
		continue;
	    }
	  else if (get_hist_obj (node, src) != obj)
	    continue;

	  // Check for recursion (inclusive metrics)
	  incl_ok = 1;
	  for (anc = NODE_IDX (node->ancestor); anc;
		  anc = NODE_IDX (anc->ancestor))
	    {
	      if (get_hist_obj (anc, src) == obj)
		{
		  incl_ok = 0;
		  break;
		}
	      if (sel_objs != NULL)
		for (int k = 0; k < sel_objs->size (); k++)
		  if (sel_objs->fetch (k) == get_hist_obj (anc, src))
		    {
		      incl_ok = 0;
		      break;
		    }
	    }

	  // Check for leaf nodes (exclusive metrics)
	  excl_ok = 0;
	  if (IS_LEAF (node) || node == NODE_IDX (root_idx))
	    excl_ok = 1;

	  MetricList *mlist = hist_data->get_metric_list ();
	  for (long ind = 0, ind_sz = mlist->size (); ind < ind_sz; ind++)
	    {
	      if (xlate[ind] == -1)
		continue;
	      Metric *mtr = mlist->get (ind);
	      Metric::SubType subtype = mtr->get_subtype ();
	      if (subtype == Metric::INCLUSIVE && !incl_ok)
		continue;
	      if (subtype == Metric::EXCLUSIVE && !excl_ok)
		continue;
	      if (subtype == Metric::ATTRIBUTED && !excl_ok)
		continue;
	      if (IS_MVAL_ZERO (slots[xlate[ind]], node_idx))
		continue;
	      ADD_METRIC_VAL (hi->value[ind], slots[xlate[ind]], node_idx);
	    }
	}
    }
}

Vector<Histable*> *
PathTree::get_clr_instr (Histable * func)
{
  Vector<Histable*> * instrs = NULL;
  if (func->get_type () != Histable::FUNCTION)
    return NULL;
  NodeIdx node_idx = fn_map->get ((Function*) func);
  Node *node = NODE_IDX (node_idx);
  if (node == NULL)
    return new Vector<Histable*>();
  int instr_num = 0;
  for (; node; node = NODE_IDX (node->funclist))
    instr_num++;
  instrs = new Vector<Histable*>(instr_num);
  node = NODE_IDX (node_idx);
  Histable *instr = NODE_IDX (node->ancestor)->instr;
  instr_num = 0;
  instrs->store (instr_num, instr);
  node = NODE_IDX (node->funclist);
  for (; node; node = NODE_IDX (node->funclist))
    {
      instr = NODE_IDX (node->ancestor)->instr;
      instr_num++;
      instrs->store (instr_num, instr);
    }
  return instrs;
}

Vector<void*> *
PathTree::get_cle_instr (Histable * func, Vector<Histable*>*&instrs)
{
  if (func->get_type () != Histable::FUNCTION)
    return NULL;
  NodeIdx node_idx = fn_map->get ((Function*) func);
  Node *node = NODE_IDX (node_idx);
  if (node == NULL)
    {
      instrs = new Vector<Histable*>();
      return new Vector<void*>();
    }
  int instr_num = 0;
  for (; node; node = NODE_IDX (node->funclist))
    instr_num++;
  instrs = new Vector<Histable*>(instr_num);
  Vector<void*> *callee_info = new Vector<void*>(instr_num);
  node = NODE_IDX (node_idx);
  Histable *instr = node->instr;
  instr_num = 0;
  instrs->store (instr_num, instr);
  int dec_num = 0;
  NodeIdx dec_idx = 0;
  if (NUM_DESCENDANTS (node) > 0)
    {
      Vector<Histable*> * callee_instrs = new Vector<Histable*>(node->descendants->size ());
      Vec_loop (NodeIdx, node->descendants, dec_num, dec_idx)
      {
	Node * dec_node = NODE_IDX (dec_idx);
	//XXXX Note: there can be more than one instrs in one leaf function
	callee_instrs->store (dec_num, dec_node->instr);
      }
      callee_info->store (instr_num, callee_instrs);
    }
  else
    callee_info->store (instr_num, NULL);
  node = NODE_IDX (node->funclist);
  for (; node; node = NODE_IDX (node->funclist))
    {
      instr = node->instr;
      instr_num++;
      instrs->store (instr_num, instr);
      if (NUM_DESCENDANTS (node) > 0)
	{
	  Vector<Histable*> * callee_instrs = new Vector<Histable*>(node->descendants->size ());
	  Vec_loop (NodeIdx, node->descendants, dec_num, dec_idx)
	  {
	    Node * dec_node = NODE_IDX (dec_idx);
	    //XXXX Note: there can be more than one instrs in one leaf function
	    callee_instrs->store (dec_num, dec_node->instr);
	  }
	  callee_info->store (instr_num, callee_instrs);
	}
      else
	callee_info->store (instr_num, NULL);
    }
  return callee_info;
}

//
//
// The following methods are used for debugging purpose only.
//
//
static int maxdepth;
static int maxwidth;

void
PathTree::print (FILE *fd)
{
  (void) reset ();
  fprintf (fd, NTXT ("n = %lld, dn = %lld, MD = %lld\n\n"),
	   (long long) nodes, (long long) dnodes, (long long) depth);
  maxdepth = 0;
  maxwidth = 0;
  print (fd, root, 0);
  fprintf (fd, NTXT ("md = %lld, mw = %lld\n"),
	   (long long) maxdepth, (long long) maxwidth);
}

void
PathTree::print (FILE *fd, PathTree::Node *node, int lvl)
{
  const char *t;
  char *n;
  if (lvl + 1 > maxdepth)
    maxdepth = lvl + 1;
  for (int i = 0; i < lvl; i++)
    fprintf (fd, NTXT ("-"));
  Histable *instr = node->instr;
  if (instr->get_type () == Histable::LINE)
    {
      t = "L";
      n = ((DbeLine *) instr)->func->get_name ();
    }
  else if (instr->get_type () == Histable::INSTR)
    {
      t = "I";
      n = ((DbeInstr *) instr)->func->get_name ();
    }
  else
    {
      t = "O";
      n = instr->get_name ();
    }
  long long addr = (long long) instr->get_addr ();
  fprintf (fd, NTXT ("%s %s (0x%08llx) -- ndesc = %lld\n"),
	   t, n, addr, (long long) (NUM_DESCENDANTS (node)));

  // Recursively process all descendants
  int dsize = NUM_DESCENDANTS (node);
  if (dsize > maxwidth)
    maxwidth = dsize;
  for (int index = 0; index < dsize; index++)
    print (fd, NODE_IDX (node->descendants->fetch (index)), lvl + 1);
}

void
PathTree::printn (FILE *fd)
{
  int n = dbg_nodes (root);
  fprintf (fd, GTXT ("Number of nodes: %d, total size: %d\n"), n, (int) (n * sizeof (Node)));
}

void
PathTree::dumpNodes (FILE *fd, Histable *obj)
{
  const char *t;
  char *n;
  NodeIdx node_idx = fn_map->get ((Function*) obj);
  Node *node = NODE_IDX (node_idx);
  if (node == NULL)
    {
      fprintf (fd, GTXT ("No nodes associated with %s\n"), obj->get_name ());
      return;
    }
  Histable *instr = node->instr;
  for (; node; node = NODE_IDX (node->funclist))
    {
      instr = node->instr;
      if (instr->get_type () == Histable::LINE)
	{
	  t = "L";
	  n = ((DbeLine *) instr)->func->get_name ();
	}
      else if (instr->get_type () == Histable::INSTR)
	{
	  t = "I";
	  n = ((DbeInstr *) instr)->func->get_name ();
	}
      else
	{
	  t = "O";
	  n = instr->get_name ();
	}
      long long addr = (long long) instr->get_addr ();
      if (addr <= 0xFFFFFFFFU)
	fprintf (fd, NTXT ("0x%08x -- %s %s\n"), (uint32_t) addr, t, n);
      else
	fprintf (fd, NTXT ("0x%016llX -- %s %s\n"), addr, t, n);
    }
}

int
PathTree::dbg_nodes (PathTree::Node *node)
{
  int res = 1;
  int dsize = NUM_DESCENDANTS (node);
  for (int index = 0; index < dsize; index++)
    res += dbg_nodes (NODE_IDX (node->descendants->fetch (index)));
  return res;
}

static int mind_g;

int
leak_alloc_comp (const void *s1, const void *s2)
{
  // See Hist_data::sort_compare() for duplicate code
  int result = 0;
  CStack_data::CStack_item *t1, *t2;
  t1 = *(CStack_data::CStack_item **)s1;
  t2 = *(CStack_data::CStack_item **)s2;

  switch (t1->value[mind_g].tag)
    {
    case VT_INT:
      if (t1->value[mind_g].i < t2->value[mind_g].i)
	result = -1;
      else if (t1->value[mind_g].i > t2->value[mind_g].i)
	result = 1;
      else
	result = 0;
      break;
    case VT_LLONG:
      if (t1->value[mind_g].ll < t2->value[mind_g].ll)
	result = -1;
      else if (t1->value[mind_g].ll > t2->value[mind_g].ll)
	result = 1;
      else
	result = 0;
      break;
    case VT_ULLONG:
      if (t1->value[mind_g].ull < t2->value[mind_g].ull)
	result = -1;
      else if (t1->value[mind_g].ull > t2->value[mind_g].ull)
	result = 1;
      else
	result = 0;
      break;
      // ignoring the following cases (why?)
    case VT_SHORT:
    case VT_FLOAT:
    case VT_DOUBLE:
    case VT_HRTIME:
    case VT_LABEL:
    case VT_ADDRESS:
    case VT_OFFSET:
      break;
    }
  // Sort in descending order
  return -result;
}

CStack_data *
PathTree::get_cstack_data (MetricList *mlist)
{
  (void) reset ();
  CStack_data *lam = new CStack_data (mlist);
  int nmetrics = mlist->get_items ()->size ();
  mind_g = -1;
  xlate = new int[nmetrics];
  for (int mind = 0; mind < nmetrics; mind++)
    {
      xlate[mind] = -1;
      Metric *mtr = mlist->get_items ()->fetch (mind);
      if (mlist->get_sort_ref_index () == mind)
	mind_g = mind;
      xlate[mind] = find_slot (mtr->get_id ());
    }

  // now fill in the actual data
  obj_list = new Histable*[depth];
  get_cstack_list (lam, root_idx, 0);
  delete[] obj_list;

  if (mind_g >= 0)
    lam->cstack_items->sort (leak_alloc_comp);
  delete[] xlate;
  return lam;
}

void
PathTree::get_cstack_list (CStack_data *lam, NodeIdx node_idx, int dpth)
{

  Node *node = NODE_IDX (node_idx);
  obj_list[dpth] = node->instr;

  CStack_data::CStack_item *item = NULL;
  if (IS_LEAF (node))
    item = lam->new_cstack_item ();
  int nmetrics = lam->metrics->get_items ()->size ();
  bool subtree_empty = true;

  for (int mind = 0; mind < nmetrics; mind++)
    {
      if (xlate[mind] == -1)
	continue;
      if (IS_MVAL_ZERO (slots[xlate[mind]], node_idx))
	continue;
      else
	subtree_empty = false;
      if (item)
	{
	  ADD_METRIC_VAL (item->value[mind], slots[xlate[mind]], node_idx);
	  ADD_METRIC_VAL (lam->total->value[mind], slots[xlate[mind]], node_idx);
	}
    }

  if (subtree_empty)
    {
      delete item;
      return;
    }

  if (item)
    {
      // Finish processing a leaf node
      item->stack = new Vector<DbeInstr*>(dpth);
      for (int i = 1; i <= dpth; i++)
	item->stack->append ((DbeInstr*) obj_list[i]);
      lam->cstack_items->append (item);
    }
  else
    {
      // Recursively process all descendants
      int dsize = NUM_DESCENDANTS (node);
      for (int index = 0; index < dsize; index++)
	get_cstack_list (lam, node->descendants->fetch (index), dpth + 1);
    }
}

Emsg *
PathTree::fetch_stats ()
{
  if (statsq == NULL)
    return NULL;
  return statsq->fetch ();
}

void
PathTree::delete_stats ()
{
  if (statsq != NULL)
    {
      delete statsq;
      statsq = new Emsgqueue (NTXT ("statsq"));
    }
}

Emsg *
PathTree::fetch_warnings ()
{
  if (warningq == NULL)
    return NULL;
  return warningq->fetch ();
}

void
PathTree::delete_warnings ()
{
  if (warningq != NULL)
    {
      delete warningq;
      warningq = new Emsgqueue (NTXT ("warningq"));
    }
}
