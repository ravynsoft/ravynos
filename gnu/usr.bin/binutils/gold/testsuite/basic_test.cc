// basic_test.cc -- a test case for gold

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

// The goal of this program is to produce as many different types of
// relocations as we can in a stand-alone program that does not use
// TLS.  This program is compiled without optimization.

// 1  Code reference to global data.
// 2  Code reference to static data.
// 3  Code reference to BSS data.
// 4  Code reference to offset within global data.
// 5  Code reference to offset within static data.
// 6  Code reference to offset within BSS data.
// 7  Switch statement with a table of destinations.
// 8  Taking the address of a label (a gcc extension).
// 9  Taking the address of a nested function (a gcc extension).
// 10 Data reference to global data.
// 11 Data reference to static data.
// 12 Data reference to BSS data.
// 13 Data reference to offset within global data.
// 14 Data reference to offset within static data.
// 15 Data reference to offset within BSS data.
// 16 Virtual table.
// 17 Inline function.
// 18 Call through pointer to method.
// 19 Initialize variable to pointer to method.
// 20 Global constructor and destructor.

// 1 Code reference to global data.
int t1 = 11;

// 2 Code reference to static data.
static int t2 = 22;

// 3 Code reference to BSS data (initialized after program starts, to
// 33).
int t3;

// 4 Code reference to offset within global data.
char t4[] = "Hello, world";

// 5 Code reference to offset within static data.
static char t5[] = "Hello, world";

// 6 Code reference to offset within BSS data (initialized after
// program starts, to contents of t4).
char t6[13];

// Test cases 1 through 6.

bool
t1_6()
{
  return (t1 == 11
	  && t2 == 22
	  && t3 == 33
	  && t4[5] == ','
	  && t5[7] == 'w'
	  && t6[9] == 'r');
}

// 7  Switch statement with a table of destinations.

int
t7(int i)
{
  switch (i)
    {
    case 0:
      return 12;
    case 1:
      return 34;
    case 2:
      return 56;
    case 3:
      return 78;
    case 4:
      return 90;
    case 5:
      return 13;
    case 6:
      return 0;
    case 7:
      return 57;
    case 8:
      return 79;
    case 9:
      return 81;
    default:
      return 144;
    }
}

// 8  Taking the address of a label (a gcc extension).

int
t8(int i)
{
  for (int j = 0; j < 10; ++j)
    {
      void* p;
      if (i + j > 6)
	p = &&lab1;
      else
	p = &&lab2;
      if (j == 7)
	goto *p;
    }
  return 15;
 lab1:
  return 0;
 lab2:
  return 12;
}

// 9  Taking the address of a nested function (a gcc extension).
// Disabled because this is only supported in C, not C++.

int
t9a(int (*pfn)(int))
{
  return (*pfn)(10) - 10;
}

int
t9(int i)
{
#if 0
  int
  t9c(int j)
  {
    return i + j;
  }
  return t9a(&t9c);
#else
  return i;
#endif
}

// 10 Data reference to global data.
int* t10 = &t1;

// 11 Data reference to static data.
int* t11 = &t2;

// 12 Data reference to BSS data.
int* t12 = &t3;

// 13 Data reference to offset within global data.
char* t13 = &t4[6];

// 14 Data reference to offset within static data.
char* t14 = &t5[8];

// 15 Data reference to offset within BSS data.
char* t15 = &t6[10];

// Test cases 10 through 15.

bool
t10_15()
{
  return (*t10 == 11
	  && *t11 == 22
	  && *t12 == 33
	  && *t13 == ' '
	  && *t14 == 'o'
	  && *t15 == 'l');
}

// 16 Virtual table.

class t16a
{
 public:
  virtual
  ~t16a()
  { }
  virtual int
  t()
  { return 83; }
};

class t16b : public t16a
{
 public:
  virtual int
  t()
  { return 92; }
};

t16b t16v;

bool
t16()
{
  return t16v.t() == 92;
}

// 17 Inline function.

inline int
t17a()
{
  return 74;
}

bool
t17()
{
  return t17a() == 74;
}

// 18 Call through pointer to method.

class t18a
{
 public:
  int
  ta()
  { return 65; }

  int
  tb()
  { return 90; }
};

t18a t18v;

int
t18f(int (t18a::* p)())
{
  return (t18v.*p)();
}

bool
t18()
{
  return t18f(&t18a::ta) == 65;
}

// 19 Initialize variable to pointer to method.

int (t18a::* t19v)() = &t18a::tb;

bool
t19()
{
  return (t18v.*t19v)() == 90;
}

// 20 Global constructor and destructor.

class t20a
{
 public:
  t20a()
    : i(96)
  { }
  ~t20a()
  { }
  int
  get() const
  { return this->i; }
 private:
  int i;
};

t20a t20v;

bool
t20()
{
  return t20v.get() == 96;
}

// Main function.  Initialize variables and call test functions.

int
main()
{
  t3 = 33;
  for (int i = 0; i < 13; ++i)
    t6[i] = t4[i];

  if (t1_6()
      && t7(6) == 0
      && t8(0) == 0
      && t9(5) == 5
      && t10_15()
      && t16()
      && t17()
      && t18()
      && t19()
      && t20())
    return 0;
  else
    return 1;
}
