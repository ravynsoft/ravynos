/*
 * Copyright © 2013-2015 Intel Corporation
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

#include "brw_vec4_surface_builder.h"

using namespace brw;

namespace {
   namespace array_utils {
      /**
       * Copy one every \p src_stride logical components of the argument into
       * one every \p dst_stride logical components of the result.
       */
      static src_reg
      emit_stride(const vec4_builder &bld, const src_reg &src, unsigned size,
                  unsigned dst_stride, unsigned src_stride)
      {
         if (src_stride == 1 && dst_stride == 1) {
            return src;
         } else {
            const dst_reg dst = bld.vgrf(src.type,
                                         DIV_ROUND_UP(size * dst_stride, 4));

            for (unsigned i = 0; i < size; ++i)
               bld.MOV(writemask(offset(dst, 8, i * dst_stride / 4),
                                 1 << (i * dst_stride % 4)),
                       swizzle(offset(src, 8, i * src_stride / 4),
                               brw_swizzle_for_mask(1 << (i * src_stride % 4))));

            return src_reg(dst);
         }
      }

      /**
       * Convert a VEC4 into an array of registers with the layout expected by
       * the recipient shared unit.  If \p has_simd4x2 is true the argument is
       * left unmodified in SIMD4x2 form, otherwise it will be rearranged into
       * a SIMD8 vector.
       */
      static src_reg
      emit_insert(const vec4_builder &bld, const src_reg &src,
                  unsigned n, bool has_simd4x2)
      {
         if (src.file == BAD_FILE || n == 0) {
            return src_reg();

         } else {
            /* Pad unused components with zeroes. */
            const unsigned mask = (1 << n) - 1;
            const dst_reg tmp = bld.vgrf(src.type);

            bld.MOV(writemask(tmp, mask), src);
            if (n < 4)
               bld.MOV(writemask(tmp, ~mask), brw_imm_d(0));

            return emit_stride(bld, src_reg(tmp), n, has_simd4x2 ? 1 : 4, 1);
         }
      }
   }
}

namespace brw {
   namespace surface_access {
      namespace {
         using namespace array_utils;

         /**
          * Generate a send opcode for a surface message and return the
          * result.
          */
         src_reg
         emit_send(const vec4_builder &bld, enum opcode op,
                   const src_reg &header,
                   const src_reg &addr, unsigned addr_sz,
                   const src_reg &src, unsigned src_sz,
                   const src_reg &surface,
                   unsigned arg, unsigned ret_sz,
                   brw_predicate pred = BRW_PREDICATE_NONE)
         {
            /* Calculate the total number of components of the payload. */
            const unsigned header_sz = (header.file == BAD_FILE ? 0 : 1);
            const unsigned sz = header_sz + addr_sz + src_sz;

            /* Construct the payload. */
            const dst_reg payload = bld.vgrf(BRW_REGISTER_TYPE_UD, sz);
            unsigned n = 0;

            if (header_sz)
               bld.exec_all().MOV(offset(payload, 8, n++),
                                  retype(header, BRW_REGISTER_TYPE_UD));

            for (unsigned i = 0; i < addr_sz; i++)
               bld.MOV(offset(payload, 8, n++),
                       offset(retype(addr, BRW_REGISTER_TYPE_UD), 8, i));

            for (unsigned i = 0; i < src_sz; i++)
               bld.MOV(offset(payload, 8, n++),
                       offset(retype(src, BRW_REGISTER_TYPE_UD), 8, i));

            /* Reduce the dynamically uniform surface index to a single
             * scalar.
             */
            const src_reg usurface = bld.emit_uniformize(surface);

            /* Emit the message send instruction. */
            const dst_reg dst = bld.vgrf(BRW_REGISTER_TYPE_UD, ret_sz);
            vec4_instruction *inst =
               bld.emit(op, dst, src_reg(payload), usurface, brw_imm_ud(arg));
            inst->mlen = sz;
            inst->size_written = ret_sz * REG_SIZE;
            inst->header_size = header_sz;
            inst->predicate = pred;

            return src_reg(dst);
         }
      }

      /**
       * Emit an untyped surface read opcode.  \p dims determines the number
       * of components of the address and \p size the number of components of
       * the returned value.
       */
      src_reg
      emit_untyped_read(const vec4_builder &bld,
                        const src_reg &surface, const src_reg &addr,
                        unsigned dims, unsigned size,
                        brw_predicate pred)
      {
         return emit_send(bld, VEC4_OPCODE_UNTYPED_SURFACE_READ, src_reg(),
                          emit_insert(bld, addr, dims, true), 1,
                          src_reg(), 0,
                          surface, size, 1, pred);
      }

      /**
       * Emit an untyped surface write opcode.  \p dims determines the number
       * of components of the address and \p size the number of components of
       * the argument.
       */
      void
      emit_untyped_write(const vec4_builder &bld, const src_reg &surface,
                         const src_reg &addr, const src_reg &src,
                         unsigned dims, unsigned size,
                         brw_predicate pred)
      {
         const bool has_simd4x2 = bld.shader->devinfo->verx10 == 75;
         emit_send(bld, VEC4_OPCODE_UNTYPED_SURFACE_WRITE, src_reg(),
                   emit_insert(bld, addr, dims, has_simd4x2),
                   has_simd4x2 ? 1 : dims,
                   emit_insert(bld, src, size, has_simd4x2),
                   has_simd4x2 ? 1 : size,
                   surface, size, 0, pred);
      }

      /**
       * Emit an untyped surface atomic opcode.  \p dims determines the number
       * of components of the address and \p rsize the number of components of
       * the returned value (either zero or one).
       */
      src_reg
      emit_untyped_atomic(const vec4_builder &bld,
                          const src_reg &surface, const src_reg &addr,
                          const src_reg &src0, const src_reg &src1,
                          unsigned dims, unsigned rsize, unsigned op,
                          brw_predicate pred)
      {
         const bool has_simd4x2 = bld.shader->devinfo->verx10 == 75;

         /* Zip the components of both sources, they are represented as the X
          * and Y components of the same vector.
          */
         const unsigned size = (src0.file != BAD_FILE) + (src1.file != BAD_FILE);
         const dst_reg srcs = bld.vgrf(BRW_REGISTER_TYPE_UD);

         if (size >= 1) {
            bld.MOV(writemask(srcs, WRITEMASK_X),
                    swizzle(src0, BRW_SWIZZLE_XXXX));
         }

         if (size >= 2) {
            bld.MOV(writemask(srcs, WRITEMASK_Y),
                    swizzle(src1, BRW_SWIZZLE_XXXX));
         }

         return emit_send(bld, VEC4_OPCODE_UNTYPED_ATOMIC, src_reg(),
                          emit_insert(bld, addr, dims, has_simd4x2),
                          has_simd4x2 ? 1 : dims,
                          emit_insert(bld, src_reg(srcs), size, has_simd4x2),
                          has_simd4x2 && size ? 1 : size,
                          surface, op, rsize, pred);
      }
   }
}
