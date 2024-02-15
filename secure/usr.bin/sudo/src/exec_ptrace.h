/*
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDO_EXEC_PTRACE_H
#define SUDO_EXEC_PTRACE_H

#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <asm/unistd.h>
#include <linux/audit.h>
#include <linux/ptrace.h>
#include <linux/seccomp.h>
#include <linux/filter.h>

/* Older kernel headers may be missing some EM_* defines in linux/elf.h. */
#include <elf.h>

/* Older systems may not support execveat(2). */
#ifndef __NR_execveat
# define __NR_execveat	-1
#endif

/* In case userland elf.h doesn't define NT_ARM_SYSTEM_CALL. */
#if defined(__aarch64__) && !defined(NT_ARM_SYSTEM_CALL)
# define NT_ARM_SYSTEM_CALL 0x404
#endif

/* In case userland doesn't define __X32_SYSCALL_BIT. */
#if defined(__x86_64__) && !defined(__X32_SYSCALL_BIT)
# define __X32_SYSCALL_BIT 0x40000000
#endif

#ifdef __mips__
# ifndef __NR_O32_Linux
#  define __NR_O32_Linux 4000
# endif
# ifndef __NR_N32_Linux
#  define __NR_N32_Linux 6000
# endif
#endif

/* Align address to a (compat) word boundary. */
#define WORDALIGN(_a, _r)	\
	(((_a) + ((unsigned long)(_r).wordsize - 1UL)) & ~((unsigned long)(_r).wordsize - 1UL))

/* Align pointer to a native word boundary. */
#define LONGALIGN(_p)	\
	(((unsigned long)(_p) + (sizeof(unsigned long) - 1UL)) & ~(sizeof(unsigned long) - 1UL))

/*
 * See syscall(2) for a list of registers used in system calls.
 * For example code, see tools/testing/selftests/seccomp/seccomp_bpf.c
 *
 * The structs and registers vary among the different platforms.
 * We define user_regs_struct as the struct to use for gettings
 * and setting the general registers and define accessor
 * macros to get/set the individual struct members.
 *
 * The value of SECCOMP_AUDIT_ARCH is used when matching the architecture
 * in the seccomp(2) filter.
 */
