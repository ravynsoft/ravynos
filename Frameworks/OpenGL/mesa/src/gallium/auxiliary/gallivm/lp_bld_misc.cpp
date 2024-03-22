/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


/**
 * The purpose of this module is to expose LLVM functionality not available
 * through the C++ bindings.
 */


// Undef these vars just to silence warnings
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION


#include <stddef.h>

#include <llvm/Config/llvm-config.h>

#if LLVM_VERSION_MAJOR < 7
// Workaround http://llvm.org/PR23628
#pragma push_macro("DEBUG")
#undef DEBUG
#endif

#include <llvm/Config/llvm-config.h>
#include <llvm-c/Core.h>
#include <llvm-c/Support.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/CodeGen/SelectionDAGNodes.h>
#if LLVM_VERSION_MAJOR >= 15
#include <llvm/Support/MemoryBuffer.h>
#endif

#if LLVM_VERSION_MAJOR >= 17
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#else
#include <llvm/Support/Host.h>
#include <llvm/ADT/Triple.h>
#endif

#if LLVM_VERSION_MAJOR < 11
#include <llvm/IR/CallSite.h>
#endif
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CBindingWrapping.h>

#include <llvm/Config/llvm-config.h>
#if LLVM_USE_INTEL_JITEVENTS
#include <llvm/ExecutionEngine/JITEventListener.h>
#endif

#if LLVM_VERSION_MAJOR < 7
// Workaround http://llvm.org/PR23628
#pragma pop_macro("DEBUG")
#endif

#include "c11/threads.h"
#include "util/u_thread.h"
#include "util/detect.h"
#include "util/u_debug.h"
#include "util/u_cpu_detect.h"

#include "lp_bld_misc.h"
#include "lp_bld_debug.h"

static void lp_run_atexit_for_destructors(void);

namespace {

class LLVMEnsureMultithreaded {
public:
   LLVMEnsureMultithreaded()
   {
      if (!LLVMIsMultithreaded()) {
         LLVMStartMultithreaded();
      }
   }
};

static LLVMEnsureMultithreaded lLVMEnsureMultithreaded;

}

static once_flag init_native_targets_once_flag = ONCE_FLAG_INIT;

static void init_native_targets()
{
   // If we have a native target, initialize it to ensure it is linked in and
   // usable by the JIT.
   llvm::InitializeNativeTarget();

   llvm::InitializeNativeTargetAsmPrinter();

   llvm::InitializeNativeTargetDisassembler();
#if DEBUG
   {
      char *env_llc_options = getenv("GALLIVM_LLC_OPTIONS");
      if (env_llc_options) {
         char *option;
         char *options[64] = {(char *) "llc"};      // Warning without cast
         int   n;
         for (n = 0, option = strtok(env_llc_options, " "); option; n++, option = strtok(NULL, " ")) {
            options[n + 1] = option;
         }
         if (gallivm_debug & (GALLIVM_DEBUG_IR | GALLIVM_DEBUG_ASM | GALLIVM_DEBUG_DUMP_BC)) {
            debug_printf("llc additional options (%d):\n", n);
            for (int i = 1; i <= n; i++)
               debug_printf("\t%s\n", options[i]);
            debug_printf("\n");
         }
         LLVMParseCommandLineOptions(n + 1, options, NULL);
      }
   }
#endif
   lp_run_atexit_for_destructors();
}

extern "C" void
lp_set_target_options(void)
{
   /* The llvm target registry is not thread-safe, so drivers and gallium frontends
    * that want to initialize targets should use the lp_set_target_options()
    * function to safely initialize targets.
    *
    * LLVM targets should be initialized before the driver or gallium frontend tries
    * to access the registry.
    */
   call_once(&init_native_targets_once_flag, init_native_targets);
}

extern "C"
LLVMTargetLibraryInfoRef
gallivm_create_target_library_info(const char *triple)
{
   return reinterpret_cast<LLVMTargetLibraryInfoRef>(
   new llvm::TargetLibraryInfoImpl(
   llvm::Triple(triple)));
}

extern "C"
void
gallivm_dispose_target_library_info(LLVMTargetLibraryInfoRef library_info)
{
   delete reinterpret_cast<
   llvm::TargetLibraryInfoImpl
   *>(library_info);
}


typedef llvm::RTDyldMemoryManager BaseMemoryManager;


/*
 * Delegating is tedious but the default manager class is hidden in an
 * anonymous namespace in LLVM, so we cannot just derive from it to change
 * its behavior.
 */
class DelegatingJITMemoryManager : public BaseMemoryManager {

