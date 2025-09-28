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
#include <new>

#include "util.h"
#include "CacheMap.h"
#include "CallStack.h"
#include "DbeSession.h"
#include "DbeView.h"
#include "DbeLinkList.h"
#include "Experiment.h"
#include "Exp_Layout.h"
#include "Function.h"
#include "LoadObject.h"
#include "Module.h"

Descendants::Descendants ()
{
  count = 0;
  limit = sizeof (first_data) / sizeof (CallStackNode *);
  data = first_data;
}

Descendants::~Descendants ()
{
  if (data != first_data)
    free (data);
}

CallStackNode *
Descendants::find (Histable *hi, int *index)
{
  int cnt = count;
  int left = 0;
  for (int right = cnt - 1; left <= right;)
    {
      int ind = (left + right) / 2;
      CallStackNode *node = data[ind];
      Histable *instr = node->get_instr ();
      if (instr == hi)
	{
	  if (index)
	    *index = ind;
	  return node;
	}
      if (instr->id < hi->id)
	right = ind - 1;
      else
	left = ind + 1;
    }
  if (index)
    *index = left;
  return NULL;
}

void
Descendants::append (CallStackNode* item)
{
  if (count < limit)
    data[count++] = item;
  else
    insert (count, item);
}

void
Descendants::insert (int ind, CallStackNode* item)
{
  CallStackNode **old_data = data;
  int old_cnt = count;
  if (old_cnt + 1 >= limit)
    {
      int new_limit = (limit == 0) ? DELTA : limit * 2;
      CallStackNode **new_data = (CallStackNode **) malloc (new_limit * sizeof (CallStackNode *));
      for (int i = 0; i < ind; i++)
	new_data[i] = old_data[i];
      new_data[ind] = item;
      for (int i = ind; i < old_cnt; i++)
	new_data[i + 1] = old_data[i];
      limit = new_limit;
      data = new_data;
      if (old_data != first_data)
	free (old_data);
    }
  else
    {
      for (int i = ind; i < old_cnt; i++)
	old_data[i + 1] = old_data[i];
      old_data[ind] = item;
    }
  count++;
}

/*
 *    Private implementation of CallStack interface
 */

// When performing pipeline optimization on resolve_frame_info + add_stack
// cstk_ctx structure contains the state (or context) for one iteration to pass on
// from Phase 2 to Phase 3 (More details in Experiment.cc)
class CallStackP : public CallStack
{
public:
  CallStackP (Experiment *exp);

  virtual ~CallStackP ();

  virtual void add_stack (DataDescriptor *dDscr, long idx, FramePacket *frp, cstk_ctx_chunk *cstCtxChunk);
  virtual void *add_stack (Vector<Histable*> *objs);
  virtual CallStackNode *get_node (int n);
  virtual void print (FILE *);

private:

  static const int CHUNKSZ = 16384;

  Experiment *experiment;
  CallStackNode *root;
  CallStackNode *jvm_node;
  int nodes;
  int nchunks;
  CallStackNode **chunks;
  Map<uint64_t, CallStackNode *> *cstackMap;
  DbeLock *cstackLock;

  CallStackNode *add_stack (long start, long end, Vector<Histable*> *objs, CallStackNode *myRoot);
  CallStackNode *new_Node (CallStackNode*, Histable*);
  CallStackNode *find_preg_stack (uint64_t);
  // objs are in the root..leaf order
  void *add_stack_d (Vector<Histable*> *objs);
  void add_stack_java (DataDescriptor *dDscr, long idx, FramePacket *frp, hrtime_t tstamp, uint32_t thrid, Vector<DbeInstr*>* natpcs, bool natpc_added, cstk_ctx_chunk *cstCtxChunk);
  void add_stack_java_epilogue (DataDescriptor *dDscr, long idx, FramePacket *frp, hrtime_t tstamp, uint32_t thrid, Vector<DbeInstr*>* natpcs, Vector<Histable*>* jpcs, bool natpc_added);

  // Adjust HW counter event to find better trigger PC, etc.
  DbeInstr *adjustEvent (DbeInstr *leafPC, DbeInstr * candPC,
			 Vaddr &eventEA, int abst_type);
  Vector<DbeInstr*> *natpcsP;
  Vector<Histable*> *jpcsP;
};

CallStackP::CallStackP (Experiment *exp)
{
  experiment = exp;
  nchunks = 0;
  chunks = NULL;
  nodes = 0;
  cstackMap = new CacheMap<uint64_t, CallStackNode *>;
  cstackLock = new DbeLock ();
  Function *total = dbeSession->get_Total_Function ();
  root = new_Node (0, total->find_dbeinstr (0, 0));
  jvm_node = NULL;
  natpcsP = NULL;
  jpcsP = NULL;
}

CallStackP::~CallStackP ()
{
  delete cstackLock;
  if (chunks)
    {
      for (int i = 0; i < nodes; i++)
	{
	  CallStackNode *node = get_node (i);
	  node->~CallStackNode ();
	}
      for (int i = 0; i < nchunks; i++)
	free (chunks[i]);
      free (chunks);
    }
  delete natpcsP;
  delete jpcsP;
  destroy_map (CallStackNode *, cstackMap);
}

