/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <alloca.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "memmgr.h"
#include "tsd.h"

/* Get dynamic module interface*/
#include "collector_module.h"

/* Get definitions for SP_LEAF_CHECK_MARKER, SP_TRUNC_STACK_MARKER */
#include "data_pckts.h"

#if ARCH(SPARC)
struct frame
{
  long fr_local[8];         /* saved locals */
  long fr_arg[6];           /* saved arguments [0 - 5] */
  struct frame *fr_savfp;   /* saved frame pointer */
  long fr_savpc;            /* saved program counter */
#if WSIZE(32)
  char *fr_stret;           /* struct return addr */
#endif
  long fr_argd[6];          /* arg dump area */
  long fr_argx[1];          /* array of args past the sixth */
};

#elif ARCH(Intel)
struct frame
{
  unsigned long fr_savfp;
  unsigned long fr_savpc;
};
#endif

/* Set the debug trace level */
#define DBG_LT0 0
#define DBG_LT1	1
#define DBG_LT2	2
#define DBG_LT3	3

int (*__collector_VM_ReadByteInstruction)(unsigned char *) = NULL;
#define VM_NO_ACCESS        (-1)
#define VM_NOT_VM_MEMORY    (-2)
#define VM_NOT_X_SEGMENT    (-3)

#define isInside(p, bgn, end) ((p) >= (bgn) && (p) < (end))

/*
 * Weed through all the arch dependent stuff to get the right definition
 * for 'pc' in the ucontext structure.  The system header files are mess
 * dealing with all the arch (just look for PC, R_PC, REG_PC).
 *
 */

#if ARCH(SPARC)

#define IN_BARRIER(x) \
	( barrier_hdl && \
	  (unsigned long)x >= barrier_hdl && \
	  (unsigned long)x < barrier_hdlx )
static unsigned long barrier_hdl = 0;
static unsigned long barrier_hdlx = 0;

#if WSIZE(64)
#define STACK_BIAS 2047
#define IN_TRAP_HANDLER(x) \
	( misalign_hdl && \
	  (unsigned long)x >= misalign_hdl && \
	  (unsigned long)x < misalign_hdlx )
static unsigned long misalign_hdl = 0;
static unsigned long misalign_hdlx = 0;
#elif  WSIZE(32)
#define STACK_BIAS 0
#endif

#if WSIZE(64)
#define GET_GREG(ctx,reg) (((ucontext_t*)ctx)->uc_mcontext.mc_gregs[(reg)])
#define GET_SP(ctx) (((ucontext_t*)ctx)->uc_mcontext.mc_gregs[MC_O6])
#define GET_PC(ctx) (((ucontext_t*)ctx)->uc_mcontext.mc_gregs[MC_PC])
#else
#define GET_GREG(ctx,reg) (((ucontext_t*)ctx)->uc_mcontext.gregs[(reg)])
#define GET_SP(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_O6])
#define GET_PC(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_PC])
#endif

#elif ARCH(Intel)
#include "opcodes/disassemble.h"

static int
fprintf_func (void *arg ATTRIBUTE_UNUSED, const char *fmt ATTRIBUTE_UNUSED, ...)
{
  return 0;
}

static int
fprintf_styled_func (void *arg ATTRIBUTE_UNUSED,
		      enum disassembler_style st ATTRIBUTE_UNUSED,
		      const char *fmt ATTRIBUTE_UNUSED, ...)
{
  return 0;
}

/* Get LENGTH bytes from info's buffer, at target address memaddr.
   Transfer them to myaddr.  */
static int
read_memory_func (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
		  disassemble_info *info)
{
  unsigned int opb = info->octets_per_byte;
  size_t end_addr_offset = length / opb;
  size_t max_addr_offset = info->buffer_length / opb;
  size_t octets = (memaddr - info->buffer_vma) * opb;
  if (memaddr < info->buffer_vma
      || memaddr - info->buffer_vma > max_addr_offset
      || memaddr - info->buffer_vma + end_addr_offset > max_addr_offset
      || (info->stop_vma && (memaddr >= info->stop_vma
			     || memaddr + end_addr_offset > info->stop_vma)))
    return -1;
  memcpy (myaddr, info->buffer + octets, length);
  return 0;
}

static void
print_address_func (bfd_vma addr ATTRIBUTE_UNUSED,
		    disassemble_info *info ATTRIBUTE_UNUSED) { }

static asymbol *
symbol_at_address_func (bfd_vma addr ATTRIBUTE_UNUSED,
			disassemble_info *info ATTRIBUTE_UNUSED)
{
  return NULL;
}

static bfd_boolean
symbol_is_valid (asymbol *sym ATTRIBUTE_UNUSED,
		 disassemble_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

static void
memory_error_func (int status ATTRIBUTE_UNUSED, bfd_vma addr ATTRIBUTE_UNUSED,
		   disassemble_info *info ATTRIBUTE_UNUSED) { }


#if WSIZE(32)
#define GET_PC(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_EIP])
#define GET_SP(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_ESP])
#define GET_FP(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_EBP])

#elif WSIZE(64)
#define GET_PC(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP])
#define GET_SP(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RSP])
#define GET_FP(ctx) (((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RBP])
#endif /* WSIZE() */

#elif ARCH(Aarch64)
#define GET_PC(ctx) (((ucontext_t*)ctx)->uc_mcontext.regs[15])
#define GET_SP(ctx) (((ucontext_t*)ctx)->uc_mcontext.regs[13])
#define GET_FP(ctx) (((ucontext_t*)ctx)->uc_mcontext.regs[14])
#endif /* ARCH() */

/*
 * FILL_CONTEXT() for all platforms
 * Could use getcontext() except:
 * - it's not guaranteed to be async signal safe
 * - it's a system call and not that lightweight
 * - it's not portable as of POSIX.1-2008
 * So we just use low-level mechanisms to fill in the few fields we need.
 */
#if ARCH(SPARC)
#if WSIZE(32)
#define FILL_CONTEXT(context) \
	{ \
	greg_t fp; \
	__asm__ __volatile__( "mov %%i6, %0" : "=r" (fp) ); \
	__asm__ __volatile__( "ta 3" ); \
	GET_SP(context) = fp; \
	GET_PC(context) = (greg_t)0; \
	}

#elif WSIZE(64)
#define FILL_CONTEXT(context) \
	{ \
	    greg_t fp; \
	    __asm__ __volatile__( "mov %%i6, %0" : "=r" (fp) ); \
	    __asm__ __volatile__( "flushw" ); \
	    GET_SP(context) = fp; \
	    GET_PC(context) = (greg_t)0; \
	}
#endif /* WSIZE() */

#elif ARCH(Intel)
#define FILL_CONTEXT(context) \
	{ \
	    context->uc_link = NULL; \
	    void *sp = __collector_getsp(); \
	    GET_SP(context) = (intptr_t)sp; \
	    GET_FP(context) = (intptr_t)__collector_getfp(); \
	    GET_PC(context) = (intptr_t)__collector_getpc(); \
	    context->uc_stack.ss_sp = sp; \
	    context->uc_stack.ss_size = 0x100000; \
	}

#elif ARCH(Aarch64)
#define FILL_CONTEXT(context) \
    { CALL_UTIL (getcontext) (context);  \
      context->uc_mcontext.sp = (__u64) __builtin_frame_address(0); \
    }

#endif /* ARCH() */

static int
getByteInstruction (unsigned char *p)
{
  if (__collector_VM_ReadByteInstruction)
    {
      int v = __collector_VM_ReadByteInstruction (p);
      if (v != VM_NOT_VM_MEMORY)
	return v;
    }
  return *p;
}

struct DataHandle *dhndl = NULL;

static unsigned unwind_key = COLLECTOR_TSD_INVALID_KEY;

/* To support two OpenMP API's we use a pointer
 * to the actual function.
 */
int (*__collector_omp_stack_trace)(char*, int, hrtime_t, void*) = NULL;
int (*__collector_mpi_stack_trace)(char*, int, hrtime_t) = NULL;

#define DEFAULT_MAX_NFRAMES 256
static int max_native_nframes = DEFAULT_MAX_NFRAMES;
static int max_java_nframes = DEFAULT_MAX_NFRAMES;

#define NATIVE_FRAME_BYTES(nframes) ( ((nframes)+1) * sizeof(long)          )
#define JAVA_FRAME_BYTES(nframes)   ( ((nframes)+1) * sizeof(long) * 2 + 16 )
#define OVERHEAD_BYTES ( 2 * sizeof(long) + 2 * sizeof(Stack_info) )

#define ROOT_UID	801425552975190205ULL
#define ROOT_UID_INV	92251691606677ULL
#define ROOT_IDX	13907816567264074199ULL
#define ROOT_IDX_INV	2075111ULL
#define	UIDTableSize	1048576
static volatile uint64_t *UIDTable = NULL;
static volatile int seen_omp = 0;

static int stack_unwind (char *buf, int size, void *bptr, void *eptr, ucontext_t *context, int mode);
static FrameInfo compute_uid (Frame_packet *frp);
static int omp_no_walk = 0;

#if ARCH(Intel)
#define ValTableSize    1048576
#define OmpValTableSize 65536
static unsigned long *AddrTable_RA_FROMFP = NULL; // Cache for RA_FROMFP pcs
static unsigned long *AddrTable_RA_EOSTCK = NULL; // Cache for RA_EOSTCK pcs
static struct WalkContext *OmpCurCtxs = NULL;
static struct WalkContext *OmpCtxs = NULL;
static uint32_t *OmpVals = NULL;
static unsigned long *OmpRAs = NULL;
static unsigned long adjust_ret_addr (unsigned long ra, unsigned long segoff, unsigned long tend);
static int parse_x86_AVX_instruction (unsigned char *pc);

struct WalkContext
{
  unsigned long pc;
  unsigned long sp;
  unsigned long fp;
  unsigned long ln;
  unsigned long sbase; /* stack boundary */
  unsigned long tbgn;  /* current memory segment start */
  unsigned long tend;  /* current memory segment end */
};
#endif

#if defined(DEBUG) && ARCH(Intel)
#include <execinfo.h>

static void
dump_stack (int nline)
{
  if ((__collector_tracelevel & SP_DUMP_STACK) == 0)
    return;

  enum Constexpr { MAX_SIZE = 1024 };
  void *array[MAX_SIZE];
  size_t sz = backtrace (array, MAX_SIZE);
  char **strings = backtrace_symbols (array, sz);
  DprintfT (SP_DUMP_STACK, "\ndump_stack: %d size=%d\n", nline, (int) sz);
  for (int i = 0; i < sz; i++)
    DprintfT (SP_DUMP_STACK, "  %3d:  %p %s\n", i, array[i],
	     strings[i] ? strings[i] : "???");
}

#define dump_targets(nline, ntrg, targets) \
    if ((__collector_tracelevel & SP_DUMP_UNWIND) != 0) \
	for(int i = 0; i < ntrg; i++) \
	     DprintfT (SP_DUMP_UNWIND, "  %2d: 0x%lx\n", i, (long) targets[i])
#else
#define dump_stack(x)
#define dump_targets(nline, ntrg, targets)
#endif

void
__collector_ext_unwind_key_init (int isPthread, void * stack)
{
  void * ptr = __collector_tsd_get_by_key (unwind_key);
  if (ptr == NULL)
    {
      TprintfT (DBG_LT2, "__collector_ext_unwind_key_init: cannot get tsd\n");
      return;
    }
  if (isPthread)
    {
      size_t stack_size = 0;
      void *stack_addr = 0;
      pthread_t pthread = pthread_self ();
      pthread_attr_t attr;
      int err = pthread_getattr_np (pthread, &attr);
      TprintfT (DBG_LT1, "__collector_ext_unwind_key_init: pthread: 0x%lx err: %d\n", pthread, err);
      if (err == 0)
	{
	  err = pthread_attr_getstack (&attr, &stack_addr, &stack_size);
	  if (err == 0)
	    stack_addr = (char*) stack_addr + stack_size;
	  TprintfT (DBG_LT1, "__collector_ext_unwind_key_init: stack_size=0x%lx eos=%p err=%d\n",
		    (long) stack_size, stack_addr, err);
	  err = pthread_attr_destroy (&attr);
	  TprintfT (DBG_LT1, "__collector_ext_unwind_key_init: destroy: %d\n", err);
	}
      *(void**) ptr = stack_addr;
    }
  else
    *(void**) ptr = stack;  // cloned thread
}

void
__collector_ext_unwind_init (int record)
{
  int sz = UIDTableSize * sizeof (*UIDTable);
  UIDTable = (uint64_t*) __collector_allocCSize (__collector_heap, sz, 1);
  if (UIDTable == NULL)
    {
      __collector_terminate_expt ();
      return;
    }
  CALL_UTIL (memset)((void*) UIDTable, 0, sz);

  char *str = CALL_UTIL (getenv)("GPROFNG_JAVA_MAX_CALL_STACK_DEPTH");
  if (str != NULL && *str != 0)
    {
      char *endptr;
      int n = CALL_UTIL (strtol)(str, &endptr, 0);
      if (endptr != str && n >= 0)
	{
	  if (n < 5)
	    n = 5;
	  if (n > MAX_STACKDEPTH)
	    n = MAX_STACKDEPTH;
	  max_java_nframes = n;
	}
    }

  str = CALL_UTIL (getenv)("GPROFNG_MAX_CALL_STACK_DEPTH");
  if (str != NULL && *str != 0)
    {
      char *endptr = str;
      int n = CALL_UTIL (strtol)(str, &endptr, 0);
      if (endptr != str && n >= 0)
	{
	  if (n < 5)
	    n = 5;
	  if (n > MAX_STACKDEPTH)
	    n = MAX_STACKDEPTH;
	  max_native_nframes = n;
	}
    }

  TprintfT (DBG_LT0, "GPROFNG_MAX_CALL_STACK_DEPTH=%d  GPROFNG_JAVA_MAX_CALL_STACK_DEPTH=%d\n",
	    max_native_nframes, max_java_nframes);
  omp_no_walk = 1;

  if (__collector_VM_ReadByteInstruction == NULL)
    __collector_VM_ReadByteInstruction = (int(*)()) dlsym (RTLD_DEFAULT, "Async_VM_ReadByteInstruction");

#if ARCH(SPARC)
#if WSIZE(64)
  misalign_hdl = (unsigned long) dlsym (RTLD_DEFAULT, "__misalign_trap_handler");
  misalign_hdlx = (unsigned long) dlsym (RTLD_DEFAULT, "__misalign_trap_handler_end");
  if (misalign_hdlx == 0)
    misalign_hdlx = misalign_hdl + 292;
  barrier_hdl = (unsigned long) dlsym (RTLD_DEFAULT, "__mt_EndOfTask_Barrier_");
  barrier_hdlx = (unsigned long) dlsym (RTLD_DEFAULT, "__mt_EndOfTask_Barrier_Dummy_");
  if (barrier_hdlx == 0)
    barrier_hdl = 0;
#else
  barrier_hdl = (unsigned long) dlsym (RTLD_DEFAULT, "__mt_EndOfTask_Barrier_");
  barrier_hdlx = (unsigned long) dlsym (RTLD_DEFAULT, "__mt_EndOfTask_Barrier_Dummy_");
  if (barrier_hdlx == 0)
    barrier_hdl = 0;
#endif /* WSIZE() */

#elif ARCH(Intel)
  sz = ValTableSize * sizeof (*AddrTable_RA_FROMFP);
  AddrTable_RA_FROMFP = (unsigned long*) __collector_allocCSize (__collector_heap, sz, 1);
  sz = ValTableSize * sizeof (*AddrTable_RA_EOSTCK);
  AddrTable_RA_EOSTCK = (unsigned long*) __collector_allocCSize (__collector_heap, sz, 1);
  if (omp_no_walk && (__collector_omp_stack_trace != NULL || __collector_mpi_stack_trace != NULL))
    {
      sz = OmpValTableSize * sizeof (*OmpCurCtxs);
      OmpCurCtxs = (struct WalkContext *) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpCtxs);
      OmpCtxs = (struct WalkContext *) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpVals);
      OmpVals = (uint32_t*) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpRAs);
      OmpRAs = (unsigned long*) __collector_allocCSize (__collector_heap, sz, 1);
      if (OmpCurCtxs == NULL || OmpCtxs == NULL || OmpVals == NULL || OmpRAs == NULL)
	{
	  TprintfT (0, "unwind_init() ERROR: failed; terminating experiment\n");
	  __collector_terminate_expt ();
	  return;
	}
    }
#endif /* ARCH() */

  if (record)
    {
      dhndl = __collector_create_handle (SP_FRINFO_FILE);
      __collector_log_write ("<%s name=\"%s\" format=\"binary\"/>\n", SP_TAG_DATAPTR, SP_FRINFO_FILE);
    }

  unwind_key = __collector_tsd_create_key (sizeof (void*), NULL, NULL);
  if (unwind_key == COLLECTOR_TSD_INVALID_KEY)
    {
      TprintfT (0, "unwind_init: ERROR: TSD key create failed.\n");
      __collector_log_write ("<%s kind=\"%s\" id=\"%d\">TSD key not created</%s>\n",
			     SP_TAG_EVENT, SP_JCMD_CERROR, COL_ERROR_GENERAL, SP_TAG_EVENT);
      return;
    }
  TprintfT (0, "unwind_init() completed normally\n");
  return;
}

void
__collector_ext_unwind_close ()
{
  __collector_delete_handle (dhndl);
  dhndl = NULL;
}

void*
__collector_ext_return_address (unsigned level)
{
  if (NULL == UIDTable)  //unwind not initialized yet
    return NULL;
  unsigned size = (level + 4) * sizeof (long); // need to strip __collector_get_return_address and its caller
  ucontext_t context;
  FILL_CONTEXT ((&context));
  char* buf = (char*) alloca (size);
  if (buf == NULL)
    {
      TprintfT (DBG_LT0, "__collector_get_return_address: ERROR: alloca(%d) fails\n", size);
      return NULL;
    }
  int sz = stack_unwind (buf, size, NULL, NULL, &context, 0);
  if (sz < (level + 3) * sizeof (long))
    {
      TprintfT (DBG_LT0, "__collector_get_return_address: size=%d, but stack_unwind returns %d\n", size, sz);
      return NULL;
    }
  long *lbuf = (long*) buf;
  TprintfT (DBG_LT2, "__collector_get_return_address: return %lx\n", lbuf[level + 2]);
  return (void *) (lbuf[level + 2]);
}
/*
 *  Collector interface method getFrameInfo
 */
FrameInfo
__collector_get_frame_info (hrtime_t ts, int mode, void *arg)
{
  ucontext_t *context = NULL;
  void *bptr = NULL;
  CM_Array *array = NULL;

  int unwind_mode = 0;
  int do_walk = 1;

  if (mode & FRINFO_NO_WALK)
    do_walk = 0;
  int bmode = mode & 0xffff;
  int pseudo_context = 0;
  if (bmode == FRINFO_FROM_STACK_ARG || bmode == FRINFO_FROM_STACK)
    {
      bptr = arg;
      context = (ucontext_t*) alloca (sizeof (ucontext_t));
      FILL_CONTEXT (context);
      unwind_mode |= bmode;
    }
  else if (bmode == FRINFO_FROM_UC)
    {
      context = (ucontext_t*) arg;
      if (context == NULL)
	return (FrameInfo) 0;
      if (GET_SP (context) == 0)
	pseudo_context = 1;
    }
  else if (bmode == FRINFO_FROM_ARRAY)
    {
      array = (CM_Array*) arg;
      if (array == NULL || array->length <= 0)
	return (FrameInfo) 0;
    }
  else
    return (FrameInfo) 0;

  int max_frame_size = OVERHEAD_BYTES + NATIVE_FRAME_BYTES (max_native_nframes);
  if (__collector_java_mode && __collector_java_asyncgetcalltrace_loaded && context && !pseudo_context)
    max_frame_size += JAVA_FRAME_BYTES (max_java_nframes);

  Frame_packet *frpckt = alloca (sizeof (Frame_packet) + max_frame_size);
  frpckt->type = FRAME_PCKT;
  frpckt->hsize = sizeof (Frame_packet);

  char *d = (char*) (frpckt + 1);
  int size = max_frame_size;

#define MIN(a,b) ((a)<(b)?(a):(b))
#if defined(GPROFNG_JAVA_PROFILING)
  /* get Java info */
  if (__collector_java_mode && __collector_java_asyncgetcalltrace_loaded && context && !pseudo_context)
    {
      /* use only 2/3 of the buffer and leave the rest for the native stack */
      int tmpsz = MIN (size, JAVA_FRAME_BYTES (max_java_nframes));
      if (tmpsz > 0)
	{
	  int sz = __collector_ext_jstack_unwind (d, tmpsz, context);
	  d += sz;
	  size -= sz;
	}
    }
#endif

  /* get native stack */
  if (context)
    {
      Stack_info *sinfo = (Stack_info*) d;
      int sz = sizeof (Stack_info);
      d += sz;
      size -= sz;
#if ARCH(Intel)
      if (omp_no_walk == 0)
	do_walk = 1;
#endif
      if (do_walk == 0)
	unwind_mode |= FRINFO_NO_WALK;

      int tmpsz = MIN (size, NATIVE_FRAME_BYTES (max_native_nframes));
      if (tmpsz > 0)
	{
	  sz = stack_unwind (d, tmpsz, bptr, NULL, context, unwind_mode);
	  d += sz;
	  size -= sz;
	}
      sinfo->kind = STACK_INFO;
      sinfo->hsize = (d - (char*) sinfo);
    }

  /* create a stack image from user data */
  if (array && array->length > 0)
    {
      Stack_info *sinfo = (Stack_info*) d;
      int sz = sizeof (Stack_info);
      d += sz;
      size -= sz;
      sz = array->length;
      if (sz > size)
	sz = size;  // YXXX should we mark this with truncation frame?
      __collector_memcpy (d, array->bytes, sz);
      d += sz;
      size -= sz;
      sinfo->kind = STACK_INFO;
      sinfo->hsize = (d - (char*) sinfo);
    }

  /* Compute the total size */
  frpckt->tsize = d - (char*) frpckt;
  FrameInfo uid = compute_uid (frpckt);
  return uid;
}

