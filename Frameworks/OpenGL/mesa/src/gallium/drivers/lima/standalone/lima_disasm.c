/*
 * Copyright (c) 2019 Vasily Khoruzhick <anarsoul@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "util/ralloc.h"

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ir/pp/codegen.h"
#include "ir/gp/codegen.h"

static void
print_usage(void)
{
   printf("Usage: lima_disasm [OPTIONS]... FILE\n");
   printf("    --help            - show this message\n");
}

typedef struct __attribute__((__packed__)) {
   char name[4];
   uint32_t size;
} mbs_chunk;

/* Parses an MBS1 file. MBS1 is used for Mali-400 and earlier which only support
 * GLES2, as opposed to MBS2 which is used by later Mali gens, and contains
 * the entire inferface between the compiler and the (blob) driver. It's
 * produced by the offline compiler as well as glGetProgramBinary(). The
 * format is documented at
 * https://web.archive.org/web/20171026141029/http://limadriver.org/MBS+File+Format/
 * and consists of a bunch of nested "chunks" where each chunk has a
 * 4-character tag followed by a 32-bit size, then the contents of the chunk.
 * The chunks are nested as follows:
 *
 * - MBS1
 *   - optional CFRA (fragment shader)
 *     - core version (uint32_t, Mali-200 vs Mali-400)
 *     - FSTA (Fragment STAck information)
 *     - FDIS (if Fragment shader contains a DIScard instruction)
 *     - FBUU (information on color/depth reads/writes)
 *     - SUNI (uniform symbol table)
 *     - SVAR (varying symbol table)
 *     - DBIN (the actual code)
 *   - optional CVER (vertex shader)
 *     - core version (uint32_t, GP2 vs Mali-400)
 *     - FINS (# of instruction and attrib_prefetch)
 *     - SUNI (uniform table)
 *     - SATT (attribute table)
 *     - SVAR (varying table)
 *     - DBIN (the actual code)
 *
 * This routine just finds the DBIN chunk and returns the binary assuming
 * there's only the fragment or vertex shader. We don't bother to parse the
 * other stuff yet.
 */
static uint32_t *
extract_shader_binary(char *filename, uint32_t *size, bool *is_frag)
{
   mbs_chunk chunk;

   if (!filename || !size || !is_frag)
      return NULL;

   FILE *in = fopen(filename, "rb");
   if (!in)
      return NULL;

   if (!fread(&chunk, sizeof(chunk), 1, in)) {
      printf("Failed to read MBS1 segment\n");
      return NULL;
   }

   if (strncmp(chunk.name, "MBS1", 4)) {
      printf("File is not MBS\n");
      return NULL;
   }

   if (!fread(&chunk, sizeof(chunk), 1, in)) {
      printf("Failed to read shader segment\n");
      return NULL;
   }

   if (!strncmp(chunk.name, "CFRA", 4)) {
      *is_frag = true;
   } else if (!strncmp(chunk.name, "CVER", 4)) {
      *is_frag = false;
   } else {
      printf("Unsupported shader type\n");
      return NULL;
   }

   /* Skip version */
   fseek(in, 4, SEEK_CUR);

   /* Skip the other chunks and find the DBIN chunk. */
   do {
      if (!fread(&chunk, sizeof(chunk), 1, in)) {
         printf("Failed to read segment\n");
         return NULL;
      }
      if (!strncmp(chunk.name, "DBIN", 4))
         break;
      fseek(in, chunk.size, SEEK_CUR);
   } while (!feof(in));

   if (feof(in)) {
      printf("CBIN segment not found!\n");
      return NULL;
   }

   *size = chunk.size;

   uint32_t *bin = ralloc_size(NULL, chunk.size);
   if (!bin) {
      printf("Failed to allocate shader binary\n");
      return NULL;
   }

   if (!fread(bin, chunk.size, 1, in)) {
      printf("Failed to read shader binary\n");
      ralloc_free(bin);
      bin = NULL;
   }

   return bin;
}

int
main(int argc, char **argv)
{
   int n;
   bool is_frag = true;

   if (argc < 2) {
      print_usage();
      return 1;
   }

   for (n = 1; n < argc; n++) {
      if (!strcmp(argv[n], "--help")) {
         print_usage();
         return 1;
      }
   }

   char *filename = NULL;
   filename = argv[argc - 1];

   uint32_t size = 0;
   uint32_t *prog = extract_shader_binary(filename, &size, &is_frag);
   if (!prog) {
      printf("Failed to parse mbs!\n");
      return -1;
   }

   if (is_frag) {
      assert((size & 0x3) == 0);
      size >>= 2;
      uint32_t *bin = prog;
      uint32_t offset = 0;
      do {
         ppir_codegen_ctrl *ctrl = (ppir_codegen_ctrl *)bin;
         printf("@%6d: ", offset);
         ppir_disassemble_instr(bin, offset, stdout);
         bin += ctrl->count;
         offset += ctrl->count;
         size -= ctrl->count;
      } while (size);
   } else {
      gpir_disassemble_program((gpir_codegen_instr *)prog, size / (sizeof(gpir_codegen_instr)), stdout);
   }

   ralloc_free(prog);

   return 0;
}

