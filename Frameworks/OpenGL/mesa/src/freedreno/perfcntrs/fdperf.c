/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <curses.h>
#include <err.h>
#include <inttypes.h>
#include <libconfig.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>

#include "drm/freedreno_drmif.h"
#include "drm/freedreno_ringbuffer.h"

#include "util/os_file.h"

#include "freedreno_dt.h"
#include "freedreno_perfcntr.h"

#define MAX_CNTR_PER_GROUP 24
#define REFRESH_MS         500

static struct {
   int refresh_ms;
   bool dump;
} options = {
   .refresh_ms = REFRESH_MS,
   .dump = false,
};

/* NOTE first counter group should always be CP, since we unconditionally
 * use CP counter to measure the gpu freq.
 */

struct counter_group {
   const struct fd_perfcntr_group *group;

   struct {
      const struct fd_perfcntr_counter *counter;
      uint16_t select_val;
      volatile uint32_t *val_hi;
      volatile uint32_t *val_lo;
   } counter[MAX_CNTR_PER_GROUP];

   /* last sample time: */
   uint32_t stime[MAX_CNTR_PER_GROUP];
   /* for now just care about the low 32b value.. at least then we don't
    * have to really care that we can't sample both hi and lo regs at the
    * same time:
    */
   uint32_t last[MAX_CNTR_PER_GROUP];
   /* current value, ie. by how many did the counter increase in last
    * sampling period divided by the sampling period:
    */
   float current[MAX_CNTR_PER_GROUP];
   /* name of currently selected counters (for UI): */
   const char *label[MAX_CNTR_PER_GROUP];
};

static struct {
   void *io;
   uint32_t min_freq;
   uint32_t max_freq;
   /* per-generation table of counters: */
   unsigned ngroups;
   struct counter_group *groups;
   /* drm device (for writing select regs via ring): */
   struct fd_device *dev;
   struct fd_pipe *pipe;
   const struct fd_dev_id *dev_id;
   struct fd_submit *submit;
   struct fd_ringbuffer *ring;
} dev;

static void config_save(void);
static void config_restore(void);
static void restore_counter_groups(void);

/*
 * helpers
 */

static uint32_t
gettime_us(void)
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