FrameInfo
compute_uid (Frame_packet *frp)
{
  uint64_t idxs[LAST_INFO];
  uint64_t uid = ROOT_UID;
  uint64_t idx = ROOT_IDX;

  Common_info *cinfo = (Common_info*) ((char*) frp + frp->hsize);
  char *end = (char*) frp + frp->tsize;
  for (;;)
    {
      if ((char*) cinfo >= end || cinfo->hsize == 0 ||
	  (char*) cinfo + cinfo->hsize > end)
	break;

      /* Start with a different value to avoid matching with uid */
      uint64_t uidt = 1;
      uint64_t idxt = 1;
      long *ptr = (long*) ((char*) cinfo + cinfo->hsize);
      long *bnd = (long*) ((char*) cinfo + sizeof (Common_info));
      TprintfT (DBG_LT2, "compute_uid: Cnt=%ld: ", (long) cinfo->hsize);
      while (ptr > bnd)
	{
	  long val = *(--ptr);
	  tprintf (DBG_LT2, "0x%8.8llx ", (unsigned long long) val);
	  uidt = (uidt + val) * ROOT_UID;
	  idxt = (idxt + val) * ROOT_IDX;
	  uid = (uid + val) * ROOT_UID;
	  idx = (idx + val) * ROOT_IDX;
	}
      if (cinfo->kind == STACK_INFO || cinfo->kind == JAVA_INFO)
	{
	  cinfo->uid = uidt;
	  idxs[cinfo->kind] = idxt;
	}
      cinfo = (Common_info*) ((char*) cinfo + cinfo->hsize);
    }
  tprintf (DBG_LT2, "\n");

  /* Check if we have already recorded that uid.
   * The following fragment contains benign data races.
   * It's important, though, that all reads from UIDTable
   * happen before writes.
   */
  int found1 = 0;
  int idx1 = (int) ((idx >> 44) % UIDTableSize);
  if (UIDTable[idx1] == uid)
    found1 = 1;
  int found2 = 0;
  int idx2 = (int) ((idx >> 24) % UIDTableSize);
  if (UIDTable[idx2] == uid)
    found2 = 1;
  int found3 = 0;
  int idx3 = (int) ((idx >> 4) % UIDTableSize);
  if (UIDTable[idx3] == uid)
    found3 = 1;
  if (!found1)
    UIDTable[idx1] = uid;
  if (!found2)
    UIDTable[idx2] = uid;
  if (!found3)
    UIDTable[idx3] = uid;

  if (found1 || found2 || found3)
    return (FrameInfo) uid;
  frp->uid = uid;

  /* Compress info's */
  cinfo = (Common_info*) ((char*) frp + frp->hsize);
  for (;;)
    {
      if ((char*) cinfo >= end || cinfo->hsize == 0 ||
	  (char*) cinfo + cinfo->hsize > end)
	break;
      if (cinfo->kind == STACK_INFO || cinfo->kind == JAVA_INFO)
	{
	  long *ptr = (long*) ((char*) cinfo + sizeof (Common_info));
	  long *bnd = (long*) ((char*) cinfo + cinfo->hsize);
	  uint64_t uidt = cinfo->uid;
	  uint64_t idxt = idxs[cinfo->kind];
	  int found = 0;
	  int first = 1;
	  while (ptr < bnd - 1)
	    {
	      int idx1 = (int) ((idxt >> 44) % UIDTableSize);
	      if (UIDTable[idx1] == uidt)
		{
		  found = 1;
		  break;
		}
	      else if (first)
		{
		  first = 0;
		  UIDTable[idx1] = uidt;
		}
	      long val = *ptr++;
	      uidt = uidt * ROOT_UID_INV - val;
	      idxt = idxt * ROOT_IDX_INV - val;
	    }
	  if (found)
	    {
	      char *d = (char*) ptr;
	      char *s = (char*) bnd;
	      if (!first)
		{
		  int i;
		  for (i = 0; i<sizeof (uidt); i++)
		    {
		      *d++ = (char) uidt;
		      uidt = uidt >> 8;
		    }
		}
	      int delta = s - d;
	      while (s < end)
		*d++ = *s++;
	      cinfo->kind |= COMPRESSED_INFO;
	      cinfo->hsize -= delta;
	      frp->tsize -= delta;
	      end -= delta;
	    }
	}
      cinfo = (Common_info*) ((char*) cinfo + cinfo->hsize);
    }
  __collector_write_packet (dhndl, (CM_Packet*) frp);
  return (FrameInfo) uid;
}

FrameInfo
__collector_getUID (CM_Array *arg, FrameInfo suid)
{
  if (arg->length % sizeof (long) != 0 ||
      (long) arg->bytes % sizeof (long) != 0)
    return (FrameInfo) - 1;
  if (arg->length == 0)
    return suid;

  uint64_t uid = suid ? suid : 1;
  uint64_t idx = suid ? suid : 1;
  long *ptr = (long*) ((char*) arg->bytes + arg->length);
  long *bnd = (long*) (arg->bytes);
  while (ptr > bnd)
    {
      long val = *(--ptr);
      uid = (uid + val) * ROOT_UID;
      idx = (idx + val) * ROOT_IDX;
    }

  /* Check if we have already recorded that uid.
   * The following fragment contains benign data races.
   * It's important, though, that all reads from UIDTable
   * happen before writes.
   */
  int found1 = 0;
  int idx1 = (int) ((idx >> 44) % UIDTableSize);
  if (UIDTable[idx1] == uid)
    found1 = 1;
  int found2 = 0;
  int idx2 = (int) ((idx >> 24) % UIDTableSize);
  if (UIDTable[idx2] == uid)
    found2 = 1;
  int found3 = 0;
  int idx3 = (int) ((idx >> 4) % UIDTableSize);
  if (UIDTable[idx3] == uid)
    found3 = 1;

  if (!found1)
    UIDTable[idx1] = uid;
  if (!found2)
    UIDTable[idx2] = uid;
  if (!found3)
    UIDTable[idx3] = uid;
  if (found1 || found2 || found3)
    return (FrameInfo) uid;

  int sz = sizeof (Uid_packet) + arg->length;
  if (suid)
    sz += sizeof (suid);
  Uid_packet *uidp = alloca (sz);
  uidp->tsize = sz;
  uidp->type = UID_PCKT;
  uidp->flags = 0;
  uidp->uid = uid;

  /* Compress */
  ptr = (long*) (arg->bytes);
  bnd = (long*) ((char*) arg->bytes + arg->length);
  long *dst = (long*) (uidp + 1);
  uint64_t uidt = uid;
  uint64_t idxt = idx;
  uint64_t luid = suid; /* link uid */

  while (ptr < bnd)
    {

      long val = *ptr++;
      *dst++ = val;

      if ((bnd - ptr) > sizeof (uidt))
	{
	  uidt = uidt * ROOT_UID_INV - val;
	  idxt = idxt * ROOT_IDX_INV - val;
	  int idx1 = (int) ((idxt >> 44) % UIDTableSize);
	  if (UIDTable[idx1] == uidt)
	    {
	      luid = uidt;
	      break;
	    }
	}
    }
  if (luid)
    {
      char *d = (char*) dst;
      for (int i = 0; i<sizeof (luid); i++)
	{
	  *d++ = (char) luid;
	  luid = luid >> 8;
	}
      uidp->flags |= COMPRESSED_INFO;
      uidp->tsize = d - (char*) uidp;
    }
  __collector_write_packet (dhndl, (CM_Packet*) uidp);

  return (FrameInfo) uid;
}

int
__collector_getStackTrace (void *buf, int size, void *bptr, void *eptr, void *arg)
{
  if (arg == (void*) __collector_omp_stack_trace)
    seen_omp = 1;
  int do_walk = 1;
  if (arg == NULL || arg == (void*) __collector_omp_stack_trace)
    {
      do_walk = (arg == (void*) __collector_omp_stack_trace && omp_no_walk) ? 0 : 1;
      ucontext_t *context = (ucontext_t*) alloca (sizeof (ucontext_t));
      FILL_CONTEXT (context);
      arg = context;
    }
  int unwind_mode = 0;
  if (do_walk == 0)
    unwind_mode |= FRINFO_NO_WALK;
  return stack_unwind (buf, size, bptr, eptr, arg, unwind_mode);
}

#if ARCH(SPARC)
/*
 * These are important data structures taken from the header files reg.h and
 * ucontext.h. They are used for the stack trace algorithm explained below.
 *
 *	typedef struct ucontext {
 * 		u_long		uc_flags;
 * 		struct ucontext	*uc_link;
 * 		usigset_t   	uc_sigmask;
 * 		stack_t 	uc_stack;
 * 		mcontext_t 	uc_mcontext;
 * 		long		uc_filler[23];
 * 	} ucontext_t;
 *
 *	#define	SPARC_MAXREGWINDOW	31
 *
 *	struct	rwindow {
 *		greg_t	rw_local[8];
 *		greg_t	rw_in[8];
 *	};
 *
 *	#define	rw_fp	rw_in[6]
 *	#define	rw_rtn	rw_in[7]
 *
 *	struct gwindows {
 *		int		wbcnt;
 *		int		*spbuf[SPARC_MAXREGWINDOW];
 *		struct rwindow	wbuf[SPARC_MAXREGWINDOW];
 *	};
 *
 *	typedef struct gwindows	gwindows_t;
 *
 *	typedef struct {
 *		gregset_t	gregs;
 *		gwindows_t	*gwins;
 *		fpregset_t	fpregs;
 *		long		filler[21];
 *	} mcontext_t;
 *
 * The stack would look like this when SIGPROF occurrs.
 *
 *	------------------------- <- high memory
 *	|			|
 *	|			|
 *	-------------------------
 *	|			|
 *	------------------------- <- fp' <-|
 *	|			|	   |
 *		:	:	 	   |
 *	|			|	   |
 *	-------------------------	   |
 *	|	fp		|----------|
 *	|			|
 *	------------------------- <- sp'
 *	|			|		             |	|
 *	| 	gwins		| <- saved stack pointers &  |  |
 *	|			|    register windows	     |  |- mcontext
 *	-------------------------			     |  |
 *	|	gregs		| <- saved registers	     |  |
 *	-------------------------			     |
 *	|			|			     |- ucontext
 *	------------------------- <- ucp (ucontext pointer)  |
 *	|			|				|
 *	|			|				|- siginfo
 *	------------------------- <- sip (siginfo pointer)	|
 *	|			|
 *	------------------------- <- sp
 *
 * Then the signal handler is called with:
 *	handler( signo, sip, uip );
 * When gwins is null, all the stack frames are saved in the user stack.
 * In that case we can find sp' from gregs and walk the stack for a backtrace.
 * However, if gwins is not null we will have a more complicated case.
 * Wbcnt(in gwins) tells you how many saved register windows are valid.
 * This is important because the kernel does not allocate the entire array.
 * And the top most frame is saved in the lowest index element. The next
 * paragraph explains the possible causes.
 *
 * There are two routines in the kernel to flush out user register windows.
 *	flush_user_windows and flush_user_windows_to_stack
 * The first routine will not cause a page fault. Therefore if the user
 * stack is not in memory, the register windows will be saved to the pcb.
 * This can happen when the kernel is trying to deliver a signal and
 * the user stack got swap out. The kernel will then build a new context for
 * the signal handler and the saved register windows will
 * be copied to the ucontext as show above. On the other hand,
 * flush_user_windows_to_stack can cause a page fault, and if it failed
 * then there is something wrong (stack overflow, misalign).
 * The first saved register window does not necessary correspond to the
 * first stack frame. So the current stack pointer must be compare with
 * the stack pointers in spbuf to find a match.
 *
 * We will also follow the uc_link field in ucontext to trace also nested
 * signal stack frames.
 *
 */

/* Dealing with trap handlers.
 * When a user defined trap handler is invoked the return address
 * (or actually the address of an instruction that raised the trap)
 * is passed to the trap handler in %l6, whereas saved %o7 contains
 * garbage. First, we need to find out if a particular pc belongs
 * to the trap handler, and if so, take the %l6 value from the stack rather
 * than %o7 from either the stack or the register.
 * There are three possible situations represented
 * by the following stacks:
 *
 *   MARKER		MARKER			MARKER
 *   trap handler pc	__func pc before 'save'	__func pc after 'save'
 *   %l6		%o7 from reg		%o7 (garbage)
 *   ...		%l6			trap handler pc
 *			...			%l6
 *						...
 * where __func is a function called from the trap handler.
 *
 * Currently this is implemented to only deal with __misalign_trap_handler
 * set for v9 FORTRAN applications. Implementation of IN_TRAP_HANDLER
 * macro shows it. A general solution is postponed.
 */

/* Special handling of unwind through the parallel loop barrier code:
 *
 *  The library defines two symbols, __mt_EndOfTask_Barrier_ and
 *	__mt_EndOfTask_Barrier_Dummy_ representing the first word of
 *	the barrier sychronization code, and the first word following
 *	it.  Whenever the leaf PC is between these two symbols,
 *	the unwind code is special-cased as follows:
 *	The __mt_EndOfTask_Barrier_ function is guaranteed to be a leaf
 *	function, so its return address is in a register, not saved on
 *	the stack.
 *
 *    MARKER
 *    __mt_EndOfTask_Barrier_ PC -- the leaf PC
 *    loop body function address for the task -- implied caller of __mt_EndOfTask_Barrier_
 *	    this address is taken from the %O0 register
 *    {mt_master or mt_slave} -- real caller of __mt_EndOfTask_Barrier_
 *     ...
 *
 *  With this trick, the analyzer will show the time in the barrier
 *	attributed to the loop at the end of which the barrier synchronization
 *	is taking place.  That loop body routine, will be shown as called
 *	from the function from which it was extracted, which will be shown
 *	as called from the real caller, either the slave or master library routine.
 */

/*
 * These no-fault-load (0x82) assembly functions are courtesy of Rob Gardner.
 *
 * Note that 0x82 is ASI_PNF.  See
 *   http://lxr.free-electrons.com/source/arch/sparc/include/uapi/asm/asi.h#L134
 *   ASI  address space identifier; PNF  primary no fault
 */

/* load an int from an address */

/* if the address is illegal, return a 0 */
static int
SPARC_no_fault_load_int (void *addr)
{
  int val;
  __asm__ __volatile__(
		       "lda [%1] 0x82, %0\n\t"
		       : "=r" (val)
		       : "r" (addr)
		       );

  return val;
}

/* check if an address is invalid
 *
 * A no-fault load of an illegal address still faults, but it does so silently to the calling process.
 * It returns a 0, but so could a load of a legal address.
 * So, we time the load.  A "fast" load must be a successful load.
 * A "slow" load is probably a fault.
 * Since it could also be a cache/TLB miss or other abnormality,
 * it's safest to retry a slow load.
 * The cost of trying a valid address should be some nanosecs.
 * The cost of trying an invalid address up to 10 times could be some microsecs.
 */
#if 0
static
int invalid_SPARC_addr(void *addr)
{
    long t1, t2;
    int i;

    for (i=0; i<10; i++) {
      __asm__ __volatile__(
	"rd %%tick, %0\n\t"
	"lduba [%2] 0x82, %%g0\n\t"
	"rd %%tick, %1\n\t"
	: "=r" (t1), "=r" (t2)
	: "r" (addr) );
      if ( (t2 - t1) < 100 )
	return 0;
    }
    return 1;
}
#endif

/*
 * The standard SPARC procedure-calling convention is that the
 * calling PC (for determining the return address when the procedure
 * is finished) is placed in register %o7.  A called procedure
 * typically executes a "save" instruction that shifts the register
 * window, and %o7 becomes %i7.
 *
 * Optimized leaf procedures do not shift the register window.
 * They assume the return address will remain %o7.  So when
 * we process a leaf PC, we walk instructions to see if there
 * is a call, restore, or other instruction that would indicate
 * we can IGNORE %o7 because this is NOT a leaf procedure.
 *
 * If a limited instruction walk uncovers no such hint, we save
 * not only the PC but the %o7 value as well... just to be safe.
 * Later, in DBE post-processing of the call stacks, we decide
 * whether any recorded %o7 value should be used as a caller
 * frame or should be discarded.
 */

#define IS_ILLTRAP(x) (((x) & 0xc1c00000) == 0)
#define IS_SAVE(x)    (((x) & 0xc1f80000) == 0x81e00000)
#define IS_MOVO7R(x)  (((x) & 0xc1f8201f) == 0x8160000f)
#define IS_MOVRO7(x)  (((x) & 0xfff82000) == 0x9f600000)
#define IS_ORRG0O7(x) (((x) & 0xff78201f) == 0x9e100000)
#define IS_ORG0RO7(x) (((x) & 0xff7fe000) == 0x9e100000)
#define IS_ORG0O7R(x) (((x) & 0xc17fe01f) == 0x8010000f)
#define IS_ORO7G0R(x) (((x) & 0xc17fe01f) == 0x8013c000)
#define IS_RESTORE(x) (((x) & 0xc1f80000) == 0x81e80000)
#define IS_RET(x)     ((x) == 0x81c7e008)
#define IS_RETL(x)    ((x) == 0x81c3e008)
#define IS_RETURN(x)  (((x) & 0xc1f80000) == 0x81c80000)
#define IS_BRANCH(x)  ((((x) & 0xc0000000) == 0) && (((x) & 0x01c00000) != 0x01000000))
#define IS_CALL(x)    (((x) & 0xc0000000) == 0x40000000)
#define IS_LDO7(x)    (((x) & 0xfff80000) == 0xde000000)

static long pagesize = 0;

