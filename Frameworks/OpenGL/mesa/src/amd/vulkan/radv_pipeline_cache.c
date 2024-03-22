/*
 * Copyright © 2015 Intel Corporation
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

#include "util/disk_cache.h"
#include "util/macros.h"
#include "util/mesa-blake3.h"
#include "util/mesa-sha1.h"
#include "util/u_atomic.h"
#include "util/u_debug.h"
#include "vulkan/util/vk_util.h"
#include "aco_interface.h"
#include "nir_serialize.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "vk_pipeline.h"

static bool
radv_is_cache_disabled(struct radv_device *device)
{
   /* Pipeline caches can be disabled with RADV_DEBUG=nocache, with MESA_GLSL_CACHE_DISABLE=1 and
    * when ACO_DEBUG is used. MESA_GLSL_CACHE_DISABLE is done elsewhere.
    */
   return (device->instance->debug_flags & RADV_DEBUG_NO_CACHE) ||
          (device->physical_device->use_llvm ? 0 : aco_get_codegen_flags());
}

void
radv_hash_shaders(const struct radv_device *device, unsigned char *hash, const struct radv_shader_stage *stages,
                  uint32_t stage_count, const struct radv_pipeline_layout *layout, const struct radv_pipeline_key *key)
{
   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);
   _mesa_sha1_update(&ctx, device->cache_hash, sizeof(device->cache_hash));
   if (key)
      _mesa_sha1_update(&ctx, key, sizeof(*key));
   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   for (unsigned s = 0; s < stage_count; s++) {
      if (!stages[s].entrypoint)
         continue;

      _mesa_sha1_update(&ctx, stages[s].shader_sha1, sizeof(stages[s].shader_sha1));
   }
   _mesa_sha1_final(&ctx, hash);
}

void
radv_hash_rt_stages(struct mesa_sha1 *ctx, const VkPipelineShaderStageCreateInfo *stages, unsigned stage_count)
{
   for (unsigned i = 0; i < stage_count; ++i) {
      unsigned char hash[20];
      vk_pipeline_hash_shader_stage(&stages[i], NULL, hash);
      _mesa_sha1_update(ctx, hash, sizeof(hash));
   }
}

void
radv_hash_rt_shaders(const struct radv_device *device, unsigned char *hash,
                     const VkRayTracingPipelineCreateInfoKHR *pCreateInfo, const struct radv_pipeline_key *key,
                     const struct radv_ray_tracing_group *groups)
{
   RADV_FROM_HANDLE(radv_pipeline_layout, layout, pCreateInfo->layout);
   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);
   _mesa_sha1_update(&ctx, device->cache_hash, sizeof(device->cache_hash));
   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   _mesa_sha1_update(&ctx, key, sizeof(*key));

   radv_hash_rt_stages(&ctx, pCreateInfo->pStages, pCreateInfo->stageCount);

   for (uint32_t i = 0; i < pCreateInfo->groupCount; i++) {
      _mesa_sha1_update(&ctx, &pCreateInfo->pGroups[i].type, sizeof(pCreateInfo->pGroups[i].type));
      _mesa_sha1_update(&ctx, &pCreateInfo->pGroups[i].generalShader, sizeof(pCreateInfo->pGroups[i].generalShader));
      _mesa_sha1_update(&ctx, &pCreateInfo->pGroups[i].anyHitShader, sizeof(pCreateInfo->pGroups[i].anyHitShader));
      _mesa_sha1_update(&ctx, &pCreateInfo->pGroups[i].closestHitShader,
                        sizeof(pCreateInfo->pGroups[i].closestHitShader));
      _mesa_sha1_update(&ctx, &pCreateInfo->pGroups[i].intersectionShader,
                        sizeof(pCreateInfo->pGroups[i].intersectionShader));
      _mesa_sha1_update(&ctx, &groups[i].handle, sizeof(struct radv_pipeline_group_handle));
   }

   if (pCreateInfo->pLibraryInfo) {
      for (uint32_t i = 0; i < pCreateInfo->pLibraryInfo->libraryCount; ++i) {
         RADV_FROM_HANDLE(radv_pipeline, lib_pipeline, pCreateInfo->pLibraryInfo->pLibraries[i]);
         struct radv_ray_tracing_pipeline *lib = radv_pipeline_to_ray_tracing(lib_pipeline);
         _mesa_sha1_update(&ctx, lib->sha1, SHA1_DIGEST_LENGTH);
      }
   }

   const uint64_t pipeline_flags =
      vk_rt_pipeline_create_flags(pCreateInfo) &
      (VK_PIPELINE_CREATE_2_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_2_RAY_TRACING_SKIP_AABBS_BIT_KHR |
       VK_PIPELINE_CREATE_2_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
       VK_PIPELINE_CREATE_2_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
       VK_PIPELINE_CREATE_2_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
       VK_PIPELINE_CREATE_2_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR | VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR);
   _mesa_sha1_update(&ctx, &pipeline_flags, sizeof(pipeline_flags));

   _mesa_sha1_final(&ctx, hash);
}

