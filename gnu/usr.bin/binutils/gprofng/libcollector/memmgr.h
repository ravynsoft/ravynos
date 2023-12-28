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

#ifndef _MEMMGR_H
#define _MEMMGR_H

struct Heap;
typedef struct Heap Heap;

Heap *__collector_newHeap ();
void __collector_deleteHeap (Heap *heap);

/*
 * Initialize memmgr mutex locks.
 */
void __collector_mmgr_init_mutex_locks (Heap *heap);

/*
 * Allocate non-resizable memory.
 */
void *__collector_allocCSize (Heap *heap, unsigned sz, int log);

/*
 * Free non-resizable memory.
 */
void __collector_freeCSize (Heap *heap, void *ptr, unsigned sz);

/*
 * Allocate resizable memory
 */
void *__collector_allocVSize (Heap *heap, unsigned sz);

/*
 * Change size of resizable memory.
 * ptr - if not NULL, it must have been previously allocated from
 *       the same heap, otherwise returns allocVSize(heap, newsz);
 * newsz - new size; if 0, memory is freed and no new allocation
 *       occurs;
 */
void *__collector_reallocVSize (Heap *heap, void *ptr, unsigned newsz);

#endif