static int
process_leaf (long *lbuf, int ind, int lsize, void *context)
{
  greg_t pc = GET_PC (context);
  greg_t o7 = GET_GREG (context, REG_O7);

  /* omazur: TBR START -- not used */
  if (IN_BARRIER (pc))
    {
      if (ind < lsize)
	lbuf[ind++] = pc;
      if (ind < lsize)
	lbuf[ind++] = GET_GREG (context, REG_O0);
      return ind;
    }
  /* omazur: TBR END */
#if WSIZE(64)
  if (IN_TRAP_HANDLER (pc))
    {
      if (ind < lsize)
	lbuf[ind++] = pc;
      return ind;
    }
#endif
  unsigned *instrp = (unsigned *) pc;
  unsigned *end_addr = instrp + 20;
  while (instrp < end_addr)
    {
      unsigned instr = *instrp++;
      if (IS_ILLTRAP (instr))
	break;
      else if (IS_SAVE (instr))
	{
	  if (ind < lsize)
	    lbuf[ind++] = pc;
	  if (o7 && ind < lsize)
	    lbuf[ind++] = o7;
	  return ind;
	}
      else if (IS_MOVO7R (instr) || IS_ORG0O7R (instr) || IS_ORO7G0R (instr))
	break;
      else if (IS_MOVRO7 (instr) || IS_ORG0RO7 (instr))
	{
	  int rs2 = (instr & 0x1f) + REG_G1 - 1;
	  o7 = (rs2 <= REG_O7) ? GET_GREG (context, rs2) : 0;
	  break;
	}
      else if (IS_ORRG0O7 (instr))
	{
	  int rs2 = ((instr & 0x7c000) >> 14) + REG_G1 - 1;
	  o7 = (rs2 <= REG_O7) ? GET_GREG (context, rs2) : 0;
	  break;
	}
      else if (IS_RESTORE (instr))
	{
	  o7 = 0;
	  break;
	}
      else if (IS_RETURN (instr))
	{
	  o7 = 0;
	  break;
	}
      else if (IS_RET (instr))
	{
	  o7 = 0;
	  break;
	}
      else if (IS_RETL (instr))
	{
	  /* process delay slot */
	  instr = *instrp++;
	  if (IS_RESTORE (instr))
	    o7 = 0;
	  break;
	}
      else if (IS_BRANCH (instr))
	{
	  unsigned *backbegin = ((unsigned *) pc - 1);
	  unsigned *backend = backbegin - 12 + (instrp - (unsigned *) pc);
	  while (backbegin > backend)
	    {
	      // 21920143 stack unwind: SPARC process_leaf backtracks too far
	      /*
	       * We've already dereferenced backbegin+1.
	       * So if backbegin is on the same page, we're fine.
	       * If we've gone to a different page, possibly things are not fine.
	       * We don't really know how to test that.
	       * Let's just assume the worst:  that dereferencing backbegin would segv.
	       * We won't know if we're in a leaf function or not.
	       */
	      if (pagesize == 0)
		pagesize = CALL_UTIL (sysconf)(_SC_PAGESIZE);
	      if ((((long) (backbegin + 1)) & (pagesize - 1)) < sizeof (unsigned*))
		break;
	      unsigned backinstr = *backbegin--;
	      if (IS_LDO7 (backinstr))
		{
		  o7 = 0;
		  break;
		}
	      else if (IS_ILLTRAP (backinstr))
		break;
	      else if (IS_RETURN (backinstr))
		break;
	      else if (IS_RET (backinstr))
		break;
	      else if (IS_RETL (backinstr))
		break;
	      else if (IS_CALL (backinstr))
		break;
	      else if (IS_SAVE (backinstr))
		{
		  o7 = 0;
		  break;
		}
	    }
	  break;
	}
      else if (IS_CALL (instr))
	o7 = 0;
    }

#if WSIZE(64)
  if (o7 != 0 && ((long) o7) < 32 && ((long) o7) > -32)
    {
      /* 20924821 SEGV in unwind code on SPARC/Linux
       * We've seen this condition in some SPARC-Linux runs.
       * o7 is non-zero but not a valid address.
       * Values like 4 or -7 have been seen.
       * Let's check if o7 is unreasonably small.
       * If so, set to 0 so that it won't be recorded.
       * Otherwise, there is risk of it being dereferenced in process_sigreturn().
       */
      // __collector_log_write("<event kind=\"%s\" id=\"%d\">time %lld, internal debug unwind at leaf; o7 = %ld, pc = %x</event>\n",
      //       SP_JCMD_COMMENT, COL_COMMENT_NONE, __collector_gethrtime() - __collector_start_time, (long) o7, pc );
      o7 = 0;
    }
#endif

  if (o7)
    {
      if (ind < lsize)
	lbuf[ind++] = SP_LEAF_CHECK_MARKER;
      if (ind < lsize)
	lbuf[ind++] = pc;
      if (ind < lsize)
	lbuf[ind++] = o7;
    }
  else if (ind < lsize)
    lbuf[ind++] = pc;
  return ind;
}

#if WSIZE(64)
// detect signal handler
static int
process_sigreturn (long *lbuf, int ind, int lsize, unsigned char * tpc,
		   struct frame **pfp, void * bptr, int extra_frame)
{
  // cheap checks whether tpc is obviously not an instruction address
  if ((4096 > (unsigned long) tpc) // the first page is off limits
      || (3 & (unsigned long) tpc))
    return ind;  // the address is not aligned

  // get the instruction at tpc, skipping over as many as 7 nop's (0x01000000)
  int insn, i;
  for (i = 0; i < 7; i++)
    {
      insn = SPARC_no_fault_load_int ((void *) tpc);
      if (insn != 0x01000000)
	break;
      tpc += 4;
    }

  // we're not expecting 0 (and it could mean an illegal address)
  if (insn == 0)
    return ind;

  // We are looking for __rt_sigreturn_stub with the instruction
  //     0x82102065 : mov 0x65 /* __NR_rt_sigreturn */, %g1
  if (insn == 0x82102065)
    {
      /*
       * according to linux kernel source code,
       * syscall(_NR_rt_sigreturn) uses the following data in stack:
       * struct rt_signal_frame {
       *     struct sparc_stackf     ss;
       *     siginfo_t               info;
       *     struct pt_regs          regs;
       *     ....};
       * sizeof(struct sparc_stackf) is 192;
       * sizeof(siginfo_t) is 128;
       * we need to get the register values from regs, which is defined as:
       * struct pt_regs {
       *     unsigned long u_regs[16];
       *     unsigned long tstate;
       *     unsigned long tpc;
       *     unsigned long tnpc;
       *     ....};
       * pc and fp register has offset of 120 and 112;
       * the pc of kill() is stored in tnpc, whose offest is 136.
       */
      greg_t pc = *((unsigned long*) ((char*) ((*pfp)) + 192 + 128 + 136));
      greg_t pc1 = *((unsigned long*) ((char*) ((*pfp)) + 192 + 128 + 120));
      (*pfp) = *((struct frame**) ((char*) ((*pfp)) + 192 + 128 + 112));
      if (pc && pc1)
	{
	  if (bptr != NULL && extra_frame && ((char*) (*pfp) + STACK_BIAS) < (char*) bptr && ind < 2)
	    {
	      lbuf[0] = pc1;
	      if (ind == 0)
		ind++;
	    }
	  if (bptr == NULL || ((char*) (*pfp) + STACK_BIAS) >= (char*) bptr)
	    {
	      if (ind < lsize)
		lbuf[ind++] = (unsigned long) tpc;
	      if (ind < lsize)
		lbuf[ind++] = pc;
	      if (ind < lsize)
		lbuf[ind++] = pc1;
	    }
	}
      DprintfT (SP_DUMP_UNWIND, "unwind.c: resolved sigreturn pc=0x%lx, pc1=0x%lx, fp=0x%lx\n", pc, pc1, *(pfp));
    }
  return ind;
}
#endif

/*
 * int stack_unwind( char *buf, int size, ucontext_t *context )
 *	This routine looks into the mcontext and
 *	trace stack frames to record return addresses.
 */
int
stack_unwind (char *buf, int size, void *bptr, void *eptr, ucontext_t *context, int mode)
{
  /*
   * trace the stack frames from user stack.
   * We are assuming that the frame pointer and return address
   * are null when we are at the top level.
   */
  long *lbuf = (long*) buf;
  int lsize = size / sizeof (long);
  struct frame *fp = (struct frame *) GET_SP (context); /* frame pointer */
  greg_t pc; /* program counter */
  int extra_frame = 0;
  if ((mode & 0xffff) == FRINFO_FROM_STACK)
    extra_frame = 1;

  int ind = 0;
  if (bptr == NULL)
    ind = process_leaf (lbuf, ind, lsize, context);

  int extra_frame = 0;
  if ((mode & 0xffff) == FRINFO_FROM_STACK)
    extra_frame = 1;
  int ind = 0;
  if (bptr == NULL)
    ind = process_leaf (lbuf, ind, lsize, context);

  while (fp)
    {
      if (ind >= lsize)
	break;
      fp = (struct frame *) ((char *) fp + STACK_BIAS);
      if (eptr && fp >= (struct frame *) eptr)
	{
	  ind = ind >= 2 ? ind - 2 : 0;
	  break;
	}
#if WSIZE(64) // detect signal handler
      unsigned char * tpc = ((unsigned char*) (fp->fr_savpc));
      struct frame * tfp = (struct frame*) ((char*) (fp->fr_savfp) + STACK_BIAS);
      int old_ind = ind;
      ind = process_sigreturn (lbuf, old_ind, lsize, tpc, &tfp, bptr, extra_frame);
      if (ind != old_ind)
	{
	  pc = (greg_t) tpc;
	  fp = tfp;
	}
      else
#endif
	{
#if WSIZE(64)
	  if (IN_TRAP_HANDLER (lbuf[ind - 1]))
	    pc = fp->fr_local[6];
	  else
	    pc = fp->fr_savpc;
#else
	  pc = fp->fr_savpc;
#endif
	  fp = fp->fr_savfp;
	  if (pc)
	    {
	      if (bptr != NULL && extra_frame && ((char*) fp + STACK_BIAS) < (char*) bptr && ind < 2)
		{
		  lbuf[0] = pc;
		  if (ind == 0)
		    ind++;
		}
	      if (bptr == NULL || ((char*) fp + STACK_BIAS) >= (char*) bptr)
		lbuf[ind++] = pc;
	    }
	}

      /* 4616238: _door_return may have a frame that has non-zero
       * saved stack pointer and zero pc
       */
      if (pc == (greg_t) NULL)
	break;
    }

  if (ind >= lsize)
    { /* truncated stack handling */
      ind = lsize - 1;
      lbuf[ind++] = SP_TRUNC_STACK_MARKER;
    }
  return ind * sizeof (long);
}

#elif ARCH(Intel)

/* get __NR_<syscall_name> constants */
#include <syscall.h>

/*
 * From uts/intel/ia32/os/sendsig.c:
 *
 * An amd64 signal frame looks like this on the stack:
 *
 * old %rsp:
 *		<128 bytes of untouched stack space>
 *		<a siginfo_t [optional]>
 *		<a ucontext_t>
 *		<siginfo_t *>
 *		<signal number>
 * new %rsp:	<return address (deliberately invalid)>
 *
 * The signal number and siginfo_t pointer are only pushed onto the stack in
 * order to allow stack backtraces.  The actual signal handling code expects the
 * arguments in registers.
 *
 * An i386 SVR4/ABI signal frame looks like this on the stack:
 *
 * old %esp:
 *		<a siginfo32_t [optional]>
 *		<a ucontext32_t>
 *		<pointer to that ucontext32_t>
 *		<pointer to that siginfo32_t>
 *		<signo>
 * new %esp:	<return address (deliberately invalid)>
 */

#if WSIZE(32)
#define OPC_REG(x)      ((x)&0x7)
#define MRM_REGD(x)     (((x)>>3)&0x7)
#define MRM_REGS(x)     ((x)&0x7)
#define RED_ZONE        0
#elif WSIZE(64)
#define OPC_REG(x)      (B|((x)&0x7))
#define MRM_REGD(x)     (R|(((x)>>3)&0x7))
#define MRM_REGS(x)     (B|((x)&0x7))
#define RED_ZONE        16
#endif
#define MRM_EXT(x)      (((x)>>3)&0x7)
#define MRM_MOD(x)      ((x)&0xc0)

#define RAX             0
#define RDX             2
#define RSP             4
#define RBP             5

struct AdvWalkContext
{
  unsigned char *pc;
  unsigned long *sp;
  unsigned long *sp_safe;
  unsigned long *fp;
  unsigned long *fp_sav;
  unsigned long *fp_loc;
  unsigned long rax;
  unsigned long rdx;
  unsigned long ra_sav;
  unsigned long *ra_loc;
  unsigned long regs[16];
  int tidx;         /* targets table index */
  uint32_t cval;    /* cache value */
};

static unsigned long
getRegVal (struct AdvWalkContext *cur, int r, int *undefRez)
{
  if (cur->regs[r] == 0)
    {
      if (r == RBP)
	{
	  tprintf (DBG_LT3, "getRegVal: returns cur->regs[RBP]=0x%lx  cur->pc=0x%lx\n",
		   (unsigned long) cur->fp, (unsigned long) cur->pc);
	  return (unsigned long) cur->fp;
	}
      *undefRez = 1;
    }
  tprintf (DBG_LT3, "getRegVal: cur->regs[%d]=0x%lx  cur->pc=0x%lx\n",
	   r, (unsigned long) cur->regs[r], (unsigned long) cur->pc);
  return cur->regs[r];
}

static unsigned char *
check_modrm (unsigned char *pc)
{
  unsigned char modrm = *pc++;
  unsigned char mod = MRM_MOD (modrm);
  if (mod == 0xc0)
    return pc;
  unsigned char regs = modrm & 0x07;
  if (regs == RSP)
    {
      if (mod == 0x40)
	return pc + 2;  // SIB + disp8
      if (mod == 0x80)
	return pc + 5;  // SIB + disp32
      return pc + 1;    // SIB
    }
  if (mod == 0x0)
    {
      if (regs == RBP)
	pc += 4; // disp32
    }
  else if (mod == 0x40)
    pc += 1; /* byte */
  else if (mod == 0x80)
    pc += 4; /* word */
  return pc;
}

static int
read_int (unsigned char *pc, int w)
{
  if (w == 1)
    return *((char *) pc);
  if (w == 2)
    return *(short*) pc;
  return *(int*) pc;
}

/* Return codes */
enum
{
  RA_FAILURE = 0,
  RA_SUCCESS,
  RA_END_OF_STACK,
  RA_SIGRETURN,
  RA_RT_SIGRETURN
};

/* Cache value encodings */
static const uint32_t RA_FROMFP = (uint32_t) - 1; /* get the RA from the frame pointer */
static const uint32_t RA_EOSTCK = (uint32_t) - 2; /* end-of-stack */


#define MAXCTX         16
#define MAXTRGTS       64
#define MAXJMPREG       2
#define MAXJMPREGCTX    3

#define DELETE_CURCTX()  __collector_memcpy (cur, buf + (--nctx), sizeof (*cur))

/**
 * Look for pc in AddrTable_RA_FROMFP and in AddrTable_RA_EOSTCK
 * @param wctx
 * @return
 */
static int
cache_get (struct WalkContext *wctx)
{
  unsigned long addr;
  if (AddrTable_RA_FROMFP != NULL)
    {
      uint64_t idx = wctx->pc % ValTableSize;
      addr = AddrTable_RA_FROMFP[ idx ];
      if (addr == wctx->pc)
	{ // Found in AddrTable_RA_FROMFP
	  unsigned long *sp = NULL;
	  unsigned long fp = wctx->fp;
	  /* validate fp before use */
	  if (fp < wctx->sp || fp >= wctx->sbase - sizeof (*sp))
	    return RA_FAILURE;
	  sp = (unsigned long *) fp;
	  fp = *sp++;
	  unsigned long ra = *sp++;
	  unsigned long tbgn = wctx->tbgn;
	  unsigned long tend = wctx->tend;
	  if (ra < tbgn || ra >= tend)
	    if (!__collector_check_segment (ra, &tbgn, &tend, 0))
	      return RA_FAILURE;
	  unsigned long npc = adjust_ret_addr (ra, ra - tbgn, tend);
	  if (npc == 0)
	    return RA_FAILURE;
	  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d cached pc=0x%lX\n", __LINE__, npc);
	  wctx->pc = npc;
	  wctx->sp = (unsigned long) sp;
	  wctx->fp = fp;
	  wctx->tbgn = tbgn;
	  wctx->tend = tend;
	  return RA_SUCCESS;
	}
    }
  if (NULL == AddrTable_RA_EOSTCK)
    return RA_FAILURE;
  uint64_t idx = wctx->pc % ValTableSize;
  addr = AddrTable_RA_EOSTCK[ idx ];
  if (addr != wctx->pc)
    return RA_FAILURE;
  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d cached RA_END_OF_STACK\n", __LINE__);
  return RA_END_OF_STACK;
}
/**
 * Save pc in RA_FROMFP or RA_EOSTCK cache depending on val
 * @param wctx
 */
static void
cache_put (struct WalkContext *wctx, const uint32_t val)
{
  if (RA_FROMFP == val)
    {
      // save pc in RA_FROMFP cache
      if (NULL != AddrTable_RA_FROMFP)
	{
	  uint64_t idx = wctx->pc % ValTableSize;
	  AddrTable_RA_FROMFP[ idx ] = wctx->pc;
	  if (NULL != AddrTable_RA_EOSTCK)
	    if (AddrTable_RA_EOSTCK[ idx ] == wctx->pc)
	      // invalidate pc in RA_EOSTCK cache
	      AddrTable_RA_EOSTCK[ idx ] = 0;
	}
      return;
    }
  if (RA_EOSTCK == val)
    {
      // save pc in RA_EOSTCK cache
      if (NULL != AddrTable_RA_EOSTCK)
	{
	  uint64_t idx = wctx->pc % ValTableSize;
	  AddrTable_RA_EOSTCK[ idx ] = wctx->pc;
	  if (NULL != AddrTable_RA_FROMFP)
	    {
	      if (AddrTable_RA_FROMFP[ idx ] == wctx->pc)
		// invalidate pc in RA_FROMFP cache
		AddrTable_RA_FROMFP[ idx ] = 0;
	    }
	}
      return;
    }
}

static int
process_return_real (struct WalkContext *wctx, struct AdvWalkContext *cur, int cache_on)
{
  if ((unsigned long) cur->sp >= wctx->sbase ||
      (unsigned long) cur->sp < wctx->sp)
    {
      DprintfT (SP_DUMP_UNWIND, "unwind.c: not in stack: %p [0x%lX-0x%lX]\n",
		cur->sp, wctx->sp, wctx->sbase);
      return RA_FAILURE;
    }

  unsigned long ra;
  if (cur->sp == cur->ra_loc)
    {
      ra = cur->ra_sav;
      cur->sp++;
    }
  else if (cur->sp >= cur->sp_safe && (unsigned long) cur->sp < wctx->sbase)
    ra = *cur->sp++;
  else
    {
      DprintfT (SP_DUMP_UNWIND, "unwind.c: not safe: %p >= %p\n", cur->sp, cur->sp_safe);
      return RA_FAILURE;
    }
  if (ra == 0)
    {
      if (cache_on)
	cache_put (wctx, RA_EOSTCK);
      wctx->pc = ra;
      wctx->sp = (unsigned long) cur->sp;
      wctx->fp = (unsigned long) cur->fp;
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d RA_END_OF_STACK\n", __LINE__);
      return RA_END_OF_STACK;
    }

  unsigned long tbgn = wctx->tbgn;
  unsigned long tend = wctx->tend;
  if (ra < tbgn || ra >= tend)
    {
      if (!__collector_check_segment (ra, &tbgn, &tend, 0))
	{
	  DprintfT (SP_DUMP_UNWIND, "unwind.c: not in segment: 0x%lX [0x%lX-0x%lX]\n",
		    ra, wctx->tbgn, wctx->tend);
	  return RA_FAILURE;
	}
    }

  if (cur->cval == RA_FROMFP)
    {
      if (wctx->fp == (unsigned long) (cur->sp - 2))
	{
	  if (cache_on)
	    cache_put (wctx, RA_FROMFP);
	}
      else
	cur->cval = 0;
    }

  unsigned long npc = adjust_ret_addr (ra, ra - tbgn, tend);
  if (npc == 0)
    {
      if (cur->cval == RA_FROMFP)
	{
	  /* We have another evidence that we can trust this RA */
	  DprintfT (SP_DUMP_UNWIND, "unwind.c: trusted fp, pc = 0x%lX\n", wctx->pc);
	  wctx->pc = ra;
	}
      else
	{
	  DprintfT (SP_DUMP_UNWIND, "unwind.c: 0 after adjustment\n");
	  return RA_FAILURE;
	}
    }
  else
    wctx->pc = npc;
  wctx->sp = (unsigned long) cur->sp;
  wctx->fp = (unsigned long) cur->fp;
  wctx->tbgn = tbgn;
  wctx->tend = tend;
  return RA_SUCCESS;
}

static int
process_return (struct WalkContext *wctx, struct AdvWalkContext *cur)
{
  return process_return_real (wctx, cur, 1);
}

static void
omp_cache_put (unsigned long *cur_sp_safe, struct WalkContext * wctx_pc_save,
	       struct WalkContext *wctx, uint32_t val)
{
  if (omp_no_walk && (OmpCurCtxs == NULL || OmpCtxs == NULL || OmpVals == NULL || OmpRAs == NULL))
    {
      size_t sz = OmpValTableSize * sizeof (*OmpCurCtxs);
      OmpCurCtxs = (struct WalkContext *) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpCtxs);
      OmpCtxs = (struct WalkContext *) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpVals);
      OmpVals = (uint32_t*) __collector_allocCSize (__collector_heap, sz, 1);
      sz = OmpValTableSize * sizeof (*OmpRAs);
      OmpRAs = (unsigned long*) __collector_allocCSize (__collector_heap, sz, 1);
    }
  if (OmpCurCtxs == NULL || OmpCtxs == NULL || OmpVals == NULL || OmpRAs == NULL)
    return;

