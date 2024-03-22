/*
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
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

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "buffers.h"
#include "cffdec.h"
#include "disasm.h"
#include "io.h"
#include "pager.h"
#include "redump.h"
#include "rnnutil.h"
#include "script.h"
#include "rdutil.h"

static struct cffdec_options options;

static bool needs_wfi = false;
static bool is_blob = false;
static int show_comp = false;
static int interactive;
static int vertices;
static const char *exename;

static int handle_file(const char *filename, int start, int end, int draw);

static void
print_usage(const char *name)
{
   /* clang-format off */
   fprintf(stderr, "Usage:\n\n"
           "\t%s [OPTSIONS]... FILE...\n\n"
           "Options:\n"
           "\t-v, --verbose    - more verbose disassembly\n"
           "\t--dump-shaders   - dump each shader to a raw file\n"
           "\t--no-color       - disable colorized output (default for non-console\n"
           "\t                   output)\n"
           "\t--color          - enable colorized output (default for tty output)\n"
           "\t--no-pager       - disable pager (default for non-console output)\n"
           "\t--pager          - enable pager (default for tty output)\n"
           "\t-s, --summary    - don't show individual register writes, but just\n"
           "\t                   register values on draws\n"
           "\t-a, --allregs    - show all registers (including ones not written\n"
           "\t                   since previous draw) on each draw\n"
           "\t-S, --start=N    - start decoding from frame N\n"
           "\t-E, --end=N      - stop decoding after frame N\n"
           "\t-F, --frame=N    - decode only frame N\n"
           "\t-D, --draw=N     - decode only draw N\n"
           "\t-e, --exe=NAME   - only decode cmdstream from named process\n"
           "\t--textures       - dump texture contents (if possible)\n"
           "\t-L, --script=LUA - run specified lua script to analyze state\n"
           "\t-q, --query=REG  - query mode, dump only specified query registers on\n"
           "\t                   each draw; multiple --query/-q args can be given to\n"
           "\t                   dump multiple registers; register can be specified\n"
           "\t                   either by name or numeric offset\n"
           "\t--query-all      - in query mode, show all queried regs on each draw\n"
           "\t                   (default query mode)\n"
           "\t--query-written  - in query mode, show queried regs on draws if any of\n"
           "\t                   them have been written since previous draw\n"
           "\t--query-delta    - in query mode, show queried regs on draws if any of\n"
           "\t                   them have changed since previous draw\n"
           "\t--query-compare  - dump registers for BINNING vs GMEM/BYPASS per draw;\n"
           "\t                   only applicable for regs set via SDS group (a6xx+),\n"
           "\t                   implies --once, can be combined with --query-all,\n"
           "\t                   --query-written, or --query-delta\n"
           "\t--once           - decode cmdstream only once (per draw mode); if same\n"
           "\t                   cmdstream is executed for each tile, this will decode\n"
           "\t                   it only for the first tile and skip the remainder,\n"
           "\t                   which can be useful when looking at state that does\n"
           "\t                   not change per tile\n"
           "\t--not-once       - decode cmdstream for each IB (default)\n"
           "\t--unit-test      - make reproducible output for unit testing\n"
           "\t-h, --help       - show this message\n"
           , name);
   /* clang-format on */
   exit(2);
}

/* clang-format off */
static const struct option opts[] = {
      /* Long opts that simply set a flag (no corresponding short alias: */
      { "dump-shaders",    no_argument, &options.dump_shaders,  1 },
      { "no-color",        no_argument, &options.color,         0 },
      { "color",           no_argument, &options.color,         1 },
      { "no-pager",        no_argument, &interactive,           0 },
      { "pager",           no_argument, &interactive,           1 },
      { "textures",        no_argument, &options.dump_textures, 1 },
      { "show-compositor", no_argument, &show_comp,             1 },
      { "query-all",       no_argument, &options.query_mode,    QUERY_ALL },
      { "query-written",   no_argument, &options.query_mode,    QUERY_WRITTEN },
      { "query-delta",     no_argument, &options.query_mode,    QUERY_DELTA },
      { "query-compare",   no_argument, &options.query_compare, 1 },
      { "once",            no_argument, &options.once,          1 },
      { "not-once",        no_argument, &options.once,          0 },
      { "unit-test",       no_argument, &options.unit_test,     1 },

      /* Long opts with short alias: */
      { "verbose",   no_argument,       0, 'v' },
      { "summary",   no_argument,       0, 's' },
      { "allregs",   no_argument,       0, 'a' },
      { "start",     required_argument, 0, 'S' },
      { "end",       required_argument, 0, 'E' },
      { "frame",     required_argument, 0, 'F' },
      { "draw",      required_argument, 0, 'D' },
      { "exe",       required_argument, 0, 'e' },
      { "script",    required_argument, 0, 'L' },
      { "query",     required_argument, 0, 'q' },
      { "help",      no_argument,       0, 'h' },
};
/* clang-format on */

