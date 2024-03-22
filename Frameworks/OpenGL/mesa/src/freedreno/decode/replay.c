/*
 * Copyright © 2022 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !FD_REPLAY_KGSL
#include <xf86drm.h>
#include "drm-uapi/msm_drm.h"
#else
#include "../vulkan/msm_kgsl.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "util/os_time.h"
#include "util/rb_tree.h"
#include "util/u_vector.h"
#include "util/vma.h"
#include "buffers.h"
#include "cffdec.h"
#include "io.h"
#include "redump.h"
#include "rdutil.h"

/**
 * Replay command stream obtained from:
 * - /sys/kernel/debug/dri/0/rd
 * - /sys/kernel/debug/dri/0/hangrd
 * !!! Command stream capture should be done with ALL buffers:
 * - echo 1 > /sys/module/msm/parameters/rd_full
 *
 * Requires kernel with MSM_INFO_SET_IOVA support.
 * In case userspace IOVAs are not supported, like on KGSL, we have to
 * pre-allocate a single buffer and hope it always allocated starting
 * from the same address.
 *
 * TODO: Misrendering, would require marking framebuffer images
 *       at each renderpass in order to fetch and decode them.
 *
 * Code from Freedreno/Turnip is not re-used here since the relevant
 * pieces may introduce additional allocations which cannot be allowed
 * during the replay.
 *
 * For how-to see freedreno.rst
 */

static const char *exename = NULL;

static const uint64_t FAKE_ADDRESS_SPACE_SIZE = 1024 * 1024 * 1024;

static int handle_file(const char *filename, uint32_t first_submit,
                       uint32_t last_submit, uint32_t submit_to_override,
                       const char *cmdstreamgen);

static void
print_usage(const char *name)
{
   /* clang-format off */
   fprintf(stderr, "Usage:\n\n"
           "\t%s [OPTSIONS]... FILE...\n\n"
           "Options:\n"
           "\t-e, --exe=NAME         - only use cmdstream from named process\n"
           "\t-o  --override=submit  - № of the submit to override\n"
           "\t-g  --generator=path   - executable which generate cmdstream for override\n"
           "\t-f  --first=submit     - first submit № to replay\n"
           "\t-l  --last=submit      - last submit № to replay\n"
           "\t-h, --help             - show this message\n"
           , name);
   /* clang-format on */
   exit(2);
}

/* clang-format off */
static const struct option opts[] = {
      { "exe",       required_argument, 0, 'e' },
      { "override",  required_argument, 0, 'o' },
      { "generator", required_argument, 0, 'g' },
      { "first",     required_argument, 0, 'f' },
      { "last",      required_argument, 0, 'l' },
      { "help",      no_argument,       0, 'h' },
};
/* clang-format on */

int
main(int argc, char **argv)
{
   int ret = -1;
   int c;

   uint32_t submit_to_override = -1;
   uint32_t first_submit = 0;
   uint32_t last_submit = -1;
   const char *cmdstreamgen = NULL;

   while ((c = getopt_long(argc, argv, "e:o:g:f:l:h", opts, NULL)) != -1) {
      switch (c) {
      case 0:
         /* option that set a flag, nothing to do */
         break;
      case 'e':
         exename = optarg;
         break;
      case 'o':
         submit_to_override = strtoul(optarg, NULL, 0);
         break;
      case 'g':
         cmdstreamgen = optarg;
         break;
      case 'f':
         first_submit = strtoul(optarg, NULL, 0);
         break;
      case 'l':
         last_submit = strtoul(optarg, NULL, 0);
         break;
      case 'h':
      default:
         print_usage(argv[0]);
      }
   }

   while (optind < argc) {
      ret = handle_file(argv[optind], first_submit, last_submit,
                        submit_to_override, cmdstreamgen);
      if (ret) {
         fprintf(stderr, "error reading: %s\n", argv[optind]);
         fprintf(stderr, "continuing..\n");
      }
      optind++;
   }

   if (ret)
      print_usage(argv[0]);

   return ret;
}

