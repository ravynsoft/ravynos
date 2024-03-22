/*
 * Copyright © 2018 Intel Corporation
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

#include "aub_write.h"
#include "intel_aub.h"

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
           "Convert an Intel GPU i915 error state to an aub file.\n"
           "  -h, --help          display this help and exit\n"
           "  -o, --output=FILE   the output aub file (default FILE.aub)\n",
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

   enum intel_engine_class engine_class;
   int engine_instance;

   struct list_head link;
};

static struct bo *
find_or_create(struct list_head *bo_list, uint64_t addr,
               enum address_space gtt,
               enum intel_engine_class engine_class,
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
                 enum intel_engine_class *engine_class,
                 int *engine_instance)
{
   const struct {
      const char *match;
      enum intel_engine_class engine_class;
      bool parse_instance;
   } rings[] = {
      { "rcs", INTEL_ENGINE_CLASS_RENDER, true },
      { "vcs", INTEL_ENGINE_CLASS_VIDEO, true },
      { "vecs", INTEL_ENGINE_CLASS_VIDEO_ENHANCE, true },
      { "bcs", INTEL_ENGINE_CLASS_COPY, true },
      { "global", INTEL_ENGINE_CLASS_INVALID, false },
      { "render command stream", INTEL_ENGINE_CLASS_RENDER, false },
      { "blt command stream", INTEL_ENGINE_CLASS_COPY, false },
      { "bsd command stream", INTEL_ENGINE_CLASS_VIDEO, false },
      { "vebox command stream", INTEL_ENGINE_CLASS_VIDEO_ENHANCE, false },
      { NULL, INTEL_ENGINE_CLASS_INVALID },
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

int
main(int argc, char *argv[])
{
   int i, c;
   bool help = false, verbose = false;
   char *out_filename = NULL, *in_filename = NULL;
   const struct option aubinator_opts[] = {
      { "help",       no_argument,       NULL,     'h' },
      { "output",     required_argument, NULL,     'o' },
      { "verbose",    no_argument,       NULL,     'v' },
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

   if (out_filename == NULL) {
      int out_filename_size = strlen(in_filename) + 5;
      out_filename = malloc(out_filename_size);
      snprintf(out_filename, out_filename_size, "%s.aub", in_filename);
   }

   FILE *err_file = fopen(in_filename, "r");
   fail_if(!err_file, "Failed to open error file \"%s\": %m\n", in_filename);

   FILE *aub_file = fopen(out_filename, "w");
   fail_if(!aub_file, "Failed to open aub file \"%s\": %m\n", in_filename);

   struct aub_file aub = {};

   enum intel_engine_class active_engine_class = INTEL_ENGINE_CLASS_INVALID;
   int active_engine_instance = -1;

   enum address_space active_gtt = PPGTT;
   enum address_space default_gtt = PPGTT;

   struct {
      struct {
         uint32_t ring_buffer_head;
         uint32_t ring_buffer_tail;
      } instances[3];
   } engines[INTEL_ENGINE_CLASS_INVALID + 1];
   memset(engines, 0, sizeof(engines));

   int num_ring_bos = 0;

   struct list_head bo_list;
   list_inithead(&bo_list);

   struct bo *last_bo = NULL;

   char *line = NULL;
   size_t line_size;
   while (getline(&line, &line_size, err_file) > 0) {
      const char *pci_id_start = strstr(line, "PCI ID");
      if (pci_id_start) {
         int pci_id;
         int matched = sscanf(line, "PCI ID: 0x%04x\n", &pci_id);
         fail_if(!matched, "Invalid error state file!\n");

         aub_file_init(&aub, aub_file,
                       NULL, pci_id, "error_state");
         if (verbose)
            aub.verbose_log_file = stdout;
         default_gtt = active_gtt = aub_use_execlists(&aub) ? PPGTT : GGTT;
         continue;
      }

      if (strstr(line, " command stream:")) {
         engine_from_name(line, &active_engine_class, &active_engine_instance);
         continue;
      }

      if (sscanf(line, "  ring->head: 0x%x\n",
                 &engines[
                    active_engine_class].instances[
                       active_engine_instance].ring_buffer_head) == 1) {
         continue;
      }

      if (sscanf(line, "  ring->tail: 0x%x\n",
                 &engines[
                    active_engine_class].instances[
                       active_engine_instance].ring_buffer_tail) == 1) {
         continue;
      }

      const char *active_start = "Active (";
      if (strncmp(line, active_start, strlen(active_start)) == 0) {
         char *ring = line + strlen(active_start);

         engine_from_name(ring, &active_engine_class, &active_engine_instance);
         active_gtt = default_gtt;

         char *count = strchr(ring, '[');
         fail_if(!count || sscanf(count, "[%d]:", &num_ring_bos) < 1,
                 "Failed to parse BO table header\n");
         continue;
      }

      const char *global_start = "Pinned (global) [";
      if (strncmp(line, global_start, strlen(global_start)) == 0) {
         active_engine_class = INTEL_ENGINE_CLASS_INVALID;
         active_engine_instance = -1;
         active_gtt = GGTT;
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
   struct bo *batch_bo = NULL;
   list_for_each_entry(struct bo, bo_entry, &bo_list, link) {
      if (bo_entry->type == BO_TYPE_BATCH) {
         batch_bo = bo_entry;
         break;
      }
   }
   fail_if(!batch_bo, "Failed to find batch buffer.\n");

   /* Add all the BOs to the aub file */
   struct bo *hwsp_bo = NULL;
   list_for_each_entry(struct bo, bo_entry, &bo_list, link) {
      switch (bo_entry->type) {
      case BO_TYPE_BATCH:
         if (bo_entry->gtt == PPGTT) {
            aub_map_ppgtt(&aub, bo_entry->addr, bo_entry->size);
            aub_write_trace_block(&aub, AUB_TRACE_TYPE_BATCH,
                                  bo_entry->data, bo_entry->size, bo_entry->addr);
         } else
            aub_write_ggtt(&aub, bo_entry->addr, bo_entry->size, bo_entry->data);
         break;
      case BO_TYPE_USER:
         if (bo_entry->gtt == PPGTT) {
            aub_map_ppgtt(&aub, bo_entry->addr, bo_entry->size);
            aub_write_trace_block(&aub, AUB_TRACE_TYPE_NOTYPE,
                                  bo_entry->data, bo_entry->size, bo_entry->addr);
         } else
            aub_write_ggtt(&aub, bo_entry->addr, bo_entry->size, bo_entry->data);
         break;
      case BO_TYPE_CONTEXT:
         if (bo_entry->engine_class == batch_bo->engine_class &&
             bo_entry->engine_instance == batch_bo->engine_instance &&
             aub_use_execlists(&aub)) {
            hwsp_bo = bo_entry;

            uint32_t *context = (uint32_t *) (bo_entry->data + 4096 /* GuC */ + 4096 /* HWSP */);

            if (context[1] == 0) {
               fprintf(stderr,
                       "Invalid context image data.\n"
                       "This is likely a kernel issue : https://bugs.freedesktop.org/show_bug.cgi?id=107691\n");
            }

            /* Update the ring buffer at the last known location. */
            context[5] = engines[bo_entry->engine_class].instances[bo_entry->engine_instance].ring_buffer_head;
            context[7] = engines[bo_entry->engine_class].instances[bo_entry->engine_instance].ring_buffer_tail;
            fprintf(stdout, "engine start=0x%x head/tail=0x%x/0x%x\n",
                    context[9], context[5], context[7]);

            /* The error state doesn't provide a dump of the page tables, so
             * we have to provide our own, that's easy enough.
             */
            context[49] = aub.pml4.phys_addr >> 32;
            context[51] = aub.pml4.phys_addr & 0xffffffff;

            fprintf(stdout, "context dump:\n");
            for (int i = 0; i < 60; i++) {
               if (i % 4 == 0)
                  fprintf(stdout, "\n 0x%08" PRIx64 ": ", bo_entry->addr + 8192 + i * 4);
               fprintf(stdout, "0x%08x ", context[i]);
            }
            fprintf(stdout, "\n");

         }
         aub_write_ggtt(&aub, bo_entry->addr, bo_entry->size, bo_entry->data);
         break;
      case BO_TYPE_RINGBUFFER:
      case BO_TYPE_STATUS:
      case BO_TYPE_CONTEXT_WA:
         aub_write_ggtt(&aub, bo_entry->addr, bo_entry->size, bo_entry->data);
         break;
      case BO_TYPE_UNKNOWN:
         if (bo_entry->gtt == PPGTT) {
            aub_map_ppgtt(&aub, bo_entry->addr, bo_entry->size);
            if (bo_entry->data) {
               aub_write_trace_block(&aub, AUB_TRACE_TYPE_NOTYPE,
                                     bo_entry->data, bo_entry->size, bo_entry->addr);
            }
         } else {
            if (bo_entry->size > 0) {
               void *zero_data = calloc(1, bo_entry->size);
               aub_write_ggtt(&aub, bo_entry->addr, bo_entry->size, zero_data);
               free(zero_data);
            }
         }
         break;
      default:
         break;
      }
   }

   if (aub_use_execlists(&aub)) {
      fail_if(!hwsp_bo, "Failed to find Context buffer.\n");
      aub_write_context_execlists(&aub, hwsp_bo->addr + 4096 /* skip GuC page */, hwsp_bo->engine_class);
   } else {
      /* Use context id 0 -- if we are not using execlists it doesn't matter
       * anyway
       */
      aub_write_exec(&aub, 0, batch_bo->addr, 0, INTEL_ENGINE_CLASS_RENDER);
   }

   /* Cleanup */
   list_for_each_entry_safe(struct bo, bo_entry, &bo_list, link) {
      list_del(&bo_entry->link);
      free(bo_entry->data);
      free(bo_entry);
   }

   free(out_filename);
   free(line);
   if(err_file) {
      fclose(err_file);
   }
   if(aub.file) {
      aub_file_finish(&aub);
   } else if(aub_file) {
      fclose(aub_file);
   }
   return EXIT_SUCCESS;
}

/* vim: set ts=8 sw=8 tw=0 cino=:0,(0 noet :*/
