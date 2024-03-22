/*
 * Copyright © 2022 Intel Corporation
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
 *
 * Capture the hanging application with INTEL_DEBUG=capture-all
 *
 * Turn the error state into a replay file with :
 *    $ intel_error2hangdump error_state
 *
 * Replay with :
 *    $ intel_hang_replay -d error_state.dmp
 */

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <xf86drm.h>

#include "common/intel_disasm.h"
#include "common/intel_gem.h"
#include "common/intel_hang_dump.h"
#include "dev/intel_device_info.h"

#include "drm-uapi/i915_drm.h"

#include "util/u_dynarray.h"
#include "util/u_math.h"

static uint32_t
gem_create(int drm_fd, uint64_t size)
{
   struct drm_i915_gem_create gem_create = {
      .size = size,
   };

   int ret = intel_ioctl(drm_fd, DRM_IOCTL_I915_GEM_CREATE, &gem_create);
   if (ret != 0) {
      /* FIXME: What do we do if this fails? */
      return 0;
   }

   return gem_create.handle;
}

static void*
gem_mmap_offset(int drm_fd,
                uint32_t gem_handle,
                uint64_t offset,
                uint64_t size,
                uint32_t flags)
{
   struct drm_i915_gem_mmap_offset gem_mmap = {
      .handle = gem_handle,
      .flags = I915_MMAP_OFFSET_WB,
   };
   assert(offset == 0);

   /* Get the fake offset back */
   int ret = intel_ioctl(drm_fd, DRM_IOCTL_I915_GEM_MMAP_OFFSET, &gem_mmap);
   if (ret != 0 && gem_mmap.flags == I915_MMAP_OFFSET_FIXED) {
      gem_mmap.flags =
         (flags & I915_MMAP_WC) ? I915_MMAP_OFFSET_WC : I915_MMAP_OFFSET_WB,
      ret = intel_ioctl(drm_fd, DRM_IOCTL_I915_GEM_MMAP_OFFSET, &gem_mmap);
   }

   if (ret != 0)
      return MAP_FAILED;

   /* And map it */
   void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    drm_fd, gem_mmap.offset);
   return map;
}

static void
write_gem_bo_data(int drm_fd,
                  uint32_t gem_handle,
                  int file_fd,
                  size_t size)
{
   void *map = gem_mmap_offset(drm_fd, gem_handle, 0, size, I915_MMAP_OFFSET_WB);
   assert(map != MAP_FAILED);

   size_t total_read_len = 0;
   ssize_t read_len;
   while (total_read_len < size &&
          (read_len = read(file_fd, map + total_read_len, size - total_read_len)) > 0) {
      total_read_len += read_len;
   }
   munmap(map, size);

   assert(total_read_len == size);
}

static void
skip_data(int file_fd, size_t size)
{
   lseek(file_fd, size, SEEK_CUR);
}

static int
get_drm_device(struct intel_device_info *devinfo)
{
   drmDevicePtr devices[8];
   int max_devices = drmGetDevices2(0, devices, 8);

   int i, fd = -1;
   for (i = 0; i < max_devices; i++) {
      if (devices[i]->available_nodes & 1 << DRM_NODE_RENDER &&
          devices[i]->bustype == DRM_BUS_PCI &&
          devices[i]->deviceinfo.pci->vendor_id == 0x8086) {
         fd = open(devices[i]->nodes[DRM_NODE_RENDER], O_RDWR | O_CLOEXEC);
         if (fd < 0)
            continue;

         if (!intel_get_device_info_from_fd(fd, devinfo) ||
             devinfo->ver < 8) {
            close(fd);
            fd = -1;
            continue;
         }

         /* Found a device! */
         break;
      }
   }

   return fd;
}

struct gem_bo {
   off_t    file_offset;
   uint32_t gem_handle;
   uint64_t offset;
   uint64_t size;
   bool     hw_img;
};

static int
compare_bos(const void *b1, const void *b2)
{
   const struct gem_bo *gem_b1 = b1, *gem_b2 = b2;

   return gem_b2->size > gem_b1->size;
}

static void
print_help(const char *filename, FILE *f)
{
   fprintf(f, "%s: %s [options]...\n", filename, filename);
   fprintf(f, "    -d, --dump FILE    hang file to replay\n");
   fprintf(f, "    -l, --list         list content of hang file (no replay)\n");
   fprintf(f, "    -s, --shader ADDR  print shader at ADDR\n");
   fprintf(f, "    -h, --help         print this screen\n");
   fprintf(f, "    -a, --address ADDR Find BO containing ADDR\n");
}

