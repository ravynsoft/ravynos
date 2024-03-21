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
#include <assert.h>
#include "CallStack.h"
#include "DbeSession.h"
#include "DbeView.h"
#include "DataObject.h"
#include "Exp_Layout.h"
#include "Experiment.h"
#include "Module.h"
#include "LoadObject.h"
#include "Expression.h"
#include "Function.h"
#include "Histable.h"
#include "Sample.h"
#include "Table.h"

//////////////////////////////////////////////////////////
//  class Expression::Context

static const uint64_t INDXOBJ_EXPGRID_SHIFT = 60;
static const uint64_t INDXOBJ_EXPID_SHIFT   = 32;

Expression::Context::Context (DbeView *_dbev, Experiment *_exp)
{
  dbev = _dbev;
  exp = _exp;
  dview = NULL;
  eventId = 0;
}

Expression::Context::Context (DbeView *_dbev, Experiment *_exp,
			      DataView *_dview, long _eventId)
{
  dbev = _dbev;
  exp = _exp;
  dview = _dview;
  eventId = _eventId;
}

//////////////////////////////////////////////////////////
//  class Expression
Expression::Expression (OpCode _op, uint64_t _v)
{
  op = _op;
  v = Value (_v);
  arg0 = NULL;
  arg1 = NULL;
}

Expression::Expression (OpCode _op, const Expression *_arg0,
			const Expression *_arg1)
{
  op = _op;
  v = Value ();
  arg0 = NULL;
  arg1 = NULL;
  if (_arg0)
    arg0 = _arg0->copy ();
  if (_arg1)
    arg1 = _arg1->copy ();
}

Expression::~Expression ()
{
  delete arg0;
  delete arg1;
}

Expression::Expression (const Expression &rhs)
{
  op = rhs.op;
  arg0 = NULL;
  arg1 = NULL;
  v = Value (rhs.v);
  if (rhs.arg0)
    {
      arg0 = rhs.arg0->copy ();
      if (v.next)
	{
	  assert (arg0 && v.next == &(rhs.arg0->v));
	  v.next = &(arg0->v);
	}
    }
  if (rhs.arg1)
    arg1 = rhs.arg1->copy ();
}

Expression::Expression (const Expression *rhs)
{
  arg0 = NULL;
  arg1 = NULL;
  copy (rhs);
}

void
Expression::copy (const Expression *rhs)
{
  op = rhs->op;
  delete arg0;
  delete arg1;
  arg0 = NULL;
  arg1 = NULL;
  v = Value (rhs->v);
  if (rhs->arg0)
    {
      arg0 = rhs->arg0->copy ();
      if (v.next)
	{
	  assert (arg0 && v.next == &(rhs->arg0->v));
	  v.next = &(arg0->v);
	}
    }
  if (rhs->arg1)
    arg1 = rhs->arg1->copy ();
}

Expression &
Expression::operator= (const Expression &rhs)
{
  if (this == &rhs)
    return *this;
  copy (&rhs);
  return *this;
}

