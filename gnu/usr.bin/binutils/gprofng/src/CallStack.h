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

#ifndef _CALLSTACK_H
#define _CALLSTACK_H

#include <stdio.h>
#include "dbe_structs.h"
#include "Experiment.h"
#include "DbeLock.h"

class DataDescriptor;
class FramePacket;
class DbeInstr;
class Histable;
template <class ITEM> class Vector;
class CallStackNode;

class Descendants /* : public DbeLock */
{
public:
  Descendants ();
  ~Descendants ();
  CallStackNode *find (Histable *hi, int *index);
  void append (CallStackNode *item);
  void insert (int ind, CallStackNode *item);
  int volatile count;

private:

  enum
  {
    DELTA = 8
  };

  int limit;
  CallStackNode **data;
  CallStackNode *first_data[4];
};

class CallStackNode : public Descendants
{
public:
  CallStackNode (CallStackNode *_ancestor, Histable *_instr);
  ~CallStackNode ();
  bool compare (long start, long end, Vector<Histable*> *objs, CallStackNode *mRoot);
  void dump ();

  CallStackNode *
  get_ancestor ()
  {
    return ancestor;
  }

  Histable *
  get_instr ()
  {
    return instr;
  }

  CallStackNode *alt_node;
  Histable *instr;
  CallStackNode *ancestor;
};

class CallStack
{
public:
  static CallStack *getInstance (Experiment *exp);
  virtual ~CallStack () { };

  virtual void add_stack (DataDescriptor *dDscr, long idx, FramePacket *frp,
			  cstk_ctx_chunk* cstCtxChunk) = 0;

  // Creates a call stack representation for objs and
  // returns an opaque pointer to it
  virtual void *add_stack (Vector<Histable*> *objs) = 0;

  // Debugging methods
  virtual void print (FILE *) = 0;

  // Call stack inquiries
  static int stackSize (void *stack);
  static Histable *getStackPC (void *stack, int n);
  static Vector<Histable*> *getStackPCs (void *stack, bool get_hide_stack = false);
  static void setHideStack (void *stack, void *hideStack);
  static int compare (void *stack1, void *stack2);

  virtual CallStackNode *
  get_node (int)
  {
    return NULL;
  };

};

#endif /* _CALLSTACK_H */
