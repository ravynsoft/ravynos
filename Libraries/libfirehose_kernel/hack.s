/*
 * Cross building xnu from Linux using my ravynOS.sdk and toolchain works
 * up to the final link, which has a few unresolved symbols that seem to be
 * artifacts of the hybrid build environment. For now, we try to resolve them
 * with aliases and stubs, and hope it works.
 * -- zoe 1/24/26
 */

#ifdef __x86_64__
#include <architecture/i386/asm_help.h>
#else
#error Unsupported platform
#endif
 
.text

.globl ___ulock_wait
___ulock_wait = _ulock_wait

.globl ___ulock_wake
___ulock_wake = _ulock_wake

LEAF(___cxa_atexit, 0)
        xorq %rax, %rax
        ret

X_LEAF(_OSSpinLockLock, _OSSpinLockTry)