struct buffer {
   struct rb_node node;

   uint32_t gem_handle;
   uint64_t size;
   uint64_t iova;
   void *map;

   bool used;
   uint32_t flags;
};

struct cmdstream {
   uint64_t iova;
   uint64_t size;
};

struct wrbuf {
   uint64_t iova;
   uint64_t size;
   char* name;
};

struct device {
   int fd;

   struct rb_tree buffers;
   struct util_vma_heap vma;

   struct u_vector cmdstreams;

   uint64_t shader_log_iova;
   uint64_t cp_log_iova;

   bool has_set_iova;

   uint32_t va_id;
   void *va_map;
   uint64_t va_iova;

#ifdef FD_REPLAY_KGSL
   uint32_t context_id;
#endif

   struct u_vector wrbufs;
};

void buffer_mem_free(struct device *dev, struct buffer *buf);

static int
rb_buffer_insert_cmp(const struct rb_node *n1, const struct rb_node *n2)
{
   const struct buffer *buf1 = (const struct buffer *)n1;
   const struct buffer *buf2 = (const struct buffer *)n2;
   /* Note that gpuaddr comparisions can overflow an int: */
   if (buf1->iova > buf2->iova)
      return 1;
   else if (buf1->iova < buf2->iova)
      return -1;
   return 0;
}

static int
rb_buffer_search_cmp(const struct rb_node *node, const void *addrptr)
{
   const struct buffer *buf = (const struct buffer *)node;
   uint64_t iova = *(uint64_t *)addrptr;
   if (buf->iova + buf->size <= iova)
      return -1;
   else if (buf->iova > iova)
      return 1;
   return 0;
}

static struct buffer *
device_get_buffer(struct device *dev, uint64_t iova)
{
   if (iova == 0)
      return NULL;
   return (struct buffer *)rb_tree_search(&dev->buffers, &iova,
                                          rb_buffer_search_cmp);
}

static void
device_mark_buffers(struct device *dev)
{
   rb_tree_foreach_safe (struct buffer, buf, &dev->buffers, node) {
      buf->used = false;
   }
}

static void
device_free_unused_buffers(struct device *dev)
{
   rb_tree_foreach_safe (struct buffer, buf, &dev->buffers, node) {
      if (!buf->used) {
         buffer_mem_free(dev, buf);
         rb_tree_remove(&dev->buffers, &buf->node);
         free(buf);
      }
   }
}

static void
device_print_shader_log(struct device *dev)
{
   struct shader_log {
      uint64_t cur_iova;
      union {
         uint32_t entries_u32[0];
         float entries_float[0];
      };
   };

   if (dev->shader_log_iova != 0)
   {
      struct buffer *buf = device_get_buffer(dev, dev->shader_log_iova);
      if (buf) {
         struct shader_log *log = buf->map + (dev->shader_log_iova - buf->iova);
         uint32_t count = (log->cur_iova - dev->shader_log_iova -
                           offsetof(struct shader_log, entries_u32)) / 4;

         printf("Shader Log Entries: %u\n", count);

         for (uint32_t i = 0; i < count; i++) {
            printf("[%u] %08x %.4f\n", i, log->entries_u32[i],
                   log->entries_float[i]);
         }

         printf("========================================\n");
      }
   }
}