CallStackNode *
CallStackP::new_Node (CallStackNode *anc, Histable *pcval)
{
  // cstackLock->aquireLock(); // Caller already locked it
  if (nodes >= nchunks * CHUNKSZ)
    {
      CallStackNode **old_chunks = chunks;
      nchunks++;

      // Reallocate Node chunk array
      chunks = (CallStackNode **) malloc (nchunks * sizeof (CallStackNode *));
      for (int i = 0; i < nchunks - 1; i++)
	chunks[i] = old_chunks[i];
      free (old_chunks);
      // Allocate new chunk for nodes.
      chunks[nchunks - 1] = (CallStackNode *) malloc (CHUNKSZ * sizeof (CallStackNode));
    }
  nodes++;
  CallStackNode *node = get_node (nodes - 1);
  new (node) CallStackNode (anc, pcval);
  // cstackLock->releaseLock();
  return node;
}

CallStackNode *
CallStackP::find_preg_stack (uint64_t prid)
{
  DataView *dview = experiment->getOpenMPdata ();
  dview->sort (PROP_CPRID);
  Datum tval;
  tval.setUINT64 (prid);
  long idx = dview->getIdxByVals (&tval, DataView::REL_EQ);
  if (idx < 0)
    return root;
  CallStackNode *node = (CallStackNode*) dview->getObjValue (PROP_USTACK, idx);
  if (node != NULL)
    return node;
  uint64_t pprid = dview->getLongValue (PROP_PPRID, idx);
  if (pprid == prid)
    return root;
  void *nat_stack = dview->getObjValue (PROP_MSTACK, idx);
  Vector<Histable*> *pcs = getStackPCs (nat_stack);

  // Find the bottom frame
  int btm;
  bool inOMP = false;
  DbeInstr *instr;
  Histable *hist;
  for (btm = 0; btm < pcs->size (); btm++)
    {
      hist = pcs->fetch (btm);
      if (hist->get_type () == Histable::INSTR)
	instr = (DbeInstr *) hist;
      else    // DBELINE
	instr = (DbeInstr *) hist->convertto (Histable::INSTR);
      LoadObject *lo = instr->func->module->loadobject;
      if (!inOMP)
	{
	  if (lo->flags & SEG_FLAG_OMP)
	    inOMP = true;
	}
      else if (!(lo->flags & SEG_FLAG_OMP))
	break;
    }

  // Find the top frame
  dview->sort (PROP_CPRID);
  int top;
  tval.setUINT64 (pprid);
  long pidx = dview->getIdxByVals (&tval, DataView::REL_EQ);
  if (pidx < 0)     // No parent. Process the entire nat_stack
    top = pcs->size () - 1;
  else
    {
      uint32_t thrid = (uint32_t) dview->getIntValue (PROP_THRID, idx);
      uint32_t pthrid = (uint32_t) dview->getIntValue (PROP_THRID, pidx);
      if (thrid != pthrid)
	{
	  // Parent is on a different stack.
	  // Process the entire nat_stack. Skip libthread.
	  for (top = pcs->size () - 1; top >= 0; top--)
	    {
	      hist = pcs->fetch (top);
	      if (hist->get_type () == Histable::INSTR)
		instr = (DbeInstr *) hist;
	      else // DBELINE
		instr = (DbeInstr *) hist->convertto (Histable::INSTR);
	      if (instr->func->module->loadobject->flags & SEG_FLAG_OMP)
		break;
	    }
	  if (top < 0)  // None found. May be incomplete call stack (x86)
	    top = pcs->size () - 1;
	}
      else
	{
	  // Parent is on the same stack. Find match.
	  top = pcs->size () - 1;
	  void *pnat_stack = dview->getObjValue (PROP_MSTACK, pidx);
	  Vector<Histable*> *ppcs = getStackPCs (pnat_stack);
	  for (int ptop = ppcs->size () - 1; top >= 0 && ptop >= 0;
		  top--, ptop--)
	    {
	      if (pcs->fetch (top) != ppcs->fetch (ptop))
		break;
	    }
	  delete ppcs;
	}
    }

  // Process the found range
  Vector<Histable*> *upcs = new Vector<Histable*>(128);
  for (int i = btm; i <= top; ++i)
    {
      hist = (DbeInstr*) pcs->fetch (i);
      if (hist->get_type () == Histable::INSTR)
	instr = (DbeInstr *) hist;
      else // DBELINE
	instr = (DbeInstr *) hist->convertto (Histable::INSTR);

      if (instr->func->module->loadobject->flags & SEG_FLAG_OMP)
	// Skip all frames from libmtsk
	continue;
      upcs->append (instr);
    }
  delete pcs;
  node = find_preg_stack (pprid);
  while (node != root)
    {
      upcs->append (node->instr);
      node = node->ancestor;
    }
  node = (CallStackNode *) add_stack (upcs);
  dview->setObjValue (PROP_USTACK, idx, node);
  delete upcs;
  return node;
}

#define JNI_MARKER -3

