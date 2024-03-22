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

#ifndef PVR_DUMP_H
#define PVR_DUMP_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pvr_types.h"
#include "pvr_util.h"
#include "util/macros.h"
#include "util/u_math.h"

/** BASIC PRINTING **/

#define PVR_DUMP_OFFSET_PREFIX "[%0*" PRIx64 "] "

/** CONTEXTS **/

#define PVR_DUMP_INDENT_SIZE 2U
#define PVR_DUMP_FIELD_COLUMN_WIDTH 36U

/* This is an invalid context used to permanently mark popped contexts as
 * unusable. All operations on a context check that it's the "top" context
 * by ensuring it has no active child. The only way to remove the active child
 * of a context is by popping the active child directly. Assigning an invalid
 * context as the active child of a context therefore makes it impossible to
 * use.
 */
extern const struct pvr_dump_ctx __pvr_dump_ctx_invalid;

struct pvr_dump_ctx {
   /* This is const because only the "top" context should ever be modified. It's
    * fine to extract information from the parent context, but not to modify it.
    * There is *one* exception: pvr_dump_ctx_pop() must cast away the const to
    * return the parent context as the new "top" context. This is considered
    * sound because the parent context was not const when assigned here in
    * pvr_dump_ctx_push().
    */
   const struct pvr_dump_ctx *parent;

   /* This is const because it's not meant to be used for access - it's just a
    * way of checking if this context is the "top" context (see the comment on
    * __pvr_dump_ctx_invalid for more details). Unlike parent, the const
    * qualifier here should never be cast away.
    */
   const struct pvr_dump_ctx *active_child;

   FILE *file;
   const char *name;

   uint32_t allowed_child_depth;
   uint32_t parent_indent;

   /* User-modifiable values */
   uint32_t indent;
   bool ok;
};

static inline uint32_t
__pvr_dump_ctx_get_indent(const struct pvr_dump_ctx *const ctx)
{
   return (ctx->parent_indent + ctx->indent) * PVR_DUMP_INDENT_SIZE;
}

struct pvr_dump_buffer_ctx {
   struct pvr_dump_ctx base;

   const void *initial_ptr;
   uint64_t capacity;

   /* User-modifiable values */
   const void *ptr;
   uint64_t remaining_size;
};

