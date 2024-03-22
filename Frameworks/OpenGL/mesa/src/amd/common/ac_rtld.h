/*
 * Copyright 2014-2019 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_RTLD_H
#define AC_RTLD_H

#include "compiler/shader_enums.h"
#include "util/u_dynarray.h"
#include "amd_family.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ac_rtld_part;
struct ac_shader_config;
struct radeon_info;

struct ac_rtld_symbol {
   const char *name;
   uint32_t size;
   uint32_t align;
   uint64_t offset;   /* filled in by ac_rtld_open */
   unsigned part_idx; /* shader part in which this symbol appears */
};

struct ac_rtld_options {
   /* Loader will insert an s_sethalt 1 instruction as the
    * first instruction. */
   bool halt_at_entry : 1;
};

/* Lightweight wrapper around underlying ELF objects. */
struct ac_rtld_binary {
   struct ac_rtld_options options;
   enum amd_gfx_level gfx_level;
   unsigned wave_size;

   /* Required buffer sizes, currently read/executable only. */
   uint64_t rx_size;

   /* Size of executable code, for reporting purposes. */
   uint64_t exec_size;

   uint64_t rx_end_markers;

   unsigned num_parts;
   struct ac_rtld_part *parts;

   struct util_dynarray lds_symbols;
   uint32_t lds_size;
};

/**
 * Callback function type used during upload to resolve external symbols that
 * are not defined in any of the ELF binaries available to the linker.
 *
 * \param cb_data caller-defined data
 * \param symbol NUL-terminated symbol name
 * \param value to be filled in by the callback
 * \return whether the symbol was found successfully
 */
typedef bool (*ac_rtld_get_external_symbol_cb)(enum amd_gfx_level gfx_level, void *cb_data,
                                               const char *symbol, uint64_t *value);

/**
 * Lifetimes of \ref info, in-memory ELF objects, and the names of
 * \ref shared_lds_symbols must extend until \ref ac_rtld_close is called on
 * the opened binary.
 */
struct ac_rtld_open_info {
   const struct radeon_info *info;
   struct ac_rtld_options options;
   gl_shader_stage shader_type;
   unsigned wave_size;

   unsigned num_parts;
   const char *const *elf_ptrs; /* in-memory ELF objects of each part */
   const size_t *elf_sizes;     /* sizes of corresponding in-memory ELF objects in bytes */

   /* Shared LDS symbols are layouted such that they are accessible from
    * all shader parts. Non-shared (private) LDS symbols of one part may
    * overlap private LDS symbols of another shader part.
    */
   unsigned num_shared_lds_symbols;
   const struct ac_rtld_symbol *shared_lds_symbols;
};

bool ac_rtld_open(struct ac_rtld_binary *binary, struct ac_rtld_open_info i);

void ac_rtld_close(struct ac_rtld_binary *binary);

bool ac_rtld_get_section_by_name(struct ac_rtld_binary *binary, const char *name, const char **data,
                                 size_t *nbytes);

bool ac_rtld_read_config(const struct radeon_info *info, struct ac_rtld_binary *binary,
                         struct ac_shader_config *config);

struct ac_rtld_upload_info {
   struct ac_rtld_binary *binary;

   /** GPU mapping of the read/executable buffer. */
   uint64_t rx_va;

   /** CPU mapping of the read/executable buffer */
   char *rx_ptr;

   /** Optional callback function that will be queried for symbols not
    * defined in any of the binary's parts. */
   ac_rtld_get_external_symbol_cb get_external_symbol;

   /** Caller-defined data that will be passed to callback functions. */
   void *cb_data;
};

int ac_rtld_upload(struct ac_rtld_upload_info *u);

#ifdef __cplusplus
}
#endif

#endif /* AC_RTLD_H */
