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
 */

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>

#include "util/list.h"

#include "common/intel_hang_dump.h"

#include "drm-uapi/i915_drm.h"

static inline void
_fail(const char *prefix, const char *format, ...)
{
   va_list args;

   va_start(args, format);
   if (prefix)
      fprintf(stderr, "%s: ", prefix);
   vfprintf(stderr, format, args);
   va_end(args);

   abort();
}

#define _fail_if(cond, prefix, ...) do { \
   if (cond) \
      _fail(prefix, __VA_ARGS__); \
} while (0)

#define fail_if(cond, ...) _fail_if(cond, NULL, __VA_ARGS__)

#define fail(...) fail_if(true, __VA_ARGS__)

static int zlib_inflate(uint32_t **ptr, int len)
{
   struct z_stream_s zstream;
   void *out;
   const uint32_t out_size = 128*4096;  /* approximate obj size */

   memset(&zstream, 0, sizeof(zstream));

   zstream.next_in = (unsigned char *)*ptr;
   zstream.avail_in = 4*len;

   if (inflateInit(&zstream) != Z_OK)
      return 0;

   out = malloc(out_size);
   zstream.next_out = out;
   zstream.avail_out = out_size;

   do {
      switch (inflate(&zstream, Z_SYNC_FLUSH)) {
      case Z_STREAM_END:
         goto end;
      case Z_OK:
         break;
      default:
         inflateEnd(&zstream);
         return 0;
      }

      if (zstream.avail_out)
         break;

      out = realloc(out, 2*zstream.total_out);
      if (out == NULL) {
         inflateEnd(&zstream);
         return 0;
      }

      zstream.next_out = (unsigned char *)out + zstream.total_out;
      zstream.avail_out = zstream.total_out;
   } while (1);
 end:
   inflateEnd(&zstream);
   free(*ptr);
   *ptr = out;
   return zstream.total_out / 4;
}

static int ascii85_decode(const char *in, uint32_t **out, bool inflate)
{
   int len = 0, size = 1024;

   *out = realloc(*out, sizeof(uint32_t)*size);
   if (*out == NULL)
      return 0;

   while (*in >= '!' && *in <= 'z') {
      uint32_t v = 0;

      if (len == size) {
         size *= 2;
         *out = realloc(*out, sizeof(uint32_t)*size);
         if (*out == NULL)
            return 0;
      }

      if (*in == 'z') {
         in++;
      } else {
         v += in[0] - 33; v *= 85;
         v += in[1] - 33; v *= 85;
         v += in[2] - 33; v *= 85;
         v += in[3] - 33; v *= 85;
         v += in[4] - 33;
         in += 5;
      }
      (*out)[len++] = v;
   }

   if (!inflate)
      return len;

   return zlib_inflate(out, len);
}

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s [OPTION]... [FILE]\n"
           "Convert an Intel GPU i915 error state to a hang dump file, replayable with intel_hang_replay.\n"
           "  -h, --help          display this help and exit\n"
           "  -o, --output=FILE   the output dump file (default FILE.dmp)\n",
           progname);
}

struct bo {
   enum address_space {
      PPGTT,
      GGTT,
   } gtt;
   enum bo_type {
      BO_TYPE_UNKNOWN = 0,
      BO_TYPE_BATCH,
      BO_TYPE_USER,
      BO_TYPE_CONTEXT,
      BO_TYPE_RINGBUFFER,
      BO_TYPE_STATUS,
      BO_TYPE_CONTEXT_WA,
   } type;
   const char *name;
   uint64_t addr;
   uint8_t *data;
   uint64_t size;

   enum drm_i915_gem_engine_class engine_class;
   int engine_instance;

   struct list_head link;
};

static struct bo *
find_or_create(struct list_head *bo_list, uint64_t addr,
               enum address_space gtt,
               enum drm_i915_gem_engine_class engine_class,
               int engine_instance)
{
   list_for_each_entry(struct bo, bo_entry, bo_list, link) {
      if (bo_entry->addr == addr &&
          bo_entry->gtt == gtt &&
          bo_entry->engine_class == engine_class &&
          bo_entry->engine_instance == engine_instance)
         return bo_entry;
   }

   struct bo *new_bo = calloc(1, sizeof(*new_bo));
   new_bo->addr = addr;
   new_bo->gtt = gtt;
   new_bo->engine_class = engine_class;
   new_bo->engine_instance = engine_instance;
   list_addtail(&new_bo->link, bo_list);

   return new_bo;
}

