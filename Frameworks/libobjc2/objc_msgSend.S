#include "common.S"
#include "asmconstants.h"
#if __x86_64
#include "objc_msgSend.x86-64.S"
#elif __i386
#include "objc_msgSend.x86-32.S"
#elif __arm__
#include "objc_msgSend.arm.S"
#elif defined(__ARM_ARCH_ISA_A64)
#include "objc_msgSend.aarch64.S"
#elif defined(__mips_n64) || defined(__mips_n32)
#include "objc_msgSend.mips.S"
#else
#warning objc_msgSend() not implemented for your architecture
#endif
#ifdef __ELF__
.section .note.GNU-stack,"",%progbits
#endif
