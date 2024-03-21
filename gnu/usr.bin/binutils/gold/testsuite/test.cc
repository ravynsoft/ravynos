// test.cc -- simplistic test framework for gold.

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

#include <cstdio>

#include "test.h"

namespace gold_testsuite
{

// Test_framework methods.

// The current test being run.

Test_report* Test_framework::current_report;

// Run a test.

void
Test_framework::run(const char *name, bool (*pfn)(Test_report*))
{
  this->testname_ = name;
  this->current_fail_ = false;

  Test_report tr(this);
  Test_framework::current_report = &tr;

  if ((*pfn)(&tr) && !this->current_fail_)
    {
      printf("PASS: %s\n", name);
      ++this->passes_;
    }
  else
    {
      printf("FAIL: %s\n", name);
      ++this->failures_;
    }

  Test_framework::current_report = NULL;
  this->testname_ = NULL;
}

// Report a failure.

void
Test_framework::fail(const char* filename, int lineno)
{
  printf("FAIL: %s: %s: %d\n", this->testname_, filename, lineno);
  this->current_fail_ = true;
}

// Let a test report an error.

void
Test_framework::error(const char* message)
{
  printf("ERROR: %s: %s\n", this->testname_, message);
  this->current_fail_ = true;
}

// Register_test methods.

// Linked list of all registered tests.

Register_test* Register_test::all_tests;

// Register a test.

Register_test::Register_test(const char* name, bool (*pfn)(Test_report*))
  : name_(name), pfn_(pfn), next_(Register_test::all_tests)
{
  Register_test::all_tests = this;
}

// Run all registered tests.

void
Register_test::run_tests(Test_framework* tf)
{
  for (Register_test* p = Register_test::all_tests;
       p != NULL;
       p = p->next_)
    tf->run(p->name_, p->pfn_);
}

} // End namespace gold_testsuite.
