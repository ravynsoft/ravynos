/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pvr_dump.h"
#include "pvr_util.h"
#include "util/u_math.h"

const struct pvr_dump_ctx __pvr_dump_ctx_invalid = {
   .active_child = &__pvr_dump_ctx_invalid,
};

/*****************************************************************************
   Hex dumps
*****************************************************************************/

#define HEX_WORD_SIZE ((unsigned)sizeof(uint32_t))
#define HEX_BYTE_FMT "%02" PRIx8

/* This must be even, and should probably always be a power of 2. */
#define HEX_LINE_SIZE (HEX_WORD_SIZE * 8)

struct pvr_dump_hex_ctx {
   struct pvr_dump_ctx base;

   const uint8_t *start_ptr;
   const uint8_t *end_ptr;
   uint64_t nr_bytes;
   uint32_t offset_digits;

   /* User-modifiable values */
   const uint8_t *line_ptr;

   uint32_t prev_non_zero_trailing_zero_bytes;
   uint64_t prev_non_zero_leading_zero_lines;
   const uint8_t *prev_non_zero_line;
   uint64_t zero_lines;
};

static bool pvr_dump_hex_ctx_push(struct pvr_dump_hex_ctx *const ctx,
                                  struct pvr_dump_buffer_ctx *const parent_ctx,
                                  const uint64_t nr_bytes)
{
   const uint64_t real_nr_bytes = nr_bytes ? nr_bytes
                                           : parent_ctx->remaining_size;
   bool ret;

   if (parent_ctx->remaining_size < nr_bytes)
      return false;

   ret = pvr_dump_ctx_push(&ctx->base, &parent_ctx->base);
   if (!ret)
      return false;

   ctx->start_ptr = parent_ctx->ptr;
   ctx->end_ptr = ctx->start_ptr + real_nr_bytes;
   ctx->nr_bytes = real_nr_bytes;
   ctx->offset_digits = u64_hex_digits(real_nr_bytes);

   ctx->line_ptr = ctx->start_ptr;

   ctx->prev_non_zero_trailing_zero_bytes = 0;
   ctx->prev_non_zero_leading_zero_lines = 0;
   ctx->prev_non_zero_line = NULL;
   ctx->zero_lines = 0;

   return true;
}

static struct pvr_dump_buffer_ctx *
pvr_dump_hex_ctx_pop(struct pvr_dump_hex_ctx *const ctx)
{
   struct pvr_dump_buffer_ctx *parent;
   struct pvr_dump_ctx *parent_base;

   if (ctx->line_ptr != ctx->end_ptr) {
      ctx->base.ok = false;
      return NULL;
   }

   parent_base = pvr_dump_ctx_pop(&ctx->base);
   if (!parent_base)
      return NULL;

   parent = container_of(parent_base, struct pvr_dump_buffer_ctx, base);

   pvr_dump_buffer_advance(parent, ctx->nr_bytes);

   return parent;
}

static inline void pvr_dump_hex_print_prefix(const struct pvr_dump_hex_ctx *ctx,
                                             const uint64_t offset)
{
   pvr_dump_printf(&ctx->base,
                   PVR_DUMP_OFFSET_PREFIX,
                   ctx->offset_digits,
                   offset);
}

