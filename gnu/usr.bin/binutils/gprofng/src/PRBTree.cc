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

//    The Persistent Red-Black Tree
//
// Implementation is based on an algorithm described in
// Sarnak, N., Tarjan, R., "Planar point location using
// persistent search trees", in Communications of the ACM,
// 1986, Vol.29, Number 7.
//

#include "config.h"
#include <memory.h>
#include <string.h>

#include "vec.h"
#include "PRBTree.h"

#define ASSERT(x)

#define IS_BLACK(x)     ((x)==NULL || (x)->color == Black)
#define IS_RED(x)       ((x)!=NULL && (x)->color == Red)
#define SET_BLACK(x)    if(x) x->color = Black
#define SET_RED(x)      (x)->color = Red

#define D_OPPOSITE(x) (((x)==Left) ? Right : Left )

PRBTree::LMap::LMap (Key_t _key, void *_item)
{
  key = _key;
  item = _item;
  color = Red;
  parent = NULL;
  for (int i = 0; i < NPTRS; i++)
    {
      dir[i] = None;
      chld[i] = NULL;
      time[i] = 0;
    }
};

PRBTree::LMap::LMap (const LMap& lm)
{
  key = lm.key;
  item = lm.item;
  color = lm.color;
  parent = lm.parent;
  for (int i = 0; i < NPTRS; i++)
    {
      dir[i] = None;
      chld[i] = NULL;
      time[i] = 0;
    }
};

PRBTree::PRBTree ()
{
  roots = new Vector<LMap*>;
  root = NULL;
  times = new Vector<Time_t>;
  rtts = (Time_t) 0;
  curts = (Time_t) 0;
  mlist = NULL;
  vals = NULL;
}

PRBTree::~PRBTree ()
{
  while (mlist)
    {
      LMap *lm = mlist;
      mlist = mlist->next;
      delete lm;
    }
  delete times;
  delete roots;
  delete vals;
}

Vector<void *> *
PRBTree::values ()
{
  if (vals == NULL)
    {
      vals = new Vector<void*>;
      for (LMap *lm = mlist; lm; lm = lm->next)
	vals->append (lm->item);
    }
  return vals;
}

PRBTree::LMap *
PRBTree::rb_new_node (Key_t key, void *item)
{
  LMap *lm = new LMap (key, item);
  lm->next = mlist;
  mlist = lm;
  return lm;
}

PRBTree::LMap *
PRBTree::rb_new_node (LMap *lm)
{
  LMap *lmnew = new LMap (*lm);
  lmnew->next = mlist;
  mlist = lmnew;
  return lmnew;
}

PRBTree::LMap *
PRBTree::rb_child (LMap *lm, Direction d, Time_t ts)
{
  if (lm == NULL)
    return NULL;
  for (int i = 0; i < NPTRS; i++)
    {
      if (lm->time[i] > ts)
	continue;
      if (lm->dir[i] == d)
	return lm->chld[i];
      else if (lm->dir[i] == None)
	break;
    }
  return NULL;
}

PRBTree::Direction
PRBTree::rb_which_chld (LMap *lm)
{
  LMap *prnt = lm->parent;
  if (prnt == NULL)
    return None;
  for (int i = 0; i < NPTRS; i++)
    {
      if (prnt->dir[i] == None)
	break;
      if (prnt->chld[i] == lm)
	return (Direction) prnt->dir[i];
    }
  return None;
}

PRBTree::LMap *
PRBTree::rb_neighbor (LMap *lm, Time_t ts)
{
  ASSERT (lm->dir[0] != None);
  Direction d = D_OPPOSITE (lm->dir[0]);
  LMap *y = NULL;
  LMap *next = lm->chld[0];
  while (next)
    {
      y = next;
      next = rb_child (y, d, ts);
    }
  return y;
}

