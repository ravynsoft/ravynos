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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(HAVE_ENDIAN_H)
# include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
# include <sys/endian.h>
#elif defined(HAVE_MACHINE_ENDIAN_H)
# include <machine/endian.h>
#else
# include <compat/endian.h>
#endif

#include <sudo.h>
#include <sudo_exec.h>

#ifdef HAVE_PTRACE_INTERCEPT
# include <exec_intercept.h>
# include <exec_ptrace.h>

/* We need to take care when ptracing 32-bit binaries on 64-bit kernels. */
# ifdef __LP64__
#  define COMPAT_FLAG 0x01
# else
#  define COMPAT_FLAG 0x00
# endif

static int seccomp_trap_supported = -1;
#ifdef HAVE_PROCESS_VM_READV
static size_t page_size;
#endif
static size_t arg_max;

/* Register getters and setters. */
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
static inline unsigned long
get_stack_pointer(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_sp(regs->u.compat);
    } else {
	return reg_sp(regs->u.native);
    }
}

static inline void
set_sc_retval(struct sudo_ptrace_regs *regs, int retval)
{
    if (regs->compat) {
	compat_reg_set_retval(regs->u.compat, retval);
    } else {
	reg_set_retval(regs->u.native, retval);
    }
}

static inline int
get_syscallno(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_syscall(regs->u.compat);
    } else {
	return reg_syscall(regs->u.native);
    }
}

static inline void
set_syscallno(pid_t pid, struct sudo_ptrace_regs *regs, int syscallno)
{
    if (regs->compat) {
	compat_reg_set_syscall(regs->u.compat, syscallno);
    } else {
	reg_set_syscall(regs->u.native, syscallno);
    }
}

static inline unsigned long
get_sc_arg1(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_arg1(regs->u.compat);
    } else {
	return reg_arg1(regs->u.native);
    }
}

static inline void
set_sc_arg1(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    if (regs->compat) {
	compat_reg_set_arg1(regs->u.compat, addr);
    } else {
	reg_set_arg1(regs->u.native, addr);
    }
}

static inline unsigned long
get_sc_arg2(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_arg2(regs->u.compat);
    } else {
	return reg_arg2(regs->u.native);
    }
}

static inline void
set_sc_arg2(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    if (regs->compat) {
	compat_reg_set_arg2(regs->u.compat, addr);
    } else {
	reg_set_arg2(regs->u.native, addr);
    }
}

static inline unsigned long
get_sc_arg3(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_arg3(regs->u.compat);
    } else {
	return reg_arg3(regs->u.native);
    }
}

#  ifdef notyet
static inline void
set_sc_arg3(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    if (regs->compat) {
	compat_reg_set_arg3(regs->u.compat, addr);
    } else {
	reg_set_arg3(regs->u.native, addr);
    }
}

static inline unsigned long
get_sc_arg4(struct sudo_ptrace_regs *regs)
{
    if (regs->compat) {
	return compat_reg_arg4(regs->u.compat);
    } else {
	return reg_arg4(regs->u.native);
    }
}

static inline void
set_sc_arg4(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    if (regs->compat) {
	compat_reg_set_arg4(regs->u.compat, addr);
    } else {
	reg_set_arg4(regs->u.native, addr);
    }
}
#  endif /* notyet */

# else /* SECCOMP_AUDIT_ARCH_COMPAT */

static inline unsigned long
get_stack_pointer(struct sudo_ptrace_regs *regs)
{
    return reg_sp(regs->u.native);
}

static inline void
set_sc_retval(struct sudo_ptrace_regs *regs, int retval)
{
    reg_set_retval(regs->u.native, retval);
}

static inline int
get_syscallno(struct sudo_ptrace_regs *regs)
{
    return reg_syscall(regs->u.native);
}

static inline void
set_syscallno(pid_t pid, struct sudo_ptrace_regs *regs, int syscallno)
{
    reg_set_syscall(regs->u.native, syscallno);
}

static inline unsigned long
get_sc_arg1(struct sudo_ptrace_regs *regs)
{
    return reg_arg1(regs->u.native);
}

static inline void
set_sc_arg1(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    reg_set_arg1(regs->u.native, addr);
}

static inline unsigned long
get_sc_arg2(struct sudo_ptrace_regs *regs)
{
    return reg_arg2(regs->u.native);
}

static inline void
set_sc_arg2(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    reg_set_arg2(regs->u.native, addr);
}

static inline unsigned long
get_sc_arg3(struct sudo_ptrace_regs *regs)
{
    return reg_arg3(regs->u.native);
}

#  ifdef notyet
static inline void
set_sc_arg3(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    reg_set_arg3(regs->u.native, addr);
}

static inline unsigned long
get_sc_arg4(struct sudo_ptrace_regs *regs)
{
    return reg_arg4(regs->u.native);
}

static inline void
set_sc_arg4(struct sudo_ptrace_regs *regs, unsigned long addr)
{
    reg_set_arg4(regs->u.native, addr);
}
#  endif /* notyet */
# endif /* SECCOMP_AUDIT_ARCH_COMPAT */

/*
 * Get the registers for the given process and store in regs, which
 * must be large enough.  If the compat flag is set, pid is expected
 * to refer to a 32-bit process and the md parameters will be filled
 * in accordingly.
 * Returns true on success, else false.
 */
static bool
ptrace_getregs(int pid, struct sudo_ptrace_regs *regs, int compat)
{
    struct iovec iov;
    debug_decl(ptrace_getregs, SUDO_DEBUG_EXEC);

    iov.iov_base = &regs->u;
    iov.iov_len = sizeof(regs->u);

# ifdef __mips__
    /* PTRACE_GETREGSET has bugs with the MIPS o32 ABI at least. */
    if (ptrace(PTRACE_GETREGS, pid, NULL, iov.iov_base) == -1)
	debug_return_bool(false);
# else
    if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
	debug_return_bool(false);
# endif /* __mips__ */
    if (compat == -1) {
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
	if (sizeof(regs->u.native) != sizeof(regs->u.compat)) {
	    /* Guess compat based on size of register struct returned. */
	    compat = iov.iov_len != sizeof(regs->u.native);
	} else {
	    /* Assume a 64-bit executable will have a 64-bit stack pointer. */
	    compat = reg_sp(regs->u.native) < 0xffffffff;
	}
# else
	compat = false;
# endif /* SECCOMP_AUDIT_ARCH_COMPAT */
    }

    /* Machine-dependent parameters to support compat binaries. */
    if (compat) {
	regs->compat = true;
	regs->wordsize = sizeof(int);
    } else {
	regs->compat = false;
	regs->wordsize = sizeof(long);
    }

    debug_return_bool(true);
}

/*
 * Set the registers, specified by regs, for the given process.
 * Returns true on success, else false.
 */
static bool
ptrace_setregs(int pid, struct sudo_ptrace_regs *regs)
{
    debug_decl(ptrace_setregs, SUDO_DEBUG_EXEC);

# ifdef __mips__
    /* PTRACE_SETREGSET has bugs with the MIPS o32 ABI at least. */
    if (ptrace(PTRACE_SETREGS, pid, NULL, &regs->u) == -1)
	debug_return_bool(false);
# else
    struct iovec iov;
    iov.iov_base = &regs->u;
    iov.iov_len = sizeof(regs->u);
    if (ptrace(PTRACE_SETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
	debug_return_bool(false);
# endif /* __mips__ */

    debug_return_bool(true);
}

#ifdef HAVE_PROCESS_VM_READV
/*
 * Read the string at addr and store in buf using process_vm_readv(2).
 * Returns the number of bytes stored, including the NUL.
 */
static ssize_t
ptrace_readv_string(pid_t pid, unsigned long addr, char *buf, size_t bufsize)
{
    const char *cp, *buf0 = buf;
    struct iovec local, remote;
    ssize_t nread;
    debug_decl(ptrace_read_string, SUDO_DEBUG_EXEC);

    /*
     * Read the string via process_vm_readv(2) one page at a time.
     * We could do larger reads but since we don't know the length
     * of the string, going one page at a time is simplest.
     */
    for (;;) {
	if (bufsize == 0) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: %d: out of space reading string", __func__, (int)pid);
	    errno = ENOSPC;
	    debug_return_ssize_t(-1);
	}

	local.iov_base = buf;
	local.iov_len = bufsize;
	remote.iov_base = (void *)addr;
	remote.iov_len = MIN(bufsize, page_size);

	nread = process_vm_readv(pid, &local, 1, &remote, 1, 0);
	switch (nread) {
	case -1:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"process_vm_readv(%d, [0x%lx, %zu], 1, [0x%lx, %zu], 1, 0)",
		(int)pid, (unsigned long)local.iov_base, local.iov_len,
		(unsigned long)remote.iov_base, remote.iov_len);
	    debug_return_ssize_t(-1);
	case 0:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"process_vm_readv(%d, [0x%lx, %zu], 1, [0x%lx, %zu], 1, 0): %s",
		(int)pid, (unsigned long)local.iov_base, local.iov_len,
		(unsigned long)remote.iov_base, remote.iov_len, "premature EOF");
	    debug_return_ssize_t(-1);
	default:
	    /* Check for NUL terminator in page. */
	    cp = memchr(buf, '\0', (size_t)nread);
	    if (cp != NULL)
		debug_return_ssize_t((cp - buf0) + 1);	/* includes NUL */
	    buf += nread;
	    bufsize -= (size_t)nread;
	    addr += sizeof(unsigned long);
	    break;
	}
    }
    debug_return_ssize_t(-1);
}
#endif /* HAVE_PROCESS_VM_READV */

