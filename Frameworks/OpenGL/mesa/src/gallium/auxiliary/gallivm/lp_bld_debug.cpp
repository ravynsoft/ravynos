/**************************************************************************
 *
 * Copyright 2009-2011 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <stddef.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <llvm/Config/llvm-config.h>
#include <llvm-c/Core.h>
#include <llvm-c/Disassembler.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Format.h>
#include <llvm/IR/Module.h>

#if LLVM_VERSION_MAJOR >= 17
#include <llvm/TargetParser/Host.h>
#else
#include <llvm/Support/Host.h>
#endif

#include "util/u_math.h"
#include "util/u_debug.h"

#include "lp_bld_debug.h"

#ifdef __linux__
#include <sys/stat.h>
#include <fcntl.h>
#endif



/**
 * Check alignment.
 *
 * It is important that this check is not implemented as a macro or inlined
 * function, as the compiler assumptions in respect to alignment of global
 * and stack variables would often make the check a no op, defeating the
 * whole purpose of the exercise.
 */
extern "C" bool
lp_check_alignment(const void *ptr, unsigned alignment)
{
   assert(util_is_power_of_two_or_zero(alignment));
   return ((uintptr_t)ptr & (alignment - 1)) == 0;
}


/**
 * Same as LLVMDumpValue, but through our debugging channels.
 */
extern "C" void
lp_debug_dump_value(LLVMValueRef value)
{
   char *str = LLVMPrintValueToString(value);
   if (str) {
      os_log_message(str);
      LLVMDisposeMessage(str);
   }
}


/*
 * Disassemble a function, using the LLVM MC disassembler.
 *
 * See also:
 * - http://blog.llvm.org/2010/01/x86-disassembler.html
 * - http://blog.llvm.org/2010/04/intro-to-llvm-mc-project.html
 */
static size_t
disassemble(const void* func, std::ostream &buffer)
{
   const uint8_t *bytes = (const uint8_t *)func;

   /*
    * Limit disassembly to this extent
    */
   const uint64_t extent = 96 * 1024;

   /*
    * Initialize all used objects.
    */

   const char *triple = LLVM_HOST_TRIPLE;
   LLVMDisasmContextRef D = LLVMCreateDisasm(triple, NULL, 0, NULL, NULL);
   char outline[1024];

   if (!D) {
      buffer << "error: could not create disassembler for triple "
             << triple << '\n';
      return 0;
   }

   uint64_t pc;
   pc = 0;
   while (pc < extent) {
      size_t Size;

      /*
       * Print address.  We use addresses relative to the start of the function,
       * so that between runs.
       */

      buffer << std::setw(6) << (unsigned long)pc << ":\t";

      Size = LLVMDisasmInstruction(D, (uint8_t *)bytes + pc, extent - pc, 0, outline,
                                   sizeof outline);

      if (!Size) {
         buffer << "invalid\n";
         pc += 1;
         break;
      }

      /*
       * Output the bytes in hexidecimal format.
       */

      if (0) {
         unsigned i;
         for (i = 0; i < Size; ++i) {
            buffer << std::hex << std::setfill('0') << std::setw(2)
                   << static_cast<int> (bytes[pc + i]);
         }
         for (; i < 16; ++i) {
            buffer << std::dec << "   ";
         }
      }

      /*
       * Print the instruction.
       */

      buffer << std::setw(Size) << outline << '\n';

      /*
       * Stop disassembling on return statements, if there is no record of a
       * jump to a successive address.
       *
       * XXX: This currently assumes x86
       */

#if DETECT_ARCH_X86 || DETECT_ARCH_X86_64
      if (Size == 1 && bytes[pc] == 0xc3) {
         break;
      }
#endif

      /*
       * Advance.
       */

      pc += Size;

      if (pc >= extent) {
         buffer << "disassembly larger than " << extent << " bytes, aborting\n";
         break;
      }
   }

   buffer << '\n';

   LLVMDisasmDispose(D);

   /*
    * Print GDB command, useful to verify output.
    */
   if (0) {
      buffer << "disassemble " << static_cast<const void*>(bytes) << ' '
             << static_cast<const void*>(bytes + pc) << '\n';
   }

   return pc;
}


extern "C" void
lp_disassemble(LLVMValueRef func, const void *code)
{
   std::ostringstream buffer;
   std::string s;

   buffer << LLVMGetValueName(func) << ":\n";
   disassemble(code, buffer);
   s = buffer.str();
   os_log_message(s.c_str());
   os_log_message("\n");
}


/*
 * Linux perf profiler integration.
 *
 * See also:
 * - http://penberg.blogspot.co.uk/2009/06/jato-has-profiler.html
 * - https://github.com/penberg/jato/commit/73ad86847329d99d51b386f5aba692580d1f8fdc
 * - http://git.kernel.org/?p=linux/kernel/git/torvalds/linux.git;a=commitdiff;h=80d496be89ed7dede5abee5c057634e80a31c82d
 */
extern "C" void
lp_profile(LLVMValueRef func, const void *code)
{
#if defined(__linux__) && defined(PROFILE)
   static std::ofstream perf_asm_file;
   static bool first_time = true;
   static FILE *perf_map_file = NULL;
   if (first_time) {
      /*
       * We rely on the disassembler for determining a function's size, but
       * the disassembly is a leaky and slow operation, so avoid running
       * this except when running inside linux perf, which can be inferred
       * by the PERF_BUILDID_DIR environment variable.
       */
      if (getenv("PERF_BUILDID_DIR")) {
         pid_t pid = getpid();
         char filename[256];
         snprintf(filename, sizeof filename, "/tmp/perf-%llu.map", (unsigned long long)pid);
         perf_map_file = fopen(filename, "wt");
         snprintf(filename, sizeof filename, "/tmp/perf-%llu.map.asm", (unsigned long long)pid);
         perf_asm_file.open(filename);
      }
      first_time = false;
   }
   if (perf_map_file) {
      const char *symbol = LLVMGetValueName(func);
      unsigned long addr = (uintptr_t)code;
      perf_asm_file << symbol << ":\n";
      unsigned long size = disassemble(code, perf_asm_file);
      perf_asm_file.flush();
      fprintf(perf_map_file, "%lx %lx %s\n", addr, size, symbol);
      fflush(perf_map_file);
   }
#else
   (void)func;
   (void)code;
#endif
}