static int
execbuffer(int drm_fd, struct util_dynarray *execbuffer_bos, struct gem_bo *exec_bo, uint64_t exec_offset)
{
   struct drm_i915_gem_execbuffer2 execbuf = {
      .buffers_ptr        = (uintptr_t)(void *)util_dynarray_begin(execbuffer_bos),
      .buffer_count       = util_dynarray_num_elements(execbuffer_bos,
                                                       struct drm_i915_gem_exec_object2),
      .batch_start_offset = exec_offset - exec_bo->offset,
      .batch_len          = exec_bo->size,
      .flags              = I915_EXEC_HANDLE_LUT | I915_EXEC_RENDER,
      .rsvd1              = 0,
   };

   int ret = intel_ioctl(drm_fd, DRM_IOCTL_I915_GEM_EXECBUFFER2_WR, &execbuf);
   if (ret == 0) {
      struct drm_i915_gem_wait gem_wait = {
         .bo_handle  = exec_bo->gem_handle,
         .timeout_ns = INT64_MAX,
      };
      ret = intel_ioctl(drm_fd, DRM_IOCTL_I915_GEM_WAIT, &gem_wait);
      if (ret)
         fprintf(stderr, "wait failed: %m\n");
   } else {
      fprintf(stderr, "execbuffer failed: %m\n");
   }

   return ret;
}

