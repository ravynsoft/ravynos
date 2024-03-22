/*
 * Copyright © 2021 Intel Corporation
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
#ifndef VK_PIPELINE_CACHE_H
#define VK_PIPELINE_CACHE_H

#include "vk_object.h"
#include "vk_util.h"

#include "util/simple_mtx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* #include "util/blob.h" */
struct blob;
struct blob_reader;

/* #include "util/set.h" */
struct set;

/* #include "compiler/nir/nir.h" */
struct nir_shader;
struct nir_shader_compiler_options;

struct vk_pipeline_cache;
struct vk_pipeline_cache_object;

#define VK_PIPELINE_CACHE_BLOB_ALIGN 8

struct vk_pipeline_cache_object_ops {
   /** Writes this cache object to the given blob
    *
    * Because the cache works with both raw blob data and driver object data
    * and can't always tell the difference between the two, we have to be very
    * careful about alignments when [de]serializing.  When serialize() is
    * called, the blob will be aligned to VK_PIPELINE_CACHE_BLOB_ALIGN.  The
    * driver must be careful to not [de]serialize any data types which require
    * a higher alignment.  When deserialize() is called, the blob_reader is
    * also guaranteed to be aligned to VK_PIPELINE_CACHE_BLOB_ALIGN.
    *
    * Returns true on success
    *
    * This function is optional.  Objects without [de]serialization support
    * will still be cached in memory but will not be placed in the disk cache
    * and will not exported to the client when vkGetPipelineCacheData() is
    * called.
    */
   bool (*serialize)(struct vk_pipeline_cache_object *object,
                     struct blob *blob);

   /** Constructs an object from cached data
    *
    * See serialize() for details about data alignment.
    *
    * returns the created object
    *
    * This function is optional.
    */
   struct vk_pipeline_cache_object *(*deserialize)(struct vk_pipeline_cache *cache,
                                                   const void *key_data,
                                                   size_t key_size,
                                                   struct blob_reader *blob);

   /** Destroys the object
    *
    * Called when vk_pipeline_cache_object.ref_cnt hits 0.
    */
   void (*destroy)(struct vk_device *device,
                   struct vk_pipeline_cache_object *object);
};

/** Base struct for cached objects
 *
 * A vk_pipeline_cache stores any number of vk_pipeline_cache_object's, each
 * of which has an associated key of arbitrary size.  Cached objects are
 * reference counted so that they can exist in multiple caches (for example,
 * when vkMergePipelineCaches() is called) and so that they can persist after
 * the pipeline cache is destroyed.  Each object also has a pointer to a
 * vk_pipeline_cache_object_ops table which the pipeline cache uses to
 * [de]serialize the object and clean it up when the reference count hits 0.
 *
 * The rest of the details of any given object are entirely up to the driver.
 * The driver may even have multiple types of objects (distinguished by their
 * vk_pipeline_cache_object_ops table) in the cache so long as it guarantees
 * it never has two objects of different types with the same key.
 */
struct vk_pipeline_cache_object {
   const struct vk_pipeline_cache_object_ops *ops;
   struct vk_pipeline_cache *weak_owner;
   uint32_t ref_cnt;

   uint32_t data_size;
   const void *key_data;
   uint32_t key_size;
};

static inline void
vk_pipeline_cache_object_init(struct vk_device *device,
                              struct vk_pipeline_cache_object *object,
                              const struct vk_pipeline_cache_object_ops *ops,
                              const void *key_data, uint32_t key_size)
{
   memset(object, 0, sizeof(*object));
   object->ops = ops;
   p_atomic_set(&object->ref_cnt, 1);
   object->data_size = 0; /* Unknown */
   object->key_data = key_data;
   object->key_size = key_size;
}

static inline void
vk_pipeline_cache_object_finish(struct vk_pipeline_cache_object *object)
{
   assert(p_atomic_read(&object->ref_cnt) <= 1);
}

static inline struct vk_pipeline_cache_object *
vk_pipeline_cache_object_ref(struct vk_pipeline_cache_object *object)
{
   assert(object && p_atomic_read(&object->ref_cnt) >= 1);
   p_atomic_inc(&object->ref_cnt);
   return object;
}

void
vk_pipeline_cache_object_unref(struct vk_device *device,
                               struct vk_pipeline_cache_object *object);

/** A generic implementation of VkPipelineCache */
struct vk_pipeline_cache {
   struct vk_object_base base;

   /* pCreateInfo::flags */
   VkPipelineCacheCreateFlags flags;
   bool weak_ref;
   bool skip_disk_cache;

   struct vk_pipeline_cache_header header;

   /** Protects object_cache */
   simple_mtx_t lock;

   struct set *object_cache;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_pipeline_cache, base, VkPipelineCache,
                               VK_OBJECT_TYPE_PIPELINE_CACHE)

struct vk_pipeline_cache_create_info {
   /* The pCreateInfo for this pipeline cache, if any.
    *
    * For driver-internal caches, this is allowed to be NULL.
    */
   const VkPipelineCacheCreateInfo *pCreateInfo;

   /** If true, ignore VK_ENABLE_PIPELINE_CACHE and enable anyway */
   bool force_enable;