#define USE_18434988_OMP_CACHE_WORKAROUND
#ifndef USE_18434988_OMP_CACHE_WORKAROUND
  uint64_t idx = wctx_pc_save->pc * ROOT_IDX;
  OmpVals[ idx % OmpValTableSize ] = val;
  idx = (idx + val) * ROOT_IDX;
  __collector_memcpy (&(OmpCurCtxs[ idx % OmpValTableSize ]), wctx_pc_save, sizeof (struct WalkContext));
  idx = (idx + val) * ROOT_IDX;
  __collector_memcpy (&(OmpCtxs[ idx % OmpValTableSize ]), wctx, sizeof (struct WalkContext));
#endif
  unsigned long *sp = NULL;
  unsigned long fp = wctx_pc_save->fp;
  int from_fp = 0;
  if (val == RA_END_OF_STACK)
    {
      sp = (unsigned long *) (wctx->sp);
      sp--;
      TprintfT (DBG_LT1, "omp_cache_put: get sp from EOS, sp=%p\n", sp);
    }
  else
    {
      if (fp < wctx_pc_save->sp || fp >= wctx_pc_save->sbase - sizeof (*sp))
	{
	  sp = (unsigned long *) (wctx->sp);
	  sp--;
	  TprintfT (DBG_LT1, "omp_cache_put: get sp from sp, sp=%p\n", sp);
	}
      else
	{
	  TprintfT (DBG_LT1, "omp_cache_put: get sp from fp=0x%lx\n", fp);
	  sp = (unsigned long *) fp;
	  from_fp = 1;
	}
    }

  if (sp < cur_sp_safe || ((unsigned long) sp >= wctx->sbase))
    return;

  unsigned long ra = *sp++;
  if (from_fp)
    {
      unsigned long tbgn = wctx_pc_save->tbgn;
      unsigned long tend = wctx_pc_save->tend;
      if (ra < tbgn || ra >= tend)
	{
	  sp = (unsigned long *) (wctx->sp);
	  sp--;
	  ra = *sp++;
	}
    }
#ifdef USE_18434988_OMP_CACHE_WORKAROUND
  uint64_t idx1 = wctx_pc_save->pc * ROOT_IDX;
  uint64_t idx2 = (idx1 + val) * ROOT_IDX;
  uint64_t idx3 = (idx2 + val) * ROOT_IDX;
  uint64_t idx4 = (idx3 + val) * ROOT_IDX;
  OmpRAs [ idx4 % OmpValTableSize ] = 0; // lock
  OmpVals[ idx1 % OmpValTableSize ] = val;
  __collector_memcpy (&(OmpCurCtxs[ idx2 % OmpValTableSize ]), wctx_pc_save, sizeof (struct WalkContext));
  __collector_memcpy (&(OmpCtxs [ idx3 % OmpValTableSize ]), wctx, sizeof (struct WalkContext));
  OmpRAs [ idx4 % OmpValTableSize ] = ra;
#else
  idx = (idx + val) * ROOT_IDX;
  OmpRAs[ idx % OmpValTableSize ] = ra;
#endif
  TprintfT (DBG_LT1, "omp_cache_put: pc=0x%lx\n", wctx_pc_save->pc);
}

/*
 *  See bug 17166877 - malloc_internal unwind failure.
 *  Sometimes there are several calls right after ret, like:
 *      leave
 *      ret
 *      call xxx
 *      call xxxx
 *      call xxxxx
 *  If they are also jump targets, we should better not
 *  create new jump context for those, since they may
 *  end up into some other function.
 */
static int
is_after_ret (unsigned char * npc)
{
  if (*npc != 0xe8)
    return 0;
  unsigned char * onpc = npc;
  int ncall = 1;
  int maxsteps = 10;
  int mincalls = 3;
  int steps = 0;
  while (*(npc - 5) == 0xe8 && steps < maxsteps)
    {
      npc -= 5;
      ncall++;
      steps++;
    }
  if (*(npc - 1) != 0xc3 || *(npc - 2) != 0xc9)
    return 0;
  steps = 0;
  while (*(onpc + 5) == 0xe8 && steps < maxsteps)
    {
      onpc += 5;
      ncall++;
      steps++;
    }
  if (ncall < mincalls)
    return 0;
  return 1;
}

static int
find_i386_ret_addr (struct WalkContext *wctx, int do_walk)
{
  if (wctx->sp == 0)
    // Some artificial contexts may have %sp set to 0. See SETFUNCTIONCONTEXT()
    return RA_FAILURE;

  /* Check cached values */
  int retc = cache_get (wctx);
  if (retc != RA_FAILURE)
    return retc;

  /* An attempt to perform code analysis for call stack tracing */
  unsigned char opcode;
  unsigned char extop;
  unsigned char extop2;
  unsigned char modrm;
  int imm8; /* immediate operand, byte */
  int immv; /* immediate operand, word(2) or doubleword(4) */
  int reg; /* register code */

  /* Buffer for branch targets (analysis stoppers) */
  unsigned char *targets[MAXTRGTS];
  int ntrg = 0; /* number of entries in the table */
  targets[ntrg++] = (unsigned char*) wctx->pc;
  targets[ntrg++] = (unsigned char*) - 1;

  struct AdvWalkContext buf[MAXCTX];
  struct AdvWalkContext *cur = buf;
  CALL_UTIL (memset)((void*) cur, 0, sizeof (*cur));

  cur->pc = (unsigned char*) wctx->pc;
  cur->sp = (unsigned long*) wctx->sp;
  cur->sp_safe = cur->sp - RED_ZONE; /* allow for the 128-byte red zone on amd64 */
  cur->fp = (unsigned long*) wctx->fp;
  cur->tidx = 1;
  DprintfT (SP_DUMP_UNWIND, "\nstack_unwind (x86 walk):%d %p start\n", __LINE__, cur->pc);

  int nctx = 1; /* number of contexts being processed */
  int cnt = 8192; /* number of instructions to analyse */

  /*
   * The basic idea of our x86 stack unwind is that we don't know
   * if we can trust the frame-pointer register.  So we walk
   * instructions to find a return instruction, at which point
   * we know the return address is on the top of the stack, etc.
   *
   * A severe challenge to walking x86 instructions is when we
   * encounter "jmp *(reg)" instructions, where we are expected
   * to jump to the (unknown-to-us) contents of a register.
   *
   * The "jmp_reg" code here attempts to keep track of the
   * context for such a jump, deferring any handling of such
   * a difficult case.  We continue with other contexts, hoping
   * that some other walk will take us to a return instruction.
   *
   * If no other walk helps, we return to "jmp_reg" contexts.
   * While we don't know the jump target, it is possible that the
   * bytes immediately following the jmp_reg instruction represent
   * one possible target, as might be the case when a "switch"
   * statement is compiled.
   *
   * Unfortunately, the bytes following a "jmp_reg" instruction might
   * instead be a jump target from somewhere else -- execution might
   * never "fall through" from the preceding "jmp_reg".  Those bytes
   * might not even be instructions at all.  There are many uses of
   * jmp_reg instructions beyond just compiling switch statements.
   *
   * So walking the bytes after a "jmp_reg" instruction can lead
   * to bugs and undefined behavior, including SEGV and core dump.
   *
   * We currently do not really understand the "jmp_reg" code below.
   */
  int jmp_reg_switch_mode = 0;
  int num_jmp_reg = 0; // number of jmp *reg met when switch mode is off or when in current switch case
  int total_num_jmp_reg = 0; // number of total jmp *reg met
  struct AdvWalkContext * jmp_reg_ctx[MAXJMPREG]; // context of jmp *reg met when switch mode is off or when in current switch case
  struct AdvWalkContext * jmp_reg_switch_ctx[MAXJMPREG]; // context of jmp *reg used in switch cases
  struct AdvWalkContext * jmp_reg_switch_backup_ctx = NULL; // context of the first jmp *reg used in switch cases

  int cur_jmp_reg_switch = 0; // current switch table
  int num_jmp_reg_switch = 0; // number of switch table
  int jmp_reg_switch_case = 0; // case number in current switch table
  unsigned char * jmp_reg_switch_pc = NULL; // the start pc of current switch case
  unsigned char * jmp_reg_switch_pc_old = NULL; // backup for deleteing context of jump target
  unsigned char * jmp_reg_switch_base = NULL; // start pc for checking offsets
  int max_jmp_reg_switch_case = 2;
#if WSIZE(32)
  int max_switch_pc_offset = 512;
#else // WSIZE(64)
  int max_switch_pc_offset = 1024;
#endif
  int expected_num_jmp_reg = 1; // should be smaller than MAXJMPREG
  int max_num_jmp_reg_seen = 4; // try to resolve return if there are so many such instructions


  int save_ctx = 0; // flag to save walk context in the cache to speed up unwind
  struct WalkContext wctx_pc_save;
  if (do_walk == 0)
    // do_walk is the flag indicating not walking through the instructions, resolving the RA from the stack fp first
    __collector_memcpy (&wctx_pc_save, wctx, sizeof (struct WalkContext));

startWalk:
  if (do_walk == 0)
    { // try to resolve RA from stack frame pointer
      if (OmpCurCtxs == NULL || OmpCtxs == NULL || OmpVals == NULL || OmpRAs == NULL)
	{
	  do_walk = 1;
	  goto startWalk;
	}
      // before goto checkFP, try the RA from cache (key: WalkContext -> value: caller's WalkContext))
      uint64_t idx = wctx->pc * ROOT_IDX;
      uint32_t val = OmpVals[idx % OmpValTableSize];
      idx = (idx + val) * ROOT_IDX;
#ifdef USE_18434988_OMP_CACHE_WORKAROUND
      // Check ra: if it is 0 - then cache is invalid
      uint64_t idx4;
      idx4 = (idx + val) * ROOT_IDX;
      idx4 = (idx4 + val) * ROOT_IDX;
      if (0 == OmpRAs[ idx4 % OmpValTableSize ])  // Invalid cache
	goto checkFP;
#endif
      struct WalkContext saved_ctx;
      __collector_memcpy (&saved_ctx, &OmpCurCtxs[ idx % OmpValTableSize ], sizeof (struct WalkContext));
      if (wctx->pc == saved_ctx.pc
	  && wctx->sp == saved_ctx.sp
	  && wctx->fp == saved_ctx.fp
	  && wctx->tbgn == saved_ctx.tbgn
	  && wctx->tend == saved_ctx.tend)
	{ // key match, RA may be valid
	  idx = (idx + val) * ROOT_IDX;
	  unsigned long *sp = NULL;
	  unsigned long fp = wctx->fp;
	  int from_fp = 0;
	  if (val == RA_END_OF_STACK)
	    {
	      DprintfT (SP_DUMP_UNWIND, "find_i386_ret_addr:%d -- RA_END_OF_STACK: pc=0x%lx\n", __LINE__, wctx->pc);
	      __collector_memcpy (wctx, &OmpCtxs[ idx % OmpValTableSize ], sizeof (struct WalkContext));
	      return val;
	    }
	  else
	    {
	      if (fp < wctx->sp || fp >= wctx->sbase - sizeof (*sp))
		{
		  TprintfT (DBG_LT1, "omp_cache_get -- wrong fp: pc=0x%lx\n", wctx->pc);
		  sp = (unsigned long *) (OmpCtxs[ idx % OmpValTableSize ].sp);
		  sp--;
		  if (sp < cur->sp_safe || (unsigned long) sp >= wctx->sbase)
		    {
		      goto checkFP;
		    }
		  unsigned long ra = *sp;
		  uint64_t idx2 = (idx + val) * ROOT_IDX;
		  if (OmpRAs[ idx2 % OmpValTableSize ] == ra)
		    {
		      __collector_memcpy (wctx, &OmpCtxs[ idx % OmpValTableSize ], sizeof (struct WalkContext));
		      TprintfT (DBG_LT1, "omp_cache_get -- ra match with target sp: pc=0x%lx, ra=0x%lx, val=%d\n", wctx->pc, ra, val);
		      return val;
		    }
		  TprintfT (DBG_LT1, "omp_cache_get -- ra mismatch: ra=0x%lx, expected ra=0x%lx, val=%d\n", ra, OmpRAs[ idx2 % OmpValTableSize ], val);
		  goto checkFP;
		}
	      sp = (unsigned long *) fp;
	      from_fp = 1;
	    }

	  uint64_t idx2 = (idx + val) * ROOT_IDX;
	  unsigned long ra = *sp++;
	  if (from_fp)
	    {
	      unsigned long tbgn = wctx->tbgn;
	      unsigned long tend = wctx->tend;
	      if (ra < tbgn || ra >= tend)
		{
		  sp = (unsigned long *) (OmpCtxs[ idx % OmpValTableSize ].sp);
		  sp--;
		  //if (sp < cur->sp_safe - 16 || (unsigned long)sp >= wctx->sbase - sizeof(*sp)) {
		  // The check above was replaced with the check below,
		  // because we do not know why "- 16" and "- sizeof(*sp)" was used.
		  if (sp < cur->sp_safe || (unsigned long) sp >= wctx->sbase)
		    goto checkFP;
		  else
		    ra = *sp;
		}
	    }
	  if (OmpRAs[ idx2 % OmpValTableSize ] == ra)
	    {
	      TprintfT (DBG_LT1, "omp_cache_get -- ra match: pc=0x%lx\n", wctx->pc);
	      __collector_memcpy (wctx, &OmpCtxs[ idx % OmpValTableSize ], sizeof (struct WalkContext));
	      return val;
	    }
	}
      goto checkFP;
    }
  else
    {
      CALL_UTIL (memset)(jmp_reg_ctx, 0, MAXJMPREG * sizeof (struct AdvWalkContext *));
      CALL_UTIL (memset)(jmp_reg_switch_ctx, 0, MAXJMPREG * sizeof (struct AdvWalkContext *));
    }
  while (cnt--)
    {
      if (nctx == 0 && (num_jmp_reg == expected_num_jmp_reg || jmp_reg_switch_mode == 1))
	{ // no context available, try jmp switch mode
	  int i = 0;
	  if (num_jmp_reg == expected_num_jmp_reg)
	    jmp_reg_switch_mode = 0; // first jmp reg expected, restart switch mode
	  DprintfT (SP_DUMP_UNWIND, "unwind.c: begin switch mode, num_jmp_reg = %d, jmp_reg_switch_backup_ctx=%p, jmp_reg_switch_case=%d, jmp_reg_switch_mode=%d.\n",
		    num_jmp_reg, jmp_reg_switch_backup_ctx, jmp_reg_switch_case, jmp_reg_switch_mode);
	  // the ideal asm of switch is
	  //   jmp reg
	  //   ...//case 1
	  //   ret
	  //   ...//case 2
	  //   ret
	  //   ...//etc
	  if (jmp_reg_switch_mode == 0)
	    {
	      num_jmp_reg_switch = num_jmp_reg; // backup num_jmp_reg
	      jmp_reg_switch_mode = 1; // begin switch mode
	      for (i = 0; i < num_jmp_reg_switch; i++)
		{
		  if (jmp_reg_switch_ctx[i] == NULL)
		    jmp_reg_switch_ctx[i] = (struct AdvWalkContext*) alloca (sizeof (*jmp_reg_switch_ctx[i]));
		  if (jmp_reg_switch_ctx[i] != NULL)
		    { // backup jmp_reg_ctx
		      __collector_memcpy (jmp_reg_switch_ctx[i], jmp_reg_ctx[i], sizeof (*jmp_reg_switch_ctx[i]));
		      cur_jmp_reg_switch = 0; // reset the current switch table
		      jmp_reg_switch_case = 0; // reset the case number in current switch table
		    }
		}
	      if (jmp_reg_switch_backup_ctx == NULL)
		{ // only backup when the first jmp *reg is met for restoring later, if switch mode fails to resolve RA
		  jmp_reg_switch_backup_ctx = (struct AdvWalkContext*) alloca (sizeof (*jmp_reg_switch_backup_ctx));
		  if (jmp_reg_switch_backup_ctx != NULL)
		    __collector_memcpy (jmp_reg_switch_backup_ctx, cur, sizeof (*cur));
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: back up context for switch mode.\n");
		}
	    }
	  if (jmp_reg_switch_mode == 1)
	    { // in the process of trying switch cases
	      if (cur_jmp_reg_switch == num_jmp_reg_switch)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: have tried all switch with max_jmp_reg_switch_case for each\n");
		  if (jmp_reg_switch_backup_ctx != NULL)
		    __collector_memcpy (cur, jmp_reg_switch_backup_ctx, sizeof (*cur));
		  int rc = process_return_real (wctx, cur, 0);
		  if (rc == RA_SUCCESS)
		    {
		      if (save_ctx)
			omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		      return rc;
		    }
		  break; // have tried all switch with max_jmp_reg_switch_case for each, goto checkFP
		}
	      unsigned char *npc = jmp_reg_switch_ctx[cur_jmp_reg_switch]->pc;
	      if (jmp_reg_switch_case == 0)
		// first switch case
		npc = check_modrm (npc); // pc next to "jmp reg" instruction
	      else if (jmp_reg_switch_pc != NULL)
		npc = jmp_reg_switch_pc; // // pc next to "ret" instruction of previous case
	      else
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: unexpected jum switch mode situation, jmp_reg_switch_case=%d, jmp_reg_switch_pc=%p\n",
			    jmp_reg_switch_case, jmp_reg_switch_pc);
		  break; //goto checkFP
		}
	      jmp_reg_switch_base = npc;
	      struct AdvWalkContext *new = buf + nctx;
	      nctx += 1;
	      __collector_memcpy (new, jmp_reg_switch_ctx[cur_jmp_reg_switch], sizeof (*new));
	      new->pc = npc;
	      cur = new; /* advance the new context first */
	      jmp_reg_switch_pc = NULL;
	      jmp_reg_switch_case++;
	      if (jmp_reg_switch_case == max_jmp_reg_switch_case)
		{ // done many cases, change to another switch table
		  cur_jmp_reg_switch++;
		  jmp_reg_switch_case = 0;
		}
	    }
	  num_jmp_reg = 0;
	}
      if (jmp_reg_switch_mode == 1)
	{ // when processing switch cases, check pc each time
	  unsigned long tbgn = wctx->tbgn;
	  unsigned long tend = wctx->tend;
	  if ((unsigned long) (cur->pc) < tbgn || (unsigned long) (cur->pc) >= tend)
	    {
	      DprintfT (SP_DUMP_UNWIND, "unwind.c: pc out of range, pc=0x%lx\n", (unsigned long) (cur->pc));
	      break;
	    }
	  if (jmp_reg_switch_base != NULL && cur->pc > jmp_reg_switch_base + max_switch_pc_offset)
	    {
	      DprintfT (SP_DUMP_UNWIND, "unwind.c: limit the walk offset after jmp reg instruction\n");
	      if (jmp_reg_switch_backup_ctx != NULL)
		__collector_memcpy (cur, jmp_reg_switch_backup_ctx, sizeof (*cur));
	      int rc = process_return_real (wctx, cur, 0);
	      if (rc == RA_SUCCESS)
		{
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		  return rc;
		}
	      break; // limit the walk offset after jmp reg instruction, got checkFP
	    }
	}

      if (nctx == 0)
	break;
//      dump_targets (__LINE__, ntrg, targets);
      while (cur->pc > targets[cur->tidx])
	cur->tidx += 1;
      if (cur->pc == targets[cur->tidx])
	{
	  /* Stop analysis. Delete context. */
	  if (jmp_reg_switch_mode == 0 || cur->pc != jmp_reg_switch_pc_old)
	    {
	      if (jmp_reg_switch_mode == 1 && nctx == 1 && jmp_reg_switch_pc == NULL)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d old target, cur->pc=%p, jmp_reg_switch_pc=%p, nctx=%d\n",
			    __LINE__, cur->pc, jmp_reg_switch_pc, nctx);
		  jmp_reg_switch_pc = cur->pc; // save cp before delete context, may be used as a start of switch case
		  jmp_reg_switch_pc_old = jmp_reg_switch_pc;
		}
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, old target.\n", __LINE__);
	      DELETE_CURCTX ();
	      if (cur >= buf + nctx)
		cur = buf;
	      continue;
	    }
	  if (jmp_reg_switch_mode == 1 && cur->pc == jmp_reg_switch_pc_old)
	    jmp_reg_switch_pc_old = NULL; // reset jmp_reg_switch_pc_old to delete the context later when cur->pc != jmp_reg_switch_pc_old
	}

      /* let's walk the next x86 instruction */
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d cur:%ld pc=0x%lx %02x %02x %02x %02x %02x %02x %02x sp=0x%lx\n",
	       __LINE__, (long) (cur - buf), (unsigned long) cur->pc,
	       (int) cur->pc[0], (int) cur->pc[1], (int) cur->pc[2],
	       (int) cur->pc[3], (int) cur->pc[4], (int) cur->pc[5],
	       (int) cur->pc[6], (unsigned long) cur->sp);
      int v = 4; /* Operand size */
      int a = 4; /* Address size */
      /* int W = 0;	   REX.W bit */
