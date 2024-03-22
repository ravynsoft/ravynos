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

#ifndef CLOVER_CORE_PRINTF_HANDLER_HPP
#define CLOVER_CORE_PRINTF_HANDLER_HPP

#include <memory>

#include "core/memory.hpp"

namespace clover {
   class printf_handler {
   public:
      static std::unique_ptr<printf_handler>
      create(const intrusive_ptr<command_queue> &q,
             const std::vector<binary::printf_info> &info,
             bool strings_in_buffer, cl_uint size);

      printf_handler(const printf_handler &arg) = delete;
      printf_handler &
      operator=(const printf_handler &arg) = delete;

      ~printf_handler() {};

      cl_mem get_mem();
      void print();

   private:
      printf_handler(const intrusive_ptr<command_queue> &q,
                     const std::vector<binary::printf_info> &infos,
                     bool strings_in_buffer, cl_uint size);

      intrusive_ptr<command_queue> _q;
      std::vector<binary::printf_info> _formatters;
      bool _strings_in_buffer;
      cl_uint _size;
      std::unique_ptr<root_buffer> _buffer;
   };
}

#endif