static void
radv_shader_destroy(struct vk_device *_device, struct vk_pipeline_cache_object *object)
{
   struct radv_device *device = container_of(_device, struct radv_device, vk);
   struct radv_shader *shader = container_of(object, struct radv_shader, base);

   if (device->shader_use_invisible_vram) {
      /* Wait for any pending upload to complete, or we'll be writing into freed shader memory. */
      radv_shader_wait_for_upload(device, shader->upload_seq);
   }

   radv_free_shader_memory(device, shader->alloc);

   free(shader->code);
   free(shader->spirv);
   free(shader->nir_string);
   free(shader->disasm_string);
   free(shader->ir_string);
   free(shader->statistics);

   vk_pipeline_cache_object_finish(&shader->base);
   free(shader);
}

static struct vk_pipeline_cache_object *
radv_shader_deserialize(struct vk_pipeline_cache *cache, const void *key_data, size_t key_size,
                        struct blob_reader *blob)
{
   struct radv_device *device = container_of(cache->base.device, struct radv_device, vk);
   const struct radv_shader_binary *binary = blob_read_bytes(blob, sizeof(struct radv_shader_binary));

   struct radv_shader *shader;
   radv_shader_create_uncached(device, binary, false, NULL, &shader);
   if (!shader)
      return NULL;

   assert(key_size == sizeof(shader->hash));
   memcpy(shader->hash, key_data, key_size);
   blob_skip_bytes(blob, binary->total_size - sizeof(struct radv_shader_binary));

   return &shader->base;
}

static bool
radv_shader_serialize(struct vk_pipeline_cache_object *object, struct blob *blob)
{
   struct radv_shader *shader = container_of(object, struct radv_shader, base);
   size_t stats_size = shader->statistics ? aco_num_statistics * sizeof(uint32_t) : 0;
   size_t code_size = shader->code_size;
   uint32_t total_size = sizeof(struct radv_shader_binary_legacy) + code_size + stats_size;

   struct radv_shader_binary_legacy binary = {
      .base =
         {
            .type = RADV_BINARY_TYPE_LEGACY,
            .config = shader->config,
            .info = shader->info,
            .total_size = total_size,
         },
      .code_size = code_size,
      .exec_size = shader->exec_size,
      .ir_size = 0,
      .disasm_size = 0,
      .stats_size = stats_size,
   };

   blob_write_bytes(blob, &binary, sizeof(struct radv_shader_binary_legacy));
   blob_write_bytes(blob, shader->statistics, stats_size);
   blob_write_bytes(blob, shader->code, code_size);

   return true;
}

struct radv_shader *
radv_shader_create(struct radv_device *device, struct vk_pipeline_cache *cache, const struct radv_shader_binary *binary,
                   bool skip_cache)
{
   if (radv_is_cache_disabled(device) || skip_cache) {
      struct radv_shader *shader;
      radv_shader_create_uncached(device, binary, false, NULL, &shader);
      return shader;
   }

   if (!cache)
      cache = device->mem_cache;

   blake3_hash hash;
   _mesa_blake3_compute(binary, binary->total_size, hash);

   struct vk_pipeline_cache_object *shader_obj;
   shader_obj = vk_pipeline_cache_create_and_insert_object(cache, hash, sizeof(hash), binary, binary->total_size,
                                                           &radv_shader_ops);

   return shader_obj ? container_of(shader_obj, struct radv_shader, base) : NULL;
}

const struct vk_pipeline_cache_object_ops radv_shader_ops = {
   .serialize = radv_shader_serialize,
   .deserialize = radv_shader_deserialize,
   .destroy = radv_shader_destroy,
};

struct radv_pipeline_cache_object {
   struct vk_pipeline_cache_object base;
   unsigned num_shaders;
   uint32_t data_size;
   void *data; /* Generic data stored alongside the shaders */
   uint8_t sha1[SHA1_DIGEST_LENGTH];
   struct radv_shader *shaders[];
};