   protected:
      virtual BaseMemoryManager *mgr() const = 0;

   public:
      /*
       * From RTDyldMemoryManager
       */
      virtual uint8_t *allocateCodeSection(uintptr_t Size,
                                           unsigned Alignment,
                                           unsigned SectionID,
                                           llvm::StringRef SectionName) {
         return mgr()->allocateCodeSection(Size, Alignment, SectionID,
                                           SectionName);
      }
      virtual uint8_t *allocateDataSection(uintptr_t Size,
                                           unsigned Alignment,
                                           unsigned SectionID,
                                           llvm::StringRef SectionName,
                                           bool IsReadOnly) {
         return mgr()->allocateDataSection(Size, Alignment, SectionID,
                                           SectionName,
                                           IsReadOnly);
      }
      virtual void registerEHFrames(uint8_t *Addr, uint64_t LoadAddr, size_t Size) {
         mgr()->registerEHFrames(Addr, LoadAddr, Size);
      }
#if LLVM_VERSION_MAJOR >= 5
      virtual void deregisterEHFrames() {
         mgr()->deregisterEHFrames();
      }
#else
      virtual void deregisterEHFrames(uint8_t *Addr, uint64_t LoadAddr, size_t Size) {
         mgr()->deregisterEHFrames(Addr, LoadAddr, Size);
      }
#endif
      virtual void *getPointerToNamedFunction(const std::string &Name,
                                              bool AbortOnFailure=true) {
         return mgr()->getPointerToNamedFunction(Name, AbortOnFailure);
      }
      virtual bool finalizeMemory(std::string *ErrMsg = 0) {
         return mgr()->finalizeMemory(ErrMsg);
      }
};


/*
 * Delegate memory management to one shared manager for more efficient use
 * of memory than creating a separate pool for each LLVM engine.
 * Keep generated code until freeGeneratedCode() is called, instead of when
 * memory manager is destroyed, which happens during engine destruction.
 * This allows additional memory savings as we don't have to keep the engine
 * around in order to use the code.
 * All methods are delegated to the shared manager except destruction and
 * deallocating code.  For the latter we just remember what needs to be
 * deallocated later.  The shared manager is deleted once it is empty.
 */
class ShaderMemoryManager : public DelegatingJITMemoryManager {

   BaseMemoryManager *TheMM;

   struct GeneratedCode {
      typedef std::vector<void *> Vec;
      Vec FunctionBody, ExceptionTable;
      BaseMemoryManager *TheMM;

      GeneratedCode(BaseMemoryManager *MM) {
         TheMM = MM;
      }

      ~GeneratedCode() {
      }
   };

   GeneratedCode *code;

   BaseMemoryManager *mgr() const {
      return TheMM;
   }

   public:

      ShaderMemoryManager(BaseMemoryManager* MM) {
         TheMM = MM;
         code = new GeneratedCode(MM);
      }

      virtual ~ShaderMemoryManager() {
         /*
          * 'code' is purposely not deleted.  It is the user's responsibility
          * to call getGeneratedCode() and freeGeneratedCode().
          */
      }

      struct lp_generated_code *getGeneratedCode() {
         return (struct lp_generated_code *) code;
      }

      static void freeGeneratedCode(struct lp_generated_code *code) {
         delete (GeneratedCode *) code;
      }

      virtual void deallocateFunctionBody(void *Body) {
         // remember for later deallocation
         code->FunctionBody.push_back(Body);
      }
};

class LPObjectCache : public llvm::ObjectCache {
private:
   bool has_object;
   struct lp_cached_code *cache_out;
public:
   LPObjectCache(struct lp_cached_code *cache) {
      cache_out = cache;
      has_object = false;
   }

   ~LPObjectCache() {
   }
   void notifyObjectCompiled(const llvm::Module *M, llvm::MemoryBufferRef Obj) {
      const std::string ModuleID = M->getModuleIdentifier();
      if (has_object)
         fprintf(stderr, "CACHE ALREADY HAS MODULE OBJECT\n");
      has_object = true;
      cache_out->data_size = Obj.getBufferSize();
      cache_out->data = malloc(cache_out->data_size);
      memcpy(cache_out->data, Obj.getBufferStart(), cache_out->data_size);
   }

   virtual std::unique_ptr<llvm::MemoryBuffer> getObject(const llvm::Module *M) {
      if (cache_out->data_size) {
         return llvm::MemoryBuffer::getMemBuffer(llvm::StringRef((const char *)cache_out->data, cache_out->data_size), "", false);
      }
      return NULL;
   }

};