/*
 * Read the string at addr and store in buf using ptrace(2).
 * Returns the number of bytes stored, including the NUL.
 */
static ssize_t
ptrace_read_string(pid_t pid, unsigned long addr, char *buf, size_t bufsize)
{
    const char *cp, *buf0 = buf;
    unsigned long word;
    size_t i;
    debug_decl(ptrace_read_string, SUDO_DEBUG_EXEC);

#ifdef HAVE_PROCESS_VM_READV
    ssize_t nread = ptrace_readv_string(pid, addr, buf, bufsize);
    if (nread != -1 || errno != ENOSYS)
	debug_return_ssize_t(nread);
#endif /* HAVE_PROCESS_VM_READV */

    /*
     * Read the string via ptrace(2) one (native) word at a time.
     * We use the native word size even in compat mode because that
     * is the unit ptrace(2) uses.
     */
    for (;;) {
	word = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
	if (word == (unsigned long)-1) {
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"ptrace(PTRACE_PEEKDATA, %d, 0x%lx, NULL)", (int)pid, addr);
	    debug_return_ssize_t(-1);
	}

	cp = (char *)&word;
	for (i = 0; i < sizeof(unsigned long); i++) {
	    if (bufsize == 0) {
		sudo_debug_printf(SUDO_DEBUG_ERROR,
		    "%s: %d: out of space reading string", __func__, (int)pid);
		errno = ENOSPC;
		debug_return_ssize_t(-1);
	    }
	    *buf = cp[i];
	    if (*buf++ == '\0')
		debug_return_ssize_t(buf - buf0);
	    bufsize--;
	}
	addr += sizeof(unsigned long);
    }
}

/*
 * Expand buf by doubling its size.
 * Updates bufp and bufsizep and recalculates curp and remp if non-NULL.
 * Returns true on success, else false.
 */
static bool
growbuf(char **bufp, size_t *bufsizep, char **curp, size_t *remp)
{
    const size_t oldsize = *bufsizep;
    char *newbuf;
    debug_decl(growbuf, SUDO_DEBUG_EXEC);

    /* Double the size of the buffer. */
    newbuf = reallocarray(*bufp, 2, oldsize);
    if (newbuf == NULL) {
       sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
       debug_return_bool(false);
    }
    if (curp != NULL)
	*curp = newbuf + (*curp - *bufp);
    if (remp != NULL)
	*remp += oldsize;
    *bufp = newbuf;
    *bufsizep = 2 * oldsize;
    debug_return_bool(true);
}

/*
 * Build a NULL-terminated string vector from a string table.
 * On success, returns number of bytes used for the vector and sets
 * vecp to the start of the vector and countp to the number of elements
 * (not including the NULL).  The buffer is resized as needed.
 * Both vecp and its elements are stored as offsets into buf, not pointers.
 * However, NULL is still stored as NULL.
 * Returns (size_t)-1 on failure.
 */
static ssize_t
strtab_to_vec(char *strtab, size_t strtab_len, int *countp, char ***vecp,
    char **bufp, size_t *bufsizep, size_t remainder)
{
    char *strend = strtab + strtab_len;
    char **vec, **vp;
    int count = 0;
    debug_decl(strtab_to_vec, SUDO_DEBUG_EXEC);

    /* Store vector in buf after string table and make it aligned. */
    while (remainder < 2 * sizeof(char *)) {
	if (!growbuf(bufp, bufsizep, &strtab, &remainder))
	    debug_return_ssize_t(-1);
	strend = strtab + strtab_len;
    }
    vec = (char **)LONGALIGN(strend);
    remainder -= (size_t)((char *)vec - strend);

    /* Fill in vector with the strings we read. */
    for (vp = vec; strtab < strend; ) {
	while (remainder < 2 * sizeof(char *)) {
	    if (!growbuf(bufp, bufsizep, &strtab, &remainder))
		debug_return_ssize_t(-1);
	    strend = strtab + strtab_len;
	    vec = (char **)LONGALIGN(strend);
	    vp = vec + count;
	}
	/* Store offset into buf (not a pointer) in case of realloc(). */
	*vp++ = (char *)(strtab - *bufp);
	remainder -= sizeof(char *);
	strtab = memchr(strtab, '\0', (size_t)(strend - strtab));
	if (strtab == NULL)
	    break;
	strtab++;
	count++;
    }
    *vp++ = NULL;		/* we always leave room for NULL */

    *countp = count;
    *vecp = (char **)((char *)vec - *bufp);

    debug_return_ssize_t((char *)vp - strend);
}

/*
 * Read the string vector at addr and store it in bufp, which
 * is reallocated as needed.  The actual vector is returned in vecp.
 * The count stored in countp does not include the terminating NULL pointer.
 * The vecp and its contents are _offsets_, not pointers, in case the buffer
 * gets reallocated later.  The caller is responsible for converting the
 * offsets into pointers based on the buffer before using.
 * Returns the number of bytes in buf consumed (including NULs).
 */
static ssize_t
ptrace_read_vec(pid_t pid, struct sudo_ptrace_regs *regs, unsigned long addr,
    int *countp, char ***vecp, char **bufp, size_t *bufsizep, size_t off)
{
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
    unsigned long next_word = (unsigned long)-1;
# endif
    size_t strtab_len, remainder = *bufsizep - off;
    char *strtab = *bufp + off;
    unsigned long word;
    ssize_t len;
    debug_decl(ptrace_read_vec, SUDO_DEBUG_EXEC);

    /* Treat a NULL vector as empty, thanks Linux. */
    if (addr == 0) {
	char **vp;

	while (remainder < 2 * sizeof(char *)) {
	    if (!growbuf(bufp, bufsizep, &strtab, &remainder))
		debug_return_ssize_t(-1);
	}
	vp = (char **)LONGALIGN(strtab);
	*vecp = (char **)((char *)vp - *bufp);
	*countp = 0;
	*vp++ = NULL;
	debug_return_ssize_t((char *)vp - strtab);
    }

    /* Fill in string table. */
    do {
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
	if (next_word == (unsigned long)-1) {
	    word = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
	    if (regs->compat) {
		/* Stash the next compat word in next_word. */
#  if BYTE_ORDER == BIG_ENDIAN
		next_word = word & 0xffffffffU;
		word >>= 32;
#  else
		next_word = word >> 32;
		word &= 0xffffffffU;
#  endif
	    }
	} else {
	    /* Use the stashed value of the next word. */
	    word = next_word;
	    next_word = (unsigned long)-1;
	}
# else /* SECCOMP_AUDIT_ARCH_COMPAT */
	word = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
# endif /* SECCOMP_AUDIT_ARCH_COMPAT */
	switch (word) {
	case -1:
	    sudo_warn("%s: ptrace(PTRACE_PEEKDATA, %d, 0x%lx, NULL)",
		__func__, (int)pid, addr);
	    debug_return_ssize_t(-1);
	case 0:
	    /* NULL terminator */
	    break;
	default:
	    for (;;) {
		len = ptrace_read_string(pid, word, strtab, remainder);
		if (len != -1)
		    break;
		if (errno != ENOSPC)
		    debug_return_ssize_t(-1);
		if (!growbuf(bufp, bufsizep, &strtab, &remainder))
		    debug_return_ssize_t(-1);
	    }
	    strtab += len;
	    remainder -= (size_t)len;
	    addr += regs->wordsize;
	    continue;
	}
    } while (word != 0);

    /* Store strings in a vector after the string table. */
    strtab_len = (size_t)(strtab - (*bufp + off));
    strtab = *bufp + off;
    len = strtab_to_vec(strtab, strtab_len, countp, vecp, bufp, bufsizep,
	remainder);
    if (len == -1)
	debug_return_ssize_t(-1);

    debug_return_ssize_t((ssize_t)strtab_len + len);
}