int
main(int argc, char **argv)
{
   enum debug_t debug = PRINT_RAW | PRINT_STATS;
   int ret = -1;
   int start = 0, end = 0x7ffffff, draw = -1;
   int c;

   interactive = isatty(STDOUT_FILENO);

   options.color = interactive;

   while ((c = getopt_long(argc, argv, "vsaS:E:F:D:e:L:q:h", opts, NULL)) !=
          -1) {
      switch (c) {
      case 0:
         /* option that set a flag, nothing to do */
         break;
      case 'v':
         debug |= (PRINT_RAW | EXPAND_REPEAT | PRINT_VERBOSE);
         break;
      case 's':
         options.summary = true;
         break;
      case 'a':
         options.allregs = true;
         break;
      case 'S':
         start = atoi(optarg);
         break;
      case 'E':
         end = atoi(optarg);
         break;
      case 'F':
         start = end = atoi(optarg);
         break;
      case 'D':
         draw = atoi(optarg);
         break;
      case 'e':
         exename = optarg;
         break;
      case 'L':
         options.script = optarg;
         if (script_load(options.script)) {
            errx(-1, "error loading %s\n", options.script);
         }
         break;
      case 'q':
         options.querystrs =
            realloc(options.querystrs,
                    (options.nquery + 1) * sizeof(*options.querystrs));
         options.querystrs[options.nquery] = optarg;
         options.nquery++;
         interactive = 0;
         break;
      case 'h':
      default:
         print_usage(argv[0]);
      }
   }

   disasm_a2xx_set_debug(debug);
   disasm_a3xx_set_debug(debug);

   if (interactive) {
      pager_open();
   }

   while (optind < argc) {
      ret = handle_file(argv[optind], start, end, draw);
      if (ret) {
         fprintf(stderr, "error reading: %s\n", argv[optind]);
         fprintf(stderr, "continuing..\n");
      }
      optind++;
   }

   if (ret)
      print_usage(argv[0]);

   if ((options.query_mode || options.query_compare) && !options.nquery) {
      fprintf(stderr, "query options only valid in query mode!\n");
      print_usage(argv[0]);
   }

   script_finish();

   if (interactive) {
      pager_close();
   }

   return ret;
}

static int
handle_file(const char *filename, int start, int end, int draw)
{
   struct io *io;
   int submit = 0, got_gpu_id = 0;
   bool needs_reset = false;
   bool skip = false;
   struct rd_parsed_section ps = {0};

   options.draw_filter = draw;

   cffdec_init(&options);

   if (!options.unit_test)
      printf("Reading %s...\n", filename);

   script_start_cmdstream(filename);

   if (!strcmp(filename, "-"))
      io = io_openfd(0);
   else
      io = io_open(filename);

   if (!io) {
      fprintf(stderr, "could not open: %s\n", filename);
      return -1;
   }

   struct {
      unsigned int len;
      uint64_t gpuaddr;
   } gpuaddr = {0};

   while (parse_rd_section(io, &ps)) {
      needs_wfi = false;

      switch (ps.type) {
      case RD_TEST:
         printl(1, "test: %s\n", (char *)ps.buf);
         break;
      case RD_CMD:
         is_blob = true;
         printl(2, "cmd: %s\n", (char *)ps.buf);
         skip = false;
         if (exename) {
            skip |= (strstr(ps.buf, exename) != ps.buf);
         } else if (!show_comp) {
            skip |= (strstr(ps.buf, "fdperf") == ps.buf);
            skip |= (strstr(ps.buf, "chrome") == ps.buf);
            skip |= (strstr(ps.buf, "surfaceflinger") == ps.buf);
            skip |= ((char *)ps.buf)[0] == 'X';
         }
         break;
      case RD_VERT_SHADER:
         printl(2, "vertex shader:\n%s\n", (char *)ps.buf);
         break;
      case RD_FRAG_SHADER:
         printl(2, "fragment shader:\n%s\n", (char *)ps.buf);
         break;
      case RD_GPUADDR:
         if (needs_reset) {
            reset_buffers();
            needs_reset = false;
         }
         parse_addr(ps.buf, ps.sz, &gpuaddr.len, &gpuaddr.gpuaddr);
         break;
      case RD_BUFFER_CONTENTS:
         add_buffer(gpuaddr.gpuaddr, gpuaddr.len, ps.buf);
         ps.buf = NULL;
         break;
      case RD_CMDSTREAM_ADDR:
         if ((start <= submit) && (submit <= end)) {
            unsigned int sizedwords;
            uint64_t gpuaddr;
            parse_addr(ps.buf, ps.sz, &sizedwords, &gpuaddr);
            printl(2, "############################################################\n");
            printl(2, "cmdstream[%d]: %d dwords\n", submit, sizedwords);
            if (!skip) {
               script_start_submit();
               dump_commands(hostptr(gpuaddr), sizedwords, 0);
               script_end_submit();
            }
            printl(2, "############################################################\n");
            printl(2, "vertices: %d\n", vertices);
         }
         needs_reset = true;
         submit++;
         break;
      case RD_GPU_ID:
         if (!got_gpu_id) {
            uint32_t gpu_id = parse_gpu_id(ps.buf);
            if (!gpu_id)
               break;
            options.dev_id.gpu_id = gpu_id;
            printl(2, "gpu_id: %d\n", options.dev_id.gpu_id);

            options.info = fd_dev_info_raw(&options.dev_id);
            if (!options.info)
               break;

            cffdec_init(&options);
            got_gpu_id = 1;
         }
         break;
      case RD_CHIP_ID:
         if (!got_gpu_id) {
            options.dev_id.chip_id = parse_chip_id(ps.buf);
            printl(2, "chip_id: 0x%" PRIx64 "\n", options.dev_id.chip_id);

            options.info = fd_dev_info_raw(&options.dev_id);
            if (!options.info)
               break;

            cffdec_init(&options);
            got_gpu_id = 1;
         }
         break;
      default:
         break;
      }
   }

   script_end_cmdstream();

   reset_buffers();

   io_close(io);
   fflush(stdout);

   if (ps.ret < 0) {
      printf("corrupt file\n");
   }
   return 0;
}