/**
 * Same as LLVMCreateJITCompilerForModule, but:
 * - allows using MCJIT and enabling AVX feature where available.
 * - set target options
 *
 * See also:
 * - llvm/lib/ExecutionEngine/ExecutionEngineBindings.cpp
 * - llvm/tools/lli/lli.cpp
 * - http://markmail.org/message/ttkuhvgj4cxxy2on#query:+page:1+mid:aju2dggerju3ivd3+state:results
 */
extern "C"
LLVMBool
lp_build_create_jit_compiler_for_module(LLVMExecutionEngineRef *OutJIT,
                                        lp_generated_code **OutCode,
                                        struct lp_cached_code *cache_out,
                                        LLVMModuleRef M,
                                        LLVMMCJITMemoryManagerRef CMM,
                                        unsigned OptLevel,
                                        char **OutError)
{
   using namespace llvm;

   std::string Error;
   EngineBuilder builder(std::unique_ptr<Module>(unwrap(M)));

   /**
    * LLVM 3.1+ haven't more "extern unsigned llvm::StackAlignmentOverride" and
    * friends for configuring code generation options, like stack alignment.
    */
   TargetOptions options;
#if DETECT_ARCH_X86 && LLVM_VERSION_MAJOR < 13
   options.StackAlignmentOverride = 4;
#endif

   builder.setEngineKind(EngineKind::JIT)
          .setErrorStr(&Error)
          .setTargetOptions(options)
#if LLVM_VERSION_MAJOR >= 18
          .setOptLevel((CodeGenOptLevel)OptLevel);
#else
          .setOptLevel((CodeGenOpt::Level)OptLevel);
#endif

#if DETECT_OS_WINDOWS
    /*
     * MCJIT works on Windows, but currently only through ELF object format.
     *
     * XXX: We could use `LLVM_HOST_TRIPLE "-elf"` but LLVM_HOST_TRIPLE has
     * different strings for MinGW/MSVC, so better play it safe and be
     * explicit.
     */
#  if DETECT_ARCH_X86_64
    LLVMSetTarget(M, "x86_64-pc-win32-elf");
#  elif DETECT_ARCH_X86
    LLVMSetTarget(M, "i686-pc-win32-elf");
#  elif DETECT_ARCH_AARCH64
    LLVMSetTarget(M, "aarch64-pc-win32-elf");
#  else
#    error Unsupported architecture for MCJIT on Windows.
#  endif
#endif

   llvm::SmallVector<std::string, 16> MAttrs;

#if DETECT_ARCH_ARM
   /* llvm-3.3+ implements sys::getHostCPUFeatures for Arm,
    * which allows us to enable/disable code generation based
    * on the results of cpuid on these architectures.
    */
   llvm::StringMap<bool> features;
   llvm::sys::getHostCPUFeatures(features);

   for (StringMapIterator<bool> f = features.begin();
        f != features.end();
        ++f) {
      MAttrs.push_back(((*f).second ? "+" : "-") + (*f).first().str());
   }
#elif DETECT_ARCH_X86 || DETECT_ARCH_X86_64
   /*
    * Because we can override cpu caps with environment variables,
    * so we do not use llvm::sys::getHostCPUFeatures to detect cpu features
    * but using util_get_cpu_caps() instead.
    */
#if DETECT_ARCH_X86_64
   /*
    * Without this, on some "buggy" qemu cpu setup, LLVM could crash
    * if LLVM detects the wrong CPU type.
    */
   MAttrs.push_back("+64bit");
#endif
   MAttrs.push_back(util_get_cpu_caps()->has_sse    ? "+sse"    : "-sse"   );
   MAttrs.push_back(util_get_cpu_caps()->has_sse2   ? "+sse2"   : "-sse2"  );
   MAttrs.push_back(util_get_cpu_caps()->has_sse3   ? "+sse3"   : "-sse3"  );
   MAttrs.push_back(util_get_cpu_caps()->has_ssse3  ? "+ssse3"  : "-ssse3" );
   MAttrs.push_back(util_get_cpu_caps()->has_sse4_1 ? "+sse4.1" : "-sse4.1");
   MAttrs.push_back(util_get_cpu_caps()->has_sse4_2 ? "+sse4.2" : "-sse4.2");
   /*
    * AVX feature is not automatically detected from CPUID by the X86 target
    * yet, because the old (yet default) JIT engine is not capable of
    * emitting the opcodes. On newer llvm versions it is and at least some
    * versions (tested with 3.3) will emit avx opcodes without this anyway.
    */
   MAttrs.push_back(util_get_cpu_caps()->has_avx  ? "+avx"  : "-avx");
   MAttrs.push_back(util_get_cpu_caps()->has_f16c ? "+f16c" : "-f16c");
   MAttrs.push_back(util_get_cpu_caps()->has_fma  ? "+fma"  : "-fma");
   MAttrs.push_back(util_get_cpu_caps()->has_avx2 ? "+avx2" : "-avx2");

   /* All avx512 have avx512f */
   MAttrs.push_back(util_get_cpu_caps()->has_avx512f ? "+avx512f"  : "-avx512f");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512cd ? "+avx512cd"  : "-avx512cd");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512er ? "+avx512er"  : "-avx512er");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512pf ? "+avx512pf"  : "-avx512pf");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512bw ? "+avx512bw"  : "-avx512bw");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512dq ? "+avx512dq"  : "-avx512dq");
   MAttrs.push_back(util_get_cpu_caps()->has_avx512vl ? "+avx512vl"  : "-avx512vl");
