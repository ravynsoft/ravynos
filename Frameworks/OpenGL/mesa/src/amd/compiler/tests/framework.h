/*
 * Copyright © 2020 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */
#ifndef ACO_TEST_COMMON_H
#define ACO_TEST_COMMON_H
#include "aco_builder.h"
#include "aco_ir.h"

#include "util/macros.h"

#include "ac_shader_util.h"
#include "amd_family.h"
#include <map>
#include <stdio.h>
#include <string>

struct TestDef {
   const char* name;
   const char* source_file;
   void (*func)();
};

extern std::map<std::string, TestDef> tests;
extern FILE* output;

bool set_variant(const char* name);

inline bool
set_variant(amd_gfx_level cls, const char* rest = "")
{
   char buf[8 + strlen(rest)];
   if (cls != GFX10_3) {
      snprintf(buf, sizeof(buf), "gfx%d%s", cls - GFX6 + 6 - (cls > GFX10_3), rest);
   } else {
      snprintf(buf, sizeof(buf), "gfx10_3%s", rest);
   }
   return set_variant(buf);
}

void fail_test(const char* fmt, ...);
void skip_test(const char* fmt, ...);

#define _BEGIN_TEST(name, struct_name)                                                             \
   static void struct_name();                                                                      \
   static __attribute__((constructor)) void CONCAT2(add_test_, __COUNTER__)()                      \
   {                                                                                               \
      tests[#name] = (TestDef){#name, ACO_TEST_BUILD_ROOT "/" __FILE__, &struct_name};             \
   }                                                                                               \
   static void struct_name()                                                                       \
   {

#define BEGIN_TEST(name)      _BEGIN_TEST(name, CONCAT2(Test_, __COUNTER__))
#define BEGIN_TEST_TODO(name) _BEGIN_TEST(name, CONCAT2(Test_, __COUNTER__))
#define BEGIN_TEST_FAIL(name) _BEGIN_TEST(name, CONCAT2(Test_, __COUNTER__))
#define END_TEST              }

#endif /* ACO_TEST_COMMON_H */
