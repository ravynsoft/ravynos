/*
 * Copyright © 2019 Intel Corporation
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * Macro and function definitions needed in order to use genxml.
 *
 * This should only be included in sources compiled per-generation.
 */

#include "iris_batch.h"

#define __gen_address_type struct iris_address
#define __gen_user_data struct iris_batch
#define __gen_combine_address iris_combine_address

static inline void *
__gen_get_batch_dwords(struct iris_batch *batch, unsigned dwords)
{
   return iris_get_command_space(batch, dwords * sizeof(uint32_t));
}

static inline struct iris_address
__gen_address_offset(struct iris_address addr, uint64_t offset)
{
   addr.offset += offset;
   return addr;
}

static uint64_t
__gen_combine_address(struct iris_batch *batch, void *location,
                      struct iris_address addr, uint32_t delta)
{
   uint64_t result = addr.offset + delta;

   if (addr.bo) {
      iris_use_pinned_bo(batch, addr.bo,
                         !iris_domain_is_read_only(addr.access), addr.access);
      /* Assume this is a general address, not relative to a base. */
      result += addr.bo->address;
   }

   return result;
}

static inline struct iris_address
__gen_get_batch_address(struct iris_batch *batch, void *location)
{
   unreachable("Not supported by iris");
}

#define __gen_address_type struct iris_address
#define __gen_user_data struct iris_batch

#define __genxml_cmd_length(cmd) cmd ## _length
#define __genxml_cmd_length_bias(cmd) cmd ## _length_bias
#define __genxml_cmd_header(cmd) cmd ## _header
#define __genxml_cmd_pack(cmd) cmd ## _pack
#define __genxml_reg_num(cmd) cmd ## _num

#include "genxml/genX_pack.h"
#include "genxml/gen_macros.h"
#include "genxml/genX_bits.h"

/* CS_GPR(15) is reserved for combining conditional rendering predicates
 * with GL_ARB_indirect_parameters draw number predicates.
 */
#define MI_BUILDER_NUM_ALLOC_GPRS 15
#include "common/mi_builder.h"

#define _iris_pack_command(batch, cmd, dst, name)                 \
   for (struct cmd name = { __genxml_cmd_header(cmd) },           \
        *_dst = (void *)(dst); __builtin_expect(_dst != NULL, 1); \
        ({ __genxml_cmd_pack(cmd)(batch, (void *)_dst, &name);    \
           _dst = NULL;                                           \
           }))

#define iris_pack_command(cmd, dst, name) \
   _iris_pack_command(NULL, cmd, dst, name)

#define _iris_pack_state(batch, cmd, dst, name)                   \
   for (struct cmd name = {},                                     \
        *_dst = (void *)(dst); __builtin_expect(_dst != NULL, 1); \
        __genxml_cmd_pack(cmd)(batch, (void *)_dst, &name),       \
        _dst = NULL)

#define iris_pack_state(cmd, dst, name)                           \
   _iris_pack_state(NULL, cmd, dst, name)

#define iris_emit_cmd(batch, cmd, name) \
   _iris_pack_command(batch, cmd, __gen_get_batch_dwords(batch, __genxml_cmd_length(cmd)), name)

#define iris_emit_dwords(batch, n) \
   __gen_get_batch_dwords(batch, n)

#define iris_emit_merge(batch, dwords0, dwords1, num_dwords)    \
   do {                                                         \
      uint32_t *dw = __gen_get_batch_dwords(batch, num_dwords); \
      for (uint32_t i = 0; i < num_dwords; i++)                 \
         dw[i] = (dwords0)[i] | (dwords1)[i];                   \
      VG(VALGRIND_CHECK_MEM_IS_DEFINED(dw, num_dwords));        \
   } while (0)

#define iris_emit_reg(batch, reg, name)                                 \
   for (struct reg name = {}, *_cont = (struct reg *)1; _cont != NULL;  \
        ({                                                              \
            uint32_t _dw[__genxml_cmd_length(reg)];                     \
            __genxml_cmd_pack(reg)(NULL, _dw, &name);                   \
            for (unsigned i = 0; i < __genxml_cmd_length(reg); i++) {   \
               iris_emit_cmd(batch, GENX(MI_LOAD_REGISTER_IMM), lri) {  \
                  lri.RegisterOffset   = __genxml_reg_num(reg);         \
                  lri.DataDWord        = _dw[i];                        \
               }                                                        \
            }                                                           \
           _cont = NULL;                                                \
         }))


/**
 * iris_address constructor helpers:
 *
 * When using these to construct a CSO, pass NULL for \p bo, and manually
 * pin the BO later.  Otherwise, genxml's address handling will add the
 * BO to the current batch's validation list at CSO creation time, rather
 * than at draw time as desired.
 */

UNUSED static struct iris_address
ro_bo(struct iris_bo *bo, uint64_t offset)
{
   return (struct iris_address) { .bo = bo, .offset = offset,
                                  .access = IRIS_DOMAIN_OTHER_READ };
}

UNUSED static struct iris_address
rw_bo(struct iris_bo *bo, uint64_t offset, enum iris_domain access)
{
   return (struct iris_address) { .bo = bo, .offset = offset,
                                  .access = access };
}