static void
engine_from_name(const char *engine_name,
                 enum drm_i915_gem_engine_class *engine_class,
                 int *engine_instance)
{
   const struct {
      const char *match;
      enum drm_i915_gem_engine_class engine_class;
      bool parse_instance;
   } rings[] = {
      { "rcs", I915_ENGINE_CLASS_RENDER, true },
      { "vcs", I915_ENGINE_CLASS_VIDEO, true },
      { "vecs", I915_ENGINE_CLASS_VIDEO_ENHANCE, true },
      { "bcs", I915_ENGINE_CLASS_COPY, true },
      { "global", I915_ENGINE_CLASS_INVALID, false },
      { "render command stream", I915_ENGINE_CLASS_RENDER, false },
      { "blt command stream", I915_ENGINE_CLASS_COPY, false },
      { "bsd command stream", I915_ENGINE_CLASS_VIDEO, false },
      { "vebox command stream", I915_ENGINE_CLASS_VIDEO_ENHANCE, false },
      { NULL, I915_ENGINE_CLASS_INVALID },
   }, *r;

   for (r = rings; r->match; r++) {
      if (strncasecmp(engine_name, r->match, strlen(r->match)) == 0) {
         *engine_class = r->engine_class;
         if (r->parse_instance)
            *engine_instance = strtol(engine_name + strlen(r->match), NULL, 10);
         else
            *engine_instance = 0;
         return;
      }
   }

   fail("Unknown engine %s\n", engine_name);
}

static void
write_header(FILE *f)
{
   struct intel_hang_dump_block_header header = {
      .base = {
         .type = INTEL_HANG_DUMP_BLOCK_TYPE_HEADER,
      },
      .magic   = INTEL_HANG_DUMP_MAGIC,
      .version = INTEL_HANG_DUMP_VERSION,
   };

   fwrite(&header, sizeof(header), 1, f);
}

static void
write_buffer(FILE *f,
             uint64_t offset,
             const void *data,
             uint64_t size,
             const char *name)
{
   struct intel_hang_dump_block_bo header = {
      .base = {
         .type = INTEL_HANG_DUMP_BLOCK_TYPE_BO,
      },
      .offset  = offset,
      .size    = size,
   };
   snprintf(header.name, sizeof(header.name), "%s", name);

   fwrite(&header, sizeof(header), 1, f);
   fwrite(data, size, 1, f);
}

static void
write_hw_image_buffer(FILE *f, const void *data, uint64_t size)
{
   struct intel_hang_dump_block_hw_image header = {
      .base = {
         .type = INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE,
      },
      .size    = size,
   };

   fwrite(&header, sizeof(header), 1, f);
   fwrite(data, size, 1, f);
}

static void
write_exec(FILE *f, uint64_t offset)
{
   struct intel_hang_dump_block_exec header = {
      .base = {
         .type = INTEL_HANG_DUMP_BLOCK_TYPE_EXEC,
      },
      .offset  = offset,
   };

   fwrite(&header, sizeof(header), 1, f);
}

