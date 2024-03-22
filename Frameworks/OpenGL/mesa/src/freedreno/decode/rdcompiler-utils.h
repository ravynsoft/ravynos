/*
 * Copyright © 2022 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "redump.h"

#include "util/u_math.h"

#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "freedreno_pm4.h"

#include "a6xx.xml.h"

#include "ir3/ir3_assembler.h"
#include "ir3/ir3_compiler.h"
#include "ir3/ir3_shader.h"

#include "util/list.h"
#include "util/vma.h"

struct cmdstream {
   struct list_head link;

   uint32_t *mem;
   uint32_t total_size;
   uint32_t cur;

   uint64_t iova;
};

static uint64_t
cs_get_cur_iova(struct cmdstream *cs)
{
   return cs->iova + cs->cur * sizeof(uint32_t);
}

struct wrbuf {
   struct list_head link;

   uint64_t iova;
   uint64_t size;
   const char *name;
};

struct replay_context {
   void *mem_ctx;

   struct util_vma_heap vma;

   struct cmdstream *submit_cs;
   struct cmdstream *state_cs;
   struct cmdstream *shader_cs;

   struct cmdstream *shader_log;
   struct cmdstream *cp_log;

   struct list_head cs_list;

   struct list_head wrbuf_list;

   struct ir3_compiler *compiler;

   struct hash_table_u64 *compiled_shaders;

   const char *output_name;
};

static void
pkt(struct cmdstream *cs, uint32_t payload)
{
   assert(cs->cur <= cs->total_size);
   cs->mem[cs->cur++] = payload;
}

static void
pkt_qw(struct cmdstream *cs, uint64_t payload)
{
   pkt(cs, payload);
   pkt(cs, payload >> 32);
}

static uint64_t
pkt_blob(struct cmdstream *cs, void *payload, uint32_t size, uint32_t alignment)
{
   cs->cur = align(cs->cur, alignment / sizeof(uint32_t));
   uint64_t start_iova = cs_get_cur_iova(cs);

   memcpy(cs->mem + cs->cur, payload, size);
   cs->cur += size;

   return start_iova;
}

static void
pkt4(struct cmdstream *cs, uint16_t regindx, uint16_t cnt, uint32_t payload)
{
   pkt(cs, pm4_pkt4_hdr(regindx, cnt));
   pkt(cs, payload);
}

static void
pkt7(struct cmdstream *cs, uint8_t opcode, uint16_t cnt)
{
   pkt(cs, pm4_pkt7_hdr(opcode, cnt));
}

struct rd_section {
   uint32_t type;
   uint32_t size;
};

static struct cmdstream *
cs_alloc(struct replay_context *ctx, uint32_t size)
{
   struct cmdstream *cs = (struct cmdstream *) calloc(1, sizeof(struct cmdstream));
   cs->mem = (uint32_t *)calloc(1, size);
   cs->total_size = size / sizeof(uint32_t);
   cs->cur = 0;
   cs->iova = util_vma_heap_alloc(&ctx->vma, size, 4096);

   assert(cs->iova != 0);

   list_addtail(&cs->link, &ctx->cs_list);

   return cs;
}

static void
rd_write_gpu_addr_section(FILE *out, struct cmdstream *cs, enum rd_sect_type section)
{
   const uint32_t packet[] = {(uint32_t)cs->iova,
                              (uint32_t)(cs->cur * sizeof(uint32_t)),
                              (uint32_t)(cs->iova >> 32)};
   struct rd_section section_address = {.type = section,
                                        .size = sizeof(packet)};
   fwrite(&section_address, sizeof(section_address), 1, out);
   fwrite(packet, sizeof(packet), 1, out);
}

static void
rd_write_cs_buffer(FILE *out, struct cmdstream *cs)
{
   if (cs->cur == 0)
      return;

   rd_write_gpu_addr_section(out, cs, RD_GPUADDR);

   struct rd_section section_contents = {.type = RD_BUFFER_CONTENTS,
                                         .size = uint32_t(cs->cur * sizeof(uint32_t))};

   fwrite(&section_contents, sizeof(section_contents), 1, out);
   fwrite(cs->mem, sizeof(uint32_t), cs->cur, out);
}

static void
rd_write_cs_submit(FILE *out, struct cmdstream *cs)
{
   const uint32_t packet[] = {(uint32_t)cs->iova, cs->cur,
                              (uint32_t)(cs->iova >> 32)};
   struct rd_section section_cmdstream = {.type = RD_CMDSTREAM_ADDR,
                                          .size = sizeof(packet)};

   fwrite(&section_cmdstream, sizeof(section_cmdstream), 1, out);
   fwrite(packet, sizeof(packet), 1, out);
}

static void
rd_write_wrbuffer(FILE *out, struct wrbuf *wrbuf)
{
   uint32_t name_len = strlen(wrbuf->name) + 1;
   struct rd_section section = {.type = RD_WRBUFFER,
                                .size = (uint32_t)(sizeof(uint32_t) * 2) + name_len};
   fwrite(&section, sizeof(section), 1, out);
   fwrite(&wrbuf->iova, sizeof(uint64_t), 1, out);
   fwrite(&wrbuf->size, sizeof(uint64_t), 1, out);
   fwrite(wrbuf->name, sizeof(char), name_len, out);
}

static void
print_usage(const char *name)
{
   /* clang-format off */
   fprintf(stderr, "Usage:\n\n"
           "\t%s [OPTSIONS]... FILE...\n\n"
           "Options:\n"
           "\t    --vastart=offset\n"
           "\t    --vasize=size\n"
           "\t-h, --help             - show this message\n"
           , name);
   /* clang-format on */
   exit(2);
}

