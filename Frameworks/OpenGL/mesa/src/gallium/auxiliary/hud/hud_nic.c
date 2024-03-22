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

/* Purpose: Reading network interface RX/TX throughput per second,
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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

struct nic_info
{
   struct list_head list;
   int mode;
   char name[64];
   uint64_t speedMbps;
   int is_wireless;

   char throughput_filename[128];
   uint64_t last_time;
   uint64_t last_nic_bytes;
};

/* TODO: We don't handle dynamic NIC arrival or removal.
 * Static globals specific to this HUD category.
 */
static int gnic_count = 0;
static struct list_head gnic_list;
static simple_mtx_t gnic_mutex = SIMPLE_MTX_INITIALIZER;

static struct nic_info *
find_nic_by_name(const char *n, int mode)
{
   list_for_each_entry(struct nic_info, nic, &gnic_list, list) {
      if (nic->mode != mode)
         continue;

      if (strcasecmp(nic->name, n) == 0)
         return nic;
   }
   return 0;
}

static int
get_file_value(const char *fname, uint64_t *value)
{
   FILE *fh = fopen(fname, "r");
   if (!fh)
      return -1;
   if (fscanf(fh, "%" PRIu64 "", value) != 0) {
      /* Error */
   }
   fclose(fh);
   return 0;
}

static bool
get_nic_bytes(const char *fn, uint64_t *bytes)
{
   if (get_file_value(fn, bytes) < 0)
      return false;

   return true;
}

static void
query_wifi_bitrate(const struct nic_info *nic, uint64_t *bitrate)
{
   int sockfd;
   struct iw_statistics stats;
   struct iwreq req;

   memset(&stats, 0, sizeof(stats));
   memset(&req, 0, sizeof(req));

   snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", nic->name);
   req.u.data.pointer = &stats;
   req.u.data.flags = 1;
   req.u.data.length = sizeof(struct iw_statistics);

   /* Any old socket will do, and a datagram socket is pretty cheap */
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      fprintf(stderr, "Unable to create socket for %s\n", nic->name);
      return;
   }

   if (ioctl(sockfd, SIOCGIWRATE, &req) == -1) {
      fprintf(stderr, "Error performing SIOCGIWSTATS on %s\n", nic->name);
      close(sockfd);
      return;
   }
   *bitrate = req.u.bitrate.value;

   close(sockfd);
}

static void
query_nic_rssi(const struct nic_info *nic, uint64_t *leveldBm)
{
   int sockfd;
   struct iw_statistics stats;
   struct iwreq req;

   memset(&stats, 0, sizeof(stats));
   memset(&req, 0, sizeof(req));

   snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", nic->name);
   req.u.data.pointer = &stats;
   req.u.data.flags = 1;
   req.u.data.length = sizeof(struct iw_statistics);

   if (nic->mode != NIC_RSSI_DBM)
      return;

   /* Any old socket will do, and a datagram socket is pretty cheap */
   if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      fprintf(stderr, "Unable to create socket for %s\n", nic->name);
      return;
   }

   /* Perform the ioctl */
   if (ioctl(sockfd, SIOCGIWSTATS, &req) == -1) {
      fprintf(stderr, "Error performing SIOCGIWSTATS on %s\n", nic->name);
      close(sockfd);
      return;
   }
   *leveldBm = ((char) stats.qual.level * -1);

   close(sockfd);
}

static void
query_nic_load(struct hud_graph *gr, struct pipe_context *pipe)
{
   /* The framework calls us at a regular but indefined period,
    * not once per second, compensate the statistics accordingly.
    */

   struct nic_info *nic = gr->query_data;
   uint64_t now = os_time_get();

   if (nic->last_time) {
      if (nic->last_time + gr->pane->period <= now) {
         switch (nic->mode) {
         case NIC_DIRECTION_RX:
         case NIC_DIRECTION_TX:
            {
               uint64_t bytes;
               get_nic_bytes(nic->throughput_filename, &bytes);
               uint64_t nic_mbps =
                  ((bytes - nic->last_nic_bytes) / 1000000) * 8;

               float speedMbps = nic->speedMbps;
               float periodMs = gr->pane->period / 1000.0;
               float bits = nic_mbps;
               float period_factor = periodMs / 1000;
               float period_speed = speedMbps * period_factor;
               float pct = (bits / period_speed) * 100;

               /* Scaling bps with a narrow time period into a second,
                * potentially suffers from rounding errors at higher
                * periods. Eg 104%. Compensate.
                */
               if (pct > 100)
                  pct = 100;
               hud_graph_add_value(gr, (uint64_t) pct);

               nic->last_nic_bytes = bytes;
            }
            break;
         case NIC_RSSI_DBM:
            {
               uint64_t leveldBm = 0;
               query_nic_rssi(nic, &leveldBm);
               hud_graph_add_value(gr, leveldBm);
            }
            break;
         }

         nic->last_time = now;
      }
   }
   else {
      /* initialize */
      switch (nic->mode) {
      case NIC_DIRECTION_RX:
      case NIC_DIRECTION_TX:
         get_nic_bytes(nic->throughput_filename, &nic->last_nic_bytes);
         break;
      case NIC_RSSI_DBM:
         break;
      }

      nic->last_time = now;
   }
}

/**
  * Create and initialize a new object for a specific network interface dev.
  * \param  pane  parent context.
  * \param  nic_name  logical block device name, EG. eth0.
  * \param  mode  query type (NIC_DIRECTION_RX/WR/RSSI) statistics.
  */