#ifdef HAVE_PROCESS_VM_READV
/*
 * Write the NUL-terminated string str to addr in the tracee using
 * process_vm_writev(2).
 * Returns the number of bytes written, including trailing NUL.
 */
static ssize_t
ptrace_writev_string(pid_t pid, unsigned long addr, const char *str0)
{
    const char *str = str0;
    size_t len = strlen(str) + 1;
    debug_decl(ptrace_writev_string, SUDO_DEBUG_EXEC);

    /*
     * Write the string via process_vm_writev(2), handling partial writes.
     */
    for (;;) {
	struct iovec local, remote;
	ssize_t nwritten;

	local.iov_base = (void *)str;
	local.iov_len = len;
	remote.iov_base = (void *)addr;
	remote.iov_len = len;

	nwritten = process_vm_writev(pid, &local, 1, &remote, 1, 0);
	switch (nwritten) {
	case -1:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"process_vm_writev(%d, [0x%lx, %zu], 1, [0x%lx, %zu], 1, 0)",
		(int)pid, (unsigned long)local.iov_base, local.iov_len,
		(unsigned long)remote.iov_base, remote.iov_len);
	    debug_return_ssize_t(-1);
	case 0:
	    /* Should not be possible. */
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"process_vm_writev(%d, [0x%lx, %zu], 1, [0x%lx, %zu], 1, 0): %s",
		(int)pid, (unsigned long)local.iov_base, local.iov_len,
		(unsigned long)remote.iov_base, remote.iov_len,
		"zero bytes written");
	    debug_return_ssize_t(-1);
	default:
	    str += nwritten;
	    len -= (size_t)nwritten;
	    addr += (size_t)nwritten;
	    if (len == 0)
		debug_return_ssize_t(str - str0);	/* includes NUL */
	    break;
	}
    }
    debug_return_ssize_t(-1);
}
#endif /* HAVE_PROCESS_VM_READV */

/*
 * Write the NUL-terminated string str to addr in the tracee using ptrace(2).
 * Returns the number of bytes written, including trailing NUL.
 */
static ssize_t
ptrace_write_string(pid_t pid, unsigned long addr, const char *str)
{
    const char *str0 = str;
    size_t i;
    union {
	unsigned long word;
	char buf[sizeof(unsigned long)];
    } u;
    debug_decl(ptrace_write_string, SUDO_DEBUG_EXEC);

#ifdef HAVE_PROCESS_VM_READV
    ssize_t nwritten = ptrace_writev_string(pid, addr, str);
    if (nwritten != -1 || errno != ENOSYS)
	debug_return_ssize_t(nwritten);
#endif /* HAVE_PROCESS_VM_READV */

    /*
     * Write the string via ptrace(2) one (native) word at a time.
     * We use the native word size even in compat mode because that
     * is the unit ptrace(2) writes in terms of.
     */
    for (;;) {
	for (i = 0; i < sizeof(u.buf); i++) {
	    if (*str == '\0') {
		/* NUL-pad buf to sizeof(unsigned long). */
		u.buf[i] = '\0';
		continue;
	    }
	    u.buf[i] = *str++;
	}
	if (ptrace(PTRACE_POKEDATA, pid, addr, u.word) == -1) {
	    sudo_warn("%s: ptrace(PTRACE_POKEDATA, %d, 0x%lx, %.*s)",
		__func__, (int)pid, addr, (int)sizeof(u.buf), u.buf);
	    debug_return_ssize_t(-1);
	}
	if ((u.word & 0xff) == 0) {
	    /* If the last byte we wrote is a NUL we are done. */
	    debug_return_ssize_t(str - str0 + 1);
	}
	addr += sizeof(unsigned long);
    }
}

#ifdef HAVE_PROCESS_VM_READV
/*
 * Write the string vector vec to addr in the tracee which must have
 * sufficient space.  Strings are written to strtab.
 * Returns the number of bytes used in strtab (including NULs).
 * process_vm_writev() version.
 */
static ssize_t
ptrace_writev_vec(pid_t pid, struct sudo_ptrace_regs *regs, char **vec,
    unsigned long addr, unsigned long strtab)
{
    const unsigned long addr0 = addr;
    const unsigned long strtab0 = strtab;
    unsigned long *addrbuf = NULL;
    struct iovec *local, *remote;
    struct iovec local_addrs, remote_addrs;
    size_t i, j, len, off = 0;
    ssize_t expected = -1, nwritten, total_written = 0;
    debug_decl(ptrace_writev_vec, SUDO_DEBUG_EXEC);

    /* Build up local and remote iovecs for process_vm_writev(2). */
    for (len = 0; vec[len] != NULL; len++)
	continue;
    local = reallocarray(NULL, len, sizeof(struct iovec));
    remote = reallocarray(NULL, len, sizeof(struct iovec));
    j = regs->compat && (len & 1) != 0;	/* pad for final NULL in compat */
    addrbuf = reallocarray(NULL, len + 1 + j, regs->wordsize);
    if (local == NULL || remote == NULL || addrbuf == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    for (i = 0, j = 0; i < len; i++) {
	unsigned long word = strtab;

	/* Store remote string. */
	const size_t size = strlen(vec[i]) + 1;
	local[i].iov_base = vec[i];
	local[i].iov_len = size;
	remote[i].iov_base = (void *)strtab;
	remote[i].iov_len = size;
	strtab += size;

	/* Store address of remote string. */
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
	if (regs->compat) {
	    /*
	     * For compat binaries we need to pack two 32-bit string addresses
	     * into a single 64-bit word.  If this is the last string, NULL
	     * will be written as the second 32-bit address.
	     */
	    if ((i & 1) == 1) {
		/* Wrote this string address last iteration. */
		continue;
	    }
#  if BYTE_ORDER == BIG_ENDIAN
	    word <<= 32;
	    if (vec[i + 1] != NULL)
		word |= strtab;
#  else
	    if (vec[i + 1] != NULL)
		word |= strtab << 32;
#  endif
	}
# endif
	addrbuf[j++] = word;
	addr += sizeof(unsigned long);
    }
    if (!regs->compat || (len & 1) == 0) {
	addrbuf[j] = 0;
    }

    /* Write strings addresses to addr0 on remote. */
    local_addrs.iov_base = addrbuf;
    local_addrs.iov_len = (len + 1) * regs->wordsize;
    remote_addrs.iov_base = (void *)addr0;
    remote_addrs.iov_len = local_addrs.iov_len;
    if (process_vm_writev(pid, &local_addrs, 1, &remote_addrs, 1, 0) == -1)
	goto done;

    /* Copy the strings to the (remote) string table. */
    expected = (ssize_t)(strtab - strtab0);
    for (;;) {
	nwritten = process_vm_writev(pid, local + off, len - off,
	    remote + off, len - off, 0);
	switch (nwritten) {
	case -1:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"process_vm_writev(%d, 0x%lx, %zu, 0x%lx, %zu, 0)",
		(int)pid, (unsigned long)local + off, len - off,
		(unsigned long)remote + off, len - off);
	    goto done;
	case 0:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"process_vm_writev(%d, 0x%lx, %zu, 0x%lx, %zu, 0): %s",
		(int)pid, (unsigned long)local + off, len - off,
		(unsigned long)remote + off, len - off,
		"zero bytes written");
	    goto done;
	default:
	    total_written += nwritten;
	    if (total_written >= expected)
		goto done;

	    /* Adjust offset for partial write (doesn't cross iov boundary). */
	    while (off < len) {
		nwritten -= (ssize_t)local[off].iov_len;
		off++;
		if (nwritten <= 0)
		    break;
	    }
	    if (off == len) {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "overflow while resuming process_vm_writev()");
		goto done;
	    }
	    break;
	}
    }
