// weak_test.cc -- test handling of weak symbols for gold

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

// This provides a set of test functions for TLS variables.  The
// functions are called by a main function in tls_test_main.cc.  This
// lets us test TLS access from a shared library.  We currently don't
// bother to test TLS access between two different files, on the
// theory that that is no more complicated than ordinary variable
// access between files.

// We test that we correctly deal with weak symbols defined in
// other libraries (in this case, libc).  We need to make sure we
// copy the associated GLOBAL reloc when we copy a WEAK reloc.


#include <cstdio>

int
main()
{
  extern char** environ;   // defined in libc
  if (environ == NULL)
    {
      fprintf(stderr, "FAILED the environ test: environ is NULL\n");
      return 1;
    }
  return 0;
}