// This is one iteration if the third stage of
// resolve_frame_info + add_stack pipeline. Works on building the java
// stacks
void
CallStackP::add_stack_java (DataDescriptor *dDscr, long idx, FramePacket *frp,
			    hrtime_t tstamp, uint32_t thrid,
			    Vector<DbeInstr*>* natpcs, bool natpc_added,
			    cstk_ctx_chunk *cstCtxChunk)
{
  Vector<Histable*> *jpcs = NULL;
  cstk_ctx *cstctx = NULL;
  if (cstCtxChunk != NULL)
    {
      cstctx = cstCtxChunk->cstCtxAr[idx % CSTCTX_CHUNK_SZ];
      jpcs = cstctx->jpcs;
      jpcs->reset ();
    }
  if (jpcs == NULL)
    {
      // this is when we are not doing the pipeline optimization
      // Temporary array for resolved addresses
      // [leaf_pc .. root_pc] == [0..stack_size-1]
      // Leave room for a possible "truncated" frame
      if (jpcsP == NULL)
	jpcsP = new Vector<Histable*>;
      jpcs = jpcsP;
      jpcs->reset ();
    }

  //
  // Construct the user stack
  //
  // Construct Java user stack
  int jstack_size = frp->stackSize (true);
  if (jstack_size)
    {
      // jpcs = new Vector<Histable*>( jstack_size );
      if (frp->isTruncatedStack (true))
	{
	  Function *truncf = dbeSession->getSpecialFunction (DbeSession::TruncatedStackFunc);
	  jpcs->append (truncf->find_dbeinstr (0, 0));
	}

      int nind = natpcs->size () - 1; // first native frame
      for (int jind = jstack_size - 1; jind >= 0; jind--)
	{
	  bool jleaf = (jind == 0); // is current java frame a leaf?
	  Vaddr mid = frp->getMthdFromStack (jind);
	  int bci = frp->getBciFromStack (jind);
	  DbeInstr *cur_instr = experiment->map_jmid_to_PC (mid, bci, tstamp);
	  jpcs->append (cur_instr);
	  if (bci == JNI_MARKER)
	    {
	      JMethod *j_method = (JMethod*) cur_instr->func;
	      // Find matching native function on the native stack
	      bool found = false;
	      for (; nind >= 0; nind--)
		{
		  DbeInstr *nat_addr = natpcs->fetch (nind);
		  if (0 == nat_addr)
		    continue;
		  Function *nat_func = nat_addr->func;
		  if (!found && j_method->jni_match (nat_func))
		    found = true;
		  if (found)
		    {
		      // XXX omazur: the following will skip JNI native method
		      // implemented in JVM itself.
		      // If we are back in JVM switch to processing Java
		      // frames if there are any.
		      if ((nat_func->module->loadobject->flags & SEG_FLAG_JVM) && !jleaf)
			break;
		      jpcs->append (nat_addr);
		    }
		}
	    }
	}
    }
  add_stack_java_epilogue (dDscr, idx, frp, tstamp, thrid, natpcs, jpcs, natpc_added);
}

// This is one iteration if the fourth stage of
// resolve_frame_info + add_stack pipeline.
// It adds the native and java stacks to the stackmap

void
CallStackP::add_stack_java_epilogue (DataDescriptor *dDscr, long idx, FramePacket *frp, hrtime_t tstamp, uint32_t thrid, Vector<DbeInstr*>* natpcs, Vector<Histable*> *jpcs, bool natpc_added)
{
  CallStackNode *node = NULL;
  if (!natpc_added)
    {
      node = (CallStackNode *) add_stack ((Vector<Histable*>*)natpcs);
      dDscr->setObjValue (PROP_MSTACK, idx, node);
      dDscr->setObjValue (PROP_XSTACK, idx, node);
      dDscr->setObjValue (PROP_USTACK, idx, node);
    }

  int jstack_size = frp->stackSize (true);
  if (jstack_size)
    {
      if (jpcs != NULL)
	node = (CallStackNode *) add_stack_d (jpcs);
      if (node == NULL)
	node = (CallStackNode*) dDscr->getObjValue (PROP_USTACK, idx);
      dDscr->setObjValue (PROP_USTACK, idx, node);
      Function *func = (Function*) node->instr->convertto (Histable::FUNCTION);
      if (func != dbeSession->get_JUnknown_Function ())
	dDscr->setObjValue (PROP_XSTACK, idx, node);
    }

  JThread *jthread = experiment->map_pckt_to_Jthread (thrid, tstamp);
  if (jthread == JTHREAD_NONE && jstack_size != 0 && node != NULL)
    {
      Function *func = (Function*) node->instr->convertto (Histable::FUNCTION);
      if (func != dbeSession->get_JUnknown_Function ())
	jthread = JTHREAD_DEFAULT;
    }
  dDscr->setObjValue (PROP_JTHREAD, idx, jthread);
  if (jthread == JTHREAD_NONE || (jthread != JTHREAD_DEFAULT && jthread->is_system ()))
    {
      if (jvm_node == NULL)
	{
	  Function *jvm = dbeSession->get_jvm_Function ();
	  if (jvm)
	    {
	      jvm_node = new_Node (root, jvm->find_dbeinstr (0, 0));
	      CommonPacket::jvm_overhead = jvm_node;
	    }
	}
      dDscr->setObjValue (PROP_USTACK, idx, jvm_node);
    }
}

