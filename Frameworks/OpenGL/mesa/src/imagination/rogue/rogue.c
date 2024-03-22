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

#include "compiler/glsl_types.h"
#include "rogue.h"
#include "util/list.h"
#include "util/macros.h"
#include "util/ralloc.h"
#include "util/sparse_array.h"

#include <stdbool.h>

/**
 * \file rogue.c
 *
 * \brief Contains general Rogue IR functions.
 */

/* TODO: Tweak these? */
#define ROGUE_REG_CACHE_NODE_SIZE 512
#define ROGUE_REGARRAY_CACHE_NODE_SIZE 512

/**
 * \brief Sets an existing register to a (new) class and/or index.
 *
 * \param[in] shader The shader containing the register.
 * \param[in] reg The register being changed.
 * \param[in] class The new register class.
 * \param[in] index The new register index.
 * \return True if the register was updated, else false.
 */
PUBLIC
bool rogue_reg_set(rogue_shader *shader,
                   rogue_reg *reg,
                   enum rogue_reg_class class,
                   unsigned index)
{
   bool changed = true;

   if (reg->class == class && reg->index == index)
      changed = false;

   const rogue_reg_info *info = &rogue_reg_infos[class];

   if (info->num) {
      assert(index < info->num);
      rogue_set_reg_use(shader, class, index);
   }

   if (reg->class != class) {
      list_del(&reg->link);
      list_addtail(&reg->link, &shader->regs[class]);
   }

   reg->class = class;
   reg->index = index;
   reg->dirty = true;

   /* Clear the old cache entry. */
   if (reg->cached && *reg->cached == reg)
      *reg->cached = NULL;

   /* Set new cache entry. */
   rogue_reg **reg_cached =
      util_sparse_array_get(&shader->reg_cache[class], index);
   *reg_cached = reg;
   reg->cached = reg_cached;

   return changed;
}

/**
 * \brief Sets an existing register to a (new) class and/or index, and updates
 * its usage bitset.
 *
 * \param[in] shader The shader containing the register.
 * \param[in] reg The register being changed.
 * \param[in] class The new register class.
 * \param[in] index The new register index.
 * \return True if the register was updated, else false.
 */
PUBLIC
bool rogue_reg_rewrite(rogue_shader *shader,
                       rogue_reg *reg,
                       enum rogue_reg_class class,
                       unsigned index)
{
   const rogue_reg_info *info = &rogue_reg_infos[reg->class];
   if (info->num) {
      assert(rogue_reg_is_used(shader, reg->class, reg->index) &&
             "Register not in use!");
      rogue_clear_reg_use(shader, reg->class, reg->index);
   }

   return rogue_reg_set(shader, reg, class, index);
}

PUBLIC
bool rogue_regarray_set(rogue_shader *shader,
                        rogue_regarray *regarray,
                        enum rogue_reg_class class,
                        unsigned base_index,
                        bool set_regs)
{
   bool updated = true;

   if (set_regs) {
      for (unsigned u = 0; u < regarray->size; ++u) {
         updated &=
            rogue_reg_set(shader, regarray->regs[u], class, base_index + u);
      }
   }

   if (regarray->cached && *regarray->cached == regarray)
      *regarray->cached = NULL;

   uint64_t key =
      rogue_regarray_cache_key(regarray->size, class, base_index, false, 0);

   rogue_regarray **regarray_cached =
      util_sparse_array_get(&shader->regarray_cache, key);
   assert(*regarray_cached == NULL);

   *regarray_cached = regarray;
   regarray->cached = regarray_cached;

   return updated;
}

bool rogue_regarray_rewrite(rogue_shader *shader,
                            rogue_regarray *regarray,
                            enum rogue_reg_class class,
                            unsigned base_index)
{
   bool progress = true;

   enum rogue_reg_class orig_class = regarray->regs[0]->class;
   unsigned orig_base_index = regarray->regs[0]->index;
   const rogue_reg_info *info = &rogue_reg_infos[orig_class];

   assert(!regarray->parent);

   if (info->num) {
      for (unsigned u = 0; u < regarray->size; ++u) {
         assert(rogue_reg_is_used(shader, orig_class, orig_base_index) &&
                "Register not in use!");
         rogue_clear_reg_use(shader, orig_class, orig_base_index);
      }
   }

   progress &= rogue_regarray_set(shader, regarray, class, base_index, true);

   rogue_foreach_subarray (subarray, regarray) {
      unsigned idx_offset = subarray->regs[0]->index - regarray->regs[0]->index;
      progress &= rogue_regarray_set(shader,
                                     subarray,
                                     class,
                                     base_index + idx_offset,
                                     false);
   }

   assert(progress);
   return progress;
}

static void rogue_shader_destructor(void *ptr)
{
   rogue_shader *shader = ptr;
   for (unsigned u = 0; u < ARRAY_SIZE(shader->reg_cache); ++u)
      util_sparse_array_finish(&shader->reg_cache[u]);

   util_sparse_array_finish(&shader->regarray_cache);
}

