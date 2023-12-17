/* Test of spin locks for communication between threads and signal handlers.
   Copyright (C) 2005, 2008-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2020.  */

#include <config.h>

/* Specification.  */
#include "asyncsafe-spin.h"

#include <signal.h>

asyncsafe_spinlock_t global_spin_lock = ASYNCSAFE_SPIN_INIT;

int
main (void)
{
  sigset_t set;

  sigemptyset (&set);
  sigaddset (&set, SIGINT);

  /* Check a spin-lock initialized through the constant initializer.  */
  {
    sigset_t saved_set;
    asyncsafe_spin_lock (&global_spin_lock, &set, &saved_set);
    asyncsafe_spin_unlock (&global_spin_lock, &saved_set);
  }

  /* Check a spin-lock initialized through asyncsafe_spin_init.  */
  {
    asyncsafe_spinlock_t local_spin_lock;
    int i;

    asyncsafe_spin_init (&local_spin_lock);

    for (i = 0; i < 10; i++)
      {
        sigset_t saved_set;
        asyncsafe_spin_lock (&local_spin_lock, &set, &saved_set);
        asyncsafe_spin_unlock (&local_spin_lock, &saved_set);
      }

    asyncsafe_spin_destroy (&local_spin_lock);
  }

  return 0;
}
