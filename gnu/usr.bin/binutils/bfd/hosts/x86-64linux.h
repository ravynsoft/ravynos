/* Copyright (C) 2006-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* This is somewhat modelled after the file of the same name on SVR4
   systems.  It provides a definition of the core file format for ELF
   used on Linux.  It doesn't have anything to do with the /proc file
   system, even though Linux has one.

   Anyway, the whole purpose of this file is for GDB and GDB only.
   Don't read too much into it.  Don't use it for anything other than
   GDB unless you know what you are doing.  */

#include <features.h>
#include <sys/time.h>
#include <sys/types.h>

/* We define here only the symbols differing from their 64-bit variant.  */
#include <sys/procfs.h>
#include <stdint.h>

/* Unsigned 64-bit integer aligned to 8 bytes.  */
typedef uint64_t __attribute__ ((__aligned__ (8))) a8_uint64_t;

#undef HAVE_PRPSINFO32_T
#define HAVE_PRPSINFO32_T
#undef HAVE_PRPSINFO32_T_PR_PID
#define HAVE_PRPSINFO32_T_PR_PID

#undef HAVE_PRSTATUS32_T
#define HAVE_PRSTATUS32_T

/* These are the 32-bit x86 structures.  */

struct user_regs32_struct
{
  int32_t ebx;
  int32_t ecx;
  int32_t edx;
  int32_t esi;
  int32_t edi;
  int32_t ebp;
  int32_t eax;
  int32_t xds;
  int32_t xes;
  int32_t xfs;
  int32_t xgs;
  int32_t orig_eax;
  int32_t eip;
  int32_t xcs;
  int32_t eflags;
  int32_t esp;
  int32_t xss;
};

struct user_regs64_struct
{
  a8_uint64_t r15;
  a8_uint64_t r14;
  a8_uint64_t r13;
  a8_uint64_t r12;
  a8_uint64_t rbp;
  a8_uint64_t rbx;
  a8_uint64_t r11;
  a8_uint64_t r10;
  a8_uint64_t r9;
  a8_uint64_t r8;
  a8_uint64_t rax;
  a8_uint64_t rcx;
  a8_uint64_t rdx;
  a8_uint64_t rsi;
  a8_uint64_t rdi;
  a8_uint64_t orig_rax;
  a8_uint64_t rip;
  a8_uint64_t cs;
  a8_uint64_t eflags;
  a8_uint64_t rsp;
  a8_uint64_t ss;
  a8_uint64_t fs_base;
  a8_uint64_t gs_base;
  a8_uint64_t ds;
  a8_uint64_t es;
  a8_uint64_t fs;
  a8_uint64_t gs;
};

/* Type for a general-purpose register.  */
typedef uint32_t elf_greg32_t;
typedef a8_uint64_t elf_greg64_t;

/* And the whole bunch of them.  We could have used `struct
   user_regs_struct' directly in the typedef, but tradition says that
   the register set is an array, which does have some peculiar
   semantics, so leave it that way.  */
#define ELF_NGREG32 (sizeof (struct user_regs32_struct) / sizeof(elf_greg32_t))
typedef elf_greg32_t elf_gregset32_t[ELF_NGREG32];
#define ELF_NGREG64 (sizeof (struct user_regs64_struct) / sizeof(elf_greg64_t))
typedef elf_greg64_t elf_gregset64_t[ELF_NGREG64];

/* Definitions to generate Intel SVR4-like core files.  These mostly
   have the same names as the SVR4 types with "elf_" tacked on the
   front to prevent clashes with Linux definitions, and the typedef
   forms have been avoided.  This is mostly like the SVR4 structure,
   but more Linuxy, with things that Linux does not support and which
   GDB doesn't really use excluded.  */

struct prstatus32_timeval
  {
    int tv_sec;
    int tv_usec;
  };

struct prstatus64_timeval
  {
    a8_uint64_t tv_sec;
    a8_uint64_t tv_usec;
  };

