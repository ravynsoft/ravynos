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

/* Purpose: Reading /sys/block/<*>/stat MB/s read/write throughput per second,
 * displaying on the HUD.
 */

#include "hud/hud_private.h"
#include "util/list.h"
#include "util/os_time.h"
#include "util/simple_mtx.h"
#include "util/u_thread.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat_s
{
   /* Read */
   uint64_t r_ios;
   uint64_t r_merges;
   uint64_t r_sectors;
   uint64_t r_ticks;
   /* Write */
   uint64_t w_ios;
   uint64_t w_merges;
   uint64_t w_sectors;
   uint64_t w_ticks;
   /* Misc */
   uint64_t in_flight;
   uint64_t io_ticks;
   uint64_t time_in_queue;
};

struct diskstat_info
{
   struct list_head list;
   int mode; /* DISKSTAT_RD, DISKSTAT_WR */
   char name[64]; /* EG. sda5 */

   char sysfs_filename[128];
   uint64_t last_time;
   struct stat_s last_stat;
};

/* TODO: We don't handle dynamic block device / partition
 * arrival or removal.
 * Static globals specific to this HUD category.
 */
static int gdiskstat_count = 0;
static struct list_head gdiskstat_list;
static simple_mtx_t gdiskstat_mutex = SIMPLE_MTX_INITIALIZER;

static struct diskstat_info *
find_dsi_by_name(const char *n, int mode)
{
   list_for_each_entry(struct diskstat_info, dsi, &gdiskstat_list, list) {
      if (dsi->mode != mode)
         continue;
      if (strcasecmp(dsi->name, n) == 0)
         return dsi;
   }
   return 0;
}

static int
get_file_values(const char *fn, struct stat_s *s)
{
   int ret = 0;
   FILE *fh = fopen(fn, "r");
   if (!fh)
      return -1;

   ret = fscanf(fh,
        "%" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64
        " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "",
        &s->r_ios, &s->r_merges, &s->r_sectors, &s->r_ticks, &s->w_ios,
        &s->w_merges, &s->w_sectors, &s->w_ticks, &s->in_flight, &s->io_ticks,
        &s->time_in_queue);

   fclose(fh);

   return ret;
}

static void
query_dsi_load(struct hud_graph *gr, struct pipe_context *pipe)
{
   /* The framework calls us periodically, compensate for the
    * calling interval accordingly when reporting per second.
    */
   struct diskstat_info *dsi = gr->query_data;
   uint64_t now = os_time_get();

   if (dsi->last_time) {
      if (dsi->last_time + gr->pane->period <= now) {
         struct stat_s stat;
         if (get_file_values(dsi->sysfs_filename, &stat) < 0)
            return;
         float val = 0;

         switch (dsi->mode) {
         case DISKSTAT_RD:
            val =
               ((stat.r_sectors -
                 dsi->last_stat.r_sectors) * 512) /
               (((float) gr->pane->period / 1000) / 1000);
            break;
         case DISKSTAT_WR:
            val =
               ((stat.w_sectors -
                 dsi->last_stat.w_sectors) * 512) /
               (((float) gr->pane->period / 1000) / 1000);
            break;
         }

         hud_graph_add_value(gr, (uint64_t) val);
         dsi->last_stat = stat;
         dsi->last_time = now;
      }
   }
   else {
      /* initialize */
      switch (dsi->mode) {
      case DISKSTAT_RD:
      case DISKSTAT_WR:
         get_file_values(dsi->sysfs_filename, &dsi->last_stat);
         break;
      }
      dsi->last_time = now;
   }
}

/**
  * Create and initialize a new object for a specific block I/O device.
  * \param  pane  parent context.
  * \param  dev_name  logical block device name, EG. sda5.
  * \param  mode  query read or write (DISKSTAT_RD/DISKSTAT_WR) statistics.
  */
void
hud_diskstat_graph_install(struct hud_pane *pane, const char *dev_name,
                           unsigned int mode)
{
   struct hud_graph *gr;
   struct diskstat_info *dsi;

   int num_devs = hud_get_num_disks(0);
   if (num_devs <= 0)
      return;

   dsi = find_dsi_by_name(dev_name, mode);
   if (!dsi)
      return;

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   dsi->mode = mode;
   if (dsi->mode == DISKSTAT_RD) {
      snprintf(gr->name, sizeof(gr->name), "%s-Read-MB/s", dsi->name);
   }
   else if (dsi->mode == DISKSTAT_WR) {
      snprintf(gr->name, sizeof(gr->name), "%s-Write-MB/s", dsi->name);
   }
   else {
      free(gr);
      return;
   }

   gr->query_data = dsi;
   gr->query_new_value = query_dsi_load;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 100);
}

