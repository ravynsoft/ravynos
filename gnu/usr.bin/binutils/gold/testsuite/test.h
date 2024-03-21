// test.h -- simplistic test framework for gold unittests -*- C++ -*-

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

#ifndef GOLD_TESTSUITE_TEST_H
#define GOLD_TESTSUITE_TEST_H

namespace gold_testsuite
{

class Test_report;

// This class handles basic test framework functionality.

class Test_framework
{
 public:
  Test_framework()
    : testname_(NULL), current_fail_(0), passes_(0), failures_(0)
  { }

  // Return number of failures.
  unsigned int
  failures() const
  { return this->failures_; }

  // Run a test.
  void
  run(const char* name, bool (*pfn)(Test_report*));

  // Get the current Test_report.  This is used by the test support
  // macros.
  static Test_report*
  report()
  { return Test_framework::current_report; }

 private:
  friend class Test_report;

  // Cause the current test to fail.
  void
  fail(const char* filename, int lineno);

  // Report an error from the current test.
  void
  error(const char* message);

  // Current Test_report.  This is a static variable valid while a
  // test is being run.
  static Test_report* current_report;

  // Current test being run.
  const char* testname_;
  // Whether the current test is failing.
  bool current_fail_;
  // Total number of passeed tests.
  unsigned int passes_;
  // Total number of failed tests.
  unsigned int failures_;
};

// An instance of this class is passed to each test function.

class Test_report
{
public:
  Test_report(Test_framework* tf)
    : tf_(tf)
  { }

  // Mark the test as failing.
  void
  fail(const char* filename, int lineno)
  { this->tf_->fail(filename, lineno); }

  // Report an error.
  void
  error(const char* message)
  { this->tf_->error(message); }

private:
  Test_framework* tf_;
};

// This class registers a test function so that the testsuite runs it.

class Register_test
{
 public: 
  Register_test(const char* name, bool (*pfn)(Test_report*));

  // Run all registered tests.
  static void
  run_tests(Test_framework*);

 private:
  // Linked list of all tests.
  static Register_test* all_tests;

  // Test name.
  const char* name_;
  // Function to call.  It should return true if the test passes,
  // false if it fails.
  bool (*pfn_)(Test_report*);
  // Next test in linked list.
  Register_test* next_;
};

} // End namespace gold_testsuite.

// These macros are for convenient use in tests.

// Check that a condition is true.  If it is false, report a failure.

#define CHECK(cond)							\
  ((void)								\
   ((cond)								\
    ? 0									\
    : (::gold_testsuite::Test_framework::report()->fail(__FILE__,	\
							__LINE__),	\
       0)))

// Report an error during a test.

#define ERROR(msg) (::gold_testsuite::Test_framework::report()->error(msg))

#endif // !defined(GOLD_TESTSUITE_TEST_H)