void
hud_nic_graph_install(struct hud_pane *pane, const char *nic_name,
                      unsigned int mode)
{
   struct hud_graph *gr;
   struct nic_info *nic;

   int num_nics = hud_get_num_nics(0);
   if (num_nics <= 0)
      return;

   nic = find_nic_by_name(nic_name, mode);
   if (!nic)
      return;

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   nic->mode = mode;
   if (nic->mode == NIC_DIRECTION_RX) {
      snprintf(gr->name, sizeof(gr->name), "%s-rx-%"PRId64"Mbps", nic->name,
         nic->speedMbps);
   }
   else if (nic->mode == NIC_DIRECTION_TX) {
      snprintf(gr->name, sizeof(gr->name), "%s-tx-%"PRId64"Mbps", nic->name,
         nic->speedMbps);
   }
   else if (nic->mode == NIC_RSSI_DBM)
      snprintf(gr->name, sizeof(gr->name), "%s-rssi", nic->name);
   else {
      free(gr);
      return;
   }

   gr->query_data = nic;
   gr->query_new_value = query_nic_load;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 100);
}

static int
is_wireless_nic(const char *dirbase)
{
   struct stat stat_buf;

   /* Check if its a wireless card */
   char fn[256];
   snprintf(fn, sizeof(fn), "%s/wireless", dirbase);
   if (stat(fn, &stat_buf) == 0)
      return 1;

   return 0;
}

static void
query_nic_bitrate(struct nic_info *nic, const char *dirbase)
{
   struct stat stat_buf;

   /* Check if its a wireless card */
   char fn[256];
   snprintf(fn, sizeof(fn), "%s/wireless", dirbase);
   if (stat(fn, &stat_buf) == 0) {
      /* we're a wireless nic */
      query_wifi_bitrate(nic, &nic->speedMbps);
      nic->speedMbps /= 1000000;
   }
   else {
      /* Must be a wired nic */
      snprintf(fn, sizeof(fn), "%s/speed", dirbase);
      get_file_value(fn, &nic->speedMbps);
   }
}

/**
  * Initialize internal object arrays and display NIC HUD help.
  * \param  displayhelp  true if the list of detected devices should be
                         displayed on the console.
  * \return  number of detected network interface devices.
  */
int
hud_get_num_nics(bool displayhelp)
{
   struct dirent *dp;
   struct stat stat_buf;
   struct nic_info *nic;
   char name[64];

   /* Return the number if network interfaces. */
   simple_mtx_lock(&gnic_mutex);
   if (gnic_count) {
      simple_mtx_unlock(&gnic_mutex);
      return gnic_count;
   }

   /* Scan /sys/block, for every object type we support, create and
    * persist an object to represent its different statistics.
    */
   list_inithead(&gnic_list);
   DIR *dir = opendir("/sys/class/net/");
   if (!dir) {
      simple_mtx_unlock(&gnic_mutex);
      return 0;
   }

   while ((dp = readdir(dir)) != NULL) {

      /* Avoid 'lo' and '..' and '.' */
      if (strlen(dp->d_name) <= 2)
         continue;

      char basename[256];
      snprintf(basename, sizeof(basename), "/sys/class/net/%s", dp->d_name);
      snprintf(name, sizeof(name), "%s/statistics/rx_bytes", basename);
      if (stat(name, &stat_buf) < 0)
         continue;

      if (!S_ISREG(stat_buf.st_mode))
         continue;              /* Not a regular file */

      int is_wireless = is_wireless_nic(basename);

      /* Add the RX object */
      nic = CALLOC_STRUCT(nic_info);
      strcpy(nic->name, dp->d_name);
      snprintf(nic->throughput_filename, sizeof(nic->throughput_filename),
         "%s/statistics/rx_bytes", basename);
      nic->mode = NIC_DIRECTION_RX;
      nic->is_wireless = is_wireless;
      query_nic_bitrate(nic, basename);

      list_addtail(&nic->list, &gnic_list);
      gnic_count++;

      /* Add the TX object */
      nic = CALLOC_STRUCT(nic_info);
      strcpy(nic->name, dp->d_name);
      snprintf(nic->throughput_filename,
         sizeof(nic->throughput_filename),
         "/sys/class/net/%s/statistics/tx_bytes", dp->d_name);
      nic->mode = NIC_DIRECTION_TX;
      nic->is_wireless = is_wireless;

      query_nic_bitrate(nic, basename);

      list_addtail(&nic->list, &gnic_list);
      gnic_count++;

      if (nic->is_wireless) {
         /* RSSI Support */
         nic = CALLOC_STRUCT(nic_info);
         strcpy(nic->name, dp->d_name);
         snprintf(nic->throughput_filename,
            sizeof(nic->throughput_filename),
            "/sys/class/net/%s/statistics/tx_bytes", dp->d_name);
         nic->mode = NIC_RSSI_DBM;

         query_nic_bitrate(nic, basename);

         list_addtail(&nic->list, &gnic_list);
         gnic_count++;
      }

   }
   closedir(dir);

   list_for_each_entry(struct nic_info, nic, &gnic_list, list) {
      char line[64];
      snprintf(line, sizeof(line), "    nic-%s-%s",
              nic->mode == NIC_DIRECTION_RX ? "rx" :
              nic->mode == NIC_DIRECTION_TX ? "tx" :
              nic->mode == NIC_RSSI_DBM ? "rssi" : "undefined", nic->name);

      puts(line);

   }

   simple_mtx_unlock(&gnic_mutex);
   return gnic_count;
}

#endif /* HAVE_GALLIUM_EXTRA_HUD */