static void
device_print_cp_log(struct device *dev)
{
   struct cp_log {
      uint64_t cur_iova;
      uint64_t tmp;
      uint64_t first_entry_size;
   };

   struct cp_log_entry {
      uint64_t size;
      uint32_t data[0];
   };

   if (dev->cp_log_iova == 0)
      return;

   struct buffer *buf = device_get_buffer(dev, dev->cp_log_iova);
   if (!buf)
      return;

   struct cp_log *log = buf->map + (dev->cp_log_iova - buf->iova);
   if (log->first_entry_size == 0)
      return;

   struct cp_log_entry *log_entry =
      buf->map + offsetof(struct cp_log, first_entry_size);
   uint32_t idx = 0;
   while (log_entry->size != 0) {
      printf("\nCP Log [%u]:\n", idx++);
      uint32_t dwords = log_entry->size / 4;

      for (uint32_t i = 0; i < dwords; i++) {
         if (i % 8 == 0)
            printf("\t");
         printf("%08x ", log_entry->data[i]);
         if (i % 8 == 7)
            printf("\n");
      }
      printf("\n");

      log_entry = (void *)log_entry + log_entry->size +
                  offsetof(struct cp_log_entry, data);
   }
}

static void
device_dump_wrbuf(struct device *dev)
{
   if (!u_vector_length(&dev->wrbufs))
      return;

   char buffer_dir[256];
   snprintf(buffer_dir, sizeof(buffer_dir), "%s/buffers", exename);
   rmdir(buffer_dir);
   mkdir(buffer_dir, 0777);

   struct wrbuf *wrbuf;
   u_vector_foreach(wrbuf, &dev->wrbufs) {
      char buffer_path[256];
      snprintf(buffer_path, sizeof(buffer_path), "%s/%s", buffer_dir, wrbuf->name);
      FILE *f = fopen(buffer_path, "wb");
      if (!f) {
         fprintf(stderr, "Error opening %s\n", buffer_path);
         goto end_it;
      }

      struct buffer *buf = device_get_buffer(dev, wrbuf->iova);
      if (!buf) {
         fprintf(stderr, "Error getting buffer for %s\n", buffer_path);
         goto end_it;
      }
      const void *buffer = buf->map + (wrbuf->iova - buf->iova);
      fwrite(buffer, wrbuf->size, 1, f);

      end_it:
      fclose(f);
   }
}

#if !FD_REPLAY_KGSL
static inline void
get_abs_timeout(struct drm_msm_timespec *tv, uint64_t ns)
{
   struct timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   tv->tv_sec = t.tv_sec + ns / 1000000000;
   tv->tv_nsec = t.tv_nsec + ns % 1000000000;
}

static struct device *
device_create()
{
   struct device *dev = calloc(sizeof(struct device), 1);

   dev->fd = drmOpenWithType("msm", NULL, DRM_NODE_RENDER);
   if (dev->fd < 0) {
      errx(1, "Cannot open MSM fd!");
   }

   uint64_t va_start, va_size;

   struct drm_msm_param req = {
      .pipe = MSM_PIPE_3D0,
      .param = MSM_PARAM_VA_START,
   };

   int ret = drmCommandWriteRead(dev->fd, DRM_MSM_GET_PARAM, &req, sizeof(req));
   va_start = req.value;

   if (!ret) {
      req.param = MSM_PARAM_VA_SIZE;
      ret = drmCommandWriteRead(dev->fd, DRM_MSM_GET_PARAM, &req, sizeof(req));
      va_size = req.value;

      dev->has_set_iova = true;
   }

   if (ret) {
      printf("MSM_INFO_SET_IOVA is not supported!\n");

      struct drm_msm_gem_new req_new = {.size = FAKE_ADDRESS_SPACE_SIZE, .flags = MSM_BO_CACHED_COHERENT};
      drmCommandWriteRead(dev->fd, DRM_MSM_GEM_NEW, &req_new, sizeof(req_new));
      dev->va_id = req_new.handle;

      struct drm_msm_gem_info req_info = {
         .handle = req_new.handle,
         .info = MSM_INFO_GET_IOVA,
      };

      drmCommandWriteRead(dev->fd,
                                 DRM_MSM_GEM_INFO, &req_info, sizeof(req_info));
      dev->va_iova = req_info.value;

      struct drm_msm_gem_info req_offset = {
         .handle = req_new.handle,
         .info = MSM_INFO_GET_OFFSET,
      };

      drmCommandWriteRead(dev->fd, DRM_MSM_GEM_INFO, &req_offset, sizeof(req_offset));

      dev->va_map = mmap(0, FAKE_ADDRESS_SPACE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       dev->fd, req_offset.value);
      if (dev->va_map == MAP_FAILED) {
         err(1, "mmap failure");
      }

      va_start = dev->va_iova;
      va_size = FAKE_ADDRESS_SPACE_SIZE;

      printf("Allocated iova %" PRIx64 "\n", dev->va_iova);
   }

   rb_tree_init(&dev->buffers);
   util_vma_heap_init(&dev->vma, va_start, ROUND_DOWN_TO(va_size, 4096));
   u_vector_init(&dev->cmdstreams, 8, sizeof(struct cmdstream));
   u_vector_init(&dev->wrbufs, 8, sizeof(struct wrbuf));

   return dev;
}