#if defined(__x86_64__)
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_X86_64
# ifndef __ILP32__
#  define X32_execve		__X32_SYSCALL_BIT + 520
#  define X32_execveat		__X32_SYSCALL_BIT + 545
# endif
# define sudo_pt_regs		struct user_regs_struct
# define reg_syscall(x)		(x).orig_rax
# define reg_retval(x)		(x).rax
# define reg_sp(x)		(x).rsp
# define reg_arg1(x)		(x).rdi
# define reg_arg2(x)		(x).rsi
# define reg_arg3(x)		(x).rdx
# define reg_arg4(x)		(x).r10
#elif defined(__aarch64__)
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_AARCH64
# define sudo_pt_regs		struct user_pt_regs
# define reg_syscall(x)		(x).regs[8]	/* w8 */
# define reg_retval(x)		(x).regs[0]	/* x0 */
# define reg_sp(x)		(x).sp		/* sp */
# define reg_arg1(x)		(x).regs[0]	/* x0 */
# define reg_arg2(x)		(x).regs[1]	/* x1 */
# define reg_arg3(x)		(x).regs[2]	/* x2 */
# define reg_arg4(x)		(x).regs[3]	/* x3 */
# define reg_set_syscall(_r, _nr) do {					\
    struct iovec _iov;							\
    long _syscallno = (_nr);						\
    _iov.iov_base = &_syscallno;					\
    _iov.iov_len = sizeof(_syscallno);					\
    ptrace(PTRACE_SETREGSET, pid, NT_ARM_SYSTEM_CALL, &_iov);		\
} while (0)
#elif defined(__arm__)
/* Note: assumes arm EABI, not OABI */
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_ARM
# define sudo_pt_regs		struct pt_regs
# define reg_syscall(x)		(x).ARM_r7
# define reg_retval(x)		(x).ARM_r0
# define reg_sp(x)		(x).ARM_sp
# define reg_arg1(x)		(x).ARM_r0
# define reg_arg2(x)		(x).ARM_r1
# define reg_arg3(x)		(x).ARM_r2
# define reg_arg4(x)		(x).ARM_r3
# define reg_set_syscall(_r, _nr) do {					\
    ptrace(PTRACE_SET_SYSCALL, pid, NULL, _nr);				\
} while (0)
#elif defined (__hppa__)
/* Untested (should also support hppa64) */
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_PARISC
# define sudo_pt_regs		struct user_regs_struct
# define reg_syscall(x)		(x).gr[20]	/* r20 */
# define reg_retval(x)		(x).gr[28]	/* r28 */
# define reg_sp(x)		(x).gr[30]	/* r30 */
# define reg_arg1(x)		(x).gr[26]	/* r26 */
# define reg_arg2(x)		(x).gr[25]	/* r25 */
# define reg_arg3(x)		(x).gr[24]	/* r24 */
# define reg_arg4(x)		(x).gr[23]	/* r23 */
#elif defined(__i386__)
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_I386
# define sudo_pt_regs		struct user_regs_struct
# define reg_syscall(x)		(x).orig_eax
# define reg_retval(x)		(x).eax
# define reg_sp(x)		(x).esp
# define reg_arg1(x)		(x).ebx
# define reg_arg2(x)		(x).ecx
# define reg_arg3(x)		(x).edx
# define reg_arg4(x)		(x).esi
#elif defined(__mips__)
# if _MIPS_SIM == _MIPS_SIM_ABI32
#  /* Linux o32 style syscalls, 4000-4999. */
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPSEL
#  else
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPS
#  endif
# elif _MIPS_SIM == _MIPS_SIM_ABI64
#  /* Linux 64-bit syscalls, 5000-5999. */
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPSEL64
#  else
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPS64
#  endif
# elif _MIPS_SIM == _MIPS_SIM_NABI32
# /* Linux N32 syscalls, 6000-6999. */
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPSEL64N32
#  else
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_MIPS64N32
#  endif
#  else
#   error "Unsupported MIPS ABI"
# endif
/*
 * If called via syscall(__NR_###), v0 holds __NR_O32_Linux and the real
 * syscall is in the first arg (a0).  The actual args are shifted by one.
 * MIPS does not support setting the syscall return value via ptrace.
 */
# define sudo_pt_regs		struct pt_regs
# define reg_syscall(_r) ({						\
    __u64 _nr;								\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	_nr = (_r).regs[4]; /* a0 */					\
    else								\
	_nr = (_r).regs[2]; /* v0 */					\
    _nr;								\
})
# define reg_retval(x)		(x).regs[2]	/* v0 */
# define reg_sp(x)		(x).regs[29]	/* sp */
# define reg_arg1(x)		\
    ((x).regs[2] == __NR_O32_Linux ? (x).regs[5] : (x).regs[4])
# define reg_set_arg1(_r, _v) do {					\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	(_r).regs[5] = _v; /* a1 */					\
    else								\
	(_r).regs[4] = _v; /* a0 */					\
} while (0)
# define reg_arg2(x)							\
    ((x).regs[2] == __NR_O32_Linux ? (x).regs[6] : (x).regs[5])
# define reg_set_arg2(_r, _v) do {					\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	(_r).regs[6] = _v; /* a2 */					\
    else								\
	(_r).regs[5] = _v; /* a1 */					\
} while (0)
# define reg_arg3(x)							\
    ((x).regs[2] == __NR_O32_Linux ? (x).regs[7] : (x).regs[6])
# define reg_set_arg3(_r, _v) do {					\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	(_r).regs[7] = _v; /* a3 */					\
    else								\
	(_r).regs[6] = _v; /* a2 */					\
} while (0)
/* XXX - reg_arg4 probably wrong for syscall() type calls on 032. */
# define reg_arg4(x)		\
    ((x).regs[2] == __NR_O32_Linux ? (x).regs[8] : (x).regs[7])
