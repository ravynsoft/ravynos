/*
 * Copyright © 2019 Raspberry Pi Ltd
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

#include "v3dv_private.h"
#include "vulkan/util/vk_util.h"
#include "util/blob.h"
#include "nir/nir_serialize.h"

static const bool debug_cache = false;
static const bool dump_stats = false;
static const bool dump_stats_on_destroy = false;

/* Shared for nir/variants */
#define V3DV_MAX_PIPELINE_CACHE_ENTRIES 4096

static uint32_t
sha1_hash_func(const void *sha1)
{
   return _mesa_hash_data(sha1, 20);
}

static bool
sha1_compare_func(const void *sha1_a, const void *sha1_b)
{
   return memcmp(sha1_a, sha1_b, 20) == 0;
}

struct serialized_nir {
   unsigned char sha1_key[20];
   size_t size;
   char data[0];
};

static void
cache_dump_stats(struct v3dv_pipeline_cache *cache)
{
   fprintf(stderr, "  NIR cache entries:      %d\n", cache->nir_stats.count);
   fprintf(stderr, "  NIR cache miss count:   %d\n", cache->nir_stats.miss);
   fprintf(stderr, "  NIR cache hit  count:   %d\n", cache->nir_stats.hit);

   fprintf(stderr, "  cache entries:      %d\n", cache->stats.count);
   fprintf(stderr, "  cache miss count:   %d\n", cache->stats.miss);
   fprintf(stderr, "  cache hit  count:   %d\n", cache->stats.hit);

   fprintf(stderr, "  on-disk cache hit  count:   %d\n", cache->stats.on_disk_hit);
}

static void
pipeline_cache_lock(struct v3dv_pipeline_cache *cache)
{
   if (!cache->externally_synchronized)
      mtx_lock(&cache->mutex);
}

static void
pipeline_cache_unlock(struct v3dv_pipeline_cache *cache)
{
   if (!cache->externally_synchronized)
      mtx_unlock(&cache->mutex);
}

void
v3dv_pipeline_cache_upload_nir(struct v3dv_pipeline *pipeline,
                               struct v3dv_pipeline_cache *cache,
                               nir_shader *nir,
                               unsigned char sha1_key[20])
{
   if (!cache || !cache->nir_cache)
      return;

   if (cache->nir_stats.count > V3DV_MAX_PIPELINE_CACHE_ENTRIES)
      return;

   pipeline_cache_lock(cache);
   struct hash_entry *entry =
      _mesa_hash_table_search(cache->nir_cache, sha1_key);
   pipeline_cache_unlock(cache);
   if (entry)
      return;

   struct blob blob;
   blob_init(&blob);

   nir_serialize(&blob, nir, false);
   if (blob.out_of_memory) {
      blob_finish(&blob);
      return;
   }

   pipeline_cache_lock(cache);
   /* Because ralloc isn't thread-safe, we have to do all this inside the
    * lock.  We could unlock for the big memcpy but it's probably not worth
    * the hassle.
    */
   entry = _mesa_hash_table_search(cache->nir_cache, sha1_key);
   if (entry) {
      blob_finish(&blob);
      pipeline_cache_unlock(cache);
      return;
   }

   struct serialized_nir *snir =
      ralloc_size(cache->nir_cache, sizeof(*snir) + blob.size);
   memcpy(snir->sha1_key, sha1_key, 20);
   snir->size = blob.size;
   memcpy(snir->data, blob.data, blob.size);

   blob_finish(&blob);

   cache->nir_stats.count++;
   if (debug_cache) {
      char sha1buf[41];
      _mesa_sha1_format(sha1buf, snir->sha1_key);
      fprintf(stderr, "pipeline cache %p, new nir entry %s\n", cache, sha1buf);
      if (dump_stats)
         cache_dump_stats(cache);
   }

   _mesa_hash_table_insert(cache->nir_cache, snir->sha1_key, snir);

   pipeline_cache_unlock(cache);
}

