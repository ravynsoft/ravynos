/*
 * Copyright © 2022 Friedrich Vock
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

#include "vk_rmv_common.h"
#include "vulkan/runtime/vk_buffer.h"
#include "vulkan/runtime/vk_device.h"

void
vk_memory_trace_init(struct vk_device *device, const struct vk_rmv_device_info *device_info)
{
   device->memory_trace_data.device_info = *device_info;
   device->memory_trace_data.is_enabled = true;
   util_dynarray_init(&device->memory_trace_data.tokens, NULL);
   simple_mtx_init(&device->memory_trace_data.token_mtx, mtx_plain);

   device->memory_trace_data.next_resource_id = 1;
   device->memory_trace_data.handle_table = _mesa_hash_table_u64_create(NULL);
}

void
vk_memory_trace_finish(struct vk_device *device)
{
   if (!device->memory_trace_data.is_enabled)
      return;
   util_dynarray_foreach (&device->memory_trace_data.tokens, struct vk_rmv_token, token) {
      switch (token->type) {
      case VK_RMV_TOKEN_TYPE_RESOURCE_CREATE: {
         struct vk_rmv_resource_create_token *create_token = &token->data.resource_create;
         if (create_token->type == VK_RMV_RESOURCE_TYPE_DESCRIPTOR_POOL) {
            free(create_token->descriptor_pool.pool_sizes);
         }
         break;
      }
      case VK_RMV_TOKEN_TYPE_USERDATA:
         free(token->data.userdata.name);
         break;
      default:
         break;
      }
   }
   util_dynarray_fini(&device->memory_trace_data.tokens);
   if (_mesa_hash_table_num_entries(device->memory_trace_data.handle_table->table))
      fprintf(stderr,
              "mesa: Unfreed resources detected at device destroy, there may be memory leaks!\n");
   _mesa_hash_table_u64_destroy(device->memory_trace_data.handle_table);
   device->memory_trace_data.is_enabled = false;
}

void
vk_rmv_emit_token(struct vk_memory_trace_data *data, enum vk_rmv_token_type type, void *token_data)
{
   struct vk_rmv_token token;
   token.type = type;
   token.timestamp = (uint64_t)os_time_get_nano();
   memcpy(&token.data, token_data, vk_rmv_token_size_from_type(type));
   util_dynarray_append(&data->tokens, struct vk_rmv_token, token);
}

uint32_t
vk_rmv_get_resource_id_locked(struct vk_device *device, uint64_t handle)
{
   void *entry = _mesa_hash_table_u64_search(device->memory_trace_data.handle_table, handle);
   if (!entry) {
      uint32_t id = device->memory_trace_data.next_resource_id++;
      _mesa_hash_table_u64_insert(device->memory_trace_data.handle_table, handle,
                                  (void *)(uintptr_t)id);
      return id;
   }
   return (uint32_t)(uintptr_t)entry;
}

void
vk_rmv_destroy_resource_id_locked(struct vk_device *device, uint64_t handle)
{
   _mesa_hash_table_u64_remove(device->memory_trace_data.handle_table, handle);
}

void
vk_rmv_log_buffer_create(struct vk_device *device, bool is_internal, VkBuffer _buffer)
{
   if (!device->memory_trace_data.is_enabled)
      return;

   VK_FROM_HANDLE(vk_buffer, buffer, _buffer);
   simple_mtx_lock(&device->memory_trace_data.token_mtx);
   struct vk_rmv_resource_create_token token = {0};
   token.is_driver_internal = is_internal;
   token.resource_id = vk_rmv_get_resource_id_locked(device, (uint64_t)_buffer);
   token.type = VK_RMV_RESOURCE_TYPE_BUFFER;
   token.buffer.create_flags = buffer->create_flags;
   token.buffer.size = buffer->size;
   token.buffer.usage_flags = buffer->usage;

   vk_rmv_emit_token(&device->memory_trace_data, VK_RMV_TOKEN_TYPE_RESOURCE_CREATE, &token);
   simple_mtx_unlock(&device->memory_trace_data.token_mtx);
}

void
vk_rmv_log_cpu_map(struct vk_device *device, uint64_t va, bool is_unmap)
{
   if (!device->memory_trace_data.is_enabled)
      return;

   struct vk_rmv_cpu_map_token map_token;
   map_token.address = va;
   map_token.unmapped = is_unmap;

   simple_mtx_lock(&device->memory_trace_data.token_mtx);
   vk_rmv_emit_token(&device->memory_trace_data, VK_RMV_TOKEN_TYPE_CPU_MAP, &map_token);
   simple_mtx_unlock(&device->memory_trace_data.token_mtx);
}

void
vk_rmv_log_misc_token(struct vk_device *device, enum vk_rmv_misc_event_type type)
{
   if (!device->memory_trace_data.is_enabled)
      return;

   simple_mtx_lock(&device->memory_trace_data.token_mtx);
   struct vk_rmv_misc_token token;
   token.type = type;
   vk_rmv_emit_token(&device->memory_trace_data, VK_RMV_TOKEN_TYPE_MISC, &token);
   simple_mtx_unlock(&device->memory_trace_data.token_mtx);
}
