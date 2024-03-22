/*
 * Copyright (C) 2019 Alyssa Rosenzweig
 * Copyright (C) 2017-2018 Lyude Paul
 * Copyright (C) 2019 Collabora, Ltd.
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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "util/macros.h"
#include "util/u_debug.h"
#include "util/u_hexdump.h"
#include "decode.h"

#include "compiler/bifrost/disassemble.h"
#include "compiler/valhall/disassemble.h"
#include "midgard/disassemble.h"

/* Used to distiguish dumped files, otherwise we would have to print the ctx
 * pointer, which is annoying for the user since it changes with every run */
static int num_ctxs = 0;

#define to_mapped_memory(x)                                                    \
   rb_node_data(struct pandecode_mapped_memory, x, node)

/*
 * Compare a GPU VA to a node, considering a GPU VA to be equal to a node if it
 * is contained in the interval the node represents. This lets us store
 * intervals in our tree.
 */
static int
pandecode_cmp_key(const struct rb_node *lhs, const void *key)
{
   struct pandecode_mapped_memory *mem = to_mapped_memory(lhs);
   uint64_t *gpu_va = (uint64_t *)key;

   if (mem->gpu_va <= *gpu_va && *gpu_va < (mem->gpu_va + mem->length))
      return 0;
   else
      return mem->gpu_va - *gpu_va;
}

static int
pandecode_cmp(const struct rb_node *lhs, const struct rb_node *rhs)
{
   return to_mapped_memory(lhs)->gpu_va - to_mapped_memory(rhs)->gpu_va;
}

static struct pandecode_mapped_memory *
pandecode_find_mapped_gpu_mem_containing_rw(struct pandecode_context *ctx,
                                            uint64_t addr)
{
   simple_mtx_assert_locked(&ctx->lock);

   struct rb_node *node =
      rb_tree_search(&ctx->mmap_tree, &addr, pandecode_cmp_key);

   return to_mapped_memory(node);
}

struct pandecode_mapped_memory *
pandecode_find_mapped_gpu_mem_containing(struct pandecode_context *ctx,
                                         uint64_t addr)
{
   simple_mtx_assert_locked(&ctx->lock);

   struct pandecode_mapped_memory *mem =
      pandecode_find_mapped_gpu_mem_containing_rw(ctx, addr);

   if (mem && mem->addr && !mem->ro) {
      mprotect(mem->addr, mem->length, PROT_READ);
      mem->ro = true;
      util_dynarray_append(&ctx->ro_mappings, struct pandecode_mapped_memory *,
                           mem);
   }

   return mem;
}

/*
 * To check for memory safety issues, validates that the given pointer in GPU
 * memory is valid, containing at least sz bytes. This function is a tool to
 * detect GPU-side memory bugs by validating pointers.
 */
void
pandecode_validate_buffer(struct pandecode_context *ctx, mali_ptr addr,
                          size_t sz)
{
   if (!addr) {
      pandecode_log(ctx, "// XXX: null pointer deref\n");
      return;
   }

   /* Find a BO */

   struct pandecode_mapped_memory *bo =
      pandecode_find_mapped_gpu_mem_containing(ctx, addr);

   if (!bo) {
      pandecode_log(ctx, "// XXX: invalid memory dereference\n");
      return;
   }

   /* Bounds check */

   unsigned offset = addr - bo->gpu_va;
   unsigned total = offset + sz;

   if (total > bo->length) {
      pandecode_log(ctx,
                    "// XXX: buffer overrun. "
                    "Chunk of size %zu at offset %d in buffer of size %zu. "
                    "Overrun by %zu bytes. \n",
                    sz, offset, bo->length, total - bo->length);
      return;
   }
}

void
pandecode_map_read_write(struct pandecode_context *ctx)
{
   simple_mtx_assert_locked(&ctx->lock);

   util_dynarray_foreach(&ctx->ro_mappings, struct pandecode_mapped_memory *,
                         mem) {
      (*mem)->ro = false;
      mprotect((*mem)->addr, (*mem)->length, PROT_READ | PROT_WRITE);
   }
   util_dynarray_clear(&ctx->ro_mappings);
}

static void
pandecode_add_name(struct pandecode_context *ctx,
                   struct pandecode_mapped_memory *mem, uint64_t gpu_va,
                   const char *name)
{
   simple_mtx_assert_locked(&ctx->lock);

   if (!name) {
      /* If we don't have a name, assign one */

      snprintf(mem->name, sizeof(mem->name) - 1, "memory_%" PRIx64, gpu_va);
   } else {
      assert((strlen(name) + 1) < sizeof(mem->name));
      memcpy(mem->name, name, strlen(name) + 1);
   }
}

