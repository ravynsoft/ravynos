/*
 * Copyright © 2016 Intel Corporation
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

#ifndef INTEL_DECODER_H
#define INTEL_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "dev/intel_device_info.h"
#include "util/hash_table.h"
#include "util/bitset.h"

#include "common/intel_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

struct intel_spec;
struct intel_group;
struct intel_field;
union intel_field_value;

#define INTEL_ENGINE_CLASS_TO_MASK(x) BITSET_BIT(x)

static inline uint32_t intel_make_gen(uint32_t major, uint32_t minor)
{
   return (major << 8) | minor;
}

struct intel_group *intel_spec_find_struct(struct intel_spec *spec, const char *name);
struct intel_spec *intel_spec_load(const struct intel_device_info *devinfo);
struct intel_spec *
intel_spec_load_from_path(const struct intel_device_info *devinfo,
                          const char *path);
struct intel_spec *intel_spec_load_filename(const char *dir, const char *name);
void intel_spec_destroy(struct intel_spec *spec);
uint32_t intel_spec_get_gen(struct intel_spec *spec);
struct intel_group *intel_spec_find_instruction(struct intel_spec *spec,
                                                enum intel_engine_class engine,
                                                const uint32_t *p);
struct intel_group *intel_spec_find_register(struct intel_spec *spec, uint32_t offset);
struct intel_group *intel_spec_find_register_by_name(struct intel_spec *spec, const char *name);
struct intel_enum *intel_spec_find_enum(struct intel_spec *spec, const char *name);

int intel_group_get_length(const struct intel_group *group, const uint32_t *p);
const char *intel_group_get_name(const struct intel_group *group);
uint32_t intel_group_get_opcode(const struct intel_group *group);
struct intel_field *intel_group_find_field(struct intel_group *group, const char *name);
struct intel_enum *intel_spec_find_enum(struct intel_spec *spec, const char *name);

bool intel_field_is_header(struct intel_field *field);

/* Only allow 5 levels of subgroup'ing
 */
#define DECODE_MAX_ARRAY_DEPTH 5

struct intel_field_iterator {
   struct intel_group *group;
   char name[128];
   char value[128];
   uint64_t raw_value;
   struct intel_group *struct_desc;
   const uint32_t *p;
   int p_bit; /**< bit offset into p */
   const uint32_t *p_end;
   int start_bit; /**< current field starts at this bit offset into p */
   int end_bit; /**< current field ends at this bit offset into p */

   struct intel_field *fields[DECODE_MAX_ARRAY_DEPTH];
   struct intel_group *groups[DECODE_MAX_ARRAY_DEPTH];
   int array_iter[DECODE_MAX_ARRAY_DEPTH];
   int level;

   struct intel_field *field;
   bool print_colors;
};

struct intel_spec {
   uint32_t gen;

   struct hash_table *commands;
   struct hash_table *structs;
   struct hash_table *registers_by_name;
   struct hash_table *registers_by_offset;
   struct hash_table *enums;

   struct hash_table *access_cache;
};

struct intel_group {
   struct intel_spec *spec;
   char *name;

   struct intel_field *fields; /* linked list of fields */
   struct intel_field *dword_length_field; /* <instruction> specific */

   uint32_t dw_length;
   uint32_t engine_mask; /* <instruction> specific */
   uint32_t bias; /* <instruction> specific */
   uint32_t array_offset; /* <group> specific */
   uint32_t array_count; /* number of elements, <group> specific */
   uint32_t array_item_size; /* <group> specific */
   bool variable; /* <group> specific */
   bool fixed_length; /* True for <struct> & <register> */

   struct intel_group *parent;
   struct intel_group *next;

   uint32_t opcode_mask;
   uint32_t opcode;

   uint32_t register_offset; /* <register> specific */
};

struct intel_value {
   char *name;
   uint64_t value;
};

struct intel_enum {
   char *name;
   int nvalues;
   struct intel_value **values;
};

struct intel_type {
   enum {
      INTEL_TYPE_UNKNOWN,
      INTEL_TYPE_INT,
      INTEL_TYPE_UINT,
      INTEL_TYPE_BOOL,
      INTEL_TYPE_FLOAT,
      INTEL_TYPE_ADDRESS,
      INTEL_TYPE_OFFSET,
      INTEL_TYPE_STRUCT,
      INTEL_TYPE_UFIXED,
      INTEL_TYPE_SFIXED,
      INTEL_TYPE_MBO,
      INTEL_TYPE_MBZ,
      INTEL_TYPE_ENUM
   } kind;

   /* Struct definition for  INTEL_TYPE_STRUCT */
   union {
      struct intel_group *intel_struct;
      struct intel_enum *intel_enum;
      struct {
         /* Integer and fractional sizes for INTEL_TYPE_UFIXED and INTEL_TYPE_SFIXED */
         int i, f;
      };
   };
};