#define pvr_dump_printf(ctx, format, args...)           \
   pvr_dump_printf_cont(ctx,                            \
                        "%*s" format,                   \
                        __pvr_dump_ctx_get_indent(ctx), \
                        "",                             \
                        ##args)

/* Same as pvr_dump_printf(), but with no indent.
 * Intended for continuation lines.
 */
#define pvr_dump_printf_cont(ctx, format, args...) \
   fprintf((ctx)->file, format, ##args)

#define pvr_dump_println(ctx, format, args...) \
   pvr_dump_printf(ctx, format "\n", ##args)

#define pvr_dump_println_cont(ctx, format, args...) \
   pvr_dump_printf_cont(ctx, format "\n", ##args)

#define pvr_dump_print_eol(ctx) fprintf((ctx)->file, "\n")

#define pvr_dump_mark_section(ctx, format, args...)                \
   do {                                                            \
      pvr_dump_print_eol(ctx);                                     \
      pvr_dump_println(ctx, "------- " format " -------", ##args); \
   } while (0)

#define pvr_dump_buffer_print_header_prefix(ctx)                            \
   do {                                                                     \
      struct pvr_dump_buffer_ctx *_prefix_ctx = (ctx);                      \
      pvr_dump_printf(&_prefix_ctx->base,                                   \
                      PVR_DUMP_OFFSET_PREFIX,                               \
                      u64_dec_digits(_prefix_ctx->capacity),                \
                      _prefix_ctx->capacity - _prefix_ctx->remaining_size); \
   } while (0)

#define pvr_dump_buffer_print_header_line(ctx, format, args...) \
   do {                                                         \
      struct pvr_dump_buffer_ctx *_ctx = (ctx);                 \
      pvr_dump_buffer_print_header_prefix(_ctx);                \
      pvr_dump_printf_cont(&_ctx->base, format "\n", ##args);   \
   } while (0)

#define pvr_dump_msg(ctx, prefix, ret, format, args...)            \
   ({                                                              \
      bool _ret = (ret);                                           \
      struct pvr_dump_ctx *_ctx = (ctx);                           \
      pvr_dump_println(_ctx, "<!" prefix "! " format ">", ##args); \
      if (!_ret)                                                   \
         _ctx->ok = _ret;                                          \
      _ret;                                                        \
   })

#define pvr_dump_error(ctx, format, args...) \
   pvr_dump_msg(ctx, "ERROR", false, format, ##args)

#define pvr_dump_warn(ctx, format, args...) \
   pvr_dump_msg(ctx, "WARN", true, format, ##args)

static inline bool pvr_dump_ctx_require_top(struct pvr_dump_ctx *const ctx)
{
   if (ctx->active_child != NULL)
      return pvr_dump_error(ctx, "use of non-top context");

   return true;
}

static inline void pvr_dump_indent(struct pvr_dump_ctx *const ctx)
{
   ctx->indent++;
}

static inline void pvr_dump_dedent(struct pvr_dump_ctx *const ctx)
{
   if (ctx->indent)
      ctx->indent--;
}

static inline void __pvr_dump_ctx_init(struct pvr_dump_ctx *const ctx,
                                       const struct pvr_dump_ctx *const parent,
                                       FILE *const file,
                                       const char *const name,
                                       const uint32_t allowed_child_depth,
                                       const uint32_t parent_indent)
{
   ctx->parent = parent;
   ctx->active_child = NULL;

   ctx->file = file;
   ctx->name = name;

   ctx->allowed_child_depth = allowed_child_depth;
   ctx->parent_indent = parent_indent;
   ctx->indent = 0;
   ctx->ok = true;
}

static inline void __pvr_dump_ctx_mark_popped(struct pvr_dump_ctx *const ctx)
{
   ctx->active_child = &__pvr_dump_ctx_invalid;
}

static inline void pvr_dump_begin(struct pvr_dump_ctx *const root_ctx,
                                  FILE *const file,
                                  const char *const name,
                                  const uint32_t max_depth)
{
   __pvr_dump_ctx_init(root_ctx, NULL, file, name, max_depth, 0);

   flockfile(file);
   pvr_dump_println(root_ctx, "======= BEGIN %s =======", name);
}

static inline bool pvr_dump_end(struct pvr_dump_ctx *const root_ctx)
{
   /* In order to end a dump, we must be in a root context (no parent) and have
    * no active child context.
    */
   if (!pvr_dump_ctx_require_top(root_ctx))
      return false;

   if (root_ctx->parent)
      return pvr_dump_error(root_ctx, "ending non-root context");

   pvr_dump_println(root_ctx, "======= END %s =======", root_ctx->name);
   funlockfile(root_ctx->file);

   __pvr_dump_ctx_mark_popped(root_ctx);

   return true;
}

static inline bool pvr_dump_ctx_push(struct pvr_dump_ctx *const ctx,
                                     struct pvr_dump_ctx *const parent_ctx)
{
   if (!parent_ctx->ok)
      return false;

   if (!parent_ctx->allowed_child_depth)
      return pvr_dump_error(parent_ctx, "context stack depth limit reached");

   __pvr_dump_ctx_init(ctx,
                       parent_ctx,
                       parent_ctx->file,
                       parent_ctx->name,
                       parent_ctx->allowed_child_depth - 1,
                       parent_ctx->parent_indent + parent_ctx->indent);

   parent_ctx->active_child = ctx;

   return true;
}

static inline struct pvr_dump_ctx *
pvr_dump_ctx_pop(struct pvr_dump_ctx *const ctx)
{
   struct pvr_dump_ctx *const parent = (struct pvr_dump_ctx *)ctx->parent;

   if (!pvr_dump_ctx_require_top(ctx))
      return NULL;

   if (!parent) {
      pvr_dump_error(ctx, "popped root context");
      return NULL;
   }

   parent->active_child = NULL;

   __pvr_dump_ctx_mark_popped(ctx);

   return parent;
}

static inline bool
pvr_dump_buffer_ctx_push(struct pvr_dump_buffer_ctx *const ctx,
                         struct pvr_dump_ctx *const parent_ctx,
                         const void *const initial_ptr,
                         const uint64_t size)
{
   if (!pvr_dump_ctx_push(&ctx->base, parent_ctx))
      return false;

   ctx->initial_ptr = initial_ptr;
   ctx->capacity = size;

   ctx->ptr = initial_ptr;
   ctx->remaining_size = size;

   return true;
}

static inline struct pvr_dump_ctx *
pvr_dump_buffer_ctx_pop(struct pvr_dump_buffer_ctx *const ctx)
{
   return pvr_dump_ctx_pop(&ctx->base);
}

bool pvr_dump_buffer_hex(struct pvr_dump_buffer_ctx *ctx, uint64_t nr_bytes);

static inline void __pvr_dump_buffer_advance(struct pvr_dump_buffer_ctx *ctx,
                                             const uint64_t nr_bytes)
{
   ctx->ptr = (uint8_t *)ctx->ptr + nr_bytes;
   ctx->remaining_size -= nr_bytes;
}

static inline bool pvr_dump_buffer_advance(struct pvr_dump_buffer_ctx *ctx,
                                           const uint64_t nr_bytes)
{
   if (!ctx->base.ok || !pvr_dump_ctx_require_top(&ctx->base))
      return false;

   if (nr_bytes > ctx->remaining_size)
      return pvr_dump_error(&ctx->base, "advanced past end of context buffer");

   __pvr_dump_buffer_advance(ctx, nr_bytes);

   return true;
}

static inline void __pvr_dump_buffer_rewind(struct pvr_dump_buffer_ctx *ctx,
                                            const uint32_t nr_bytes)
{
   ctx->ptr = (uint8_t *)ctx->ptr - nr_bytes;
   ctx->remaining_size += nr_bytes;
}

static inline bool pvr_dump_buffer_rewind(struct pvr_dump_buffer_ctx *ctx,
                                          const uint32_t nr_bytes)
{
   if (!ctx->base.ok || !pvr_dump_ctx_require_top(&ctx->base))
      return false;

   if (nr_bytes > ctx->capacity - ctx->remaining_size)
      return pvr_dump_error(&ctx->base, "rewound past start of context buffer");

   __pvr_dump_buffer_rewind(ctx, nr_bytes);

   return true;
}

static inline bool pvr_dump_buffer_truncate(struct pvr_dump_buffer_ctx *ctx,
                                            const uint64_t remaining_size)
{
   if (!ctx->base.ok || !pvr_dump_ctx_require_top(&ctx->base))
      return false;

   if (remaining_size > ctx->remaining_size)
      return pvr_dump_error(&ctx->base, "truncated to larger size");

   ctx->remaining_size = remaining_size;

   return true;
}

static inline const void *restrict
pvr_dump_buffer_peek(struct pvr_dump_buffer_ctx *const restrict ctx,
                     const uint64_t nr_bytes)
{
   if (!ctx->base.ok || !pvr_dump_ctx_require_top(&ctx->base))
      return NULL;

   if (nr_bytes > ctx->remaining_size) {
      pvr_dump_error(&ctx->base, "peeked past end of context buffer");
      return NULL;
   }

   return ctx->ptr;
}

static inline const void *restrict
pvr_dump_buffer_take(struct pvr_dump_buffer_ctx *const restrict ctx,
                     const uint64_t nr_bytes)
{
   const void *const ptr = pvr_dump_buffer_peek(ctx, nr_bytes);

   if (ptr)
      __pvr_dump_buffer_advance(ctx, nr_bytes);

   return ptr;
}

static inline void
pvr_dump_buffer_restart(struct pvr_dump_buffer_ctx *const ctx)
{
   ctx->ptr = ctx->initial_ptr;
   ctx->remaining_size = ctx->capacity;
}

/*****************************************************************************
   Field printers
*****************************************************************************/

#define pvr_dump_field(ctx, name, format, args...)     \
   pvr_dump_println(ctx,                               \
                    "%-*s : " format,                  \
                    PVR_DUMP_FIELD_COLUMN_WIDTH -      \
                       __pvr_dump_ctx_get_indent(ctx), \
                    name,                              \
                    ##args)

#define pvr_dump_field_computed(ctx, name, format, raw_format, args...) \
   pvr_dump_field(ctx, name, format " (" raw_format ")", ##args)

#define pvr_dump_field_error(ctx, format, args...)               \
   ({                                                            \
      struct pvr_dump_ctx *_ctx = (ctx);                         \
      pvr_dump_field(_ctx, "<!ERROR!>", "<" format ">", ##args); \
      _ctx->ok = false;                                          \
      false;                                                     \
   })

/*****************************************************************************
   Field printers: integers
*****************************************************************************/

static inline void pvr_dump_field_u32(struct pvr_dump_ctx *const ctx,
                                      const char *const name,
                                      const uint32_t value)
{
   pvr_dump_field(ctx, name, "%" PRIu32, value);
}

static inline void pvr_dump_field_u32_units(struct pvr_dump_ctx *const ctx,
                                            const char *const name,
                                            const uint32_t value,
                                            const char *const units)
{
   pvr_dump_field(ctx, name, "%" PRIu32 " %s", value, units);
}

static inline void pvr_dump_field_u32_offset(struct pvr_dump_ctx *const ctx,
                                             const char *const name,
                                             const uint32_t value,
                                             const uint32_t offset)
{
   pvr_dump_field_computed(ctx,
                           name,
                           "%" PRIu32,
                           "%" PRIu32 " + %" PRIu32,
                           value + offset,
                           value,
                           offset);
}

static inline void pvr_dump_field_u32_scaled(struct pvr_dump_ctx *const ctx,
                                             const char *const name,
                                             const uint32_t value,
                                             const uint32_t scale)
{
   pvr_dump_field_computed(ctx,
                           name,
                           "%" PRIu32,
                           "%" PRIu32 " x %" PRIu32,
                           value * scale,
                           value,
                           scale);
}

static inline void
pvr_dump_field_u32_scaled_units(struct pvr_dump_ctx *const ctx,
                                const char *const name,
                                const uint32_t value,
                                const uint32_t scale,
                                const char *const units)
{
   pvr_dump_field_computed(ctx,
                           name,
                           "%" PRIu32 " %s",
                           "%" PRIu32 " x %" PRIu32 " %s",
                           value * scale,
                           units,
                           value,
                           scale,
                           units);
}

static inline void pvr_dump_field_u32_zero(struct pvr_dump_ctx *const ctx,
                                           const char *const name,
                                           const uint32_t value,
                                           const uint32_t zero_value)
{
   if (value)
      pvr_dump_field_u32(ctx, name, value);
   else
      pvr_dump_field_computed(ctx, name, "%" PRIu32, "0", zero_value);
}

static inline void pvr_dump_field_x32(struct pvr_dump_ctx *const ctx,
                                      const char *const name,
                                      const uint32_t value,
                                      const uint32_t chars)
{
   pvr_dump_field(ctx,
                  name,
                  "0x%0*" PRIx32,
                  chars,
                  value & BITFIELD_MASK(chars * 4));
}

static inline void pvr_dump_field_u64(struct pvr_dump_ctx *const ctx,
                                      const char *const name,
                                      const uint64_t value)
{
   pvr_dump_field(ctx, name, "%" PRIu64, value);
}

static inline void pvr_dump_field_u64_units(struct pvr_dump_ctx *const ctx,
                                            const char *const name,
                                            const uint64_t value,
                                            const char *const units)
{
   pvr_dump_field(ctx, name, "%" PRIu64 " %s", value, units);
}

/*****************************************************************************
   Field printers: floating point
*****************************************************************************/

static inline void pvr_dump_field_f32(struct pvr_dump_ctx *const ctx,
                                      const char *const name,
                                      const float value)
{
   pvr_dump_field_computed(ctx, name, "%f", "0x%08" PRIx32, value, fui(value));
}

/*****************************************************************************
   Field printers: fixed point
*****************************************************************************/

/* clang-format off */
static const char *const __fixed_frac_str_table_4[1 << 4] = {
   "0", "0625", "125", "1875", "25", "3125", "375", "4375",
   "5", "5625", "625", "6875", "75", "8125", "875", "9375",
};
/* clang-format on */

static inline void pvr_dump_field_uq4_4(struct pvr_dump_ctx *const ctx,
                                        const char *const name,
                                        const uint32_t raw_value)
{
   const uint32_t int_part = (raw_value & BITFIELD_RANGE(4, 4)) >> 4;
   const uint32_t frac_part = raw_value & BITFIELD_MASK(4);

   pvr_dump_field_computed(ctx,
                           name,
                           "%" PRIu32 ".%s",
                           "0x%02" PRIx32, /* Or %0*x where *=(nr_bits+3)/4 */
                           int_part,
                           __fixed_frac_str_table_4[frac_part],
                           raw_value & BITFIELD_MASK(8));
}

static inline void pvr_dump_field_uq4_4_offset(struct pvr_dump_ctx *const ctx,
                                               const char *const name,
                                               const uint32_t raw_value,
                                               const uint32_t raw_offset)
{
   const uint32_t raw_offset_value = raw_value + raw_offset;

   const uint32_t int_part = (raw_offset_value & BITFIELD_RANGE(4, 4)) >> 4;
   const uint32_t frac_part = raw_offset_value & BITFIELD_MASK(4);

   pvr_dump_field_computed(ctx,
                           name,
                           "%" PRIu32 ".%s",
                           "0x%02" PRIx32 " + 0x%02" PRIx32,
                           int_part,
                           __fixed_frac_str_table_4[frac_part],
                           raw_value & BITFIELD_MASK(8),
                           raw_offset);
}

/*****************************************************************************
   Field printers: device address
*****************************************************************************/

static inline void pvr_dump_field_addr_non_null(struct pvr_dump_ctx *const ctx,
                                                const char *const name,
                                                const pvr_dev_addr_t value)
{
   pvr_dump_field(ctx, name, PVR_DEV_ADDR_FMT, value.addr);
}

static inline void pvr_dump_field_addr(struct pvr_dump_ctx *const ctx,
                                       const char *const name,
                                       const pvr_dev_addr_t value)
{
   if (value.addr)
      pvr_dump_field_addr_non_null(ctx, name, value);
   else
      pvr_dump_field(ctx, name, "<null>");
}

static inline void pvr_dump_field_addr_split(struct pvr_dump_ctx *const ctx,
                                             const char *const name,
                                             const pvr_dev_addr_t msb,
                                             const pvr_dev_addr_t lsb)
{
   pvr_dump_field_addr(ctx, name, PVR_DEV_ADDR(msb.addr | lsb.addr));

   pvr_dump_indent(ctx);
   pvr_dump_field_addr_non_null(ctx, "msb", msb);
   pvr_dump_field_addr_non_null(ctx, "lsb", lsb);
   pvr_dump_dedent(ctx);
}

static inline void pvr_dump_field_addr_offset(struct pvr_dump_ctx *const ctx,
                                              const char *const name,
                                              const pvr_dev_addr_t value,
                                              const pvr_dev_addr_t base)
{
   pvr_dump_field_computed(ctx,
                           name,
                           PVR_DEV_ADDR_FMT,
                           PVR_DEV_ADDR_FMT " + " PVR_DEV_ADDR_FMT,
                           PVR_DEV_ADDR_OFFSET(base, value.addr).addr,
                           base.addr,
                           value.addr);
}

/*****************************************************************************
   Field printers: enums
*****************************************************************************/

#define pvr_dump_field_enum(ctx, name, value, to_str)               \
   do {                                                             \
      __typeof__(value) _value = (value);                           \
      const char *_str = to_str(_value);                            \
      if (!_str)                                                    \
         _str = "<unknown>";                                        \
      pvr_dump_field_computed(ctx, name, "%s", "%u", _str, _value); \
   } while (0)

static inline const char *__bool_to_str(const bool b)
{
   return b ? "yes" : "no";
}

/* A bool is just an enum with two values. */
static inline void pvr_dump_field_bool(struct pvr_dump_ctx *const ctx,
                                       const char *const name,
                                       const bool value)
{
   pvr_dump_field_enum(ctx, name, value, __bool_to_str);
}

/*****************************************************************************
   Field printers: string
*****************************************************************************/

static inline void pvr_dump_field_string(struct pvr_dump_ctx *const ctx,
                                         const char *const name,
                                         const char *const value)
{
   pvr_dump_field(ctx, name, "%s", value);
}

/*****************************************************************************
   Field printers: not present
*****************************************************************************/

static inline void pvr_dump_field_no_fields(struct pvr_dump_ctx *const ctx)
{
   pvr_dump_println(ctx, "<no fields>");
}

static inline void pvr_dump_field_not_present(struct pvr_dump_ctx *const ctx,
                                              const char *const name)
{
   pvr_dump_field(ctx, name, "<not present>");
}

/*****************************************************************************
   Field printers: helpers for members
*****************************************************************************/

/* clang-format off */

#define pvr_dump_field_member_u32(ctx, compound, member) \
   pvr_dump_field_u32(ctx, #member, (compound)->member)

#define pvr_dump_field_member_u32_units(ctx, compound, member, units) \
   pvr_dump_field_u32_units(ctx, #member, (compound)->member, units)

#define pvr_dump_field_member_u32_offset(ctx, compound, member, offset) \
   pvr_dump_field_u32_offset(ctx, #member, (compound)->member, offset)

#define pvr_dump_field_member_u32_scaled(ctx, compound, member, scale) \
   pvr_dump_field_u32_scaled(ctx, #member, (compound)->member, scale)

#define pvr_dump_field_member_u32_scaled_units(ctx, compound, member, scale, units) \
   pvr_dump_field_u32_scaled_units(ctx, #member, (compound)->member, scale, units)

#define pvr_dump_field_member_u32_zero(ctx, compound, member, zero_value) \
   pvr_dump_field_u32_zero(ctx, #member, (compound)->member, zero_value)

#define pvr_dump_field_member_x32(ctx, compound, member, chars) \
   pvr_dump_field_x32(ctx, #member, (compound)->member, chars)

#define pvr_dump_field_member_u64(ctx, compound, member) \
   pvr_dump_field_u64(ctx, #member, (compound)->member)

#define pvr_dump_field_member_u64_units(ctx, compound, member, units) \
   pvr_dump_field_u64_units(ctx, #member, (compound)->member, units)

#define pvr_dump_field_member_f32(ctx, compound, member) \
   pvr_dump_field_f32(ctx, #member, (compound)->member)

#define pvr_dump_field_member_uq4_4(ctx, compound, member) \
   pvr_dump_field_uq4_4(ctx, #member, (compound)->member)

#define pvr_dump_field_member_uq4_4_offset(ctx, compound, member, raw_offset) \
   pvr_dump_field_uq4_4_offset(ctx, #member, (compound)->member, raw_offset)

#define pvr_dump_field_member_addr(ctx, compound, member) \
   pvr_dump_field_addr(ctx, #member, (compound)->member)

#define pvr_dump_field_member_addr_offset(ctx, compound, member, base) \
   pvr_dump_field_addr_offset(ctx, #member, (compound)->member, base)

#define pvr_dump_field_member_enum(ctx, compound, member, to_str) \
   pvr_dump_field_enum(ctx, #member, (compound)->member, to_str)

#define pvr_dump_field_member_bool(ctx, compound, member) \
   pvr_dump_field_bool(ctx, #member, (compound)->member)

#define pvr_dump_field_member_string(ctx, compound, member) \
   pvr_dump_field_string(ctx, #member, (compound)->member)

/* clang-format on */

#define pvr_dump_field_member_not_present(ctx, compound, member) \
   do {                                                          \
      (void)&(compound)->member;                                 \
      pvr_dump_field_not_present(ctx, #member);                  \
   } while (0)

#endif /* PVR_DUMP_H */