done:
    free(local);
    free(remote);
    free(addrbuf);
    if (total_written == expected)
	debug_return_ssize_t(total_written);
    debug_return_ssize_t(-1);
}
#endif /* HAVE_PROCESS_VM_READV */

/*
 * Write the string vector vec to addr in the tracee which must have
 * sufficient space.  Strings are written to strtab.
 * Returns the number of bytes used in strtab (including NULs).
 */
static ssize_t
ptrace_write_vec(pid_t pid, struct sudo_ptrace_regs *regs, char **vec,
    unsigned long addr, unsigned long strtab)
{
    const unsigned long strtab0 = strtab;
    size_t i;
    debug_decl(ptrace_write_vec, SUDO_DEBUG_EXEC);

#ifdef HAVE_PROCESS_VM_READV
    ssize_t nwritten = ptrace_writev_vec(pid, regs, vec, addr, strtab);
    if (nwritten != -1 || errno != ENOSYS)
	debug_return_ssize_t(nwritten);
#endif /* HAVE_PROCESS_VM_READV */

    /* Copy string vector into tracee one word at a time. */
    for (i = 0; vec[i] != NULL; i++) {
	unsigned long word = strtab;

	/* First write the actual string to tracee's string table. */
	nwritten = ptrace_write_string(pid, strtab, vec[i]);
	if (nwritten == -1)
	    debug_return_ssize_t(-1);
	strtab += (size_t)nwritten;

# ifdef SECCOMP_AUDIT_ARCH_COMPAT
	if (regs->compat) {
	    /*
	     * For compat binaries we need to pack two 32-bit string addresses
	     * into a single 64-bit word.  If this is the last string, NULL
	     * will be written as the second 32-bit address.
	     */
	    if ((i & 1) == 1) {
		/* Wrote this string address last iteration. */
		continue;
	    }
#  if BYTE_ORDER == BIG_ENDIAN
	    word <<= 32;
	    if (vec[i + 1] != NULL)
		word |= strtab;
#  else
	    if (vec[i + 1] != NULL)
		word |= strtab << 32;
#  endif
	}
# endif
	/* Next write the string address to tracee at addr. */
	if (ptrace(PTRACE_POKEDATA, pid, addr, word) == -1) {
	    sudo_warn("%s: ptrace(PTRACE_POKEDATA, %d, 0x%lx, 0x%lx)",
		__func__, (int)pid, addr, word);
	    debug_return_ssize_t(-1);
	}
	addr += sizeof(unsigned long);
    }

    /* Finally, write the terminating NULL to tracee if needed. */
    if (!regs->compat || (i & 1) == 0) {
	if (ptrace(PTRACE_POKEDATA, pid, addr, NULL) == -1) {
	    sudo_warn("%s: ptrace(PTRACE_POKEDATA, %d, 0x%lx, NULL)",
		__func__, (int)pid, addr);
	    debug_return_ssize_t(-1);
	}
    }

    debug_return_ssize_t((ssize_t)(strtab - strtab0));
}

/*
 * Read a link from /proc/PID and store the result in buf.
 * Used to read the cwd and exe links in /proc/PID.
 * Returns true on success, else false.
 */
