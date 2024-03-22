#ifndef __NOUVEAU_MM_H__
#define __NOUVEAU_MM_H__

union nouveau_bo_config;
struct nouveau_mman;

/* Since a resource can be migrated, we need to decouple allocations from
 * them. This struct is linked with fences for delayed freeing of allocs.
 */
struct nouveau_mm_allocation {
   void *priv;
   uint32_t offset;
};

extern struct nouveau_mman *
nouveau_mm_create(struct nouveau_device *, uint32_t domain,
                  union nouveau_bo_config *);

extern void
nouveau_mm_destroy(struct nouveau_mman *);

extern struct nouveau_mm_allocation *
nouveau_mm_allocate(struct nouveau_mman *, uint32_t size,
                    struct nouveau_bo **, uint32_t *offset);

extern void
nouveau_mm_free(struct nouveau_mm_allocation *);

extern void
nouveau_mm_free_work(void *);

#endif // __NOUVEAU_MM_H__