nir_shader*
v3dv_pipeline_cache_search_for_nir(struct v3dv_pipeline *pipeline,
                                   struct v3dv_pipeline_cache *cache,
                                   const nir_shader_compiler_options *nir_options,
                                   unsigned char sha1_key[20])
{
   if (!cache || !cache->nir_cache)
      return NULL;

   if (debug_cache) {
      char sha1buf[41];
      _mesa_sha1_format(sha1buf, sha1_key);

      fprintf(stderr, "pipeline cache %p, search for nir %s\n", cache, sha1buf);
   }

   const struct serialized_nir *snir = NULL;

   pipeline_cache_lock(cache);
   struct hash_entry *entry =
      _mesa_hash_table_search(cache->nir_cache, sha1_key);
   if (entry)
      snir = entry->data;
   pipeline_cache_unlock(cache);

   if (snir) {
      struct blob_reader blob;
      blob_reader_init(&blob, snir->data, snir->size);

      /* We use context NULL as we want the p_stage to keep the reference to
       * nir, as we keep open the possibility of provide a shader variant
       * after cache creation
       */
      nir_shader *nir = nir_deserialize(NULL, nir_options, &blob);
      if (blob.overrun) {
         ralloc_free(nir);
      } else {
         cache->nir_stats.hit++;
         if (debug_cache) {
            fprintf(stderr, "[v3dv nir cache] hit: %p\n", nir);
            if (dump_stats)
               cache_dump_stats(cache);
         }
         return nir;
      }
   }

   cache->nir_stats.miss++;
   if (debug_cache) {
      fprintf(stderr, "[v3dv nir cache] miss\n");
      if (dump_stats)
         cache_dump_stats(cache);
   }

   return NULL;
}

void
v3dv_pipeline_cache_init(struct v3dv_pipeline_cache *cache,
                         struct v3dv_device *device,
                         VkPipelineCacheCreateFlags flags,
                         bool cache_enabled)
{
   cache->device = device;
   mtx_init(&cache->mutex, mtx_plain);

   if (cache_enabled) {
      cache->nir_cache = _mesa_hash_table_create(NULL, sha1_hash_func,
                                                 sha1_compare_func);
      cache->nir_stats.miss = 0;
      cache->nir_stats.hit = 0;
      cache->nir_stats.count = 0;

      cache->cache = _mesa_hash_table_create(NULL, sha1_hash_func,
                                             sha1_compare_func);
      cache->stats.miss = 0;
      cache->stats.hit = 0;
      cache->stats.count = 0;

      cache->externally_synchronized = flags &
         VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
   } else {
      cache->nir_cache = NULL;
      cache->cache = NULL;
   }

}

static struct v3dv_pipeline_shared_data *
v3dv_pipeline_shared_data_create_from_blob(struct v3dv_pipeline_cache *cache,
                                           struct blob_reader *blob);

static void
pipeline_cache_upload_shared_data(struct v3dv_pipeline_cache *cache,
                                  struct v3dv_pipeline_shared_data *shared_data,
                                  bool from_disk_cache);

static bool
v3dv_pipeline_shared_data_write_to_blob(const struct v3dv_pipeline_shared_data *cache_entry,
                                        struct blob *blob);

/**
 * It searches for pipeline cached data, and returns a v3dv_pipeline_shared_data with
 * it, or NULL if doesn't have it cached. On the former, it will increases the
 * ref_count, so caller is responsible to unref it.
 */