/**
 * \brief Allocates and initializes a new rogue_shader object.
 *
 * \param[in] mem_ctx The new shader's memory context.
 * \param[in] stage The new shader's stage.
 * \return The new shader.
 */
PUBLIC
rogue_shader *rogue_shader_create(void *mem_ctx, gl_shader_stage stage)
{
   rogue_debug_init();

   rogue_shader *shader = rzalloc_size(mem_ctx, sizeof(*shader));

   shader->stage = stage;

   list_inithead(&shader->blocks);

   for (enum rogue_reg_class class = 0; class < ROGUE_REG_CLASS_COUNT;
        ++class) {
      list_inithead(&shader->regs[class]);

      const rogue_reg_info *info = &rogue_reg_infos[class];
      if (info->num) {
         unsigned bitset_size =
            sizeof(*shader->regs_used[class]) * BITSET_WORDS(info->num);
         shader->regs_used[class] = rzalloc_size(shader, bitset_size);
      }
   }

   for (unsigned u = 0; u < ARRAY_SIZE(shader->reg_cache); ++u)
      util_sparse_array_init(&shader->reg_cache[u],
                             sizeof(rogue_reg *),
                             ROGUE_REG_CACHE_NODE_SIZE);

   list_inithead(&shader->regarrays);

   util_sparse_array_init(&shader->regarray_cache,
                          sizeof(rogue_regarray *),
                          ROGUE_REGARRAY_CACHE_NODE_SIZE);

   for (unsigned u = 0; u < ARRAY_SIZE(shader->drc_trxns); ++u)
      list_inithead(&shader->drc_trxns[u]);

   list_inithead(&shader->imm_uses);

   ralloc_set_destructor(shader, rogue_shader_destructor);

   return shader;
}

/**
 * \brief Allocates and initializes a new rogue_reg object.
 *
 * \param[in] shader The shader which will contain the register.
 * \param[in] class The register class.
 * \param[in] index The register index.
 * \param[in] reg_cached The shader register cache.
 * \return The new register.
 */
static rogue_reg *rogue_reg_create(rogue_shader *shader,
                                   enum rogue_reg_class class,
                                   uint32_t index,
                                   rogue_reg **reg_cached)
{
   rogue_reg *reg = rzalloc_size(shader, sizeof(*reg));

   reg->shader = shader;
   reg->class = class;
   reg->index = index;
   reg->cached = reg_cached;

   list_addtail(&reg->link, &shader->regs[class]);
   list_inithead(&reg->writes);
   list_inithead(&reg->uses);

   const rogue_reg_info *info = &rogue_reg_infos[class];
   if (info->num) {
      assert(index < info->num);
      assert(!rogue_reg_is_used(shader, class, index) &&
             "Register already in use!");
      rogue_set_reg_use(shader, class, index);
   }

   return reg;
}

/**
 * \brief Deletes and frees a Rogue register.
 *
 * \param[in] reg The register to delete.
 */
PUBLIC
void rogue_reg_delete(rogue_reg *reg)
{
   assert(rogue_reg_is_unused(reg));
   const rogue_reg_info *info = &rogue_reg_infos[reg->class];
   if (info->num) {
      assert(rogue_reg_is_used(reg->shader, reg->class, reg->index) &&
             "Register not in use!");
      rogue_clear_reg_use(reg->shader, reg->class, reg->index);
   }

   if (reg->cached && *reg->cached == reg)
      *reg->cached = NULL;

   list_del(&reg->link);
   ralloc_free(reg);
}

static inline rogue_reg *rogue_reg_cached_common(rogue_shader *shader,
                                                 enum rogue_reg_class class,
                                                 uint32_t index,
                                                 uint8_t component,
                                                 bool vec)
{
   uint32_t key = rogue_reg_cache_key(index, vec, component);

   rogue_reg **reg_cached =
      util_sparse_array_get(&shader->reg_cache[class], key);
   if (!*reg_cached)
      *reg_cached = rogue_reg_create(shader, class, key, reg_cached);

   return *reg_cached;
}

static inline rogue_reg *rogue_reg_cached(rogue_shader *shader,
                                          enum rogue_reg_class class,
                                          uint32_t index)
{
   return rogue_reg_cached_common(shader, class, index, 0, false);
}

static inline rogue_reg *rogue_vec_reg_cached(rogue_shader *shader,
                                              enum rogue_reg_class class,
                                              unsigned index,
                                              unsigned component)
{
   return rogue_reg_cached_common(shader, class, index, component, true);
}

/* TODO: Static inline in rogue.h? */
PUBLIC
rogue_reg *rogue_ssa_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_SSA, index);
}

PUBLIC
rogue_reg *rogue_temp_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_TEMP, index);
}

PUBLIC
rogue_reg *rogue_coeff_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_COEFF, index);
}

PUBLIC
rogue_reg *rogue_shared_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_SHARED, index);
}

