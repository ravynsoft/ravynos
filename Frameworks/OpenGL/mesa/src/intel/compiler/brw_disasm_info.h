/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _INTEL_ASM_ANNOTATION_H
#define _INTEL_ASM_ANNOTATION_H

#include "compiler/glsl/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cfg_t;
struct backend_instruction;
struct intel_device_info;

struct inst_group {
   struct exec_node link;

   int offset;

   size_t error_length;
   char *error;

   /* Pointers to the basic block in the CFG if the instruction group starts
    * or ends a basic block.
    */
   struct bblock_t *block_start;
   struct bblock_t *block_end;

   /* Annotation for the generated IR.  One of the two can be set. */
   const void *ir;
   const char *annotation;
};

struct disasm_info {
   struct exec_list group_list;

   const struct brw_isa_info *isa;
   const struct cfg_t *cfg;

   /** Block index in the cfg. */
   int cur_block;
   bool use_tail;
};

void
dump_assembly(void *assembly, int start_offset, int end_offset,
              struct disasm_info *disasm, const unsigned *block_latency);

struct disasm_info *
disasm_initialize(const struct brw_isa_info *isa,
                  const struct cfg_t *cfg);

struct inst_group *
disasm_new_inst_group(struct disasm_info *disasm, unsigned offset);

void
disasm_annotate(struct disasm_info *disasm,
                struct backend_instruction *inst, unsigned offset);

void
disasm_insert_error(struct disasm_info *disasm, unsigned offset,
                    unsigned inst_size, const char *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _INTEL_ASM_ANNOTATION_H */
