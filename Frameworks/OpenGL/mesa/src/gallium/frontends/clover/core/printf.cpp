//
// Copyright 2020 Serge Martin
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include <cstring>
#include <cstdio>
#include <string>
#include <iostream>

#include "util/u_math.h"
#include "core/printf.hpp"

#include "util/u_printf.h"
using namespace clover;

namespace {

   const cl_uint hdr_dwords = 2;
   const cl_uint initial_buffer_offset = hdr_dwords * sizeof(cl_uint);

   /* all valid chars that can appear in CL C printf string. */
   const std::string clc_printf_whitelist = "%0123456789-+ #.AacdeEfFgGhilopsuvxX";

   void
   print_formatted(std::vector<binary::printf_info> &formatters,
                   bool _strings_in_buffer,
                   const std::vector<char> &buffer) {

      static std::atomic<unsigned> warn_count;

      if (buffer.empty() && !warn_count++)
         std::cerr << "Printf used but no printf occurred - may cause performance issue." << std::endl;

      std::vector<u_printf_info> infos;
      for (auto &f : formatters) {
         u_printf_info info;

         info.num_args = f.arg_sizes.size();
         info.arg_sizes = f.arg_sizes.data();
         info.string_size = f.strings.size();
         info.strings = f.strings.data();

         infos.push_back(info);
      }

      u_printf(stdout, buffer.data(), buffer.size(), infos.data(), infos.size());
   }
}

std::unique_ptr<printf_handler>
printf_handler::create(const intrusive_ptr<command_queue> &q,
                       const std::vector<binary::printf_info> &infos,
                       bool strings_in_buffer,
                       cl_uint size) {
   return std::unique_ptr<printf_handler>(
                                       new printf_handler(q, infos, strings_in_buffer, size));
}

printf_handler::printf_handler(const intrusive_ptr<command_queue> &q,
                               const std::vector<binary::printf_info> &infos,
                               bool strings_in_buffer,
                               cl_uint size) :
   _q(q), _formatters(infos), _strings_in_buffer(strings_in_buffer), _size(size), _buffer() {

   if (_size) {
      std::string data;
      data.reserve(_size);
      cl_uint header[2] = { 0 };

      header[0] = initial_buffer_offset;
      header[1] = _size;

      data.append((char *)header, (char *)(header+hdr_dwords));
      _buffer = std::unique_ptr<root_buffer>(new root_buffer(_q->context,
                                             std::vector<cl_mem_properties>(),
                                             CL_MEM_COPY_HOST_PTR,
                                             _size, (char*)data.data()));
   }
}

cl_mem
printf_handler::get_mem() {
   return (cl_mem)(_buffer.get());
}

void
printf_handler::print() {
   if (!_buffer)
      return;

   mapping src = { *_q, _buffer->resource_in(*_q), CL_MAP_READ, true,
                  {{ 0 }}, {{ _size, 1, 1 }} };

   cl_uint header[2] = { 0 };
   std::memcpy(header,
               static_cast<const char *>(src),
               initial_buffer_offset);

   cl_uint buffer_size = header[0];
   buffer_size -= initial_buffer_offset;
   std::vector<char> buf;
   buf.resize(buffer_size);

   std::memcpy(buf.data(),
               static_cast<const char *>(src) + initial_buffer_offset,
               buffer_size);

   // mixed endian isn't going to work, sort it out if anyone cares later.
   assert(_q->device().endianness() == PIPE_ENDIAN_NATIVE);
   print_formatted(_formatters, _strings_in_buffer, buf);
}
