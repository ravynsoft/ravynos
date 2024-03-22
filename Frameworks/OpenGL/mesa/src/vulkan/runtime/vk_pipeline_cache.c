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

#include "vk_pipeline_cache.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"
#include "vk_physical_device.h"

#include "compiler/nir/nir_serialize.h"

#include "util/blob.h"
#include "util/u_debug.h"
#include "util/disk_cache.h"
#include "util/hash_table.h"
#include "util/set.h"

#define vk_pipeline_cache_log(cache, ...)                                      \
   if (cache->base.client_visible)                                             \
      vk_logw(VK_LOG_OBJS(cache), __VA_ARGS__)

static bool
vk_raw_data_cache_object_serialize(struct vk_pipeline_cache_object *object,
                                   struct blob *blob)
{
   struct vk_raw_data_cache_object *data_obj =
      container_of(object, struct vk_raw_data_cache_object, base);

   blob_write_bytes(blob, data_obj->data, data_obj->data_size);

   return true;
}

static struct vk_pipeline_cache_object *
vk_raw_data_cache_object_deserialize(struct vk_pipeline_cache *cache,
                                     const void *key_data,
                                     size_t key_size,
                                     struct blob_reader *blob)
{
   /* We consume the entire blob_reader.  Each call to ops->deserialize()
    * happens with a brand new blob reader for error checking anyway so we
    * can assume the blob consumes the entire reader and we don't need to
    * serialize the data size separately.
    */
   assert(blob->current < blob->end);
   size_t data_size = blob->end - blob->current;
   const void *data = blob_read_bytes(blob, data_size);

   struct vk_raw_data_cache_object *data_obj =
      vk_raw_data_cache_object_create(cache->base.device, key_data, key_size,
                                      data, data_size);

   return data_obj ? &data_obj->base : NULL;
}

static void
vk_raw_data_cache_object_destroy(struct vk_device *device,
                                 struct vk_pipeline_cache_object *object)
{
   struct vk_raw_data_cache_object *data_obj =
      container_of(object, struct vk_raw_data_cache_object, base);

   vk_free(&device->alloc, data_obj);
}

const struct vk_pipeline_cache_object_ops vk_raw_data_cache_object_ops = {
   .serialize = vk_raw_data_cache_object_serialize,
   .deserialize = vk_raw_data_cache_object_deserialize,
   .destroy = vk_raw_data_cache_object_destroy,
};