static void
device_submit_cmdstreams(struct device *dev)
{
   device_free_unused_buffers(dev);
   device_mark_buffers(dev);

   if (!u_vector_length(&dev->cmdstreams))
      return;

   struct drm_msm_gem_submit_cmd cmds[u_vector_length(&dev->cmdstreams)];

   uint32_t idx = 0;
   struct cmdstream *cmd;
   u_vector_foreach(cmd, &dev->cmdstreams) {
      struct buffer *cmdstream_buf = device_get_buffer(dev, cmd->iova);

      uint32_t bo_idx = 0;
      rb_tree_foreach (struct buffer, buf, &dev->buffers, node) {
         if (buf == cmdstream_buf)
            break;

         bo_idx++;
      }

      if (cmdstream_buf)
         cmdstream_buf->flags = MSM_SUBMIT_BO_DUMP;

      struct drm_msm_gem_submit_cmd *submit_cmd = &cmds[idx];
      submit_cmd->type = MSM_SUBMIT_CMD_BUF;
      submit_cmd->submit_idx = bo_idx;
      if (dev->has_set_iova) {
         submit_cmd->submit_offset = cmd->iova - cmdstream_buf->iova;
      } else {
         submit_cmd->submit_offset = cmd->iova - dev->va_iova;
      }
      submit_cmd->size = cmd->size;
      submit_cmd->pad = 0;
      submit_cmd->nr_relocs = 0;
      submit_cmd->relocs = 0;

      idx++;
   }

   uint32_t bo_count = 0;
   rb_tree_foreach (struct buffer, buf, &dev->buffers, node) {
      if (buf)
         bo_count++;
   }

   if (!dev->has_set_iova) {
      bo_count = 1;
   }

   struct drm_msm_gem_submit_bo *bo_list =
      calloc(sizeof(struct drm_msm_gem_submit_bo), bo_count);

   if (dev->has_set_iova) {
      uint32_t bo_idx = 0;
      rb_tree_foreach (struct buffer, buf, &dev->buffers, node) {
         struct drm_msm_gem_submit_bo *submit_bo = &bo_list[bo_idx++];
         submit_bo->handle = buf->gem_handle;
         submit_bo->flags =
            buf->flags | MSM_SUBMIT_BO_READ | MSM_SUBMIT_BO_WRITE;
         submit_bo->presumed = buf->iova;

         buf->flags = 0;
      }
   } else {
      bo_list[0].handle = dev->va_id;
      bo_list[0].flags =
         MSM_SUBMIT_BO_DUMP | MSM_SUBMIT_BO_READ | MSM_SUBMIT_BO_WRITE;
      bo_list[0].presumed = dev->va_iova;
   }

   struct drm_msm_gem_submit submit_req = {
      .flags = MSM_PIPE_3D0,
      .queueid = 0,
      .bos = (uint64_t)(uintptr_t)bo_list,
      .nr_bos = bo_count,
      .cmds = (uint64_t)(uintptr_t)cmds,
      .nr_cmds = u_vector_length(&dev->cmdstreams),
      .in_syncobjs = 0,
      .out_syncobjs = 0,
      .nr_in_syncobjs = 0,
      .nr_out_syncobjs = 0,
      .syncobj_stride = sizeof(struct drm_msm_gem_submit_syncobj),
   };

   int ret = drmCommandWriteRead(dev->fd, DRM_MSM_GEM_SUBMIT, &submit_req,
                                 sizeof(submit_req));

   if (ret) {
      err(1, "DRM_MSM_GEM_SUBMIT failure %d", ret);
   }

   /* Wait for submission to complete in order to be sure that
    * freeing buffers would free their VMAs in the kernel.
    * Makes sure that new allocations won't clash with old ones.
    */
   struct drm_msm_wait_fence wait_req = {
      .fence = submit_req.fence,
      .queueid = 0,
   };
   get_abs_timeout(&wait_req.timeout, 1000000000);

   ret =
      drmCommandWrite(dev->fd, DRM_MSM_WAIT_FENCE, &wait_req, sizeof(wait_req));
   if (ret && (ret != -ETIMEDOUT)) {
      err(1, "DRM_MSM_WAIT_FENCE failure %d", ret);
   }

   u_vector_finish(&dev->cmdstreams);
   u_vector_init(&dev->cmdstreams, 8, sizeof(struct cmdstream));

   device_print_shader_log(dev);
   device_print_cp_log(dev);

   device_dump_wrbuf(dev);
}

