// weak_undef_test_2.cc -- test handling of weak undefined symbols for gold

// Copyright (C) 2014-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

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

// This file tests that we correctly deal with weak undefined symbols
// when searching archive libraries.  If we have a weak undefined symbol,
// it should not cause us to link an archive library member that defines
// that symbol.  However, if the symbol is also listed in a -u option on
// the command line, it should cause the archive member to be linked.


#include <cstdio>

// This symbol is defined in weak_undef_file3.cc, but we should
// not load it from the library.
extern int weak_undef_1 __attribute__ ((weak));

// This symbol is defined in weak_undef_file4.cc, but is also
// listed in a -u option on the link command, so we should
// load it from the library.
extern int weak_undef_2 __attribute__ ((weak));

int *p1 = &weak_undef_1;

int *p2 = &weak_undef_2;

int
main()
{
  int status = 0;

  if (&weak_undef_1 != NULL)
    {
      fprintf(stderr, "FAILED weak undef test 1: %s\n",
              "&weak_undef_1 is not NULL");
      status = 1;
    }

  if (&weak_undef_2 == NULL)
    {
      fprintf(stderr, "FAILED weak undef test 2: %s\n",
              "&weak_undef_2 is NULL");
      status = 1;
    }

  if (p1 != NULL)
    {
      fprintf(stderr, "FAILED weak undef test 3: %s\n",
              "p1 is not NULL");
      status = 1;
    }

  if (p2 == NULL)
    {
      fprintf(stderr, "FAILED weak undef test 4: %s\n",
              "p2 is NULL");
      status = 1;
    }

  return status;
}