PUBLIC
rogue_reg *rogue_const_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_CONST, index);
}

PUBLIC
rogue_reg *rogue_pixout_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_PIXOUT, index);
}

PUBLIC
rogue_reg *rogue_special_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_SPECIAL, index);
}

PUBLIC
rogue_reg *rogue_vtxin_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_VTXIN, index);
}

PUBLIC
rogue_reg *rogue_vtxout_reg(rogue_shader *shader, unsigned index)
{
   return rogue_reg_cached(shader, ROGUE_REG_CLASS_VTXOUT, index);
}

PUBLIC
rogue_reg *
rogue_ssa_vec_reg(rogue_shader *shader, unsigned index, unsigned component)
{
   return rogue_vec_reg_cached(shader, ROGUE_REG_CLASS_SSA, index, component);
}

static rogue_regarray *rogue_find_common_regarray(rogue_regarray *regarray,
                                                  bool *is_parent,
                                                  rogue_reg ***parent_regptr)
{
   rogue_regarray *common_regarray = NULL;

   for (unsigned u = 0; u < regarray->size; ++u) {
      if (regarray->regs[u]->regarray) {
         if (common_regarray && regarray->regs[u]->regarray != common_regarray)
            unreachable("Can't have overlapping regarrays.");
         else if (!common_regarray)
            common_regarray = regarray->regs[u]->regarray;
      }
   }

   if (common_regarray) {
      unsigned min_index = regarray->regs[0]->index;
      unsigned max_index = min_index + regarray->size - 1;

      unsigned min_common_index = common_regarray->regs[0]->index;
      unsigned max_common_index = min_common_index + common_regarray->size - 1;

      /* TODO: Create a new parent array that encapsulates both ranges? */
      /* Ensure that the new regarray doesn't occupy only part of its parent,
       * and also registers *beyond* its parent. */
      if ((min_index > min_common_index && max_index > max_common_index) ||
          (min_index < min_common_index && max_index < max_common_index))
         unreachable("Can't have overflowing partial regarrays.");

      *is_parent = regarray->size > common_regarray->size;
      const rogue_regarray *parent_regarray = *is_parent ? regarray
                                                         : common_regarray;
      const rogue_regarray *child_regarray = *is_parent ? common_regarray
                                                        : regarray;

      for (unsigned u = 0; u < parent_regarray->size; ++u) {
         if (child_regarray->regs[0]->index ==
             parent_regarray->regs[u]->index) {
            *parent_regptr = &parent_regarray->regs[u];
            break;
         }
      }
   }

   return common_regarray;
}

static rogue_regarray *rogue_regarray_create(rogue_shader *shader,
                                             unsigned size,
                                             enum rogue_reg_class class,
                                             unsigned start_index,
                                             uint8_t component,
                                             bool vec,
                                             rogue_regarray **regarray_cached)
{
   rogue_regarray *regarray = rzalloc_size(shader, sizeof(*regarray));
   regarray->regs = rzalloc_size(regarray, sizeof(*regarray->regs) * size);
   regarray->size = size;
   regarray->cached = regarray_cached;
   list_inithead(&regarray->children);
   list_inithead(&regarray->writes);
   list_inithead(&regarray->uses);

   for (unsigned u = 0; u < size; ++u) {
      regarray->regs[u] =
         vec ? rogue_vec_reg_cached(shader, class, start_index, component + u)
             : rogue_reg_cached(shader, class, start_index + u);
   }

   bool is_parent = false;
   rogue_reg **parent_regptr = NULL;
   rogue_regarray *common_regarray =
      rogue_find_common_regarray(regarray, &is_parent, &parent_regptr);

   if (!common_regarray) {
      /* We don't share any registers with another regarray. */
      for (unsigned u = 0; u < size; ++u)
         regarray->regs[u]->regarray = regarray;
   } else {
      if (is_parent) {
         /* We share registers with another regarray, and it is a subset of us.
          */
         for (unsigned u = 0; u < common_regarray->size; ++u)
            common_regarray->regs[u]->regarray = regarray;

         /* Steal its children. */
         rogue_foreach_subarray_safe (subarray, common_regarray) {
            unsigned parent_index = common_regarray->regs[0]->index;
            unsigned child_index = subarray->regs[0]->index;
            assert(child_index >= parent_index);

            subarray->parent = regarray;
            subarray->regs = &parent_regptr[child_index - parent_index];

            list_del(&subarray->child_link);
            list_addtail(&subarray->child_link, &regarray->children);
         }

         common_regarray->parent = regarray;
         ralloc_free(common_regarray->regs);
         common_regarray->regs = parent_regptr;
         list_addtail(&common_regarray->child_link, &regarray->children);
      } else {
         /* We share registers with another regarray, and we are a subset of it.
          */
         regarray->parent = common_regarray;
         ralloc_free(regarray->regs);
         regarray->regs = parent_regptr;
         assert(list_is_empty(&regarray->children));
         list_addtail(&regarray->child_link, &common_regarray->children);
      }
   }

   list_addtail(&regarray->link, &shader->regarrays);

   return regarray;
}

