/**************************************************************************
 *
 * Copyright (C) 2016 Steven Toth <stoth@kernellabs.com>
 * Copyright (C) 2016 Zodiac Inflight Innovations
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifdef HAVE_GALLIUM_EXTRA_HUD

/* Purpose:
 * Reading /sys/devices/system/cpu/cpu?/cpufreq/scaling_???_freq
 * cpu frequency (KHz), displaying on the HUD in Hz.
 */

#include "hud/hud_private.h"
#include "util/list.h"
#include "util/os_time.h"
#include "util/simple_mtx.h"
#include "util/u_thread.h"
#include "util/u_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

struct cpufreq_info
{
   struct list_head list;
   int mode; /* CPUFREQ_MINIMUM, CPUFREQ_CURRENT, CPUFREQ_MAXIMUM */
   char name[16]; /* EG. cpu0 */
   int cpu_index;

   /* EG. /sys/devices/system/cpu/cpu?/cpufreq/scaling_cur_freq */
   char sysfs_filename[128];
   uint64_t KHz;
   uint64_t last_time;
};

static int gcpufreq_count = 0;
static struct list_head gcpufreq_list;
static simple_mtx_t gcpufreq_mutex = SIMPLE_MTX_INITIALIZER;

static struct cpufreq_info *
find_cfi_by_index(int cpu_index, int mode)
{
   list_for_each_entry(struct cpufreq_info, cfi, &gcpufreq_list, list) {
      if (cfi->mode != mode)
         continue;
      if (cfi->cpu_index == cpu_index)
         return cfi;
   }
   return 0;
}

static int
get_file_value(const char *fn, uint64_t *KHz)
{
   FILE *fh = fopen(fn, "r");
   if (!fh) {
      fprintf(stderr, "%s error: %s\n", fn, strerror(errno));
      return -1;
   }
   int ret = fscanf(fh, "%" PRIu64 "", KHz);
   fclose(fh);

   return ret;
}

static void
query_cfi_load(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct cpufreq_info *cfi = gr->query_data;

   uint64_t now = os_time_get();
   if (cfi->last_time) {
      if (cfi->last_time + gr->pane->period <= now) {
         switch (cfi->mode) {
         case CPUFREQ_MINIMUM:
         case CPUFREQ_CURRENT:
         case CPUFREQ_MAXIMUM:
            get_file_value(cfi->sysfs_filename, &cfi->KHz);
            hud_graph_add_value(gr, (uint64_t)cfi->KHz * 1000);
         }
         cfi->last_time = now;
      }
   } else {
      /* initialize */
      get_file_value(cfi->sysfs_filename, &cfi->KHz);
      cfi->last_time = now;
   }
}

/**
  * Create and initialize a new object for a specific CPU.
  * \param  pane  parent context.
  * \param  cpu_index  CPU identifier Eg. 0 (CPU0)
  * \param  mode  query CPUFREQ_MINIMUM | CURRENT | MAXIMUM statistic.
  */
void
hud_cpufreq_graph_install(struct hud_pane *pane, int cpu_index,
                           unsigned int mode)
{
   struct hud_graph *gr;
   struct cpufreq_info *cfi;

   int num_cpus = hud_get_num_cpufreq(0);
   if (num_cpus <= 0)
      return;

   cfi = find_cfi_by_index(cpu_index, mode);
   if (!cfi)
      return;

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   cfi->mode = mode;
   switch(cfi->mode) {
   case CPUFREQ_MINIMUM:
      snprintf(gr->name, sizeof(gr->name), "%s-Min", cfi->name);
      break;
   case CPUFREQ_CURRENT:
      snprintf(gr->name, sizeof(gr->name), "%s-Cur", cfi->name);
      break;
   case CPUFREQ_MAXIMUM:
      snprintf(gr->name, sizeof(gr->name), "%s-Max", cfi->name);
      break;
   default:
      free(gr);
      return;
   }

   gr->query_data = cfi;
   gr->query_new_value = query_cfi_load;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 3000000 /* 3 GHz */);
}

static void
add_object(const char *name, const char *fn, int objmode, int cpu_index)
{
   struct cpufreq_info *cfi = CALLOC_STRUCT(cpufreq_info);

   strcpy(cfi->name, name);
   strcpy(cfi->sysfs_filename, fn);
   cfi->mode = objmode;
   cfi->cpu_index = cpu_index;
   list_addtail(&cfi->list, &gcpufreq_list);
   gcpufreq_count++;
}

/**
  * Initialize internal object arrays and display cpu freq HUD help.
  * \param  displayhelp  true if the list of detected cpus should be
                         displayed on the console.
  * \return  number of detected CPU metrics (CPU count * 3)
  */
int
hud_get_num_cpufreq(bool displayhelp)
{
   struct dirent *dp;
   struct stat stat_buf;
   char fn[128];
   int cpu_index;

   /* Return the number of CPU metrics we support. */
   simple_mtx_lock(&gcpufreq_mutex);
   if (gcpufreq_count) {
      simple_mtx_unlock(&gcpufreq_mutex);
      return gcpufreq_count;
   }

   /* Scan /sys/devices.../cpu, for every object type we support, create
    * and persist an object to represent its different metrics.
    */
   list_inithead(&gcpufreq_list);
   DIR *dir = opendir("/sys/devices/system/cpu");
   if (!dir) {
      simple_mtx_unlock(&gcpufreq_mutex);
      return 0;
   }

   while ((dp = readdir(dir)) != NULL) {

      size_t d_name_len = strlen(dp->d_name);

      /* Avoid 'lo' and '..' and '.', and avoid overlong names that
       * would  result in a buffer overflow in add_object.
       */
      if (d_name_len <= 2 || d_name_len > 15)
         continue;

      if (sscanf(dp->d_name, "cpu%d\n", &cpu_index) != 1)
         continue;

      char basename[256];
      snprintf(basename, sizeof(basename), "/sys/devices/system/cpu/%s", dp->d_name);

      snprintf(fn, sizeof(fn), "%s/cpufreq/scaling_cur_freq", basename);
      if (stat(fn, &stat_buf) < 0)
         continue;

      if (!S_ISREG(stat_buf.st_mode))
         continue;              /* Not a regular file */

      snprintf(fn, sizeof(fn), "%s/cpufreq/scaling_min_freq", basename);
      add_object(dp->d_name, fn, CPUFREQ_MINIMUM, cpu_index);

      snprintf(fn, sizeof(fn), "%s/cpufreq/scaling_cur_freq", basename);
      add_object(dp->d_name, fn, CPUFREQ_CURRENT, cpu_index);

      snprintf(fn, sizeof(fn), "%s/cpufreq/scaling_max_freq", basename);
      add_object(dp->d_name, fn, CPUFREQ_MAXIMUM, cpu_index);
   }
   closedir(dir);

   if (displayhelp) {
      list_for_each_entry(struct cpufreq_info, cfi, &gcpufreq_list, list) {
         char line[128];
         snprintf(line, sizeof(line), "    cpufreq-%s-%s",
                 cfi->mode == CPUFREQ_MINIMUM ? "min" :
                 cfi->mode == CPUFREQ_CURRENT ? "cur" :
                 cfi->mode == CPUFREQ_MAXIMUM ? "max" : "undefined", cfi->name);

         puts(line);
      }
   }

   simple_mtx_unlock(&gcpufreq_mutex);
   return gcpufreq_count;
}

#endif /* HAVE_GALLIUM_EXTRA_HUD */
