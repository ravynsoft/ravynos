/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * ShaderDump.c --
 *    Functions for printing out shaders.
 */

#include "DriverIncludes.h"

#include "ShaderDump.h"
#include "util/u_debug.h"


static void
dump_uints(const unsigned *data,
           unsigned count)
{
   unsigned i;

   for (i = 0; i < count; i++) {
      if (i % 8 == 7) {
         debug_printf("0x%08x,\n", data[i]);
      } else {
         debug_printf("0x%08x, ", data[i]);
      }
   }
   if (i % 8) {
      debug_printf("\n");
   }
}

void
dx10_shader_dump_binary(const unsigned *code)
{
   dump_uints(code, code[1]);
}


void
dx10_shader_dump_tokens(const unsigned *code)
{
   /*
    * TODO: Dump SM4/5 disassembly via D3DDisassemble.  However this requires
    * rebuilding DXBC container.
    */
   dx10_shader_dump_binary(code);
}