static void
sleep_us(uint32_t us)
{
   const struct timespec ts = {
      .tv_sec = us / 1000000,
      .tv_nsec = (us % 1000000) * 1000,
   };
   clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

static uint32_t
delta(uint32_t a, uint32_t b)
{
   /* deal with rollover: */
   if (a > b)
      return 0xffffffff - a + b;
   else
      return b - a;
}

static void
find_device(void)
{
   int ret;

   dev.dev = fd_device_open();
   if (!dev.dev)
      err(1, "could not open drm device");

   dev.pipe = fd_pipe_new(dev.dev, FD_PIPE_3D);

   dev.dev_id = fd_pipe_dev_id(dev.pipe);
   if (!fd_dev_info_raw(dev.dev_id))
      err(1, "unknown device");

   printf("device: %s\n", fd_dev_name(dev.dev_id));

   /* try MAX_FREQ first as that will work regardless of old dt
    * dt bindings vs upstream bindings:
    */
   uint64_t val;
   ret = fd_pipe_get_param(dev.pipe, FD_MAX_FREQ, &val);
   if (ret) {
      printf("falling back to parsing DT bindings for freq\n");
      if (!fd_dt_find_freqs(&dev.min_freq, &dev.max_freq))
         err(1, "could not find GPU freqs");
   } else {
      dev.min_freq = 0;
      dev.max_freq = val;
   }

   printf("min_freq=%u, max_freq=%u\n", dev.min_freq, dev.max_freq);

   dev.io = fd_dt_find_io();
   if (!dev.io) {
      err(1, "could not map device");
   }

   fd_pipe_set_param(dev.pipe, FD_SYSPROF, 1);
}

/*
 * perf-monitor
 */

static void
flush_ring(void)
{
   if (!dev.submit)
      return;

   struct fd_fence *fence = fd_submit_flush(dev.submit, -1, false);

   if (!fence)
      errx(1, "submit failed");

   fd_fence_flush(fence);
   fd_fence_del(fence);
   fd_ringbuffer_del(dev.ring);
   fd_submit_del(dev.submit);

   dev.ring = NULL;
   dev.submit = NULL;
}

static void
select_counter(struct counter_group *group, int ctr, int n)
{
   assert(n < group->group->num_countables);
   assert(ctr < group->group->num_counters);

   group->label[ctr] = group->group->countables[n].name;
   group->counter[ctr].select_val = n;

   if (!dev.submit) {
      dev.submit = fd_submit_new(dev.pipe);
      dev.ring = fd_submit_new_ringbuffer(
         dev.submit, 0x1000, FD_RINGBUFFER_PRIMARY | FD_RINGBUFFER_GROWABLE);
   }

   /* bashing select register directly while gpu is active will end
    * in tears.. so we need to write it via the ring:
    *
    * TODO it would help startup time, if gpu is loaded, to batch
    * all the initial writes and do a single flush.. although that
    * makes things more complicated for capturing inital sample value
    */
   struct fd_ringbuffer *ring = dev.ring;
   switch (fd_dev_gen(dev.dev_id)) {
   case 2:
   case 3:
   case 4:
      OUT_PKT3(ring, CP_WAIT_FOR_IDLE, 1);
      OUT_RING(ring, 0x00000000);

      if (group->group->counters[ctr].enable) {
         OUT_PKT0(ring, group->group->counters[ctr].enable, 1);
         OUT_RING(ring, 0);
      }

      if (group->group->counters[ctr].clear) {
         OUT_PKT0(ring, group->group->counters[ctr].clear, 1);
         OUT_RING(ring, 1);

         OUT_PKT0(ring, group->group->counters[ctr].clear, 1);
         OUT_RING(ring, 0);
      }

      OUT_PKT0(ring, group->group->counters[ctr].select_reg, 1);
      OUT_RING(ring, n);

      if (group->group->counters[ctr].enable) {
         OUT_PKT0(ring, group->group->counters[ctr].enable, 1);
         OUT_RING(ring, 1);
      }

      break;
   case 5:
   case 6:
      OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);

      if (group->group->counters[ctr].enable) {
         OUT_PKT4(ring, group->group->counters[ctr].enable, 1);
         OUT_RING(ring, 0);
      }

      if (group->group->counters[ctr].clear) {
         OUT_PKT4(ring, group->group->counters[ctr].clear, 1);
         OUT_RING(ring, 1);

         OUT_PKT4(ring, group->group->counters[ctr].clear, 1);
         OUT_RING(ring, 0);
      }

      OUT_PKT4(ring, group->group->counters[ctr].select_reg, 1);
      OUT_RING(ring, n);

      if (group->group->counters[ctr].enable) {
         OUT_PKT4(ring, group->group->counters[ctr].enable, 1);
         OUT_RING(ring, 1);
      }

      break;
   }

   group->last[ctr] = *group->counter[ctr].val_lo;
   group->stime[ctr] = gettime_us();
}

static void
resample_counter(struct counter_group *group, int ctr)
{
   uint32_t val = *group->counter[ctr].val_lo;
   uint32_t t = gettime_us();
   uint32_t dt = delta(group->stime[ctr], t);
   uint32_t dval = delta(group->last[ctr], val);
   group->current[ctr] = (float)dval * 1000000.0 / (float)dt;
   group->last[ctr] = val;
   group->stime[ctr] = t;
}

/* sample all the counters: */
static void
resample(void)
{
   static uint64_t last_time;
   uint64_t current_time = gettime_us();

   if ((current_time - last_time) < (options.refresh_ms * 1000 / 2))
      return;

   last_time = current_time;

   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      for (unsigned j = 0; j < group->group->num_counters; j++) {
         resample_counter(group, j);
      }
   }
}

/*
 * The UI
 */