struct elf_prstatus32
  {
    struct elf_siginfo pr_info;		/* Info associated with signal.  */
    short int pr_cursig;		/* Current signal.  */
    unsigned int pr_sigpend;		/* Set of pending signals.  */
    unsigned int pr_sighold;		/* Set of held signals.  */
    pid_t pr_pid;
    pid_t pr_ppid;
    pid_t pr_pgrp;
    pid_t pr_sid;
    struct prstatus32_timeval pr_utime;		/* User time.  */
    struct prstatus32_timeval pr_stime;		/* System time.  */
    struct prstatus32_timeval pr_cutime;	/* Cumulative user time.  */
    struct prstatus32_timeval pr_cstime;	/* Cumulative system time.  */
    elf_gregset32_t pr_reg;		/* GP registers.  */
    int pr_fpvalid;			/* True if math copro being used.  */
  };

struct elf_prstatusx32
  {
    struct elf_siginfo pr_info;		/* Info associated with signal.  */
    short int pr_cursig;		/* Current signal.  */
    unsigned int pr_sigpend;		/* Set of pending signals.  */
    unsigned int pr_sighold;		/* Set of held signals.  */
    pid_t pr_pid;
    pid_t pr_ppid;
    pid_t pr_pgrp;
    pid_t pr_sid;
    struct prstatus32_timeval pr_utime;		/* User time.  */
    struct prstatus32_timeval pr_stime;		/* System time.  */
    struct prstatus32_timeval pr_cutime;	/* Cumulative user time.  */
    struct prstatus32_timeval pr_cstime;	/* Cumulative system time.  */
    elf_gregset64_t pr_reg;		/* GP registers.  */
    int pr_fpvalid;			/* True if math copro being used.  */
  };

struct elf_prstatus64
  {
    struct elf_siginfo pr_info;	/* Info associated with signal.  */
    short int pr_cursig;		/* Current signal.  */
    a8_uint64_t pr_sigpend;		/* Set of pending signals.  */
    a8_uint64_t pr_sighold;		/* Set of held signals.  */
    pid_t pr_pid;
    pid_t pr_ppid;
    pid_t pr_pgrp;
    pid_t pr_sid;
    struct prstatus64_timeval pr_utime;		/* User time.  */
    struct prstatus64_timeval pr_stime;		/* System time.  */
    struct prstatus64_timeval pr_cutime;	/* Cumulative user time.  */
    struct prstatus64_timeval pr_cstime;	/* Cumulative system time.  */
    elf_gregset64_t pr_reg;		/* GP registers.  */
    int pr_fpvalid;			/* True if math copro being used.  */
  };

struct elf_prpsinfo32
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    unsigned int pr_flag;		/* Flags.  */
    unsigned short int pr_uid;
    unsigned short int pr_gid;
    int pr_pid, pr_ppid, pr_pgrp, pr_sid;
    /* Lots missing */
    char pr_fname[16];			/* Filename of executable.  */
    char pr_psargs[ELF_PRARGSZ];	/* Initial part of arg list.  */
  };

struct elf_prpsinfo64
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    a8_uint64_t pr_flag;		/* Flags.  */
    unsigned int pr_uid;
    unsigned int pr_gid;
    int pr_pid, pr_ppid, pr_pgrp, pr_sid;
    /* Lots missing */
    char pr_fname[16];			/* Filename of executable.  */
    char pr_psargs[ELF_PRARGSZ];	/* Initial part of arg list.  */
  };

/* The rest of this file provides the types for emulation of the
   Solaris <proc_service.h> interfaces that should be implemented by
   users of libthread_db.  */

/* Process status and info.  In the end we do provide typedefs for them.  */
typedef struct elf_prstatus32 prstatus32_t;
typedef struct elf_prstatusx32 prstatusx32_t;
typedef struct elf_prstatus64 prstatus64_t;
typedef struct elf_prpsinfo32 prpsinfo32_t;
typedef struct elf_prpsinfo64 prpsinfo64_t;