   /** If true, the cache operates in weak reference mode.
    *
    * The weak reference mode is designed for device-global caches for the
    * purpose of de-duplicating identical shaders and pipelines.  In the weak
    * reference mode, an object's reference count is not incremented when it is
    * added to the cache.  Therefore the object will be destroyed as soon as
    * there's no external references to it, and the runtime will perform the
    * necessary bookkeeping to remove the dead reference from this cache's table.
    *
    * As the weak reference mode is designed for driver-internal use, it has
    * several limitations:
    * - Merging against a weak reference mode cache is not supported.
    * - Lazy deserialization from vk_raw_data_cache_object_ops is not supported.
    * - An object can only belong to up to one weak reference mode cache.
    * - The cache must outlive the object, as the object will try to access its
    *   owner when it's destroyed.
    */
   bool weak_ref;

   /** If true, do not attempt to use the disk cache */
   bool skip_disk_cache;
};

struct vk_pipeline_cache *
vk_pipeline_cache_create(struct vk_device *device,
                         const struct vk_pipeline_cache_create_info *info,
                         const VkAllocationCallbacks *pAllocator);
void
vk_pipeline_cache_destroy(struct vk_pipeline_cache *cache,
                          const VkAllocationCallbacks *pAllocator);

/** Attempts to look up an object in the cache by key
 *
 * If an object is found in the cache matching the given key, *cache_hit is
 * set to true and a reference to that object is returned.
 *
 * If the driver sets vk_device.disk_cache, we attempt to look up any missing
 * objects in the disk cache before declaring failure.  If an object is found
 * in the disk cache but not the in-memory cache, *cache_hit is set to false.
 *
 * The deserialization of pipeline cache objects found in the cache data
 * provided via VkPipelineCacheCreateInfo::pInitialData happens during
 * vk_pipeline_cache_lookup() rather than during vkCreatePipelineCache().
 * Prior to the first vk_pipeline_cache_lookup() of a given object, it is
 * stored as an internal raw data object with the same hash.  This allows us
 * to avoid any complex object type tagging in the serialized cache.  It does,
 * however, mean that drivers need to be careful to ensure that objects with
 * different types (ops) have different keys.
 *
 * Returns a reference to the object, if found
 */
struct vk_pipeline_cache_object * MUST_CHECK
vk_pipeline_cache_lookup_object(struct vk_pipeline_cache *cache,
                                const void *key_data, size_t key_size,
                                const struct vk_pipeline_cache_object_ops *ops,
                                bool *cache_hit);

/** Adds an object to the pipeline cache
 *
 * This function adds the given object to the pipeline cache.  We do not
 * specify a key here because the key is part of the object. See also
 * vk_pipeline_cache_object_init().
 *
 * This function consumes a reference to the object and returns a reference to
 * the (possibly different) object in the cache.  The intended usage pattern
 * is as follows:
 *
 *    key = compute_key();
 *    struct vk_pipeline_cache_object *object =
 *       vk_pipeline_cache_lookup_object(cache, &key, sizeof(key),
 *                                       &driver_type_ops, &cache_hit);
 *    if (object != NULL)
 *       return container_of(object, driver_type, base);
 *
 *    object = do_compile();
 *    assert(object != NULL);
 *
 *    object = vk_pipeline_cache_add_object(cache, object);
 *    return container_of(object, driver_type, base);
 */
struct vk_pipeline_cache_object * MUST_CHECK
vk_pipeline_cache_add_object(struct vk_pipeline_cache *cache,
                             struct vk_pipeline_cache_object *object);

/** Creates and inserts an object into the pipeline cache
 *
 * This function takes serialized data and emplaces the deserialized object
 * into the pipeline cache.  It is the responsibility of the caller to
 * specify a deserialize() function that properly initializes the object.
 *
 * This function can be used to avoid an extra serialize() step for
 * disk-cache insertion.  For the intended usage pattern, see
 * vk_pipeline_cache_add_object().
 *
 */
struct vk_pipeline_cache_object *
vk_pipeline_cache_create_and_insert_object(struct vk_pipeline_cache *cache,
                                           const void *key_data, uint32_t key_size,
                                           const void *data, size_t data_size,
                                           const struct vk_pipeline_cache_object_ops *ops);

struct nir_shader *
vk_pipeline_cache_lookup_nir(struct vk_pipeline_cache *cache,
                             const void *key_data, size_t key_size,
                             const struct nir_shader_compiler_options *nir_options,
                             bool *cache_hit, void *mem_ctx);
void
vk_pipeline_cache_add_nir(struct vk_pipeline_cache *cache,
                          const void *key_data, size_t key_size,
                          const struct nir_shader *nir);

/** Specialized type of vk_pipeline_cache_object for raw data objects.
 *
 * This cache object implementation, together with vk_raw_data_cache_object_ops,
 * can be used to cache plain objects as well as already serialized data.
 */
struct vk_raw_data_cache_object {
   struct vk_pipeline_cache_object base;

   const void *data;
   size_t data_size;
};

struct vk_raw_data_cache_object *
vk_raw_data_cache_object_create(struct vk_device *device,
                                const void *key_data, size_t key_size,
                                const void *data, size_t data_size);

extern const struct vk_pipeline_cache_object_ops vk_raw_data_cache_object_ops;

#ifdef __cplusplus
}
#endif

#endif /* VK_PIPELINE_CACHE_H */