struct vk_raw_data_cache_object *
vk_raw_data_cache_object_create(struct vk_device *device,
                                const void *key_data, size_t key_size,
                                const void *data, size_t data_size)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct vk_raw_data_cache_object, data_obj, 1);
   VK_MULTIALLOC_DECL_SIZE(&ma, char, obj_key_data, key_size);
   VK_MULTIALLOC_DECL_SIZE(&ma, char, obj_data, data_size);

   if (!vk_multialloc_alloc(&ma, &device->alloc,
                            VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   vk_pipeline_cache_object_init(device, &data_obj->base,
                                 &vk_raw_data_cache_object_ops,
                                 obj_key_data, key_size);
   data_obj->data = obj_data;
   data_obj->data_size = data_size;

   memcpy(obj_key_data, key_data, key_size);
   memcpy(obj_data, data, data_size);

   return data_obj;
}

static bool
object_keys_equal(const void *void_a, const void *void_b)
{
   const struct vk_pipeline_cache_object *a = void_a, *b = void_b;
   if (a->key_size != b->key_size)
      return false;

   return memcmp(a->key_data, b->key_data, a->key_size) == 0;
}

static uint32_t
object_key_hash(const void *void_object)
{
   const struct vk_pipeline_cache_object *object = void_object;
   return _mesa_hash_data(object->key_data, object->key_size);
}

static void
vk_pipeline_cache_lock(struct vk_pipeline_cache *cache)
{

   if (!(cache->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT))
      simple_mtx_lock(&cache->lock);
}

static void
vk_pipeline_cache_unlock(struct vk_pipeline_cache *cache)
{
   if (!(cache->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT))
      simple_mtx_unlock(&cache->lock);
}

/* cache->lock must be held when calling */
static void
vk_pipeline_cache_remove_object(struct vk_pipeline_cache *cache,
                                uint32_t hash,
                                struct vk_pipeline_cache_object *object)
{
   struct set_entry *entry =
      _mesa_set_search_pre_hashed(cache->object_cache, hash, object);
   if (entry && entry->key == (const void *)object) {
      /* Drop the reference owned by the cache */
      if (!cache->weak_ref)
         vk_pipeline_cache_object_unref(cache->base.device, object);

      _mesa_set_remove(cache->object_cache, entry);
   }
}

static inline struct vk_pipeline_cache_object *
vk_pipeline_cache_object_weak_ref(struct vk_pipeline_cache *cache,
                                  struct vk_pipeline_cache_object *object)
{
   assert(!object->weak_owner);
   p_atomic_set(&object->weak_owner, cache);
   return object;
}

void
vk_pipeline_cache_object_unref(struct vk_device *device, struct vk_pipeline_cache_object *object)
{
   assert(object && p_atomic_read(&object->ref_cnt) >= 1);

   struct vk_pipeline_cache *weak_owner = p_atomic_read(&object->weak_owner);
   if (!weak_owner) {
      if (p_atomic_dec_zero(&object->ref_cnt))
         object->ops->destroy(device, object);
   } else {
      vk_pipeline_cache_lock(weak_owner);
      bool destroy = p_atomic_dec_zero(&object->ref_cnt);
      if (destroy) {
         uint32_t hash = object_key_hash(object);
         vk_pipeline_cache_remove_object(weak_owner, hash, object);
      }
      vk_pipeline_cache_unlock(weak_owner);
      if (destroy)
         object->ops->destroy(device, object);
   }
}

static bool
vk_pipeline_cache_object_serialize(struct vk_pipeline_cache *cache,
                                   struct vk_pipeline_cache_object *object,
                                   struct blob *blob, uint32_t *data_size)
{
   if (object->ops->serialize == NULL)
      return false;

   assert(blob->size == align64(blob->size, VK_PIPELINE_CACHE_BLOB_ALIGN));
   size_t start = blob->size;

   /* Special case for if we're writing to a NULL blob (just to get the size)
    * and we already know the data size of the allocation.  This should make
    * the first GetPipelineCacheData() call to get the data size faster in the
    * common case where a bunch of our objects were loaded from a previous
    * cache or where we've already serialized the cache once.
    */
   if (blob->data == NULL && blob->fixed_allocation) {
      *data_size = p_atomic_read(&object->data_size);
      if (*data_size > 0) {
         blob_write_bytes(blob, NULL, *data_size);
         return true;
      }
   }

   if (!object->ops->serialize(object, blob)) {
      vk_pipeline_cache_log(cache, "Failed to serialize pipeline cache object");
      return false;
   }

   size_t size = blob->size - start;
   if (size > UINT32_MAX) {
      vk_pipeline_cache_log(cache, "Skipping giant (4 GiB or larger) object");
      return false;
   }

   if (blob->out_of_memory) {
      vk_pipeline_cache_log(cache,
                            "Insufficient memory for pipeline cache data");
      return false;
   }

   *data_size = (uint32_t)size;
   p_atomic_set(&object->data_size, *data_size);

   return true;
}

static struct vk_pipeline_cache_object *
vk_pipeline_cache_object_deserialize(struct vk_pipeline_cache *cache,
                                     const void *key_data, uint32_t key_size,
                                     const void *data, size_t data_size,
                                     const struct vk_pipeline_cache_object_ops *ops)
{
   if (ops == NULL)
      ops = &vk_raw_data_cache_object_ops;

   if (unlikely(ops->deserialize == NULL)) {
      vk_pipeline_cache_log(cache,
                            "Pipeline cache object cannot be deserialized");
      return NULL;
   }

   struct blob_reader reader;
   blob_reader_init(&reader, data, data_size);

   struct vk_pipeline_cache_object *object =
      ops->deserialize(cache, key_data, key_size, &reader);

   if (object == NULL)
      return NULL;

   assert(reader.current == reader.end && !reader.overrun);
   assert(object->ops == ops);
   assert(object->ref_cnt == 1);
   assert(object->key_size == key_size);
   assert(memcmp(object->key_data, key_data, key_size) == 0);

   return object;
}

static struct vk_pipeline_cache_object *
vk_pipeline_cache_insert_object(struct vk_pipeline_cache *cache,
                                struct vk_pipeline_cache_object *object)
{
   assert(object->ops != NULL);

   if (cache->object_cache == NULL)
      return object;

   uint32_t hash = object_key_hash(object);

   vk_pipeline_cache_lock(cache);
   bool found = false;
   struct set_entry *entry = _mesa_set_search_or_add_pre_hashed(
       cache->object_cache, hash, object, &found);

   struct vk_pipeline_cache_object *result = NULL;
   /* add reference to either the found or inserted object */
   if (found) {
       struct vk_pipeline_cache_object *found_object = (void *)entry->key;
       if (found_object->ops != object->ops) {
          /* The found object in the cache isn't fully formed. Replace it. */
          assert(!cache->weak_ref);
          assert(found_object->ops == &vk_raw_data_cache_object_ops);
          assert(object->ref_cnt == 1);
          entry->key = object;
          object = found_object;
       }

      result = vk_pipeline_cache_object_ref((void *)entry->key);
   } else {
      result = object;
      if (!cache->weak_ref)
         vk_pipeline_cache_object_ref(result);
      else
         vk_pipeline_cache_object_weak_ref(cache, result);
   }
   vk_pipeline_cache_unlock(cache);

   if (found) {
      vk_pipeline_cache_object_unref(cache->base.device, object);
   }
   return result;
}

struct vk_pipeline_cache_object *
vk_pipeline_cache_lookup_object(struct vk_pipeline_cache *cache,
                                const void *key_data, size_t key_size,
                                const struct vk_pipeline_cache_object_ops *ops,
                                bool *cache_hit)
{
   assert(key_size <= UINT32_MAX);
   assert(ops != NULL);

   if (cache_hit != NULL)
      *cache_hit = false;

   struct vk_pipeline_cache_object key = {
      .key_data = key_data,
      .key_size = key_size,
   };
   uint32_t hash = object_key_hash(&key);

   struct vk_pipeline_cache_object *object = NULL;

   if (cache != NULL && cache->object_cache != NULL) {
      vk_pipeline_cache_lock(cache);
      struct set_entry *entry =
         _mesa_set_search_pre_hashed(cache->object_cache, hash, &key);
      if (entry) {
         object = vk_pipeline_cache_object_ref((void *)entry->key);
         if (cache_hit != NULL)
            *cache_hit = true;
      }
      vk_pipeline_cache_unlock(cache);
   }

   if (object == NULL) {
      struct disk_cache *disk_cache = cache->base.device->physical->disk_cache;
      if (!cache->skip_disk_cache && disk_cache && cache->object_cache) {
         cache_key cache_key;
         disk_cache_compute_key(disk_cache, key_data, key_size, cache_key);

         size_t data_size;
         uint8_t *data = disk_cache_get(disk_cache, cache_key, &data_size);
         if (data) {
            object = vk_pipeline_cache_object_deserialize(cache,
                                                          key_data, key_size,
                                                          data, data_size,
                                                          ops);
            free(data);
            if (object != NULL) {
               return vk_pipeline_cache_insert_object(cache, object);
            }
         }
      }

      /* No disk cache or not found in the disk cache */
      return NULL;
   }

   if (object->ops == &vk_raw_data_cache_object_ops &&
       ops != &vk_raw_data_cache_object_ops) {
      /* The object isn't fully formed yet and we need to deserialize it into
       * a real object before it can be used.
       */
      struct vk_raw_data_cache_object *data_obj =
         container_of(object, struct vk_raw_data_cache_object, base);

      struct vk_pipeline_cache_object *real_object =
         vk_pipeline_cache_object_deserialize(cache,
                                              data_obj->base.key_data,
                                              data_obj->base.key_size,
                                              data_obj->data,
                                              data_obj->data_size, ops);
      if (real_object == NULL) {
         vk_pipeline_cache_log(cache,
                               "Deserializing pipeline cache object failed");

         vk_pipeline_cache_lock(cache);
         vk_pipeline_cache_remove_object(cache, hash, object);
         vk_pipeline_cache_unlock(cache);
         vk_pipeline_cache_object_unref(cache->base.device, object);
         return NULL;
      }

      vk_pipeline_cache_object_unref(cache->base.device, object);
      object = vk_pipeline_cache_insert_object(cache, real_object);
   }

   assert(object->ops == ops);

   return object;
}

struct vk_pipeline_cache_object *
vk_pipeline_cache_add_object(struct vk_pipeline_cache *cache,
                             struct vk_pipeline_cache_object *object)
{
   struct vk_pipeline_cache_object *inserted =
       vk_pipeline_cache_insert_object(cache, object);

   if (object == inserted) {
      /* If it wasn't in the object cache, it might not be in the disk cache
       * either.  Better try and add it.
       */

      struct disk_cache *disk_cache = cache->base.device->physical->disk_cache;
      if (!cache->skip_disk_cache && object->ops->serialize && disk_cache) {
         struct blob blob;
         blob_init(&blob);

         if (object->ops->serialize(object, &blob) && !blob.out_of_memory) {
            cache_key cache_key;
            disk_cache_compute_key(disk_cache, object->key_data,
                                   object->key_size, cache_key);

            disk_cache_put(disk_cache, cache_key, blob.data, blob.size, NULL);
         }

         blob_finish(&blob);
      }
   }

   return inserted;
}

struct vk_pipeline_cache_object *
vk_pipeline_cache_create_and_insert_object(struct vk_pipeline_cache *cache,
                                           const void *key_data, uint32_t key_size,
                                           const void *data, size_t data_size,
                                           const struct vk_pipeline_cache_object_ops *ops)
{
   struct disk_cache *disk_cache = cache->base.device->physical->disk_cache;
   if (!cache->skip_disk_cache && disk_cache) {
      cache_key cache_key;
      disk_cache_compute_key(disk_cache, key_data, key_size, cache_key);
      disk_cache_put(disk_cache, cache_key, data, data_size, NULL);
   }

   struct vk_pipeline_cache_object *object =
       vk_pipeline_cache_object_deserialize(cache, key_data, key_size, data,
                                            data_size, ops);

   if (object)
      object = vk_pipeline_cache_insert_object(cache, object);

   return object;
}

nir_shader *
vk_pipeline_cache_lookup_nir(struct vk_pipeline_cache *cache,
                             const void *key_data, size_t key_size,
                             const struct nir_shader_compiler_options *nir_options,
                             bool *cache_hit, void *mem_ctx)
{
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, key_data, key_size,
                                      &vk_raw_data_cache_object_ops,
                                      cache_hit);
   if (object == NULL)
      return NULL;

   struct vk_raw_data_cache_object *data_obj =
      container_of(object, struct vk_raw_data_cache_object, base);

   struct blob_reader blob;
   blob_reader_init(&blob, data_obj->data, data_obj->data_size);

   nir_shader *nir = nir_deserialize(mem_ctx, nir_options, &blob);
   vk_pipeline_cache_object_unref(cache->base.device, object);

   if (blob.overrun) {
      ralloc_free(nir);
      return NULL;
   }

   return nir;
}

