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

//
//    The Persistent Red-Black Tree
//

#ifndef _PRBTREE_H
#define _PRBTREE_H

#include "dbe_types.h"
template <class ITEM> class Vector;

// The number of pointers in a node must be set greater than 2.
// The higher the number the faster the search seems to be and
// the more memory the tree takes.
#define NPTRS   5

class PRBTree
{
public:

  typedef Vaddr Key_t;
  typedef hrtime_t Time_t;

  PRBTree ();
  ~PRBTree ();

  bool insert (Key_t key, Time_t ts, void *item);
  bool remove (Key_t key, Time_t ts);
  void *locate (Key_t key, Time_t ts);
  void *locate_exact_match (Key_t key, Time_t ts);
  void *locate_up (Key_t key, Time_t ts);
  Vector<void *> *values ();

private:

  enum Color
  {
    Red,
    Black
  };

  enum Direction
  {
    None,
    Left,
    Right
  };

  struct LMap
  {
    Key_t key;
    void *item;
    LMap *parent;
    LMap *chld[NPTRS];
    Time_t time[NPTRS];
    char dir[NPTRS];
    char color;
    LMap *next;

    LMap (Key_t _key, void *_item);
    LMap (const LMap& lm);
  };
  friend struct LMap;

  LMap *mlist; // The master list of all nodes
  Vector<LMap*> *roots;
  Vector<Time_t> *times;
  Vector<void *> *vals;
  LMap *root;
  Time_t rtts;  // root timestamp
  Time_t curts; // last update timestamp

  LMap *rb_locate (Key_t key, Time_t ts, bool low);
  LMap *rb_new_node (Key_t key, void *item);
  LMap *rb_new_node (LMap *lm);
  LMap *rb_copy_node (LMap *lm, Direction d);
  LMap *rb_fix_chld (LMap *prnt, LMap *lm, Direction d);
  LMap *rb_rotate (LMap *x, Direction d);
  void rb_remove_fixup (LMap *x, LMap *prnt, Direction d0);

  static LMap *rb_child (LMap *lm, Direction d, Time_t ts);
  static Direction rb_which_chld (LMap *lm);
  static LMap *rb_neighbor (LMap *lm, Time_t ts);

};

#endif /* _PRBTREE_H */
