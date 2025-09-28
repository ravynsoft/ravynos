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

#ifndef _PATH_TREE_H
#define _PATH_TREE_H

#include <vec.h>
#include <Map.h>

#include "dbe_structs.h"
#include "Hist_data.h"
#include "Histable.h"
#include "Metric.h"

typedef enum
{
  NORMAL = 0, CANCELED
} PtreePhaseStatus;

class PathTree
{
public:

  PathTree (DbeView *_dbev, int _indxtype = -1)
  {
    construct (_dbev, _indxtype, PATHTREE_MAIN);
  }

  ~PathTree ();

  static void make_deltas (int vtype, TValue *v1, TValue *v2);
  static void make_ratios (int vtype, TValue *v1, TValue *v2);

  typedef enum
  {
    COMPUTEOPT_NONE = 0,
    COMPUTEOPT_OMP_CALLEE
  } PtreeComputeOption;

  Hist_data *compute_metrics (MetricList *, Histable::Type,
			      Hist_data::Mode, Vector<Histable*>*,
			      Histable*, Vector<Histable*>* sel_objs = NULL,
			      PtreeComputeOption flag = COMPUTEOPT_NONE);
  // Get aggregated callstack data
  CStack_data *get_cstack_data (MetricList *);

  Vector<Histable*> *get_clr_instr (Histable *);
  Vector<void*> *get_cle_instr (Histable *, Vector<Histable*>*&);

  int
  get_status ()
  {
    return status;
  }

  int
  get_depth ()
  {
    return depth;
  }

  int
  getStackProp ()
  {
    return stack_prop;
  }

  typedef long NodeIdx;

  struct Node
  {
    inline void
    reset ()
    {
      ancestor = 0;
      descendants = NULL;
      instr = NULL;
      funclist = 0;
    }

    NodeIdx ancestor;
    Vector<NodeIdx> *descendants;
    Histable *instr;
    NodeIdx funclist;
  };

  static const int CHUNKSZ = 16384;

  inline Node *
  NODE_IDX (NodeIdx idx)
  {
    return idx ? &chunks[idx / CHUNKSZ][idx % CHUNKSZ] : NULL;
  }

  // queue for messages (statistics for pathtree processing)
  Emsg *fetch_stats (void);     // fetch the queue of comment messages
  void delete_stats (void);     // delete the queue of stats messages
  Emsg *fetch_warnings (void);  // fetch the queue of warnings messages
  void delete_warnings (void);  // delete the queue of warnings messages

  NodeIdx
  get_func_nodeidx (Function * func)
  {
    return fn_map == NULL ? (NodeIdx) 0 : fn_map->get (func);
  }

  void print (FILE *);
  void dumpNodes (FILE *, Histable *);

  // flame charts functions - get values from ftree_internal
  int get_ftree_depth ();   // Depth of tree
  Vector<void*>* get_ftree_level (BaseMetric *bm, int dpth);
  Vector<void*>* get_ftree_node_children (BaseMetric *bm, NodeIdx node_idx);
  Vector<Function*>* get_ftree_funcs ();
  Vector<Function*>* get_funcs ();      // Unique functions in tree

private:

  enum
  {
    MAX_DESC_HTABLE_SZ = 65535
  };

  typedef struct hash_node
  {
    NodeIdx nd;
    struct hash_node *next;
  } hash_node_t;

  int desc_htable_size;
  int desc_htable_nelem;
  hash_node_t **descHT;

  struct Slot
  {
    int id;
    ValueTag vtype;
    union
    {
      int **mvals;
      int64_t **mvals64;
    };
  };

  typedef enum
  {
    PATHTREE_MAIN = 0,
    PATHTREE_INTERNAL_OMP,
    PATHTREE_INTERNAL_FUNCTREE
  } PathTreeType;