#define OPT_VA_START 1000
#define OPT_VA_SIZE  1001

/* clang-format off */
static const struct option opts[] = {
      { "vastart",  required_argument, 0, OPT_VA_START },
      { "vasize",   required_argument, 0, OPT_VA_SIZE },
      { "help",     no_argument,       0, 'h' },
};
/* clang-format on */

static void
replay_context_init(struct replay_context *ctx, struct fd_dev_id *dev_id,
                    int argc, char **argv)
{
   uint64_t va_start = 0;
   uint64_t va_size = 0;

   int c;
   while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1) {
      switch (c) {
      case OPT_VA_START:
         va_start = strtoull(optarg, NULL, 0);
         break;
      case OPT_VA_SIZE:
         va_size = strtoull(optarg, NULL, 0);
         break;
      case 'h':
      default:
         print_usage(argv[0]);
      }
   }

   if (optind < argc) {
      ctx->output_name = argv[optind];
   } else {
   }

   if (!va_start || !va_size || !ctx->output_name) {
      print_usage(argv[0]);
      exit(1);
   }

   ctx->mem_ctx = ralloc_context(NULL);
   list_inithead(&ctx->cs_list);
   list_inithead(&ctx->wrbuf_list);

   util_vma_heap_init(&ctx->vma, va_start, ROUND_DOWN_TO(va_size, 4096));

   ctx->submit_cs = cs_alloc(ctx, 1024 * 1024);
   ctx->state_cs = cs_alloc(ctx, 2 * 1024 * 1024);
   ctx->shader_cs = cs_alloc(ctx, 8 * 1024 * 1024);

   ctx->shader_log = cs_alloc(ctx, 1024 * 1024);
   ctx->shader_log->mem[0] = (ctx->shader_log->iova & 0xffffffff) + sizeof(uint64_t);
   ctx->shader_log->mem[1] = ctx->shader_log->iova >> 32;
   ctx->shader_log->cur = ctx->shader_log->total_size;

   ctx->cp_log = cs_alloc(ctx, 8 * 1024 * 1024);
   ((uint64_t *)ctx->cp_log->mem)[0] = ctx->cp_log->iova + 2 * sizeof(uint64_t);
   ((uint64_t *)ctx->cp_log->mem)[1] = sizeof(uint64_t);
   ctx->cp_log->cur = ctx->cp_log->total_size;

   struct ir3_compiler_options options{};
   ctx->compiler =
      ir3_compiler_create(NULL, dev_id, fd_dev_info_raw(dev_id), &options);
   ctx->compiled_shaders = _mesa_hash_table_u64_create(ctx->mem_ctx);
}

static void
replay_context_finish(struct replay_context *ctx)
{
   FILE *out = fopen(ctx->output_name, "w");
   if (!out) {
      errx(1, "Cannot open '%s' for writing\n", ctx->output_name);
   }

   static const uint32_t gpu_id = 660;
   struct rd_section section_gpu_id = {.type = RD_GPU_ID,
                                       .size = 1 * sizeof(uint32_t)};
   fwrite(&section_gpu_id, sizeof(section_gpu_id), 1, out);
   fwrite(&gpu_id, sizeof(uint32_t), 1, out);

   rd_write_gpu_addr_section(out, ctx->shader_log, RD_SHADER_LOG_BUFFER);
   rd_write_gpu_addr_section(out, ctx->cp_log, RD_CP_LOG_BUFFER);

   list_for_each_entry (struct cmdstream, cs, &ctx->cs_list, link) {
      rd_write_cs_buffer(out, cs);
   }
   rd_write_cs_submit(out, ctx->submit_cs);

   list_for_each_entry (struct wrbuf, wrbuf, &ctx->wrbuf_list, link) {
      rd_write_wrbuffer(out, wrbuf);
   }

   fclose(out);
}

