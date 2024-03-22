
#include <inttypes.h>

#include "util/simple_mtx.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/list.h"

#include "nouveau_winsys.h"
#include "nouveau_screen.h"
#include "nouveau_mm.h"

/* TODO: Higher orders can waste a lot of space for npot size buffers, should
 * add an extra cache for such buffer objects.
 *
 * HACK: Max order == 21 to accommodate TF2's 1.5 MiB, frequently reallocated
 * vertex buffer (VM flush (?) decreases performance dramatically).
 */

#define MM_MIN_ORDER 7 /* >= 6 to not violate ARB_map_buffer_alignment */
#define MM_MAX_ORDER 21

#define MM_NUM_BUCKETS (MM_MAX_ORDER - MM_MIN_ORDER + 1)

#define MM_MIN_SIZE (1 << MM_MIN_ORDER)
#define MM_MAX_SIZE (1 << MM_MAX_ORDER)

struct mm_bucket {
   struct list_head free;
   struct list_head used;
   struct list_head full;
   int num_free;
   simple_mtx_t lock;
};

struct nouveau_mman {
   struct nouveau_device *dev;
   struct mm_bucket bucket[MM_NUM_BUCKETS];
   uint32_t domain;
   union nouveau_bo_config config;
   uint64_t allocated;
};

struct mm_slab {
   struct list_head head;
   struct nouveau_bo *bo;
   struct nouveau_mman *cache;
   int order;
   int count;
   int free;
   uint32_t bits[0];
};

static int
mm_slab_alloc(struct mm_slab *slab)
{
   int i, n, b;

   if (slab->free == 0)
      return -1;

   for (i = 0; i < (slab->count + 31) / 32; ++i) {
      b = ffs(slab->bits[i]) - 1;
      if (b >= 0) {
         n = i * 32 + b;
         assert(n < slab->count);
         slab->free--;
         slab->bits[i] &= ~(1 << b);
         return n;
      }
   }
   return -1;
}

static inline void
mm_slab_free(struct mm_slab *slab, int i)
{
   assert(i < slab->count);
   slab->bits[i / 32] |= 1 << (i % 32);
   slab->free++;
   assert(slab->free <= slab->count);
}

static inline int
mm_get_order(uint32_t size)
{
   int s = __builtin_clz(size) ^ 31;

   if (size > (1 << s))
      s += 1;
   return s;
}

static struct mm_bucket *
mm_bucket_by_order(struct nouveau_mman *cache, int order)
{
   if (order > MM_MAX_ORDER)
      return NULL;
   return &cache->bucket[MAX2(order, MM_MIN_ORDER) - MM_MIN_ORDER];
}

static struct mm_bucket *
mm_bucket_by_size(struct nouveau_mman *cache, unsigned size)
{
   return mm_bucket_by_order(cache, mm_get_order(size));
}

/* size of bo allocation for slab with chunks of (1 << chunk_order) bytes */
static inline uint32_t
mm_default_slab_size(unsigned chunk_order)
{
   static const int8_t slab_order[MM_MAX_ORDER - MM_MIN_ORDER + 1] =
   {
      12, 12, 13, 14, 14, 17, 17, 17, 17, 19, 19, 20, 21, 22, 22
   };

   assert(chunk_order <= MM_MAX_ORDER && chunk_order >= MM_MIN_ORDER);

   return 1 << slab_order[chunk_order - MM_MIN_ORDER];
}

