// relro_test.cc -- test -z relro for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <stdint.h>
#include <unistd.h>

// This tests we were linked with a script.  If we were linked with a
// script, relro currently does not work.

extern char using_script[] __attribute__ ((weak));

// This code is put into a shared library linked with -z relro.

// i1 and i2 are not relro variables.
int i1 = 1;
static int i2 = 2;

// P1 is a global relro variable.
int* const p1 __attribute__ ((aligned(64))) = &i1;

// P2 is a local relro variable.
int* const p2 __attribute__ ((aligned(64))) = &i2;

// Add a TLS variable to make sure -z relro works correctly with TLS.
__thread int i3 = 1;

// Test symbol addresses.

bool
t1()
{
  if (using_script)
    return true;

  void* i1addr = static_cast<void*>(&i1);
  void* i2addr = static_cast<void*>(&i2);
  const void* p1addr = static_cast<const void*>(&p1);
  const void* p2addr = static_cast<const void*>(&p2);

  // The relro variables should precede the non-relro variables in the
  // memory image.
  assert(i1addr > p1addr);
  assert(i1addr > p2addr);
  assert(i2addr > p1addr);
  assert(i2addr > p2addr);

  // The relro variables should not be on the same page as the
  // non-relro variables.
  const size_t page_size = getpagesize();
  uintptr_t i1page = reinterpret_cast<uintptr_t>(i1addr) & ~ (page_size - 1);
  uintptr_t i2page = reinterpret_cast<uintptr_t>(i2addr) & ~ (page_size - 1);
  uintptr_t p1page = reinterpret_cast<uintptr_t>(p1addr) & ~ (page_size - 1);
  uintptr_t p2page = reinterpret_cast<uintptr_t>(p2addr) & ~ (page_size - 1);
  assert(i1page != p1page);
  assert(i1page != p2page);
  assert(i2page != p1page);
  assert(i2page != p2page);
  assert(i3 == 1);

  return true;
}

// Tell terminate handler that we are throwing from a signal handler.

static bool throwing;

// A signal handler for SIGSEGV.

extern "C"
void
sigsegv_handler(int)
{
  throwing = true;
  throw 0;
}

// The original terminate handler.

std::terminate_handler orig_terminate;

// Throwing an exception out of a signal handler doesn't always work
// reliably.  When that happens the program will call terminate.  We
// set a terminate handler to indicate that the test probably passed.

void
terminate_handler()
{
  if (!throwing)
    {
      orig_terminate();
      ::exit(EXIT_FAILURE);
    }
  fprintf(stderr,
	  "relro_test: terminate called due to failure to throw through signal handler\n");
  fprintf(stderr, "relro_test: assuming test succeeded\n");
  ::exit(EXIT_SUCCESS);
}

// Use a separate function to throw the exception, so that we don't
// need to use -fnon-call-exceptions.

void f2() __attribute__ ((noinline));
void
f2()
{
  int** pp1 = const_cast<int**>(&p1);
  *pp1 = &i2;

  // We shouldn't get here--the assignment to *pp1 should write to
  // memory which the dynamic linker marked as read-only, giving us a
  // SIGSEGV, causing sigsegv_handler to be invoked, to throw past us.
  assert(0);
}

// Changing a relro variable should give us a SIGSEGV.

bool
t2()
{
  if (using_script)
    return true;

  signal(SIGSEGV, sigsegv_handler);
  orig_terminate = std::set_terminate(terminate_handler);

  try
    {
      f2();
      return false;
    }
  catch (int i)
    {
      assert(i == 0);
      return true;
    }
}