const struct vk_pipeline_cache_object_ops radv_pipeline_ops;

static struct radv_pipeline_cache_object *
radv_pipeline_cache_object_create(struct vk_device *device, unsigned num_shaders, const void *hash, unsigned data_size)
{
   const size_t size =
      sizeof(struct radv_pipeline_cache_object) + (num_shaders * sizeof(struct radv_shader *)) + data_size;

   struct radv_pipeline_cache_object *object = vk_alloc(&device->alloc, size, 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
   if (!object)
      return NULL;

   vk_pipeline_cache_object_init(device, &object->base, &radv_pipeline_ops, object->sha1, SHA1_DIGEST_LENGTH);
   object->num_shaders = num_shaders;
   object->data = &object->shaders[num_shaders];
   object->data_size = data_size;
   memcpy(object->sha1, hash, SHA1_DIGEST_LENGTH);
   memset(object->shaders, 0, sizeof(object->shaders[0]) * num_shaders);

   return object;
}

static void
radv_pipeline_cache_object_destroy(struct vk_device *_device, struct vk_pipeline_cache_object *object)
{
   struct radv_device *device = container_of(_device, struct radv_device, vk);
   struct radv_pipeline_cache_object *pipeline_obj = container_of(object, struct radv_pipeline_cache_object, base);

   for (unsigned i = 0; i < pipeline_obj->num_shaders; i++) {
      if (pipeline_obj->shaders[i])
         radv_shader_unref(device, pipeline_obj->shaders[i]);
   }

   vk_pipeline_cache_object_finish(&pipeline_obj->base);
   vk_free(&_device->alloc, pipeline_obj);
}

static struct vk_pipeline_cache_object *
radv_pipeline_cache_object_deserialize(struct vk_pipeline_cache *cache, const void *key_data, size_t key_size,
                                       struct blob_reader *blob)
{
   struct radv_device *device = container_of(cache->base.device, struct radv_device, vk);
   assert(key_size == SHA1_DIGEST_LENGTH);
   unsigned total_size = blob->end - blob->current;
   unsigned num_shaders = blob_read_uint32(blob);
   unsigned data_size = blob_read_uint32(blob);

   struct radv_pipeline_cache_object *object;
   object = radv_pipeline_cache_object_create(&device->vk, num_shaders, key_data, data_size);
   if (!object)
      return NULL;

   object->base.data_size = total_size;

   for (unsigned i = 0; i < num_shaders; i++) {
      const uint8_t *hash = blob_read_bytes(blob, sizeof(blake3_hash));
      struct vk_pipeline_cache_object *shader =
         vk_pipeline_cache_lookup_object(cache, hash, sizeof(blake3_hash), &radv_shader_ops, NULL);

      if (!shader) {
         /* If some shader could not be created from cache, better return NULL here than having
          * an incomplete cache object which needs to be fixed up later.
          */
         vk_pipeline_cache_object_unref(&device->vk, &object->base);
         return NULL;
      }

      object->shaders[i] = container_of(shader, struct radv_shader, base);
   }

   blob_copy_bytes(blob, object->data, data_size);

   return &object->base;
}

static bool
radv_pipeline_cache_object_serialize(struct vk_pipeline_cache_object *object, struct blob *blob)
{
   struct radv_pipeline_cache_object *pipeline_obj = container_of(object, struct radv_pipeline_cache_object, base);

   blob_write_uint32(blob, pipeline_obj->num_shaders);
   blob_write_uint32(blob, pipeline_obj->data_size);

   for (unsigned i = 0; i < pipeline_obj->num_shaders; i++)
      blob_write_bytes(blob, pipeline_obj->shaders[i]->hash, sizeof(pipeline_obj->shaders[i]->hash));

   blob_write_bytes(blob, pipeline_obj->data, pipeline_obj->data_size);

   return true;
}

const struct vk_pipeline_cache_object_ops radv_pipeline_ops = {
   .serialize = radv_pipeline_cache_object_serialize,
   .deserialize = radv_pipeline_cache_object_deserialize,
   .destroy = radv_pipeline_cache_object_destroy,
};

bool
radv_pipeline_cache_search(struct radv_device *device, struct vk_pipeline_cache *cache, struct radv_pipeline *pipeline,
                           const unsigned char *sha1, bool *found_in_application_cache)
{
   *found_in_application_cache = false;

   if (radv_is_cache_disabled(device))
      return false;

   bool *found = found_in_application_cache;
   if (!cache) {
      cache = device->mem_cache;
      found = NULL;
   }

   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, sha1, SHA1_DIGEST_LENGTH, &radv_pipeline_ops, found);

   if (!object)
      return false;

   struct radv_pipeline_cache_object *pipeline_obj = container_of(object, struct radv_pipeline_cache_object, base);

   for (unsigned i = 0; i < pipeline_obj->num_shaders; i++) {
      gl_shader_stage s = pipeline_obj->shaders[i]->info.stage;
      if (s == MESA_SHADER_VERTEX && i > 0) {
         /* The GS copy-shader is a VS placed after all other stages */
         assert(i == pipeline_obj->num_shaders - 1 && pipeline->shaders[MESA_SHADER_GEOMETRY]);
         pipeline->gs_copy_shader = radv_shader_ref(pipeline_obj->shaders[i]);
      } else {
         pipeline->shaders[s] = radv_shader_ref(pipeline_obj->shaders[i]);
      }
   }

   pipeline->cache_object = object;
   return true;
}