void
vk_pipeline_cache_add_nir(struct vk_pipeline_cache *cache,
                          const void *key_data, size_t key_size,
                          const nir_shader *nir)
{
   struct blob blob;
   blob_init(&blob);

   nir_serialize(&blob, nir, false);
   if (blob.out_of_memory) {
      vk_pipeline_cache_log(cache, "Ran out of memory serializing NIR shader");
      blob_finish(&blob);
      return;
   }

   struct vk_raw_data_cache_object *data_obj =
      vk_raw_data_cache_object_create(cache->base.device,
                                      key_data, key_size,
                                      blob.data, blob.size);
   blob_finish(&blob);

   struct vk_pipeline_cache_object *cached =
      vk_pipeline_cache_add_object(cache, &data_obj->base);
   vk_pipeline_cache_object_unref(cache->base.device, cached);
}

static int32_t
find_type_for_ops(const struct vk_physical_device *pdevice,
                  const struct vk_pipeline_cache_object_ops *ops)
{
   const struct vk_pipeline_cache_object_ops *const *import_ops =
      pdevice->pipeline_cache_import_ops;

   if (import_ops == NULL)
      return -1;

   for (int32_t i = 0; import_ops[i]; i++) {
      if (import_ops[i] == ops)
         return i;
   }

   return -1;
}