// This is one iteration of the 2nd stage of
// resolve_frame_info + add_stack() pipeline. Builds the stack for a given framepacket.
// When pipeline optimization is turnd off, cstctxchunk passed is NULL
void
CallStackP::add_stack (DataDescriptor *dDscr, long idx, FramePacket *frp,
		       cstk_ctx_chunk* cstCtxChunk)
{
  Vector<DbeInstr*> *natpcs = NULL;
  cstk_ctx *cstctx = NULL;
  int stack_size = frp->stackSize ();
  if (cstCtxChunk != NULL)
    {
      cstctx = cstCtxChunk->cstCtxAr[idx % CSTCTX_CHUNK_SZ];
      natpcs = cstctx->natpcs;
      natpcs->reset ();
    }
  if (natpcs == NULL)
    {
      // this is when we are not doing the pipeline optimization
      // Temporary array for resolved addresses
      // [leaf_pc .. root_pc] == [0..stack_size-1]
      // Leave room for a possible "truncated" frame
      if (natpcsP == NULL)
	natpcsP = new Vector<DbeInstr*>;
      natpcs = natpcsP;
      natpcs->reset ();
    }

  bool leaf = true;
  hrtime_t tstamp = (hrtime_t) dDscr->getLongValue (PROP_TSTAMP, idx);
  uint32_t thrid = (uint32_t) dDscr->getIntValue (PROP_THRID, idx);

  enum
  {
    NONE,
    CHECK_O7,
    USE_O7,
    SKIP_O7
  } state = NONE;

  Vaddr o7_to_skip = 0;
  for (int index = 0; index < stack_size; index++)
    {
      if (frp->isLeafMark (index))
	{
	  state = CHECK_O7;
	  continue;
	}

      if (state == SKIP_O7)
	{
	  // remember this bad o7 value since OMP might not recognize it
	  o7_to_skip = frp->getFromStack (index);
	  state = NONE;
	  continue;
	}

      Vaddr va = frp->getFromStack (index);
      DbeInstr *cur_instr = experiment->map_Vaddr_to_PC (va, tstamp);
#if ARCH(Intel)// TBR? FIXUP_XXX_SPARC_LINUX: switch should be on experiment ARCH, not dbe ARCH
      // We need to adjust return addresses on intel
      // in order to attribute inclusive metrics to
      // proper call instructions.
      if (experiment->exp_maj_version <= 9)
	if (!leaf && cur_instr->addr != 0)
	  cur_instr = cur_instr->func->find_dbeinstr (0, cur_instr->addr - 1);
#endif

      // Skip PC's from PLT, update leaf and state accordingly
      if ((cur_instr->func->flags & FUNC_FLAG_PLT)
	   && (leaf || state == CHECK_O7))
	{
	  if (state == CHECK_O7)
	    state = USE_O7;
	  leaf = false;
	  continue;
	}
      if (state == CHECK_O7)
	{
	  state = USE_O7;
	  uint64_t saddr = cur_instr->func->save_addr;
	  if (cur_instr->func->isOutlineFunction)
	    // outline functions assume 'save' instruction
	    // Note: they accidentally have saddr == FUNC_ROOT
	    state = SKIP_O7;
	  else if (saddr == FUNC_ROOT)
	    {
	      // If a function is statically determined as a root
	      // but dynamically appears not, don't discard o7.
	      // One such case is __misalign_trap_handler on sparcv9.
	      if (stack_size == 3)
		state = SKIP_O7;
	    }
	  else if (saddr != FUNC_NO_SAVE && cur_instr->addr > saddr)
	    state = SKIP_O7;
	}
      else if (state == USE_O7)
	{
	  state = NONE;
	  if (cur_instr->flags & PCInvlFlag)
	    continue;
	}
      if (leaf)
	{
	  Vaddr evpc = (Vaddr) dDscr->getLongValue (PROP_VIRTPC, idx);
	  if (evpc != 0
	      && !(index > 0 && frp->isLeafMark (index - 1)
		   && evpc == (Vaddr) (-1)))
	    {
	      /* contains hwcprof info */
	      cur_instr->func->module->read_hwcprof_info ();

	      // complete ABS validation of candidate eventPC/eventEA
	      // and correction/adjustment of collected callstack leaf PC
	      DbeInstr *candPC = experiment->map_Vaddr_to_PC (evpc, tstamp);
	      Vaddr vaddr = (Vaddr) dDscr->getLongValue (PROP_VADDR, idx);
	      Vaddr tmp_vaddr = vaddr;
	      int abst_type;
	      uint32_t tag = dDscr->getIntValue (PROP_HWCTAG, idx);
	      if (tag < 0 || tag >= MAX_HWCOUNT)
		abst_type = ABST_NOPC;
	      else
		abst_type = experiment->coll_params.hw_tpc[tag];

	      // We need to adjust addresses for ABST_EXACT_PEBS_PLUS1
	      // (Nehalem/SandyBridge PEBS identifies PC+1, not PC)
	      if (abst_type == ABST_EXACT_PEBS_PLUS1 && candPC->addr != 0)
		candPC = candPC->func->find_dbeinstr (0, candPC->func->find_previous_addr (candPC->addr));

	      cur_instr = adjustEvent (cur_instr, candPC, tmp_vaddr, abst_type);
	      if (vaddr != tmp_vaddr)
		{
		  if (tmp_vaddr < ABS_CODE_RANGE)
		    {
		      /* post processing backtrack failed */
		      dDscr->setValue (PROP_VADDR, idx, tmp_vaddr);
		      dDscr->setValue (PROP_PADDR, idx, ABS_NULL);
		      /* hwcp->eventVPC =  xxxxx leave eventPC alone,
		       *   or can we set it to leafpc? */
		      dDscr->setValue (PROP_PHYSPC, idx, ABS_NULL);
		    }
		  else
		    {
		      /* internal error: why would post-processing modify vaddr? */
		      dDscr->setValue (PROP_PADDR, idx, (Vaddr) (-1));
		      dDscr->setValue (PROP_PHYSPC, idx, (Vaddr) (-1));
		    }
		}
	    }
	}
      natpcs->append (cur_instr);
      leaf = false;

      // A hack to deceive the user into believing that outlined code
      // is called from the base function
      DbeInstr *drvd = cur_instr->func->derivedNode;
      if (drvd != NULL)
	natpcs->append (drvd);
    }
  if (frp->isTruncatedStack ())
    {
      Function *truncf = dbeSession->getSpecialFunction (DbeSession::TruncatedStackFunc);
      natpcs->append (truncf->find_dbeinstr (0, 0));
    }
  else if (frp->isFailedUnwindStack ())
    {
      Function *funwf = dbeSession->getSpecialFunction (DbeSession::FailedUnwindFunc);
      natpcs->append (funwf->find_dbeinstr (0, 0));
    }

  CallStackNode *node = (CallStackNode*) add_stack ((Vector<Histable*>*)natpcs);
  dDscr->setObjValue (PROP_MSTACK, idx, node);
  dDscr->setObjValue (PROP_XSTACK, idx, node);
  dDscr->setObjValue (PROP_USTACK, idx, node);

  // OpenMP 3.0 stacks
  stack_size = frp->ompstack->size ();
  if (stack_size > 0 || frp->omp_state == OMP_IDLE_STATE)
    {
      Function *func;
      Vector<Histable*> *omppcs = new Vector<Histable*>(stack_size);
      Vector<Histable*> *ompxpcs = new Vector<Histable*>(stack_size);
      switch (frp->omp_state)
	{
	case OMP_IDLE_STATE:
	case OMP_RDUC_STATE:
	case OMP_IBAR_STATE:
	case OMP_EBAR_STATE:
	case OMP_LKWT_STATE:
	case OMP_CTWT_STATE:
	case OMP_ODWT_STATE:
	case OMP_ATWT_STATE:
	  {
	    func = dbeSession->get_OMP_Function (frp->omp_state);
	    DbeInstr *instr = func->find_dbeinstr (0, 0);
	    omppcs->append (instr);
	    ompxpcs->append (instr);
	    break;
	  }
	}
      Vector<Vaddr> *stck = frp->ompstack;
      leaf = true;
      for (int index = 0; index < stack_size; index++)
	{
	  if (stck->fetch (index) == SP_LEAF_CHECK_MARKER)
	    {
	      state = CHECK_O7;
	      continue;
	    }
	  if (state == SKIP_O7)
	    {
	      state = NONE;
	      continue;
	    }

	  // The OMP stack might not have enough information to know to discard a bad o7.
	  // So just remember what the native stack skipped.
	  if (o7_to_skip == stck->fetch (index))
	    {
	      state = NONE;
	      continue;
	    }
	  Vaddr va = stck->fetch (index);
	  DbeInstr *cur_instr = experiment->map_Vaddr_to_PC (va, tstamp);

	  // Skip PC's from PLT, update leaf and state accordingly
	  if ((cur_instr->func->flags & FUNC_FLAG_PLT) &&
	      (leaf || state == CHECK_O7))
	    {
	      if (state == CHECK_O7)
		state = USE_O7;
	      leaf = false;
	      continue;
	    }
	  if (state == CHECK_O7)
	    {
	      state = USE_O7;
	      uint64_t saddr = cur_instr->func->save_addr;
	      if (cur_instr->func->isOutlineFunction)
		// outline functions assume 'save' instruction
		// Note: they accidentally have saddr == FUNC_ROOT
		state = SKIP_O7;
	      else if (saddr == FUNC_ROOT)
		{
		  // If a function is statically determined as a root
		  // but dynamically appears not, don't discard o7.
		  // One such case is __misalign_trap_handler on sparcv9.
		  if (stack_size == 3)
		    state = SKIP_O7;
		}
	      else if (saddr != FUNC_NO_SAVE && cur_instr->addr > saddr)
		state = SKIP_O7;
	    }
	  else if (state == USE_O7)
	    {
	      state = NONE;
	      if (cur_instr->flags & PCInvlFlag)
		continue;
	    }

	  DbeLine *dbeline = (DbeLine*) cur_instr->convertto (Histable::LINE);
	  if (cur_instr->func->usrfunc)
	    {
	      dbeline = dbeline->sourceFile->find_dbeline (cur_instr->func->usrfunc, dbeline->lineno);
	      omppcs->append (dbeline);
	    }
	  else if (dbeline->lineno > 0)
	    omppcs->append (dbeline);
	  else
	    omppcs->append (cur_instr);
	  if (dbeline->is_set (DbeLine::OMPPRAGMA) &&
	      frp->omp_state == OMP_WORK_STATE)
	    dDscr->setValue (PROP_OMPSTATE, idx, OMP_OVHD_STATE);
	  ompxpcs->append (cur_instr);
	  leaf = false;
	}
      if (frp->omptruncated == SP_TRUNC_STACK_MARKER)
	{
	  func = dbeSession->getSpecialFunction (DbeSession::TruncatedStackFunc);
	  DbeInstr *instr = func->find_dbeinstr (0, 0);
	  omppcs->append (instr);
	  ompxpcs->append (instr);
	}
      else if (frp->omptruncated == SP_FAILED_UNWIND_MARKER)
	{
	  func = dbeSession->getSpecialFunction (DbeSession::FailedUnwindFunc);
	  DbeInstr *instr = func->find_dbeinstr (0, 0);
	  omppcs->append (instr);
	  ompxpcs->append (instr);
	}

      // User model call stack
      node = (CallStackNode*) add_stack (omppcs);
      dDscr->setObjValue (PROP_USTACK, idx, node);
      delete omppcs;

      // Expert call stack
      node = (CallStackNode*) add_stack (ompxpcs);
      dDscr->setObjValue (PROP_XSTACK, idx, node);
      delete ompxpcs;
      dDscr->setObjValue (PROP_JTHREAD, idx, JTHREAD_DEFAULT);
      return;
    }

  // OpenMP 2.5 stacks
  if (frp->omp_cprid || frp->omp_state)
    {
      DataView *dview = experiment->getOpenMPdata ();
      if (dview == NULL)
	{
	  // It appears we may get OMP_SERL_STATE from a passive libmtsk
	  dDscr->setObjValue (PROP_JTHREAD, idx, JTHREAD_DEFAULT);
	  return;
	}
      if (dview->getDataDescriptor () == dDscr)
	{
	  // Don't process the user stack for OpenMP fork events yet
	  dDscr->setObjValue (PROP_USTACK, idx, (void*) NULL);
	  dDscr->setObjValue (PROP_JTHREAD, idx, JTHREAD_DEFAULT);
	  return;
	}
      Vector<Histable*> *omppcs = new Vector<Histable*>(stack_size);

      // Construct OMP user stack
      // Find the bottom frame
      int btm = 0;
      switch (frp->omp_state)
	{
	case OMP_IDLE_STATE:
	  {
	    Function *func = dbeSession->get_OMP_Function (frp->omp_state);
	    omppcs->append (func->find_dbeinstr (0, 0));
	    // XXX: workaround for inconsistency between OMP_IDLE_STATE
	    // and omp_cprid != 0
	    frp->omp_cprid = 0;
	    btm = natpcs->size ();
	    break;
	  }
	case OMP_RDUC_STATE:
	case OMP_IBAR_STATE:
	case OMP_EBAR_STATE:
	case OMP_LKWT_STATE:
	case OMP_CTWT_STATE:
	case OMP_ODWT_STATE:
	case OMP_ATWT_STATE:
	  {
	    Function *func = dbeSession->get_OMP_Function (frp->omp_state);
	    omppcs->append (func->find_dbeinstr (0, 0));
	    bool inOMP = false;
	    for (btm = 0; btm < natpcs->size (); btm++)
	      {
		LoadObject *lo = natpcs->fetch (btm)->func->module->loadobject;
		if (!inOMP)
		  {
		    if (lo->flags & SEG_FLAG_OMP)
		      inOMP = true;
		  }
		else if (!(lo->flags & SEG_FLAG_OMP))
		  break;
	      }
	    break;
	  }
	case OMP_NO_STATE:
	case OMP_WORK_STATE:
	case OMP_SERL_STATE:
	default:
	  break;
	}

      // Find the top frame
      int top = -1;
      switch (frp->omp_state)
	{
	case OMP_IDLE_STATE:
	  break;
	default:
	  {
	    dview->sort (PROP_CPRID);
	    Datum tval;
	    tval.setUINT64 (frp->omp_cprid);
	    long pidx = dview->getIdxByVals (&tval, DataView::REL_EQ);
	    if (pidx < 0)   // No parent. Process the entire nat_stack
	      top = natpcs->size () - 1;
	    else
	      {
		uint32_t pthrid = (uint32_t) dview->getIntValue (PROP_THRID, pidx);
		if (thrid != pthrid)
		  {
		    // Parent is on a different stack.
		    // Process the entire nat_stack. Skip libthread.
		    for (top = natpcs->size () - 1; top >= 0; top--)
		      {
			DbeInstr *instr = natpcs->fetch (top);
			if (instr->func->module->loadobject->flags & SEG_FLAG_OMP)
			  break;
		      }
		    if (top < 0) // None found. May be incomplete call stack
		      top = natpcs->size () - 1;
		  }
		else
		  {
		    // Parent is on the same stack. Find match.
		    top = natpcs->size () - 1;
		    void *pnat_stack = dview->getObjValue (PROP_MSTACK, pidx);
		    Vector<Histable*> *ppcs = getStackPCs (pnat_stack);
		    for (int ptop = ppcs->size () - 1; top >= 0 && ptop >= 0;
			    top--, ptop--)
		      {
			if (natpcs->fetch (top) != ppcs->fetch (ptop))
			  break;
		      }
		    delete ppcs;
		  }
	      }
	    // If no frames are found for Barrier/Reduction save at least one
	    if ((frp->omp_state == OMP_RDUC_STATE
		 || frp->omp_state == OMP_IBAR_STATE
		 || frp->omp_state == OMP_EBAR_STATE)
		&& top < btm && btm < natpcs->size ())
	      top = btm;
	  }
	}
      for (int i = btm; i <= top; ++i)
	{
	  DbeInstr *instr = natpcs->fetch (i);
	  if (instr->func->module->loadobject->flags & SEG_FLAG_OMP)
	    continue; // Skip all frames from libmtsk
	  omppcs->append (instr);
	}
      node = find_preg_stack (frp->omp_cprid);
      while (node != root)
	{
	  omppcs->append (node->instr);
	  node = node->ancestor;
	}
      node = (CallStackNode *) add_stack (omppcs);
      dDscr->setObjValue (PROP_USTACK, idx, node);
      delete omppcs;
      dDscr->setObjValue (PROP_JTHREAD, idx, JTHREAD_DEFAULT);
      return;
    }

  // Construct Java user stack
  add_stack_java (dDscr, idx, frp, tstamp, thrid, natpcs, true, NULL);
}

