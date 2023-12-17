/* Spin locks (native Windows implementation).
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#ifndef _WINDOWS_SPIN_H
#define _WINDOWS_SPIN_H

#define WIN32_LEAN_AND_MEAN  /* avoid including junk */
#include <windows.h>

typedef struct
        {
          LONG volatile word;
        }
        glwthread_spinlock_t;

#define GLWTHREAD_SPIN_INIT { 0 }

#ifdef __cplusplus
extern "C" {
#endif

extern void glwthread_spin_init (glwthread_spinlock_t *lock);
extern int glwthread_spin_lock (glwthread_spinlock_t *lock);
extern int glwthread_spin_trylock (glwthread_spinlock_t *lock);
extern int glwthread_spin_unlock (glwthread_spinlock_t *lock);
extern int glwthread_spin_destroy (glwthread_spinlock_t *lock);

#ifdef __cplusplus
}
#endif

#endif /* _WINDOWS_SPIN_H */