static void
add_object_part(const char *basename, const char *name, int objmode)
{
   struct diskstat_info *dsi = CALLOC_STRUCT(diskstat_info);

   snprintf(dsi->name, sizeof(dsi->name), "%s", name);
   snprintf(dsi->sysfs_filename, sizeof(dsi->sysfs_filename), "%s/%s/stat",
      basename, name);
   dsi->mode = objmode;
   list_addtail(&dsi->list, &gdiskstat_list);
   gdiskstat_count++;
}

static void
add_object(const char *basename, const char *name, int objmode)
{
   struct diskstat_info *dsi = CALLOC_STRUCT(diskstat_info);

   snprintf(dsi->name, sizeof(dsi->name), "%s", name);
   snprintf(dsi->sysfs_filename, sizeof(dsi->sysfs_filename), "%s/stat",
      basename);
   dsi->mode = objmode;
   list_addtail(&dsi->list, &gdiskstat_list);
   gdiskstat_count++;
}

/**
  * Initialize internal object arrays and display block I/O HUD help.
  * \param  displayhelp  true if the list of detected devices should be
                         displayed on the console.
  * \return  number of detected block I/O devices.
  */
int
hud_get_num_disks(bool displayhelp)
{
   struct dirent *dp;
   struct stat stat_buf;
   char name[64];

   /* Return the number of block devices and partitions. */
   simple_mtx_lock(&gdiskstat_mutex);
   if (gdiskstat_count) {
      simple_mtx_unlock(&gdiskstat_mutex);
      return gdiskstat_count;
   }

   /* Scan /sys/block, for every object type we support, create and
    * persist an object to represent its different statistics.
    */
   list_inithead(&gdiskstat_list);
   DIR *dir = opendir("/sys/block/");
   if (!dir) {
      simple_mtx_unlock(&gdiskstat_mutex);
      return 0;
   }

   while ((dp = readdir(dir)) != NULL) {

      /* Avoid 'lo' and '..' and '.' */
      if (strlen(dp->d_name) <= 2)
         continue;

      char basename[256];
      snprintf(basename, sizeof(basename), "/sys/block/%s", dp->d_name);
      snprintf(name, sizeof(name), "%s/stat", basename);
      if (stat(name, &stat_buf) < 0)
         continue;

      if (!S_ISREG(stat_buf.st_mode))
         continue;              /* Not a regular file */

      /* Add a physical block device with R/W stats */
      add_object(basename, dp->d_name, DISKSTAT_RD);
      add_object(basename, dp->d_name, DISKSTAT_WR);

      /* Add any partitions */
      struct dirent *dpart;
      DIR *pdir = opendir(basename);
      if (!pdir) {
         simple_mtx_unlock(&gdiskstat_mutex);
         closedir(dir);
         return 0;
      }

      while ((dpart = readdir(pdir)) != NULL) {
         /* Avoid 'lo' and '..' and '.' */
         if (strlen(dpart->d_name) <= 2)
            continue;

         char p[64];
         snprintf(p, sizeof(p), "%s/%s/stat", basename, dpart->d_name);
         if (stat(p, &stat_buf) < 0)
            continue;

         if (!S_ISREG(stat_buf.st_mode))
            continue;           /* Not a regular file */

         /* Add a partition with R/W stats */
         add_object_part(basename, dpart->d_name, DISKSTAT_RD);
         add_object_part(basename, dpart->d_name, DISKSTAT_WR);
      }
   }
   closedir(dir);

   if (displayhelp) {
      list_for_each_entry(struct diskstat_info, dsi, &gdiskstat_list, list) {
         char line[32];
         snprintf(line, sizeof(line), "    diskstat-%s-%s",
                 dsi->mode == DISKSTAT_RD ? "rd" :
                 dsi->mode == DISKSTAT_WR ? "wr" : "undefined", dsi->name);

         puts(line);
      }
   }
   simple_mtx_unlock(&gdiskstat_mutex);

   return gdiskstat_count;
}

#endif /* HAVE_GALLIUM_EXTRA_HUD */