// adjustment of leafPC/eventVA for XHWC packets with candidate eventPC
//  Called from CallStack during initial processing of the events
DbeInstr *
CallStackP::adjustEvent (DbeInstr *leafPC, DbeInstr *candPC, Vaddr &eventVA,
			 int abst_type)
{
  // increment counter of dataspace events
  experiment->dsevents++;
  bool isPrecise;
  if (abst_type == ABST_EXACT_PEBS_PLUS1)
    isPrecise = true;
  else if (abst_type == ABST_EXACT)
    isPrecise = true;
  else
    isPrecise = false;

  if (isPrecise)
    /* precise backtracking */
    /* assume within 1 instruction of leaf (this could be checked here) */
    // no change to eventVA or candPC
    return candPC;

  Function *func = leafPC->func;
  unsigned int bt_entries = func->module->bTargets.size ();
  DbeInstr *bestPC = NULL;

  // bt == branch target (potential destination of a branch
  if (bt_entries == 0)
    { // no XHWCprof info for this module
      // increment counter
      experiment->dsnoxhwcevents++;

      // see if event is to be processed anyway
      if (!dbeSession->check_ignore_no_xhwcprof ())
	{
	  // Don't ignore error
	  // XXX -- set error code in event VA -- replace with other mechanism
	  if (eventVA > ABS_CODE_RANGE)
	    eventVA = ABS_NULL;
	  eventVA |= ABS_NO_CTI_INFO; // => effective address can't be validated
	  bestPC = leafPC; // => no PC correction possible
	}
      else
	bestPC = candPC; // assume the event valid
    }
  else
    {
      // we have the info to verify the backtracking
      target_info_t *bt;
      int bt_entry = bt_entries;
      uint64_t leafPC_offset = func->img_offset + leafPC->addr;
      uint64_t candPC_offset = candPC->func->img_offset + candPC->addr;
      do
	{
	  bt_entry--;
	  bt = func->module->bTargets.fetch (bt_entry);
	  /* bts seem to be sorted by offset, smallest to largest */
	}
      while (bt_entry > 0 && bt->offset > leafPC_offset);
      /* if bt_entry == 0, all items have been checked */

      if (bt->offset > leafPC_offset)
	{ /* XXXX isn't is possible that all bt's are after leafPC_offset? */
	  bestPC = leafPC; // actual event PC can't be determined
	  if (eventVA > ABS_CODE_RANGE)
	    eventVA = ABS_NULL;
	  eventVA |= ABS_INFO_FAILED; // effective address can't be validated
	}
      else if (bt->offset > candPC_offset)
	{
	  // use synthetic PC corresponding to bTarget
	  bestPC = func->find_dbeinstr (PCTrgtFlag, bt->offset - func->img_offset);
	  if (eventVA > ABS_CODE_RANGE)
	    eventVA = ABS_NULL;
	  eventVA |= ABS_CTI_TARGET; // effective  address can't be validated
	}
      else
	bestPC = candPC;    // accept provided virtual address as valid
    }
  return bestPC;
}

