/* Copyright (C) 2012-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef cg_arcs_h
#define cg_arcs_h

/*
 * Arc structure for call-graph.
 *
 * With pointers to the symbols of the parent and the child, a count
 * of how many times this arc was traversed, and pointers to the next
 * parent of this child and the next child of this parent.
 */
typedef struct arc
  {
    Sym *parent;		/* source vertice of arc */
    Sym *child;			/* dest vertice of arc */
    unsigned long count;	/* # of calls from parent to child */
    double time;		/* time inherited along arc */
    double child_time;		/* child-time inherited along arc */
    struct arc *next_parent;	/* next parent of CHILD */
    struct arc *next_child;	/* next child of PARENT */
    int has_been_placed;	/* have this arc's functions been placed? */
  }
Arc;

extern unsigned int num_cycles;	/* number of cycles discovered */
extern Sym *cycle_header;	/* cycle headers */

extern void arc_add (Sym * parent, Sym * child, unsigned long count);
extern Arc *arc_lookup (Sym * parent, Sym * child);
extern Sym **cg_assemble (void);
extern Arc **arcs;
extern unsigned int numarcs;

#endif /* cg_arcs_h */