int
main(int argc, char *argv[])
{
   bool help = false, list = false;
   const struct option aubinator_opts[] = {
      { "address",    required_argument, NULL, 'a' },
      { "dump",       required_argument, NULL, 'd' },
      { "shader",     required_argument, NULL, 's' },
      { "list",       no_argument,       NULL, 'l' },
      { "help",       no_argument,       NULL, 'h' },
      { NULL,         0,                 NULL,   0 },
   };

   void *mem_ctx = ralloc_context(NULL);

   struct util_dynarray shader_addresses;

   util_dynarray_init(&shader_addresses, mem_ctx);

   const char *file = NULL;
   uint64_t check_addr = -1;
   int c, i;
   while ((c = getopt_long(argc, argv, "a:d:hls:", aubinator_opts, &i)) != -1) {
      switch (c) {
      case 'a':
         check_addr = strtol(optarg, NULL, 0);
         break;
      case 'd':
         file = optarg;
         break;
      case 's': {
         uint64_t *addr = util_dynarray_grow(&shader_addresses, uint64_t, 1);
         *addr = strtol(optarg, NULL, 0);
         fprintf(stderr, "shader addr=0x%016"PRIx64"\n", *addr);
         break;
      }
      case 'h':
         help = true;
         break;
      case 'l':
         list = true;
         break;
      default:
         break;
      }
   }

   if (help) {
      print_help(argv[0], stderr);
      exit(EXIT_SUCCESS);
   }

   int file_fd = open(file, O_RDONLY);
   if (file_fd < 0)
      exit(EXIT_FAILURE);

   struct stat file_stats;
   if (fstat(file_fd, &file_stats) != 0)
      exit(EXIT_FAILURE);

   struct intel_device_info devinfo;
   int drm_fd = get_drm_device(&devinfo);
   if (drm_fd < 0)
      exit(EXIT_FAILURE);

   struct util_dynarray buffers;
   uint64_t total_vma = 0;

   util_dynarray_init(&buffers, mem_ctx);

   union intel_hang_dump_block_all block_header;
   struct intel_hang_dump_block_exec init = {
      .offset = -1,
   }, exec = {
      .offset = -1,
   };

   while (read(file_fd, &block_header.base, sizeof(block_header.base)) ==
          sizeof(block_header.base)) {

      static const size_t block_size[] = {
         [INTEL_HANG_DUMP_BLOCK_TYPE_HEADER]   = sizeof(struct intel_hang_dump_block_header),
         [INTEL_HANG_DUMP_BLOCK_TYPE_BO]       = sizeof(struct intel_hang_dump_block_bo),
         [INTEL_HANG_DUMP_BLOCK_TYPE_MAP]      = sizeof(struct intel_hang_dump_block_map),
         [INTEL_HANG_DUMP_BLOCK_TYPE_EXEC]     = sizeof(struct intel_hang_dump_block_exec),
         [INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE] = sizeof(struct intel_hang_dump_block_hw_image),
      };

      assert(block_header.base.type < ARRAY_SIZE(block_size));

      size_t remaining_size = block_size[block_header.base.type] - sizeof(block_header.base);
      ssize_t ret = read(file_fd, &block_header.base + 1, remaining_size);
      bool has_hw_image = false;
      assert(ret == remaining_size);

      switch (block_header.base.type) {
      case INTEL_HANG_DUMP_BLOCK_TYPE_HEADER:
         assert(block_header.header.magic == INTEL_HANG_DUMP_MAGIC);
         assert(block_header.header.version == INTEL_HANG_DUMP_VERSION);
         break;

      case INTEL_HANG_DUMP_BLOCK_TYPE_BO: {
         struct gem_bo *bo = util_dynarray_grow(&buffers, struct gem_bo, 1);
         *bo = (struct gem_bo) {
            .file_offset = lseek(file_fd, 0, SEEK_CUR),
            .offset = block_header.bo.offset,
            .size = block_header.bo.size,
         };
         total_vma += bo->size;
         skip_data(file_fd, bo->size);
         if (list) {
            fprintf(stderr, "buffer: offset=0x%016"PRIx64" size=0x%016"PRIx64" name=%s\n",
                    bo->offset, bo->size, block_header.bo.name);
         }
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE: {
         struct gem_bo *bo = util_dynarray_grow(&buffers, struct gem_bo, 1);
         *bo = (struct gem_bo) {
            .file_offset = lseek(file_fd, 0, SEEK_CUR),
            .offset = 0,
            .size = block_header.hw_img.size,
            .hw_img = true,
         };
         total_vma += bo->size;
         skip_data(file_fd, bo->size);
         if (list) {
            fprintf(stderr, "buffer: offset=0x%016"PRIx64" size=0x%016"PRIx64" name=hw_img\n",
                    bo->offset, bo->size);
         }
         has_hw_image = true;
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_MAP: {
         struct gem_bo *bo = util_dynarray_grow(&buffers, struct gem_bo, 1);
         *bo = (struct gem_bo) {
            .file_offset = 0,
            .offset = block_header.map.offset,
            .size = block_header.map.size,
         };
         total_vma += bo->size;
         if (list) {
            fprintf(stderr, "map   : offset=0x%016"PRIx64" size=0x%016"PRIx64" name=%s\n",
                    bo->offset, bo->size, block_header.map.name);
         }
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_EXEC: {
         if (init.offset == 0 && !has_hw_image) {
            if (list)
               fprintf(stderr, "init  : offset=0x%016"PRIx64"\n", block_header.exec.offset);
            init = block_header.exec;
         } else {
            if (list)
               fprintf(stderr, "exec  : offset=0x%016"PRIx64"\n", block_header.exec.offset);
            exec = block_header.exec;
         }
         break;
      }

      default:
         unreachable("Invalid block type");
      }
   }

   fprintf(stderr, "total_vma: 0x%016"PRIx64"\n", total_vma);

   if (check_addr != -1) {
      struct gem_bo *check_bo = NULL;
      util_dynarray_foreach(&buffers, struct gem_bo, bo) {
         if (check_addr >= bo->offset && check_addr < (bo->offset + bo->size)) {
            check_bo = bo;
            break;
         }
      }

      if (check_bo) {
         fprintf(stderr, "address=0x%016"PRIx64" found in buffer 0x%016"PRIx64" size=0x%016"PRIx64"\n",
                 check_addr, check_bo->offset, check_bo->size);
      } else {
         fprintf(stderr, "address=0x%016"PRIx64" not found in buffer list\n", check_addr);
      }
   }

   util_dynarray_foreach(&shader_addresses, uint64_t, addr) {
      bool found = false;
      util_dynarray_foreach(&buffers, struct gem_bo, bo) {
         if (*addr < bo->offset || *addr >= (bo->offset + bo->size))
            continue;
         if (!bo->file_offset)
            break;

         uint64_t aligned_offset = ROUND_DOWN_TO(bo->file_offset, 4096);
         uint64_t remaining_length = file_stats.st_size - aligned_offset;
         void *map = mmap(NULL, remaining_length, PROT_READ, MAP_PRIVATE,
                          file_fd, aligned_offset);
         if (map == MAP_FAILED)
            break;

         found = true;
         fprintf(stderr, "shader at 0x%016"PRIx64" file_offset=0%016"PRIx64" addr_offset=%016"PRIx64":\n", *addr,
                 (bo->file_offset - aligned_offset), (*addr - bo->offset));
         struct brw_isa_info _isa, *isa = &_isa;
         brw_init_isa_info(isa, &devinfo);
         intel_disassemble(isa,
                           map + (bo->file_offset - aligned_offset) + (*addr - bo->offset),
                           0, stderr);

         munmap(map, remaining_length);
      }

      if (!found)
         fprintf(stderr, "shader at 0x%016"PRIx64" not found\n", *addr);
   }

   if (!list && util_dynarray_num_elements(&shader_addresses, uint64_t) == 0) {
      /* Sort buffers by size */
      qsort(util_dynarray_begin(&buffers),
            util_dynarray_num_elements(&buffers, struct gem_bo),
            sizeof(struct gem_bo),
            compare_bos);

      /* Allocate BOs populate them */
      uint64_t gem_allocated = 0;
      util_dynarray_foreach(&buffers, struct gem_bo, bo) {
         bo->gem_handle = gem_create(drm_fd, bo->size);
         if (bo->file_offset != 0) {
            lseek(file_fd, bo->file_offset, SEEK_SET);
            write_gem_bo_data(drm_fd, bo->gem_handle, file_fd, bo->size);
         }

         gem_allocated += bo->size;
      }

      struct util_dynarray execbuffer_bos;
      util_dynarray_init(&execbuffer_bos, mem_ctx);

      struct gem_bo *init_bo = NULL, *batch_bo = NULL;
      util_dynarray_foreach(&buffers, struct gem_bo, bo) {
         if (bo->offset <= init.offset &&
             (bo->offset + bo->size) > init.offset) {
               init_bo = bo;
               continue;
         }

         if (bo->offset <= exec.offset &&
             (bo->offset + bo->size) > exec.offset) {
               batch_bo = bo;
               continue;
         }

         struct drm_i915_gem_exec_object2 *execbuf_bo =
            util_dynarray_grow(&execbuffer_bos, struct drm_i915_gem_exec_object2, 1);
         *execbuf_bo = (struct drm_i915_gem_exec_object2) {
            .handle           = bo->gem_handle,
            .relocation_count = 0,
            .relocs_ptr       = 0,
            .flags            = EXEC_OBJECT_SUPPORTS_48B_ADDRESS |
                                EXEC_OBJECT_PINNED,
            .offset           = bo->offset,
         };

         if (bo->hw_img)
            execbuf_bo->flags |= EXEC_OBJECT_NEEDS_GTT;
      }

      assert(batch_bo != NULL);

      struct drm_i915_gem_exec_object2 *execbuf_bo =
         util_dynarray_grow(&execbuffer_bos, struct drm_i915_gem_exec_object2, 1);

      int ret;

      if (init_bo) {
         fprintf(stderr, "init: 0x%016"PRIx64"\n", init_bo->offset);
         *execbuf_bo = (struct drm_i915_gem_exec_object2) {
            .handle           = init_bo->gem_handle,
            .relocation_count = 0,
            .relocs_ptr       = 0,
            .flags            = EXEC_OBJECT_SUPPORTS_48B_ADDRESS |
                                EXEC_OBJECT_PINNED |
                                EXEC_OBJECT_WRITE /* to be able to wait on the BO */,
            .offset           = init_bo->offset,
         };
         ret = execbuffer(drm_fd, &execbuffer_bos, init_bo, init.offset);
         if (ret != 0) {
            fprintf(stderr, "initialization buffer failed to execute errno=%i\n", errno);
            exit(-1);
         }
      } else {
         fprintf(stderr, "no init BO\n");
      }

      if (batch_bo) {
         fprintf(stderr, "exec: 0x%016"PRIx64" aperture=%.2fMb\n", batch_bo->offset,
                 gem_allocated / 1024.0 / 1024.0);
         *execbuf_bo = (struct drm_i915_gem_exec_object2) {
            .handle           = batch_bo->gem_handle,
            .relocation_count = 0,
            .relocs_ptr       = 0,
            .flags            = EXEC_OBJECT_SUPPORTS_48B_ADDRESS |
                                EXEC_OBJECT_PINNED |
                                EXEC_OBJECT_WRITE /* to be able to wait on the BO */,
            .offset           = batch_bo->offset,
         };
         ret = execbuffer(drm_fd, &execbuffer_bos, batch_bo, exec.offset);
         if (ret != 0) {
            fprintf(stderr, "replayed buffer failed to execute errno=%i\n", errno);
            exit(-1);
         } else {
            fprintf(stderr, "exec completed successfully\n");
         }
      } else {
         fprintf(stderr, "no exec BO\n");
      }
   }

   close(drm_fd);
   close(file_fd);

   ralloc_free(mem_ctx);

   return EXIT_SUCCESS;
}