bool
Expression::getVal (int propId, Context *ctx)
{
  v.val = 0;
  v.next = NULL;
  int origPropId = propId;
  switch (propId)
    {
    default:
      {
	if (!ctx->dview)
	  return false;
	PropDescr *propDscr = ctx->dview->getProp (propId);
	if (!propDscr)
	  return false;
	switch (propDscr->vtype)
	  {
	  case TYPE_INT32:
	    v.val = ctx->dview->getIntValue (propId, ctx->eventId);
	    break;
	  case TYPE_UINT32:
	    v.val = (uint32_t) ctx->dview->getIntValue (propId, ctx->eventId); //prevent sign extension
	    break;
	  case TYPE_INT64:
	  case TYPE_UINT64:
	    v.val = ctx->dview->getLongValue (propId, ctx->eventId);
	    break;
	  case TYPE_OBJ:
	    // YM: not sure if we should allow this
	    v.val = (long long) ctx->dview->getObjValue (propId, ctx->eventId);
	    break;
	  case TYPE_STRING:
	  case TYPE_DOUBLE:
	  default:
	    return false; // Weird, programming error?
	  }
	break;
      }
    case PROP_FREQ_MHZ:
      if (ctx->exp && ctx->exp->clock)
	v.val = ctx->exp->clock;
      else
	return false;
      break;
    case PROP_PID:
      if (ctx->exp == NULL)
	return false;
      v.val = ctx->exp->getPID ();
      break;
    case PROP_EXPID:
      if (ctx->exp == NULL)
	return false;
      v.val = ctx->exp->getUserExpId ();
      break;
    case PROP_EXPID_CMP:
      if (ctx->exp == NULL)
	return false;
      else
	{
	  Experiment *exp = ctx->exp;
	  if (ctx->dbev && ctx->dbev->comparingExperiments ())
	    exp = (Experiment *) exp->get_compare_obj ();
	  v.val = exp->getUserExpId ();
	}
      break;
    case PROP_EXPGRID:
      if (ctx->exp == NULL)
	return false;
      v.val = ctx->exp->groupId;
      break;
    case PROP_NTICK_USEC:
      if (ctx->exp == NULL)
	return false;
      if (ctx->dview && ctx->dview->getProp (PROP_NTICK))
	v.val = ctx->dview->getIntValue (PROP_NTICK, ctx->eventId)
		* ctx->exp->get_params ()->ptimer_usec;
      else
	return false;
      break;
    case PROP_ATSTAMP:
    case PROP_ETSTAMP:
      if (ctx->exp == NULL)
	return false;
      if (ctx->dview && ctx->dview->getProp (PROP_TSTAMP))
	v.val = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
      else
	return false;
      if (propId == PROP_ATSTAMP)
	break; // absolute time, no adjustments
      // propId==PROP_ETSTAMP
      // calculate relative time from start of this experiment
      v.val -= ctx->exp->getStartTime ();
      break;
    case PROP_TSTAMP:
    case PROP_TSTAMP_LO:
    case PROP_TSTAMP_HI:
      {
	if (ctx->exp == NULL)
	  return false;
	if (!(ctx->dview && ctx->dview->getProp (PROP_TSTAMP)))
	  return false;
	hrtime_t tstamp = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
	// compute relative time from start of founder experiment
	v.val = tstamp - ctx->exp->getStartTime ()
		+ ctx->exp->getRelativeStartTime ();
	if (propId == PROP_TSTAMP)
	  break;
	if (ctx->dview->getProp (PROP_EVT_TIME))
	  {
	    hrtime_t delta = ctx->dview->getLongValue (PROP_EVT_TIME, ctx->eventId);
	    if (propId == PROP_TSTAMP_LO)
	      {
		if (delta > 0)
		  { // positive delta means TSTAMP is at end
		    // TSTAMP_LO = TSTAMP-delta
		    v.val -= delta;
		    break;
		  }
		break;
	      }
	    else
	      { // PROP_TSTAMP_HI
		if (delta < 0)
		  { // negative delta means TSTAMP is at start
		    // TSTAMP_HI = TSTAMP+(-delta)
		    v.val -= delta;
		    break;
		  }
		break;
	      }
	  }
	else if (ctx->dview->getProp (PROP_TSTAMP2))
	  {
	    if (propId == PROP_TSTAMP_HI)
	      {
		hrtime_t tstamp2 = ctx->dview->getLongValue (PROP_TSTAMP2,
							     ctx->eventId);
		if (tstamp2 == 0)
		  break; // if not initialized, event does not have duration
		if (tstamp2 == MAX_TIME)
		  tstamp2 = ctx->exp->getLastEvent ();
		hrtime_t delta = tstamp2 - tstamp;
		if (delta >= 0)
		  {
		    v.val += delta;
		    break;
		  }
		break; // weird, delta should not be negative
	      }
	    break; // PROP_TSTAMP_LO, no modification needed
	  }
	break; // should never be hit
      }
    case PROP_IOHEAPBYTES:
      {
	propId = PROP_IONBYTE;
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (propId))
	  { // has property?
	    propId = PROP_HSIZE;
	    if (!ctx->dview->getProp (propId))
	      return false;
	  }
	v.val = ctx->dview->getLongValue (propId, ctx->eventId);
	break;
      }
    case PROP_SAMPLE_MAP:
      {
	if (ctx->exp == NULL)
	  return false;
	if (ctx->dview == NULL)
	  return false;
	if (ctx->dview->getProp (PROP_SAMPLE))
	  v.val = ctx->dview->getIntValue (PROP_SAMPLE, ctx->eventId);
	else
	  { // does not have property, convert to time.
	    uint64_t tstamp;
	    tstamp = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
	    Sample *sample = ctx->exp->map_event_to_Sample (tstamp);
	    v.val = sample ? sample->get_number () : -1;
	  }
	break;
      }
    case PROP_GCEVENT_MAP:
      {
	if (ctx->exp == NULL)
	  return false;
	if (ctx->dview == NULL)
	  return false;
	if (ctx->dview->getProp (PROP_GCEVENT))
	  v.val = ctx->dview->getIntValue (PROP_GCEVENT, ctx->eventId);
	else
	  { // does not have property, convert to time.
	    uint64_t tstamp;
	    tstamp = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
	    GCEvent *gcevent = ctx->exp->map_event_to_GCEvent (tstamp);
	    v.val = gcevent ? gcevent->id : 0;
	  }
	break;
      }
    case PROP_LEAF:
      {
	if (ctx->dview == NULL)
	  return false;
	VMode vmode = ctx->dbev ? ctx->dbev->get_view_mode () : VMODE_USER;
	int prop_id;
	if (vmode == VMODE_MACHINE)
	  prop_id = PROP_MSTACK;
	else if (vmode == VMODE_EXPERT)
	  prop_id = PROP_XSTACK;
	else
	  prop_id = PROP_USTACK;
	if (!ctx->dview->getProp (prop_id))
	  return false;
	Histable *obj = CallStack::getStackPC (ctx->dview->getObjValue (prop_id, ctx->eventId), 0);
	Function *func = (Function*) obj->convertto (Histable::FUNCTION);
	v.val = func->id; // LEAF
	break;
      }
    case PROP_STACKID:
      {
	VMode vmode = ctx->dbev ? ctx->dbev->get_view_mode () : VMODE_USER;
	if (vmode == VMODE_MACHINE)
	  propId = PROP_MSTACK;
	else if (vmode == VMODE_EXPERT)
	  propId = PROP_XSTACK;
	else
	  propId = PROP_USTACK;
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (propId))
	  return false;
	v.val = (long) ctx->dview->getObjValue (propId, ctx->eventId);
	break;
      }
    case PROP_STACKL:
    case PROP_STACKI:
    case PROP_STACK:
      {
	VMode vmode = ctx->dbev ? ctx->dbev->get_view_mode () : VMODE_USER;
	if (vmode == VMODE_MACHINE)
	  propId = PROP_MSTACK;
	else if (vmode == VMODE_EXPERT)
	  propId = PROP_XSTACK;
	else
	  propId = PROP_USTACK;
      }
      // no break;
    case PROP_MSTACKL:
    case PROP_XSTACKL:
    case PROP_USTACKL:
    case PROP_MSTACKI:
    case PROP_XSTACKI:
    case PROP_USTACKI:
      switch (propId)
	{
	case PROP_MSTACKL:
	case PROP_MSTACKI:
	  propId = PROP_MSTACK;
	  break;
	case PROP_XSTACKL:
	case PROP_XSTACKI:
	  propId = PROP_XSTACK;
	  break;
	case PROP_USTACKL:
	case PROP_USTACKI:
	  propId = PROP_USTACK;
	  break;
	default:
	  break;
	}
      // no break;
    case PROP_MSTACK:
    case PROP_XSTACK:
    case PROP_USTACK:
      {
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (propId))
	  return false;
	bool hide_mode = !ctx->dbev->isShowAll ()
		|| ctx->dbev->isFilterHideMode ();
	Expression *cur = this;
	for (CallStackNode *stack = (CallStackNode *)
		ctx->dview->getObjValue (propId, ctx->eventId);
		stack; stack = stack->get_ancestor ())
	  {
	    Histable *hist = stack->get_instr ();
	    if (origPropId == PROP_STACK || origPropId == PROP_MSTACK
		|| origPropId == PROP_XSTACK || origPropId == PROP_USTACK)
	      {
		cur->v.val = hist->convertto (Histable::FUNCTION)->id;
		cur->v.fn = cur->v.val;
	      }
	    else if (origPropId == PROP_STACKL || origPropId == PROP_MSTACKL
		    || origPropId == PROP_XSTACKL || origPropId == PROP_USTACKL)
	      {
		cur->v.val = hist->convertto (Histable::LINE)->id;
		if (hide_mode)
		  cur->v.fn = hist->convertto (Histable::FUNCTION)->id;
		else
		  cur->v.fn = 0;
	      }
	    else if (origPropId == PROP_STACKI || origPropId == PROP_MSTACKI
		    || origPropId == PROP_XSTACKI || origPropId == PROP_USTACKI)
	      {
		cur->v.val = hist->convertto (Histable::INSTR)->id;
		if (hide_mode)
		  cur->v.fn = hist->convertto (Histable::FUNCTION)->id;
		else
		  cur->v.fn = 0;
	      }
	    if (cur->arg1 == NULL)
	      cur->arg1 = new Expression (OP_NONE, (uint64_t) 0);
	    if (stack->get_ancestor () == NULL)
	      {
		if (origPropId == PROP_STACKL || origPropId == PROP_MSTACKL
		    || origPropId == PROP_XSTACKL || origPropId == PROP_USTACKL
		    || origPropId == PROP_STACKI || origPropId == PROP_MSTACKI
		    || origPropId == PROP_XSTACKI || origPropId == PROP_USTACKI)
		  {
		    cur->v.next = NULL;
		    continue;
		  }
	      }
	    cur->v.next = &cur->arg1->v;
	    cur = cur->arg1;
	  }
	if (origPropId == PROP_STACK || origPropId == PROP_MSTACK
	    || origPropId == PROP_XSTACK || origPropId == PROP_USTACK)
	  {
	    cur->v.val = dbeSession->get_Total_Function ()->id;
	    cur->v.fn = cur->v.val;
	    cur->v.next = NULL;
	  }
	break;
      }
    case PROP_DOBJ:
      {
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (PROP_DOBJ))
	  return false;
	DataObject *dobj = (DataObject*)
		ctx->dview->getObjValue (PROP_DOBJ, ctx->eventId);
	if (dobj != NULL)
	  {
	    Expression *cur = this;
	    for (;;)
	      {
		cur->v.val = dobj->id;
		dobj = dobj->parent;
		if (dobj == NULL)
		  break;
		if (cur->arg1 == NULL)
		  cur->arg1 = new Expression (OP_NONE, (uint64_t) 0);
		cur->v.next = &cur->arg1->v;
		cur = cur->arg1;
	      }
	    cur->v.next = NULL;
	  }
	break;
      }
    case PROP_CPRID:
    case PROP_TSKID:
      {
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (propId))
	  return false;
	CallStackNode *ompstack = (CallStackNode *)
		ctx->dview->getObjValue (propId, ctx->eventId);
	Histable *hobj = ompstack->get_instr ();
	if (hobj != NULL)
	  v.val = hobj->id;
	break;
      }
    case PROP_JTHREAD:
      {
	if (ctx->exp == NULL)
	  return false;
	if (ctx->dview == NULL)
	  return false;
	if (!ctx->dview->getProp (propId))
	  return false;
	uint64_t tstamp;
	tstamp = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
	uint32_t thrid;
	uint64_t jthr_id = 0;
	thrid = ctx->dview->getIntValue (PROP_THRID, ctx->eventId);
	JThread *jthread = ctx->exp->map_pckt_to_Jthread (thrid, tstamp);
	if (jthread != JTHREAD_NONE && jthread != JTHREAD_DEFAULT)
	  {
	    jthr_id = jthread->jthr_id;
	    uint64_t grid = ctx->exp->groupId;
	    uint64_t expid = ctx->exp->getUserExpId ();
	    v.val = (grid << INDXOBJ_EXPGRID_SHIFT) |
		    (expid << INDXOBJ_EXPID_SHIFT) | jthr_id;
	  }
	break;
      }
    }
  return true;
}