# define reg_set_arg4(_r, _v) do {					\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	(_r).regs[8] = _v; /* a4 */					\
    else								\
	(_r).regs[7] = _v; /* a3 */					\
} while (0)
# define reg_set_syscall(_r, _nr) do {					\
    if ((_r).regs[2] == __NR_O32_Linux)					\
	(_r).regs[4] = _nr; /* a0 */					\
    else								\
	(_r).regs[2] = _nr; /* v0 */					\
} while (0)
#elif defined(__powerpc__)
# if defined(__powerpc64__)
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_PPC64LE
#  else
#   define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_PPC64
#  endif
# else
#  define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_PPC
# endif
# define sudo_pt_regs		struct pt_regs
# define reg_syscall(x)		(x).gpr[0]	/* r0 */
# define reg_retval(x)		(x).gpr[3]	/* r3 */
# define reg_sp(x)		(x).gpr[1]	/* r1 */
# define reg_arg1(x)		(x).orig_gpr3	/* r3 */
# define reg_arg2(x)		(x).gpr[4]	/* r4 */
# define reg_arg3(x)		(x).gpr[5]	/* r5 */
# define reg_arg4(x)		(x).gpr[6]	/* r6 */
# define reg_set_retval(_r, _v) do {					\
    if (((_r).trap & 0xfff0) == 0x3000) {				\
	/* scv 0 system call, uses negative error code for result. */	\
	reg_retval(_r) = (_v);						\
    } else {								\
	/*								\
	 * Set CR0 SO bit to indicate a syscall error, which is stored	\
	 * as a positive error code.					\
	 */								\
	reg_retval(_r) = -(_v);						\
	(_r).ccr |= 0x10000000;						\
    }									\
} while (0)
#elif defined(__riscv) && __riscv_xlen == 64
# define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_RISCV64
# define sudo_pt_regs		struct user_regs_struct
# define reg_syscall(x)		(x).a7
# define reg_retval(x)		(x).a0
# define reg_sp(x)		(x).sp
# define reg_arg1(x)		(x).a0
# define reg_arg2(x)		(x).a1
# define reg_arg3(x)		(x).a2
# define reg_arg4(x)		(x).a3
#elif defined(__s390__)
/*
 * Both the syscall number and return value are stored in r2 for
 * the s390 ptrace API.  The first argument is stored in orig_gpr2.
 */
# if defined(__s390x__)
#  define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_S390X
# else
#  define SECCOMP_AUDIT_ARCH	AUDIT_ARCH_S390
# endif
# define sudo_pt_regs		s390_regs
# define reg_syscall(x)		(x).gprs[2]	/* r2 */
# define reg_retval(x)		(x).gprs[2]	/* r2 */
# define reg_sp(x)		(x).gprs[15]	/* r15 */
# define reg_arg1(x)		(x).orig_gpr2	/* r2 */
# define reg_arg2(x)		(x).gprs[3]	/* r3 */
# define reg_arg3(x)		(x).gprs[4]	/* r4 */
# define reg_arg4(x)		(x).gprs[5]	/* r6 */
#else
# error "Do not know how to find your architecture's registers"
#endif

/*
 * Compat definitions for running 32-bit binaries on 64-bit platforms.
 * We must define the register struct too since there is no way to
 * get it directly from the system headers.
 *
 * The value of SECCOMP_AUDIT_ARCH_COMPAT is used when matching the
 * architecture in the seccomp(2) filter.  We can tell when the compat
 * arch matched by inspecting the message returned by PTRACE_GETEVENTMSG.
 */