void
radv_pipeline_cache_insert(struct radv_device *device, struct vk_pipeline_cache *cache, struct radv_pipeline *pipeline,
                           const unsigned char *sha1)
{
   if (radv_is_cache_disabled(device))
      return;

   if (!cache)
      cache = device->mem_cache;

   /* Count shaders */
   unsigned num_shaders = 0;
   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i)
      num_shaders += pipeline->shaders[i] ? 1 : 0;
   num_shaders += pipeline->gs_copy_shader ? 1 : 0;

   struct radv_pipeline_cache_object *pipeline_obj;
   pipeline_obj = radv_pipeline_cache_object_create(&device->vk, num_shaders, sha1, 0);

   if (!pipeline_obj)
      return;

   unsigned idx = 0;
   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      if (pipeline->shaders[i])
         pipeline_obj->shaders[idx++] = radv_shader_ref(pipeline->shaders[i]);
   }
   /* Place the GS copy-shader after all other stages */
   if (pipeline->gs_copy_shader)
      pipeline_obj->shaders[idx++] = radv_shader_ref(pipeline->gs_copy_shader);

   assert(idx == num_shaders);

   /* Add the object to the cache */
   pipeline->cache_object = vk_pipeline_cache_add_object(cache, &pipeline_obj->base);
}

struct radv_ray_tracing_stage_cache_data {
   uint32_t stack_size : 31;
   uint32_t has_shader : 1;
};

struct radv_ray_tracing_pipeline_cache_data {
   uint32_t has_traversal_shader : 1;
   struct radv_ray_tracing_stage_cache_data stages[];
};

bool
radv_ray_tracing_pipeline_cache_search(struct radv_device *device, struct vk_pipeline_cache *cache,
                                       struct radv_ray_tracing_pipeline *pipeline,
                                       const VkRayTracingPipelineCreateInfoKHR *pCreateInfo)
{
   if (radv_is_cache_disabled(device))
      return false;

   if (!cache)
      cache = device->mem_cache;

   bool cache_hit = false;
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, pipeline->sha1, SHA1_DIGEST_LENGTH, &radv_pipeline_ops, &cache_hit);

   if (!object)
      return false;

   struct radv_pipeline_cache_object *pipeline_obj = container_of(object, struct radv_pipeline_cache_object, base);
   struct radv_ray_tracing_pipeline_cache_data *data = pipeline_obj->data;

   bool is_library = pipeline->base.base.create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR;
   bool complete = true;
   unsigned idx = 0;

   if (data->has_traversal_shader)
      pipeline->base.base.shaders[MESA_SHADER_INTERSECTION] = radv_shader_ref(pipeline_obj->shaders[idx++]);

   for (unsigned i = 0; i < pCreateInfo->stageCount; i++) {
      pipeline->stages[i].stack_size = data->stages[i].stack_size;

      if (data->stages[i].has_shader)
         pipeline->stages[i].shader = radv_shader_ref(pipeline_obj->shaders[idx++]);

      if (is_library) {
         pipeline->stages[i].nir = radv_pipeline_cache_search_nir(device, cache, pipeline->stages[i].sha1);
         complete &= pipeline->stages[i].nir != NULL;
      }
   }

   assert(idx == pipeline_obj->num_shaders);

   if (cache_hit && cache != device->mem_cache) {
      const VkPipelineCreationFeedbackCreateInfo *creation_feedback =
         vk_find_struct_const(pCreateInfo->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);
      if (creation_feedback)
         creation_feedback->pPipelineCreationFeedback->flags |=
            VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
   }

   pipeline->base.base.cache_object = object;
   return complete;
}