bool
Expression::bEval (Context *ctx)
{
  uint64_t v0, v1;
  switch (op)
    {
    case OP_DEG:
      if (!arg1->bEval (ctx))
	return false;
      if (arg1->v.val < 0)
	{
	  v.val = 0;
	  return true;
	}
      if (!arg0->bEval (ctx))
	{
	  return false;
	}
      v0 = arg0->v.val;
      v1 = arg1->v.val;
      for (v.val = 1; v1 > 0; v1--)
	v.val *= v0;
      return true;
    case OP_MUL:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val * arg1->v.val;
	  return true;
	}
      return false;
    case OP_DIV:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v1 = arg1->v.val;
	  v.val = (v1 == 0) ? 0 : (arg0->v.val / v1);
	  return true;
	}
      return false;
    case OP_REM:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v1 = arg1->v.val;
	  v.val = (v1 == 0) ? 0 : (arg0->v.val % v1);
	  return true;
	}
      return false;
    case OP_ADD:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val + arg1->v.val;
	  // DBFIXME LIBRARY VISIBILITY
	  // hack to pass v.fn value to new expression for leaf filters USTACK+0
	  v.fn = arg0->v.fn + arg1->v.fn;
	  return true;
	}
      return false;
    case OP_MINUS:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val - arg1->v.val;
	  return true;
	}
      return false;
    case OP_LS:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val << arg1->v.val;
	  return true;
	}
      return false;
    case OP_RS:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val >> arg1->v.val;
	  return true;
	}
      return false;
    case OP_LT:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val < arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_LE:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val <= arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_GT:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val > arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_GE:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val >= arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_EQ:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val == arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_NE:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val != arg1->v.val ? 1 : 0;
	  return true;
	}
      return false;
    case OP_BITAND:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val & arg1->v.val;
	  return true;
	}
      return false;
    case OP_BITXOR:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val ^ arg1->v.val;
	  return true;
	}
      return false;
    case OP_BITOR:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v.val = arg0->v.val | arg1->v.val;
	  return true;
	}
      return false;
    case OP_AND:
      if (arg0->bEval (ctx))
	{
	  if (arg0->v.val == 0)
	    {
	      v.val = 0;
	      return true;
	    }
	  if (arg1->bEval (ctx))
	    {
	      v.val = arg1->v.val == 0 ? 0 : 1;
	      return true;
	    }
	  return false;
	}
      if (arg1->bEval (ctx) && arg1->v.val == 0)
	{
	  v.val = 0;
	  return true;
	}
      return false;
    case OP_OR:
      if (arg0->bEval (ctx))
	{
	  if (arg0->v.val != 0)
	    {
	      v.val = 1;
	      return true;
	    }
	  if (arg1->bEval (ctx))
	    {
	      v.val = arg1->v.val == 0 ? 0 : 1;
	      return true;
	    }
	  return false;
	}
      if (arg1->bEval (ctx) && arg1->v.val != 0)
	{
	  v.val = 1;
	  return true;
	}
      return false;
    case OP_NEQV:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v0 = arg0->v.val;
	  v1 = arg1->v.val;
	  v.val = (v0 == 0 && v1 != 0) || (v0 != 0 && v1 == 0) ? 1 : 0;
	  return true;
	}
      return false;
    case OP_EQV:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  v0 = arg0->v.val;
	  v1 = arg1->v.val;
	  v.val = (v0 == 0 && v1 == 0) || (v0 != 0 && v1 != 0) ? 1 : 0;
	  return true;
	}
      return false;
    case OP_QWE:
      if (arg0->bEval (ctx))
	{
	  if (arg0->v.val != 0)
	    {
	      if (arg1->arg0->bEval (ctx))
		{
		  v.val = arg1->arg0->v.val;
		  return true;
		}
	    }
	  else
	    {
	      if (arg1->arg1->bEval (ctx))
		{
		  v.val = arg1->arg1->v.val;
		  return true;
		}
	    }
	}
      return false;
    case OP_COMMA:
      if (arg0->bEval (ctx))
	{
	  v.next = &arg0->v;
	  if (arg1->bEval (ctx))
	    {
	      v.val = arg1->v.val;
	      return true;
	    }
	}
      return false;
    case OP_IN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *s = &arg0->v; s; s = s->next)
	    {
	      bool found = false;
	      for (Value *t = &arg1->v; t; t = t->next)
		{
		  if (t->val == s->val)
		    {
		      found = true;
		      break;
		    }
		}
	      if (!found)
		{
		  v.val = 0;
		  return true;
		}
	    }
	  v.val = 1;
	  return true;
	}
      return false;
    case OP_SOMEIN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *s = &arg0->v; s; s = s->next)
	    {
	      for (Value *t = &arg1->v; t; t = t->next)
		{
		  if (t->val == s->val)
		    {
		      v.val = 1;
		      return true;
		    }
		}
	    }
	  v.val = 0;
	  return true;
	}
      return false;
    case OP_ORDRIN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *t0 = &arg1->v; t0; t0 = t0->next)
	    {
	      bool found = true;
	      for (Value *s = &arg0->v, *t = t0; s; s = s->next, t = t->next)
		{
		  if (t == NULL || t->val != s->val)
		    {
		      found = false;
		      break;
		    }
		}
	      if (found)
		{
		  v.val = 1;
		  return true;
		}
	    }
	  v.val = 0;
	  return true;
	}
      return false;
      // LIBRARY_VISIBILITY
    case OP_LIBRARY_IN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *s = &arg0->v; s; s = s->next)
	    {
	      bool found = false;
	      uint64_t objId = s->val;
	      Histable *obj = dbeSession->findObjectById (objId);
	      bool libraryFound = false;
	      Function *fn;
	      if (obj != NULL && obj->get_type () == Histable::FUNCTION)
		{
		  fn = (Function *) obj;
		  if (fn->isHideFunc)
		    // this belongss to a loadobject in hide/library mode
		    libraryFound = true;
		}

	      if (libraryFound)
		{
		  uint64_t lo_id = fn->module->loadobject->id;
		  for (Value *t = &arg1->v; t; t = t->next)
		    {
		      uint64_t t_id = t->fn;
		      Histable *obj2 = dbeSession->findObjectById (t_id);
		      if (obj2 != NULL
			  && obj2->get_type () == Histable::FUNCTION)
			{
			  Function *func2 = (Function *) obj2;
			  uint64_t lo_id2 = func2->module->loadobject->id;
			  if (lo_id2 == lo_id)
			    {
			      found = true;
			      break;
			    }
			}
		    }
		}
	      else
		{
		  // Not a loadobject
		  for (Value *t = &arg1->v; t; t = t->next)
		    {
		      if (t->val == s->val)
			{
			  found = true;
			  break;
			}
		    }
		}
	      if (!found)
		{
		  v.val = 0;
		  return true;
		}
	    }
	  v.val = 1;
	  return true;
	}
      return false;
    case OP_LIBRARY_SOMEIN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *s = &arg0->v; s; s = s->next)
	    {
	      uint64_t objId = s->val;
	      Histable *obj = dbeSession->findObjectById (objId);
	      bool libraryFound = false;
	      Function *fn;
	      if (obj != NULL && obj->get_type () == Histable::FUNCTION)
		{
		  fn = (Function *) obj;
		  if (fn->isHideFunc)
		    // this belongs to a loadobject in hide/library mode
		    libraryFound = true;
		}

	      if (libraryFound)
		{
		  uint64_t lo_id = fn->module->loadobject->id;
		  for (Value *t = &arg1->v; t; t = t->next)
		    {
		      uint64_t t_id = t->fn;
		      Histable *obj2 = dbeSession->findObjectById (t_id);
		      if (obj2 != NULL && obj2->get_type () == Histable::FUNCTION)
			{
			  Function *func2 = (Function *) obj2;
			  uint64_t lo_id2 = func2->module->loadobject->id;
			  if (lo_id2 == lo_id)
			    {
			      v.val = 1;
			      return true;
			    }
			}
		    }
		}
	      else
		{
		  for (Value *t = &arg1->v; t; t = t->next)
		    if (t->val == s->val)
		      {
			v.val = 1;
			return true;
		      }
		}
	    }
	  v.val = 0;
	  return true;
	}
      return false;
    case OP_LIBRARY_ORDRIN:
      if (arg0->bEval (ctx) && arg1->bEval (ctx))
	{
	  for (Value *t0 = &arg1->v; t0; t0 = t0->next)
	    {
	      bool found = true;
	      Value *t = t0;
	      for (Value *s = &arg0->v; s; s = s->next)
		{
		  // start comparing s->val with t->val
		  // if matches move on to s->next and t->next
		  uint64_t objId = s->val;
		  Histable *obj = dbeSession->findObjectById (objId);
		  bool libraryFound = false;
		  Function *fn;
		  if (obj != NULL && obj->get_type () == Histable::FUNCTION)
		    {
		      fn = (Function *) obj;
		      if (fn->isHideFunc)
			libraryFound = true;
		    }
		  if (libraryFound)
		    {
		      // s->val is from a loadobject
		      // check if t->val is a func whose loadobject matches s->val
		      uint64_t lo_id = fn->module->loadobject->id;
		      uint64_t t_id = t->fn;
		      Histable *obj2 = dbeSession->findObjectById (t_id);
		      if (obj2 != NULL
			  && obj2->get_type () == Histable::FUNCTION)
			{
			  Function *func2 = (Function *) obj2;
			  uint64_t lo_id2 = func2->module->loadobject->id;
			  if (lo_id2 != lo_id)
			    {
			      // no match
			      found = false;
			      break;
			    }
			  else
			    {
			      // t->val is a func whose loadobject matches s->val
			      while (t != NULL && lo_id2 == lo_id)
				{
				  // skip frames with same load object
				  t = t->next;
				  t_id = t->fn;
				  obj2 = dbeSession->findObjectById (t_id);
				  if (obj2 != NULL
				      && obj2->get_type () == Histable::FUNCTION)
				    {
				      func2 = (Function *) obj2;
				      lo_id2 = func2->module->loadobject->id;
				    }
				}
			    }
			}
		    }
		  else
		    {
		      if (t == NULL || t->val != s->val)
			{
			  found = false;
			  break;
			}
		      t = t->next;
		    }
		}
	      if (found)
		{
		  v.val = 1;
		  return true;
		}
	    }
	  v.val = 0;
	  return true;
	}
      return false;
    case OP_BITNOT:
      if (arg0->bEval (ctx))
	{
	  v.val = ~arg0->v.val;
	  return true;
	}
      return false;
    case OP_NOT:
      if (arg0->bEval (ctx))
	{
	  v.val = !arg0->v.val;
	  return true;
	}
      return false;
    case OP_NUM:
      return true;
    case OP_NAME:
      if (ctx && arg0->bEval (ctx) && getVal ((int) arg0->v.val, ctx))
	return true;
      return false;
    case OP_FUNC:
      // FNAME is completely processed by pEval for now
      v.val = 0;
      return true;
    case OP_HASPROP:
      if (!ctx || !ctx->dview)
	return false; // can't be resolved (occurs during pEval() )
      else if (arg0->op != OP_NAME || !arg0->arg0)
	return false; // weird, wrong arg type
      else
	{
	  int propId = (int) arg0->arg0->v.val;
	  if (ctx->dview->getProp (propId))
	    v.val = 1;
	  else
	    v.val = 0;
	  return true;
	}
    case OP_FILE:
      // FILENAME is completely processed by pEval for now
      v.val = 0;
      return true;
    case OP_JAVA:
      //JGROUP & JPARENT is completely processed by pEval for now
      v.val = 0;
      return true;
    case OP_COLON:
      return false; // OK for arg1 of OP_QWE
    default:
#ifdef IPC_LOG
      fprintf (stderr, "INTERNAL ERROR: Expression::eval op=%d\n", op);
#endif
      return false;
    }
  return false;
}

Expression *
Expression::pEval (Context *ctx) // partial evaluation (dview may be NULL)
{
  Expression *res = NULL;
  switch (op)
    {
    case OP_FUNC:
      {
	Vector<Histable*> *objs = NULL;
	if (arg0->v.val == FUNC_FNAME)
	  {
	    Histable::NameFormat nfmt = ctx ? ctx->dbev->get_name_format () : Histable::NA;
	    objs = (Vector<Histable*>*)dbeSession->match_func_names ((char*) arg1->v.val, nfmt);
	  }
	else if (arg0->v.val == FUNC_DNAME)
	  objs = (Vector<Histable*>*)dbeSession->match_dobj_names ((char*) arg1->v.val);
	Expression *cur = new Expression (Expression::OP_NUM, (uint64_t) 0);
	res = cur;
	int i = objs ? objs->size () - 1 : -1;
	for (; i >= 0; i--)
	  {
	    cur->v.val = objs->fetch (i)->id;
	    if (i == 0)
	      break;
	    cur->arg0 = new Expression (OP_NONE, (uint64_t) 0);
	    cur->v.next = &cur->arg0->v;
	    cur = cur->arg0;
	  }
	cur->v.next = NULL;
	if (objs)
	  delete objs;
	break;
      }
    case OP_JAVA:
      {
	Vector<JThread*> *objs = NULL;
	Vector<uint64_t> *grids = NULL;
	Vector<uint64_t> *expids = NULL;
	if (arg0->v.val == JAVA_JGROUP)
	  objs = dbeSession->match_java_threads ((char*) arg1->v.val, 0, grids,
						 expids);
	else if (arg0->v.val == JAVA_JPARENT)
	  objs = dbeSession->match_java_threads ((char*) arg1->v.val, 1, grids,
						 expids);
	Expression *cur = new Expression (Expression::OP_NUM, (uint64_t) 0);
	res = cur;
	int i = objs ? objs->size () - 1 : -1;
	for (; i >= 0; i--)
	  {
	    uint64_t jthr_id = 0;
	    JThread *jthread = (JThread *) (objs->fetch (i));
	    jthr_id = jthread->jthr_id;
	    uint64_t grid = grids->fetch (i);
	    uint64_t expid = expids->fetch (i);
	    cur->v.val = (grid << INDXOBJ_EXPGRID_SHIFT) |
		    (expid << INDXOBJ_EXPID_SHIFT) | jthr_id;
	    if (i == 0)
	      break;
	    cur->arg0 = new Expression (OP_NONE, (uint64_t) 0);
	    cur->v.next = &cur->arg0->v;
	    cur = cur->arg0;
	  }
	cur->v.next = NULL;
	delete objs;
	delete grids;
	delete expids;
	break;
      }
    case OP_FILE:
      {
	Vector<Histable*> *objs = NULL;
	Histable::NameFormat nfmt = ctx ? ctx->dbev->get_name_format () : Histable::NA;
	if (ctx)
	  objs = (Vector<Histable*>*)dbeSession->match_file_names ((char*) arg1->v.val, nfmt);
	Expression *cur = new Expression (Expression::OP_NUM, (uint64_t) 0);
	res = cur;
	int i = objs ? objs->size () - 1 : -1;
	for (; i >= 0; i--)
	  {
	    cur->v.val = objs->fetch (i)->id;
	    if (i == 0)
	      break;
	    cur->arg0 = new Expression (OP_NONE, (uint64_t) 0);
	    cur->v.next = &cur->arg0->v;
	    cur = cur->arg0;
	  }
	cur->v.next = NULL;
	if (objs)
	  delete objs;
	break;
      }
    case OP_NUM:
    case OP_COMMA:
      res = copy ();
      break;
    case OP_IN:
    case OP_SOMEIN:
    case OP_ORDRIN:
      {
	// LIBRARY_VISIBILITY:
	// Evaluate the arg0 of OP_IN, OP_SOMEIN, OP_ORDRIN to see if it has any library/loadobject
	// Change it to OP_LIBRARY_IN, OP_LIBRARY_SOMEIN or OP_LIBRARY_ORDRIN respectively
	if (dbeSession->is_lib_visibility_used () && (arg0->hasLoadObject ()
						     || arg1->hasLoadObject ()))
	  {
	    OpCode new_op;
	    switch (op)
	      {
	      case OP_IN:
		new_op = OP_LIBRARY_IN;
		break;
	      case OP_SOMEIN:
		new_op = OP_LIBRARY_SOMEIN;
		break;
	      case OP_ORDRIN:
		new_op = OP_LIBRARY_ORDRIN;
		break;
	      default:
		new_op = op; // Should never reach here
		break;
	      }
	    if (arg1->hasLoadObject ())
	      res = new Expression (new_op, arg1 ? arg1->pEval (ctx) : NULL,
				    arg0 ? arg0->pEval (ctx) : NULL);
	    else
	      res = new Expression (new_op, arg0 ? arg0->pEval (ctx) : NULL,
				    arg1 ? arg1->pEval (ctx) : NULL);
	    res->v = v;
	    ctx->dbev->setFilterHideMode ();
	    return res;
	  }
      }
      // no break; if no loadobjects found fall thru to the default case
    default:
      if (bEval (ctx))
	{
	  res = new Expression (OP_NUM, v.val);
	  break;
	}
      res = new Expression (op, arg0 ? arg0->pEval (ctx) : NULL,
			    arg1 ? arg1->pEval (ctx) : NULL);
      res->v = v;
      break;
    }
  return res;
}

bool
Expression::verifyObjectInExpr (Histable *obj)
{
  uint64_t id = ((uint64_t) obj->id);
  if (op == OP_NUM && v.val == id)
    return true;
  bool inArg0 = false;
  bool inArg1 = false;
  if (arg0 != NULL)
    inArg0 = arg0->verifyObjectInExpr (obj);
  if (inArg0)
    return true;
  if (arg1 != NULL)
    inArg1 = arg1->verifyObjectInExpr (obj);
  if (inArg1)
    return true;
  return false;
}

bool
Expression::hasLoadObject ()
{
  if (op == OP_NUM)
    {
      uint64_t id = v.val;
      Histable *obj = dbeSession->findObjectById (id);
      if (obj != NULL && obj->get_type () == Histable::FUNCTION)
	{
	  Function *func = (Function *) obj;
	  if (func->isHideFunc)
	    return true;
	}
    }
  if (arg0 && arg0->hasLoadObject ())
    return true;
  if (arg1 && arg1->hasLoadObject ())
    return true;
  return false;
}