PRBTree::LMap *
PRBTree::rb_copy_node (LMap *lm, Direction d)
{
  LMap *nlm = rb_new_node (lm);
  rb_fix_chld (lm->parent, nlm, rb_which_chld (lm));
  if (d == None)
    {
      d = Left;
      rb_fix_chld (nlm, rb_child (lm, d, curts), d);
    }

  // copy the other child
  Direction dd = D_OPPOSITE (d);
  rb_fix_chld (nlm, rb_child (lm, dd, curts), dd);
  return nlm;
}

PRBTree::LMap *
PRBTree::rb_fix_chld (LMap *prnt, LMap *lm, Direction d)
{

  if (prnt == NULL)
    {
      // fixing root
      ASSERT (d == None);
      if (rtts == curts)
	root = lm;
      else
	{
	  roots->append (root);
	  times->append (rtts);
	  root = lm;
	  rtts = curts;
	}
      if (lm != NULL)
	lm->parent = prnt;
      return prnt;
    }

  // If we already have a d-pointer at time curts, reuse it
  for (int i = 0; prnt->time[i] == curts; i++)
    {
      if (prnt->dir[i] == d)
	{
	  prnt->chld[i] = lm;
	  if (lm != NULL)
	    lm->parent = prnt;
	  return prnt;
	}
    }

  if (prnt->dir[NPTRS - 1] != None)
    prnt = rb_copy_node (prnt, d);
  ASSERT (prnt->dir[NPTRS - 1] == None);

  for (int i = NPTRS - 1; i > 0; i--)
    {
      prnt->dir[i] = prnt->dir[i - 1];
      prnt->chld[i] = prnt->chld[i - 1];
      prnt->time[i] = prnt->time[i - 1];
    }
  prnt->dir[0] = d;
  prnt->chld[0] = lm;
  prnt->time[0] = curts;
  if (lm != NULL)
    lm->parent = prnt;
  return prnt;
}

PRBTree::LMap *
PRBTree::rb_rotate (LMap *x, Direction d)
{
  Direction dd = D_OPPOSITE (d);
  LMap *y = rb_child (x, dd, curts);
  x = rb_fix_chld (x, rb_child (y, d, curts), dd);
  rb_fix_chld (x->parent, y, rb_which_chld (x));
  rb_fix_chld (y, x, d);
  return x;
}

void
PRBTree::rb_remove_fixup (LMap *x, LMap *prnt, Direction d0)
{

  while (IS_BLACK (x) && (x != root))
    {
      Direction d = (x == NULL) ? d0 : rb_which_chld (x);
      Direction dd = D_OPPOSITE (d);
      LMap *y = rb_child (prnt, dd, curts);
      if (IS_RED (y))
	{
	  SET_BLACK (y);
	  SET_RED (prnt);
	  prnt = rb_rotate (prnt, d);
	  y = rb_child (prnt, dd, curts);
	}
      LMap *y_d = rb_child (y, d, curts);
      LMap *y_dd = rb_child (y, dd, curts);
      if (IS_BLACK (y_d) && IS_BLACK (y_dd))
	{
	  SET_RED (y);
	  x = prnt;
	  prnt = x->parent;
	}
      else
	{
	  if (IS_BLACK (y_dd))
	    {
	      SET_BLACK (y_d);
	      SET_RED (y);
	      y = rb_rotate (y, dd);
	      prnt = y->parent->parent;
	      y = rb_child (prnt, dd, curts);
	      y_dd = rb_child (y, dd, curts);
	    }
	  y->color = prnt->color;
	  SET_BLACK (prnt);
	  SET_BLACK (y_dd);
	  prnt = rb_rotate (prnt, d);
	  break;
	}
    }
  SET_BLACK (x);
}

