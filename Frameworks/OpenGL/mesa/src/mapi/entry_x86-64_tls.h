/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
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
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifdef __CET__
#define ENDBR "endbr64\n\t"
#else
#define ENDBR
#endif

#ifdef HAVE_FUNC_ATTRIBUTE_VISIBILITY
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

__asm__(".text\n"
        ".balign 32\n"
        "x86_64_entry_start:");

#define STUB_ASM_ENTRY(func)                             \
   ".globl " func "\n"                                   \
   ".type " func ", @function\n"                         \
   ".balign 32\n"                                        \
   func ":"

#ifndef __ILP32__

#define STUB_ASM_CODE(slot)                              \
   ENDBR                                                 \
   "movq _glapi_tls_Dispatch@GOTTPOFF(%rip), %rax\n\t"  \
   "movq %fs:(%rax), %r11\n\t"                           \
   "jmp *(8 * " slot ")(%r11)"

#else

#define STUB_ASM_CODE(slot)                              \
   ENDBR                                                 \
   "movq _glapi_tls_Dispatch@GOTTPOFF(%rip), %rax\n\t"  \
   "movl %fs:(%rax), %r11d\n\t"                          \
   "movl 4*" slot "(%r11d), %r11d\n\t"                   \
   "jmp *%r11"

#endif

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

#ifndef MAPI_MODE_BRIDGE

#include <string.h>

void
entry_patch_public(void)
{
}

extern char
x86_64_entry_start[] HIDDEN;

mapi_func
entry_get_public(int slot)
{
   return (mapi_func) (x86_64_entry_start + slot * 32);
}


#endif /* MAPI_MODE_BRIDGE */