struct v3dv_pipeline_shared_data *
v3dv_pipeline_cache_search_for_pipeline(struct v3dv_pipeline_cache *cache,
                                        unsigned char sha1_key[20],
                                        bool *cache_hit)
{
   if (!cache || !cache->cache)
      return NULL;

   if (debug_cache) {
      char sha1buf[41];
      _mesa_sha1_format(sha1buf, sha1_key);

      fprintf(stderr, "pipeline cache %p, search pipeline with key %s\n", cache, sha1buf);
   }

   pipeline_cache_lock(cache);

   struct hash_entry *entry =
      _mesa_hash_table_search(cache->cache, sha1_key);

   if (entry) {
      struct v3dv_pipeline_shared_data *cache_entry =
         (struct v3dv_pipeline_shared_data *) entry->data;
      assert(cache_entry);

      cache->stats.hit++;
      *cache_hit = true;
      if (debug_cache) {
         fprintf(stderr, "[v3dv cache] hit: %p\n", cache_entry);
         if (dump_stats)
            cache_dump_stats(cache);
      }


      v3dv_pipeline_shared_data_ref(cache_entry);

      pipeline_cache_unlock(cache);

      return cache_entry;
   }

   cache->stats.miss++;
   if (debug_cache) {
      fprintf(stderr, "[v3dv cache] miss\n");
      if (dump_stats)
         cache_dump_stats(cache);
   }

   pipeline_cache_unlock(cache);

#ifdef ENABLE_SHADER_CACHE
   struct v3dv_device *device = cache->device;
   struct disk_cache *disk_cache = device->pdevice->disk_cache;
   /* Note that the on-disk-cache can be independently disabled, while keeping
    * the pipeline cache working, by using the environment variable
    * MESA_SHADER_CACHE_DISABLE. In that case the calls to disk_cache_put/get
    * will not do anything.
    */
   if (disk_cache && device->instance->pipeline_cache_enabled) {
      cache_key cache_key;
      disk_cache_compute_key(disk_cache, sha1_key, 20, cache_key);

      size_t buffer_size;
      uint8_t *buffer = disk_cache_get(disk_cache, cache_key, &buffer_size);
      if (V3D_DBG(CACHE)) {
         char sha1buf[41];
         _mesa_sha1_format(sha1buf, cache_key);
         fprintf(stderr, "[v3dv on-disk cache] %s %s\n",
                 buffer ? "hit" : "miss",
                 sha1buf);
      }

      if (buffer) {
         struct blob_reader blob;
         struct v3dv_pipeline_shared_data *shared_data;

         blob_reader_init(&blob, buffer, buffer_size);
         shared_data = v3dv_pipeline_shared_data_create_from_blob(cache, &blob);
         free(buffer);

         if (shared_data) {
            /* Technically we could increase on_disk_hit as soon as we have a
             * buffer, but we are more interested on hits that got a valid
             * shared_data
             */
            cache->stats.on_disk_hit++;
            if (cache)
               pipeline_cache_upload_shared_data(cache, shared_data, true);
            return shared_data;
         }
      }
   }
#endif

   return NULL;
}

void
v3dv_pipeline_shared_data_destroy(struct v3dv_device *device,
                                  struct v3dv_pipeline_shared_data *shared_data)
{
   assert(shared_data->ref_cnt == 0);

   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      if (shared_data->variants[stage] != NULL)
         v3dv_shader_variant_destroy(device, shared_data->variants[stage]);

      /* We don't free binning descriptor maps as we are sharing them
       * with the render shaders.
       */
      if (shared_data->maps[stage] != NULL &&
          !broadcom_shader_stage_is_binning(stage)) {
         vk_free(&device->vk.alloc, shared_data->maps[stage]);
      }
   }

   if (shared_data->assembly_bo)
      v3dv_bo_free(device, shared_data->assembly_bo);

   vk_free(&device->vk.alloc, shared_data);
}

static struct v3dv_pipeline_shared_data *
v3dv_pipeline_shared_data_new(struct v3dv_pipeline_cache *cache,
                              const unsigned char sha1_key[20],
                              struct v3dv_descriptor_maps **maps,
                              struct v3dv_shader_variant **variants,
                              const uint64_t *total_assembly,
                              const uint32_t total_assembly_size)
{
   size_t size = sizeof(struct v3dv_pipeline_shared_data);
   /* We create new_entry using the device alloc. Right now shared_data is ref
    * and unref by both the pipeline and the pipeline cache, so we can't
    * ensure that the cache or pipeline alloc will be available on the last
    * unref.
    */
   struct v3dv_pipeline_shared_data *new_entry =
      vk_zalloc2(&cache->device->vk.alloc, NULL, size, 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (new_entry == NULL)
      return NULL;

   new_entry->ref_cnt = 1;
   memcpy(new_entry->sha1_key, sha1_key, 20);

   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      new_entry->maps[stage] = maps[stage];
      new_entry->variants[stage] = variants[stage];
   }

   struct v3dv_bo *bo = v3dv_bo_alloc(cache->device, total_assembly_size,
                                      "pipeline shader assembly", true);
   if (!bo) {
      fprintf(stderr, "failed to allocate memory for shaders assembly\n");
      goto fail;
   }

   bool ok = v3dv_bo_map(cache->device, bo, total_assembly_size);
   if (!ok) {
      fprintf(stderr, "failed to map source shader buffer\n");
      goto fail;
   }

   memcpy(bo->map, total_assembly, total_assembly_size);

   new_entry->assembly_bo = bo;

   return new_entry;

fail:
   v3dv_pipeline_shared_data_unref(cache->device, new_entry);
   return NULL;
}