#define COLOR_GROUP_HEADER 1
#define COLOR_FOOTER       2
#define COLOR_INVERSE      3

static int w, h;
static int ctr_width;
static int max_rows, current_cntr = 1;

static void
redraw_footer(WINDOW *win)
{
   char *footer;
   int n;

   n = asprintf(&footer, " fdperf: %s (%.2fMHz..%.2fMHz)",
                fd_dev_name(dev.dev_id), ((float)dev.min_freq) / 1000000.0,
                ((float)dev.max_freq) / 1000000.0);

   wmove(win, h - 1, 0);
   wattron(win, COLOR_PAIR(COLOR_FOOTER));
   waddstr(win, footer);
   whline(win, ' ', w - n);
   wattroff(win, COLOR_PAIR(COLOR_FOOTER));

   free(footer);
}

static void
redraw_group_header(WINDOW *win, int row, const char *name)
{
   wmove(win, row, 0);
   wattron(win, A_BOLD);
   wattron(win, COLOR_PAIR(COLOR_GROUP_HEADER));
   waddstr(win, name);
   whline(win, ' ', w - strlen(name));
   wattroff(win, COLOR_PAIR(COLOR_GROUP_HEADER));
   wattroff(win, A_BOLD);
}

static void
redraw_counter_label(WINDOW *win, int row, const char *name, bool selected)
{
   int n = strlen(name);
   assert(n <= ctr_width);
   wmove(win, row, 0);
   whline(win, ' ', ctr_width - n);
   wmove(win, row, ctr_width - n);
   if (selected)
      wattron(win, COLOR_PAIR(COLOR_INVERSE));
   waddstr(win, name);
   if (selected)
      wattroff(win, COLOR_PAIR(COLOR_INVERSE));
   waddstr(win, ": ");
}

static void
redraw_counter_value_cycles(WINDOW *win, float val)
{
   char *str;
   int x = getcurx(win);
   int valwidth = w - x;
   int barwidth, n;

   /* convert to fraction of max freq: */
   val = val / (float)dev.max_freq;

   /* figure out percentage-bar width: */
   barwidth = (int)(val * valwidth);

   /* sometimes things go over 100%.. idk why, could be
    * things running faster than base clock, or counter
    * summing up cycles in multiple cores?
    */
   barwidth = MIN2(barwidth, valwidth - 1);

   n = asprintf(&str, "%.2f%%", 100.0 * val);
   wattron(win, COLOR_PAIR(COLOR_INVERSE));
   waddnstr(win, str, barwidth);
   if (barwidth > n) {
      whline(win, ' ', barwidth - n);
      wmove(win, getcury(win), x + barwidth);
   }
   wattroff(win, COLOR_PAIR(COLOR_INVERSE));
   if (barwidth < n)
      waddstr(win, str + barwidth);
   whline(win, ' ', w - getcurx(win));

   free(str);
}

static void
redraw_counter_value_raw(WINDOW *win, float val)
{
   char *str;
   (void)asprintf(&str, "%'.2f", val);
   waddstr(win, str);
   whline(win, ' ', w - getcurx(win));
   free(str);
}

static void
redraw_counter(WINDOW *win, int row, struct counter_group *group, int ctr,
               bool selected)
{
   redraw_counter_label(win, row, group->label[ctr], selected);

   /* quick hack, if the label has "CYCLE" in the name, it is
    * probably a cycle counter ;-)
    * Perhaps add more info in rnndb schema to know how to
    * treat individual counters (ie. which are cycles, and
    * for those we want to present as a percentage do we
    * need to scale the result.. ie. is it running at some
    * multiple or divisor of core clk, etc)
    *
    * TODO it would be much more clever to get this from xml
    * Also.. in some cases I think we want to know how many
    * units the counter is counting for, ie. if a320 has 2x
    * shader as a306 we might need to scale the result..
    */
   if (strstr(group->label[ctr], "CYCLE") ||
       strstr(group->label[ctr], "BUSY") || strstr(group->label[ctr], "IDLE"))
      redraw_counter_value_cycles(win, group->current[ctr]);
   else
      redraw_counter_value_raw(win, group->current[ctr]);
}