void *
CallStackP::add_stack_d (Vector<Histable*> *objs)
{
  // objs: root..leaf
  // Reverse objs
  for (int i = 0, j = objs->size () - 1; i < j; ++i, --j)
    objs->swap (i, j);
  return add_stack (objs);
}

CallStackNode::CallStackNode (CallStackNode *_ancestor, Histable *_instr)
{
  ancestor = _ancestor;
  instr = _instr;
  alt_node = NULL;
}

CallStackNode::~CallStackNode () { }

bool
CallStackNode::compare (long start, long end, Vector<Histable*> *objs, CallStackNode *mRoot)
{
  CallStackNode *p = this;
  for (long i = start; i < end; i++, p = p->get_ancestor ())
    if (p == NULL || p->get_instr () != objs->get (i))
      return false;
  return p == mRoot;
}

void
CallStackNode::dump ()
{
  const char *s = "";
  int sz = 0;
  for (CallStackNode *p = this; p; p = p->get_ancestor ())
    {
      fprintf (stderr, NTXT ("%.*s 0x%08llx id=0x%08llx %s\n"), sz, s,
	       (long long) p, (long long) p->get_instr ()->id,
	       STR (p->get_instr ()->get_name ()));
      s = "-";
      sz += 1;
    }
}

long total_calls_add_stack, total_stacks, total_nodes, call_stack_size[201];

