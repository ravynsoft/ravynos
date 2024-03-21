// leb_unittest.cc -- test read_signed_LEB_128 and read_unsigned_LEB_128 

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

#include <sys/types.h>

#include "int_encoding.h"

#include "test.h"

namespace gold_testsuite
{

using namespace gold;

bool
Leb128_test(Test_report*)
{
  size_t len;

  // Unsigned tests.
  static unsigned char u1[] = { 0 };		// 0
  static unsigned char u2[] = { 1 };		// 1
  static unsigned char u3[] = { 126 };		// 126
  static unsigned char u4[] = { 127 };		// 127
  static unsigned char u5[] = { 0x80+0, 1 };	// 128
  static unsigned char u6[] = { 0x80+1, 1 };	// 129
  static unsigned char u7[] = { 0x80+57, 100 };	// 12857
  static unsigned char u8[] = { 0x80, 0x80, 0x80, 0x80,
				0x80, 0x80, 0x80, 0x80,
				0x80, 1};	// 1ULL << 63

  // Signed tests.
  static unsigned char s1[] = { 0 };		// 0
  static unsigned char s2[] = { 1 };		// 1
  static unsigned char s3[] = { 0x7e };		// -2
  static unsigned char s4[] = { 0x80+127, 0 };	// 127
  static unsigned char s5[] = { 0x80+1, 0x7f };	// -127
  static unsigned char s6[] = { 0x80+0, 1 };	// 128
  static unsigned char s7[] = { 0x80+0, 0x7f };	// -128
  static unsigned char s8[] = { 0x80+1, 1 };	// 129
  static unsigned char s9[] = { 0xff, 0x7e };	// -129

  CHECK(read_unsigned_LEB_128(u1, &len) == 0 && len == sizeof(u1));
  CHECK(read_unsigned_LEB_128(u2, &len) == 1 && len == sizeof(u2));
  CHECK(read_unsigned_LEB_128(u3, &len) == 126 && len == sizeof(u3));
  CHECK(read_unsigned_LEB_128(u4, &len) == 127 && len == sizeof(u4));
  CHECK(read_unsigned_LEB_128(u5, &len) == 128 && len == sizeof(u5));
  CHECK(read_unsigned_LEB_128(u6, &len) == 129 && len == sizeof(u6));
  CHECK(read_unsigned_LEB_128(u7, &len) == 12857 && len == sizeof(u7));
  CHECK(read_unsigned_LEB_128(u8, &len) == (1ULL << 63) && len == sizeof(u8));

  CHECK(read_signed_LEB_128(s1, &len) == 0 && len == sizeof(s1));
  CHECK(read_signed_LEB_128(s2, &len) == 1 && len == sizeof(s2));
  CHECK(read_signed_LEB_128(s3, &len) == -2 && len == sizeof(s3));
  CHECK(read_signed_LEB_128(s4, &len) == 127 && len == sizeof(s4));
  CHECK(read_signed_LEB_128(s5, &len) == -127 && len == sizeof(s5));
  CHECK(read_signed_LEB_128(s6, &len) == 128 && len == sizeof(s6));
  CHECK(read_signed_LEB_128(s7, &len) == -128 && len == sizeof(s7));
  CHECK(read_signed_LEB_128(s8, &len) == 129 && len == sizeof(s8));
  CHECK(read_signed_LEB_128(s9, &len) == -129 && len == sizeof(s9));

  return true;
}

Register_test leb128_register("LEB128", Leb128_test);

} // End namespace gold_testsuite.