static bool
proc_read_link(pid_t pid, const char *name, char *buf, size_t bufsize)
{
    ssize_t len;
    char path[PATH_MAX];
    debug_decl(proc_read_link, SUDO_DEBUG_EXEC);

    len = snprintf(path, sizeof(path), "/proc/%d/%s", (int)pid, name);
    if (len > 0 && len < ssizeof(path)) {
	len = readlink(path, buf, bufsize - 1);
	if (len != -1) {
	    /* readlink(2) does not add the NUL for us. */
	    buf[len] = '\0';
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

/*
 * Read the filename, argv and envp of the execve(2) system call.
 * Returns a dynamically allocated buffer the parent is responsible for.
 */
static char *
get_execve_info(pid_t pid, struct sudo_ptrace_regs *regs, char **pathname_out,
    int *argc_out, char ***argv_out, int *envc_out, char ***envp_out)
{
    char *argbuf, **argv, **envp, *pathname = NULL;
    unsigned long argv_addr, envp_addr, path_addr;
    size_t bufsize, off = 0;
    int i, argc, envc = 0;
    ssize_t nread;
    debug_decl(get_execve_info, SUDO_DEBUG_EXEC);

    bufsize = PATH_MAX + arg_max;
    argbuf = malloc(bufsize);
    if (argbuf == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }

    /* execve(2) takes three arguments: pathname, argv, envp. */
    path_addr = get_sc_arg1(regs);
    argv_addr = get_sc_arg2(regs);
    envp_addr = get_sc_arg3(regs);
    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: %d: path 0x%lx, argv 0x%lx, envp 0x%lx", __func__,
	(int)pid, path_addr, argv_addr, envp_addr);

    /* Read the pathname, if not NULL. */
    if (path_addr != 0) {
	nread = ptrace_read_string(pid, path_addr, argbuf, bufsize);
	if (nread == -1) {
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to read execve pathname for process %d", (int)pid);
	    goto bad;
	}
	/* Defer setting pathname until after all reallocations are done. */
	off = (size_t)nread;
    }

    /* Read argv */
    nread = ptrace_read_vec(pid, regs, argv_addr, &argc, &argv, &argbuf,
	&bufsize, off);
    if (nread == -1) {
	sudo_debug_printf(
	    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to read execve argv for process %d", (int)pid);
	goto bad;
    }
    off += (size_t)nread;

    if (argc == 0) {
	/* Reserve an extra slot so we can store argv[0]. */
	while (bufsize - off < sizeof(char *)) {
	    if (!growbuf(&argbuf, &bufsize, NULL, NULL))
		goto bad;
	}
	off += sizeof(char *);
    }

    /* Read envp */
    nread = ptrace_read_vec(pid, regs, envp_addr, &envc, &envp, &argbuf,
	&bufsize, off);
    if (nread == -1) {
	sudo_debug_printf(
	    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to read execve envp for process %d", (int)pid);
	goto bad;
    }

    /* Set pathname now that argbuf has been fully allocated. */
    if (path_addr != 0)
	pathname = argbuf;

    /* Convert offsets in argv and envp to pointers. */
    argv = (char **)(argbuf + (unsigned long)argv);
    for (i = 0; i < argc; i++) {
	argv[i] = argbuf + (unsigned long)argv[i];
    }
    envp = (char **)(argbuf + (unsigned long)envp);
    for (i = 0; i < envc; i++) {
	envp[i] = argbuf + (unsigned long)envp[i];
    }

    sudo_debug_execve(SUDO_DEBUG_DIAG, pathname, argv, envp);

    *pathname_out = pathname;
    *argc_out = argc;
    *argv_out = argv;
    *envc_out = envc;
    *envp_out = envp;

    debug_return_ptr(argbuf);
bad:
    free(argbuf);
    debug_return_ptr(NULL);
}

/*
 * Cause the current syscall to fail and set the error value to ecode.
 */
static bool
ptrace_fail_syscall(pid_t pid, struct sudo_ptrace_regs *regs, int ecode)
{
    sigset_t chldmask;
    bool ret = false;
    int status;
    debug_decl(ptrace_fail_syscall, SUDO_DEBUG_EXEC);

    /* Cause the syscall to fail by changing its number to -1. */
    set_syscallno(pid, regs, -1);
    if (!ptrace_setregs(pid, regs)) {
	sudo_warn(U_("unable to set registers for process %d"), (int)pid);
	debug_return_bool(false);
    }

    /* Block SIGCHLD for the critical section (waitpid). */
    sigemptyset(&chldmask);
    sigaddset(&chldmask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &chldmask, NULL);

    /* Allow the syscall to continue and change return value to ecode. */
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    for (;;) {
	if (waitpid(pid, &status, __WALL) != -1)
	    break;
	if (errno == EINTR)
	    continue;
	sudo_warn(U_("%s: %s"), __func__, "waitpid");
	goto done;
    }
    if (!WIFSTOPPED(status)) {
	sudo_warnx(U_("process %d exited unexpectedly"), (int)pid);
	goto done;
    }
    set_sc_retval(regs, -ecode);
    if (!ptrace_setregs(pid, regs)) {
	sudo_warn(U_("unable to set registers for process %d"), (int)pid);
	goto done;
    }

    ret = true;

done:
    sigprocmask(SIG_UNBLOCK, &chldmask, NULL);

    debug_return_bool(ret);
}

/*
 * Check whether seccomp(2) filtering supports ptrace(2) traps.
 * Only supported by Linux 4.14 and higher.
 */
static bool
have_seccomp_action(const char *action)
{
    char line[LINE_MAX];
    bool ret = false;
    FILE *fp;
    debug_decl(have_seccomp_action, SUDO_DEBUG_EXEC);

    fp = fopen("/proc/sys/kernel/seccomp/actions_avail", "r");
    if (fp != NULL) {
	if (fgets(line, sizeof(line), fp) != NULL) {
	    char *cp, *last;

	    for ((cp = strtok_r(line, " \t\n", &last)); cp != NULL;
		(cp = strtok_r(NULL, " \t\n", &last))) {
		if (strcmp(cp, action) == 0) {
		    ret = true;
		    break;
		}
	    }
	}
	fclose(fp);
    }
    debug_return_bool(ret);
}

/*
 * Intercept execve(2) and execveat(2) using seccomp(2) and ptrace(2).
 * If no tracer is present, execve(2) and execveat(2) will fail with ENOSYS.
 * Must be called with CAP_SYS_ADMIN, before privs are dropped.
 */
bool
set_exec_filter(void)
{
    struct sock_filter exec_filter[] = {
	/* Load architecture value (AUDIT_ARCH_*) into the accumulator. */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, arch)),
# ifdef SECCOMP_AUDIT_ARCH_COMPAT2
	/* Match on the compat2 architecture or jump to the compat check. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SECCOMP_AUDIT_ARCH_COMPAT2, 0, 4),
	/* Load syscall number into the accumulator. */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
	/* Jump to trace for compat2 execve(2)/execveat(2), else allow. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, COMPAT2_execve, 1, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, COMPAT2_execveat, 0, 13),
	/* Trace execve(2)/execveat(2) syscalls (w/ compat flag) */
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE | COMPAT_FLAG),
# endif /* SECCOMP_AUDIT_ARCH_COMPAT2 */
# ifdef SECCOMP_AUDIT_ARCH_COMPAT
	/* Match on the compat architecture or jump to the native arch check. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SECCOMP_AUDIT_ARCH_COMPAT, 0, 4),
	/* Load syscall number into the accumulator. */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
	/* Jump to trace for compat execve(2)/execveat(2), else allow. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, COMPAT_execve, 1, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, COMPAT_execveat, 0, 8),
	/* Trace execve(2)/execveat(2) syscalls (w/ compat flag) */
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE | COMPAT_FLAG),
# endif /* SECCOMP_AUDIT_ARCH_COMPAT */
	/* Jump to the end unless the architecture matches. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SECCOMP_AUDIT_ARCH, 0, 6),
	/* Load syscall number into the accumulator. */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
	/* Jump to trace for execve(2)/execveat(2), else allow. */
# ifdef X32_execve
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, X32_execve, 3, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, X32_execveat, 2, 0),
# else
	/* No x32 support, check native system call numbers. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_execve, 3, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_execveat, 2, 3),
# endif /* X32_execve */
	/* If no x32 support, these two instructions are never reached. */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_execve, 1, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_execveat, 0, 1),
	/* Trace execve(2)/execveat(2) syscalls */
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
	/* Allow non-matching syscalls */
	BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
    };
    const struct sock_fprog exec_fprog = {
	nitems(exec_filter),
	exec_filter
    };
    debug_decl(set_exec_filter, SUDO_DEBUG_EXEC);

    /* We must set SECCOMP_MODE_FILTER before dropping privileges. */
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &exec_fprog) == -1) {
	sudo_warn("%s", U_("unable to set seccomp filter"));
	debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Seize control of the specified child process which must be in
 * ptrace wait.  Returns true on success, false if child is already
 * being traced and -1 on error.
 */
int
exec_ptrace_seize(pid_t child)
{
    const long ptrace_opts = PTRACE_O_TRACESECCOMP|PTRACE_O_TRACECLONE|
			     PTRACE_O_TRACEFORK|PTRACE_O_TRACEVFORK|
			     PTRACE_O_TRACEEXEC;
    int ret = -1;
    int status;
    debug_decl(exec_ptrace_seize, SUDO_DEBUG_EXEC);

#ifdef HAVE_PROCESS_VM_READV
    page_size = (size_t)sysconf(_SC_PAGESIZE);
    if (page_size == (size_t)-1)
	page_size = 4096;
#endif
    arg_max = (size_t)sysconf(_SC_ARG_MAX);
    if (arg_max == (size_t)-1)
	arg_max = 128 * 1024;

    /* Seize control of the child process. */
    if (ptrace(PTRACE_SEIZE, child, NULL, ptrace_opts) == -1) {
	/*
	 * If the process is already being traced, we will get EPERM.
	 * We don't treat that as a fatal error since we want it to be
	 * possible to run sudo inside a sudo shell with intercept enabled.
	 */
	if (errno != EPERM) {
	    sudo_warn("%s: ptrace(PTRACE_SEIZE, %d, NULL, 0x%lx)",
		__func__, (int)child, ptrace_opts);
	    goto done;
	}
	sudo_debug_printf(SUDO_DEBUG_WARN,
	    "%s: unable to trace process %d, already being traced?",
		__func__, (int)child);
	ret = false;
    }

    /* The child is suspended waiting for SIGUSR1, wake it up. */
    if (kill(child, SIGUSR1) == -1) {
	sudo_warn("kill(%d, SIGUSR1)", (int)child);
	goto done;
    }
    if (!ret)
	goto done;

    /* Wait for the child to enter trace stop and continue it. */
    for (;;) {
	if (waitpid(child, &status, __WALL) != -1)
	    break;
	if (errno == EINTR)
	    continue;
	sudo_warn(U_("%s: %s"), __func__, "waitpid");
	goto done;
    }
    if (!WIFSTOPPED(status)) {
	sudo_warnx(U_("process %d exited unexpectedly"), (int)child);
	goto done;
    }
    if (ptrace(PTRACE_CONT, child, NULL, (void *)SIGUSR1) == -1) {
	sudo_warn("%s: ptrace(PTRACE_CONT, %d, NULL, SIGUSR1)",
	    __func__, (int)child);
	goto done;
    }

    ret = true;

done:
    debug_return_int(ret);
}

/*
 * Compare two pathnames.  If do_stat is true, fall back to stat(2)ing
 * the paths for a dev/inode match if the strings don't match.
 * Returns true on match, else false.
 */
static bool
pathname_matches(const char *path1, const char *path2, bool do_stat)
{
    struct stat sb1, sb2;
    debug_decl(pathname_matches, SUDO_DEBUG_EXEC);

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: compare %s to %s", __func__,
	path1 ? path1 : "(NULL)", path2 ? path2 : "(NULL)");

    if (path1 == NULL || path2 == NULL)
	debug_return_bool(false);

    if (strcmp(path1, path2) == 0)
	debug_return_bool(true);

    if (do_stat && stat(path1, &sb1) == 0 && stat(path2, &sb2) == 0) {
	if (sb1.st_dev == sb2.st_dev && sb1.st_ino == sb2.st_ino)
	    debug_return_bool(true);
    }

    debug_return_bool(false);
}

/*
 * Open script and check for '#!' magic number followed by an interpreter.
 * If present, check the interpreter against execpath, and argument string
 * (if any) against argv[1].
 * Returns number of argv entries to skip on success, else 0.
 */