PRBTree::LMap *
PRBTree::rb_locate (Key_t key, Time_t ts, bool low)
{
  LMap *lm;
  Direction d;
  int i, lt, rt;
  int tsz = times->size ();

  if (ts >= rtts)
    lm = root;
  else
    {
      // exponential search
      for (i = 1; i <= tsz; i = i * 2)
	if (times->fetch (tsz - i) <= ts)
	  break;

      if (i <= tsz)
	{
	  lt = tsz - i;
	  rt = tsz - i / 2 - 1;
	}
      else
	{
	  lt = 0;
	  rt = tsz - 1;
	}
      while (lt <= rt)
	{
	  int md = (lt + rt) / 2;
	  if (times->fetch (md) <= ts)
	    lt = md + 1;
	  else
	    rt = md - 1;
	}
      if (rt < 0)
	return NULL;
      lm = roots->fetch (rt);
    }

  LMap *last_lo = NULL;
  LMap *last_hi = NULL;
  while (lm != NULL)
    {
      if (key >= lm->key)
	{
	  last_lo = lm;
	  d = Right;
	}
      else
	{
	  last_hi = lm;
	  d = Left;
	}
      lm = rb_child (lm, d, ts);
    }
  return low ? last_lo : last_hi;
}

//==================================================== Public interface

bool
PRBTree::insert (Key_t key, Time_t ts, void *item)
{
  LMap *lm, *y;
  Direction d, dd;
  if (ts > curts)
    curts = ts;
  else if (ts < curts)
    return false; // can only update the current tree

  // Insert in the tree in the usual way
  y = NULL;
  d = None;
  for (LMap *next = root; next;)
    {
      y = next;
      if (key == y->key)
	{
	  // copy the node with both children
	  lm = rb_copy_node (y, None);
	  // but use the new item
	  lm->item = item;
	  return true;
	}
      d = (key < y->key) ? Left : Right;
      next = rb_child (y, d, curts);
    }
  lm = rb_new_node (key, item);
  rb_fix_chld (y, lm, d);

  // Rebalance the tree
  while (IS_RED (lm->parent))
    {
      d = rb_which_chld (lm->parent);
      dd = D_OPPOSITE (d);

      y = rb_child (lm->parent->parent, dd, curts);
      if (IS_RED (y))
	{
	  SET_BLACK (lm->parent);
	  SET_BLACK (y);
	  SET_RED (lm->parent->parent);
	  lm = lm->parent->parent;
	}
      else
	{
	  if (rb_which_chld (lm) == dd)
	    {
	      lm = lm->parent;
	      lm = rb_rotate (lm, d);
	    }
	  SET_BLACK (lm->parent);
	  SET_RED (lm->parent->parent);
	  rb_rotate (lm->parent->parent, dd);
	}
    }

  // Color the root Black
  SET_BLACK (root);
  return true;
}

bool
PRBTree::remove (Key_t key, Time_t ts)
{
  LMap *lm, *x, *y, *prnt;
  if (ts > curts)
    curts = ts;
  else if (ts < curts)
    return false; // can only update the current tree

  lm = rb_locate (key, curts, true);
  if (lm == NULL || lm->key != key)
    return false;

  if (rb_child (lm, Left, curts) && rb_child (lm, Right, curts))
    y = rb_neighbor (lm, curts);
  else
    y = lm;

  x = rb_child (y, Left, curts);
  if (x == NULL)
    x = rb_child (y, Right, curts);

  if (y != lm)
    {
      lm = rb_copy_node (lm, None); // copied with children
      lm->key = y->key;
      lm->item = y->item;
    }

  Direction d = rb_which_chld (y);
  prnt = rb_fix_chld (y->parent, x, d);
  if (IS_BLACK (y))
    rb_remove_fixup (x, prnt, d);
  return true;
}

void *
PRBTree::locate (Key_t key, Time_t ts)
{
  LMap *lm = rb_locate (key, ts, true);
  return lm ? lm->item : NULL;
}

void *
PRBTree::locate_up (Key_t key, Time_t ts)
{
  LMap *lm = rb_locate (key, ts, false);
  return lm ? lm->item : NULL;
}

void *
PRBTree::locate_exact_match (Key_t key, Time_t ts)
{
  LMap *lm = rb_locate (key, ts, true);
  if (lm && key == lm->key)
    return lm->item;
  return NULL;
}