#if WSIZE(64)
      int R = 0; /* REX.R bit */
#endif
      int X = 0; /* REX.X bit */
      int B = 0; /* REX.B bit */
      /* Check prefixes */
      int done = 0;
      while (!done)
	{
	  opcode = *cur->pc++;
	  switch (opcode)
	    {
	    case 0x66: /* opd size override */
	      v = 2;
	      break;
	    case 0x67: /*addr size override */
	      a = 2;
	      break;
#if WSIZE(64)
	    case 0x40: /* REX */
	    case 0x41:
	    case 0x42:
	    case 0x43:
	    case 0x44:
	    case 0x45:
	    case 0x46:
	    case 0x47:
	    case 0x48:
	    case 0x49:
	    case 0x4a:
	    case 0x4b:
	    case 0x4c:
	    case 0x4d:
	    case 0x4e:
	    case 0x4f:
	      B = (opcode & 0x1) ? 8 : 0;
	      X = (opcode & 0x2) ? 8 : 0;
	      R = (opcode & 0x4) ? 8 : 0;
	      if (opcode & 0x8)  /* 64 bit operand size */
		v = 8;
	      opcode = *cur->pc++;
	      done = 1;
	      break;
#endif
	    default:
	      done = 1;
	      break;
	    }
	}
      int z = (v == 8) ? 4 : v;
      switch (opcode)
	{
	case 0x0: /* add Eb,Gb */
	case 0x01: /* add Ev,Gv */
	case 0x02: /* add Gb,Eb */
	case 0x03: /* add Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x04: /* add %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x05: /* add %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x06: /* push es */
	  cur->sp -= 1;
	  break;
	case 0x07: /* pop es */
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0x08: /* or Eb,Gb */
	case 0x09: /* or Ev,Gv */
	case 0x0a: /* or Gb,Eb */
	case 0x0b: /* or Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x0c: /* or %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x0d: /* or %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x0e: /* push cs */
	  cur->sp -= 1;
	  break;
	case 0x0f: /* two-byte opcodes */
	  extop = *cur->pc++;
	  switch (extop)
	    { /* RTM or HLE */
	    case 0x01:
	      extop2 = *cur->pc;
	      switch (extop2)
		{
		case 0xd5: /* xend */
		case 0xd6: /* xtest */
		  cur->pc++;
		  break;
		default:
		  break;
		}
	      break;
	    case 0x03:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x0b:
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, undefined instruction. opcode=0x%02x\n",
		       __LINE__, (int) opcode);
	      DELETE_CURCTX ();
	      break;
	    case 0x05: /* syscall */
	    case 0x34: /* sysenter */
	      if (cur->rax == __NR_exit)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode=0x%02x\n",
			   __LINE__, (int) opcode);
		  DELETE_CURCTX ();
		  break;
		}
	      else if (cur->rax == __NR_rt_sigreturn)
		{
		  if (jmp_reg_switch_mode == 1)
		    {
		      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address under jmp switch mode, opcode=0x%02x\n",
			       __LINE__, (int) opcode);
		      goto checkFP;
		    }
		  wctx->sp = (unsigned long) cur->sp;
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_RT_SIGRETURN);
		  return RA_RT_SIGRETURN;
		}
#if WSIZE(32)
	      else if (cur->rax == __NR_sigreturn)
		{
		  if (jmp_reg_switch_mode == 1)
		    {
		      DprintfT (SP_DUMP_UNWIND, "unwind.c: give up return address under jmp switch mode, opcode = 0x34\n");
		      goto checkFP;
		    }
		  wctx->sp = (unsigned long) cur->sp;
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_SIGRETURN);
		  return RA_SIGRETURN;
		}
#endif
	      /* Check for Linus' trick in the vsyscall page */
	      while (*cur->pc == 0x90)  /* nop */
		cur->pc++;
	      if (*cur->pc == 0xeb)  /* jmp imm8 */
		cur->pc += 2;
	      break;
	    case 0x0d: /* nop Ev */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x10: /* xmm Vq,Wq */
	    case 0x11:
	    case 0x12:
	    case 0x13:
	    case 0x14:
	    case 0x15:
	    case 0x16:
	    case 0x17:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x18: /* prefetch */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x1E: /* endbr64/endbr32 (f3 0f 1e .. ) is parsing as repz nop edx */
	      cur->pc += 2;
	      break;
	    case 0x1f: /* nop Ev */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x28: /* xmm Vq,Wq */
	    case 0x29:
	    case 0x2a:
	    case 0x2b:
	    case 0x2c:
	    case 0x2d:
	    case 0x2e:
	    case 0x2f:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x30: /* wrmsr */
	    case 0x31: /* rdtsc */
	    case 0x32: /* rdmsr */
	    case 0x33: /* rdpmc */
	      break;
	      /* case 0x34: sysenter (see above) */
	    case 0x38: case 0x3a:
	      extop2 = *cur->pc++;
	      cur->pc = check_modrm (cur->pc);
	      // 21275311 Unwind failure in native stack for java application running on jdk8
	      // Three-byte opcodes "66 0f 3a ??" should consume an additional "immediate" byte.
	      if (extop == 0x3a)
		cur->pc++;
	      break;
	    case 0x40: case 0x41: case 0x42: case 0x43: /* CMOVcc Gv,Ev */
	    case 0x44: case 0x45: case 0x46: case 0x47:
	    case 0x48: case 0x49: case 0x4a: case 0x4b:
	    case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x50: case 0x51: case 0x52: case 0x53:
	    case 0x54: case 0x55: case 0x56: case 0x57:
	    case 0x58: case 0x59: case 0x5a: case 0x5b:
	    case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	    case 0x60: case 0x61: case 0x62: case 0x63:
	    case 0x64: case 0x65: case 0x66: case 0x67:
	    case 0x68: case 0x69: case 0x6a: case 0x6b:
	    case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x70: case 0x71: case 0x72: case 0x73:
	      cur->pc = check_modrm (cur->pc) + 1;
	      break;
	    case 0x74: case 0x75: case 0x76:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x77:
	      break;
	    case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x80: case 0x81: case 0x82: case 0x83: /* Jcc Jz */
	    case 0x84: case 0x85: case 0x86: case 0x87:
	    case 0x88: case 0x89: case 0x8a: case 0x8b:
	    case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	      immv = read_int (cur->pc, z);
	      cur->pc += z;
	      if (nctx < (jmp_reg_switch_mode ? MAXJMPREGCTX : MAXCTX))
		{
		  int tidx = 0;
		  unsigned char *npc = cur->pc + immv;
		  if ((unsigned long) npc < wctx->tbgn || (unsigned long) npc >= wctx->tend)
		    {
		      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode=0x%02x\n",
			       __LINE__, (int) opcode);
		      DELETE_CURCTX ();
		      break;
		    }
		  if (is_after_ret (npc))
		    break;
		  while (npc > targets[tidx])
		    tidx += 1;
		  if (npc != targets[tidx])
		    {
		      if (ntrg < MAXTRGTS)
			{
			  for (int i = 0; i < nctx; i++)
			    if (buf[i].tidx >= tidx)
			      buf[i].tidx++;

			  /* insert a new target */
			  for (int i = ntrg; i > tidx; i--)
			    targets[i] = targets[i - 1];
			  ntrg += 1;
			  targets[tidx++] = npc;
			}
		      else
			DprintfT (SP_DUMP_UNWIND, "unwind.c:%d ntrg=max(%d)\n",
				  __LINE__, ntrg);
		      struct AdvWalkContext *new = buf + nctx;
		      nctx += 1;
		      __collector_memcpy (new, cur, sizeof (*new));
		      new->pc = npc;
		      new->tidx = tidx;
		      cur = new; /* advance the new context first */
		      continue;
		    }
		}
	      else
		DprintfT (SP_DUMP_UNWIND, "unwind.c:%d nctx=max(%d)\n",
			  __LINE__, ntrg);
	      break;
	    case 0x90: case 0x91: case 0x92: case 0x93: /* setcc Eb */
	    case 0x94: case 0x95: case 0x96: case 0x97:
	    case 0x98: case 0x99: case 0x9a: case 0x9b:
	    case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xa0: /* push fs */
	      cur->sp -= 1;
	      break;
	    case 0xa1: /* pop fs */
	      cur->sp += 1;
	      if (cur->sp - RED_ZONE > cur->sp_safe)
		cur->sp_safe = cur->sp - RED_ZONE;
	      break;
	    case 0xa2: /* cpuid */
	      break;
	    case 0xa3: /* bt Ev,Gv */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xa4: /* shld Ev,Gv,Ib */
	      cur->pc = check_modrm (cur->pc);
	      cur->pc += 1;
	      break;
	    case 0xa5: /* shld Ev,Gv,%cl */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xa8: /* push gs */
	      cur->sp -= 1;
	      break;
	    case 0xa9: /* pop gs */
	      cur->sp += 1;
	      if (cur->sp - RED_ZONE > cur->sp_safe)
		cur->sp_safe = cur->sp - RED_ZONE;
	      break;
	    case 0xaa: /* rsm */
	      break;
	    case 0xab: /* bts Ev,Gv */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xac: /* shrd Ev,Gv,Ib */
	      cur->pc = check_modrm (cur->pc);
	      cur->pc += 1;
	      break;
	    case 0xad: /* shrd Ev,Gv,%cl */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xae: /* group15 */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xaf: /* imul Gv,Ev */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xb1: /* cmpxchg Ev,Gv */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xb3:
	    case 0xb6: /* movzx Gv,Eb */
	    case 0xb7: /* movzx Gv,Ew */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xba: /* group8 Ev,Ib */
	      cur->pc = check_modrm (cur->pc);
	      cur->pc += 1;
	      break;
	    case 0xbb: /* btc Ev,Gv */
	    case 0xbc: /* bsf Gv,Ev */
	    case 0xbd: /* bsr Gv,Ev */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xbe: /* movsx Gv,Eb */
	    case 0xbf: /* movsx Gv,Ew */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xc0: /* xadd Eb,Gb */
	    case 0xc1: /* xadd Ev,Gv */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xc2: /* cmpps V,W,Ib */
	      cur->pc = check_modrm (cur->pc);
	      cur->pc += 1;
	      break;
	    case 0xc3: /* movnti M,G */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xc6: /* shufps V,W,Ib */
	      cur->pc = check_modrm (cur->pc);
	      cur->pc += 1;
	      break;
	    case 0xc7: /* RDRAND */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0xc8: case 0xc9: case 0xca: case 0xcb: /* bswap */
	    case 0xcc: case 0xcd: case 0xce: case 0xcf:
	      break;
	    case 0xd0: case 0xd1: case 0xd2: case 0xd3:
	    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	    case 0xd8: case 0xd9: case 0xda: case 0xdb:
	    case 0xdc: case 0xdd: case 0xde: case 0xdf:
	    case 0xe0: case 0xe1: case 0xe2: case 0xe3:
	    case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	    case 0xe8: case 0xe9: case 0xea: case 0xeb:
	    case 0xec: case 0xed: case 0xee: case 0xef:
	    case 0xf0: case 0xf1: case 0xf2: case 0xf3:
	    case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	    case 0xf8: case 0xf9: case 0xfa: case 0xfb:
	    case 0xfc: case 0xfd: case 0xfe: case 0xff:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    default:
	      if (jmp_reg_switch_mode == 1 && extop == 0x0b)
		DprintfT (SP_DUMP_UNWIND, "unwind.c:%d invalid opcode ub2: 0x0f %x jmp_reg_switch_mode=%d\n",
			  __LINE__, (int) extop, jmp_reg_switch_mode);
	      else
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d unknown opcode: 0x0f %x jmp_reg_switch_mode=%d\n",
			    __LINE__, (int) extop, jmp_reg_switch_mode);
		  DELETE_CURCTX ();
		}
	      break;
	    }
	  break;
	case 0x10: /* adc Eb,Gb */
	case 0x11: /* adc Ev,Gv */
	case 0x12: /* adc Gb,Eb */
	case 0x13: /* adc Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x14: /* adc %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x15: /* adc %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x16: /* push ss */
	  cur->sp -= 1;
	  break;
	case 0x17: /* pop ss */
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0x18: /* sbb Eb,Gb */
	case 0x19: /* sbb Ev,Gv */
	case 0x1a: /* sbb Gb,Eb */
	case 0x1b: /* sbb Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x1c: /* sbb %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x1d: /* sbb %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x1e: /* push ds */
	  cur->sp -= 1;
	  break;
	case 0x1f: /* pop ds */
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0x20: /* and Eb,Gb */
	case 0x21: /* and Ev,Gv */
	case 0x22: /* and Gb,Eb */
	case 0x23: /* and Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x24: /* and %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x25: /* and %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x26: /* seg=es prefix */
	  break;
	case 0x27: /* daa */
	  break;
	case 0x28: /* sub Eb,Gb */
	case 0x29: /* sub Ev,Gv */
	case 0x2a: /* sub Gb,Eb */
	case 0x2b: /* sub Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x2c: /* sub %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x2d: /* sub %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x2e: /* seg=cs prefix */
	  break;
	case 0x2f: /* das */
	  break;
	case 0x30: /* xor Eb,Gb */
	case 0x31: /* xor Ev,Gv */
	case 0x32: /* xor Gb,Eb */
	case 0x33: /* xor Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x34: /* xor %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x35: /* xor %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x36: /* seg=ss prefix */
	  break;
	case 0x37: /* aaa */
	  break;
	case 0x38: /* cmp Eb,Gb */
	case 0x39: /* cmp Ev,Gv */
	case 0x3a: /* cmp Gb,Eb */
	case 0x3b: /* cmp Gv,Ev */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x3c: /* cmp %al,Ib */
	  cur->pc += 1;
	  break;
	case 0x3d: /* cmp %eax,Iz */
	  cur->pc += z;
	  break;
	case 0x3e: /* seg=ds prefix */
	  break;
	case 0x3f: /* aas */
	  break;
#if WSIZE(32)
	case 0x40: /* inc %eax */
	case 0x41: /* inc %ecx */
	case 0x42: /* inc %edx */
	case 0x43: /* inc %ebx */
	  break;
	case 0x44: /* inc %esp */
	  /* Can't be a valid stack pointer - delete context */
	  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0x44.\n", __LINE__);
	  DELETE_CURCTX ();
	  break;
	case 0x45: /* inc %ebp */
	case 0x46: /* inc %esi */
	case 0x47: /* inc %edi */
	case 0x48: /* dec %eax */
	case 0x49: /* dec %ecx */
	case 0x4a: /* dec %edx */
	case 0x4b: /* dec %ebx */
	  break;
	case 0x4c: /* dec %esp */
	  /* Can't be a valid stack pointer - delete context */
	  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0x4c.\n", __LINE__);
	  DELETE_CURCTX ();
	  break;
	case 0x4d: /* dec %ebp */
	case 0x4e: /* dec %esi */
	case 0x4f: /* dec %edi */
	  break;