static void
pipeline_cache_upload_shared_data(struct v3dv_pipeline_cache *cache,
                                  struct v3dv_pipeline_shared_data *shared_data,
                                  bool from_disk_cache)
{
   assert(shared_data);

   if (!cache || !cache->cache)
      return;

   if (cache->stats.count > V3DV_MAX_PIPELINE_CACHE_ENTRIES)
      return;

   pipeline_cache_lock(cache);
   struct hash_entry *entry = NULL;

   /* If this is being called from the disk cache, we already know that the
    * entry is not on the hash table
    */
   if (!from_disk_cache)
      entry = _mesa_hash_table_search(cache->cache, shared_data->sha1_key);

   if (entry) {
      pipeline_cache_unlock(cache);
      return;
   }

   v3dv_pipeline_shared_data_ref(shared_data);
   _mesa_hash_table_insert(cache->cache, shared_data->sha1_key, shared_data);
   cache->stats.count++;
   if (debug_cache) {
      char sha1buf[41];
      _mesa_sha1_format(sha1buf, shared_data->sha1_key);

      fprintf(stderr, "pipeline cache %p, new cache entry with sha1 key %s:%p\n\n",
              cache, sha1buf, shared_data);
      if (dump_stats)
         cache_dump_stats(cache);
   }

   pipeline_cache_unlock(cache);

#ifdef ENABLE_SHADER_CACHE
   /* If we are being called from a on-disk-cache hit, we can skip writing to
    * the disk cache
    */
   if (from_disk_cache)
      return;

   struct v3dv_device *device = cache->device;
   struct disk_cache *disk_cache = device->pdevice->disk_cache;
   if (disk_cache) {
      struct blob binary;
      blob_init(&binary);
      if (v3dv_pipeline_shared_data_write_to_blob(shared_data, &binary)) {
         cache_key cache_key;
         disk_cache_compute_key(disk_cache, shared_data->sha1_key, 20, cache_key);

         if (V3D_DBG(CACHE)) {
            char sha1buf[41];
            _mesa_sha1_format(sha1buf, shared_data->sha1_key);
            fprintf(stderr, "[v3dv on-disk cache] storing %s\n", sha1buf);
         }
         disk_cache_put(disk_cache, cache_key, binary.data, binary.size, NULL);
      }

      blob_finish(&binary);
   }
#endif
}

/* Uploads all the "cacheable" or shared data from the pipeline */
void
v3dv_pipeline_cache_upload_pipeline(struct v3dv_pipeline *pipeline,
                                    struct v3dv_pipeline_cache *cache)
{
   pipeline_cache_upload_shared_data(cache, pipeline->shared_data, false);
}

static struct serialized_nir*
serialized_nir_create_from_blob(struct v3dv_pipeline_cache *cache,
                                struct blob_reader *blob)
{
   const unsigned char *sha1_key = blob_read_bytes(blob, 20);
   uint32_t snir_size = blob_read_uint32(blob);
   const char* snir_data = blob_read_bytes(blob, snir_size);
   if (blob->overrun)
      return NULL;

   struct serialized_nir *snir =
      ralloc_size(cache->nir_cache, sizeof(*snir) + snir_size);
   memcpy(snir->sha1_key, sha1_key, 20);
   snir->size = snir_size;
   memcpy(snir->data, snir_data, snir_size);

   return snir;
}

static struct v3dv_shader_variant*
shader_variant_create_from_blob(struct v3dv_device *device,
                                struct blob_reader *blob)
{
   VkResult result;

   enum broadcom_shader_stage stage = blob_read_uint32(blob);

   uint32_t prog_data_size = blob_read_uint32(blob);
   /* FIXME: as we include the stage perhaps we can avoid prog_data_size? */
   assert(prog_data_size == v3d_prog_data_size(broadcom_shader_stage_to_gl(stage)));

   const void *prog_data = blob_read_bytes(blob, prog_data_size);
   if (blob->overrun)
      return NULL;

   uint32_t ulist_count = blob_read_uint32(blob);
   uint32_t contents_size = sizeof(enum quniform_contents) * ulist_count;
   const void *contents_data = blob_read_bytes(blob, contents_size);
   if (blob->overrun)
      return NULL;