void *
CallStackP::add_stack (Vector<Histable*> *objs)
{
  // objs: leaf..root
  uint64_t hash = objs->size ();
  for (long i = objs->size () - 1; i >= 0; --i)
    hash ^= (unsigned long long) objs->get (i);

  uint64_t key = hash ? hash : 1;
  CallStackNode *node = cstackMap->get (key);
#ifdef DEBUG
  if (DUMP_CALL_STACK)
    {
      total_calls_add_stack++;
      call_stack_size[objs->size () > 200 ? 200 : objs->size ()]++;
      Dprintf (DUMP_CALL_STACK,
	       "add_stack: %lld size=%lld  key=0x%08llx cashNode=0x%08llx\n",
	       (long long) total_calls_add_stack, (long long) objs->size (),
	       (long long) key, (long long) node);
      for (long i = 0, sz = VecSize (objs); i < sz; i++)
	Dprintf (DUMP_CALL_STACK, "  add_stack: %.*s 0x%08llx id=0x%08llx %s\n",
		 (int) i, NTXT (" "), (long long) objs->get (i),
		 (long long) objs->get (i)->id, STR (objs->get (i)->get_name ()));
    }
#endif
  if (node && node->compare (0, objs->size (), objs, root))
    {
      Dprintf (DUMP_CALL_STACK, NTXT ("STACK FOUND: key=0x%08llx 0x%08llx id=0x%08llx %s\n"),
	       (long long) key, (long long) node,
	       (long long) node->get_instr ()->id,
	       STR (node->get_instr ()->get_name ()));
      return node;
    }
  node = root;
  for (long i = objs->size () - 1; i >= 0; i--)
    {
      Histable *instr = objs->get (i);
      int old_count = node->count;
      int left;
      CallStackNode *nd = node->find (instr, &left);
      if (nd)
	{
	  node = nd;
	  continue;
	}
      cstackLock->aquireLock (); // Use one lock for all nodes
      // node->aquireLock();
      if (old_count != node->count)
	{
	  nd = node->find (instr, &left);
	  if (nd)
	    { // the other thread has created this node
	      cstackLock->releaseLock ();
	      // node->releaseLock();
	      node = nd;
	      continue;
	    }
	}
      // New Call Stack
      total_stacks++;
      nd = node;
      CallStackNode *first = NULL;
      do
	{
	  CallStackNode *anc = node;
	  total_nodes++;
	  node = new_Node (anc, objs->get (i));
	  if (first)
	    anc->append (node);
	  else
	    first = node;
	}
      while (i-- > 0);
      nd->insert (left, first);
      cstackLock->releaseLock ();
      // nd->releaseLock();
      break;
    }
  cstackMap->put (key, node);
  if (DUMP_CALL_STACK)
    node->dump ();
  return node;
}

