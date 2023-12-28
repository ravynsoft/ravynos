// ver_matching_def.cc - test matching rules in version_script.map

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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

extern "C" {
void foo() {} // V1
void foo1() {} // local
};

void bar() {} // V1
void bar1() {} // global

void baz(int*) {}          // V1
void baz(int*, char) {}    // global
void baz(char*, int) {}    // global

extern "C" {
void bar2() {}  // V1
};

namespace myns {
void blah() {} // V1
void bip() {} // V1

class Stuff {
 public:
  Stuff() {} // V1
};
}

class Biz {
 public:
  Biz() {} // global
};

namespace otherns {
Biz biz; // global
myns::Stuff stuff;  // V2
};

extern "C" {
void blaza() {}  // V1
void blaza1() {}  // V1

void original_blaza2() {} // V2
__asm__(".symver original_blaza2,blaza2@@V2");  // overrides script

void bla() {} // global
void blaz() {} // V2
void blazb() {} // V2

int globaoeufxstuff = 0;  // V1
int globaoeufostuff = 0; // global
float sizeof_headers = 50.0;  // V1
};
