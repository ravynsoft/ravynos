// object_unittest.cc -- test Object, Relobj, etc.

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

#include "gold.h"

#include "object.h"
#include "options.h"
#include "parameters.h"

#include "test.h"
#include "testfile.h"

namespace gold_testsuite
{

using namespace gold;

// Test basic Object functionality.

template<int size, bool big_endian>
bool
Sized_object_test(const unsigned char* test_file, unsigned int test_file_size)
{
  parameters_clear_target();
  // We need a pretend Task.
  const Task* task = reinterpret_cast<const Task*>(-1);
  Input_file input_file(task, "test.o", test_file, test_file_size);
  Object* object = make_elf_object("test.o", &input_file, 0,
				   test_file, test_file_size, NULL);
  CHECK(object->name() == "test.o");
  CHECK(!object->is_dynamic());
  CHECK(object->is_locked());
  object->unlock(task);
  CHECK(!object->is_locked());
  object->lock(task);
  CHECK(object->shnum() == 5);
  CHECK(object->section_name(0).empty());
  CHECK(object->section_name(1) == ".test");
  CHECK(object->section_flags(0) == 0);
  CHECK(object->section_flags(1) == elfcpp::SHF_ALLOC);
  object->unlock(task);
  return true;
}

bool
Object_test(Test_report*)
{
  General_options options;
  int fail = 0;

  set_parameters_options(&options);

#ifdef HAVE_TARGET_32_LITTLE
  if (!Sized_object_test<32, false>(test_file_1_32_little,
				    test_file_1_size_32_little))
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_32_little);
#endif

#ifdef HAVE_TARGET_32_BIG
  if (!Sized_object_test<32, true>(test_file_1_32_big,
				   test_file_1_size_32_big))
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_32_big);
#endif

#ifdef HAVE_TARGET_64_LITTLE
  if (!Sized_object_test<64, false>(test_file_1_64_little,
				    test_file_1_size_64_little))
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_64_little);
#endif

#ifdef HAVE_TARGET_64_BIG
  if (!Sized_object_test<64, true>(test_file_1_64_big,
				   test_file_1_size_64_big))
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_64_big);
#endif

  return fail == 0;
}

Register_test object_register("Object", Object_test);

} // End namespace gold_testsuite.