static void
redraw(WINDOW *win)
{
   static int scroll = 0;
   int max, row = 0;

   w = getmaxx(win);
   h = getmaxy(win);

   max = h - 3;

   if ((current_cntr - scroll) > (max - 1)) {
      scroll = current_cntr - (max - 1);
   } else if ((current_cntr - 1) < scroll) {
      scroll = current_cntr - 1;
   }

   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      unsigned j = 0;

      /* NOTE skip CP the first CP counter */
      if (i == 0)
         j++;

      if (j < group->group->num_counters) {
         if ((scroll <= row) && ((row - scroll) < max))
            redraw_group_header(win, row - scroll, group->group->name);
         row++;
      }

      for (; j < group->group->num_counters; j++) {
         if ((scroll <= row) && ((row - scroll) < max))
            redraw_counter(win, row - scroll, group, j, row == current_cntr);
         row++;
      }
   }

   /* convert back to physical (unscrolled) offset: */
   row = max;

   redraw_group_header(win, row, "Status");
   row++;

   /* Draw GPU freq row: */
   redraw_counter_label(win, row, "Freq (MHz)", false);
   redraw_counter_value_raw(win, dev.groups[0].current[0] / 1000000.0);
   row++;

   redraw_footer(win);

   refresh();
}

static struct counter_group *
current_counter(int *ctr)
{
   int n = 0;

   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      unsigned j = 0;

      /* NOTE skip the first CP counter (CP_ALWAYS_COUNT) */
      if (i == 0)
         j++;

      /* account for group header: */
      if (j < group->group->num_counters) {
         /* cannot select group header.. return null to indicate this
          * main_ui():
          */
         if (n == current_cntr)
            return NULL;
         n++;
      }

      for (; j < group->group->num_counters; j++) {
         if (n == current_cntr) {
            if (ctr)
               *ctr = j;
            return group;
         }
         n++;
      }
   }

   assert(0);
   return NULL;
}