#if defined(__x86_64__)
struct i386_user_regs_struct {
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int ebp;
    unsigned int eax;
    unsigned int xds;
    unsigned int xes;
    unsigned int xfs;
    unsigned int xgs;
    unsigned int orig_eax;
    unsigned int eip;
    unsigned int xcs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int xss;
};
# define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_I386
# define COMPAT_execve			11
# define COMPAT_execveat		358
# define compat_sudo_pt_regs		struct i386_user_regs_struct
# define compat_reg_syscall(x)		(x).orig_eax
# define compat_reg_retval(x)		(x).eax
# define compat_reg_sp(x)		(x).esp
# define compat_reg_arg1(x)		(x).ebx
# define compat_reg_arg2(x)		(x).ecx
# define compat_reg_arg3(x)		(x).edx
# define compat_reg_arg4(x)		(x).esi
#elif defined(__aarch64__)
struct arm_pt_regs {
  unsigned int uregs[18];
};
# define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_ARM
# define COMPAT_execve			11
# define COMPAT_execveat		387
# define compat_sudo_pt_regs		struct arm_pt_regs
# define compat_reg_syscall(x)		(x).uregs[7]	/* r7 */
# define compat_reg_retval(x)		(x).uregs[0]	/* r0 */
# define compat_reg_sp(x)		(x).uregs[13]	/* r13 */
# define compat_reg_arg1(x)		(x).uregs[0]	/* r0 */
# define compat_reg_arg2(x)		(x).uregs[1]	/* r1 */
# define compat_reg_arg3(x)		(x).uregs[2]	/* r2 */
# define compat_reg_arg4(x)		(x).uregs[3]	/* r3 */
# define compat_reg_set_syscall(_r, _nr) reg_set_syscall(_r, _nr)
#elif defined(__mips__)
# if _MIPS_SIM == _MIPS_SIM_ABI64
/* MIPS o32/n32 binary compatibility on a mips64 system. */
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_MIPSEL
#   define SECCOMP_AUDIT_ARCH_COMPAT2	AUDIT_ARCH_MIPSEL64N32
#  else
#   define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_MIPS
#   define SECCOMP_AUDIT_ARCH_COMPAT2	AUDIT_ARCH_MIPS64N32
#  endif
#  define COMPAT_execve			__NR_O32_Linux + 11
#  define COMPAT_execveat		__NR_O32_Linux + 356
#  define COMPAT2_execve		__NR_N32_Linux + 57
#  define COMPAT2_execveat		__NR_N32_Linux + 320
# elif _MIPS_SIM == _MIPS_SIM_NABI32
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_MIPSEL
#  else
#   define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_MIPS
#  endif
#  define COMPAT_execve			__NR_O32_Linux + 11
#  define COMPAT_execveat		__NR_O32_Linux + 356
# endif /* _MIPS_SIM_ABI64 */
/* MIPS ABIs use a common ptrace interface. */
# define compat_sudo_pt_regs		struct pt_regs
# define compat_reg_syscall(x)		reg_syscall(x)
# define compat_reg_retval(x)		reg_retval(x)
# define compat_reg_sp(x)		reg_sp(x)
# define compat_reg_arg1(x)		reg_arg1(x)
# define compat_reg_set_arg1(_r, _v)	reg_set_arg1(_r, _v)
# define compat_reg_arg2(x)		reg_arg2(x)
# define compat_reg_set_arg2(_r, _v)	reg_set_arg2(_r, _v)
# define compat_reg_arg3(x)		reg_arg3(x)
# define compat_reg_set_arg3(_r, _v)	reg_set_arg3(_r, _v)
# define compat_reg_arg4(x)		reg_arg4(x)
# define compat_reg_set_arg4(_r, _v)	reg_set_arg4(_r, _v)
# define compat_reg_set_syscall(_r, _nr) reg_set_syscall(_r, _nr)
#elif defined(__powerpc64__)
struct ppc_pt_regs {
    unsigned int gpr[32];
    unsigned int nip;
    unsigned int msr;
    unsigned int orig_gpr3;
    unsigned int ctr;
    unsigned int link;
    unsigned int xer;
    unsigned int ccr;
    unsigned int mq;
    unsigned int trap;
    unsigned int dar;
    unsigned int dsisr;
    unsigned int result;
};
#  if BYTE_ORDER == LITTLE_ENDIAN
/* There is no AUDIT_ARCH_PPCLE define. */
#   define SECCOMP_AUDIT_ARCH_COMPAT	(AUDIT_ARCH_PPC|__AUDIT_ARCH_LE)
#  else
#   define SECCOMP_AUDIT_ARCH_COMPAT	AUDIT_ARCH_PPC
#  endif
# define COMPAT_execve			__NR_execve
# define COMPAT_execveat		__NR_execveat
# define compat_sudo_pt_regs		struct ppc_pt_regs
# define compat_reg_syscall(x)		(x).gpr[0]	/* r0 */
# define compat_reg_retval(x)		(x).gpr[3]	/* r3 */
# define compat_reg_sp(x)		(x).gpr[1]	/* r1 */
# define compat_reg_arg1(x)		(x).orig_gpr3	/* r3 */
# define compat_reg_arg2(x)		(x).gpr[4]	/* r4 */
# define compat_reg_arg3(x)		(x).gpr[5]	/* r5 */
# define compat_reg_arg4(x)		(x).gpr[6]	/* r6 */
# define compat_reg_set_retval(_r, _v)	reg_set_retval(_r, _v)
#endif