static inline rogue_regarray *
rogue_regarray_cached_common(rogue_shader *shader,
                             unsigned size,
                             enum rogue_reg_class class,
                             uint32_t start_index,
                             uint8_t component,
                             bool vec)
{
   uint64_t key =
      rogue_regarray_cache_key(size, class, start_index, vec, component);

   rogue_regarray **regarray_cached =
      util_sparse_array_get(&shader->regarray_cache, key);
   if (!*regarray_cached)
      *regarray_cached = rogue_regarray_create(shader,
                                               size,
                                               class,
                                               start_index,
                                               component,
                                               vec,
                                               regarray_cached);

   return *regarray_cached;
}

PUBLIC
rogue_regarray *rogue_regarray_cached(rogue_shader *shader,
                                      unsigned size,
                                      enum rogue_reg_class class,
                                      uint32_t start_index)
{
   return rogue_regarray_cached_common(shader,
                                       size,
                                       class,
                                       start_index,
                                       0,
                                       false);
}

PUBLIC
rogue_regarray *rogue_vec_regarray_cached(rogue_shader *shader,
                                          unsigned size,
                                          enum rogue_reg_class class,
                                          uint32_t start_index,
                                          uint8_t component)
{
   return rogue_regarray_cached_common(shader,
                                       size,
                                       class,
                                       start_index,
                                       component,
                                       true);
}

PUBLIC
rogue_regarray *
rogue_ssa_regarray(rogue_shader *shader, unsigned size, unsigned start_index)
{
   return rogue_regarray_cached(shader, size, ROGUE_REG_CLASS_SSA, start_index);
}

PUBLIC
rogue_regarray *
rogue_temp_regarray(rogue_shader *shader, unsigned size, unsigned start_index)
{
   return rogue_regarray_cached(shader, size, ROGUE_REG_CLASS_TEMP, start_index);
}

PUBLIC
rogue_regarray *
rogue_coeff_regarray(rogue_shader *shader, unsigned size, unsigned start_index)
{
   return rogue_regarray_cached(shader,
                                size,
                                ROGUE_REG_CLASS_COEFF,
                                start_index);
}

PUBLIC
rogue_regarray *
rogue_shared_regarray(rogue_shader *shader, unsigned size, unsigned start_index)
{
   return rogue_regarray_cached(shader,
                                size,
                                ROGUE_REG_CLASS_SHARED,
                                start_index);
}

PUBLIC
rogue_regarray *rogue_ssa_vec_regarray(rogue_shader *shader,
                                       unsigned size,
                                       unsigned start_index,
                                       unsigned component)
{
   return rogue_vec_regarray_cached(shader,
                                    size,
                                    ROGUE_REG_CLASS_SSA,
                                    start_index,
                                    component);
}

/**
 * \brief Allocates and initializes a new rogue_block object.
 *
 * \param[in] shader The shader that the new block belongs to.
 * \param[in] label The (optional) block label.
 * \return The new block.
 */
PUBLIC
rogue_block *rogue_block_create(rogue_shader *shader, const char *label)
{
   rogue_block *block = rzalloc_size(shader, sizeof(*block));

   block->shader = shader;
   list_inithead(&block->instrs);
   list_inithead(&block->uses);
   block->index = shader->next_block++;
   block->label = ralloc_strdup(block, label);

   return block;
}

/**
 * \brief Initialises a Rogue instruction.
 *
 * \param[in] instr The instruction to initialise.
 * \param[in] type The instruction type.
 * \param[in] block The block which will contain the instruction.
 */
static inline void rogue_instr_init(rogue_instr *instr,
                                    enum rogue_instr_type type,
                                    rogue_block *block)
{
   instr->type = type;
   instr->exec_cond = ROGUE_EXEC_COND_PE_TRUE;
   instr->repeat = 1;
   instr->index = block->shader->next_instr++;
   instr->block = block;
}

/**
 * \brief Allocates and initializes a new rogue_alu_instr object.
 *
 * \param[in] block The block that the new ALU instruction belongs to.
 * \param[in] op The ALU operation.
 * \return The new ALU instruction.
 */
PUBLIC
rogue_alu_instr *rogue_alu_instr_create(rogue_block *block,
                                        enum rogue_alu_op op)
{
   rogue_alu_instr *alu = rzalloc_size(block, sizeof(*alu));
   rogue_instr_init(&alu->instr, ROGUE_INSTR_TYPE_ALU, block);
   alu->op = op;

   return alu;
}

/**
 * \brief Allocates and initializes a new rogue_backend_instr object.
 *
 * \param[in] block The block that the new backend instruction belongs to.
 * \param[in] op The backend operation.
 * \return The new backend instruction.
 */