  DbeView *dbev;
  int indxtype;
  int stack_prop;
  Expression *indx_expr;
  Histable *total_obj;
  Map<Function*, NodeIdx> *fn_map;
  Map<uint64_t, NodeIdx> *pathMap;
  Map<uint64_t, uint64_t> *hideMap;
  int status;
  NodeIdx root_idx;
  Node *root;
  int depth;
  long nodes;
  long dnodes;
  long nchunks;
  Node **chunks;
  int nslots;
  Slot *slots;
  int phaseIdx;
  int nexps;
  Emsgqueue *statsq;
  Emsgqueue *warningq;
  Hist_data *hist_data;
  int percent;
  int ndone;
  Histable **obj_list;
  Node **node_list;
  int *xlate;
  bool cancel_ok;
  PathTreeType pathTreeType;
  PathTree *ptree_internal;
  PathTree *ftree_internal;             // function-based pathtree
  bool ftree_needs_update;
  Vector<Vector<NodeIdx>*> *depth_map; // for each depth level, list of nodes

  void init ();
  void fini ();
  PtreePhaseStatus reset ();
  PtreePhaseStatus add_experiment (int);
  PtreePhaseStatus process_packets (Experiment*, DataView*, int);
  DataView *get_filtered_events (int exp_index, int data_type);
  void construct (DbeView *_dbev, int _indxtype, PathTreeType _pathTreeType);

  PathTree (DbeView *_dbev, int _indxtype, PathTreeType _pathTreeType)
  {
    construct (_dbev, _indxtype, _pathTreeType);
  }

  inline int *
  allocate_chunk (int **p, NodeIdx idx)
  {
    int *res = new int[CHUNKSZ];
    for (int i = 0; i < CHUNKSZ; i++)
      res[i] = 0;
    p[idx] = res;
    return res;
  };

  inline int64_t *
  allocate_chunk (int64_t **p, NodeIdx idx)
  {
    int64_t *res = new int64_t[CHUNKSZ];
    for (int i = 0; i < CHUNKSZ; i++)
      res[i] = 0;
    p[idx] = res;
    return res;
  };

  inline Node *
  allocate_chunk (Node **p, NodeIdx idx)
  {
    Node *res = new Node[CHUNKSZ];
    for (int i = 0; i < CHUNKSZ; i++)
      res[i].reset ();
    p[idx] = res;
    return res;
  };

  inline bool
  IS_MVAL_ZERO (Slot& slot, NodeIdx idx)
  {
    if (slot.vtype == VT_LLONG || slot.vtype == VT_ULLONG)
      {
	int64_t *tmp = slot.mvals64[idx / CHUNKSZ];
	return tmp ? tmp[idx % CHUNKSZ] == 0 : true;
      }
    else
      {
	int *tmp = slot.mvals[idx / CHUNKSZ];
	return tmp ? tmp[idx % CHUNKSZ] == 0 : true;
      }
  }

  inline void
  ASN_METRIC_VAL (TValue& v, Slot& slot, NodeIdx idx)
  {
    if (slot.vtype == VT_LLONG)
      {
	int64_t *tmp = slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ll = tmp[idx % CHUNKSZ];
      }
    else if (slot.vtype == VT_ULLONG)
      {
	uint64_t *tmp = (uint64_t *) slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ull = tmp[idx % CHUNKSZ];
      }
    else
      {
	int *tmp = slot.mvals[idx / CHUNKSZ];
	if (tmp)
	  v.i = tmp[idx % CHUNKSZ];
      }
  }

  inline void
  ADD_METRIC_VAL (TValue& v, Slot& slot, NodeIdx idx)
  {
    if (slot.vtype == VT_LLONG)
      {
	int64_t *tmp = slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ll += tmp[idx % CHUNKSZ];
      }
    else if (slot.vtype == VT_ULLONG)
      {
	uint64_t *tmp = (uint64_t *) slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ull += tmp[idx % CHUNKSZ];
      }
    else
      {
	int *tmp = slot.mvals[idx / CHUNKSZ];
	if (tmp) v.i += tmp[idx % CHUNKSZ];
      }
  }