static int
script_matches(const char *script, const char *execpath, int argc,
    char * const *argv)
{
    char * const *orig_argv = argv;
    size_t linesize = 0;
    char *interp, *interp_args, *line = NULL;
    char magic[2];
    int count;
    FILE *fp = NULL;
    ssize_t len;
    debug_decl(get_interpreter, SUDO_DEBUG_EXEC);

    /* Linux allows up to 4 nested interpreters. */
    for (count = 0; count < 4; count++) {
	if (fp != NULL)
	    fclose(fp);
	fp = fopen(script, "r");
	if (fp == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO,
		"%s: unable to open %s for reading", __func__, script);
	    goto done;
	}

	if (fread(magic, 1, 2, fp) != 2 || memcmp(magic, "#!", 2) != 0) {
	    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: %s: not a script",
		__func__, script);
	    goto done;
	}

	/* Check interpreter, skipping the shebang and trim trailing space. */
	len = getdelim(&line, &linesize, '\n', fp);
	if (len == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: %s: can't get interpreter",
		__func__, script);
	    goto done;
	}
	while (len > 0 && isspace((unsigned char)line[len - 1])) {
	    len--;
	    line[len] = '\0';
	}
	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: %s: shebang line \"%s\"",
	    __func__, script, line);

	/*
	 * Split line into interpreter and args.
	 * Whitespace is not supported in the interpreter path.
	 */
	for (interp = line; isspace((unsigned char)*interp); interp++)
	    continue;
	interp_args = strpbrk(interp, " \t");
	if (interp_args != NULL) {
	    *interp_args++ = '\0';
	    while (isspace((unsigned char)*interp_args))
		interp_args++;
	}

	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: interpreter %s, args \"%s\"",
	    __func__, interp, interp_args ? interp_args : "");

	/* Match interpreter. */
	if (!pathname_matches(execpath, interp, true)) {
	    /* It is possible for the interpreter to be a script too. */
	    if (argv > 0 && strcmp(interp, argv[1]) == 0) {
		/* Interpreter args must match for *this* interpreter. */
		if (interp_args == NULL ||
			(argc > 1 && strcmp(interp_args, argv[2]) == 0)) {
		    script = interp;
		    argv++;
		    argc--;
		    if (interp_args != NULL) {
			argv++;
			argc--;
		    }
		    /* Check whether interp is itself a script. */
		    continue;
		}
	    }
	}
	if (argc > 0 && interp_args != NULL) {
	    if (strcmp(interp_args, argv[1]) != 0) {
		sudo_warnx(
		    U_("interpreter argument , expected \"%s\", got \"%s\""),
		    interp_args, argc > 1 ? argv[1] : "(NULL)");
		goto done;
	    }
	    argv++;
	}
	argv++;
	break;
    }

done:
    free(line);
    if (fp != NULL)
	fclose(fp);
    debug_return_int((int)(argv - orig_argv));
}

static ssize_t
proc_read_vec(pid_t pid, const char *name, int *countp, char ***vecp,
    char **bufp, size_t *bufsizep, size_t off)
{
    size_t strtab_len, remainder = *bufsizep - off;
    char path[PATH_MAX], *strtab = *bufp + off;
    ssize_t len, nread;
    int fd;
    debug_decl(proc_read_vec, SUDO_DEBUG_EXEC);

    len = snprintf(path, sizeof(path), "/proc/%d/%s", (int)pid, name);
    if (len >= ssizeof(path))
	debug_return_ssize_t(-1);

    fd = open(path, O_RDONLY);
    if (fd == -1)
	debug_return_ssize_t(-1);

    /* Read in strings until EOF. */
    do {
	nread = read(fd, strtab, remainder);
	if (nread == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"%s: unable to read %s", __func__, path);
	    close(fd);
	    debug_return_ssize_t(-1);
	}
	strtab += nread;
	remainder -= (size_t)nread;
	if (remainder < sizeof(char *)) {
	    while (!growbuf(bufp, bufsizep, &strtab, &remainder)) {
		close(fd);
		debug_return_ssize_t(-1);
	    }
	}
    } while (nread != 0);
    close(fd);

    /* Trim off the extra NUL byte at the end of the string table. */
    if (strtab - *bufp >= 2 && strtab[-1] == '\0' && strtab[-2] == '\0') {
	strtab--;
	remainder++;
    }

    /* Store strings in a vector after the string table. */
    strtab_len = (size_t)(strtab - (*bufp + off));
    strtab = *bufp + off;
    len = strtab_to_vec(strtab, strtab_len, countp, vecp, bufp, bufsizep,
	remainder);
    if (len == -1)
	debug_return_ssize_t(-1);

    debug_return_ssize_t((ssize_t)strtab_len + len);
}

/*
 * Check if the execve(2) arguments match the contents of closure.
 * Returns true if they match, else false.
 */
static bool
execve_args_match(const char *pathname, int argc, char * const *argv,
    int envc, char * const *envp, bool do_stat,
    struct intercept_closure *closure)
{
    bool ret = true;
    int i;
    debug_decl(execve_args_match, SUDO_DEBUG_EXEC);

    if (!pathname_matches(pathname, closure->command, do_stat)) {
	/* For scripts, pathname will refer to the interpreter instead. */
	if (do_stat) {
	    int skip = script_matches(closure->command, pathname,
		argc, argv);
	    if (skip != 0) {
		/* Skip interpreter (and args) in argv. */
		argv += skip;
		argc -= skip;
		goto check_argv;
	    }
	}
	sudo_warnx(
	    U_("pathname mismatch, expected \"%s\", got \"%s\""),
	    closure->command, pathname ? pathname : "(NULL)");
	ret = false;
    }
check_argv:
    for (i = 0; i < argc; i++) {
	if (closure->run_argv[i] == NULL) {
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"argv", i, "(NULL)", argv[i] ? argv[i] : "(NULL)");
	    break;
	}
	if (argv[i] == NULL) {
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"argv", i, closure->run_argv[i], "(NULL)");
	    break;
	}
	if (strcmp(argv[i], closure->run_argv[i]) != 0) {
	    if (i == 0) {
		/* Special case for argv[0] which may contain the basename. */
		const char *base;
		if (argv[0][0] == '/') {
		    if (closure->run_argv[0][0] != '/') {
			base = sudo_basename(argv[0]);
			if (strcmp(base, closure->run_argv[0]) == 0)
			    continue;
		    }
		} else {
		    if (closure->run_argv[0][0] == '/') {
			base = sudo_basename(closure->run_argv[0]);
			if (strcmp(argv[0], base) == 0)
			    continue;
		    }
		}
	    }
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"argv", i, closure->run_argv[i], argv[i]);
	}
    }
    for (i = 0; i < envc; i++) {
	if (closure->run_envp[i] == NULL) {
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"envp", i, "(NULL)", envp[i] ? envp[i] : "(NULL)");
	    break;
	} else if (envp[i] == NULL) {
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"envp", i, closure->run_envp[i], "(NULL)");
	    break;
	} else if (strcmp(envp[i], closure->run_envp[i]) != 0) {
	    ret = false;
	    sudo_warnx(
		U_("%s[%d] mismatch, expected \"%s\", got \"%s\""),
		"envp", i, closure->run_envp[i], envp[i]);
	}
    }

    debug_return_bool(ret);
}

/*
 * Verify that the execve(2) argument we wrote match the contents of closure.
 * Returns true if they match, else false.
 */
static bool
verify_execve_args(pid_t pid, struct sudo_ptrace_regs *regs,
    struct intercept_closure *closure)
{
    char *pathname, **argv, **envp, *buf;
    int argc, envc;
    bool ret = false;
    debug_decl(verify_execve_args, SUDO_DEBUG_EXEC);

    buf = get_execve_info(pid, regs, &pathname, &argc, &argv,
	&envc, &envp);
    if (buf != NULL) {
	ret = execve_args_match(pathname, argc, argv, envc, envp, false, closure);
	free(buf);
    }

    debug_return_bool(ret);
}

/*
 * Verify that the command executed matches the arguments we checked.
 * Returns true on success and false on error.
 */
static bool
ptrace_verify_post_exec(pid_t pid, struct sudo_ptrace_regs *regs,
    struct intercept_closure *closure)
{
    char **argv, **envp, *argbuf = NULL;
    char pathname[PATH_MAX];
    sigset_t chldmask;
    bool ret = false;
    int argc, envc, i, status;
    size_t bufsize;
    ssize_t len;
    debug_decl(ptrace_verify_post_exec, SUDO_DEBUG_EXEC);

