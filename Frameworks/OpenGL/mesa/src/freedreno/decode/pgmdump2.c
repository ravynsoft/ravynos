/*
 * Copyright (c) 2018 Rob Clark <robdclark@gmail.com>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Decoder for "new" GL_OES_get_program_binary format.
 *
 * Overall structure is:
 *
 *   - header at top, contains, amongst other things, offsets of
 *     per shader stage sections.
 *   - per shader stage section (shader_info) starts with a header,
 *     followed by a variably length list of descriptors.  Each
 *     descriptor has a type/count/size plus offset from the start
 *     of shader_info section where the data is found
 */

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "disasm.h"
#include "io.h"
#include "redump.h"
#include "util.h"

const char *infile;
static int dump_full = 0;
static int dump_offsets = 0;
static int gpu_id = 320;
static int shaderdb = 0; /* output shaderdb style traces to stderr */

struct state {
   char *buf;
   int sz;
   int lvl;

   /* current shader_info section, some offsets calculated relative to
    * this, rather than relative to start of buffer.
    */
   void *shader;

   /* size of each entry within a shader_descriptor_blk: */
   int desc_size;

   const char *shader_type;
   int full_regs;
   int half_regs;
};

#define PACKED __attribute__((__packed__))

#define OFF(field)                                                             \
   do {                                                                        \
      if (dump_offsets)                                                        \
         printf("%08x: ", (uint32_t)((char *)&field - state->buf));            \
   } while (0)