PUBLIC
rogue_backend_instr *rogue_backend_instr_create(rogue_block *block,
                                                enum rogue_backend_op op)
{
   rogue_backend_instr *backend = rzalloc_size(block, sizeof(*backend));
   rogue_instr_init(&backend->instr, ROGUE_INSTR_TYPE_BACKEND, block);
   backend->op = op;

   return backend;
}

/**
 * \brief Allocates and initializes a new rogue_ctrl_instr object.
 *
 * \param[in] block The block that the new control instruction belongs to.
 * \param[in] op The control operation.
 * \return The new control instruction.
 */
PUBLIC
rogue_ctrl_instr *rogue_ctrl_instr_create(rogue_block *block,
                                          enum rogue_ctrl_op op)
{
   rogue_ctrl_instr *ctrl = rzalloc_size(block, sizeof(*ctrl));
   rogue_instr_init(&ctrl->instr, ROGUE_INSTR_TYPE_CTRL, block);
   ctrl->op = op;

   return ctrl;
}

/**
 * \brief Allocates and initializes a new rogue_bitwise_instr object.
 *
 * \param[in] block The block that the new bitwise instruction belongs to.
 * \param[in] op The bitwise operation.
 * \return The new bitwise instruction.
 */
PUBLIC
rogue_bitwise_instr *rogue_bitwise_instr_create(rogue_block *block,
                                                enum rogue_bitwise_op op)
{
   rogue_bitwise_instr *bitwise = rzalloc_size(block, sizeof(*bitwise));
   rogue_instr_init(&bitwise->instr, ROGUE_INSTR_TYPE_BITWISE, block);
   bitwise->op = op;

   return bitwise;
}

/**
 * \brief Tracks/links objects that are written to/modified by an instruction.
 *
 * \param[in] instr The instruction.
 */