#endif
	case 0x50: /* push %eax */
	case 0x51: /* push %ecx */
	case 0x52: /* push %edx */
	case 0x53: /* push %ebx */
	case 0x54: /* push %esp */
	case 0x55: /* push %ebp */
	case 0x56: /* push %esi */
	case 0x57: /* push %edi */
	  cur->sp -= 1;
	  reg = OPC_REG (opcode);
	  if (reg == RBP)
	    {
#if 0
	      /* Don't do this check yet. Affects tail calls. */
	      /* avoid other function's prologue */
	      if ((cur->pc[0] == 0x89 && cur->pc[1] == 0xe5) ||
		  (cur->pc[0] == 0x8b && cur->pc[1] == 0xec))
		{
		  /* mov %esp,%ebp */
		  DELETE_CURCTX ();
		  break;
		}
#endif
	      if (cur->fp_loc == NULL)
		{
		  cur->fp_loc = cur->sp;
		  cur->fp_sav = cur->fp;
		}
	    }
	  break;
	case 0x58: /* pop %eax */
	case 0x59: /* pop %ecx */
	case 0x5a: /* pop %edx */
	case 0x5b: /* pop %ebx */
	case 0x5c: /* pop %esp */
	case 0x5d: /* pop %ebp */
	case 0x5e: /* pop %esi */
	case 0x5f: /* pop %edi */
	  reg = OPC_REG (opcode);
	  cur->regs[reg] = 0;
	  if (isInside ((unsigned long) cur->sp, (unsigned long) cur->sp_safe, wctx->sbase))
	    cur->regs[reg] = *cur->sp;
	  DprintfT (SP_DUMP_UNWIND, "stack_unwind:%d cur->regs[%d]=0x%lx\n",
		   __LINE__, reg, (unsigned long) cur->regs[reg]);
	  if (reg == RDX)
	    {
	      if (cur->sp >= cur->sp_safe &&
		  (unsigned long) cur->sp < wctx->sbase)
		cur->rdx = *cur->sp;
	    }
	  else if (reg == RBP)
	    {
	      if (cur->fp_loc == cur->sp)
		{
		  cur->fp = cur->fp_sav;
		  cur->fp_loc = NULL;
		}
	      else if (cur->sp >= cur->sp_safe &&
		       (unsigned long) cur->sp < wctx->sbase)
		cur->fp = (unsigned long*) (*cur->sp);
	    }
	  else if (reg == RSP)
	    {
	      /* f.e. JVM I2CAdapter */
	      if (cur->sp >= cur->sp_safe && (unsigned long) cur->sp < wctx->sbase)
		{
		  unsigned long *nsp = (unsigned long*) (*cur->sp);
		  if (nsp >= cur->sp && nsp <= cur->fp)
		    {
		      cur->sp = nsp;
		    }
		  else
		    {
		      DprintfT (SP_DUMP_UNWIND, "stack_unwind%d give up return address, opcode=0x%02x\n",
			       __LINE__, opcode);
		      goto checkFP;
		    }
		}
	      else
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address, opcode=0x%02x\n",
			    __LINE__, opcode);
		  goto checkFP;
		}
	      break;
	    }
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    {
	      cur->sp_safe = cur->sp - RED_ZONE;
	    }
	  break;
	case 0x60: /* pusha(d) */
	  cur->sp -= 8;
	  break;
	case 0x61: /* popa(d) */
	  cur->sp += 8;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0x62: /* group AVX, 4-bytes EVEX prefix */
	  {
	    unsigned char *pc = cur->pc - 1; // points to the beginning of the instruction
	    int len = parse_x86_AVX_instruction (pc);
	    if (len < 4)
	      {
		DELETE_CURCTX ();
	      }
	    else
	      {
		pc += len;
		cur->pc = pc;
	      }
	  }
	  break;
	case 0x63: /* arpl Ew,Gw (32) movsxd Gv,Ev (64)*/
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x64: /* seg=fs prefix */
	case 0x65: /* seg=gs prefix */
	  break;
	case 0x66: /* opd size override */
	case 0x67: /* addr size override */
	  break;
	case 0x68: /* push Iz */
	  cur->sp = (unsigned long*) ((long) cur->sp - z);
	  cur->pc += z;
	  break;
	case 0x69: /* imul Gv,Ev,Iz */
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += z;
	  break;
	case 0x6a: /* push Ib */
	  cur->sp = (unsigned long*) ((long) cur->sp - v);
	  cur->pc += 1;
	  break;
	case 0x6b: /* imul Gv,Ev,Ib */
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += 1;
	  break;
	case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x70: /* jo Jb */
	case 0x71: /* jno Jb */
	case 0x72: /* jb Jb */
	case 0x73: /* jnb Jb */
	case 0x74: /* jz Jb */
	case 0x75: /* jnz Jb */
	case 0x76: /* jna Jb */
	case 0x77: /* ja Jb */
	case 0x78: /* js Jb */
	case 0x79: /* jns Jb */
	case 0x7a: /* jp Jb */
	case 0x7b: /* jnp Jb */
	case 0x7c: /* jl Jb */
	case 0x7d: /* jge Jb */
	case 0x7e: /* jle Jb */
	case 0x7f: /* jg Jb */
	  imm8 = *(char*) cur->pc++;
	  if (nctx < (jmp_reg_switch_mode ? MAXJMPREGCTX : MAXCTX))
	    {
	      int tidx = 0;
	      unsigned char *npc = cur->pc + imm8;
	      if (is_after_ret (npc))
		break;
	      while (npc > targets[tidx])
		tidx += 1;
	      if (npc != targets[tidx])
		{
		  if (ntrg < MAXTRGTS)
		    {
		      for (int i = 0; i < nctx; i++)
			if (buf[i].tidx >= tidx)
			  buf[i].tidx++;

		      /* insert a new target */
		      for (int i = ntrg; i > tidx; i--)
			targets[i] = targets[i - 1];
		      ntrg += 1;
		      targets[tidx++] = npc;
		    }
		  else
		    DprintfT (SP_DUMP_UNWIND, "unwind.c:%d ntrg(%d)=max\n", __LINE__, ntrg);
		  struct AdvWalkContext *new = buf + nctx;
		  nctx += 1;
		  __collector_memcpy (new, cur, sizeof (*new));
		  new->pc = npc;
		  new->tidx = tidx;
		  cur = new; /* advance the new context first */
		  continue;
		}
	    }
	  else
	    DprintfT (SP_DUMP_UNWIND, "unwind.c:%d nctx(%d)=max\n", __LINE__, nctx);
	  break;
	case 0x80: /* group1 Eb,Ib */
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += 1;
	  break;
	case 0x81: /* group1 Ev,Iz */
	  modrm = *cur->pc;
	  if (MRM_MOD (modrm) == 0xc0 && MRM_REGS (modrm) == RSP)
	    {
	      int immz = read_int (cur->pc + 1, z);
	      extop = MRM_EXT (modrm);
	      if (extop == 0) /* add  imm32,%esp */
		cur->sp = (unsigned long*) ((long) cur->sp + immz);
	      else if (extop == 4) /* and imm32,%esp */
		cur->sp = (unsigned long*) ((long) cur->sp & immz);
	      else if (extop == 5) /* sub imm32,%esp */
		cur->sp = (unsigned long*) ((long) cur->sp - immz);
	      if (cur->sp - RED_ZONE > cur->sp_safe)
		cur->sp_safe = cur->sp - RED_ZONE;
	    }
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += z;
	  break;
	case 0x82: /* group1 Eb,Ib */
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += 1;
	  break;
	case 0x83: /* group1 Ev,Ib */
	  modrm = *cur->pc;
	  if (MRM_MOD (modrm) == 0xc0 && MRM_REGS (modrm) == RSP)
	    {
	      imm8 = (char) cur->pc[1]; /* sign extension */
	      extop = MRM_EXT (modrm);
	      if (extop == 0) /* add  imm8,%esp */
		cur->sp = (unsigned long*) ((long) cur->sp + imm8);
	      else if (extop == 4) /* and imm8,%esp */
		  cur->sp = (unsigned long*) ((long) cur->sp & imm8);
	      else if (extop == 5) /* sub imm8,%esp */
		cur->sp = (unsigned long*) ((long) cur->sp - imm8);
	      if (cur->sp - RED_ZONE > cur->sp_safe)
		cur->sp_safe = cur->sp - RED_ZONE;
	    }
	  cur->pc = check_modrm (cur->pc);
	  cur->pc += 1;
	  break;
	case 0x84: /* test Eb,Gb */
	case 0x85: /* test Ev,Gv */
	case 0x86: /* xchg Eb,Gb */
	case 0x87: /* xchg Ev,Gv */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x88: /* mov Eb,Gb */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x89: /* mov Ev,Gv */
	  modrm = *cur->pc;
	  if (MRM_MOD (modrm) == 0xc0)
	    {
	      if (MRM_REGS (modrm) == RBP && MRM_REGD (modrm) == RSP)
		/* movl %esp,%ebp */
		cur->fp = cur->sp;
	      else if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{ /* mov %ebp,%esp */
		  cur->sp = cur->fp;
		  if (cur->sp - RED_ZONE > cur->sp_safe)
		    cur->sp_safe = cur->sp - RED_ZONE;
		  if (wctx->fp == (unsigned long) cur->sp)
		    cur->cval = RA_FROMFP;
		}
	    }
	  else if (MRM_MOD (modrm) == 0x80)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov %ebp,disp32(%esp) - JVM */
		      immv = read_int (cur->pc + 2, 4);
		      cur->fp_loc = (unsigned long*) ((char*) cur->sp + immv);
		      cur->fp_sav = cur->fp;
		    }
		}
	    }
	  else if (MRM_MOD (modrm) == 0x40)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RDX)
		{
		  if (cur->pc[1] == 0x24 && cur->pc[2] == 0x0)
		    { /* movl %edx,0(%esp) */
		      cur->ra_loc = cur->sp;
		      cur->ra_sav = cur->rdx;
		    }
		}
	      else if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov %ebp,disp8(%esp) - JVM */
		      imm8 = ((char*) (cur->pc))[2];
		      cur->fp_loc = (unsigned long*) ((char*) cur->sp + imm8);
		      cur->fp_sav = cur->fp;
		    }
		}
	    }
	  else if (MRM_MOD (modrm) == 0x0)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov %ebp,(%esp) */
		      cur->fp_loc = cur->sp;
		      cur->fp_sav = cur->fp;
		    }
		}
	      else if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RDX)
		{
		  if (cur->pc[1] == 0x24)
		    { /* movl %edx,(%esp) */
		      cur->ra_loc = cur->sp;
		      cur->ra_sav = cur->rdx;
		    }
		}
	    }
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8a: /* mov Gb,Eb */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8b: /* mov Gv,Ev */
	  modrm = *cur->pc;
	  if (MRM_MOD (modrm) == 0xc0)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		/* mov %esp,%ebp */
		cur->fp = cur->sp;
	      else if (MRM_REGS (modrm) == RBP && MRM_REGD (modrm) == RSP)
		{ /* mov %ebp,%esp */
		  cur->sp = cur->fp;
		  if (cur->sp - RED_ZONE > cur->sp_safe)
		    cur->sp_safe = cur->sp - RED_ZONE;
		  if (wctx->fp == (unsigned long) cur->sp)
		    cur->cval = RA_FROMFP;
		}
	    }
	  else if (MRM_MOD (modrm) == 0x80)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov disp32(%esp),%ebp */
		      immv = read_int (cur->pc + 2, 4);
		      unsigned long *ptr = (unsigned long*) ((char*) cur->sp + immv);
		      if (cur->fp_loc == ptr)
			{
			  cur->fp = cur->fp_sav;
			  cur->fp_loc = NULL;
			}
		      else if (ptr >= cur->sp_safe && (unsigned long) ptr < wctx->sbase)
			cur->fp = (unsigned long*) (*ptr);
		    }
		}
	    }
	  else if (MRM_MOD (modrm) == 0x40)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov disp8(%esp),%ebp - JVM */
		      imm8 = ((char*) (cur->pc))[2];
		      unsigned long *ptr = (unsigned long*) ((char*) cur->sp + imm8);
		      if (cur->fp_loc == ptr)
			{
			  cur->fp = cur->fp_sav;
			  cur->fp_loc = NULL;
			}
		      else if (ptr >= cur->sp_safe && (unsigned long) ptr < wctx->sbase)
			cur->fp = (unsigned long*) (*ptr);
		    }
		}
	    }
	  else if (MRM_MOD (modrm) == 0x0)
	    {
	      if (MRM_REGS (modrm) == RSP && MRM_REGD (modrm) == RBP)
		{
		  if (cur->pc[1] == 0x24)
		    { /* mov (%esp),%ebp */
		      if (cur->fp_loc == cur->sp)
			{
			  cur->fp = cur->fp_sav;
			  cur->fp_loc = NULL;
			}
		      else if (cur->sp >= cur->sp_safe &&
			       (unsigned long) cur->sp < wctx->sbase)
			cur->fp = (unsigned long*) *cur->sp;
		    }
		}
	    }
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8c: /* mov Mw,Sw */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8d: /* lea Gv,M */
	  modrm = *cur->pc;
	  if (MRM_REGD (modrm) == RSP)
	    {
	      unsigned char *pc = cur->pc;
	      // Mez: need to use always regs[RSP/RBP] instead cur->sp(or fp):
	      cur->regs[RSP] = (unsigned long) cur->sp;
	      cur->regs[RBP] = (unsigned long) cur->fp;
	      cur->pc++;
	      int mod = (modrm >> 6) & 3;
	      int r_m = modrm & 7;
	      long val = 0;
	      int undefRez = 0;
	      if (mod == 0x3)
		val = getRegVal (cur, MRM_REGS (modrm), &undefRez);
	      else if (r_m == 4)
		{ // SP or R12. Decode SIB-byte.
		  int sib = *cur->pc++;
		  int scale = 1 << (sib >> 6);
		  int index = X | ((sib >> 3) & 7);
		  int base = B | (sib & 7);
		  if (mod == 0)
		    {
		      if ((base & 7) == 5)
			{ // BP or R13
			  if (index != 4) // SP
			    val += getRegVal (cur, index, &undefRez) * scale;
			  val += read_int (cur->pc, 4);
			  cur->pc += 4;
			}
		      else
			{
			  val += getRegVal (cur, base, &undefRez);
			  if (index != 4) // SP
			    val += getRegVal (cur, index, &undefRez) * scale;
			}
		    }
		  else
		    {
		      val += getRegVal (cur, base, &undefRez);
		      if (index != 4) // SP
			val += getRegVal (cur, index, &undefRez) * scale;
		      if (mod == 1)
			{
			  val += read_int (cur->pc, 1);
			  cur->pc++;
			}
		      else
			{ // mod == 2
			  val += read_int (cur->pc, 4);
			  cur->pc += 4;
			}
		    }
		}
	      else if (mod == 0)
		{
		  if (r_m == 5)
		    { // BP or R13
		      val += read_int (cur->pc, 4);
		      cur->pc += 4;
		    }
		  else
		    val += getRegVal (cur, MRM_REGS (modrm), &undefRez);
		}
	      else
		{ // mod == 1 || mod == 2
		  val += getRegVal (cur, MRM_REGS (modrm), &undefRez);
		  if (mod == 1)
		    {
		      val += read_int (cur->pc, 1);
		      cur->pc++;
		    }
		  else
		    { // mod == 2
		      val += read_int (cur->pc, 4);
		      cur->pc += 4;
		    }
		}
	      if (undefRez)
		{
		  DprintfT (SP_DUMP_UNWIND, "stack_unwind%d cannot calculate RSP. cur->pc=0x%lx val=0x%lx\n",
			   __LINE__, (unsigned long) cur->pc, (unsigned long) val);
		  goto checkFP;
		}
	      cur->regs[MRM_REGD (modrm)] = val;
	      DprintfT (SP_DUMP_UNWIND, "stack_unwind%d cur->pc=0x%lx val=0x%lx wctx->sp=0x%lx wctx->sbase=0x%lx\n",
		       __LINE__, (unsigned long) cur->pc, (unsigned long) val,
		       (unsigned long) wctx->sp, (unsigned long) wctx->sbase);
	      if (cur->pc != check_modrm (pc))
		DprintfT (SP_DUMP_UNWIND, "stack_unwind%d ERROR: cur->pc=0x%lx != check_modrm(0x%lx)=0x%lx\n",
			 __LINE__, (unsigned long) cur->pc, (unsigned long) pc,
			 (unsigned long) check_modrm (pc));
	      if (MRM_REGD (modrm) == RSP)
		{
		  if (!isInside ((unsigned long) val, wctx->sp, wctx->sbase))
		    {
		      DprintfT (SP_DUMP_UNWIND, "stack_unwind%d cannot calculate RSP. cur->pc=0x%lx opcode=0x%02x val=0x%lx wctx->sp=0x%lx wctx->sbase=0x%lx\n",
			       __LINE__, (unsigned long) cur->pc, opcode, (unsigned long) val,
			       (unsigned long) wctx->sp, (unsigned long) wctx->sbase);
		      goto checkFP;
		    }
		  cur->sp = (unsigned long *) val;
		  if (cur->sp - RED_ZONE > cur->sp_safe)
		    cur->sp_safe = cur->sp - RED_ZONE;
		}
	    }
	  else
	    cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8e: /* mov Sw,Ew */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0x8f: /* pop Ev */
	  cur->pc = check_modrm (cur->pc);
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0x90: /* nop */
	  break;
	case 0x91: /* xchg %eax,%ecx */
	case 0x92: /* xchg %eax,%edx */
	case 0x93: /* xchg %eax,%ebx */
	case 0x94: /* xchg %eax,%esp XXXX */
	case 0x95: /* xchg %eax,%ebp XXXX */
	case 0x96: /* xchg %eax,%esi */
	case 0x97: /* xchg %eax,%edi */
	  break;
	case 0x98: /* cbw/cwde */
	case 0x99: /* cwd/cwq */
	  break;
	case 0x9a: /* callf Ap */
	  if (jmp_reg_switch_mode == 1)
	    {
	      struct AdvWalkContext* tmpctx = (struct AdvWalkContext *) alloca (sizeof (*cur));
	      __collector_memcpy (tmpctx, cur, sizeof (*cur));
	      int rc = process_return (wctx, tmpctx);
	      if (rc != RA_FAILURE)
		{
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		  return rc;
		}
	    }
	  cur->pc += 2 + a;
	  break;
	case 0x9b: /* fwait */
	case 0x9c: /* pushf Fv */
	case 0x9d: /* popf Fv */
	case 0x9e: /* sahf */
	case 0x9f: /* lahf */
	  break;
	case 0xa0: /* mov al,Ob */
	case 0xa1: /* mov eax,Ov */
	case 0xa2: /* mov Ob,al */
	case 0xa3: /* mov Ov,eax */
	  cur->pc += a;
	  break;
	case 0xa4: /* movsb Yb,Xb */
	case 0xa5: /* movsd Yv,Xv */
	case 0xa6: /* cmpsb Yb,Xb */
	case 0xa7: /* cmpsd Xv,Yv */
	  break;
	case 0xa8: /* test al,Ib */
	  cur->pc += 1;
	  break;
	case 0xa9: /* test eax,Iz */
	  cur->pc += z;
	  break;
	case 0xaa: /* stosb Yb,%al */
	case 0xab: /* stosd Yv,%eax */
	case 0xac: /* lodsb %al,Xb */
	case 0xad: /* lodsd %eax,Xv */
	case 0xae: /* scasb %al,Yb */
	case 0xaf: /* scasd %eax,Yv */
	  break;
	case 0xb0: /* mov %al,Ib */
	case 0xb1: /* mov %cl,Ib */
	case 0xb2: /* mov %dl,Ib */
	case 0xb3: /* mov %bl,Ib */
	case 0xb4: /* mov %ah,Ib */
	case 0xb5: /* mov %ch,Ib */
	case 0xb6: /* mov %dh,Ib */
	case 0xb7: /* mov %bh,Ib */
	  cur->pc += 1;
	  break;
	case 0xb8: /* mov Iv,%eax */
	case 0xb9: /* mov Iv,%ecx */
	case 0xba: /* mov Iv,%edx */
	case 0xbb: /* mov Iv,%ebx */
	case 0xbc: /* mov Iv,%esp */
	case 0xbd: /* mov Iv,%rbp */
	case 0xbe: /* mov Iv,%esi */
	case 0xbf: /* mov Iv,%edi */
	  reg = OPC_REG (opcode);
	  if (reg == RAX)
	    cur->rax = read_int (cur->pc, v);
	  cur->pc += v;
	  break;
	case 0xc0: /* group2 Eb,Ib */
	case 0xc1: /* group2 Ev,Ib */
	  cur->pc = check_modrm (cur->pc) + 1;
	  break;
	case 0xc2: /* ret Iw */
	  /* In the dynamic linker we may see that
	   * the actual return address is at sp+immv,
	   * while sp points to the resolved address.
	   */
	  {
	    immv = read_int (cur->pc, 2);
	    int rc = process_return (wctx, cur);
	    if (rc != RA_FAILURE)
	      {
		if (jmp_reg_switch_mode == 1)
		  {
		    DprintfT (SP_DUMP_UNWIND, "stack_unwind%d give up return address under jmp switch mode, opcode = 0xc2\n", __LINE__);
		    goto checkFP;
		  }
		wctx->sp += immv;
		if (save_ctx)
		  omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		return rc;
	      }
	    DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xc2.\n", __LINE__);
	    DELETE_CURCTX ();
	  }
	  break;
	case 0xc3: /* ret */
	  {
	    int rc = process_return (wctx, cur);
	    if (rc != RA_FAILURE)
	      {
		if (save_ctx)
		  omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		return rc;
	      }
	    if (jmp_reg_switch_mode == 1)
	      jmp_reg_switch_pc = cur->pc;
	    DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xc3.\n", __LINE__);
	    DELETE_CURCTX ();
	  }
	  break;
	case 0xc4: /* group AVX, 3-bytes VEX prefix */
	  {
	    unsigned char *pc = cur->pc - 1; // points to the beginning of the instruction
	    int len = parse_x86_AVX_instruction (pc);
	    if (len < 3)
	      DELETE_CURCTX ();
	    else
	      {
		pc += len;
		cur->pc = pc;
	      }
	  }
	  break;
	case 0xc5: /* group AVX, 2-bytes VEX prefix */
	  {
	    unsigned char *pc = cur->pc - 1; // points to the beginning of the instruction
	    int len = parse_x86_AVX_instruction (pc);
	    if (len < 2)
	      DELETE_CURCTX ();
	    else
	      {
		pc += len;
		cur->pc = pc;
	      }
	  }
	  break;
	case 0xc6:
	  modrm = *cur->pc;
	  if (modrm == 0xf8) /* xabort */
	    cur->pc += 2;
	  else /* mov Eb,Ib */
	    cur->pc = check_modrm (cur->pc) + 1;
	  break;
	case 0xc7:
	  modrm = *cur->pc;
	  if (modrm == 0xf8) /* xbegin */
	    cur->pc += v + 1;
	  else
	    { /* mov Ev,Iz */
	      extop = MRM_EXT (modrm);
	      if (extop != 0)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address, opcode = 0xc7\n", __LINE__);
		  goto checkFP;
		}
	      if (MRM_MOD (modrm) == 0xc0 && MRM_REGS (modrm) == RAX)
		cur->rax = read_int (cur->pc + 1, z);
	      cur->pc = check_modrm (cur->pc) + z;
	    }
	  break;
	case 0xc8: /* enter Iw,Ib */
	  cur->pc += 3;
	  break;
	case 0xc9: /* leave */
	  /* mov %ebp,%esp */
	  cur->sp = cur->fp;
	  /* pop %ebp */
	  if (cur->fp_loc == cur->sp)
	    {
	      cur->fp = cur->fp_sav;
	      cur->fp_loc = NULL;
	    }
	  else if (cur->sp >= cur->sp_safe &&
		   (unsigned long) cur->sp < wctx->sbase)
	    {
	      cur->fp = (unsigned long*) (*cur->sp);
	      if (wctx->fp == (unsigned long) cur->sp)
		cur->cval = RA_FROMFP;
	    }
	  cur->sp += 1;
	  if (cur->sp - RED_ZONE > cur->sp_safe)
	    cur->sp_safe = cur->sp - RED_ZONE;
	  break;
	case 0xca: /* retf Iw */
	  cur->pc += 2; /* XXXX process return */
	  break;
	case 0xcb: /* retf */
	  break; /* XXXX process return */
	case 0xcc: /* int 3 */
	  break;
	case 0xcd: /* int Ib */
	  if (*cur->pc == 0x80)
	    {
	      if (cur->rax == __NR_exit)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xcd.\n", __LINE__);
		  DELETE_CURCTX ();
		  break;
		}
	      else if (cur->rax == __NR_rt_sigreturn)
		{
		  if (jmp_reg_switch_mode == 1)
		    {
		      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address under jmp switch mode, opcode=0xcd\n",
				__LINE__);
		      goto checkFP;
		    }
		  wctx->sp = (unsigned long) cur->sp;
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_RT_SIGRETURN);
		  return RA_RT_SIGRETURN;
		}
#if WSIZE(32)
	      else if (cur->rax == __NR_sigreturn)
		{
		  if (jmp_reg_switch_mode == 1)
		    {
		      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address under jmp switch mode, opcode = 0xc2\n",
				__LINE__);
		      goto checkFP;
		    }
		  wctx->sp = (unsigned long) cur->sp;
		  if (save_ctx)
		    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_SIGRETURN);
		  return RA_SIGRETURN;
		}