   size_t ulist_data_size = sizeof(uint32_t) * ulist_count;
   const void *ulist_data_data = blob_read_bytes(blob, ulist_data_size);
   if (blob->overrun)
      return NULL;

   uint32_t assembly_offset = blob_read_uint32(blob);
   uint32_t qpu_insts_size = blob_read_uint32(blob);

   /* shader_variant_create expects a newly created prog_data for their own,
    * as it is what the v3d compiler returns. So we are also allocating one
    * (including the uniform list) and filled it up with the data that we read
    * from the blob
    */
   struct v3d_prog_data *new_prog_data = rzalloc_size(NULL, prog_data_size);
   memcpy(new_prog_data, prog_data, prog_data_size);
   struct v3d_uniform_list *ulist = &new_prog_data->uniforms;
   ulist->count = ulist_count;
   ulist->contents = ralloc_array(new_prog_data, enum quniform_contents, ulist->count);
   memcpy(ulist->contents, contents_data, contents_size);
   ulist->data = ralloc_array(new_prog_data, uint32_t, ulist->count);
   memcpy(ulist->data, ulist_data_data, ulist_data_size);

   return v3dv_shader_variant_create(device, stage,
                                     new_prog_data, prog_data_size,
                                     assembly_offset,
                                     NULL, qpu_insts_size,
                                     &result);
}

static struct v3dv_pipeline_shared_data *
v3dv_pipeline_shared_data_create_from_blob(struct v3dv_pipeline_cache *cache,
                                           struct blob_reader *blob)
{
   const unsigned char *sha1_key = blob_read_bytes(blob, 20);

   struct v3dv_descriptor_maps *maps[BROADCOM_SHADER_STAGES] = { 0 };
   struct v3dv_shader_variant *variants[BROADCOM_SHADER_STAGES] = { 0 };

