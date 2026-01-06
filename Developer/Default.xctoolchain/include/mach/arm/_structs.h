/*
 * Copyright (c) 2004-2007 Apple Inc. All rights reserved.
 */
/*
 * @OSF_COPYRIGHT@
 */
#ifndef	_MACH_ARM__STRUCTS_H_
#define	_MACH_ARM__STRUCTS_H_

#define _STRUCT_ARM_EXCEPTION_STATE	struct __darwin_arm_exception_state
_STRUCT_ARM_EXCEPTION_STATE
{
	__uint32_t	__exception; /* number of arm exception taken */
	__uint32_t	__fsr; /* Fault status */
	__uint32_t	__far; /* Virtual Fault Address */
};

#define _STRUCT_ARM_EXCEPTION_STATE64	struct __darwin_arm_exception_state64
_STRUCT_ARM_EXCEPTION_STATE64
{
	__uint64_t	__far; /* Virtual Fault Address */
	__uint32_t	__esr; /* Exception syndrome */
	__uint32_t	__exception; /* number of arm exception taken */
};

#define _STRUCT_ARM_THREAD_STATE	struct __darwin_arm_thread_state
_STRUCT_ARM_THREAD_STATE
{
	__uint32_t	__r[13];	/* General purpose register r0-r12 */
	__uint32_t	__sp;		/* Stack pointer r13 */
	__uint32_t	__lr;		/* Link register r14 */
	__uint32_t	__pc;		/* Program counter r15 */
	__uint32_t	__cpsr;		/* Current program status register */
};

#define _STRUCT_ARM_THREAD_STATE64	struct __darwin_arm_thread_state64
_STRUCT_ARM_THREAD_STATE64
{
	__uint64_t    __x[29];	/* General purpose registers x0-x28 */
	__uint64_t    __fp;		/* Frame pointer x29 */
	__uint64_t    __lr;		/* Link register x30 */
	__uint64_t    __sp;		/* Stack pointer x31 */
	__uint64_t    __pc;		/* Program counter */
	__uint32_t    __cpsr;	/* Current program status register */
};

#define _STRUCT_ARM_VFP_STATE		struct __darwin_arm_vfp_state
_STRUCT_ARM_VFP_STATE
{
	__uint32_t        __r[64];
	__uint32_t        __fpscr;

};

#define _STRUCT_ARM_NEON_STATE64		struct __darwin_arm_neon_state64
#define _STRUCT_ARM_NEON_STATE		struct __darwin_arm_neon_state

#if defined(__arm64__)
_STRUCT_ARM_NEON_STATE64
{
	__uint128_t       __v[32];
	__uint32_t        __fpsr;
	__uint32_t        __fpcr;
};

_STRUCT_ARM_NEON_STATE
{
	__uint128_t       __v[16];
	__uint32_t        __fpsr;
	__uint32_t        __fpcr;
};

#elif defined(__arm__)
/*
 * No 128-bit intrinsic for ARM; leave it opaque for now.
 */
_STRUCT_ARM_NEON_STATE64 
{
	char opaque[(32 * 16) + (2 * sizeof(__uint32_t))];
} __attribute__((aligned(16)));

_STRUCT_ARM_NEON_STATE
{
	char opaque[(16 * 16) + (2 * sizeof(__uint32_t))];
} __attribute__((aligned(16)));

#else
/* #error Unknown architecture. */
#endif


/*
 * Debug State
 */
#if defined(__arm__)
#define _STRUCT_ARM_DEBUG_STATE	struct __darwin_arm_debug_state
_STRUCT_ARM_DEBUG_STATE
{
	__uint32_t        __bvr[16];
	__uint32_t        __bcr[16];
	__uint32_t        __wvr[16];
	__uint32_t        __wcr[16];
};

#elif defined(__arm64__)
#define _STRUCT_ARM_LEGACY_DEBUG_STATE	struct arm_legacy_debug_state
_STRUCT_ARM_LEGACY_DEBUG_STATE
{
	__uint32_t        __bvr[16];
	__uint32_t        __bcr[16];
	__uint32_t        __wvr[16];
	__uint32_t        __wcr[16];
};

#define _STRUCT_ARM_DEBUG_STATE32	struct __darwin_arm_debug_state32
_STRUCT_ARM_DEBUG_STATE32
{
	__uint32_t        __bvr[16];
	__uint32_t        __bcr[16];
	__uint32_t        __wvr[16];
	__uint32_t        __wcr[16];
	__uint64_t	  __mdscr_el1; /* Bit 0 is SS (Hardware Single Step) */
};

#define _STRUCT_ARM_DEBUG_STATE64	struct __darwin_arm_debug_state64
_STRUCT_ARM_DEBUG_STATE64
{
	__uint64_t        __bvr[16];
	__uint64_t        __bcr[16];
	__uint64_t        __wvr[16];
	__uint64_t        __wcr[16];
	__uint64_t	  __mdscr_el1; /* Bit 0 is SS (Hardware Single Step) */
};

#else
/* #error unknown architecture */
#endif

#endif /* _MACH_ARM__STRUCTS_H_ */
