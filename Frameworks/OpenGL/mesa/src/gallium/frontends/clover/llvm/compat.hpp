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
/// Some thin wrappers around the Clang/LLVM API used to preserve
/// compatibility with older API versions while keeping the ifdef clutter low
/// in the rest of the clover::llvm subtree.  In case of an API break please
/// consider whether it's possible to preserve backwards compatibility by
/// introducing a new one-liner inline function or typedef here under the
/// compat namespace in order to keep the running code free from preprocessor
/// conditionals.
///

#ifndef CLOVER_LLVM_COMPAT_HPP
#define CLOVER_LLVM_COMPAT_HPP

#include "util/algorithm.hpp"

#include <llvm/Config/llvm-config.h>

#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PreprocessorOptions.h>

#if LLVM_VERSION_MAJOR >= 14
#include <llvm/MC/TargetRegistry.h>
#else
#include <llvm/Support/TargetRegistry.h>
#endif

#if LLVM_VERSION_MAJOR >= 17
#include <llvm/TargetParser/Triple.h>
#else
#include <llvm/ADT/Triple.h>
#endif

namespace clover {
   namespace llvm {
      namespace compat {

#if LLVM_VERSION_MAJOR >= 18
         const auto CGFT_ObjectFile = ::llvm::CodeGenFileType::ObjectFile;
         const auto CGFT_AssemblyFile = ::llvm::CodeGenFileType::AssemblyFile;
#else
         const auto CGFT_ObjectFile = ::llvm::CGFT_ObjectFile;
         const auto CGFT_AssemblyFile = ::llvm::CGFT_AssemblyFile;
#endif
         typedef ::llvm::CodeGenFileType CodeGenFileType;

         const clang::InputKind ik_opencl = clang::Language::OpenCL;

         template<typename T> inline bool
         create_compiler_invocation_from_args(clang::CompilerInvocation &cinv,
                                              T copts,
                                              clang::DiagnosticsEngine &diag)
         {
            return clang::CompilerInvocation::CreateFromArgs(
               cinv, copts, diag);
         }

         static inline void
         compiler_set_lang_defaults(std::unique_ptr<clang::CompilerInstance> &c,
                                    clang::InputKind ik, const ::llvm::Triple& triple,
                                    clang::LangStandard::Kind d)
         {
#if LLVM_VERSION_MAJOR >= 15
            c->getLangOpts().setLangDefaults(c->getLangOpts(), ik.getLanguage(), triple,
#else
            c->getInvocation().setLangDefaults(c->getLangOpts(), ik, triple,
#endif
#if LLVM_VERSION_MAJOR >= 12
                                               c->getPreprocessorOpts().Includes,
#else
                                               c->getPreprocessorOpts(),
#endif
                                               d);
         }

         static inline unsigned
         get_abi_type_alignment(::llvm::DataLayout dl, ::llvm::Type *type)
         {
#if LLVM_VERSION_MAJOR >= 16
            return dl.getABITypeAlign(type).value();
#else
            return dl.getABITypeAlignment(type);
#endif
         }

         static inline bool
         is_scalable_vector(const ::llvm::Type *type)
         {
            return ::llvm::isa<::llvm::ScalableVectorType>(type);
         }

         static inline bool
         is_fixed_vector(const ::llvm::Type *type)
         {
            return ::llvm::isa<::llvm::FixedVectorType>(type);
         }

         static inline unsigned
         get_fixed_vector_elements(const ::llvm::Type *type)
         {
            return ::llvm::cast<::llvm::FixedVectorType>(type)->getNumElements();
         }
      }
   }
}

#endif