/* decode field as hex */
#define X(s, field)                                                            \
   do {                                                                        \
      OFF(s->field);                                                           \
      printf("%s%12s:\t0x%x\n", tab(state->lvl), #field, s->field);            \
   } while (0)

/* decode field as digit */
#define D(s, field)                                                            \
   do {                                                                        \
      OFF(s->field);                                                           \
      printf("%s%12s:\t%u\n", tab(state->lvl), #field, s->field);              \
   } while (0)

/* decode field as float/hex */
#define F(s, field)                                                            \
   do {                                                                        \
      OFF(s->field);                                                           \
      printf("%s%12s:\t%f (0x%0x)\n", tab(state->lvl), #field, uif(s->field),  \
             s->field);                                                        \
   } while (0)

/* decode field as register: (type is 'r' or 'c') */
#define R(s, field, type)                                                      \
   do {                                                                        \
      OFF(s->field);                                                           \
      printf("%s%12s:\t%c%u.%c\n", tab(state->lvl), #field, type,              \
             (s->field >> 2), "xyzw"[s->field & 0x3]);                         \
   } while (0)

/* decode inline string (presumably null terminated?) */
#define S(s, field)                                                            \
   do {                                                                        \
      OFF(s->field);                                                           \
      printf("%s%12s:\t%s\n", tab(state->lvl), #field, s->field);              \
   } while (0)

/* decode string-table string */
#define T(s, field) TODO

/* decode field as unknown */
#define U(s, start, end)                                                       \
   dump_unknown(state, s->unk_##start##_##end, 0x##start,                      \
                (4 + 0x##end - 0x##start) / 4)

/* decode field as offset to other section */
#define O(s, field, type)                                                      \
   do {                                                                        \
      X(s, field);                                                             \
      assert(s->field < state->sz);                                            \
      void *_p = &state->buf[s->field];                                        \
      state->lvl++;                                                            \
      decode_##type(state, _p);                                                \
      state->lvl--;                                                            \
   } while (0)

struct shader_info;
static void decode_shader_info(struct state *state, struct shader_info *info);

static void
dump_unknown(struct state *state, void *buf, unsigned start, unsigned n)
{
   uint32_t *ptr = buf;
   uint8_t *ascii = buf;

   for (unsigned i = 0; i < n; i++) {
      uint32_t d = ptr[i];

      if (dump_offsets)
         printf("%08x:", (uint32_t)((char *)&ptr[i] - state->buf));

      printf("%s        %04x:\t%08x", tab(state->lvl), start + i * 4, d);

      printf("\t|");
      for (unsigned j = 0; j < 4; j++) {
         uint8_t c = *(ascii++);
         printf("%c", (isascii(c) && !iscntrl(c)) ? c : '.');
      }
      printf("|\t%f", uif(d));

      /* TODO maybe scan for first non-null and non-ascii char starting from
       * end of shader binary to (roughly) establish the start of the string
       * table.. that would be a bit better filter for deciding if something
       * might be a pointer into the string table.  Also, the previous char
       * to what it points to should probably be null.
       */
      if ((d < state->sz) && isascii(state->buf[d]) &&
          (strlen(&state->buf[d]) > 2) && isascii(state->buf[d + 1]))
         printf("\t<== %s", &state->buf[d]);

      printf("\n");
   }
}

struct PACKED header {
   uint32_t version; /* I guess, always b10bcace ? */
   uint32_t unk_0004_0014[5];
   uint32_t size;
   uint32_t size2; /* just to be sure? */
   uint32_t unk_0020_0020[1];
   uint32_t
      chksum; /* I guess?  Small changes seem to result in big diffs here */
   uint32_t unk_0028_0050[11];
   uint32_t fs_info; /* offset of FS shader_info section */
   uint32_t unk_0058_0090[15];
   uint32_t vs_info; /* offset of VS shader_info section */
   uint32_t unk_0098_00b0[7];
   uint32_t vs_info2; /* offset of VS shader_info section (again?) */
   uint32_t unk_00b8_0110[23];
   uint32_t bs_info; /* offset of binning shader_info section */
};

static void
decode_header(struct state *state, struct header *hdr)
{
   X(hdr, version);
   U(hdr, 0004, 0014);
   X(hdr, size);
   X(hdr, size2);
   U(hdr, 0020, 0020);
   X(hdr, chksum);
   U(hdr, 0028, 0050);
   state->shader_type = "FRAG";
   O(hdr, fs_info, shader_info);
   U(hdr, 0058, 0090);
   state->shader_type = "VERT";
   O(hdr, vs_info, shader_info);
   U(hdr, 0098, 00b0);
   assert(hdr->vs_info ==
          hdr->vs_info2); /* not sure what this if it is ever different */
   X(hdr, vs_info2);
   U(hdr, 00b8, 0110);
   state->shader_type = "BVERT";
   O(hdr, bs_info, shader_info);

   /* not sure how much of the rest of contents before start of fs_info
    * is the header, vs other things.. just dump it all as unknown for
    * now:
    */
   dump_unknown(state, (void *)hdr + sizeof(*hdr), sizeof(*hdr),
                (hdr->fs_info - sizeof(*hdr)) / 4);
}

struct PACKED shader_entry_point {
   /* entry point name, ie. "main" of TBD length, followed by unknown */
   char name[8];
};

static void
decode_shader_entry_point(struct state *state, struct shader_entry_point *e)
{
   S(e, name);
}

struct PACKED shader_config {
   uint32_t unk_0000_0008[3];
   uint32_t full_regs;
   uint32_t half_regs;
};

static void
decode_shader_config(struct state *state, struct shader_config *cfg)
{
   U(cfg, 0000, 0008);
   D(cfg, full_regs);
   D(cfg, half_regs);

   state->full_regs = cfg->full_regs;
   state->half_regs = cfg->half_regs;

   /* dump reset of unknown (size differs btwn versions) */
   dump_unknown(state, (void *)cfg + sizeof(*cfg), sizeof(*cfg),
                (state->desc_size - sizeof(*cfg)) / 4);
}

struct PACKED shader_io_block {
   /* name of TBD length followed by unknown.. 42 dwords total */
   char name[20];
   uint32_t unk_0014_00a4[37];
};

static void
decode_shader_io_block(struct state *state, struct shader_io_block *io)
{
   S(io, name);
   U(io, 0014, 00a4);
}

struct PACKED shader_constant_block {
   uint32_t value;
   uint32_t unk_0004_000c[3];
   uint32_t regid;
   uint32_t unk_0014_0024[5];
};

static void
decode_shader_constant_block(struct state *state,
                             struct shader_constant_block *c)
{
   F(c, value);
   U(c, 0004, 000c);
   R(c, regid, 'c');
   U(c, 0014, 0024);
}

enum {
   ENTRY_POINT = 0,   /* shader_entry_point */
   SHADER_CONFIG = 1, /* XXX placeholder name */
   SHADER_INPUT = 2,  /* shader_io_block */
   SHADER_OUTPUT = 3, /* shader_io_block */
   CONSTANTS = 6,     /* shader_constant_block */
   INTERNAL = 8,      /* internal input, like bary.f coord */
   SHADER = 10,
} shader_info_block_type;

/* Refers to location of some type of records, with an offset relative to
 * start of shader_info block.
 */
struct PACKED shader_descriptor_block {
   uint32_t type;   /* block type */
   uint32_t offset; /* offset (relative to start of shader_info block) */
   uint32_t size;   /* size in bytes */
   uint32_t count;  /* number of records */
   uint32_t unk_0010_0010[1];
};

static void
decode_shader_descriptor_block(struct state *state,
                               struct shader_descriptor_block *blk)
{
   D(blk, type);
   X(blk, offset);
   D(blk, size);
   D(blk, count);
   U(blk, 0010, 0010);

   /* offset relative to current shader block: */
   void *ptr = state->shader + blk->offset;

   if (blk->count == 0) {
      assert(blk->size == 0);
   } else {
      assert((blk->size % blk->count) == 0);
   }

   state->desc_size = blk->size / blk->count;
   state->lvl++;
   for (unsigned i = 0; i < blk->count; i++) {
      switch (blk->type) {
      case ENTRY_POINT:
         printf("%sentry point %u:\n", tab(state->lvl - 1), i);
         decode_shader_entry_point(state, ptr);
         break;
      case SHADER_CONFIG:
         printf("%sconfig %u:\n", tab(state->lvl - 1), i);
         decode_shader_config(state, ptr);
         break;
      case SHADER_INPUT:
         printf("%sinput %u:\n", tab(state->lvl - 1), i);
         decode_shader_io_block(state, ptr);
         break;
      case SHADER_OUTPUT:
         printf("%soutput %u:\n", tab(state->lvl - 1), i);
         decode_shader_io_block(state, ptr);
         break;
      case INTERNAL:
         printf("%sinternal input %u:\n", tab(state->lvl - 1), i);
         decode_shader_io_block(state, ptr);
         break;
      case CONSTANTS:
         printf("%sconstant %u:\n", tab(state->lvl - 1), i);
         decode_shader_constant_block(state, ptr);
         break;
      case SHADER: {
         struct shader_stats stats;
         printf("%sshader %u:\n", tab(state->lvl - 1), i);
         disasm_a3xx_stat(ptr, blk->size / 4, state->lvl, stdout, gpu_id,
                          &stats);
         if (shaderdb) {
            unsigned dwords = 2 * stats.instlen;

            if (gpu_id >= 400) {
               dwords = ALIGN(dwords, 16 * 2);
            } else {
               dwords = ALIGN(dwords, 4 * 2);
            }

            unsigned half_regs = state->half_regs;
            unsigned full_regs = state->full_regs;

            /* On a6xx w/ merged/conflicting half and full regs, the
             * full_regs footprint will be max of full_regs and half
             * of half_regs.. we only care about which value is higher.
             */
            if (gpu_id >= 600) {
               /* footprint of half_regs in units of full_regs: */
               unsigned half_full = (half_regs + 1) / 2;
               if (half_full > full_regs)
                  full_regs = half_full;
               half_regs = 0;
            }

            fprintf(stderr,
                    "%s shader: %u inst, %u nops, %u non-nops, %u dwords, "
                    "%u half, %u full, %u constlen, "
                    "%u (ss), %u (sy), %d max_sun, %d loops\n",
                    state->shader_type, stats.instructions, stats.nops,
                    stats.instructions - stats.nops, dwords, half_regs,
                    full_regs, stats.constlen, stats.ss, stats.sy, 0,
                    0); /* max_sun or loops not possible */
         }
         /* this is a special case in a way, blk->count is # of
          * instructions but disasm_a3xx() decodes all instructions,
          * so just bail.
          */
         i = blk->count;
         break;
      }
      default:
         dump_unknown(state, ptr, 0, state->desc_size / 4);
         break;
      }
      ptr += state->desc_size;
   }
   state->lvl--;
}

/* there looks like one of these per shader, followed by "main" and
 * some more info, and then the shader itself.
 */
struct PACKED shader_info {
   uint32_t unk_0000_0010[5];
   uint32_t desc_off; /* offset to first descriptor block */
   uint32_t num_blocks;
};

static void
decode_shader_info(struct state *state, struct shader_info *info)
{
   assert((info->desc_off % 4) == 0);

   U(info, 0000, 0010);
   X(info, desc_off);
   D(info, num_blocks);

   dump_unknown(state, &info[1], 0, (info->desc_off - sizeof(*info)) / 4);

   state->shader = info;

   struct shader_descriptor_block *blocks = ((void *)info) + info->desc_off;
   for (unsigned i = 0; i < info->num_blocks; i++) {
      printf("%sdescriptor %u:\n", tab(state->lvl), i);
      state->lvl++;
      decode_shader_descriptor_block(state, &blocks[i]);
      state->lvl--;
   }
}

static void
dump_program(struct state *state)
{
   struct header *hdr = (void *)state->buf;

   if (dump_full)
      dump_unknown(state, state->buf, 0, state->sz / 4);

   decode_header(state, hdr);
}

int
main(int argc, char **argv)
{
   enum rd_sect_type type = RD_NONE;
   enum debug_t debug = PRINT_RAW | PRINT_STATS;
   void *buf = NULL;
   int sz;
   struct io *io;
   int raw_program = 0;

   /* lame argument parsing: */

   while (1) {
      if ((argc > 1) && !strcmp(argv[1], "--verbose")) {
         debug |= PRINT_RAW | PRINT_VERBOSE;
         argv++;
         argc--;
         continue;
      }
      if ((argc > 1) && !strcmp(argv[1], "--expand")) {
         debug |= EXPAND_REPEAT;
         argv++;
         argc--;
         continue;
      }
      if ((argc > 1) && !strcmp(argv[1], "--full")) {
         /* only short dump, original shader, symbol table, and disassembly */
         dump_full = 1;
         argv++;
         argc--;
         continue;
      }
      if ((argc > 1) && !strcmp(argv[1], "--dump-offsets")) {
         dump_offsets = 1;
         argv++;
         argc--;
         continue;
      }
      if ((argc > 1) && !strcmp(argv[1], "--raw")) {
         raw_program = 1;
         argv++;
         argc--;
         continue;
      }
      if ((argc > 1) && !strcmp(argv[1], "--shaderdb")) {
         shaderdb = 1;
         argv++;
         argc--;
         continue;
      }
      break;
   }

   if (argc != 2) {
      fprintf(stderr, "usage: pgmdump2 [--verbose] [--expand] [--full] "
                      "[--dump-offsets] [--raw] [--shaderdb] testlog.rd\n");
      return -1;
   }

   disasm_a3xx_set_debug(debug);

   infile = argv[1];

   io = io_open(infile);
   if (!io) {
      fprintf(stderr, "could not open: %s\n", infile);
      return -1;
   }

   if (raw_program) {
      io_readn(io, &sz, 4);
      free(buf);

      /* note: allow hex dumps to go a bit past the end of the buffer..
       * might see some garbage, but better than missing the last few bytes..
       */
      buf = calloc(1, sz + 3);
      io_readn(io, buf + 4, sz);
      (*(int *)buf) = sz;

      struct state state = {
         .buf = buf,
         .sz = sz,
      };
      printf("############################################################\n");
      printf("program:\n");
      dump_program(&state);
      printf("############################################################\n");
      return 0;
   }

   /* figure out what sort of input we are dealing with: */
   if (!(check_extension(infile, ".rd") || check_extension(infile, ".rd.gz"))) {
      int ret;
      buf = calloc(1, 100 * 1024);
      ret = io_readn(io, buf, 100 * 1024);
      if (ret < 0) {
         fprintf(stderr, "error: %m");
         return -1;
      }
      return disasm_a3xx(buf, ret / 4, 0, stdout, gpu_id);
   }

   while ((io_readn(io, &type, sizeof(type)) > 0) &&
          (io_readn(io, &sz, 4) > 0)) {
      free(buf);

      /* note: allow hex dumps to go a bit past the end of the buffer..
       * might see some garbage, but better than missing the last few bytes..
       */
      buf = calloc(1, sz + 3);
      io_readn(io, buf, sz);

      switch (type) {
      case RD_TEST:
         if (dump_full)
            printf("test: %s\n", (char *)buf);
         break;
      case RD_VERT_SHADER:
         printf("vertex shader:\n%s\n", (char *)buf);
         break;
      case RD_FRAG_SHADER:
         printf("fragment shader:\n%s\n", (char *)buf);
         break;
      case RD_PROGRAM: {
         struct state state = {
            .buf = buf,
            .sz = sz,
         };
         printf(
            "############################################################\n");
         printf("program:\n");
         dump_program(&state);
         printf(
            "############################################################\n");
         break;
      }
      case RD_GPU_ID:
         gpu_id = *((unsigned int *)buf);
         printf("gpu_id: %d\n", gpu_id);
         break;
      default:
         break;
      }
   }

   io_close(io);

   return 0;
}