#define pvr_dump_hex_println(ctx, offset, format, args...) \
   pvr_dump_println(&(ctx)->base,                          \
                    PVR_DUMP_OFFSET_PREFIX format,         \
                    (ctx)->offset_digits,                  \
                    offset,                                \
                    ##args)

#define pvr_dump_hex_println_no_prefix(ctx, format, args...) \
   pvr_dump_println(&(ctx)->base,                            \
                    "%*c" format,                            \
                    (ctx)->offset_digits + 3,                \
                    ' ',                                     \
                    ##args)

static void pvr_dump_hex_print_line(const struct pvr_dump_hex_ctx *ctx,
                                    const uint8_t *const line_ptr,
                                    const uint32_t truncate)
{
   const uint32_t nr_bytes =
      MIN2(HEX_LINE_SIZE - truncate, ctx->end_ptr - line_ptr);

   pvr_dump_hex_print_prefix(ctx, line_ptr - ctx->start_ptr);

   for (uint32_t i = 0; i < nr_bytes; i++) {
      if (i == HEX_LINE_SIZE / 2)
         pvr_dump_printf_cont(&ctx->base, " ");

      if (i % HEX_WORD_SIZE == 0)
         pvr_dump_printf_cont(&ctx->base, " ");

      if (line_ptr[i])
         pvr_dump_printf_cont(&ctx->base, HEX_BYTE_FMT, line_ptr[i]);
      else
         pvr_dump_printf_cont(&ctx->base, "..");
   }

   pvr_dump_print_eol(&ctx->base);
}

static void
pvr_dump_hex_print_zero_lines(const struct pvr_dump_hex_ctx *const ctx,
                              const uint64_t zero_lines)
{
   const uint64_t zero_bytes = zero_lines * HEX_LINE_SIZE;

   if (!zero_lines)
      return;

   /* If we've only buffered a single zero line, print it normally. We don't
    * save any space by folding it, and it's more readable this way.
    */
   if (zero_lines == 1) {
      pvr_dump_hex_print_line(ctx, ctx->prev_non_zero_line + HEX_LINE_SIZE, 0);
      return;
   }

   pvr_dump_hex_println_no_prefix(ctx,
                                  "  + %" PRIu64 " zero line%s (%" PRIu64
                                  "/0x%" PRIx64 " bytes)",
                                  zero_lines,
                                  zero_lines == 1 ? "" : "s",
                                  zero_bytes,
                                  zero_bytes);
}

static void
pvr_dump_hex_print_trailing_zeroes(const struct pvr_dump_hex_ctx *const ctx)
{
   const uint64_t zero_bytes =
      ctx->zero_lines * HEX_LINE_SIZE + ctx->prev_non_zero_trailing_zero_bytes;

   if (!ctx->prev_non_zero_trailing_zero_bytes)
      return pvr_dump_hex_print_zero_lines(ctx, ctx->zero_lines);

   if (!ctx->zero_lines)
      return;

   pvr_dump_hex_println_no_prefix(ctx,
                                  "  + %" PRIu64 "+%" PRIu32
                                  " zero lines (%" PRIu64 "/0x%" PRIx64
                                  " bytes)",
                                  ctx->zero_lines,
                                  ctx->prev_non_zero_trailing_zero_bytes,
                                  zero_bytes,
                                  zero_bytes);
}

static void pvr_dump_hex_process_line(struct pvr_dump_hex_ctx *const ctx,
                                      uint32_t truncate)
{
   const uint32_t max_bytes = HEX_LINE_SIZE - truncate;

   uint32_t trailing_zero_bytes = max_bytes;

   for (uint32_t i = max_bytes; i > 0; i--) {
      if (ctx->line_ptr[i - 1]) {
         trailing_zero_bytes = HEX_LINE_SIZE - i;
         break;
      }
   }

   if (trailing_zero_bytes == max_bytes) {
      /* No non-zero words were found in this line; mark it and move on. */
      ctx->zero_lines++;
      return;
   }

   /* We have at least one non-zero word in this line. If we have a previous
    * non-zero line stored, collapse and print any leading zero-only lines
    * before it then print the stored line.
    */
   if (ctx->prev_non_zero_line) {
      pvr_dump_hex_print_zero_lines(ctx, ctx->prev_non_zero_leading_zero_lines);
      pvr_dump_hex_print_line(ctx, ctx->prev_non_zero_line, truncate);
   }

   /* Now we store the current non-zero line for printing later. This way we
    * can treat the last non-zero line specially.
    */
   ctx->prev_non_zero_line = ctx->line_ptr;
   ctx->prev_non_zero_leading_zero_lines = ctx->zero_lines;
   ctx->prev_non_zero_trailing_zero_bytes = trailing_zero_bytes;
   ctx->zero_lines = 0;
}

static void pvr_dump_hex(struct pvr_dump_hex_ctx *const ctx)
{
   while (ctx->line_ptr < (ctx->end_ptr - HEX_LINE_SIZE)) {
      pvr_dump_hex_process_line(ctx, 0);
      ctx->line_ptr += HEX_LINE_SIZE;
   }

   pvr_dump_hex_process_line(ctx,
                             HEX_LINE_SIZE - (ctx->end_ptr - ctx->line_ptr));
   ctx->line_ptr = ctx->end_ptr;

   if (ctx->prev_non_zero_line) {
      /* If we don't have any zero lines to collapse, print the trailing zeroes
       * on the last line.
       */
      if (!ctx->zero_lines) {
         pvr_dump_hex_print_line(ctx, ctx->prev_non_zero_line, 0);
      } else {
         pvr_dump_hex_print_zero_lines(ctx,
                                       ctx->prev_non_zero_leading_zero_lines);

         pvr_dump_hex_print_line(ctx,
                                 ctx->prev_non_zero_line,
                                 ctx->prev_non_zero_trailing_zero_bytes);

         /* Collapse and print any trailing zeroes. */
         pvr_dump_hex_print_trailing_zeroes(ctx);
      }
   } else {
      /* We made it to the end of the buffer without ever encountering a
       * non-zero word. Make this known.
       */
      pvr_dump_hex_println(ctx, UINT64_C(0), " <empty buffer>");
   }

   pvr_dump_hex_println(ctx, ctx->nr_bytes, " <end of buffer>");
}

bool pvr_dump_buffer_hex(struct pvr_dump_buffer_ctx *const ctx,
                         const uint64_t nr_bytes)
{
   struct pvr_dump_hex_ctx hex_ctx;

   if (!pvr_dump_hex_ctx_push(&hex_ctx, ctx, nr_bytes))
      return false;

   pvr_dump_hex(&hex_ctx);

   return !!pvr_dump_hex_ctx_pop(&hex_ctx);
}