static const struct vk_pipeline_cache_object_ops *
find_ops_for_type(const struct vk_physical_device *pdevice,
                  int32_t type)
{
   const struct vk_pipeline_cache_object_ops *const *import_ops =
      pdevice->pipeline_cache_import_ops;

   if (import_ops == NULL || type < 0)
      return NULL;

   return import_ops[type];
}

static void
vk_pipeline_cache_load(struct vk_pipeline_cache *cache,
                       const void *data, size_t size)
{
   struct blob_reader blob;
   blob_reader_init(&blob, data, size);

   struct vk_pipeline_cache_header header;
   blob_copy_bytes(&blob, &header, sizeof(header));
   uint32_t count = blob_read_uint32(&blob);
   if (blob.overrun)
      return;

   if (memcmp(&header, &cache->header, sizeof(header)) != 0)
      return;

   for (uint32_t i = 0; i < count; i++) {
      int32_t type = blob_read_uint32(&blob);
      uint32_t key_size = blob_read_uint32(&blob);
      uint32_t data_size = blob_read_uint32(&blob);
      const void *key_data = blob_read_bytes(&blob, key_size);
      blob_reader_align(&blob, VK_PIPELINE_CACHE_BLOB_ALIGN);
      const void *data = blob_read_bytes(&blob, data_size);
      if (blob.overrun)
         break;

      const struct vk_pipeline_cache_object_ops *ops =
         find_ops_for_type(cache->base.device->physical, type);

      struct vk_pipeline_cache_object *object =
         vk_pipeline_cache_create_and_insert_object(cache, key_data, key_size,
                                                    data, data_size, ops);

      if (object == NULL) {
         vk_pipeline_cache_log(cache, "Failed to load pipeline cache object");
         continue;
      }

      vk_pipeline_cache_object_unref(cache->base.device, object);
   }
}

