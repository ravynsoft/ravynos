// overflow_unittest.cc -- test functions that check for overflow.

// Copyright (C) 2016-2023 Free Software Foundation, Inc.
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

#include "gold.h"
#include "reloc.h"

#include <sys/types.h>

#include "test.h"

namespace gold_testsuite
{

using namespace gold;

bool
Overflow_test(Test_report*)
{
  CHECK(! Bits<16>::has_unsigned_overflow(0ULL));
  CHECK(! Bits<16>::has_unsigned_overflow(1ULL));
  CHECK(! Bits<16>::has_unsigned_overflow(0x7fffULL));
  CHECK(! Bits<16>::has_unsigned_overflow(0x8000ULL));
  CHECK(! Bits<16>::has_unsigned_overflow(0x8001ULL));
  CHECK(! Bits<16>::has_unsigned_overflow(0xffffULL));
  CHECK(Bits<16>::has_unsigned_overflow(0x10000ULL));
  CHECK(Bits<16>::has_unsigned_overflow(0x10001ULL));
  CHECK(Bits<16>::has_unsigned_overflow(~0ULL));
  CHECK(Bits<16>::has_unsigned_overflow(~0x7fffULL + 1));
  CHECK(Bits<16>::has_unsigned_overflow(~0x8000ULL + 1));
  CHECK(Bits<16>::has_unsigned_overflow(~0x8001ULL + 1));
  CHECK(Bits<16>::has_unsigned_overflow(~0xffffULL + 1));
  CHECK(Bits<16>::has_unsigned_overflow(~0x10000ULL + 1));
  CHECK(Bits<16>::has_unsigned_overflow(~0x10001ULL + 1));

  CHECK(! Bits<16>::has_overflow(0ULL));
  CHECK(! Bits<16>::has_overflow(1ULL));
  CHECK(! Bits<16>::has_overflow(0x7fffULL));
  CHECK(Bits<16>::has_overflow(0x8000ULL));
  CHECK(Bits<16>::has_overflow(0x8001ULL));
  CHECK(Bits<16>::has_overflow(0xffffULL));
  CHECK(Bits<16>::has_overflow(0x10000ULL));
  CHECK(Bits<16>::has_overflow(0x10001ULL));
  CHECK(! Bits<16>::has_overflow(~0ULL));
  CHECK(! Bits<16>::has_overflow(~0x7fffULL + 1));
  CHECK(! Bits<16>::has_overflow(~0x8000ULL + 1));
  CHECK(Bits<16>::has_overflow(~0x8001ULL + 1));
  CHECK(Bits<16>::has_overflow(~0xffffULL + 1));
  CHECK(Bits<16>::has_overflow(~0x10000ULL + 1));
  CHECK(Bits<16>::has_overflow(~0x10001ULL + 1));

  CHECK(! Bits<16>::has_signed_unsigned_overflow64(0ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(1ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(0x7fffULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(0x8000ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(0x8001ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(0xffffULL));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(0x10000ULL));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(0x10001ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(~0ULL));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(~0x7fffULL + 1));
  CHECK(! Bits<16>::has_signed_unsigned_overflow64(~0x8000ULL + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(~0x8001ULL + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(~0xffffULL + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(~0x10000ULL + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow64(~0x10001ULL + 1));

  CHECK(! Bits<16>::has_unsigned_overflow32(0U));
  CHECK(! Bits<16>::has_unsigned_overflow32(1U));
  CHECK(! Bits<16>::has_unsigned_overflow32(0x7fffU));
  CHECK(! Bits<16>::has_unsigned_overflow32(0x8000U));
  CHECK(! Bits<16>::has_unsigned_overflow32(0x8001U));
  CHECK(! Bits<16>::has_unsigned_overflow32(0xffffU));
  CHECK(Bits<16>::has_unsigned_overflow32(0x10000U));
  CHECK(Bits<16>::has_unsigned_overflow32(0x10001U));
  CHECK(Bits<16>::has_unsigned_overflow32(~0U));
  CHECK(Bits<16>::has_unsigned_overflow32(~0x7fffU + 1));
  CHECK(Bits<16>::has_unsigned_overflow32(~0x8000U + 1));
  CHECK(Bits<16>::has_unsigned_overflow32(~0x8001U + 1));
  CHECK(Bits<16>::has_unsigned_overflow32(~0xffffU + 1));
  CHECK(Bits<16>::has_unsigned_overflow32(~0x10000U + 1));
  CHECK(Bits<16>::has_unsigned_overflow32(~0x10001U + 1));

  CHECK(! Bits<16>::has_overflow32(0U));
  CHECK(! Bits<16>::has_overflow32(1U));
  CHECK(! Bits<16>::has_overflow32(0x7fffU));
  CHECK(Bits<16>::has_overflow32(0x8000U));
  CHECK(Bits<16>::has_overflow32(0x8001U));
  CHECK(Bits<16>::has_overflow32(0xffffU));
  CHECK(Bits<16>::has_overflow32(0x10000U));
  CHECK(Bits<16>::has_overflow32(0x10001U));
  CHECK(! Bits<16>::has_overflow32(~0U));
  CHECK(! Bits<16>::has_overflow32(~0x7fffU + 1));
  CHECK(! Bits<16>::has_overflow32(~0x8000U + 1));
  CHECK(Bits<16>::has_overflow32(~0x8001U + 1));
  CHECK(Bits<16>::has_overflow32(~0xffffU + 1));
  CHECK(Bits<16>::has_overflow32(~0x10000U + 1));
  CHECK(Bits<16>::has_overflow32(~0x10001U + 1));

  CHECK(! Bits<16>::has_signed_unsigned_overflow32(0U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(1U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(0x7fffU));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(0x8000U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(0x8001U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(0xffffU));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(0x10000U));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(0x10001U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(~0U));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(~0x7fffU + 1));
  CHECK(! Bits<16>::has_signed_unsigned_overflow32(~0x8000U + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(~0x8001U + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(~0xffffU + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(~0x10000U + 1));
  CHECK(Bits<16>::has_signed_unsigned_overflow32(~0x10001U + 1));

  return true;
}

Register_test overflow_register("Overflow", Overflow_test);

} // End namespace gold_testsuite.
