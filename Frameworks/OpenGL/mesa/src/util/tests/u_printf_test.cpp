/*
 * Copyright © 2022 Yonggang Luo
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
 */

#include <gtest/gtest.h>

#include "util/memstream.h"
#include "util/u_memory.h"
#include "util/u_printf.h"

class u_printf_test : public ::testing::Test
{
private:
   template<typename T>
   void add_to_buffer(const T& t)
   {
      const uint8_t *data = reinterpret_cast<const uint8_t*>(&t);
      buffer.insert(buffer.end(), data, data + sizeof(t));
   }

protected:
   char *out_buffer = nullptr;
   size_t buffer_size = 0;

   struct format {
      std::vector<char> fmt;
      std::vector<unsigned> args;
   };

   std::vector<format> formats;
   std::vector<char> buffer;
   unsigned last_format;

   virtual void SetUp()
   {
   }

   virtual void TearDown()
   {
      if (out_buffer != NULL) {
         free(out_buffer);
      }
   }

   void add_format(const char *string, std::vector<unsigned> arg_sizes)
   {
      format fmt;
      fmt.fmt = std::vector<char>(string, string + strlen(string) + 1);
      fmt.args = arg_sizes;
      formats.push_back(fmt);
   }

   void use_format(uint32_t idx)
   {
      last_format = idx;
      idx += 1;
      add_to_buffer(idx);
   }

   void add_arg(const char *str)
   {
      format &fmt = formats[last_format];
      uint64_t pos = fmt.fmt.size();

      fmt.fmt.insert(fmt.fmt.end(), str, str + strlen(str) + 1);

      add_to_buffer(pos);
   }

   template<typename T>
   void add_arg(typename std::enable_if<std::is_scalar<T>::value, T>::type t)
   {
      add_to_buffer(t);
   }

   template<typename T, size_t N>
   void add_arg(const T (&t)[N])
   {
      for (unsigned i = 0; i < N; i++)
         add_to_buffer(t[i]);
   }

   std::string parse()
   {
      u_memstream stream;
      FILE* out;
      u_memstream_open(&stream, &out_buffer, &buffer_size);
      out = u_memstream_get(&stream);
      std::vector<u_printf_info> infos;

      for (auto& format : formats) {
         u_printf_info info;
         info.num_args = static_cast<unsigned>(format.args.size());
         info.arg_sizes = format.args.data();
         info.string_size = format.fmt.size();
         info.strings = const_cast<char*>(format.fmt.data());
         infos.push_back(info);
      }

      u_printf(out, buffer.data(), buffer.size(), infos.data(), infos.size());
      u_memstream_close(&stream);

      return out_buffer;
   }
};

TEST_F(u_printf_test, util_printf_next_spec_pos)
{
   EXPECT_EQ(util_printf_next_spec_pos("%d%d", 0), 1);
   EXPECT_EQ(util_printf_next_spec_pos("%%d", 0), -1);
   EXPECT_EQ(util_printf_next_spec_pos("%dd", 0), 1);
   EXPECT_EQ(util_printf_next_spec_pos("%%%%%%", 0), -1);
   EXPECT_EQ(util_printf_next_spec_pos("%%%%%%%d", 0), 7);
   EXPECT_EQ(util_printf_next_spec_pos("abcdef%d", 0), 7);
   EXPECT_EQ(util_printf_next_spec_pos("abcdef%d", 6), 7);
   EXPECT_EQ(util_printf_next_spec_pos("abcdef%d", 7), -1);
   EXPECT_EQ(util_printf_next_spec_pos("abcdef%%", 7), -1);
   EXPECT_EQ(util_printf_next_spec_pos("abcdef%d%d", 0), 7);
   EXPECT_EQ(util_printf_next_spec_pos("%rrrrr%d%d", 0), 7);
   EXPECT_EQ(util_printf_next_spec_pos("%%rrrr%d%d", 0), 7);
}

TEST_F(u_printf_test, util_printf_simple)
{
   add_format("Hello", { });
   use_format(0);
   EXPECT_EQ(parse(), "Hello");
}

TEST_F(u_printf_test, util_printf_single_string)
{
   add_format("%s\n", { 8 });

   use_format(0);
   add_arg("Hello");

   EXPECT_EQ(parse(), "Hello\n");
}

TEST_F(u_printf_test, util_printf_multiple_string)
{
   add_format("foo %s bar\n", { 8 });
   add_format("%s[%s]: %s\n", { 8, 8, 8 });
   add_format("!!! %s !!!", { 8 });

   use_format(2);
   add_arg("ERROR");

   use_format(0);
   add_arg("OpenCL\n");

   use_format(1);
   add_arg("mesa");
   add_arg("12345");
   add_arg("error");

   use_format(0);
   add_arg("ABCDE\n");

   EXPECT_EQ(parse(), "!!! ERROR !!!foo OpenCL\n bar\nmesa[12345]: error\nfoo ABCDE\n bar\n");
}

TEST_F(u_printf_test, util_printf_mixed)
{
   add_format("LOG[%s] %u + %u = %u", { 8, 4, 4, 4 });

   use_format(0);
   add_arg("DEBUG");
   add_arg<uint32_t>(5);
   add_arg<uint32_t>(8);
   add_arg<uint32_t>(13);

   EXPECT_EQ(parse(), "LOG[DEBUG] 5 + 8 = 13");
}

TEST_F(u_printf_test, util_printf_mixed_multiple)
{
   add_format("%%%%%s %u %f %.1v4hlf ", { 8, 4, 4, 16 });
   add_format("%i %u\n", { 4, 4 });
   add_format("ABCED\n", {});
   add_format("foo %hx bar\n", { 2 });

   use_format(2);

   use_format(1);
   add_arg<int32_t>(-5);
   add_arg<uint32_t>(6);

   float floats[4] = {
      1.0,
      2.0,
      3.0,
      4.0
   };
   use_format(0);
   add_arg("mesa");
   add_arg<uint32_t>(10);
   add_arg<float>(3.0);
   add_arg<float, 4>(floats);

   EXPECT_EQ(parse(), "ABCED\n-5 6\n%%%%mesa 10 3.000000 1.0,2.0,3.0,4.0 ");
}
