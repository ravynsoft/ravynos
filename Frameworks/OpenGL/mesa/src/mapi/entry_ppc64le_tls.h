/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2017 Red Hat
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Ben Crocker <bcrocker@redhat.com>
 */

#ifdef HAVE_FUNC_ATTRIBUTE_VISIBILITY
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

// NOTE: These must be powers of two:
#define PPC64LE_ENTRY_SIZE 64
#define PPC64LE_PAGE_ALIGN 65536
#if ((PPC64LE_ENTRY_SIZE & (PPC64LE_ENTRY_SIZE - 1)) != 0)
#error PPC64LE_ENTRY_SIZE must be a power of two!
#endif
#if ((PPC64LE_PAGE_ALIGN & (PPC64LE_PAGE_ALIGN - 1)) != 0)
#error PPC64LE_PAGE_ALIGN must be a power of two!
#endif

__asm__(".text\n"
        ".balign " U_STRINGIFY(PPC64LE_ENTRY_SIZE) "\n"
        "ppc64le_entry_start:");

#define STUB_ASM_ENTRY(func)                            \
   ".globl " func "\n"                                  \
   ".type " func ", @function\n"                        \
   ".balign " U_STRINGIFY(PPC64LE_ENTRY_SIZE) "\n"        \
   func ":\n\t"                                         \
   "  addis  2, 12, .TOC.-" func "@ha\n\t"              \
   "  addi   2, 2, .TOC.-" func "@l\n\t"                \
   "  .localentry  " func ", .-" func "\n\t"

#define STUB_ASM_CODE(slot)                                     \
   "  addis  11, 2, _glapi_tls_Dispatch@got@tprel@ha\n\t"   \
   "  ld     11, _glapi_tls_Dispatch@got@tprel@l(11)\n\t"   \
   "  add    11, 11,_glapi_tls_Dispatch@tls\n\t"            \
   "  ld     11, 0(11)\n\t"                                     \
   "  ld     12, " slot "*8(11)\n\t"                            \
   "  mtctr  12\n\t"                                            \
   "  bctr\n"                                                   \

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

#ifndef MAPI_MODE_BRIDGE

#include <string.h>

void
entry_patch_public(void)
{
}

extern char
ppc64le_entry_start[] HIDDEN;

mapi_func
entry_get_public(int slot)
{
   return (mapi_func) (ppc64le_entry_start + slot * PPC64LE_ENTRY_SIZE);
}

#endif /* MAPI_MODE_BRIDGE */