    /* Block SIGCHLD for the critical section (waitpid). */
    sigemptyset(&chldmask);
    sigaddset(&chldmask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &chldmask, NULL);

    /* Allow execve(2) to continue and wait for PTRACE_EVENT_EXEC. */
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    for (;;) {
	if (waitpid(pid, &status, __WALL) != -1)
	    break;
	if (errno == EINTR)
	    continue;
	sudo_warn(U_("%s: %s"), __func__, "waitpid");
	goto done;
    }
    if (!WIFSTOPPED(status)) {
	sudo_warnx(U_("process %d exited unexpectedly"), (int)pid);
	goto done;
    }
    if (status >> 8 != (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
	sudo_warnx(U_("process %d unexpected status 0x%x"), (int)pid, status);
	goto done;
    }

    /* Get the executable path. */
    if (!proc_read_link(pid, "exe", pathname, sizeof(pathname))) {
	/* Missing /proc file system is not a fatal error. */
	sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: unable to read /proc/%d/exe",
	    __func__, (int)pid);
	ret = true;
	goto done;
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %d: verify %s", __func__,
	(int)pid, pathname);

    /* Allocate a single buffer for argv, envp and their strings. */
    bufsize = arg_max;
    argbuf = malloc(bufsize);
    if (argbuf == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    len = proc_read_vec(pid, "cmdline", &argc, &argv, &argbuf, &bufsize, 0);
    if (len == -1) {
	sudo_debug_printf(
	    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to read execve argv for process %d", (int)pid);
	goto done;
    }

    len = proc_read_vec(pid, "environ", &envc, &envp, &argbuf, &bufsize,
	(size_t)len);
    if (len == -1) {
	sudo_debug_printf(
	    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "unable to read execve envp for process %d", (int)pid);
	goto done;
    }

    /* Convert offsets in argv and envp to pointers. */
    argv = (char **)(argbuf + (unsigned long)argv);
    for (i = 0; i < argc; i++) {
	argv[i] = argbuf + (unsigned long)argv[i];
    }
    envp = (char **)(argbuf + (unsigned long)envp);
    for (i = 0; i < envc; i++) {
	envp[i] = argbuf + (unsigned long)envp[i];
    }

    ret = execve_args_match(pathname, argc, argv, envc, envp, true, closure);
    if (!ret) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: %d new execve args don't match closure", __func__, (int)pid);
    }

done:
    free(argbuf);
    sigprocmask(SIG_UNBLOCK, &chldmask, NULL);

    debug_return_bool(ret);
}

/*
 * Intercept execve(2) and perform a policy check.
 * Reads current registers and execve(2) arguments.
 * If the command is not allowed by policy, fail with EACCES.
 * If the command is allowed, update argv if needed before continuing.
 * Returns true on success and false on error.
 */
static bool
ptrace_intercept_execve(pid_t pid, struct intercept_closure *closure)
{
    char *pathname, **argv, **envp, *buf;
    const unsigned int flags = closure->details->flags;
    int argc, envc, syscallno;
    struct sudo_ptrace_regs regs;
    bool path_mismatch = false;
    bool argv_mismatch = false;
    char cwd[PATH_MAX], *orig_argv0;
    unsigned long msg;
    bool ret = false;
    int i, oldcwd = -1;
    debug_decl(ptrace_intercept_execve, SUDO_DEBUG_EXEC);

    /* Do not check the policy if we are executing the initial command. */
    if (closure->initial_command != 0) {
	closure->initial_command--;
	debug_return_bool(true);
    }

    /* Get compat flag. */
    if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &msg) == -1) {
	sudo_warn(U_("unable to get event message for process %d"), (int)pid);
	debug_return_bool(false);
    }

    /* Get the registers. */
    memset(&regs, 0, sizeof(regs));
    if (!ptrace_getregs(pid, &regs, msg)) {
	sudo_warn(U_("unable to get registers for process %d"), (int)pid);
	debug_return_bool(false);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %d: compat: %s, wordsize: %u",
	__func__, (int)pid, regs.compat ? "true" : "false", regs.wordsize);

# ifdef SECCOMP_AUDIT_ARCH_COMPAT
    if (regs.compat) {
	syscallno = get_syscallno(&regs);
	switch (syscallno) {
	case COMPAT_execve:
	    /* Handled below. */
	    break;
	case COMPAT_execveat:
	    /* We don't currently check execveat(2). */
	    debug_return_bool(true);
	    break;
	default:
	    sudo_warnx("%s: unexpected compat system call %d",
		__func__, syscallno);
	    debug_return_bool(false);
	}
    } else
# endif /* SECCOMP_AUDIT_ARCH_COMPAT */
    {
	syscallno = get_syscallno(&regs);
	switch (syscallno) {
# ifdef X32_execve
	case X32_execve:
# endif
	case __NR_execve:
	    /* Handled below. */
	    break;
# ifdef X32_execveat
	case X32_execveat:
# endif
	case __NR_execveat:
	    /* We don't currently check execveat(2). */
	    debug_return_bool(true);
	    break;
	default:
	    sudo_warnx("%s: unexpected system call %d", __func__, syscallno);
	    debug_return_bool(false);
	}
    }

    /* Get the current working directory and execve info. */
    if (!proc_read_link(pid, "cwd", cwd, sizeof(cwd)))
	(void)strlcpy(cwd, "unknown", sizeof(cwd));
    buf = get_execve_info(pid, &regs, &pathname, &argc, &argv,
	&envc, &envp);
    if (buf == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: %d: unable to get execve info", __func__, (int)pid);
	/* EIO from ptrace is like EFAULT from the kernel. */
	if (errno == EIO)
	    errno = EFAULT;
	ptrace_fail_syscall(pid, &regs, errno);
	goto done;
    }

    /* Must have a pathname. */
    if (pathname == NULL) {
	ptrace_fail_syscall(pid, &regs, EINVAL);
	goto done;
    }

    /* We can only pass the pathname to exececute via argv[0] (plugin API). */
    orig_argv0 = argv[0] ? argv[0] : (char *)"";
    argv[0] = pathname;
    if (argc == 0) {
	/* Rewrite an empty argv[] with the path to execute. */
	argv[1] = NULL;
	argc = 1;
	argv_mismatch = true;
    }

    /* Perform a policy check. */
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %d: checking policy for %s",
	__func__, (int)pid, pathname);
    if (!intercept_check_policy(pathname, argc, argv, envc, envp, cwd,
	    &oldcwd, closure)) {
	if (closure->errstr != NULL)
	    sudo_warnx("%s", U_(closure->errstr));
    }

    switch (closure->state) {
    case POLICY_TEST:
	path_mismatch = true;
	argv_mismatch = true;
	if (closure->command == NULL)
	    closure->command = pathname;
	if (closure->run_argv == NULL)
	    closure->run_argv = argv;
	if (closure->run_envp == NULL)
	    closure->run_envp = envp;
	FALLTHROUGH;
    case POLICY_ACCEPT:
	/*
	 * Update pathname and argv if the policy modified it.
	 * We don't currently ever modify envp.
	 */
	if (strcmp(pathname, closure->command) != 0)
	    path_mismatch = true;
	if (!path_mismatch) {
	    /* Path unchanged, restore original argv[0]. */
	    if (strcmp(argv[0], orig_argv0) != 0) {
		argv[0] = orig_argv0;
		free(closure->run_argv[0]);
		closure->run_argv[0] = strdup(orig_argv0);
		if (closure->run_argv[0] == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		}
	    }
	}
	for (i = 0; closure->run_argv[i] != NULL && argv[i] != NULL; i++) {
	    if (strcmp(closure->run_argv[i], argv[i]) != 0) {
		argv_mismatch = true;
		break;
	    }
	}
	if (closure->run_argv[i] != NULL || argv[i] != NULL)
	    argv_mismatch = true;

	if (path_mismatch || argv_mismatch) {
	    /*
	     * Need to rewrite pathname and/or argv.
	     * We can use space below the stack pointer to store the data.
	     * On amd64 there is a 128 byte red zone that must be avoided.
	     * Note: on pa-risc the stack grows up, not down.
	     */
	    unsigned long sp = get_stack_pointer(&regs) - 128;
	    unsigned long strtab;
	    size_t space = 0;
	    ssize_t nwritten;

	    sudo_debug_execve(SUDO_DEBUG_DIAG, closure->command,
		closure->run_argv, envp);

	    /*
	     * Calculate the amount of space required for pointers + strings.
	     * Since ptrace(2) always writes in sizeof(long) increments we
	     * need to be careful to avoid overwriting what we have already
	     * written for compat binaries (where the word size doesn't match).
	     *
	     * This is mostly a problem for the string table since we do
	     * interleaved writes of the argument vector pointers and the
	     * strings they refer to.  For native binaries, it is sufficient
	     * to align the string table on a word boundary.  For compat
	     * binaries, if argc is odd, writing the last pointer will overlap
	     * the first string so leave an extra word in between them.
	     */
	    if (argv_mismatch) {
		/* argv pointers */
		space += ((size_t)argc + 1 + regs.compat) * regs.wordsize;

		/* argv strings */
		for (argc = 0; closure->run_argv[argc] != NULL; argc++) {
		    space += strlen(closure->run_argv[argc]) + 1;
		}
	    }
	    if (path_mismatch) {
		/* pathname string */
		space += strlen(closure->command) + 1;
	    }

	    /* Reserve stack space for path, argv (w/ NULL) and its strings. */
	    sp -= WORDALIGN(space, regs);
	    strtab = sp;

	    if (argv_mismatch) {
		/* Update argv address in the tracee to our new value. */
		set_sc_arg2(&regs, sp);

		/* Skip over argv pointers (plus NULL) for string table. */
		strtab += ((size_t)argc + 1 + regs.compat) * regs.wordsize;

		nwritten = ptrace_write_vec(pid, &regs, closure->run_argv,
		    sp, strtab);
		if (nwritten == -1)
		    goto done;
		strtab += (unsigned long)nwritten;
	    }
	    if (path_mismatch) {
		/* Update pathname address in the tracee to our new value. */
		set_sc_arg1(&regs, strtab);

		/* Write pathname to the string table. */
		nwritten = ptrace_write_string(pid, strtab, closure->command);
		if (nwritten == -1)
		    goto done;
	    }

	    /* Update args in the tracee to the new values. */
	    if (!ptrace_setregs(pid, &regs)) {
		sudo_warn(U_("unable to set registers for process %d"),
		    (int)pid);
		goto done;
	    }

	    if (closure->state == POLICY_TEST) {
		/* Verify the contents of what we just wrote. */
		if (!verify_execve_args(pid, &regs, closure)) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR,
			"%s: new execve args don't match closure", __func__);
		}
	    }
	}
	if (closure->state == POLICY_ACCEPT && ISSET(flags, CD_INTERCEPT)) {
	    if (ISSET(flags, CD_INTERCEPT_VERIFY)) {
		/* Verify execve(2) args post-exec. */
		if (!ptrace_verify_post_exec(pid, &regs, closure)) {
		    if (errno != ESRCH)
			kill(pid, SIGKILL);
		}
	    }
	}
	break;
    case POLICY_REJECT:
	/* If rejected, fake the syscall and set return to EACCES */
	errno = EACCES;
	FALLTHROUGH;
    default:
	ptrace_fail_syscall(pid, &regs, errno);
	break;
    }

    ret = true;