void
pandecode_inject_mmap(struct pandecode_context *ctx, uint64_t gpu_va, void *cpu,
                      unsigned sz, const char *name)
{
   simple_mtx_lock(&ctx->lock);

   /* First, search if we already mapped this and are just updating an address */

   struct pandecode_mapped_memory *existing =
      pandecode_find_mapped_gpu_mem_containing_rw(ctx, gpu_va);

   if (existing && existing->gpu_va == gpu_va) {
      existing->length = sz;
      existing->addr = cpu;
      pandecode_add_name(ctx, existing, gpu_va, name);
   } else {
      /* Otherwise, add a fresh mapping */
      struct pandecode_mapped_memory *mapped_mem = NULL;

      mapped_mem = calloc(1, sizeof(*mapped_mem));
      mapped_mem->gpu_va = gpu_va;
      mapped_mem->length = sz;
      mapped_mem->addr = cpu;
      pandecode_add_name(ctx, mapped_mem, gpu_va, name);

      /* Add it to the tree */
      rb_tree_insert(&ctx->mmap_tree, &mapped_mem->node, pandecode_cmp);
   }

   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_inject_free(struct pandecode_context *ctx, uint64_t gpu_va,
                      unsigned sz)
{
   simple_mtx_lock(&ctx->lock);

   struct pandecode_mapped_memory *mem =
      pandecode_find_mapped_gpu_mem_containing_rw(ctx, gpu_va);

   if (mem) {
      assert(mem->gpu_va == gpu_va);
      assert(mem->length == sz);

      rb_tree_remove(&ctx->mmap_tree, &mem->node);
      free(mem);
   }

   simple_mtx_unlock(&ctx->lock);
}

char *
pointer_as_memory_reference(struct pandecode_context *ctx, uint64_t ptr)
{
   simple_mtx_assert_locked(&ctx->lock);

   struct pandecode_mapped_memory *mapped;
   char *out = malloc(128);

   /* Try to find the corresponding mapped zone */

   mapped = pandecode_find_mapped_gpu_mem_containing_rw(ctx, ptr);

   if (mapped) {
      snprintf(out, 128, "%s + %d", mapped->name, (int)(ptr - mapped->gpu_va));
      return out;
   }

   /* Just use the raw address if other options are exhausted */

   snprintf(out, 128, "0x%" PRIx64, ptr);
   return out;
}

void
pandecode_dump_file_open(struct pandecode_context *ctx)
{
   simple_mtx_assert_locked(&ctx->lock);

   /* This does a getenv every frame, so it is possible to use
    * setenv to change the base at runtime.
    */
   const char *dump_file_base =
      debug_get_option("PANDECODE_DUMP_FILE", "pandecode.dump");
   if (!strcmp(dump_file_base, "stderr"))
      ctx->dump_stream = stderr;
   else if (!ctx->dump_stream) {
      char buffer[1024];
      snprintf(buffer, sizeof(buffer), "%s.ctx-%d.%04d", dump_file_base,
               ctx->id, ctx->dump_frame_count);
      printf("pandecode: dump command stream to file %s\n", buffer);
      ctx->dump_stream = fopen(buffer, "w");
      if (!ctx->dump_stream)
         fprintf(stderr,
                 "pandecode: failed to open command stream log file %s\n",
                 buffer);
   }
}

static void
pandecode_dump_file_close(struct pandecode_context *ctx)
{
   simple_mtx_assert_locked(&ctx->lock);

   if (ctx->dump_stream && ctx->dump_stream != stderr) {
      if (fclose(ctx->dump_stream))
         perror("pandecode: dump file");

      ctx->dump_stream = NULL;
   }
}

struct pandecode_context *
pandecode_create_context(bool to_stderr)
{
   struct pandecode_context *ctx = calloc(1, sizeof(*ctx));

   /* Not thread safe, but we shouldn't ever hit this, and even if we do, the
    * worst that could happen is having the files dumped with their filenames
    * in a different order. */
   ctx->id = num_ctxs++;

   /* This will be initialized later and can be changed at run time through
    * the PANDECODE_DUMP_FILE environment variable.
    */
   ctx->dump_stream = to_stderr ? stderr : NULL;

   rb_tree_init(&ctx->mmap_tree);
   util_dynarray_init(&ctx->ro_mappings, NULL);

   simple_mtx_t mtx_init = SIMPLE_MTX_INITIALIZER;
   memcpy(&ctx->lock, &mtx_init, sizeof(simple_mtx_t));

   return ctx;
}

void
pandecode_next_frame(struct pandecode_context *ctx)
{
   simple_mtx_lock(&ctx->lock);

   pandecode_dump_file_close(ctx);
   ctx->dump_frame_count++;

   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_destroy_context(struct pandecode_context *ctx)
{
   simple_mtx_lock(&ctx->lock);

   rb_tree_foreach_safe(struct pandecode_mapped_memory, it, &ctx->mmap_tree,
                        node) {
      rb_tree_remove(&ctx->mmap_tree, &it->node);
      free(it);
   }

   util_dynarray_fini(&ctx->ro_mappings);
   pandecode_dump_file_close(ctx);

   simple_mtx_unlock(&ctx->lock);

   free(ctx);
}

void
pandecode_dump_mappings(struct pandecode_context *ctx)
{
   simple_mtx_lock(&ctx->lock);

   pandecode_dump_file_open(ctx);

   rb_tree_foreach(struct pandecode_mapped_memory, it, &ctx->mmap_tree, node) {
      if (!it->addr || !it->length)
         continue;

      fprintf(ctx->dump_stream, "Buffer: %s gpu %" PRIx64 "\n\n", it->name,
              it->gpu_va);

      u_hexdump(ctx->dump_stream, it->addr, it->length, false);
      fprintf(ctx->dump_stream, "\n");
   }

   fflush(ctx->dump_stream);
   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_abort_on_fault(struct pandecode_context *ctx, mali_ptr jc_gpu_va,
                         unsigned gpu_id)
{
   simple_mtx_lock(&ctx->lock);

   switch (pan_arch(gpu_id)) {
   case 4:
      pandecode_abort_on_fault_v4(ctx, jc_gpu_va);
      break;
   case 5:
      pandecode_abort_on_fault_v5(ctx, jc_gpu_va);
      break;
   case 6:
      pandecode_abort_on_fault_v6(ctx, jc_gpu_va);
      break;
   case 7:
      pandecode_abort_on_fault_v7(ctx, jc_gpu_va);
      break;
   case 9:
      pandecode_abort_on_fault_v9(ctx, jc_gpu_va);
      break;
   default:
      unreachable("Unsupported architecture");
   }

   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_jc(struct pandecode_context *ctx, mali_ptr jc_gpu_va, unsigned gpu_id)
{
   simple_mtx_lock(&ctx->lock);

   switch (pan_arch(gpu_id)) {
   case 4:
      pandecode_jc_v4(ctx, jc_gpu_va, gpu_id);
      break;
   case 5:
      pandecode_jc_v5(ctx, jc_gpu_va, gpu_id);
      break;
   case 6:
      pandecode_jc_v6(ctx, jc_gpu_va, gpu_id);
      break;
   case 7:
      pandecode_jc_v7(ctx, jc_gpu_va, gpu_id);
      break;
   case 9:
      pandecode_jc_v9(ctx, jc_gpu_va, gpu_id);
      break;
   default:
      unreachable("Unsupported architecture");
   }

   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_cs(struct pandecode_context *ctx, mali_ptr queue_gpu_va,
             uint32_t size, unsigned gpu_id, uint32_t *regs)
{
   simple_mtx_lock(&ctx->lock);

   switch (pan_arch(gpu_id)) {
   case 10:
      pandecode_cs_v10(ctx, queue_gpu_va, size, gpu_id, regs);
      break;
   default:
      unreachable("Unsupported architecture");
   }

   simple_mtx_unlock(&ctx->lock);
}

void
pandecode_shader_disassemble(struct pandecode_context *ctx, mali_ptr shader_ptr,
                             unsigned gpu_id)
{
   uint8_t *PANDECODE_PTR_VAR(ctx, code, shader_ptr);

   /* Compute maximum possible size */
   struct pandecode_mapped_memory *mem =
      pandecode_find_mapped_gpu_mem_containing(ctx, shader_ptr);
   size_t sz = mem->length - (shader_ptr - mem->gpu_va);

   /* Print some boilerplate to clearly denote the assembly (which doesn't
    * obey indentation rules), and actually do the disassembly! */

   pandecode_log_cont(ctx, "\nShader %p (GPU VA %" PRIx64 ") sz %" PRId64 "\n",
                      code, shader_ptr, sz);

   if (pan_arch(gpu_id) >= 9) {
      disassemble_valhall(ctx->dump_stream, (const uint64_t *)code, sz, true);
   } else if (pan_arch(gpu_id) >= 6)
      disassemble_bifrost(ctx->dump_stream, code, sz, false);
   else
      disassemble_midgard(ctx->dump_stream, code, sz, gpu_id, true);

   pandecode_log_cont(ctx, "\n\n");
}
