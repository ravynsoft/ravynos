// binary_unittest.cc -- test Binary_to_elf

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

#include "gold.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "elfcpp.h"
#include "parameters.h"
#include "errors.h"
#include "options.h"
#include "binary.h"
#include "object.h"
#include "descriptors.h"

#include "test.h"
#include "testfile.h"

namespace
{

ssize_t
read_all (int fd, unsigned char* buf, ssize_t size)
{
  ssize_t total_read = 0;
  while (size > 0)
    {
      ssize_t nread = ::read(fd, buf, size);
      if (nread < 0)
	return nread;
      if (nread == 0)
	break;
      buf += nread;
      size -= nread;
      total_read += nread;
    }
  return total_read;
}

} // End anonymous namespace.

namespace gold_testsuite
{

using namespace gold;

template<int size, bool big_endian>
bool
Sized_binary_test()
{
  parameters_clear_target();
  // We need a pretend Task.
  const Task* task = reinterpret_cast<const Task*>(-1);

  // Use the executable itself as the binary data.
  struct stat st;
  CHECK(::stat(gold::program_name, &st) == 0);
  int o = open_descriptor(-1, gold::program_name, O_RDONLY);
  CHECK(o >= 0);
  unsigned char* filedata = new unsigned char[st.st_size];
  CHECK(read_all(o, filedata, st.st_size) == static_cast<ssize_t>(st.st_size));
  CHECK(::close(o) == 0);

  Binary_to_elf binary(static_cast<elfcpp::EM>(0xffff), size, big_endian,
		       gold::program_name);

  CHECK(binary.convert(task));

  Input_file input_file(task, "test.o", binary.converted_data(),
			binary.converted_size());
  Object* object = make_elf_object("test.o", &input_file, 0,
				   binary.converted_data(),
				   binary.converted_size(), NULL);
  CHECK(object != NULL);
  if (object == NULL)
    return false;

  CHECK(!object->is_dynamic());
  CHECK(object->shnum() == 5);
  CHECK(object->section_name(1) == ".data");
  CHECK(object->section_flags(1) == (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE));
  section_size_type len;
  const unsigned char* contents = object->section_contents(1, &len, false);
  CHECK(len == convert_to_section_size_type(st.st_size));
  CHECK(memcmp(filedata, contents, len) == 0);

  // Force the symbols to be read internally, so that
  // symbol_section_and_value will work.
  Read_symbols_data sd;
  object->read_symbols(&sd);
  delete sd.section_headers;
  sd.section_headers = NULL;
  delete sd.section_names;
  sd.section_names = NULL;
  delete sd.symbols;
  sd.symbols = NULL;
  delete sd.symbol_names;
  sd.symbol_names = NULL;

  Sized_relobj_file<size, big_endian>* relobj =
    static_cast<Sized_relobj_file<size, big_endian>*>(object);
  typename Sized_relobj_file<size, big_endian>::Address value;
  bool is_ordinary;
  CHECK(relobj->symbol_section_and_value(0, &value, &is_ordinary) == 0);
  CHECK(is_ordinary);
  CHECK(value == 0);
  CHECK(relobj->symbol_section_and_value(1, &value, &is_ordinary) == 1);
  CHECK(is_ordinary);
  CHECK(value == 0);
  CHECK(relobj->symbol_section_and_value(2, &value, &is_ordinary) == 1);
  CHECK(is_ordinary);
  CHECK(static_cast<off_t>(value) == st.st_size);
  CHECK(relobj->symbol_section_and_value(3, &value, &is_ordinary)
	== elfcpp::SHN_ABS);
  CHECK(!is_ordinary);
  CHECK(static_cast<off_t>(value) == st.st_size);

  object->unlock(task);
  return true;
}

bool
Binary_test(Test_report*)
{
  Errors errors(gold::program_name);
  set_parameters_errors(&errors);

  General_options options;
  set_parameters_options(&options);

  int fail = 0;

#ifdef HAVE_TARGET_32_LITTLE
  if (!Sized_binary_test<32, false>())
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_32_little);
#endif

#ifdef HAVE_TARGET_32_BIG
  if (!Sized_binary_test<32, true>())
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_32_big);
#endif

#ifdef HAVE_TARGET_64_LITTLE
  if (!Sized_binary_test<64, false>())
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_64_little);
#endif

#ifdef HAVE_TARGET_64_BIG
  if (!Sized_binary_test<64, true>())
    ++fail;
  CHECK(&parameters->target() == target_test_pointer_64_big);
#endif

  return fail == 0;
}

Register_test binary_register("Binary", Binary_test);

} // End namespace gold_testsuite.