PUBLIC
void rogue_link_instr_write(rogue_instr *instr)
{
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU: {
      rogue_alu_instr *alu = rogue_instr_as_alu(instr);
      const unsigned num_dsts = rogue_alu_op_infos[alu->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&alu->dst[i].ref)) {
            rogue_reg_write *write = &alu->dst_write[i].reg;
            rogue_reg *reg = alu->dst[i].ref.reg;
            rogue_link_instr_write_reg(instr, write, reg, i);
         } else if (rogue_ref_is_regarray(&alu->dst[i].ref)) {
            rogue_regarray_write *write = &alu->dst_write[i].regarray;
            rogue_regarray *regarray = alu->dst[i].ref.regarray;
            rogue_link_instr_write_regarray(instr, write, regarray, i);
         } else if (rogue_ref_is_io(&alu->dst[i].ref)) { /* TODO: check WHICH IO
                                                            IT IS */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BACKEND: {
      rogue_backend_instr *backend = rogue_instr_as_backend(instr);
      const unsigned num_dsts = rogue_backend_op_infos[backend->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&backend->dst[i].ref)) {
            rogue_reg_write *write = &backend->dst_write[i].reg;
            rogue_reg *reg = backend->dst[i].ref.reg;
            rogue_link_instr_write_reg(instr, write, reg, i);
         } else if (rogue_ref_is_regarray(&backend->dst[i].ref)) {
            rogue_regarray_write *write = &backend->dst_write[i].regarray;
            rogue_regarray *regarray = backend->dst[i].ref.regarray;
            rogue_link_instr_write_regarray(instr, write, regarray, i);
         } else if (rogue_ref_is_io(&backend->dst[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_CTRL: {
      rogue_ctrl_instr *ctrl = rogue_instr_as_ctrl(instr);
      const unsigned num_dsts = rogue_ctrl_op_infos[ctrl->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&ctrl->dst[i].ref)) {
            rogue_reg_write *write = &ctrl->dst_write[i].reg;
            rogue_reg *reg = ctrl->dst[i].ref.reg;
            rogue_link_instr_write_reg(instr, write, reg, i);
         } else if (rogue_ref_is_regarray(&ctrl->dst[i].ref)) {
            rogue_regarray_write *write = &ctrl->dst_write[i].regarray;
            rogue_regarray *regarray = ctrl->dst[i].ref.regarray;
            rogue_link_instr_write_regarray(instr, write, regarray, i);
         } else if (rogue_ref_is_io(&ctrl->dst[i].ref)) { /* TODO: check WHICH
                                                             IO IT IS */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BITWISE: {
      rogue_bitwise_instr *bitwise = rogue_instr_as_bitwise(instr);
      const unsigned num_dsts = rogue_bitwise_op_infos[bitwise->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&bitwise->dst[i].ref)) {
            rogue_reg_write *write = &bitwise->dst_write[i].reg;
            rogue_reg *reg = bitwise->dst[i].ref.reg;
            rogue_link_instr_write_reg(instr, write, reg, i);
         } else if (rogue_ref_is_regarray(&bitwise->dst[i].ref)) {
            rogue_regarray_write *write = &bitwise->dst_write[i].regarray;
            rogue_regarray *regarray = bitwise->dst[i].ref.regarray;
            rogue_link_instr_write_regarray(instr, write, regarray, i);
         } else if (rogue_ref_is_io(&bitwise->dst[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   default:
      unreachable("Unsupported instruction type.");
   }
}

/**
 * \brief Tracks/links objects that are used by/read from an instruction.
 *
 * \param[in] instr The instruction.
 */
PUBLIC
void rogue_link_instr_use(rogue_instr *instr)
{
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU: {
      rogue_alu_instr *alu = rogue_instr_as_alu(instr);
      const unsigned num_srcs = rogue_alu_op_infos[alu->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&alu->src[i].ref)) {
            rogue_reg_use *use = &alu->src_use[i].reg;
            rogue_reg *reg = alu->src[i].ref.reg;
            rogue_link_instr_use_reg(instr, use, reg, i);
         } else if (rogue_ref_is_regarray(&alu->src[i].ref)) {
            rogue_regarray_use *use = &alu->src_use[i].regarray;
            rogue_regarray *regarray = alu->src[i].ref.regarray;
            rogue_link_instr_use_regarray(instr, use, regarray, i);
         } else if (rogue_ref_is_imm(&alu->src[i].ref)) {
            rogue_link_imm_use(instr->block->shader,
                               instr,
                               i,
                               rogue_ref_get_imm(&alu->src[i].ref));
         } else if (rogue_ref_is_io(&alu->src[i].ref)) { /* TODO: check WHICH IO
                                                            IT IS */
         } else if (rogue_ref_is_val(&alu->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BACKEND: {
      rogue_backend_instr *backend = rogue_instr_as_backend(instr);
      const unsigned num_srcs = rogue_backend_op_infos[backend->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&backend->src[i].ref)) {
            rogue_reg_use *use = &backend->src_use[i].reg;
            rogue_reg *reg = backend->src[i].ref.reg;
            rogue_link_instr_use_reg(instr, use, reg, i);
         } else if (rogue_ref_is_regarray(&backend->src[i].ref)) {
            rogue_regarray_use *use = &backend->src_use[i].regarray;
            rogue_regarray *regarray = backend->src[i].ref.regarray;
            rogue_link_instr_use_regarray(instr, use, regarray, i);
         } else if (rogue_ref_is_drc(&backend->src[i].ref)) {
            rogue_link_drc_trxn(instr->block->shader,
                                instr,
                                rogue_ref_get_drc(&backend->src[i].ref));
         } else if (rogue_ref_is_io(&backend->src[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else if (rogue_ref_is_val(&backend->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_CTRL: {
      rogue_ctrl_instr *ctrl = rogue_instr_as_ctrl(instr);
      const unsigned num_srcs = rogue_ctrl_op_infos[ctrl->op].num_srcs;

      /* Branch instruction. */
      if (!num_srcs && ctrl->target_block) {
         rogue_link_instr_use_block(instr,
                                    &ctrl->block_use,
                                    ctrl->target_block);
         break;
      }

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&ctrl->src[i].ref)) {
            rogue_reg_use *use = &ctrl->src_use[i].reg;
            rogue_reg *reg = ctrl->src[i].ref.reg;
            rogue_link_instr_use_reg(instr, use, reg, i);
         } else if (rogue_ref_is_regarray(&ctrl->src[i].ref)) {
            rogue_regarray_use *use = &ctrl->src_use[i].regarray;
            rogue_regarray *regarray = ctrl->src[i].ref.regarray;
            rogue_link_instr_use_regarray(instr, use, regarray, i);
         } else if (rogue_ref_is_drc(&ctrl->src[i].ref)) {
            /* WDF instructions consume/release drcs, handled independently. */
            if (ctrl->op != ROGUE_CTRL_OP_WDF)
               rogue_link_drc_trxn(instr->block->shader,
                                   instr,
                                   rogue_ref_get_drc(&ctrl->src[i].ref));
         } else if (rogue_ref_is_io(&ctrl->src[i].ref)) { /* TODO: check WHICH
                                                             IO IT IS */
         } else if (rogue_ref_is_val(&ctrl->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BITWISE: {
      rogue_bitwise_instr *bitwise = rogue_instr_as_bitwise(instr);
      const unsigned num_srcs = rogue_bitwise_op_infos[bitwise->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&bitwise->src[i].ref)) {
            rogue_reg_use *use = &bitwise->src_use[i].reg;
            rogue_reg *reg = bitwise->src[i].ref.reg;
            rogue_link_instr_use_reg(instr, use, reg, i);
         } else if (rogue_ref_is_regarray(&bitwise->src[i].ref)) {
            rogue_regarray_use *use = &bitwise->src_use[i].regarray;
            rogue_regarray *regarray = bitwise->src[i].ref.regarray;
            rogue_link_instr_use_regarray(instr, use, regarray, i);
         } else if (rogue_ref_is_drc(&bitwise->src[i].ref)) {
            rogue_link_drc_trxn(instr->block->shader,
                                instr,
                                rogue_ref_get_drc(&bitwise->src[i].ref));
         } else if (rogue_ref_is_io(&bitwise->src[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else if (rogue_ref_is_val(&bitwise->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   default:
      unreachable("Unsupported instruction type.");
   }
}

/**
 * \brief Untracks/unlinks objects that are written to/modified by an
 * instruction.
 *
 * \param[in] instr The instruction.
 */
PUBLIC
void rogue_unlink_instr_write(rogue_instr *instr)
{
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU: {
      rogue_alu_instr *alu = rogue_instr_as_alu(instr);
      const unsigned num_dsts = rogue_alu_op_infos[alu->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&alu->dst[i].ref)) {
            rogue_reg_write *write = &alu->dst_write[i].reg;
            rogue_unlink_instr_write_reg(instr, write);
         } else if (rogue_ref_is_regarray(&alu->dst[i].ref)) {
            rogue_regarray_write *write = &alu->dst_write[i].regarray;
            rogue_unlink_instr_write_regarray(instr, write);
         } else if (rogue_ref_is_io(&alu->dst[i].ref)) { /* TODO: check WHICH IO
                                                            IT IS */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BACKEND: {
      rogue_backend_instr *backend = rogue_instr_as_backend(instr);
      const unsigned num_dsts = rogue_backend_op_infos[backend->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&backend->dst[i].ref)) {
            rogue_reg_write *write = &backend->dst_write[i].reg;
            rogue_unlink_instr_write_reg(instr, write);
         } else if (rogue_ref_is_regarray(&backend->dst[i].ref)) {
            rogue_regarray_write *write = &backend->dst_write[i].regarray;
            rogue_unlink_instr_write_regarray(instr, write);
         } else if (rogue_ref_is_io(&backend->dst[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_CTRL: {
      rogue_ctrl_instr *ctrl = rogue_instr_as_ctrl(instr);
      const unsigned num_dsts = rogue_ctrl_op_infos[ctrl->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&ctrl->dst[i].ref)) {
            rogue_reg_write *write = &ctrl->dst_write[i].reg;
            rogue_unlink_instr_write_reg(instr, write);
         } else if (rogue_ref_is_regarray(&ctrl->dst[i].ref)) {
            rogue_regarray_write *write = &ctrl->dst_write[i].regarray;
            rogue_unlink_instr_write_regarray(instr, write);
         } else if (rogue_ref_is_io(&ctrl->dst[i].ref)) { /* TODO: check WHICH
                                                             IO IT IS */
         } else {
            unreachable("Unsupported destination reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BITWISE: {
      rogue_bitwise_instr *bitwise = rogue_instr_as_bitwise(instr);
      const unsigned num_dsts = rogue_bitwise_op_infos[bitwise->op].num_dsts;

      for (unsigned i = 0; i < num_dsts; ++i) {
         if (rogue_ref_is_reg(&bitwise->dst[i].ref)) {
            rogue_reg_write *write = &bitwise->dst_write[i].reg;
            rogue_unlink_instr_write_reg(instr, write);
         } else if (rogue_ref_is_regarray(&bitwise->dst[i].ref)) {
            rogue_regarray_write *write = &bitwise->dst_write[i].regarray;
            rogue_unlink_instr_write_regarray(instr, write);
         } else {
            unreachable("Invalid destination reference type.");
         }
      }

      break;
   }

   default:
      unreachable("Unsupported instruction type.");
   }
}

/**
 * \brief Untracks/unlinks objects that are used by/read from an instruction.
 *
 * \param[in] instr The instruction.
 */
PUBLIC
void rogue_unlink_instr_use(rogue_instr *instr)
{
   switch (instr->type) {
   case ROGUE_INSTR_TYPE_ALU: {
      rogue_alu_instr *alu = rogue_instr_as_alu(instr);
      const unsigned num_srcs = rogue_alu_op_infos[alu->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&alu->src[i].ref)) {
            rogue_reg_use *use = &alu->src_use[i].reg;
            rogue_unlink_instr_use_reg(instr, use);
         } else if (rogue_ref_is_regarray(&alu->src[i].ref)) {
            rogue_regarray_use *use = &alu->src_use[i].regarray;
            rogue_unlink_instr_use_regarray(instr, use);
         } else if (rogue_ref_is_imm(&alu->src[i].ref)) {
            rogue_unlink_imm_use(instr,
                                 &rogue_ref_get_imm(&alu->src[i].ref)->use);
         } else if (rogue_ref_is_io(&alu->src[i].ref)) { /* TODO: check WHICH IO
                                                            IT IS */
         } else if (rogue_ref_is_val(&alu->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BACKEND: {
      rogue_backend_instr *backend = rogue_instr_as_backend(instr);
      const unsigned num_srcs = rogue_backend_op_infos[backend->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&backend->src[i].ref)) {
            rogue_reg_use *use = &backend->src_use[i].reg;
            rogue_unlink_instr_use_reg(instr, use);
         } else if (rogue_ref_is_regarray(&backend->src[i].ref)) {
            rogue_regarray_use *use = &backend->src_use[i].regarray;
            rogue_unlink_instr_use_regarray(instr, use);
         } else if (rogue_ref_is_drc(&backend->src[i].ref)) {
            rogue_unlink_drc_trxn(instr->block->shader,
                                  instr,
                                  rogue_ref_get_drc(&backend->src[i].ref));
         } else if (rogue_ref_is_io(&backend->src[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else if (rogue_ref_is_val(&backend->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_CTRL: {
      rogue_ctrl_instr *ctrl = rogue_instr_as_ctrl(instr);
      const unsigned num_srcs = rogue_ctrl_op_infos[ctrl->op].num_srcs;

      /* Branch instruction. */
      if (!num_srcs && ctrl->target_block) {
         rogue_unlink_instr_use_block(instr, &ctrl->block_use);
         break;
      }

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&ctrl->src[i].ref)) {
            rogue_reg_use *use = &ctrl->src_use[i].reg;
            rogue_unlink_instr_use_reg(instr, use);
         } else if (rogue_ref_is_regarray(&ctrl->src[i].ref)) {
            rogue_regarray_use *use = &ctrl->src_use[i].regarray;
            rogue_unlink_instr_use_regarray(instr, use);
         } else if (rogue_ref_is_drc(&ctrl->src[i].ref)) {
            /* WDF instructions consume/release drcs, handled independently. */
            if (ctrl->op != ROGUE_CTRL_OP_WDF)
               rogue_unlink_drc_trxn(instr->block->shader,
                                     instr,
                                     rogue_ref_get_drc(&ctrl->src[i].ref));
         } else if (rogue_ref_is_io(&ctrl->src[i].ref)) { /* TODO: check WHICH
                                                             IO IT IS */
         } else if (rogue_ref_is_val(&ctrl->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   case ROGUE_INSTR_TYPE_BITWISE: {
      rogue_bitwise_instr *bitwise = rogue_instr_as_bitwise(instr);
      const unsigned num_srcs = rogue_bitwise_op_infos[bitwise->op].num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         if (rogue_ref_is_reg(&bitwise->src[i].ref)) {
            rogue_reg_use *use = &bitwise->src_use[i].reg;
            rogue_unlink_instr_use_reg(instr, use);
         } else if (rogue_ref_is_regarray(&bitwise->src[i].ref)) {
            rogue_regarray_use *use = &bitwise->src_use[i].regarray;
            rogue_unlink_instr_use_regarray(instr, use);
         } else if (rogue_ref_is_drc(&bitwise->src[i].ref)) {
            rogue_unlink_drc_trxn(instr->block->shader,
                                  instr,
                                  rogue_ref_get_drc(&bitwise->src[i].ref));
         } else if (rogue_ref_is_io(&bitwise->src[i].ref)) { /* TODO: check
                                                                WHICH IO IT IS
                                                              */
         } else if (rogue_ref_is_val(&bitwise->src[i].ref)) {
         } else {
            unreachable("Unsupported source reference type.");
         }
      }

      break;
   }

   default:
      unreachable("Unsupported instruction type.");
   }
}

static void rogue_compiler_destructor(UNUSED void *ptr)
{
   glsl_type_singleton_decref();
}

/**
 * \brief Creates and sets up a Rogue compiler context.
 *
 * \param[in] dev_info Device info pointer.
 * \return A pointer to the new compiler context, or NULL on failure.
 */
PUBLIC
rogue_compiler *rogue_compiler_create(const struct pvr_device_info *dev_info)
{
   rogue_compiler *compiler;

   rogue_debug_init();

   compiler = rzalloc_size(NULL, sizeof(*compiler));
   if (!compiler)
      return NULL;

   compiler->dev_info = dev_info;

   /* TODO: Additional compiler setup (e.g. number of internal registers, BRNs,
    * and other hw-specific info). */

   glsl_type_singleton_init_or_ref();

   ralloc_set_destructor(compiler, rogue_compiler_destructor);

   return compiler;
}

/**
 * \brief Creates and sets up a shared multi-stage build context.
 *
 * \param[in] compiler The compiler context.
 * \return A pointer to the new build context, or NULL on failure.
 */
PUBLIC
rogue_build_ctx *
rogue_build_context_create(rogue_compiler *compiler,
                           struct pvr_pipeline_layout *pipeline_layout)
{
   rogue_build_ctx *ctx;

   ctx = rzalloc_size(NULL, sizeof(*ctx));
   if (!ctx)
      return NULL;

   ctx->compiler = compiler;
   ctx->pipeline_layout = pipeline_layout;

   /* nir/rogue/binary shaders need to be default-zeroed;
    * this is taken care of by rzalloc_size.
    */

   /* Setup non-zero defaults. */
   ctx->stage_data.fs.msaa_mode = ROGUE_MSAA_MODE_PIXEL;

   return ctx;
}