static void
counter_dialog(void)
{
   WINDOW *dialog;
   struct counter_group *group;
   int cnt = 0, current = 0, scroll;

   /* figure out dialog size: */
   int dh = h / 2;
   int dw = ctr_width + 2;

   group = current_counter(&cnt);

   /* find currently selected idx (note there can be discontinuities
    * so the selected value does not map 1:1 to current idx)
    */
   uint32_t selected = group->counter[cnt].select_val;
   for (int i = 0; i < group->group->num_countables; i++) {
      if (group->group->countables[i].selector == selected) {
         current = i;
         break;
      }
   }

   /* scrolling offset, if dialog is too small for all the choices: */
   scroll = 0;

   dialog = newwin(dh, dw, (h - dh) / 2, (w - dw) / 2);
   box(dialog, 0, 0);
   wrefresh(dialog);
   keypad(dialog, true);

   while (true) {
      int max = MIN2(dh - 2, group->group->num_countables);
      int selector = -1;

      if ((current - scroll) >= (dh - 3)) {
         scroll = current - (dh - 3);
      } else if (current < scroll) {
         scroll = current;
      }

      for (int i = 0; i < max; i++) {
         int n = scroll + i;
         wmove(dialog, i + 1, 1);
         if (n == current) {
            assert(n < group->group->num_countables);
            selector = group->group->countables[n].selector;
            wattron(dialog, COLOR_PAIR(COLOR_INVERSE));
         }
         if (n < group->group->num_countables)
            waddstr(dialog, group->group->countables[n].name);
         whline(dialog, ' ', dw - getcurx(dialog) - 1);
         if (n == current)
            wattroff(dialog, COLOR_PAIR(COLOR_INVERSE));
      }

      assert(selector >= 0);

      switch (wgetch(dialog)) {
      case KEY_UP:
         current = MAX2(0, current - 1);
         break;
      case KEY_DOWN:
         current = MIN2(group->group->num_countables - 1, current + 1);
         break;
      case KEY_LEFT:
      case KEY_ENTER:
         /* select new sampler */
         select_counter(group, cnt, selector);
         flush_ring();
         config_save();
         goto out;
      case 'q':
         goto out;
      default:
         /* ignore */
         break;
      }

      resample();
   }

out:
   wborder(dialog, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
   delwin(dialog);
}

static void
scroll_cntr(int amount)
{
   if (amount < 0) {
      current_cntr = MAX2(1, current_cntr + amount);
      if (current_counter(NULL) == NULL) {
         current_cntr = MAX2(1, current_cntr - 1);
      }
   } else {
      current_cntr = MIN2(max_rows - 1, current_cntr + amount);
      if (current_counter(NULL) == NULL)
         current_cntr = MIN2(max_rows - 1, current_cntr + 1);
   }
}

static void
main_ui(void)
{
   WINDOW *mainwin;
   uint32_t last_time = gettime_us();

   /* curses setup: */
   mainwin = initscr();
   if (!mainwin)
      goto out;

   cbreak();
   wtimeout(mainwin, options.refresh_ms);
   noecho();
   keypad(mainwin, true);
   curs_set(0);
   start_color();
   init_pair(COLOR_GROUP_HEADER, COLOR_WHITE, COLOR_GREEN);
   init_pair(COLOR_FOOTER, COLOR_WHITE, COLOR_BLUE);
   init_pair(COLOR_INVERSE, COLOR_BLACK, COLOR_WHITE);

   while (true) {
      switch (wgetch(mainwin)) {
      case KEY_UP:
         scroll_cntr(-1);
         break;
      case KEY_DOWN:
         scroll_cntr(+1);
         break;
      case KEY_NPAGE: /* page-down */
         /* TODO figure out # of rows visible? */
         scroll_cntr(+15);
         break;
      case KEY_PPAGE: /* page-up */
         /* TODO figure out # of rows visible? */
         scroll_cntr(-15);
         break;
      case KEY_RIGHT:
         counter_dialog();
         break;
      case 'q':
         goto out;
         break;
      default:
         /* ignore */
         break;
      }
      resample();
      redraw(mainwin);

      /* restore the counters every 0.5s in case the GPU has suspended,
       * in which case the current selected countables will have reset:
       */
      uint32_t t = gettime_us();
      if (delta(last_time, t) > 500000) {
         restore_counter_groups();
         flush_ring();
         last_time = t;
      }
   }

   /* restore settings.. maybe we need an atexit()??*/
out:
   delwin(mainwin);
   endwin();
   refresh();
}

static void
dump_counters(void)
{
   resample();
   sleep_us(options.refresh_ms * 1000);
   resample();

   for (unsigned i = 0; i < dev.ngroups; i++) {
      const struct counter_group *group = &dev.groups[i];
      for (unsigned j = 0; j < group->group->num_counters; j++) {
         const char *label = group->label[j];
         float val = group->current[j];

         /* we did not config the first CP counter */
         if (i == 0 && j == 0)
            label = group->group->countables[0].name;

         int n = printf("%s: ", label) - 2;
         while (n++ < ctr_width)
            fputc(' ', stdout);

         if (strstr(label, "CYCLE") ||
             strstr(label, "BUSY") ||
             strstr(label, "IDLE")) {
            val = val / dev.max_freq * 100.0f;
            printf("%.2f%%\n", val);
         } else {
            printf("%'.2f\n", val);
         }
      }
   }
}

static void
restore_counter_groups(void)
{
   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      unsigned j = 0;

      /* NOTE skip CP the first CP counter */
      if (i == 0)
         j++;

      for (; j < group->group->num_counters; j++) {
         select_counter(group, j, group->counter[j].select_val);
      }
   }
}