/* Set the syscall number the "normal" way by default. */
#ifndef reg_set_syscall
# define reg_set_syscall(_r, _nr) do {					\
    reg_syscall(_r) = (_nr);						\
} while (0)
#endif
#ifndef compat_reg_set_syscall
# define compat_reg_set_syscall(_r, _nr) do {				\
    compat_reg_syscall(_r) = (_nr);					\
} while (0)
#endif

/* Set the syscall return value the "normal" way by default. */
#ifndef reg_set_retval
# define reg_set_retval(_r, _v) do {					\
    reg_retval(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_retval
# define compat_reg_set_retval(_r, _v) do {				\
    compat_reg_retval(_r) = (_v);					\
} while (0)
#endif

/* Set the syscall arguments the "normal" way by default. */
#ifndef reg_set_arg1
# define reg_set_arg1(_r, _v) do {					\
    reg_arg1(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg1
# define compat_reg_set_arg1(_r, _v) do {				\
    compat_reg_arg1(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg2
# define reg_set_arg2(_r, _v) do {					\
    reg_arg2(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg2
# define compat_reg_set_arg2(_r, _v) do {				\
    compat_reg_arg2(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg3
# define reg_set_arg3(_r, _v) do {					\
    reg_arg3(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg3
# define compat_reg_set_arg3(_r, _v) do {				\
    compat_reg_arg3(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg4
# define reg_set_arg4(_r, _v) do {					\
    reg_arg4(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg4
# define compat_reg_set_arg4(_r, _v) do {				\
    compat_reg_arg4(_r) = (_v);						\
} while (0)
#endif

/* Set the syscall arguments the "normal" way by default. */
#ifndef reg_set_arg1
# define reg_set_arg1(_r, _v) do {					\
    reg_arg1(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg1
# define compat_reg_set_arg1(_r, _v) do {				\
    compat_reg_arg1(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg2
# define reg_set_arg2(_r, _v) do {					\
    reg_arg2(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg2
# define compat_reg_set_arg2(_r, _v) do {				\
    compat_reg_arg2(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg3
# define reg_set_arg3(_r, _v) do {					\
    reg_arg3(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg3
# define compat_reg_set_arg3(_r, _v) do {				\
    compat_reg_arg3(_r) = (_v);						\
} while (0)
#endif
#ifndef reg_set_arg4
# define reg_set_arg4(_r, _v) do {					\
    reg_arg4(_r) = (_v);						\
} while (0)
#endif
#ifndef compat_reg_set_arg4
# define compat_reg_set_arg4(_r, _v) do {				\
    compat_reg_arg4(_r) = (_v);						\
} while (0)
#endif

struct sudo_ptrace_regs {
    union {
	sudo_pt_regs native;
#ifdef SECCOMP_AUDIT_ARCH_COMPAT
	compat_sudo_pt_regs compat;
#endif
    } u;
    unsigned int wordsize;
    bool compat;
};

#endif /* SUDO_EXEC_PTRACE_H */