done:
    if (oldcwd != -1) {
        if (fchdir(oldcwd) == -1) {
            sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
                "%s: unable to restore saved cwd", __func__);
        }
        close(oldcwd);
    }
    free(buf);
    intercept_closure_reset(closure);

    debug_return_bool(ret);
}

/*
 * Handle a process stopped due to ptrace.
 * Restarts the tracee with PTRACE_LISTEN (for a group-stop)
 * or PTRACE_CONT (for signal-delivery-stop).
 * Returns true if stopped by a group-stop, else false.
 */
bool
exec_ptrace_stopped(pid_t pid, int status, void *intercept)
{
    struct intercept_closure *closure = intercept;
    const int stopsig = WSTOPSIG(status);
    const int sigtrap = status >> 8;
    long signo = 0;
    bool group_stop = false;
    debug_decl(exec_ptrace_stopped, SUDO_DEBUG_EXEC);

    if (sigtrap == (SIGTRAP | (PTRACE_EVENT_SECCOMP << 8))) {
	if (!ptrace_intercept_execve(pid, closure)) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: %d failed to intercept execve", __func__, (int)pid);
	}
    } else if (sigtrap == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: %d PTRACE_EVENT_EXEC", __func__, (int)pid);
    } else if (sigtrap == (SIGTRAP | (PTRACE_EVENT_CLONE << 8)) ||
	sigtrap == (SIGTRAP | (PTRACE_EVENT_VFORK << 8)) ||
	sigtrap == (SIGTRAP | (PTRACE_EVENT_FORK << 8))) {
	unsigned long new_pid;

	/* New child process, it will inherit the parent's trace flags. */
	if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
	    if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &new_pid) != -1) {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: %d forked new child %lu", __func__, (int)pid, new_pid);
	    } else {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "ptrace(PTRACE_GETEVENTMSG, %d, NULL, %p)", (int)pid,
		    &new_pid);
	    }
	}
    } else {
	switch (stopsig) {
	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
	    /* Is this a group-stop? */
	    if (status >> 16 == PTRACE_EVENT_STOP) {
		/* Group-stop, do not deliver signal. */
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: %d: group-stop signal %d",
		    __func__, (int)pid, stopsig);
		group_stop = true;
		break;
	    }
	    FALLTHROUGH;
	default:
	    /* Signal-delivery-stop, deliver signal. */
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"%s: %d: signal-delivery-stop signal %d",
		__func__, (int)pid, stopsig);
	    signo = stopsig;
	    break;
	}
    }

    if (group_stop) {
	/*
	 * Restart child but prevent it from executing
	 * until SIGCONT is received (simulate SIGSTOP, etc).
	 */
	if (ptrace(PTRACE_LISTEN, pid, NULL, 0L) == -1 && errno != ESRCH)
	    sudo_warn("%s: ptrace(PTRACE_LISTEN, %d, NULL, 0L)",
		__func__, (int)pid);
    } else {
	/* Restart child immediately. */
	if (ptrace(PTRACE_CONT, pid, NULL, signo) == -1 && errno != ESRCH)
	    sudo_warn("%s: ptrace(PTRACE_CONT, %d, NULL, %ld)",
		__func__, (int)pid, signo);
    }

    debug_return_bool(group_stop);
}

bool
exec_ptrace_intercept_supported(void)
{
# ifdef __mips__
    /* MIPS doesn't support changing the syscall return value. */
    return false;
# else
    if (seccomp_trap_supported == -1)
	seccomp_trap_supported = have_seccomp_action("trap");

    return seccomp_trap_supported == true;
# endif
}

bool
exec_ptrace_subcmds_supported(void)
{
    if (seccomp_trap_supported == -1)
	seccomp_trap_supported = have_seccomp_action("trap");

    return seccomp_trap_supported == true;
}
#else
/* STUB */
bool
exec_ptrace_stopped(pid_t pid, int status, void *intercept)
{
    return true;
}

/* STUB */
int
exec_ptrace_seize(pid_t child)
{
    return true;
}

/* STUB */
bool
exec_ptrace_intercept_supported(void)
{
    return false;
}

/* STUB */
bool
exec_ptrace_subcmds_supported(void)
{
    return false;
}
#endif /* HAVE_PTRACE_INTERCEPT */

/*
 * Adjust flags based on the availability of ptrace support.
 */
void
exec_ptrace_fix_flags(struct command_details *details)
{
    debug_decl(exec_ptrace_fix_flags, SUDO_DEBUG_EXEC);

    if (ISSET(details->flags, CD_USE_PTRACE)) {
	/* If both CD_INTERCEPT and CD_LOG_SUBCMDS set, CD_INTERCEPT wins. */
	if (ISSET(details->flags, CD_INTERCEPT)) {
	    if (!exec_ptrace_intercept_supported())
		CLR(details->flags, CD_USE_PTRACE);
	} else if (ISSET(details->flags, CD_LOG_SUBCMDS)) {
	    if (!exec_ptrace_subcmds_supported())
		CLR(details->flags, CD_USE_PTRACE);
	} else {
	    CLR(details->flags, CD_USE_PTRACE);
	}
    }
    debug_return;
}