static int
mm_slab_new(struct nouveau_mman *cache, struct mm_bucket *bucket, int chunk_order)
{
   struct mm_slab *slab;
   int words, ret;
   const uint32_t size = mm_default_slab_size(chunk_order);

   simple_mtx_assert_locked(&bucket->lock);

   words = ((size >> chunk_order) + 31) / 32;
   assert(words);

   slab = MALLOC(sizeof(struct mm_slab) + words * 4);
   if (!slab)
      return PIPE_ERROR_OUT_OF_MEMORY;

   memset(&slab->bits[0], ~0, words * 4);

   slab->bo = NULL;

   ret = nouveau_bo_new(cache->dev, cache->domain, 0, size, &cache->config,
                        &slab->bo);
   if (ret) {
      FREE(slab);
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   list_inithead(&slab->head);

   slab->cache = cache;
   slab->order = chunk_order;
   slab->count = slab->free = size >> chunk_order;

   assert(bucket == mm_bucket_by_order(cache, chunk_order));
   list_add(&slab->head, &bucket->free);

   p_atomic_add(&cache->allocated, size);

   if (nouveau_mesa_debug)
      debug_printf("MM: new slab, total memory = %"PRIu64" KiB\n",
                   cache->allocated / 1024);

   return PIPE_OK;
}

/* @return token to identify slab or NULL if we just allocated a new bo */
struct nouveau_mm_allocation *
nouveau_mm_allocate(struct nouveau_mman *cache,
                    uint32_t size, struct nouveau_bo **bo, uint32_t *offset)
{
   struct mm_bucket *bucket;
   struct mm_slab *slab;
   struct nouveau_mm_allocation *alloc;
   int ret;

   bucket = mm_bucket_by_size(cache, size);
   if (!bucket) {
      ret = nouveau_bo_new(cache->dev, cache->domain, 0, size, &cache->config,
                           bo);
      if (ret)
         debug_printf("bo_new(%x, %x): %i\n",
                      size, cache->config.nv50.memtype, ret);

      *offset = 0;
      return NULL;
   }

   alloc = MALLOC_STRUCT(nouveau_mm_allocation);
   if (!alloc)
      return NULL;

   simple_mtx_lock(&bucket->lock);
   if (!list_is_empty(&bucket->used)) {
      slab = list_entry(bucket->used.next, struct mm_slab, head);
   } else {
      if (list_is_empty(&bucket->free)) {
         mm_slab_new(cache, bucket, MAX2(mm_get_order(size), MM_MIN_ORDER));
      }
      slab = list_entry(bucket->free.next, struct mm_slab, head);

      list_del(&slab->head);
      list_add(&slab->head, &bucket->used);
   }

   *offset = mm_slab_alloc(slab) << slab->order;

   nouveau_bo_ref(slab->bo, bo);

   if (slab->free == 0) {
      list_del(&slab->head);
      list_add(&slab->head, &bucket->full);
   }
   simple_mtx_unlock(&bucket->lock);

   alloc->offset = *offset;
   alloc->priv = (void *)slab;

   return alloc;
}

void
nouveau_mm_free(struct nouveau_mm_allocation *alloc)
{
   struct mm_slab *slab = (struct mm_slab *)alloc->priv;
   struct mm_bucket *bucket = mm_bucket_by_order(slab->cache, slab->order);

   simple_mtx_lock(&bucket->lock);
   mm_slab_free(slab, alloc->offset >> slab->order);

   if (slab->free == slab->count) {
      list_del(&slab->head);
      list_addtail(&slab->head, &bucket->free);
   } else
   if (slab->free == 1) {
      list_del(&slab->head);
      list_addtail(&slab->head, &bucket->used);
   }
   simple_mtx_unlock(&bucket->lock);

   FREE(alloc);
}

void
nouveau_mm_free_work(void *data)
{
   nouveau_mm_free(data);
}

struct nouveau_mman *
nouveau_mm_create(struct nouveau_device *dev, uint32_t domain,
                  union nouveau_bo_config *config)
{
   struct nouveau_mman *cache = MALLOC_STRUCT(nouveau_mman);
   int i;

   if (!cache)
      return NULL;

   cache->dev = dev;
   cache->domain = domain;
   cache->config = *config;
   cache->allocated = 0;

   for (i = 0; i < MM_NUM_BUCKETS; ++i) {
      list_inithead(&cache->bucket[i].free);
      list_inithead(&cache->bucket[i].used);
      list_inithead(&cache->bucket[i].full);
      simple_mtx_init(&cache->bucket[i].lock, mtx_plain);
   }

   return cache;
}

static inline void
nouveau_mm_free_slabs(struct list_head *head)
{
   struct mm_slab *slab, *next;

   LIST_FOR_EACH_ENTRY_SAFE(slab, next, head, head) {
      list_del(&slab->head);
      nouveau_bo_ref(NULL, &slab->bo);
      FREE(slab);
   }
}

void
nouveau_mm_destroy(struct nouveau_mman *cache)
{
   int i;

   if (!cache)
      return;

   for (i = 0; i < MM_NUM_BUCKETS; ++i) {
      if (!list_is_empty(&cache->bucket[i].used) ||
          !list_is_empty(&cache->bucket[i].full))
         debug_printf("WARNING: destroying GPU memory cache "
                      "with some buffers still in use\n");

      nouveau_mm_free_slabs(&cache->bucket[i].free);
      nouveau_mm_free_slabs(&cache->bucket[i].used);
      nouveau_mm_free_slabs(&cache->bucket[i].full);
      simple_mtx_destroy(&cache->bucket[i].lock);
   }

   FREE(cache);
}