   uint8_t descriptor_maps_count = blob_read_uint8(blob);
   for (uint8_t count = 0; count < descriptor_maps_count; count++) {
      uint8_t stage = blob_read_uint8(blob);

      const struct v3dv_descriptor_maps *current_maps =
         blob_read_bytes(blob, sizeof(struct v3dv_descriptor_maps));

      if (blob->overrun)
         goto fail;

      maps[stage] = vk_zalloc2(&cache->device->vk.alloc, NULL,
                               sizeof(struct v3dv_descriptor_maps), 8,
                               VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      if (maps[stage] == NULL)
         goto fail;

      memcpy(maps[stage], current_maps, sizeof(struct v3dv_descriptor_maps));
      if (broadcom_shader_stage_is_render_with_binning(stage)) {
         enum broadcom_shader_stage bin_stage =
            broadcom_binning_shader_stage_for_render_stage(stage);
            maps[bin_stage] = maps[stage];
      }
   }

   uint8_t variant_count = blob_read_uint8(blob);

   for (uint8_t count = 0; count < variant_count; count++) {
      uint8_t stage = blob_read_uint8(blob);
      struct v3dv_shader_variant *variant =
         shader_variant_create_from_blob(cache->device, blob);
      variants[stage] = variant;
   }

   uint32_t total_assembly_size = blob_read_uint32(blob);
   const uint64_t *total_assembly =
      blob_read_bytes(blob, total_assembly_size);

   if (blob->overrun)
      goto fail;

   struct v3dv_pipeline_shared_data *data =
      v3dv_pipeline_shared_data_new(cache, sha1_key, maps, variants,
                                    total_assembly, total_assembly_size);

   if (!data)
      goto fail;

   return data;

fail:
   for (int i = 0; i < BROADCOM_SHADER_STAGES; i++) {
      if (maps[i])
         vk_free2(&cache->device->vk.alloc, NULL, maps[i]);
      if (variants[i])
         v3dv_shader_variant_destroy(cache->device, variants[i]);
   }
   return NULL;
}

static void
pipeline_cache_load(struct v3dv_pipeline_cache *cache,
                    size_t size,
                    const void *data)
{
   struct v3dv_device *device = cache->device;
   struct v3dv_physical_device *pdevice = device->pdevice;
   struct vk_pipeline_cache_header header;

   if (cache->cache == NULL || cache->nir_cache == NULL)
      return;

   struct blob_reader blob;
   blob_reader_init(&blob, data, size);

   blob_copy_bytes(&blob, &header, sizeof(header));
   if (size < sizeof(header))
      return;
   memcpy(&header, data, sizeof(header));
   if (header.header_size < sizeof(header))
      return;
   if (header.header_version != VK_PIPELINE_CACHE_HEADER_VERSION_ONE)
      return;
   if (header.vendor_id != v3dv_physical_device_vendor_id(pdevice))
      return;
   if (header.device_id != v3dv_physical_device_device_id(pdevice))
      return;
   if (memcmp(header.uuid, pdevice->pipeline_cache_uuid, VK_UUID_SIZE) != 0)
      return;

   uint32_t nir_count = blob_read_uint32(&blob);
   if (blob.overrun)
      return;

   for (uint32_t i = 0; i < nir_count; i++) {
      struct serialized_nir *snir =
         serialized_nir_create_from_blob(cache, &blob);

      if (!snir)
         break;

      _mesa_hash_table_insert(cache->nir_cache, snir->sha1_key, snir);
      cache->nir_stats.count++;
   }

   uint32_t count = blob_read_uint32(&blob);
   if (blob.overrun)
      return;

   for (uint32_t i = 0; i < count; i++) {
      struct v3dv_pipeline_shared_data *cache_entry =
         v3dv_pipeline_shared_data_create_from_blob(cache, &blob);
      if (!cache_entry)
         break;

      _mesa_hash_table_insert(cache->cache, cache_entry->sha1_key, cache_entry);
      cache->stats.count++;
   }

   if (debug_cache) {
      fprintf(stderr, "pipeline cache %p, loaded %i nir shaders and "
              "%i entries\n", cache, nir_count, count);
      if (dump_stats)
         cache_dump_stats(cache);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_CreatePipelineCache(VkDevice _device,
                         const VkPipelineCacheCreateInfo *pCreateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         VkPipelineCache *pPipelineCache)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   struct v3dv_pipeline_cache *cache;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);

   cache = vk_object_zalloc(&device->vk, pAllocator,
                            sizeof(*cache),
                            VK_OBJECT_TYPE_PIPELINE_CACHE);

   if (cache == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   v3dv_pipeline_cache_init(cache, device, pCreateInfo->flags,
                            device->instance->pipeline_cache_enabled);

   if (pCreateInfo->initialDataSize > 0) {
      pipeline_cache_load(cache,
                          pCreateInfo->initialDataSize,
                          pCreateInfo->pInitialData);
   }

   *pPipelineCache = v3dv_pipeline_cache_to_handle(cache);

   return VK_SUCCESS;
}

void
v3dv_pipeline_cache_finish(struct v3dv_pipeline_cache *cache)
{
   mtx_destroy(&cache->mutex);

   if (dump_stats_on_destroy)
      cache_dump_stats(cache);

   if (cache->nir_cache) {
      hash_table_foreach(cache->nir_cache, entry)
         ralloc_free(entry->data);

      _mesa_hash_table_destroy(cache->nir_cache, NULL);
   }

   if (cache->cache) {
      hash_table_foreach(cache->cache, entry) {
         struct v3dv_pipeline_shared_data *cache_entry = entry->data;
         if (cache_entry)
            v3dv_pipeline_shared_data_unref(cache->device, cache_entry);
      }

      _mesa_hash_table_destroy(cache->cache, NULL);
   }
}

VKAPI_ATTR void VKAPI_CALL
v3dv_DestroyPipelineCache(VkDevice _device,
                          VkPipelineCache _cache,
                          const VkAllocationCallbacks *pAllocator)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_pipeline_cache, cache, _cache);

   if (!cache)
      return;

   v3dv_pipeline_cache_finish(cache);

