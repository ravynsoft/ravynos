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

///
/// \file
/// Tools to generate various forms of binary code from existing LLVM IR in
/// the given llvm::Module object and output the result as a clover::binary.
///

#ifndef CLOVER_LLVM_CODEGEN_HPP
#define CLOVER_LLVM_CODEGEN_HPP

#include "llvm/util.hpp"
#include "core/binary.hpp"

#include <llvm/IR/Module.h>

#include <clang/Frontend/CompilerInstance.h>

namespace clover {
   namespace llvm {
      std::string
      print_module_bitcode(const ::llvm::Module &mod);

      binary
      build_module_library(const ::llvm::Module &mod,
                           enum binary::section::type section_type);

      std::unique_ptr< ::llvm::Module>
      parse_module_library(const binary &b, ::llvm::LLVMContext &ctx,
                           std::string &r_log);

      binary
      build_module_native(::llvm::Module &mod, const target &target,
                          const clang::CompilerInstance &c,
                          std::string &r_log);

      std::string
      print_module_native(const ::llvm::Module &mod, const target &target);

      binary
      build_module_common(const ::llvm::Module &mod,
                          const std::vector<char> &code,
                          const std::map<std::string, unsigned> &offsets,
                          const clang::CompilerInstance &c);
   }
}

#endif
