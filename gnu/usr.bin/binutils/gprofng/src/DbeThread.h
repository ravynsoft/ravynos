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

#ifndef _DBETHREAD_H
#define _DBETHREAD_H
#include <pthread.h>
#include "DbeLinkList.h"

template <class ITEM> class Vector;

class DbeQueue
{
public:
  DbeQueue (int (*_func)(void *arg), void *_arg);
  ~DbeQueue ();

  int (*func) (void *arg);
  void *arg;
  int id;
  DbeQueue *next;
};

class DbeThreadPool
{
public:
  DbeThreadPool (int _max_threads);
  ~DbeThreadPool ();
  DbeQueue *get_queue ();
  void put_queue (DbeQueue *q);
  void wait_queues ();

  pthread_mutex_t p_mutex;
  pthread_cond_t p_cond_var;
  volatile bool no_new_queues;
private:
  Vector<pthread_t> *threads;
  int max_threads;
  DbeQueue *volatile queue;
  DbeQueue *volatile last_queue;
  volatile int queues_cnt;
  volatile int total_queues;
};

#endif