static void
buffer_mem_alloc(struct device *dev, struct buffer *buf)
{
   util_vma_heap_alloc_addr(&dev->vma, buf->iova, buf->size);

   if (!dev->has_set_iova) {
      uint64_t offset = buf->iova - dev->va_iova;
      assert(offset < FAKE_ADDRESS_SPACE_SIZE && (offset + buf->size) <= FAKE_ADDRESS_SPACE_SIZE);
      buf->map = ((uint8_t*)dev->va_map) + offset;
      return;
   }

   {
      struct drm_msm_gem_new req = {.size = buf->size, .flags = MSM_BO_WC};

      int ret =
         drmCommandWriteRead(dev->fd, DRM_MSM_GEM_NEW, &req, sizeof(req));
      if (ret) {
         err(1, "DRM_MSM_GEM_NEW failure %d", ret);
      }

      buf->gem_handle = req.handle;
   }

   {
      struct drm_msm_gem_info req = {
         .handle = buf->gem_handle,
         .info = MSM_INFO_SET_IOVA,
         .value = buf->iova,
      };

      int ret =
         drmCommandWriteRead(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));

      if (ret) {
         err(1, "MSM_INFO_SET_IOVA failure %d", ret);
      }
   }

   {
      struct drm_msm_gem_info req = {
         .handle = buf->gem_handle,
         .info = MSM_INFO_GET_OFFSET,
      };

      int ret =
         drmCommandWriteRead(dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));
      if (ret) {
         err(1, "MSM_INFO_GET_OFFSET failure %d", ret);
      }

      void *map = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                       dev->fd, req.value);
      if (map == MAP_FAILED) {
         err(1, "mmap failure");
      }

      buf->map = map;
   }
}