union intel_field_value {
   bool b32;
   float f32;
   uint64_t u64;
   int64_t i64;
};

struct intel_field {
   struct intel_group *parent;
   struct intel_field *next;
   struct intel_group *array;

   char *name;
   int start, end;
   struct intel_type type;
   bool has_default;
   uint32_t default_value;

   struct intel_enum inline_enum;
};

void intel_field_iterator_init(struct intel_field_iterator *iter,
                               struct intel_group *group,
                               const uint32_t *p, int p_bit,
                               bool print_colors);

bool intel_field_iterator_next(struct intel_field_iterator *iter);

void intel_print_group(FILE *out,
                       struct intel_group *group,
                       uint64_t offset, const uint32_t *p, int p_bit,
                       bool color);

enum intel_batch_decode_flags {
   /** Print in color! */
   INTEL_BATCH_DECODE_IN_COLOR    = (1 << 0),
   /** Print everything, not just headers */
   INTEL_BATCH_DECODE_FULL        = (1 << 1),
   /** Print offsets along with the batch */
   INTEL_BATCH_DECODE_OFFSETS     = (1 << 2),
   /** Guess when a value is a float and print it as such */
   INTEL_BATCH_DECODE_FLOATS      = (1 << 3),
   /** Print surface states */
   INTEL_BATCH_DECODE_SURFACES    = (1 << 4),
   /** Print sampler states */
   INTEL_BATCH_DECODE_SAMPLERS    = (1 << 5),
   /** Print accumulated state
    *
    *  Instead of printing instructions as we parse them, retain a pointer to
    *  each of the last instruction emitted and print it upon parsing one of
    *  the following instructions :
    *     - 3DPRIMITIVE
    *     - GPGPU_WALKER
    *     - 3DSTATE_WM_HZ_OP
    *     - COMPUTE_WALKER
    */
   INTEL_BATCH_DECODE_ACCUMULATE  = (1 << 6),
};

#define INTEL_BATCH_DECODE_DEFAULT_FLAGS \
   (INTEL_BATCH_DECODE_FULL |            \
    INTEL_BATCH_DECODE_OFFSETS |         \
    INTEL_BATCH_DECODE_FLOATS |          \
    INTEL_BATCH_DECODE_SURFACES |        \
    INTEL_BATCH_DECODE_SAMPLERS)

struct intel_batch_decode_bo {
   uint64_t addr;
   uint32_t size;
   const void *map;
};

struct intel_batch_decode_ctx {
   /**
    * Return information about the buffer containing the given address.
    *
    * If the given address is inside a buffer, the map pointer should be
    * offset accordingly so it points at the data corresponding to address.
    */
   struct intel_batch_decode_bo (*get_bo)(void *user_data, bool ppgtt, uint64_t address);
   unsigned (*get_state_size)(void *user_data,
                              uint64_t address,
                              uint64_t base_address);

   void (*shader_binary)(void *user_data,
                         const char *short_name,
                         uint64_t address,
                         const void *data,
                         unsigned data_length);

   void *user_data;

   FILE *fp;
   const struct brw_isa_info *isa;
   struct intel_device_info devinfo;
   struct intel_spec *spec;
   enum intel_batch_decode_flags flags;

   bool use_256B_binding_tables;
   uint64_t surface_base;
   uint64_t bt_pool_base;
   uint64_t dynamic_base;
   uint64_t instruction_base;

   int max_vbo_decoded_lines;

   enum intel_engine_class engine;

   int n_batch_buffer_start;
   uint64_t acthd;

   struct hash_table *commands;
   struct hash_table *stats;
};

void intel_batch_decode_ctx_init(struct intel_batch_decode_ctx *ctx,
                                 const struct brw_isa_info *isa,
                                 const struct intel_device_info *devinfo,
                                 FILE *fp, enum intel_batch_decode_flags flags,
                                 const char *xml_path,
                                 struct intel_batch_decode_bo (*get_bo)(void *,
                                                                        bool,
                                                                        uint64_t),
                                 unsigned (*get_state_size)(void *, uint64_t,
                                                            uint64_t),
                                 void *user_data);
void intel_batch_decode_ctx_finish(struct intel_batch_decode_ctx *ctx);


void intel_print_batch(struct intel_batch_decode_ctx *ctx,
                       const uint32_t *batch, uint32_t batch_size,
                       uint64_t batch_addr, bool from_ring);

void intel_batch_stats_reset(struct intel_batch_decode_ctx *ctx);

void intel_batch_stats(struct intel_batch_decode_ctx *ctx,
                       const uint32_t *batch, uint32_t batch_size,
                       uint64_t batch_addr, bool from_ring);

void intel_batch_print_stats(struct intel_batch_decode_ctx *ctx);

#ifdef __cplusplus
}
#endif


#endif /* INTEL_DECODER_H */
