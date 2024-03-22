/*
 * Copyright © 2021 Corporation
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

#include "anv_private.h"
#include "genX_grl.h"

static struct anv_shader_bin *
get_shader_bin(struct anv_device *device,
               enum grl_cl_kernel kernel)
{
   const char *key = genX(grl_get_cl_kernel_sha1)(kernel);
   int key_len = strlen(key);

   bool cache_hit = false;
   struct anv_shader_bin *bin =
      anv_device_search_for_kernel(device, device->internal_cache,
                                   key, key_len, &cache_hit);
   if (bin != NULL)
      return bin;

   uint32_t dummy_param[32];
   struct brw_kernel kernel_data;
   genX(grl_get_cl_kernel)(&kernel_data, kernel);

   assert(kernel_data.prog_data.base.nr_params <= ARRAY_SIZE(dummy_param));
   kernel_data.prog_data.base.param = dummy_param;

   struct anv_push_descriptor_info push_desc_info = {};
   struct anv_pipeline_bind_map bind_map = {
      .kernel_args_size = kernel_data.args_size,
      .kernel_arg_count = kernel_data.arg_count,
      .kernel_args = (struct brw_kernel_arg_desc *)kernel_data.args,
   };

   bin = anv_device_upload_kernel(device, device->internal_cache,
                                  MESA_SHADER_KERNEL,
                                  key, key_len,
                                  kernel_data.code,
                                  kernel_data.prog_data.base.program_size,
                                  &kernel_data.prog_data.base,
                                  sizeof(kernel_data.prog_data),
                                  NULL, 0, NULL, &bind_map,
                                  &push_desc_info,
                                  0 /* dynamic_push_values */);

   /* The cache already has a reference and it's not going anywhere so there
    * is no need to hold a second reference.
    */
   anv_shader_bin_unref(device, bin);

   return bin;
}

void
genX(grl_dispatch)(struct anv_cmd_buffer *cmd_buffer,
                   enum grl_cl_kernel kernel,
                   const uint32_t *global_size,
                   uint32_t arg_count,
                   const struct anv_kernel_arg *args)
{
   struct anv_device *device = cmd_buffer->device;

   const struct intel_l3_weights w =
      intel_get_default_l3_weights(device->info, true, true);

   struct anv_kernel ak = {
      .bin = get_shader_bin(device, kernel),
      .l3_config = intel_get_l3_config(device->info, w),
   };

   genX(cmd_buffer_dispatch_kernel)(cmd_buffer, &ak, global_size,
                                    arg_count, args);
}

uint32_t
genX(grl_max_scratch_size)(void)
{
   uint32_t scratch_size = 0;

   for (uint32_t i = 0; i < GRL_CL_KERNEL_MAX; i++) {
      struct brw_kernel kernel_data;
      genX(grl_get_cl_kernel)(&kernel_data, i);

      scratch_size = MAX2(kernel_data.prog_data.base.total_scratch,
                          scratch_size);
   }

   return scratch_size;
}