#endif
#if DETECT_ARCH_ARM
   if (!util_get_cpu_caps()->has_neon) {
      MAttrs.push_back("-neon");
      MAttrs.push_back("-crypto");
      MAttrs.push_back("-vfp2");
   }
#endif

#if DETECT_ARCH_PPC
   MAttrs.push_back(util_get_cpu_caps()->has_altivec ? "+altivec" : "-altivec");
   /*
    * Bug 25503 is fixed, by the same fix that fixed
    * bug 26775, in versions of LLVM later than 3.8 (starting with 3.8.1).
    * BZ 33531 actually comprises more than one bug, all of
    * which are fixed in LLVM 4.0.
    *
    * With LLVM 4.0 or higher:
    * Make sure VSX instructions are ENABLED (if supported), unless
    * VSX instructions are explicitly enabled/disabled via GALLIVM_VSX=1 or 0.
    */
   if (util_get_cpu_caps()->has_altivec) {
      MAttrs.push_back(util_get_cpu_caps()->has_vsx ? "+vsx" : "-vsx");
   }
#endif

#if DETECT_ARCH_MIPS64
   MAttrs.push_back(util_get_cpu_caps()->has_msa ? "+msa" : "-msa");
   /* MSA requires a 64-bit FPU register file */
   MAttrs.push_back("+fp64");
#endif

   builder.setMAttrs(MAttrs);

   if (gallivm_debug & (GALLIVM_DEBUG_IR | GALLIVM_DEBUG_ASM | GALLIVM_DEBUG_DUMP_BC)) {
      int n = MAttrs.size();
      if (n > 0) {
         debug_printf("llc -mattr option(s): ");
         for (int i = 0; i < n; i++)
            debug_printf("%s%s", MAttrs[i].c_str(), (i < n - 1) ? "," : "");
         debug_printf("\n");
      }
   }

   StringRef MCPU = llvm::sys::getHostCPUName();
   /*
    * The cpu bits are no longer set automatically, so need to set mcpu manually.
    * Note that the MAttrs set above will be sort of ignored (since we should
    * not set any which would not be set by specifying the cpu anyway).
    * It ought to be safe though since getHostCPUName() should include bits
    * not only from the cpu but environment as well (for instance if it's safe
    * to use avx instructions which need OS support). According to
    * http://llvm.org/bugs/show_bug.cgi?id=19429 however if I understand this
    * right it may be necessary to specify older cpu (or disable mattrs) though
    * when not using MCJIT so no instructions are generated which the old JIT
    * can't handle. Not entirely sure if we really need to do anything yet.
    */

#if DETECT_ARCH_PPC_64
   /*
    * Large programs, e.g. gnome-shell and firefox, may tax the addressability
    * of the Medium code model once dynamically generated JIT-compiled shader
    * programs are linked in and relocated.  Yet the default code model as of
    * LLVM 8 is Medium or even Small.
    * The cost of changing from Medium to Large is negligible:
    * - an additional 8-byte pointer stored immediately before the shader entrypoint;
    * - change an add-immediate (addis) instruction to a load (ld).
    */
   builder.setCodeModel(CodeModel::Large);

#if UTIL_ARCH_LITTLE_ENDIAN
   /*
    * Versions of LLVM prior to 4.0 lacked a table entry for "POWER8NVL",
    * resulting in (big-endian) "generic" being returned on
    * little-endian Power8NVL systems.  The result was that code that
    * attempted to load the least significant 32 bits of a 64-bit quantity
    * from memory loaded the wrong half.  This resulted in failures in some
    * Piglit tests, e.g.
    * .../arb_gpu_shader_fp64/execution/conversion/frag-conversion-explicit-double-uint
    */
   if (MCPU == "generic")
      MCPU = "pwr8";