static void
setup_counter_groups(const struct fd_perfcntr_group *groups)
{
   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];

      group->group = &groups[i];

      max_rows += group->group->num_counters + 1;

      /* the first CP counter is hidden: */
      if (i == 0) {
         max_rows--;
         if (group->group->num_counters <= 1)
            max_rows--;
      }

      for (unsigned j = 0; j < group->group->num_counters; j++) {
         group->counter[j].counter = &group->group->counters[j];

         group->counter[j].val_hi =
            dev.io + (group->counter[j].counter->counter_reg_hi * 4);
         group->counter[j].val_lo =
            dev.io + (group->counter[j].counter->counter_reg_lo * 4);

         group->counter[j].select_val = j;
      }

      for (unsigned j = 0; j < group->group->num_countables; j++) {
         ctr_width =
            MAX2(ctr_width, strlen(group->group->countables[j].name) + 1);
      }
   }
}

/*
 * configuration / persistence
 */

static config_t cfg;
static config_setting_t *setting;

static void
config_save(void)
{
   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      unsigned j = 0;

      /* NOTE skip CP the first CP counter */
      if (i == 0)
         j++;

      config_setting_t *sect =
         config_setting_get_member(setting, group->group->name);

      for (; j < group->group->num_counters; j++) {
         char name[] = "counter0000";
         sprintf(name, "counter%d", j);
         config_setting_t *s = config_setting_lookup(sect, name);
         config_setting_set_int(s, group->counter[j].select_val);
      }
   }

   config_write_file(&cfg, "fdperf.cfg");
}

static void
config_restore(void)
{
   char *str;

   config_init(&cfg);

   /* Read the file. If there is an error, report it and exit. */
   if (!config_read_file(&cfg, "fdperf.cfg")) {
      warn("could not restore settings");
   }

   config_setting_t *root = config_root_setting(&cfg);

   /* per device settings: */
   (void)asprintf(&str, "%s", fd_dev_name(dev.dev_id));
   setting = config_setting_get_member(root, str);
   if (!setting)
      setting = config_setting_add(root, str, CONFIG_TYPE_GROUP);
   free(str);
   if (!setting)
      return;

   for (unsigned i = 0; i < dev.ngroups; i++) {
      struct counter_group *group = &dev.groups[i];
      unsigned j = 0;

      /* NOTE skip CP the first CP counter */
      if (i == 0)
         j++;

      config_setting_t *sect =
         config_setting_get_member(setting, group->group->name);

      if (!sect) {
         sect =
            config_setting_add(setting, group->group->name, CONFIG_TYPE_GROUP);
      }

      for (; j < group->group->num_counters; j++) {
         char name[] = "counter0000";
         sprintf(name, "counter%d", j);
         config_setting_t *s = config_setting_lookup(sect, name);
         if (!s) {
            config_setting_add(sect, name, CONFIG_TYPE_INT);
            continue;
         }
         select_counter(group, j, config_setting_get_int(s));
      }
   }
}

static void
print_usage(const char *argv0)
{
   fprintf(stderr,
           "Usage: %s [OPTION]...\n"
           "\n"
           "  -r <N>     refresh every N milliseconds\n"
           "  -d         dump counters and exit\n"
           "  -h         show this message\n",
           argv0);
   exit(2);
}

static void
parse_options(int argc, char **argv)
{
   int c;

   while ((c = getopt(argc, argv, "r:d")) != -1) {
      switch (c) {
      case 'r':
         options.refresh_ms = atoi(optarg);
         break;
      case 'd':
         options.dump = true;
         break;
      default:
         print_usage(argv[0]);
         break;
      }
   }
}

/*
 * main
 */

int
main(int argc, char **argv)
{
   parse_options(argc, argv);

   find_device();

   const struct fd_perfcntr_group *groups;
   groups = fd_perfcntrs(dev.dev_id, &dev.ngroups);
   if (!groups) {
      errx(1, "no perfcntr support");
   }

   dev.groups = calloc(dev.ngroups, sizeof(struct counter_group));

   setlocale(LC_NUMERIC, "en_US.UTF-8");

   setup_counter_groups(groups);
   restore_counter_groups();
   config_restore();
   flush_ring();

   if (options.dump)
      dump_counters();
   else
      main_ui();

   return 0;
}