   vk_object_free(&device->vk, pAllocator, cache);
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_MergePipelineCaches(VkDevice device,
                         VkPipelineCache dstCache,
                         uint32_t srcCacheCount,
                         const VkPipelineCache *pSrcCaches)
{
   V3DV_FROM_HANDLE(v3dv_pipeline_cache, dst, dstCache);

   if (!dst->cache || !dst->nir_cache)
      return VK_SUCCESS;

   for (uint32_t i = 0; i < srcCacheCount; i++) {
      V3DV_FROM_HANDLE(v3dv_pipeline_cache, src, pSrcCaches[i]);
      if (!src->cache || !src->nir_cache)
         continue;

      hash_table_foreach(src->nir_cache, entry) {
         struct serialized_nir *src_snir = entry->data;
         assert(src_snir);

         if (_mesa_hash_table_search(dst->nir_cache, src_snir->sha1_key))
            continue;

         /* FIXME: we are using serialized nir shaders because they are
          * convenient to create and store on the cache, but requires to do a
          * copy here (and some other places) of the serialized NIR. Perhaps
          * it would make sense to move to handle the NIR shaders with shared
          * structures with ref counts, as the variants.
          */
         struct serialized_nir *snir_dst =
            ralloc_size(dst->nir_cache, sizeof(*snir_dst) + src_snir->size);
         memcpy(snir_dst->sha1_key, src_snir->sha1_key, 20);
         snir_dst->size = src_snir->size;
         memcpy(snir_dst->data, src_snir->data, src_snir->size);

         _mesa_hash_table_insert(dst->nir_cache, snir_dst->sha1_key, snir_dst);
         dst->nir_stats.count++;
         if (debug_cache) {
            char sha1buf[41];
            _mesa_sha1_format(sha1buf, snir_dst->sha1_key);

            fprintf(stderr, "pipeline cache %p, added nir entry %s "
                    "from pipeline cache %p\n",
                    dst, sha1buf, src);
            if (dump_stats)
               cache_dump_stats(dst);
         }
      }

      hash_table_foreach(src->cache, entry) {
         struct v3dv_pipeline_shared_data *cache_entry = entry->data;
         assert(cache_entry);

         if (_mesa_hash_table_search(dst->cache, cache_entry->sha1_key))
            continue;

         v3dv_pipeline_shared_data_ref(cache_entry);
         _mesa_hash_table_insert(dst->cache, cache_entry->sha1_key, cache_entry);

         dst->stats.count++;
         if (debug_cache) {
            char sha1buf[41];
            _mesa_sha1_format(sha1buf, cache_entry->sha1_key);

            fprintf(stderr, "pipeline cache %p, added entry %s "
                    "from pipeline cache %p\n",
                    dst, sha1buf, src);
            if (dump_stats)
               cache_dump_stats(dst);
         }
      }
   }

   return VK_SUCCESS;
}

static bool
shader_variant_write_to_blob(const struct v3dv_shader_variant *variant,
                             struct blob *blob)
{
   blob_write_uint32(blob, variant->stage);

   blob_write_uint32(blob, variant->prog_data_size);
   blob_write_bytes(blob, variant->prog_data.base, variant->prog_data_size);

   struct v3d_uniform_list *ulist = &variant->prog_data.base->uniforms;
   blob_write_uint32(blob, ulist->count);
   blob_write_bytes(blob, ulist->contents, sizeof(enum quniform_contents) * ulist->count);
   blob_write_bytes(blob, ulist->data, sizeof(uint32_t) * ulist->count);

   blob_write_uint32(blob, variant->assembly_offset);
   blob_write_uint32(blob, variant->qpu_insts_size);

   return !blob->out_of_memory;
}

static bool
v3dv_pipeline_shared_data_write_to_blob(const struct v3dv_pipeline_shared_data *cache_entry,
                                        struct blob *blob)
{
   blob_write_bytes(blob, cache_entry->sha1_key, 20);

   uint8_t descriptor_maps_count = 0;
   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      if (broadcom_shader_stage_is_binning(stage))
         continue;
      if (cache_entry->maps[stage] == NULL)
         continue;
      descriptor_maps_count++;
   }

   /* Compute pipelines only have one descriptor map,
    * graphics pipelines may have 2 (VS+FS) or 3 (VS+GS+FS), since the binning
    * stages take the descriptor map from the render stage.
    */
   assert((descriptor_maps_count >= 2 && descriptor_maps_count <= 3) ||
          (descriptor_maps_count == 1 && cache_entry->variants[BROADCOM_SHADER_COMPUTE]));
   blob_write_uint8(blob, descriptor_maps_count);

   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      if (cache_entry->maps[stage] == NULL)
         continue;
      if (broadcom_shader_stage_is_binning(stage))
         continue;