struct vk_pipeline_cache *
vk_pipeline_cache_create(struct vk_device *device,
                         const struct vk_pipeline_cache_create_info *info,
                         const VkAllocationCallbacks *pAllocator)
{
   static const struct VkPipelineCacheCreateInfo default_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
   };
   struct vk_pipeline_cache *cache;

   const struct VkPipelineCacheCreateInfo *pCreateInfo =
      info->pCreateInfo != NULL ? info->pCreateInfo : &default_create_info;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);

   cache = vk_object_zalloc(device, pAllocator, sizeof(*cache),
                            VK_OBJECT_TYPE_PIPELINE_CACHE);
   if (cache == NULL)
      return NULL;

   cache->flags = pCreateInfo->flags;
   cache->weak_ref = info->weak_ref;
#ifndef ENABLE_SHADER_CACHE
   cache->skip_disk_cache = true;
#else
   cache->skip_disk_cache = info->skip_disk_cache;
#endif

   struct VkPhysicalDeviceProperties pdevice_props;
   device->physical->dispatch_table.GetPhysicalDeviceProperties(
      vk_physical_device_to_handle(device->physical), &pdevice_props);

   cache->header = (struct vk_pipeline_cache_header) {
      .header_size = sizeof(struct vk_pipeline_cache_header),
      .header_version = VK_PIPELINE_CACHE_HEADER_VERSION_ONE,
      .vendor_id = pdevice_props.vendorID,
      .device_id = pdevice_props.deviceID,
   };
   memcpy(cache->header.uuid, pdevice_props.pipelineCacheUUID, VK_UUID_SIZE);

   simple_mtx_init(&cache->lock, mtx_plain);

   if (info->force_enable ||
       debug_get_bool_option("VK_ENABLE_PIPELINE_CACHE", true)) {
      cache->object_cache = _mesa_set_create(NULL, object_key_hash,
                                             object_keys_equal);
   }

   if (cache->object_cache && pCreateInfo->initialDataSize > 0) {
      vk_pipeline_cache_load(cache, pCreateInfo->pInitialData,
                             pCreateInfo->initialDataSize);
   }

   return cache;
}

