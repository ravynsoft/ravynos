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

#ifndef cg_dfn_h
#define cg_dfn_h

/*
 * Flags which mark a symbol as topologically ``busy'' or as
 * topologically ``not_numbered'':
 */
#define	DFN_BUSY	-1
#define	DFN_NAN		0

/*
 * Depth-first numbering of a call-graph.
 */

extern void cg_dfn (Sym * root);

#endif /* cg_dfn_h */
