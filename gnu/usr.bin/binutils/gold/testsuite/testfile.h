// testfile.h -- test input files   -*- C++ -*-

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

#ifndef GOLD_TESTSUITE_TESTFILE_H
#define GOLD_TESTSUITE_TESTFILE_H

namespace gold
{
class Target;
}

namespace gold_testsuite
{

extern gold::Target* target_test_pointer_32_little;
extern gold::Target* target_test_pointer_32_big;
extern gold::Target* target_test_pointer_64_little;
extern gold::Target* target_test_pointer_64_big;
extern const unsigned char test_file_1_32_little[];
extern const unsigned int test_file_1_size_32_little;
extern const unsigned char test_file_1_32_big[];
extern const unsigned int test_file_1_size_32_big;
extern const unsigned char test_file_1_64_little[];
extern const unsigned int test_file_1_size_64_little;
extern const unsigned char test_file_1_64_big[];
extern const unsigned int test_file_1_size_64_big;

}; // End namespace gold_testsuite.

#endif // !defined(GOLD_TESTSUITE_TESTFILE_H)
