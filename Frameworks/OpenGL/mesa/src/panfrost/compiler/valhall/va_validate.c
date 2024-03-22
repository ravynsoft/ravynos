/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_builder.h"
#include "va_compiler.h"
#include "valhall.h"

/* Valhall has limits on access to fast-access uniforms:
 *
 *   An instruction may access no more than a single 64-bit uniform slot.
 *   An instruction may access no more than 64-bits of combined uniforms and
 * constants. An instruction may access no more than a single special immediate
 * (e.g. lane_id).
 *
 * We validate these constraints.
 *
 * An instruction may only access a single page of (special or uniform) FAU.
 * This constraint does not need explicit validation: since FAU slots are
 * naturally aligned, they never cross page boundaries, so this condition is
 * implied by only acesssing a single 64-bit slot.
 */

struct fau_state {
   signed uniform_slot;
   bi_index buffer[2];
};

static bool
fau_state_buffer(struct fau_state *fau, bi_index idx)
{
   for (unsigned i = 0; i < ARRAY_SIZE(fau->buffer); ++i) {
      if (bi_is_word_equiv(fau->buffer[i], idx))
         return true;
      else if (bi_is_null(fau->buffer[i])) {
         fau->buffer[i] = idx;
         return true;
      }
   }

   return false;
}

static bool
fau_state_uniform(struct fau_state *fau, bi_index idx)
{
   /* Each slot is 64-bits. The low/high half is encoded as the offset of the
    * bi_index, which we want to ignore.
    */
   unsigned slot = (idx.value & 63);

   if (fau->uniform_slot < 0)
      fau->uniform_slot = slot;

   return fau->uniform_slot == slot;
}

static bool
fau_is_special(enum bir_fau fau)
{
   return !(fau & (BIR_FAU_UNIFORM | BIR_FAU_IMMEDIATE));
}

static bool
fau_state_special(struct fau_state *fau, bi_index idx)
{
   for (unsigned i = 0; i < ARRAY_SIZE(fau->buffer); ++i) {
      bi_index buf = fau->buffer[i];
      bool special = !bi_is_null(buf) && fau_is_special(buf.value);

      if (special && !bi_is_equiv(buf, idx))
         return false;
   }

   return true;
}

static bool
valid_src(struct fau_state *fau, unsigned fau_page, bi_index src)
{
   if (src.type != BI_INDEX_FAU)
      return true;

   bool valid = (fau_page == va_fau_page(src.value));
   valid &= fau_state_buffer(fau, src);

   if (src.value & BIR_FAU_UNIFORM)
      valid &= fau_state_uniform(fau, src);
   else if (fau_is_special(src.value))
      valid &= fau_state_special(fau, src);

   return valid;
}

bool
va_validate_fau(bi_instr *I)
{
   bool valid = true;
   struct fau_state fau = {.uniform_slot = -1};
   unsigned fau_page = va_select_fau_page(I);

   bi_foreach_src(I, s) {
      valid &= valid_src(&fau, fau_page, I->src[s]);
   }

   return valid;
}

void
va_repair_fau(bi_builder *b, bi_instr *I)
{
   struct fau_state fau = {.uniform_slot = -1};
   unsigned fau_page = va_select_fau_page(I);

   bi_foreach_src(I, s) {
      struct fau_state push = fau;
      bi_index src = I->src[s];

      if (!valid_src(&fau, fau_page, src)) {
         bi_replace_src(I, s, bi_mov_i32(b, bi_strip_index(src)));

         /* Rollback update. Since the replacement move doesn't affect FAU
          * state, there is no need to call valid_src again.
          */
         fau = push;
      }
   }
}

void
va_validate(FILE *fp, bi_context *ctx)
{
   bool errors = false;

   bi_foreach_instr_global(ctx, I) {
      if (!va_validate_fau(I)) {
         if (!errors) {
            fprintf(fp, "Validation failed, this is a bug. Shader:\n\n");
            bi_print_shader(ctx, fp);
            fprintf(fp, "Offending code:\n");
         }

         bi_print_instr(I, fp);
         fprintf(fp, "\n");
         errors = true;
      }
   }

   if (errors)
      exit(1);
}