  inline void
  SUB_METRIC_VAL (TValue& v, Slot& slot, NodeIdx idx)
  {
    if (slot.vtype == VT_LLONG)
      {
	int64_t *tmp = slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ll -= tmp[idx % CHUNKSZ];
      }
    else if (slot.vtype == VT_ULLONG)
      {
	uint64_t *tmp = (uint64_t *) slot.mvals64[idx / CHUNKSZ];
	if (tmp)
	  v.ull -= tmp[idx % CHUNKSZ];
      }
    else
      {
	int *tmp = slot.mvals[idx / CHUNKSZ];
	if (tmp)
	  v.i -= tmp[idx % CHUNKSZ];
      }
  }

  inline void
  INCREMENT_METRIC (Slot *slot, NodeIdx idx, int64_t val)
  {
    if (slot->vtype == VT_LLONG)
      {
	int64_t *tmp = slot->mvals64[idx / CHUNKSZ];
	if (tmp == NULL)
	  tmp = allocate_chunk (slot->mvals64, idx / CHUNKSZ);
	tmp[idx % CHUNKSZ] += val;
      }
    else if (slot->vtype == VT_ULLONG)
      {
	uint64_t *tmp = (uint64_t *) slot->mvals64[idx / CHUNKSZ];
	if (tmp == NULL)
	  tmp = (uint64_t *) allocate_chunk (slot->mvals64, idx / CHUNKSZ);
	tmp[idx % CHUNKSZ] += val;
      }
    else
      {
	int *tmp = slot->mvals[idx / CHUNKSZ];
	if (tmp == NULL)
	  tmp = allocate_chunk (slot->mvals, idx / CHUNKSZ);
	tmp[idx % CHUNKSZ] += (int) val;
      }
  }

  inline Slot *
  SLOT_IDX (int idx)
  {
    if (idx < 0 || idx >= nslots)
      return NULL;
    return &slots[idx];
  }

  int allocate_slot (int id, ValueTag vtype);
  void allocate_slots (Slot *slots, int nslots);
  int find_slot (int);
  NodeIdx new_Node (NodeIdx, Histable*, bool);
  NodeIdx find_path (Experiment*, DataView*, long);
  NodeIdx find_desc_node (NodeIdx, Histable*, bool);
  NodeIdx find_in_desc_htable (NodeIdx, Histable*, bool);
  Histable *get_hist_obj (Node *, Histable* = NULL);
  Histable *get_hist_func_obj (Node *);
  Histable *get_compare_obj (Histable *obj);
  void get_metrics (NodeIdx, int);
  void get_metrics (Vector<Function*> *, Histable *);
  void get_clr_metrics (Vector<Histable*>*, NodeIdx, int, int);
  void get_clr_metrics (Vector<Histable*>*);
  void get_cle_metrics (Vector<Histable*>*, NodeIdx, int, int, int);
  void get_cle_metrics (Vector<Histable*>*, NodeIdx, int);
  void get_cle_metrics (Vector<Histable*>*);
  void get_self_metrics (Vector<Histable*>*, NodeIdx, bool, int);
  void get_self_metrics (Vector<Histable*>*);
  void get_self_metrics (Histable *, Vector<Function*> *funclist,
			 Vector<Histable*>* sel_objs = NULL);
  void get_cstack_list (CStack_data *, NodeIdx, int);

  // Generate PathTree based on Functions instead of Instructions // Used for flame chart
  void ftree_reset ();
  void ftree_build (PathTree *mstr);
  void ftree_build (PathTree *mstr, NodeIdx mstr_node_idx, NodeIdx local_node_idx);
  void depth_map_build ();
  void depth_map_build (NodeIdx node_idx, int depth);
  Vector<void*>* get_level (BaseMetric *bm, int dpth);
  Vector<void*>* get_nodes (BaseMetric *bm, Vector<NodeIdx> *node_idxs);
  Vector<void*>* get_node_children (BaseMetric *bm, NodeIdx node_idx);
  bool ftree_debug_match_hist_data (Hist_data *data, Hist_data *data_tmp);
  void ftree_dump ();

  // Debugging functions
  void print (FILE *, PathTree::Node*, int);
  void printn (FILE *);
  int dbg_nodes (PathTree::Node*);
};

#endif /* _PATH_TREE_H */