void
buffer_mem_free(struct device *dev, struct buffer *buf)
{
   if (dev->has_set_iova) {
      munmap(buf->map, buf->size);

      struct drm_gem_close req = {
         .handle = buf->gem_handle,
      };
      drmIoctl(dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
   }

   util_vma_heap_free(&dev->vma, buf->iova, buf->size);
}

#else
static int
safe_ioctl(int fd, unsigned long request, void *arg)
{
   int ret;

   do {
      ret = ioctl(fd, request, arg);
   } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

   return ret;
}

static struct device *
device_create()
{
   struct device *dev = calloc(sizeof(struct device), 1);

   static const char path[] = "/dev/kgsl-3d0";

   dev->fd = open(path, O_RDWR | O_CLOEXEC);
   if (dev->fd < 0) {
      errx(1, "Cannot open KGSL fd!");
   }

   struct kgsl_gpumem_alloc_id req = {
      .size = FAKE_ADDRESS_SPACE_SIZE,
      .flags = KGSL_MEMFLAGS_IOCOHERENT,
   };

   int ret = safe_ioctl(dev->fd, IOCTL_KGSL_GPUMEM_ALLOC_ID, &req);
   if (ret) {
      err(1, "IOCTL_KGSL_GPUMEM_ALLOC_ID failure");
   }

   dev->va_id = req.id;
   dev->va_iova = req.gpuaddr;
   dev->va_map = mmap(0, FAKE_ADDRESS_SPACE_SIZE, PROT_READ | PROT_WRITE,
                      MAP_SHARED, dev->fd, req.id << 12);

   rb_tree_init(&dev->buffers);
   util_vma_heap_init(&dev->vma, req.gpuaddr, ROUND_DOWN_TO(FAKE_ADDRESS_SPACE_SIZE, 4096));
   u_vector_init(&dev->cmdstreams, 8, sizeof(struct cmdstream));

   struct kgsl_drawctxt_create drawctxt_req = {
      .flags = KGSL_CONTEXT_SAVE_GMEM |
              KGSL_CONTEXT_NO_GMEM_ALLOC |
              KGSL_CONTEXT_PREAMBLE,
   };

   ret = safe_ioctl(dev->fd, IOCTL_KGSL_DRAWCTXT_CREATE, &drawctxt_req);
   if (ret) {
      err(1, "IOCTL_KGSL_DRAWCTXT_CREATE failure");
   }

   printf("Allocated iova %" PRIx64 "\n", dev->va_iova);

   dev->context_id = drawctxt_req.drawctxt_id;

   return dev;
}

static void
device_submit_cmdstreams(struct device *dev)
{
   device_free_unused_buffers(dev);
   device_mark_buffers(dev);

   if (!u_vector_length(&dev->cmdstreams))
      return;

   struct kgsl_command_object cmds[u_vector_length(&dev->cmdstreams)];

   uint32_t idx = 0;
   struct cmdstream *cmd;
   u_vector_foreach(cmd, &dev->cmdstreams) {
      struct kgsl_command_object *submit_cmd = &cmds[idx++];
      submit_cmd->gpuaddr = cmd->iova;
      submit_cmd->size = cmd->size;
      submit_cmd->flags = KGSL_CMDLIST_IB;
      submit_cmd->id = dev->va_id;
   }

   struct kgsl_gpu_command submit_req = {
      .flags = KGSL_CMDBATCH_SUBMIT_IB_LIST,
      .cmdlist = (uintptr_t) &cmds,
      .cmdsize = sizeof(struct kgsl_command_object),
      .numcmds = u_vector_length(&dev->cmdstreams),
      .numsyncs = 0,
      .context_id = dev->context_id,
   };

   int ret = safe_ioctl(dev->fd, IOCTL_KGSL_GPU_COMMAND, &submit_req);

   if (ret) {
      err(1, "IOCTL_KGSL_GPU_COMMAND failure %d", ret);
   }

   struct kgsl_device_waittimestamp_ctxtid wait = {
      .context_id = dev->context_id,
      .timestamp = submit_req.timestamp,
      .timeout = 3000,
   };

   ret = safe_ioctl(dev->fd, IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID, &wait);

   if (ret) {
      err(1, "IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID failure %d", ret);
   }

   u_vector_finish(&dev->cmdstreams);
   u_vector_init(&dev->cmdstreams, 8, sizeof(struct cmdstream));

   device_print_shader_log(dev);
   device_print_cp_log(dev);

   device_dump_wrbuf(dev);
}

static void
buffer_mem_alloc(struct device *dev, struct buffer *buf)
{
   util_vma_heap_alloc_addr(&dev->vma, buf->iova, buf->size);

   buf->map = ((uint8_t*)dev->va_map) + (buf->iova - dev->va_iova);
}

void
buffer_mem_free(struct device *dev, struct buffer *buf)
{
   util_vma_heap_free(&dev->vma, buf->iova, buf->size);
}
#endif

static void
upload_buffer(struct device *dev, uint64_t iova, unsigned int size,
              void *hostptr)
{
   struct buffer *buf = device_get_buffer(dev, iova);

   if (!buf) {
      buf = calloc(sizeof(struct buffer), 1);
      buf->iova = iova;
      buf->size = size;

      rb_tree_insert(&dev->buffers, &buf->node, rb_buffer_insert_cmp);

      buffer_mem_alloc(dev, buf);
   } else if (buf->size != size) {
      buffer_mem_free(dev, buf);
      buf->size = size;
      buffer_mem_alloc(dev, buf);
   }

   memcpy(buf->map, hostptr, size);

   buf->used = true;
}

static int
override_cmdstream(struct device *dev, struct cmdstream *cs,
                   const char *cmdstreamgen)
{
#if FD_REPLAY_KGSL
   static const char *tmpfilename = "/sdcard/Download/cmdstream_override.rd";
#else
   static const char *tmpfilename = "/tmp/cmdstream_override.rd";
#endif


   /* Find a free space for the new cmdstreams and resources we will use
    * when overriding existing cmdstream.
    */
   /* TODO: should the size be configurable? */
   uint64_t hole_size = 32 * 1024 * 1024;
   dev->vma.alloc_high = true;
   uint64_t hole_iova = util_vma_heap_alloc(&dev->vma, hole_size, 4096);
   dev->vma.alloc_high = false;
   util_vma_heap_free(&dev->vma, hole_iova, hole_size);

   char cmd[2048];
   snprintf(cmd, sizeof(cmd),
            "%s --vastart=%" PRIu64 " --vasize=%" PRIu64 " %s", cmdstreamgen,
            hole_iova, hole_size, tmpfilename);

   printf("generating cmdstream '%s'\n", cmd);

   int ret = system(cmd);
   if (ret) {
      fprintf(stderr, "Error executing %s\n", cmd);
      return -1;
   }

   struct io *io;
   struct rd_parsed_section ps = {0};

   io = io_open(tmpfilename);
   if (!io) {
      fprintf(stderr, "could not open: %s\n", tmpfilename);
      return -1;
   }

   struct {
      unsigned int len;
      uint64_t gpuaddr;
   } gpuaddr = {0};

   while (parse_rd_section(io, &ps)) {
      switch (ps.type) {
      case RD_GPUADDR:
         parse_addr(ps.buf, ps.sz, &gpuaddr.len, &gpuaddr.gpuaddr);
         /* no-op */
         break;
      case RD_BUFFER_CONTENTS:
         upload_buffer(dev, gpuaddr.gpuaddr, gpuaddr.len, ps.buf);
         ps.buf = NULL;
         break;
      case RD_CMDSTREAM_ADDR: {
         unsigned int sizedwords;
         uint64_t gpuaddr;
         parse_addr(ps.buf, ps.sz, &sizedwords, &gpuaddr);
         printf("override cmdstream: %d dwords\n", sizedwords);

         cs->iova = gpuaddr;
         cs->size = sizedwords * sizeof(uint32_t);
         break;
      }
      case RD_SHADER_LOG_BUFFER: {
         unsigned int sizedwords;
         parse_addr(ps.buf, ps.sz, &sizedwords, &dev->shader_log_iova);
         break;
      }
      case RD_CP_LOG_BUFFER: {
         unsigned int sizedwords;
         parse_addr(ps.buf, ps.sz, &sizedwords, &dev->cp_log_iova);
         break;
      }
      case RD_WRBUFFER: {
         struct wrbuf *wrbuf = u_vector_add(&dev->wrbufs);
         uint64_t *p = (uint64_t *)ps.buf;
         wrbuf->iova = p[0];
         wrbuf->size = p[1];
         wrbuf->name = calloc(1, p[2]);
         memcpy(wrbuf->name, (char *)ps.buf + 3 * sizeof(uint64_t), p[2]);
         break;
      }
      default:
         break;
      }
   }

   io_close(io);
   if (ps.ret < 0) {
      fprintf(stderr, "corrupt file %s\n", tmpfilename);
   }

   return ps.ret;
}

static int
handle_file(const char *filename, uint32_t first_submit, uint32_t last_submit,
            uint32_t submit_to_override, const char *cmdstreamgen)
{
   struct io *io;
   int submit = 0;
   bool skip = false;
   bool need_submit = false;
   struct rd_parsed_section ps = {0};

   printf("Reading %s...\n", filename);

   if (!strcmp(filename, "-"))
      io = io_openfd(0);
   else
      io = io_open(filename);

   if (!io) {
      fprintf(stderr, "could not open: %s\n", filename);
      return -1;
   }

   struct device *dev = device_create();

   struct {
      unsigned int len;
      uint64_t gpuaddr;
   } gpuaddr = {0};

   while (parse_rd_section(io, &ps)) {
      switch (ps.type) {
      case RD_TEST:
      case RD_VERT_SHADER:
      case RD_FRAG_SHADER:
         /* no-op */
         break;
      case RD_CMD:
         skip = false;
         if (exename) {
            skip |= (strstr(ps.buf, exename) != ps.buf);
         } else {
            skip |= (strstr(ps.buf, "fdperf") == ps.buf);
            skip |= (strstr(ps.buf, "chrome") == ps.buf);
            skip |= (strstr(ps.buf, "surfaceflinger") == ps.buf);
            skip |= ((char *)ps.buf)[0] == 'X';
         }
         break;

      case RD_GPUADDR:
         if (need_submit) {
            need_submit = false;
            device_submit_cmdstreams(dev);
         }

         parse_addr(ps.buf, ps.sz, &gpuaddr.len, &gpuaddr.gpuaddr);
         /* no-op */
         break;
      case RD_BUFFER_CONTENTS:
         /* TODO: skip buffer uploading and even reading if this buffer
          * is used for submit outside of [first_submit, last_submit]
          * range. A set of buffers is shared between several cmdstreams,
          * so we'd have to find starting from which RD_CMD to upload
          * the buffers.
          */
         upload_buffer(dev, gpuaddr.gpuaddr, gpuaddr.len, ps.buf);
         break;
      case RD_CMDSTREAM_ADDR: {
         unsigned int sizedwords;
         uint64_t gpuaddr;
         parse_addr(ps.buf, ps.sz, &sizedwords, &gpuaddr);

         bool add_submit = !skip && (submit >= first_submit) && (submit <= last_submit);
         printf("%scmdstream %d: %d dwords\n", add_submit ? "" : "skipped ",
                submit, sizedwords);

         if (add_submit) {
            struct cmdstream *cs = u_vector_add(&dev->cmdstreams);

            if (submit == submit_to_override) {
               if (override_cmdstream(dev, cs, cmdstreamgen) < 0)
                  break;
            } else {
               cs->iova = gpuaddr;
               cs->size = sizedwords * sizeof(uint32_t);
            }

            need_submit = true;
         }

         submit++;
         break;
      }
      case RD_GPU_ID: {
         uint32_t gpu_id = parse_gpu_id(ps.buf);
         if (gpu_id)
            printf("gpuid: %d\n", gpu_id);
         break;
      }
      case RD_CHIP_ID: {
         uint64_t chip_id = parse_chip_id(ps.buf);
         printf("chip_id: 0x%" PRIx64 "\n", chip_id);
         break;
      }
      default:
         break;
      }
   }

   if (need_submit)
      device_submit_cmdstreams(dev);

   close(dev->fd);

   io_close(io);
   fflush(stdout);

   if (ps.ret < 0) {
      printf("corrupt file\n");
   }
   return 0;
}