      blob_write_uint8(blob, stage);
      blob_write_bytes(blob, cache_entry->maps[stage],
                       sizeof(struct v3dv_descriptor_maps));
   }

   uint8_t variant_count = 0;
   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      if (cache_entry->variants[stage] == NULL)
         continue;
      variant_count++;
   }

   /* Graphics pipelines with VS+FS have 3 variants, VS+GS+FS will have 5 and
    * compute pipelines only have 1.
    */
   assert((variant_count == 5  || variant_count == 3) ||
          (variant_count == 1 && cache_entry->variants[BROADCOM_SHADER_COMPUTE]));
   blob_write_uint8(blob, variant_count);

   uint32_t total_assembly_size = 0;
   for (uint8_t stage = 0; stage < BROADCOM_SHADER_STAGES; stage++) {
      if (cache_entry->variants[stage] == NULL)
         continue;

      blob_write_uint8(blob, stage);
      if (!shader_variant_write_to_blob(cache_entry->variants[stage], blob))
         return false;

      total_assembly_size += cache_entry->variants[stage]->qpu_insts_size;
   }
   blob_write_uint32(blob, total_assembly_size);

   assert(cache_entry->assembly_bo->map);
   assert(cache_entry->assembly_bo->size >= total_assembly_size);
   blob_write_bytes(blob, cache_entry->assembly_bo->map, total_assembly_size);

   return !blob->out_of_memory;
}


VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetPipelineCacheData(VkDevice _device,
                          VkPipelineCache _cache,
                          size_t *pDataSize,
                          void *pData)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_pipeline_cache, cache, _cache);

   struct blob blob;
   if (pData) {
      blob_init_fixed(&blob, pData, *pDataSize);
   } else {
      blob_init_fixed(&blob, NULL, SIZE_MAX);
   }

   struct v3dv_physical_device *pdevice = device->pdevice;
   VkResult result = VK_INCOMPLETE;

   pipeline_cache_lock(cache);

   struct vk_pipeline_cache_header header = {
      .header_size = sizeof(struct vk_pipeline_cache_header),
      .header_version = VK_PIPELINE_CACHE_HEADER_VERSION_ONE,
      .vendor_id = v3dv_physical_device_vendor_id(pdevice),
      .device_id = v3dv_physical_device_device_id(pdevice),
   };
   memcpy(header.uuid, pdevice->pipeline_cache_uuid, VK_UUID_SIZE);
   blob_write_bytes(&blob, &header, sizeof(header));

   uint32_t nir_count = 0;
   intptr_t nir_count_offset = blob_reserve_uint32(&blob);
   if (nir_count_offset < 0) {
      *pDataSize = 0;
      goto done;
   }

   if (cache->nir_cache) {
      hash_table_foreach(cache->nir_cache, entry) {
         const struct serialized_nir *snir = entry->data;

         size_t save_size = blob.size;

         blob_write_bytes(&blob, snir->sha1_key, 20);
         blob_write_uint32(&blob, snir->size);
         blob_write_bytes(&blob, snir->data, snir->size);

         if (blob.out_of_memory) {
            blob.size = save_size;
            goto done;
         }

         nir_count++;
      }
   }
   blob_overwrite_uint32(&blob, nir_count_offset, nir_count);

   uint32_t count = 0;
   intptr_t count_offset = blob_reserve_uint32(&blob);
   if (count_offset < 0) {
      *pDataSize = 0;
      goto done;
   }

   if (cache->cache) {
      hash_table_foreach(cache->cache, entry) {
         struct v3dv_pipeline_shared_data *cache_entry = entry->data;

         size_t save_size = blob.size;
         if (!v3dv_pipeline_shared_data_write_to_blob(cache_entry, &blob)) {
            /* If it fails reset to the previous size and bail */
            blob.size = save_size;
            goto done;
         }

         count++;
      }
   }

   blob_overwrite_uint32(&blob, count_offset, count);

   *pDataSize = blob.size;

   result = VK_SUCCESS;

   if (debug_cache) {
      assert(count <= cache->stats.count);
      fprintf(stderr, "GetPipelineCacheData: serializing cache %p, "
              "%i nir shader entries "
              "%i entries, %u DataSize\n",
              cache, nir_count, count, (uint32_t) *pDataSize);
   }

 done:
   blob_finish(&blob);

   pipeline_cache_unlock(cache);

   return result;
}