CallStackNode *
CallStackP::get_node (int n)
{
  if (n < nodes)
    return &chunks[n / CHUNKSZ][n % CHUNKSZ];
  return NULL;
}

/*
 *  Debugging methods
 */
void
CallStackP::print (FILE *fd)
{
  FILE *f = (fd == NULL ? stderr : fd);
  fprintf (f, GTXT ("CallStack: nodes = %d\n\n"), nodes);
  int maxdepth = 0;
  int maxwidth = 0;
  const char *t;
  char *n;
  for (int i = 0; i < nodes; i++)
    {
      CallStackNode *node = &chunks[i / CHUNKSZ][i % CHUNKSZ];
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
      fprintf (f, GTXT ("node: 0x%016llx anc: 0x%016llx -- 0x%016llX:  %s %s\n"),
	       (unsigned long long) node, (unsigned long long) node->ancestor,
	       addr, t, n);
    }
  fprintf (f, GTXT ("md = %d, mw = %d\n"), maxdepth, maxwidth);
}

/*
 *  Static CallStack methods
 */
CallStack *
CallStack::getInstance (Experiment *exp)
{
  return new CallStackP (exp);
}

int
CallStack::stackSize (void *stack)
{
  CallStackNode *node = (CallStackNode *) stack;
  int sz = 0;
  for (; node; node = node->ancestor)
    sz++;
  return sz - 1; // don't count the root node
}

Histable *
CallStack::getStackPC (void *stack, int n)
{
  CallStackNode *node = (CallStackNode *) stack;
  while (n-- && node)
    node = node->ancestor;
  if (node == NULL)
    return dbeSession->get_Unknown_Function ()->find_dbeinstr (PCInvlFlag, 0);
  return node->instr;
}

Vector<Histable*> *
CallStack::getStackPCs (void *stack, bool get_hide_stack)
{
  Vector<Histable*> *res = new Vector<Histable*>;
  CallStackNode *node = (CallStackNode *) stack;
  if (get_hide_stack && node->alt_node != NULL)
    node = node->alt_node;
  while (node && node->ancestor)
    { // skip the root node
      res->append (node->instr);
      node = node->ancestor;
    }
  return res;
}

int
CallStack::compare (void *stack1, void *stack2)
{
  // Quick comparision
  if (stack1 == stack2)
    return 0;

  CallStackNode *node1 = (CallStackNode *) stack1;
  CallStackNode *node2 = (CallStackNode *) stack2;
  while (node1 != NULL && node2 != NULL)
    {
      //to keep the result const on different platforms
      //we use instr->id instead of instr
      if (node1->instr->id < node2->instr->id)
	return -1;
      else if (node1->instr->id > node2->instr->id)
	return 1;
      node1 = node1->ancestor;
      node2 = node2->ancestor;
    }
  if (node1 == NULL && node2 != NULL)
    return -1;
  else if (node1 != NULL && node2 == NULL)
    return 1;
  else
    return 0;
}

// LIBRARY VISIBILITY

void
CallStack::setHideStack (void *stack, void *hideStack)
{
  CallStackNode *hNode = (CallStackNode *) stack;
  hNode->alt_node = (CallStackNode *) hideStack;
}
