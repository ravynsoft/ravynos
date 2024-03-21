// copy_test_relro.cc -- test copy relocs against read-only and relro symbols.

// Copyright (C) 2016-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@gmail.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include <cassert>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>

extern int* const p;
extern const int b[];
extern const int* const q;
extern const int c;
int a = 123;

extern const int* const cp __attribute__ ((section (".rodata"))) = &c;
extern const int* const* const qp __attribute__ ((section (".rodata"))) = &q;

volatile int segfaults = 0;
sigjmp_buf jmp;

void segv(int)
{
  ++segfaults;
  siglongjmp(jmp, 1);
}

int main()
{
  assert(*p == 123);
  assert(b[0] == 100);
  assert(b[1] == 200);
  assert(b[2] == 300);
  assert(b[3] == 400);
  assert(c == 500);

  struct sigaction act;
  act.sa_handler = segv;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGSEGV, &act, 0);

  assert(segfaults == 0);
  if (sigsetjmp(jmp, 1) == 0)
    *const_cast<const int **>(&p) = &c;
  assert(segfaults == 1);
  if (sigsetjmp(jmp, 1) == 0)
    *const_cast<int *>(b) = 99;
  assert(segfaults == 2);
  if (sigsetjmp(jmp, 1) == 0)
    *const_cast<int *>(cp) = c - 1;
  assert(segfaults == 3);
  if (sigsetjmp(jmp, 1) == 0)
    *const_cast<int **>(qp) = &a;
  assert(segfaults == 4);

  return 0;
}
