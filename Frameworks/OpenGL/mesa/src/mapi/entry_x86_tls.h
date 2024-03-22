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

#include <string.h>

#ifdef __CET__
#define ENDBR "endbr32\n\t"
#else
#define ENDBR
#endif

#ifdef HAVE_FUNC_ATTRIBUTE_VISIBILITY
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

#define X86_ENTRY_SIZE 32

__asm__(".text");

__asm__("x86_current_tls:\n\t"
	"call 1f\n"
        "1:\n\t"
        "popl %eax\n\t"
	"addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %eax\n\t"
	"movl _glapi_tls_Dispatch@GOTNTPOFF(%eax), %eax\n\t"
	"ret");

#ifndef GLX_X86_READONLY_TEXT
__asm__(".section wtext, \"awx\", @progbits");
#endif /* GLX_X86_READONLY_TEXT */

__asm__(".balign 16\n"
        "x86_entry_start:");

#define STUB_ASM_ENTRY(func)     \
   ".globl " func "\n"           \
   ".type " func ", @function\n" \
   ".balign 16\n"                \
   func ":"

#define STUB_ASM_CODE(slot)                                 \
   ENDBR                                                    \
   "call 1f\n"                                              \
   "1:\n\t"                                                 \
   "popl %eax\n\t"                                          \
   "addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %eax\n\t"           \
   "movl _glapi_tls_Dispatch@GOTNTPOFF(%eax), %eax\n\t" \
   "movl %gs:(%eax), %eax\n\t"                              \
   "jmp *(4 * " slot ")(%eax)"

#define MAPI_TMP_STUB_ASM_GCC
#include "mapi_tmp.h"

#ifndef GLX_X86_READONLY_TEXT
__asm__(".balign 16\n"
        "x86_entry_end:");
__asm__(".text");
#endif /* GLX_X86_READONLY_TEXT */

#ifndef MAPI_MODE_BRIDGE

extern unsigned long
x86_current_tls();

extern char x86_entry_start[] HIDDEN;
extern char x86_entry_end[] HIDDEN;

static inline mapi_func
entry_generate_or_patch(int, char *, size_t);

void
entry_patch_public(void)
{
#ifndef GLX_X86_READONLY_TEXT
   char *entry;
   int slot = 0;
   for (entry = x86_entry_start; entry < x86_entry_end;
        entry += X86_ENTRY_SIZE, ++slot)
      entry_generate_or_patch(slot, entry, X86_ENTRY_SIZE);
#endif
}

mapi_func
entry_get_public(int slot)
{
   return (mapi_func) (x86_entry_start + slot * X86_ENTRY_SIZE);
}

static void
entry_patch(mapi_func entry, int slot)
{
   char *code = (char *) entry;
   *((unsigned long *) (code + 8)) = slot * sizeof(mapi_func);
}

static inline mapi_func
entry_generate_or_patch(int slot, char *code, size_t size)
{
   const char code_templ[16] = {
      0x65, 0xa1, 0x00, 0x00, 0x00, 0x00, /* movl %gs:0x0, %eax */
      0xff, 0xa0, 0x34, 0x12, 0x00, 0x00, /* jmp *0x1234(%eax) */
      0x90, 0x90, 0x90, 0x90              /* nop's */
   };
   mapi_func entry;

   if (size < sizeof(code_templ))
      return NULL;

   memcpy(code, code_templ, sizeof(code_templ));

   *((unsigned long *) (code + 2)) = x86_current_tls();
   entry = (mapi_func) code;
   entry_patch(entry, slot);

   return entry;
}

#endif /* MAPI_MODE_BRIDGE */