#endif
	    }
	  cur->pc += 1;
	  break;
	case 0xce: /* into */
	case 0xcf: /* iret */
	  break;
	case 0xd0: /* shift group2 Eb,1 */
	case 0xd1: /* shift group2 Ev,1 */
	case 0xd2: /* shift group2 Eb,%cl */
	case 0xd3: /* shift group2 Ev,%cl */
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0xd4: /* aam Ib */
	  cur->pc += 1;
	  break;
	case 0xd5: /* aad Ib */
	  cur->pc += 1;
	  break;
	case 0xd6: /* falc? */
	  break;
	case 0xd7:
	  cur->pc = check_modrm (cur->pc);
	  cur->pc++;
	  break;
	case 0xd8: /* esc instructions */
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0xe0: /* loopne Jb */
	case 0xe1: /* loope Jb */
	case 0xe2: /* loop Jb */
	case 0xe3: /* jcxz Jb */
	  imm8 = *(char*) cur->pc++;
	  if (nctx < (jmp_reg_switch_mode ? MAXJMPREGCTX : MAXCTX))
	    {
	      int tidx = 0;
	      unsigned char *npc = cur->pc + imm8;
	      if (is_after_ret (npc))
		break;
	      while (npc > targets[tidx])
		tidx += 1;
	      if (npc != targets[tidx])
		{
		  if (ntrg < MAXTRGTS)
		    {
		      for (int i = 0; i < nctx; i++)
			if (buf[i].tidx >= tidx)
			  buf[i].tidx++;
		      /* insert a new target */
		      for (int i = ntrg; i > tidx; i--)
			targets[i] = targets[i - 1];
		      ntrg += 1;
		      targets[tidx++] = npc;
		    }
		  else
		    DprintfT (SP_DUMP_UNWIND, "unwind.c: ntrg = max\n");
		  struct AdvWalkContext *new = buf + nctx;
		  nctx += 1;
		  __collector_memcpy (new, cur, sizeof (*new));
		  new->pc = npc;
		  new->tidx = tidx;
		  cur = new; /* advance the new context first */
		  continue;
		}
	    }
	  else
	    DprintfT (SP_DUMP_UNWIND, "unwind.c: nctx = max\n");
	  break;
	case 0xe4: case 0xe5:
	  cur->pc = check_modrm (cur->pc);
	  cur->pc++;
	  break;
	case 0xe6: case 0xe7:
	  cur->pc++;
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0xec: case 0xed: case 0xee: case 0xef:
	  cur->pc = check_modrm (cur->pc);
	  break;
	case 0xe8: /* call Jz (f64) */
	  {
	    if (jmp_reg_switch_mode == 1)
	      {
		struct AdvWalkContext* tmpctx = (struct AdvWalkContext *) alloca (sizeof (*cur));
		__collector_memcpy (tmpctx, cur, sizeof (*cur));
		int rc = process_return (wctx, tmpctx);
		if (rc != RA_FAILURE)
		  {
		    if (save_ctx)
		      omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		    return rc;
		  }
	      }
	    int immz = read_int (cur->pc, z);
	    if (immz == 0)
	      /* special case in PIC code */
	      cur->sp -= 1;
	    cur->pc += z;
	  }
	  break;
	case 0xe9: /* jump Jz */
	  {
	    int immz = read_int (cur->pc, z);
	    unsigned char *npc = cur->pc + z + immz;
	    if ((unsigned long) npc < wctx->tbgn || (unsigned long) npc >= wctx->tend)
	      {
		DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xe9.\n", __LINE__);
		DELETE_CURCTX ();
		break;
	      }
	    int tidx = 0;
	    while (npc > targets[tidx])
	      tidx += 1;
	    if (npc != targets[tidx])
	      {
		if (ntrg < MAXTRGTS)
		  {
		    for (int i = 0; i < nctx; i++)
		      if (buf[i].tidx >= tidx)
			buf[i].tidx++;
		    /* insert a new target */
		    for (int i = ntrg; i > tidx; i--)
		      targets[i] = targets[i - 1];
		    ntrg += 1;
		    targets[tidx++] = npc;
		  }
		else
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: ntrg = max\n");
		cur->pc = npc;
		cur->tidx = tidx;
		continue; /* advance this context first */
	      }
	    else
	      {
		/* Delete context */
		DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xe9.\n", __LINE__);
		DELETE_CURCTX ();
	      }
	  }
	  break;
	case 0xeb: /* jump imm8 */
	  {
	    imm8 = *(char*) cur->pc++;
	    int tidx = 0;
	    unsigned char *npc = cur->pc + imm8;
	    while (npc > targets[tidx])
	      tidx += 1;
	    if (npc != targets[tidx])
	      {
		if (ntrg < MAXTRGTS)
		  {
		    for (int i = 0; i < nctx; i++)
		      if (buf[i].tidx >= tidx)
			buf[i].tidx++;
		    /* insert a new target */
		    for (int i = ntrg; i > tidx; i--)
		      targets[i] = targets[i - 1];
		    ntrg += 1;
		    targets[tidx++] = npc;
		  }
		else
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: ntrg = max\n");
		cur->pc = npc;
		cur->tidx = tidx;
		continue; /* advance this context first */
	      }
	    else
	      {
		/* Delete context */
		DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xeb.\n", __LINE__);
		DELETE_CURCTX ();
	      }
	  }
	  break;
	case 0xf0: /* lock prefix */
	case 0xf2: /* repne prefix */
	case 0xf3: /* repz prefix */
	  break;
	case 0xf4: /* hlt */
	  extop2 = *(cur->pc - 3);
	  if (extop2 == 0x90)
	    {
	      // 17851712 occasional SEGV in find_i386_ret_addr in unwind.c during attach
	      if (save_ctx)
		omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_END_OF_STACK);
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d returns RA_END_OF_STACK\n", __LINE__);
	      return RA_END_OF_STACK;
	    }
	  /* We see 'hlt' in _start. Stop analysis, revert to FP */
	  /* A workaround for the Linux main stack */
	  if (nctx > 1)
	    {
	      DELETE_CURCTX ();
	      break;
	    }
	  if (cur->fp == 0)
	    {
	      if (jmp_reg_switch_mode == 1)
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: give up return address under jmp switch mode, opcode = 0xf4\n");
		  goto checkFP;
		}
	      cache_put (wctx, RA_EOSTCK);
	      wctx->pc = 0;
	      wctx->sp = 0;
	      wctx->fp = 0;
	      if (save_ctx)
		omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_END_OF_STACK);
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d returns RA_END_OF_STACK\n", __LINE__);
	      return RA_END_OF_STACK;
	    }
	  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d give up return address, opcode = 0xf4\n", __LINE__);
	  goto checkFP;
	case 0xf5: /* cmc */
	  break;
	case 0xf6: /* group3 Eb */
	  modrm = *cur->pc;
	  extop = MRM_EXT (modrm);
	  cur->pc = check_modrm (cur->pc);
	  if (extop == 0x0) /* test Ib */
	    cur->pc += 1;
	  break;
	case 0xf7: /* group3 Ev */
	  modrm = *cur->pc;
	  extop = MRM_EXT (modrm);
	  cur->pc = check_modrm (cur->pc);
	  if (extop == 0x0)  /* test Iz */
	    cur->pc += z;
	  break;
	case 0xf8: /* clc */
	case 0xf9: /* stc */
	case 0xfa: /* cli */
	case 0xfb: /* sti */
	case 0xfc: /* cld */
	case 0xfd: /* std */
	  break;
	case 0xfe: /* group4 */
	  modrm = *cur->pc;
	  extop = MRM_EXT (modrm);
	  switch (extop)
	    {
	    case 0x0: /* inc Eb */
	    case 0x1: /* dec Eb */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x7:
	      cur->pc = check_modrm (cur->pc);
	      break;
	    default:
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d unknown opcode: 0xfe %x\n",
			__LINE__, extop);
	      DELETE_CURCTX ();
	      break;
	    }
	  break;
	case 0xff: /* group5 */
	  modrm = *cur->pc;
	  extop = MRM_EXT (modrm);
	  switch (extop)
	    {
	    case 0x0: /* inc Ev */
	    case 0x1: /* dec Ev */
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x2: /* calln Ev */
	      if (jmp_reg_switch_mode == 1)
		{
		  struct AdvWalkContext* tmpctx = (struct AdvWalkContext *) alloca (sizeof (*cur));
		  __collector_memcpy (tmpctx, cur, sizeof (*cur));
		  int rc = process_return (wctx, tmpctx);
		  if (rc != RA_FAILURE)
		    {
		      if (save_ctx)
			omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		      return rc;
		    }
		}
	      cur->pc = check_modrm (cur->pc);
	      break;
	    case 0x3: /* callf Ep */
	      if (jmp_reg_switch_mode == 1)
		{
		  struct AdvWalkContext* tmpctx = (struct AdvWalkContext *) alloca (sizeof (*cur));
		  __collector_memcpy (tmpctx, cur, sizeof (*cur));
		  int rc = process_return (wctx, tmpctx);
		  if (rc != RA_FAILURE)
		    {
		      if (save_ctx)
			omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		      return rc;
		    }
		}
	      cur->pc = check_modrm (cur->pc); /* XXXX */
	      break;
	    case 0x4: /* jumpn Ev */
	      /* This instruction appears in PLT or
	       * in tail call optimization.
	       * In both cases treat it as return.
	       * Save jump *(reg) - switch, etc, for later use when no ctx left
	       */
	      if (modrm == 0x25 || /* jumpn *disp32 */
		  MRM_MOD (modrm) == 0x40 || /* jumpn byte(reg) */
		  MRM_MOD (modrm) == 0x80) /* jumpn word(reg) */
		{
		  DprintfT (SP_DUMP_UNWIND, "unwind.c: PLT or tail call: %p\n", cur->pc - 1);
		  int rc = process_return (wctx, cur);
		  if (rc != RA_FAILURE)
		    {
		      if (jmp_reg_switch_mode == 1 && total_num_jmp_reg < max_num_jmp_reg_seen)
			{
			  DprintfT (SP_DUMP_UNWIND, "unwind.c: give up return address under jmp switch mode, opcode = 0xff\n");
			  goto checkFP;
			}
		      if (save_ctx)
			omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		      return rc;
		    }
		}
	      else if (modrm != 0x24 /*ignore SIB*/) /* jumpn *(reg) or jumpn reg */
		{
		  // 22846120 stack unwind does not find caller of __memcpy_ssse3_back with B64 intel-Linux
		  /*
		   * For now, let's deal rather narrowly with this scenario.  If:
		   * - we are in the middle of an "ff e2" instruction, and
		   * - the next instruction is undefined ( 0f 0b == ud2 )
		   * then test return.  (Might eventually have to broaden the scope
		   * of this fix to other registers/etc.)
		   */
		  if (cur->pc[0] == 0xe2 && cur->pc[1] == 0x0f && cur->pc[2] == 0x0b)
		    {
		      int rc = process_return_real (wctx, cur, 0);
		      if (rc == RA_SUCCESS)
			{
			  if (save_ctx)
			    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
			  return rc;
			}
		    }

		  // 22691241 shjsynprog, jsynprog core dump from find_i386_ret_addr
		  /*
		   * Here is another oddity.  Java 9 seems to emit dynamically generated
		   * code where a code block ends with a "jmp *reg" and then padding to a
		   * multiple-of-16 boundary and then a bunch of 0s.  In this case, let's
		   * not continue to walk bytes since we would be walking off the end of
		   * the instructions into ... something.  Treating them as instructions
		   * can lead to unexpected results, including SEGV.
		   */
		  /*
		   * While the general problem deserves a better solution, let's look
		   * here only for one particular case:
		   *    0xff 0xe7               jmp *reg
		   *                            nop to bring us to a multiple-of-16 boundary
		   *    0x0000000000000a00      something that does not look like an instruction
		   *
		   * A different nop might be used depending on how much padding is needed
		   * to reach that multiple-of-16 boundary.  We've seen two:
		   *    0x90                    one byte
		   *    0x0f 0x1f 0x40 0x00     four bytes
		   */
		  // confirm the instruction is 0xff 0xe7
		  if (cur->pc[0] == 0xe7)
		    {
		      // check for correct-length nop and find next 16-byte boundary
		      int found_nop = 0;
		      unsigned long long *boundary = 0;
		      switch ((((unsigned long) (cur->pc)) & 0xf))
			{
			case 0xb: // look for 4-byte nop
			  if (*((unsigned *) (cur->pc + 1)) == 0x00401f0f)
			    found_nop = 1;
			  boundary = (unsigned long long *) (cur->pc + 5);
			  break;
			case 0xe: // look for 1-byte nop
			  if (cur->pc[1] == 0x90)
			    found_nop = 1;
			  boundary = (unsigned long long *) (cur->pc + 2);
			  break;
			default:
			  break;
			}

		      // if nop is found, check what's at the boundary
		      if (found_nop && *boundary == 0x000000000a00)
			{
			  DELETE_CURCTX ();
			  break;
			}
		    }

		  DprintfT (SP_DUMP_UNWIND, "unwind.c: probably PLT or tail call or switch table: %p\n",
			    cur->pc - 1);
		  if (num_jmp_reg < expected_num_jmp_reg)
		    {
		      if (jmp_reg_ctx[num_jmp_reg] == NULL)
			jmp_reg_ctx[num_jmp_reg] = (struct AdvWalkContext *) alloca (sizeof (*cur));
		      if (jmp_reg_ctx[num_jmp_reg] != NULL)
			__collector_memcpy (jmp_reg_ctx[num_jmp_reg], cur, sizeof (*cur));
		    }
		  if (num_jmp_reg < expected_num_jmp_reg ||
		      (num_jmp_reg >= expected_num_jmp_reg &&
		       jmp_reg_ctx[expected_num_jmp_reg - 1] != NULL &&
		       cur->pc != jmp_reg_ctx[expected_num_jmp_reg - 1]->pc))
		    {
		      num_jmp_reg++;
		      total_num_jmp_reg++;
		    }
		  if (jmp_reg_switch_mode == 1 && total_num_jmp_reg >= max_num_jmp_reg_seen)
		    {
		      int rc = process_return_real (wctx, cur, 0);
		      if (rc == RA_SUCCESS)
			{
			  if (save_ctx)
			    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
			  return rc;
			}
		    }
		}
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d delete context, opcode 0xff.\n", __LINE__);
	      DELETE_CURCTX ();
	      break;
	    case 0x5: /* jmpf Ep */
	      cur->pc = check_modrm (cur->pc); /* XXXX */
	      break;
	    case 0x6: /* push Ev */
	      cur->pc = check_modrm (cur->pc);
	      cur->sp -= 1;
	      break;
	    case 0x7:
	      cur->pc = check_modrm (cur->pc); /* XXXX */
	      if (jmp_reg_switch_mode == 1)
		{
		  int rc = process_return_real (wctx, cur, 0);
		  if (rc == RA_SUCCESS)
		    {
		      if (save_ctx)
			omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, rc);
		      return rc;
		    }
		}
	      break;
	    default:
	      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d unknown opcode: 0xff %x\n",
			__LINE__, (int) extop);
	      DELETE_CURCTX ();
	      break;
	    }
	  break;
	default:
	  DprintfT (SP_DUMP_UNWIND, "unwind.c:%d unknown opcode: 0x%x\n",
		    __LINE__, (int) opcode);
	  DELETE_CURCTX ();
	  break;
	}

      /* switch to next context */
      if (++cur >= buf + nctx)
	cur = buf;
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d switch context: cur=0x%lx(%ld)  nctx=%d  cnt=%d\n",
	       __LINE__, (unsigned long) cur, (long) (cur - buf), (int) nctx, (int) cnt);
    }

checkFP:
  Tprintf (DBG_LT3, "find_i386_ret_addr:%d checkFP: wctx=0x%lx fp=0x%lx ln=0x%lx pc=0x%lx sbase=0x%lx sp=0x%lx tbgn=0x%lx tend=0x%lx\n",
	   __LINE__, (unsigned long) wctx, (unsigned long) wctx->fp,
	   (unsigned long) wctx->ln, (unsigned long) wctx->pc, (unsigned long) wctx->sbase,
	   (unsigned long) wctx->sp, (unsigned long) wctx->tbgn, (unsigned long) wctx->tend);

  if (jmp_reg_switch_mode == 1)
    { // not deal with switch cases not ending with ret
      if (jmp_reg_switch_backup_ctx != NULL)
	__collector_memcpy (cur, jmp_reg_switch_backup_ctx, sizeof (*cur));
      DprintfT (SP_DUMP_UNWIND, "stack_unwind jmp reg mode on: pc = 0x%lx cnt = %d, nctx = %d\n", wctx->pc, cnt, nctx);
    }

  unsigned long *cur_fp = cur->fp;
  unsigned long *cur_sp = cur->sp;
  if (do_walk == 0)
    __collector_memcpy (&wctx_pc_save, wctx, sizeof (struct WalkContext));

  /* Resort to the frame pointer */
  if (cur->fp_loc)
    cur->fp = cur->fp_sav;
  cur->sp = cur->fp;
  if ((unsigned long) cur->sp >= wctx->sbase ||
      (unsigned long) cur->sp < wctx->sp)
    {
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d do_walk=%d cur->sp=0x%p out of range. wctx->sbase=0x%lx wctx->sp=0x%lx wctx->pc=0x%lx\n",
		__LINE__, (int) do_walk, cur->sp, (unsigned long) wctx->sbase,
		(unsigned long) wctx->sp, (unsigned long) wctx->pc);
      if (do_walk == 0)
	{
	  cur->sp = cur_sp;
	  cur->fp = cur_fp;
	  do_walk = 1;
	  save_ctx = 1;
	  goto startWalk;
	}
      if (save_ctx)
	omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_FAILURE);
      return RA_FAILURE;
    }

  unsigned long fp = *cur->sp++;
  if (fp <= (unsigned long) cur->sp || fp >= wctx->sbase)
    {
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d fp=0x%016llx out of range. cur->sp=%p wctx->sbase=0x%lx wctx->pc=0x%lx\n",
	       __LINE__, (unsigned long long) fp, cur->sp,
	       (unsigned long) wctx->sbase, (unsigned long) wctx->pc);
      if (do_walk == 0)
	{
	  cur->sp = cur_sp;
	  cur->fp = cur_fp;
	  do_walk = 1;
	  save_ctx = 1;
	  goto startWalk;
	}
      if (save_ctx)
	omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_FAILURE);
      return RA_FAILURE;
    }

  unsigned long ra = *cur->sp++;
  if (ra == 0)
    {
      cache_put (wctx, RA_EOSTCK);
      DprintfT (SP_DUMP_UNWIND, "unwind.c:%d returns RA_END_OF_STACK wctx->pc = 0x%lx\n", __LINE__, wctx->pc);
      if (save_ctx)
	omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_END_OF_STACK);
      return RA_END_OF_STACK;
    }

  unsigned long tbgn = wctx->tbgn;
  unsigned long tend = wctx->tend;
  if (ra < tbgn || ra >= tend)
    {
      // We do not know yet if update_map_segments is really needed
      if (!__collector_check_segment (ra, &tbgn, &tend, 0))
	{
	  DprintfT (SP_DUMP_UNWIND, "unwind.c: __collector_check_segment fail. wctx->pc = 0x%lx\n", wctx->pc);
	  if (do_walk == 0)
	    {
	      cur->sp = cur_sp;
	      cur->fp = cur_fp;
	      do_walk = 1;
	      save_ctx = 1;
	      goto startWalk;
	    }
	  if (save_ctx)
	    omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_FAILURE);
	  return RA_FAILURE;
	}
    }

  unsigned long npc = adjust_ret_addr (ra, ra - tbgn, tend);
  if (npc == 0)
    {
      DprintfT (SP_DUMP_UNWIND, "unwind.c: adjust_ret_addr fail. wctx->pc = 0x%lx\n", wctx->pc);
      if (do_walk == 0)
	{
	  cur->sp = cur_sp;
	  cur->fp = cur_fp;
	  do_walk = 1;
	  save_ctx = 1;
	  goto startWalk;
	}
      if (save_ctx)
	omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_FAILURE);
      return RA_FAILURE;
    }
  wctx->pc = npc;
  wctx->sp = (unsigned long) cur->sp;
  wctx->fp = fp;
  wctx->tbgn = tbgn;
  wctx->tend = tend;

  if (save_ctx)
    {
      omp_cache_put (cur->sp_safe, &wctx_pc_save, wctx, RA_SUCCESS);
      DprintfT (SP_DUMP_UNWIND, "unwind.c: cache walk context. wctx_pc_save->pc = 0x%lx\n", wctx_pc_save.pc);
    }
  return RA_SUCCESS;
}

/*
 * We have the return address, but we would like to report to the user
 * the calling PC, which is the instruction immediately preceding the
 * return address.  Unfortunately, x86 instructions can have variable
 * length.  So we back up 8 bytes and try to figure out where the
 * calling PC starts.  (FWIW, call instructions are often 5-bytes long.)
 */
