/* Copyright (C) 2021 Free Software Foundation, Inc.
   This file is part of the GNU LIBICONV Library.

   The GNU LIBICONV Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   The GNU LIBICONV Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU LIBICONV Library; see the file COPYING.LIB.
   If not, see <https://www.gnu.org/licenses/>.  */

#include "config.h"

#include "qemu.h"

/* Returns 0 (success) in a native environment.
   Returns 1 (failure) in a cross-executing environment, that is, in an
   environment where compiled programs use a different libc than the system's
   libc.  Currently, only QEMU user-mode environments are recognized.  */

int main ()
{
  return is_running_under_qemu_user () ? 1 : 0;
}