void
radv_ray_tracing_pipeline_cache_insert(struct radv_device *device, struct vk_pipeline_cache *cache,
                                       struct radv_ray_tracing_pipeline *pipeline, unsigned num_stages,
                                       const unsigned char *sha1)
{
   if (radv_is_cache_disabled(device))
      return;

   if (!cache)
      cache = device->mem_cache;

   /* Skip insertion on cache hit.
    * This branch can be triggered if a cache_object was found but not all NIR shaders could be
    * looked up. The cache_object is already complete in that case.
    */
   if (pipeline->base.base.cache_object)
      return;

   /* Count compiled shaders excl. library shaders */
   unsigned num_shaders = pipeline->base.base.shaders[MESA_SHADER_INTERSECTION] ? 1 : 0;
   for (unsigned i = 0; i < num_stages; ++i)
      num_shaders += pipeline->stages[i].shader ? 1 : 0;

   uint32_t data_size = sizeof(struct radv_ray_tracing_pipeline_cache_data) +
                        num_stages * sizeof(struct radv_ray_tracing_stage_cache_data);

   struct radv_pipeline_cache_object *pipeline_obj =
      radv_pipeline_cache_object_create(&device->vk, num_shaders, sha1, data_size);
   struct radv_ray_tracing_pipeline_cache_data *data = pipeline_obj->data;

   data->has_traversal_shader = !!pipeline->base.base.shaders[MESA_SHADER_INTERSECTION];

   unsigned idx = 0;
   if (data->has_traversal_shader)
      pipeline_obj->shaders[idx++] = radv_shader_ref(pipeline->base.base.shaders[MESA_SHADER_INTERSECTION]);

   for (unsigned i = 0; i < num_stages; ++i) {
      data->stages[i].stack_size = pipeline->stages[i].stack_size;
      data->stages[i].has_shader = !!pipeline->stages[i].shader;

      if (pipeline->stages[i].shader)
         pipeline_obj->shaders[idx++] = radv_shader_ref(pipeline->stages[i].shader);
   }
   assert(idx == num_shaders);

   /* Add the object to the cache */
   pipeline->base.base.cache_object = vk_pipeline_cache_add_object(cache, &pipeline_obj->base);
}

struct vk_pipeline_cache_object *
radv_pipeline_cache_search_nir(struct radv_device *device, struct vk_pipeline_cache *cache, const uint8_t *sha1)
{
   if (radv_is_cache_disabled(device))
      return NULL;

   if (!cache)
      cache = device->mem_cache;

   return vk_pipeline_cache_lookup_object(cache, sha1, SHA1_DIGEST_LENGTH, &vk_raw_data_cache_object_ops, NULL);
}

struct nir_shader *
radv_pipeline_cache_handle_to_nir(struct radv_device *device, struct vk_pipeline_cache_object *object)
{
   struct blob_reader blob;
   struct vk_raw_data_cache_object *nir_object = container_of(object, struct vk_raw_data_cache_object, base);
   blob_reader_init(&blob, nir_object->data, nir_object->data_size);
   nir_shader *nir = nir_deserialize(NULL, NULL, &blob);

   if (blob.overrun) {
      ralloc_free(nir);
      return NULL;
   }
   nir->options = &device->physical_device->nir_options[nir->info.stage];

   return nir;
}

struct vk_pipeline_cache_object *
radv_pipeline_cache_nir_to_handle(struct radv_device *device, struct vk_pipeline_cache *cache, struct nir_shader *nir,
                                  const uint8_t *sha1, bool cached)
{
   if (!cache)
      cache = device->mem_cache;

   struct blob blob;
   blob_init(&blob);
   nir_serialize(&blob, nir, true);

   if (blob.out_of_memory) {
      blob_finish(&blob);
      return NULL;
   }

   void *data;
   size_t size;
   blob_finish_get_buffer(&blob, &data, &size);
   struct vk_pipeline_cache_object *object;

   if (cached && !radv_is_cache_disabled(device)) {
      object = vk_pipeline_cache_create_and_insert_object(cache, sha1, SHA1_DIGEST_LENGTH, data, size,
                                                          &vk_raw_data_cache_object_ops);
   } else {
      struct vk_raw_data_cache_object *nir_object =
         vk_raw_data_cache_object_create(&device->vk, sha1, SHA1_DIGEST_LENGTH, data, size);
      object = nir_object ? &nir_object->base : NULL;
   }

   free(data);
   return object;
}
