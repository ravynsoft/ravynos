//
// Copyright 2016 Francisco Jerez
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

#ifndef CLOVER_LLVM_INVOCATION_HPP
#define CLOVER_LLVM_INVOCATION_HPP

#include "core/error.hpp"
#include "core/binary.hpp"
#include "core/program.hpp"
#include "pipe/p_defines.h"

namespace clover {
   namespace llvm {
      binary compile_program(const std::string &source,
                             const header_map &headers,
                             const device &device,
                             const std::string &opts,
                             std::string &r_log);

      binary link_program(const std::vector<binary> &binaries,
                          const device &device,
                          const std::string &opts,
                          std::string &r_log);

#ifdef HAVE_CLOVER_SPIRV
      binary compile_to_spirv(const std::string &source,
                              const header_map &headers,
                              const device &dev,
                              const std::string &opts,
                              std::string &r_log);
#endif
   }
}

#endif
