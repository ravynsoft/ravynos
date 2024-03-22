#ifndef __NOUVEAU_BUFFER_H__
#define __NOUVEAU_BUFFER_H__

#include "util/u_range.h"
#include "util/u_transfer.h"
#include "util/list.h"

struct pipe_resource;
struct nouveau_context;
struct nouveau_bo;

/* DIRTY: buffer was (or will be after the next flush) written to by GPU and
 *  resource->data has not been updated to reflect modified VRAM contents
 *
 * USER_MEMORY: resource->data is a pointer to client memory and may change
 *  between GL calls
 *
 * USER_PTR: bo is backed by user memory mapped into the GPUs VM
 */
#define NOUVEAU_BUFFER_STATUS_GPU_READING (1 << 0)
#define NOUVEAU_BUFFER_STATUS_GPU_WRITING (1 << 1)
#define NOUVEAU_BUFFER_STATUS_DIRTY       (1 << 2)
#define NOUVEAU_BUFFER_STATUS_USER_PTR    (1 << 6)
#define NOUVEAU_BUFFER_STATUS_USER_MEMORY (1 << 7)

#define NOUVEAU_BUFFER_STATUS_REALLOC_MASK NOUVEAU_BUFFER_STATUS_USER_MEMORY

/* Resources, if mapped into the GPU's address space, are guaranteed to
 * have constant virtual addresses (nv50+).
 *
 * The address of a resource will lie within the nouveau_bo referenced,
 * and this bo should be added to the memory manager's validation list.
 */
struct nv04_resource {
   struct pipe_resource base;

   uint64_t address; /* virtual address (nv50+) */

   uint8_t *data; /* resource's contents, if domain == 0, or cached */
   struct nouveau_bo *bo;
   uint32_t offset; /* offset into the data/bo */

   uint8_t status;
   uint8_t domain;

   uint16_t cb_bindings[6]; /* per-shader per-slot bindings */

   struct nouveau_fence *fence;
   struct nouveau_fence *fence_wr;

   struct nouveau_mm_allocation *mm;

   /* buffer range that has been initialized */
   struct util_range valid_buffer_range;
};

void
nouveau_buffer_release_gpu_storage(struct nv04_resource *);

void
nouveau_copy_buffer(struct nouveau_context *,
                    struct nv04_resource *dst, unsigned dst_pos,
                    struct nv04_resource *src, unsigned src_pos, unsigned size);

bool
nouveau_buffer_migrate(struct nouveau_context *,
                       struct nv04_resource *, unsigned domain);

void *
nouveau_resource_map_offset(struct nouveau_context *, struct nv04_resource *,
                            uint32_t offset, uint32_t flags);

void
nouveau_buffer_destroy(struct pipe_screen *pscreen,
                       struct pipe_resource *presource);

void
nouveau_buffer_transfer_flush_region(struct pipe_context *pipe,
                                     struct pipe_transfer *transfer,
                                     const struct pipe_box *box);

static inline void
nouveau_resource_unmap(struct nv04_resource *res)
{
   /* no-op */
}

static inline struct nv04_resource *
nv04_resource(struct pipe_resource *resource)
{
   return (struct nv04_resource *)resource;
}

/* is resource mapped into the GPU's address space (i.e. VRAM or GART) ? */
static inline bool
nouveau_resource_mapped_by_gpu(struct pipe_resource *resource)
{
   return nv04_resource(resource)->domain != 0;
}

struct pipe_resource *
nouveau_buffer_create(struct pipe_screen *pscreen,
                      const struct pipe_resource *templ);

struct pipe_resource *
nouveau_buffer_create_from_user(struct pipe_screen *pscreen,
                                const struct pipe_resource *templ,
                                void *user_ptr);

struct pipe_resource *
nouveau_user_buffer_create(struct pipe_screen *screen, void *ptr,
                           unsigned bytes, unsigned usage);

bool
nouveau_user_buffer_upload(struct nouveau_context *, struct nv04_resource *,
                           unsigned base, unsigned size);

void
nouveau_buffer_invalidate(struct pipe_context *pipe,
                          struct pipe_resource *resource);

/* Copy data to a scratch buffer and return address & bo the data resides in.
 * Returns 0 on failure.
 */
uint64_t
nouveau_scratch_data(struct nouveau_context *,
                     const void *data, unsigned base, unsigned size,
                     struct nouveau_bo **);

void *
nouveau_buffer_transfer_map(struct pipe_context *pipe,
                            struct pipe_resource *resource,
                            unsigned level, unsigned usage,
                            const struct pipe_box *box,
                            struct pipe_transfer **ptransfer);

void
nouveau_buffer_transfer_unmap(struct pipe_context *pipe,
                              struct pipe_transfer *transfer);

#endif
