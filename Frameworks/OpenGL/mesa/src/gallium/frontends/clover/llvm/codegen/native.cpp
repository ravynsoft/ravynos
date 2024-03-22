//
// Copyright 2012-2016 Francisco Jerez
// Copyright 2012-2016 Advanced Micro Devices, Inc.
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
/// Generate code using an arbitrary LLVM back-end capable of emitting
/// executable code as an ELF object file.
///

#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "llvm/codegen.hpp"
#include "llvm/compat.hpp"
#include "llvm/util.hpp"
#include "core/error.hpp"

using clover::binary;
using clover::build_error;
using namespace clover::llvm;
using ::llvm::TargetMachine;

#if defined(USE_LIBELF)

#include <libelf.h>
#include <gelf.h>

namespace {
   namespace elf {
      std::unique_ptr<Elf, int (*)(Elf *)>
      get(const std::vector<char> &code) {
         // One of the libelf implementations
         // (http://www.mr511.de/software/english.htm) requires calling
         // elf_version() before elf_memory().
         elf_version(EV_CURRENT);
         return { elf_memory(const_cast<char *>(code.data()), code.size()),
                  elf_end };
      }

      Elf_Scn *
      get_symbol_table(Elf *elf) {
         size_t section_str_index;
         elf_getshdrstrndx(elf, &section_str_index);

         for (Elf_Scn *s = elf_nextscn(elf, NULL); s; s = elf_nextscn(elf, s)) {
            GElf_Shdr header;
            if (gelf_getshdr(s, &header) != &header)
               return nullptr;

            if (!std::strcmp(elf_strptr(elf, section_str_index, header.sh_name),
                             ".symtab"))
               return s;
         }

         return nullptr;
      }

      std::map<std::string, unsigned>
      get_symbol_offsets(Elf *elf, Elf_Scn *symtab) {
         Elf_Data *const symtab_data = elf_getdata(symtab, NULL);
         GElf_Shdr header;
         if (gelf_getshdr(symtab, &header) != &header)
            return {};

         std::map<std::string, unsigned> symbol_offsets;
         GElf_Sym symbol;
         unsigned i = 0;

         while (GElf_Sym *s = gelf_getsym(symtab_data, i++, &symbol)) {
            const char *name = elf_strptr(elf, header.sh_link, s->st_name);
            symbol_offsets[name] = s->st_value;
         }

         return symbol_offsets;
      }
   }

   std::map<std::string, unsigned>
   get_symbol_offsets(const std::vector<char> &code, std::string &r_log) {
      const auto elf = elf::get(code);
      const auto symtab = elf::get_symbol_table(elf.get());
      if (!symtab)
         fail(r_log, build_error(), "Unable to find symbol table.");

      return elf::get_symbol_offsets(elf.get(), symtab);
   }

   std::vector<char>
   emit_code(::llvm::Module &mod, const target &target,
             compat::CodeGenFileType ft,
             std::string &r_log) {
      std::string err;
      auto t = ::llvm::TargetRegistry::lookupTarget(target.triple, err);
      if (!t)
         fail(r_log, build_error(), err);

      std::unique_ptr<TargetMachine> tm {
         t->createTargetMachine(target.triple, target.cpu, "", {},
#if LLVM_VERSION_MAJOR >= 16
                                std::nullopt, std::nullopt,
#else
                                ::llvm::None, ::llvm::None,
#endif
#if LLVM_VERSION_MAJOR >= 18
                                ::llvm::CodeGenOptLevel::Default) };
#else
                                ::llvm::CodeGenOpt::Default) };
#endif
      if (!tm)
         fail(r_log, build_error(),
              "Could not create TargetMachine: " + target.triple);

      ::llvm::SmallVector<char, 1024> data;

      {
         ::llvm::legacy::PassManager pm;
         ::llvm::raw_svector_ostream os { data };

         mod.setDataLayout(tm->createDataLayout());
         tm->Options.MCOptions.AsmVerbose =
            (ft == compat::CGFT_AssemblyFile);

         if (tm->addPassesToEmitFile(pm, os, nullptr, ft))
            fail(r_log, build_error(), "TargetMachine can't emit this file");

         pm.run(mod);
      }

      return { data.begin(), data.end() };
   }
}

binary
clover::llvm::build_module_native(::llvm::Module &mod, const target &target,
                                  const clang::CompilerInstance &c,
                                  std::string &r_log) {
   const auto code = emit_code(mod, target,
                               compat::CGFT_ObjectFile, r_log);
   return build_module_common(mod, code, get_symbol_offsets(code, r_log), c);
}

std::string
clover::llvm::print_module_native(const ::llvm::Module &mod,
                                  const target &target) {
   std::string log;
   try {
      std::unique_ptr< ::llvm::Module> cmod { ::llvm::CloneModule(mod) };
      return as_string(emit_code(*cmod, target,
                                 compat::CGFT_AssemblyFile, log));
   } catch (...) {
      return "Couldn't output native disassembly: " + log;
   }
}

#else

binary
clover::llvm::build_module_native(::llvm::Module &mod, const target &target,
                                  const clang::CompilerInstance &c,
                                  std::string &r_log) {
   unreachable("Native codegen support disabled at build time");
}

std::string
clover::llvm::print_module_native(const ::llvm::Module &mod,
                                  const target &target) {
   unreachable("Native codegen support disabled at build time");
}

#endif