#endif
#endif

#if DETECT_ARCH_MIPS64
      /*
       * ls3a4000 CPU and ls2k1000 SoC is a mips64r5 compatible with MSA SIMD
       * instruction set implemented, while ls3a3000 is mips64r2 compatible
       * only. getHostCPUName() return "generic" on all loongson
       * mips CPU currently. So we override the MCPU to mips64r5 if MSA is
       * implemented, feedback to mips64r2 for all other ordinary mips64 cpu.
       */
   if (MCPU == "generic")
      MCPU = util_get_cpu_caps()->has_msa ? "mips64r5" : "mips64r2";
#endif

   builder.setMCPU(MCPU);
   if (gallivm_debug & (GALLIVM_DEBUG_IR | GALLIVM_DEBUG_ASM | GALLIVM_DEBUG_DUMP_BC)) {
      debug_printf("llc -mcpu option: %s\n", MCPU.str().c_str());
   }

   ShaderMemoryManager *MM = NULL;
   BaseMemoryManager* JMM = reinterpret_cast<BaseMemoryManager*>(CMM);
   MM = new ShaderMemoryManager(JMM);
   *OutCode = MM->getGeneratedCode();

   builder.setMCJITMemoryManager(std::unique_ptr<RTDyldMemoryManager>(MM));
   MM = NULL; // ownership taken by std::unique_ptr

   ExecutionEngine *JIT;

   JIT = builder.create();

   if (cache_out) {
      LPObjectCache *objcache = new LPObjectCache(cache_out);
      JIT->setObjectCache(objcache);
      cache_out->jit_obj_cache = (void *)objcache;
   }

#if LLVM_USE_INTEL_JITEVENTS
   JITEventListener *JEL = JITEventListener::createIntelJITEventListener();
   JIT->RegisterJITEventListener(JEL);
#endif
   if (JIT) {
      *OutJIT = wrap(JIT);
      return 0;
   }
   lp_free_generated_code(*OutCode);
   *OutCode = 0;
   delete MM;
   *OutError = strdup(Error.c_str());
   return 1;
}


extern "C"
void
lp_free_generated_code(struct lp_generated_code *code)
{
   ShaderMemoryManager::freeGeneratedCode(code);
}

extern "C"
LLVMMCJITMemoryManagerRef
lp_get_default_memory_manager()
{
   BaseMemoryManager *mm;
   mm = new llvm::SectionMemoryManager();
   return reinterpret_cast<LLVMMCJITMemoryManagerRef>(mm);
}

extern "C"
void
lp_free_memory_manager(LLVMMCJITMemoryManagerRef memorymgr)
{
   delete reinterpret_cast<BaseMemoryManager*>(memorymgr);
}

extern "C" void
lp_free_objcache(void *objcache_ptr)
{
   LPObjectCache *objcache = (LPObjectCache *)objcache_ptr;
   delete objcache;
}

extern "C" LLVMValueRef
lp_get_called_value(LLVMValueRef call)
{
	return LLVMGetCalledValue(call);
}

extern "C" bool
lp_is_function(LLVMValueRef v)
{
	return LLVMGetValueKind(v) == LLVMFunctionValueKind;
}

extern "C" void
lp_set_module_stack_alignment_override(LLVMModuleRef MRef, unsigned align)
{
#if LLVM_VERSION_MAJOR >= 13
   llvm::Module *M = llvm::unwrap(MRef);
   M->setOverrideStackAlignment(align);
#endif
}

using namespace llvm;

class GallivmRunAtExitForStaticDestructors : public SDNode
{
public:
   /* getSDVTList (protected) calls getValueTypeList (private), which contains static variables. */
   GallivmRunAtExitForStaticDestructors(): SDNode(0, 0, DebugLoc(), getSDVTList(MVT::Other))
   {
   }
};

static void
lp_run_atexit_for_destructors(void)
{
   /* LLVM >= 16 registers static variable destructors on the first compile, which gcc
    * implements by calling atexit there. Before that, u_queue registers its atexit
    * handler to kill all threads. Since exit() runs atexit handlers in the reverse order,
    * the LLVM destructors are called first while shader compiler threads may still be
    * running, which crashes in LLVM in SelectionDAG.cpp.
    *
    * The solution is to run the code that declares the LLVM static variables first,
    * so that atexit for LLVM is registered first and u_queue is registered after that,
    * which ensures that all u_queue threads are terminated before LLVM destructors are
    * called.
    *
    * This just executes the code that declares static variables.
    */
   GallivmRunAtExitForStaticDestructors();
}