int
main(int argc, char *argv[])
{
   int i, c;
   bool help = false, verbose = false;
   char *out_filename = NULL, *in_filename = NULL, *capture_engine_name = "rcs";
   const struct option aubinator_opts[] = {
      { "help",       no_argument,       NULL,     'h' },
      { "output",     required_argument, NULL,     'o' },
      { "verbose",    no_argument,       NULL,     'v' },
      { "engine",     required_argument, NULL,     'e' },
      { NULL,         0,                 NULL,     0 }
   };

   i = 0;
   while ((c = getopt_long(argc, argv, "ho:v", aubinator_opts, &i)) != -1) {
      switch (c) {
      case 'h':
         help = true;
         break;
      case 'o':
         out_filename = strdup(optarg);
         break;
      case 'v':
         verbose = true;
         break;
      case 'e':
         capture_engine_name = optarg;
         break;
      default:
         break;
      }
   }

   if (optind < argc)
      in_filename = argv[optind++];

   if (help || argc == 1 || !in_filename) {
      print_help(argv[0], stderr);
      return in_filename ? EXIT_SUCCESS : EXIT_FAILURE;
   }

   enum drm_i915_gem_engine_class capture_engine;
   engine_from_name(capture_engine_name, &capture_engine, &c);

   if (out_filename == NULL) {
      int out_filename_size = strlen(in_filename) + 5;
      out_filename = malloc(out_filename_size);
      snprintf(out_filename, out_filename_size, "%s.dmp", in_filename);
   }

   FILE *err_file = fopen(in_filename, "r");
   fail_if(!err_file, "Failed to open error file \"%s\": %m\n", in_filename);

   FILE *hang_file = fopen(out_filename, "w");
   fail_if(!hang_file, "Failed to open aub file \"%s\": %m\n", in_filename);

   enum address_space active_gtt = PPGTT;
   enum address_space default_gtt = PPGTT;

   int num_ring_bos = 0;

   struct list_head bo_list;
   list_inithead(&bo_list);

   struct bo *last_bo = NULL;

   enum drm_i915_gem_engine_class active_engine_class = I915_ENGINE_CLASS_INVALID;
   int active_engine_instance = -1;

   char *line = NULL;
   size_t line_size;
   while (getline(&line, &line_size, err_file) > 0) {
      if (strstr(line, " command stream:")) {
         engine_from_name(line, &active_engine_class, &active_engine_instance);
         continue;
      }

      if (num_ring_bos > 0) {
         unsigned hi, lo, size;
         if (sscanf(line, " %x_%x %d", &hi, &lo, &size) == 3) {
            struct bo *bo_entry = find_or_create(&bo_list, ((uint64_t)hi) << 32 | lo,
                                                 active_gtt,
                                                 active_engine_class,
                                                 active_engine_instance);
            bo_entry->size = size;
            num_ring_bos--;
         } else {
            fail("Not enough BO entries in the active table\n");
         }
         continue;
      }

      if (line[0] == ':' || line[0] == '~') {
         if (!last_bo || last_bo->type == BO_TYPE_UNKNOWN)
            continue;

         int count = ascii85_decode(line+1, (uint32_t **) &last_bo->data, line[0] == ':');
         fail_if(count == 0, "ASCII85 decode failed.\n");
         last_bo->size = count * 4;
         continue;
      }

      char *dashes = strstr(line, " --- ");
      if (dashes) {
         dashes += 5;

         engine_from_name(line, &active_engine_class, &active_engine_instance);

         uint32_t hi, lo;
         char *bo_address_str = strchr(dashes, '=');
         if (!bo_address_str || sscanf(bo_address_str, "= 0x%08x %08x\n", &hi, &lo) != 2)
            continue;

         const struct {
            const char *match;
            enum bo_type type;
            enum address_space gtt;
         } bo_types[] = {
            { "gtt_offset", BO_TYPE_BATCH,      default_gtt },
            { "batch",      BO_TYPE_BATCH,      default_gtt },
            { "user",       BO_TYPE_USER,       default_gtt },
            { "HW context", BO_TYPE_CONTEXT,    GGTT },
            { "ringbuffer", BO_TYPE_RINGBUFFER, GGTT },
            { "HW Status",  BO_TYPE_STATUS,     GGTT },
            { "WA context", BO_TYPE_CONTEXT_WA, GGTT },
            { "unknown",    BO_TYPE_UNKNOWN,    GGTT },
         }, *b;

         for (b = bo_types; b->type != BO_TYPE_UNKNOWN; b++) {
            if (strncasecmp(dashes, b->match, strlen(b->match)) == 0)
               break;
         }

         last_bo = find_or_create(&bo_list, ((uint64_t) hi) << 32 | lo,
                                  b->gtt,
                                  active_engine_class, active_engine_instance);

         /* The batch buffer will appear twice as gtt_offset and user. Only
          * keep the batch type.
          */
         if (last_bo->type == BO_TYPE_UNKNOWN) {
            last_bo->type = b->type;
            last_bo->name = b->match;
         }

         continue;
      }
   }

   if (verbose) {
      fprintf(stdout, "BOs found:\n");
      list_for_each_entry(struct bo, bo_entry, &bo_list, link) {
         fprintf(stdout, "\t type=%i addr=0x%016" PRIx64 " size=%" PRIu64 "\n",
                 bo_entry->type, bo_entry->addr, bo_entry->size);
      }
   }

   /* Find the batch that trigger the hang */
   struct bo *batch_bo = NULL, *hw_image_bo = NULL;
   list_for_each_entry(struct bo, bo_entry, &bo_list, link) {
      if (batch_bo != NULL && hw_image_bo != NULL)
         break;

      if (bo_entry->engine_class != capture_engine)
         continue;

      switch (bo_entry->type) {
      case BO_TYPE_BATCH:
         batch_bo = bo_entry;
         break;
      case BO_TYPE_CONTEXT:
         hw_image_bo = bo_entry;
         break;
      default:
         break;
      }
   }
   fail_if(!batch_bo, "Failed to find batch buffer.\n");
   fail_if(!hw_image_bo, "Failed to find HW image buffer.\n");

   /* Add all the user BOs to the aub file */
   list_for_each_entry(struct bo, bo_entry, &bo_list, link) {
      if (bo_entry->type == BO_TYPE_USER && bo_entry->gtt == PPGTT)
         write_buffer(hang_file, bo_entry->addr, bo_entry->data, bo_entry->size, "user");
   }

   write_buffer(hang_file, batch_bo->addr, batch_bo->data, batch_bo->size, "batch");
   fprintf(stderr, "writing image buffer 0x%016"PRIx64" size=0x%016"PRIx64"\n",
           hw_image_bo->addr, hw_image_bo->size);
   write_hw_image_buffer(hang_file, hw_image_bo->data, hw_image_bo->size);
   write_exec(hang_file, batch_bo->addr);

   /* Cleanup */
   list_for_each_entry_safe(struct bo, bo_entry, &bo_list, link) {
      list_del(&bo_entry->link);
      free(bo_entry->data);
      free(bo_entry);
   }

   free(out_filename);
   free(line);
   if (err_file)
      fclose(err_file);
   fclose(hang_file);

   return EXIT_SUCCESS;
}

/* vim: set ts=8 sw=8 tw=0 cino=:0,(0 noet :*/