unsigned long
adjust_ret_addr (unsigned long ra, unsigned long segoff, unsigned long tend)
{
  unsigned long npc = 0;
  int i = segoff < 8 ? segoff : 8;
  for (; i > 1; i--)
    {
      unsigned char *ptr = (unsigned char*) ra - i;
      int z = 4;
      int a = 4;
      int done = 0;
      int bVal;
      while (!done)
	{
	  bVal = getByteInstruction (ptr);
	  if (bVal < 0)
	    return 0;
	  switch (bVal)
	    {
	    case 0x26:
	    case 0x36:
#if WSIZE(64)
	      ptr += 1;
	      break;
#endif
	    case 0x64:
	    case 0x65:
	      bVal = getByteInstruction (ptr + 1);
	      if (bVal < 0)
		return 0;
	      if (bVal == 0xe8)
		// a workaround for bug 16193041, assuming "call Jz" has no segment override prefix
	       done = 1;
	      else
		ptr += 1;
	      break;
	    case 0x66:
	      z = 2;
	      ptr += 1;
	      break;
	    case 0x67:
	      a = 2;
	      ptr += 1;
	      break;
	    default:
	      done = 1;
	      break;
	    }
	}
#if WSIZE(64)
      bVal = getByteInstruction (ptr);
      if (bVal < 0)
	return 0;
      if (bVal >= 0x40 && bVal <= 0x4f)
	{ /* XXXX not all REX codes applicable */
	  if (bVal & 0x8)
	    z = 4;
	  ptr += 1;
	}
#endif
      int opcode = getByteInstruction (ptr);
      if (opcode < 0)
	return 0;
      ptr++;
      switch (opcode)
	{
	case 0xe8: /* call Jz (f64) */
	  ptr += z;
	  break;
	case 0x9a: /* callf Ap */
	  ptr += 2 + a;
	  break;
	case 0xff: /* calln Ev , callf Ep */
	  {
	    int extop = MRM_EXT (*ptr);
	    if (extop == 2 || extop == 3)
	      ptr = check_modrm (ptr);
	  }
	  break;
	default:
	  continue;
	}
      if ((unsigned long) ptr == ra)
	{
	  npc = ra - i;
	  break;
	}
    }
  if (npc == 0)
    {
      unsigned char * ptr = (unsigned char *) ra;
#if WSIZE(32)
      // test __kernel_sigreturn or __kernel_rt_sigreturn
      if ((ra + 7 < tend && getByteInstruction (ptr) == 0x58
	   && getByteInstruction (ptr + 1) == 0xb8
	   && getByteInstruction (ptr + 6) == 0xcd
	   && getByteInstruction (ptr + 7) == 0x80) /* pop %eax; mov $NNNN, %eax; int */
	  || (ra + 7 < tend && getByteInstruction (ptr) == 0x58
	      && getByteInstruction (ptr + 1) == 0xb8
	      && getByteInstruction (ptr + 6) == 0x0f
	      && getByteInstruction (ptr + 7) == 0x05) /* pop %eax; mov $NNNN, %eax; syscall */
	  || (ra + 6 < tend && getByteInstruction (ptr) == 0xb8
	      && getByteInstruction (ptr + 5) == 0xcd
	      && getByteInstruction (ptr + 6) == 0x80) /* mov $NNNN, %eax; int */
	  || (ra + 6 < tend && getByteInstruction (ptr) == 0xb8
	      && getByteInstruction (ptr + 5) == 0x0f
	      && getByteInstruction (ptr + 6) == 0x05)) /* mov $NNNN, %eax; syscall */
#else //WSIZE(64)
      // test __restore_rt
      if (ra + 8 < tend && getByteInstruction (ptr) == 0x48
	  && getByteInstruction (ptr + 7) == 0x0f
	  && getByteInstruction (ptr + 8) == 0x05) /* mov $NNNNNNNN, %rax; syscall */
#endif
	{
	  npc = ra;
	}
    }
  if (npc == 0 && __collector_java_mode
      && __collector_java_asyncgetcalltrace_loaded)
    { // detect jvm interpreter code for java user threads
      unsigned char * ptr = (unsigned char *) ra;
#if WSIZE(32)
      // up to J170
      /*
       * ff 24 9d e0 64 02 f5    jmp     *-0xafd9b20(,%ebx,4)
       * 8b 4e 01                movl    1(%esi),%ecx
       * f7 d1                   notl    %ecx
       * 8b 5d ec                movl    -0x14(%ebp),%ebx
       * c1 e1 02                shll    $2,%ecx
       * eb d8                   jmp     .-0x26 [ 0x92a ]
       * 83 ec 08                subl    $8,%esp || 8b 65 f8                movl    -8(%ebp),%esp
       * */
      if (ra - 20 >= (ra - segoff) && ((*ptr == 0x83 && *(ptr + 1) == 0xec) || (*ptr == 0x8b && *(ptr + 1) == 0x65))
	  && *(ptr - 2) == 0xeb
	  && *(ptr - 5) == 0xc1 && *(ptr - 4) == 0xe1
	  && *(ptr - 8) == 0x8b && *(ptr - 7) == 0x5d
	  && *(ptr - 10) == 0xf7 && *(ptr - 9) == 0xd1
	  && *(ptr - 13) == 0x8b && *(ptr - 12) == 0x4e
	  && *(ptr - 20) == 0xff && *(ptr - 19) == 0x24 && *(ptr - 18) == 0x9d)
	{
	  npc = ra - 20;
	}
      // J180 J190
      // ff 24 9d ** ** ** **    jmp     *-0x*******(,%ebx,4)
      if (npc == 0
	  && ra - 7 >= (ra - segoff)
	  && *(ptr - 7) == 0xff
	  && *(ptr - 6) == 0x24
	  && *(ptr - 5) == 0x9d)
	{
	  npc = ra - 7;
	}
#else //WSIZE(64)
      // up to J170
      /*
       * 41 ff 24 da             jmp     *(%r10,%rbx,8)
       * 41 8b 4d 01             movl    1(%r13),%ecx
       * f7 d1                   notl    %ecx
       * 48 8b 5d d8             movq    -0x28(%rbp),%rbx
       * c1 e1 02                shll    $2,%ecx
       * eb cc                   jmp     .-0x32 [ 0xd23 ]
       * 48 8b 65 f0             movq    -0x10(%rbp),%rsp
       */
      if (ra - 19 >= (ra - segoff) && *ptr == 0x48 && ((*(ptr + 1) == 0x8b && *(ptr + 2) == 0x65) || (*(ptr + 1) == 0x83 && *(ptr + 2) == 0xec))
	  && *(ptr - 2) == 0xeb
	  && *(ptr - 5) == 0xc1 && *(ptr - 4) == 0xe1
	  && *(ptr - 9) == 0x48 && *(ptr - 8) == 0x8b && *(ptr - 7) == 0x5d
	  && *(ptr - 11) == 0xf7 && *(ptr - 10) == 0xd1
	  && *(ptr - 15) == 0x41 && *(ptr - 14) == 0x8b && *(ptr - 13) == 0x4d
	  && *(ptr - 19) == 0x41 && *(ptr - 18) == 0xff)
	npc = ra - 19;
      // J180 J190
      // 41 ff 24 da             jmp     *(%r10,%rbx,8)
      if (npc == 0
	  && ra - 4 >= (ra - segoff)
	  && *(ptr - 4) == 0x41
	  && *(ptr - 3) == 0xff
	  && *(ptr - 2) == 0x24
	  && *(ptr - 1) == 0xda)
	npc = ra - 4;
#endif
    }

  return npc;
}

/*
 * Parses AVX instruction and returns its length.
 * Returns 0 if parsing failed.
 * https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
 */
static int
parse_x86_AVX_instruction (unsigned char *pc)
{
  /*
   * VEX prefix has a two-byte form (0xc5) and a three byte form (0xc4).
   * If an instruction syntax can be encoded using the two-byte form,
   * it can also be encoded using the three byte form of VEX.
   * The latter increases the length of the instruction by one byte.
   * This may be helpful in some situations for code alignment.
   *
		     Byte 0           Byte 1              Byte 2         Byte 3
     (Bit Position) 7      0     7 6 5   4    0     7   6  3   2   10
     3-byte VEX   [ 11000100 ] [ R X B | m-mmmm ] [ W | vvvv | L | pp ]
		    7      0     7   6  3   2   10
     2-byte VEX   [ 11000101 ] [ R | vvvv | L | pp ]
		    7      0     7 6 5  4 3 2 1 0     7 6 5 4 3 2 1 0     7  6 5  4  3 2 1 0
     4-byte EVEX  [ 01100010 ] [ R X B R1 0 0 m m ] [ W v v v v 1 p p ] [ z L1 L B1 V1 a a a ]

     R: REX.R in 1's complement (inverted) form
	  0: Same as REX.R=1 (64-bit mode only)
	  1: Same as REX.R=0 (must be 1 in 32-bit mode)

     X: REX.X in 1's complement (inverted) form
	  0: Same as REX.X=1 (64-bit mode only)
	  1: Same as REX.X=0 (must be 1 in 32-bit mode)

     B: REX.B in 1's complement (inverted) form
	  0: Same as REX.B=1 (64-bit mode only)
	  1: Same as REX.B=0 (Ignored in 32-bit mode).

     W: opcode specific (use like REX.W, or used for opcode
	  extension, or ignored, depending on the opcode byte)

     m-mmmm:
	  00000: Reserved for future use (will #UD)
	  00001: implied 0F leading opcode byte
	  00010: implied 0F 38 leading opcode bytes
	  00011: implied 0F 3A leading opcode bytes
	  00100-11111: Reserved for future use (will #UD)

     vvvv: a register specifier (in 1's complement form) or 1111 if unused.

     L: Vector Length
	  0: scalar or 128-bit vector
	  1: 256-bit vector

     pp: opcode extension providing equivalent functionality of a SIMD prefix
	  00: None
	  01: 66
	  10: F3
	  11: F2
   *
   * Example: 0xc5f877L vzeroupper
   * VEX prefix: 0xc5 0x77
   * Opcode: 0xf8
   *
   */
  int len = 0;
  disassemble_info dis_info;
  dis_info.arch = bfd_arch_i386;
  dis_info.mach = bfd_mach_x86_64;
  dis_info.flavour = bfd_target_unknown_flavour;
  dis_info.endian = BFD_ENDIAN_UNKNOWN;
  dis_info.endian_code = dis_info.endian;
  dis_info.octets_per_byte = 1;
  dis_info.disassembler_needs_relocs = FALSE;
  dis_info.fprintf_func = fprintf_func;
  dis_info.fprintf_styled_func = fprintf_styled_func;
  dis_info.stream = NULL;
  dis_info.disassembler_options = NULL;
  dis_info.read_memory_func = read_memory_func;
  dis_info.memory_error_func = memory_error_func;
  dis_info.print_address_func = print_address_func;
  dis_info.symbol_at_address_func = symbol_at_address_func;
  dis_info.symbol_is_valid = symbol_is_valid;
  dis_info.display_endian = BFD_ENDIAN_UNKNOWN;
  dis_info.symtab = NULL;
  dis_info.symtab_size = 0;
  dis_info.buffer_vma = 0;
  dis_info.buffer = pc;
  dis_info.buffer_length = 8;

  disassembler_ftype disassemble = print_insn_i386;
  if (disassemble == NULL)
    {
      DprintfT (SP_DUMP_UNWIND, "parse_x86_AVX_instruction ERROR: unsupported disassemble\n");
      return 0;
    }
  len = disassemble (0, &dis_info);
  DprintfT (SP_DUMP_UNWIND, "parse_x86_AVX_instruction: returned %d  pc: %p\n", len, pc);
  return len;
}

/*
 * In the Intel world, a stack frame looks like this:
 *
 * %fp0->|                               |
 *       |-------------------------------|
 *       |  Args to next subroutine      |
 *       |-------------------------------|-\
 * %sp0->|  One word struct-ret address  | |
 *       |-------------------------------|  > minimum stack frame (8 bytes)
 *       |  Previous frame pointer (%fp0)| |
 * %fp1->|-------------------------------|-/
 *       |  Local variables              |
 * %sp1->|-------------------------------|
 *
 */

int
stack_unwind (char *buf, int size, void *bptr, void *eptr, ucontext_t *context, int mode)
{
  long *lbuf = (long*) buf;
  int lsize = size / sizeof (long);
  int ind = 0;
  int do_walk = 1;
  int extra_frame = 0;
  if (mode & FRINFO_NO_WALK)
    do_walk = 0;
  if ((mode & 0xffff) == FRINFO_FROM_STACK)
    extra_frame = 1;

  /*
   * trace the stack frames from user stack.
   * We are assuming that the frame pointer and return address
   * are null when we are at the top level.
   */
  struct WalkContext wctx;
  wctx.pc = GET_PC (context);
  wctx.sp = GET_SP (context);
  wctx.fp = GET_FP (context);
  wctx.ln = (unsigned long) context->uc_link;
  unsigned long *sbase = (unsigned long*) __collector_tsd_get_by_key (unwind_key);
  if (sbase && *sbase > wctx.sp)
    wctx.sbase = *sbase;
  else
    {
      wctx.sbase = wctx.sp + 0x100000;
      if (wctx.sbase < wctx.sp)  /* overflow */
	wctx.sbase = (unsigned long) - 1;
    }
  // We do not know yet if update_map_segments is really needed
  __collector_check_segment (wctx.pc, &wctx.tbgn, &wctx.tend, 0);

  for (;;)
    {
      if (ind >= lsize || wctx.pc == 0)
	break;
      if (bptr != NULL && extra_frame && wctx.sp <= (unsigned long) bptr && ind < 2)
	{
	  lbuf[0] = wctx.pc;
	  if (ind == 0)
	    {
	      ind++;
	      if (ind >= lsize)
		break;
	    }
	}
      if (bptr == NULL || wctx.sp > (unsigned long) bptr)
	{
	  lbuf[ind++] = wctx.pc;
	  if (ind >= lsize)
	    break;
	}

      for (;;)
	{
	  if (eptr != NULL && wctx.sp >= (unsigned long) eptr)
	    {
	      ind = ind >= 2 ? ind - 2 : 0;
	      goto exit;
	    }
	  int ret = find_i386_ret_addr (&wctx, do_walk);
	  DprintfT (SP_DUMP_UNWIND, "stack_unwind (x86 walk):%d find_i386_ret_addr returns %d\n", __LINE__, ret);
	  if (ret == RA_FAILURE)
	    {
	      /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
	      goto exit;
	    }

	  if (ret == RA_END_OF_STACK)
	    goto exit;
#if WSIZE(32)
	  if (ret == RA_RT_SIGRETURN)
	    {
	      struct SigFrame
	      {
		unsigned long arg0;
		unsigned long arg1;
		unsigned long arg2;
	      } *sframe = (struct SigFrame*) wctx.sp;
	      ucontext_t *ncontext = (ucontext_t*) sframe->arg2;
	      wctx.pc = GET_PC (ncontext);
	      if (!__collector_check_segment (wctx.pc, &wctx.tbgn, &wctx.tend, 0))
		{
		  /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
		  goto exit;
		}
	      unsigned long nsp = GET_SP (ncontext);
	      /* Check the new stack pointer */
	      if (nsp <= sframe->arg2 || nsp > sframe->arg2 + sizeof (ucontext_t) + 1024)
		{
		  /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
		  goto exit;
		}
	      wctx.sp = nsp;
	      wctx.fp = GET_FP (ncontext);
	      break;
	    }
	  else if (ret == RA_SIGRETURN)
	    {
	      struct sigcontext *sctx = (struct sigcontext*) wctx.sp;
	      wctx.pc = sctx->eip;
	      if (!__collector_check_segment (wctx.pc, &wctx.tbgn, &wctx.tend, 0))
		{
		  /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
		  goto exit;
		}
	      wctx.sp = sctx->esp;
	      wctx.fp = sctx->ebp;
	      break;
	    }
#elif WSIZE(64)
	  if (ret == RA_RT_SIGRETURN)
	    {
	      ucontext_t *ncontext = (ucontext_t*) wctx.sp;
	      wctx.pc = GET_PC (ncontext);
	      if (!__collector_check_segment (wctx.pc, &wctx.tbgn, &wctx.tend, 0))
		{
		  /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
		  goto exit;
		}
	      unsigned long nsp = GET_SP (ncontext);
	      /* Check the new stack pointer */
	      if (nsp <= wctx.sp || nsp > wctx.sp + sizeof (ucontext_t) + 1024)
		{
		  /* lbuf[ind++] = SP_FAILED_UNWIND_MARKER; */
		  goto exit;
		}
	      wctx.sp = nsp;
	      wctx.fp = GET_FP (ncontext);
	      break;
	    }
#endif /* WSIZE() */
	  if (bptr != NULL && extra_frame && wctx.sp <= (unsigned long) bptr && ind < 2)
	    {
	      lbuf[0] = wctx.pc;
	      if (ind == 0)
		{
		  ind++;
		  if (ind >= lsize)
		    break;
		}
	    }
	  if (bptr == NULL || wctx.sp > (unsigned long) bptr)
	    {
	      lbuf[ind++] = wctx.pc;
	      if (ind >= lsize)
		goto exit;
	    }
	}
    }

exit:
#if defined(DEBUG)
  if ((SP_DUMP_UNWIND & __collector_tracelevel) != 0)
    {
      DprintfT (SP_DUMP_UNWIND, "stack_unwind (x86 walk):%d found %d frames\n\n", __LINE__, ind);
      for (int i = 0; i < ind; i++)
	DprintfT (SP_DUMP_UNWIND, "  %3d:  0x%lx\n", i, (unsigned long) lbuf[i]);
    }
#endif
  dump_stack (__LINE__);
  if (ind >= lsize)
    {
      ind = lsize - 1;
      lbuf[ind++] = (unsigned long) SP_TRUNC_STACK_MARKER;
    }
  return ind * sizeof (long);
}

#elif ARCH(Aarch64)

static int
stack_unwind (char *buf, int size, void *bptr, void *eptr, ucontext_t *context, int mode)
{
  if (buf && bptr && eptr && context && size + mode > 0)
    getByteInstruction ((unsigned char *) eptr);
  int ind = 0;
  __u64 *lbuf = (void *) buf;
  int lsize = size / sizeof (__u64);
  __u64 pc = context->uc_mcontext.pc;
  __u64 sp = context->uc_mcontext.sp;
  __u64 stack_base;
  unsigned long tbgn = 0;
  unsigned long tend = 0;

  unsigned long *sbase = (unsigned long*) __collector_tsd_get_by_key (unwind_key);
  if (sbase && *sbase > sp)
    stack_base = *sbase;
  else
    {
      stack_base = sp + 0x100000;
      if (stack_base < sp)  // overflow
	stack_base = (__u64) -1;
    }
  DprintfT (SP_DUMP_UNWIND,
    "unwind.c:%d stack_unwind %2d pc=0x%llx  sp=0x%llx  stack_base=0x%llx\n",
    __LINE__, ind, (unsigned long long) pc, (unsigned long long) sp,
    (unsigned long long) stack_base);

  while (sp && pc)
  {
    DprintfT (SP_DUMP_UNWIND,
	"unwind.c:%d stack_unwind %2d pc=0x%llx  sp=0x%llx\n",
	__LINE__, ind, (unsigned long long) pc, (unsigned long long) sp);
//      Dl_info dlinfo;
//      if (!dladdr ((void *) pc, &dlinfo))
//	break;
//      DprintfT (SP_DUMP_UNWIND, "%2d: %llx <%s+%llu> (%s)\n",
//		ind, (unsigned long long) pc,
//		dlinfo.dli_sname ? dlinfo.dli_sname : "(?)",
//		(unsigned long long) pc - (unsigned long long) dlinfo.dli_saddr,
//		dlinfo.dli_fname);
      lbuf[ind++] = pc;
      if (ind >= lsize || sp >= stack_base || (sp & 15) != 0)
	break;
      if (pc < tbgn || pc >= tend)
	if (!__collector_check_segment ((unsigned long) pc, &tbgn, &tend, 0))
	  {
	    DprintfT (SP_DUMP_UNWIND,
		     "unwind.c:%d __collector_check_segment failed. sp=0x%lx\n",
		      __LINE__, (unsigned long) sp);
	    break;
	  }
      pc = ((__u64 *) sp)[1];
      __u64 old_sp = sp;
      sp = ((__u64 *) sp)[0];
      if (sp < old_sp)
	break;
    }
  if (ind >= lsize)
    {
      ind = lsize - 1;
      lbuf[ind++] = (__u64) SP_TRUNC_STACK_MARKER;
    }
  return ind * sizeof (__u64);
}
#endif /* ARCH() */
