/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glapi_entrypoint.c
 *
 * Arch-specific code for manipulating GL API entrypoints (dispatch stubs).
 */


#include <string.h>

#include "c11/threads.h"
#include "glapi/glapi_priv.h"


#ifdef USE_X86_ASM

extern       GLubyte gl_dispatch_functions_start[];
extern       GLubyte gl_dispatch_functions_end[];

#endif /* USE_X86_ASM */


#if defined(DISPATCH_FUNCTION_SIZE)

_glapi_proc
get_entrypoint_address(unsigned int functionOffset)
{
   return (_glapi_proc) (gl_dispatch_functions_start
                         + (DISPATCH_FUNCTION_SIZE * functionOffset));
}

#endif


#if defined(USE_X86_ASM)

/**
 * Perform platform-specific GL API entry-point fixups.
 */
static void
init_glapi_relocs( void )
{
#if !defined(GLX_X86_READONLY_TEXT)
    extern unsigned long _x86_get_dispatch(void);
    char run_time_patch[] = {
       0x65, 0xa1, 0, 0, 0, 0 /* movl %gs:0,%eax */
    };
    GLuint *offset = (GLuint *) &run_time_patch[2]; /* 32-bits for x86/32 */
    const GLubyte * const get_disp = (const GLubyte *) run_time_patch;
    GLubyte * curr_func = (GLubyte *) gl_dispatch_functions_start;

    *offset = _x86_get_dispatch();
    while ( curr_func != (GLubyte *) gl_dispatch_functions_end ) {
	(void) memcpy( curr_func, get_disp, sizeof(run_time_patch));
	curr_func += DISPATCH_FUNCTION_SIZE;
    }
#endif
}

#elif defined(USE_SPARC_ASM)

extern void __glapi_sparc_icache_flush(unsigned int *);

static void
init_glapi_relocs( void )
{
    static const unsigned int template[] = {
	0x05000000, /* sethi %hi(_glapi_tls_Dispatch), %g2 */
	0x8730e00a, /* srl %g3, 10, %g3 */
	0x8410a000, /* or %g2, %lo(_glapi_tls_Dispatch), %g2 */
#ifdef __arch64__
	0xc259c002, /* ldx [%g7 + %g2], %g1 */
	0xc2584003, /* ldx [%g1 + %g3], %g1 */
#else
	0xc201c002, /* ld [%g7 + %g2], %g1 */
	0xc2004003, /* ld [%g1 + %g3], %g1 */
#endif
	0x81c04000, /* jmp %g1 */
	0x01000000, /* nop  */
    };
    extern unsigned int __glapi_sparc_tls_stub;
    extern unsigned long __glapi_sparc_get_dispatch(void);
    unsigned int *code = &__glapi_sparc_tls_stub;
    unsigned long dispatch = __glapi_sparc_get_dispatch();

    code[0] = template[0] | (dispatch >> 10);
    code[1] = template[1];
    __glapi_sparc_icache_flush(&code[0]);
    code[2] = template[2] | (dispatch & 0x3ff);
    code[3] = template[3];
    __glapi_sparc_icache_flush(&code[2]);
    code[4] = template[4];
    code[5] = template[5];
    __glapi_sparc_icache_flush(&code[4]);
    code[6] = template[6];
    __glapi_sparc_icache_flush(&code[6]);
}


#else /* USE_*_ASM */

static void
init_glapi_relocs( void )
{
}

#endif /* USE_*_ASM */


void
init_glapi_relocs_once( void )
{
   static once_flag flag = ONCE_FLAG_INIT;
   call_once(&flag, init_glapi_relocs);
}
