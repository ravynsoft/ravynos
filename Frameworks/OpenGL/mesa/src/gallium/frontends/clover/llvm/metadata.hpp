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
/// Utility functions for LLVM IR metadata introspection.
///

#ifndef CLOVER_LLVM_METADATA_HPP
#define CLOVER_LLVM_METADATA_HPP

#include "llvm/compat.hpp"
#include "util/algorithm.hpp"

#include <vector>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Metadata.h>

namespace clover {
   namespace llvm {
      namespace detail {
         inline bool
         is_kernel(const ::llvm::Function &f) {
            return f.getMetadata("kernel_arg_type");
         }

         inline iterator_range< ::llvm::MDNode::op_iterator>
         get_kernel_metadata_operands(const ::llvm::Function &f,
                                      const std::string &name) {
            const auto data_node = f.getMetadata(name);
            if (data_node)
               return range(data_node->op_begin(), data_node->op_end());
            else
               return iterator_range< ::llvm::MDNode::op_iterator>();
         }
      }

      ///
      /// Extract the string metadata node \p name.
      ///
      inline std::string
      get_str_kernel_metadata(const ::llvm::Function &f,
                          const std::string &name) {
         auto operands = detail::get_kernel_metadata_operands(f, name);
         if (operands.size()) {
            return ::llvm::cast< ::llvm::MDString>(
               detail::get_kernel_metadata_operands(f, name)[0])
               ->getString().str();
         } else {
            return "";
         }
      }

      ///
      /// Extract the string metadata node \p name.
      ///
      inline std::vector<size_t>
      get_uint_vector_kernel_metadata(const ::llvm::Function &f,
                          const std::string &name) {
         auto operands = detail::get_kernel_metadata_operands(f, name);
         if (operands.size()) {
            return map([=](const ::llvm::MDOperand& o) {
               auto value = ::llvm::cast< ::llvm::ConstantAsMetadata>(o)
                                                               ->getValue();
               return ::llvm::cast< ::llvm::ConstantInt>(value)
                                                ->getLimitedValue(UINT_MAX);
            }, operands);
         } else {
            return {};
         }
      }

      ///
      /// Extract the string metadata node \p name.
      ///
      inline std::string
      get_type_kernel_metadata(const ::llvm::Function &f,
                          const std::string &name) {
         auto operands = detail::get_kernel_metadata_operands(f, name);
         if (operands.size()) {
            auto value = ::llvm::cast< ::llvm::ConstantAsMetadata>(operands[0])
                                                               ->getValue();
            auto type = ::llvm::cast< ::llvm::UndefValue>(value)
                                                               ->getType();

            value = ::llvm::cast< ::llvm::ConstantAsMetadata>(operands[1])
                                                               ->getValue();
            bool is_signed = ::llvm::cast< ::llvm::ConstantInt>(value)
                                                ->getLimitedValue(UINT_MAX);

            std::string data;
            if (type->isIntOrIntVectorTy()) {
               if (!is_signed)
                  data = "unsigned ";

               const auto size = type->getScalarSizeInBits();
               switch(size) {
                  case 8:
                     data += "char";
                     break;
                  case 16:
                     data += "short";
                     break;
                  case 32:
                     data += "int";
                     break;
                  case 64:
                     data += "long";
                     break;
               }
               if (compat::is_scalable_vector(type))
                  throw build_error("hit unexpected scalable vector");
               if (compat::is_fixed_vector(type))
                  data += std::to_string(compat::get_fixed_vector_elements(type));

            } else {
               ::llvm::raw_string_ostream os { data };
               type->print(os);
               os.flush();
            }

            return data;
         } else {
            return "";
         }
      }

      ///
      /// Extract the string metadata node \p name corresponding to the kernel
      /// argument given by \p arg.
      ///
      inline std::string
      get_str_argument_metadata(const ::llvm::Function &f,
                            const ::llvm::Argument &arg,
                            const std::string &name) {
         auto operands = detail::get_kernel_metadata_operands(f, name);
         if (operands.size() > arg.getArgNo()) {
            return ::llvm::cast< ::llvm::MDString>(operands[arg.getArgNo()])
               ->getString().str();
         } else {
            return "";
         }
      }

      ///
      /// Extract the int metadata node \p name corresponding to the kernel
      /// argument given by \p arg.
      ///
      inline uint64_t
      get_uint_argument_metadata(const ::llvm::Function &f,
                            const ::llvm::Argument &arg,
                            const std::string &name) {
         auto operands = detail::get_kernel_metadata_operands(f, name);
         if (operands.size() >= arg.getArgNo()) {
            auto meta_arg_value = ::llvm::cast< ::llvm::ConstantAsMetadata>(
               operands[arg.getArgNo()])->getValue();
            return ::llvm::cast< ::llvm::ConstantInt>(meta_arg_value)
               ->getLimitedValue(UINT_MAX);
         } else {
            return 0;
         }
      }

      ///
      /// Return a vector with all CL kernel functions found in the LLVM
      /// module \p mod.
      ///
      inline std::vector<const ::llvm::Function *>
      get_kernels(const ::llvm::Module &mod) {
         std::vector<const ::llvm::Function *> fs;

         for (auto &f : mod.getFunctionList()) {
            if (detail::is_kernel(f))
               fs.push_back(&f);
         }

         return fs;
      }
   }
}

#endif