void
vk_pipeline_cache_destroy(struct vk_pipeline_cache *cache,
                          const VkAllocationCallbacks *pAllocator)
{
   if (cache->object_cache) {
      if (!cache->weak_ref) {
         set_foreach(cache->object_cache, entry) {
            vk_pipeline_cache_object_unref(cache->base.device, (void *)entry->key);
         }
      } else {
         assert(cache->object_cache->entries == 0);
      }
      _mesa_set_destroy(cache->object_cache, NULL);
   }
   simple_mtx_destroy(&cache->lock);
   vk_object_free(cache->base.device, pAllocator, cache);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreatePipelineCache(VkDevice _device,
                              const VkPipelineCacheCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *pAllocator,
                              VkPipelineCache *pPipelineCache)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_pipeline_cache *cache;

   struct vk_pipeline_cache_create_info info = {
      .pCreateInfo = pCreateInfo,
   };
   cache = vk_pipeline_cache_create(device, &info, pAllocator);
   if (cache == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   *pPipelineCache = vk_pipeline_cache_to_handle(cache);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyPipelineCache(VkDevice device,
                               VkPipelineCache pipelineCache,
                               const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);

   if (cache == NULL)
      return;

   assert(cache->base.device == vk_device_from_handle(device));
   vk_pipeline_cache_destroy(cache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetPipelineCacheData(VkDevice _device,
                               VkPipelineCache pipelineCache,
                               size_t *pDataSize,
                               void *pData)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);

   struct blob blob;
   if (pData) {
      blob_init_fixed(&blob, pData, *pDataSize);
   } else {
      blob_init_fixed(&blob, NULL, SIZE_MAX);
   }

   blob_write_bytes(&blob, &cache->header, sizeof(cache->header));

   uint32_t count = 0;
   intptr_t count_offset = blob_reserve_uint32(&blob);
   if (count_offset < 0) {
      *pDataSize = 0;
      blob_finish(&blob);
      return VK_INCOMPLETE;
   }

   vk_pipeline_cache_lock(cache);

   VkResult result = VK_SUCCESS;
   if (cache->object_cache != NULL) {
      set_foreach(cache->object_cache, entry) {
         struct vk_pipeline_cache_object *object = (void *)entry->key;

         if (object->ops->serialize == NULL)
            continue;

         size_t blob_size_save = blob.size;

         int32_t type = find_type_for_ops(device->physical, object->ops);
         blob_write_uint32(&blob, type);
         blob_write_uint32(&blob, object->key_size);
         intptr_t data_size_resv = blob_reserve_uint32(&blob);
         blob_write_bytes(&blob, object->key_data, object->key_size);

         if (!blob_align(&blob, VK_PIPELINE_CACHE_BLOB_ALIGN)) {
            result = VK_INCOMPLETE;
            break;
         }

         uint32_t data_size;
         if (!vk_pipeline_cache_object_serialize(cache, object,
                                                 &blob, &data_size)) {
            blob.size = blob_size_save;
            if (blob.out_of_memory) {
               result = VK_INCOMPLETE;
               break;
            }

            /* Failed for some other reason; keep going */
            continue;
         }

         /* vk_pipeline_cache_object_serialize should have failed */
         assert(!blob.out_of_memory);

         assert(data_size_resv >= 0);
         blob_overwrite_uint32(&blob, data_size_resv, data_size);

         count++;
      }
   }

   vk_pipeline_cache_unlock(cache);

   blob_overwrite_uint32(&blob, count_offset, count);

   *pDataSize = blob.size;

   blob_finish(&blob);

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_MergePipelineCaches(VkDevice _device,
                              VkPipelineCache dstCache,
                              uint32_t srcCacheCount,
                              const VkPipelineCache *pSrcCaches)
{
   VK_FROM_HANDLE(vk_pipeline_cache, dst, dstCache);
   VK_FROM_HANDLE(vk_device, device, _device);
   assert(dst->base.device == device);
   assert(!dst->weak_ref);

   if (!dst->object_cache)
      return VK_SUCCESS;

   vk_pipeline_cache_lock(dst);

   for (uint32_t i = 0; i < srcCacheCount; i++) {
      VK_FROM_HANDLE(vk_pipeline_cache, src, pSrcCaches[i]);
      assert(src->base.device == device);

      if (!src->object_cache)
         continue;

      assert(src != dst);
      if (src == dst)
         continue;

      vk_pipeline_cache_lock(src);

      set_foreach(src->object_cache, src_entry) {
         struct vk_pipeline_cache_object *src_object = (void *)src_entry->key;

         bool found_in_dst = false;
         struct set_entry *dst_entry =
            _mesa_set_search_or_add_pre_hashed(dst->object_cache,
                                               src_entry->hash,
                                               src_object, &found_in_dst);
         if (found_in_dst) {
            struct vk_pipeline_cache_object *dst_object = (void *)dst_entry->key;
            if (dst_object->ops == &vk_raw_data_cache_object_ops &&
                src_object->ops != &vk_raw_data_cache_object_ops) {
               /* Even though dst has the object, it only has the blob version
                * which isn't as useful.  Replace it with the real object.
                */
               vk_pipeline_cache_object_unref(device, dst_object);
               dst_entry->key = vk_pipeline_cache_object_ref(src_object);
            }
         } else {
            /* We inserted src_object in dst so it needs a reference */
            assert(dst_entry->key == (const void *)src_object);
            vk_pipeline_cache_object_ref(src_object);
         }
      }

      vk_pipeline_cache_unlock(src);
   }

   vk_pipeline_cache_unlock(dst);

   return VK_SUCCESS;
}