static void
upload_shader(struct replay_context *ctx, uint64_t id, const char *source)
{
   FILE *in = fmemopen((void *)source, strlen(source), "r");

   struct ir3_kernel_info info = {
      .shader_print_buffer_iova = ctx->shader_log->iova,
   };
   struct ir3_shader *shader = ir3_parse_asm(ctx->compiler, &info, in);
   assert(shader);

   fclose(in);

   uint64_t *shader_iova = ralloc(ctx->mem_ctx, uint64_t);
   *shader_iova = pkt_blob(ctx->shader_cs, shader->variants->bin,
                           shader->variants->info.size, 128);
   ralloc_free(shader);

   _mesa_hash_table_u64_insert(ctx->compiled_shaders, id, shader_iova);
}

static void
emit_shader_iova(struct replay_context *ctx, struct cmdstream *cs, uint64_t id)
{
   uint64_t *shader_iova = (uint64_t *)
      _mesa_hash_table_u64_search(ctx->compiled_shaders, id);
   pkt_qw(cs, *shader_iova);
}

#define begin_draw_state()                                                     \
   uint64_t subcs_iova_start = cs_get_cur_iova(ctx.state_cs);                  \
   struct cmdstream *prev_cs = cs;                                             \
   struct cmdstream *cs = ctx.state_cs;

#define end_draw_state(params)                                                 \
   uint64_t subcs_iova_end = cs_get_cur_iova(ctx.state_cs);                    \
   uint32_t subcs_size =                                                       \
      (subcs_iova_end - subcs_iova_start) / sizeof(uint32_t);                  \
   pkt7(prev_cs, CP_SET_DRAW_STATE, 3);                                        \
   pkt(prev_cs, (params) | subcs_size);                                        \
   pkt_qw(prev_cs, subcs_iova_start);

#define begin_ib()                                                             \
   struct cmdstream *prev_cs = cs;                                             \
   struct cmdstream *cs = cs_alloc(&ctx, 1024 * 1024);

#define end_ib()                                                               \
   uint64_t ibcs_size = cs->cur;                                               \
   pkt7(prev_cs, CP_INDIRECT_BUFFER, 3);                                       \
   pkt_qw(prev_cs, cs->iova);                                                  \
   pkt(prev_cs, ibcs_size);

static void
gpu_print(struct replay_context *ctx, struct cmdstream *_cs, uint64_t iova,
          uint32_t dwords)
{
   uint64_t header_iova, body_iova;
   struct cmdstream *prev_cs = _cs;
   struct cmdstream *cs = cs_alloc(ctx, 4096);
   /* Commands that are being modified should be in a separate cmdstream,
    * otherwise they would be prefetched and writes would not be visible.
    */
   {
      /* Write size into entry's header */
      pkt7(cs, CP_MEM_WRITE, 4);
      header_iova = cs_get_cur_iova(cs);
      pkt_qw(cs, 0xdeadbeef);
      uint64_t size_iova = cs_get_cur_iova(cs);
      pkt(cs, dwords * 4);
      pkt(cs, 0);

      /* Copy the data into entry's body */
      pkt7(cs, CP_MEMCPY, 5);
      pkt(cs, dwords);
      pkt_qw(cs, iova);
      body_iova = cs_get_cur_iova(cs);
      pkt_qw(cs, 0xdeadbeef);

      /* iova = iova + body_size + header_size */
      pkt7(cs, CP_MEM_TO_MEM, 9);
      pkt(cs, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_WAIT_FOR_MEM_WRITES);
      pkt_qw(cs, ctx->cp_log->iova);
      pkt_qw(cs, ctx->cp_log->iova);
      pkt_qw(cs, size_iova);
      pkt_qw(cs, ctx->cp_log->iova + sizeof(uint64_t));
   }

   {
      struct cmdstream *cs = prev_cs;
      pkt7(cs, CP_MEM_TO_MEM, 5);
      pkt(cs, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_WAIT_FOR_MEM_WRITES);
      pkt_qw(cs, header_iova);
      pkt_qw(cs, ctx->cp_log->iova);

      pkt7(cs, CP_MEM_TO_MEM, 7);
      pkt(cs, CP_MEM_TO_MEM_0_DOUBLE);
      pkt_qw(cs, body_iova);
      pkt_qw(cs, ctx->cp_log->iova);
      pkt_qw(cs, ctx->cp_log->iova + sizeof(uint64_t));

      pkt7(cs, CP_WAIT_MEM_WRITES, 0);
      pkt7(cs, CP_WAIT_FOR_ME, 0);
   }

   end_ib();
}

static void
gpu_read_into_file(struct replay_context *ctx, struct cmdstream *_cs,
                    uint64_t iova, uint64_t size, const char *name)
{
   struct wrbuf *wrbuf = (struct wrbuf *) calloc(1, sizeof(struct wrbuf));
   wrbuf->iova = iova;
   wrbuf->size = size;
   wrbuf->name = strdup(name);

   assert(wrbuf->iova != 0);

   list_addtail(&wrbuf->link, &ctx->wrbuf_list);
}