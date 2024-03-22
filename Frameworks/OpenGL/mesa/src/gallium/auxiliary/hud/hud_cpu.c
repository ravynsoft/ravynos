/**************************************************************************
 *
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
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

/* This file contains code for reading CPU load for displaying on the HUD.
 */

#include "hud/hud_private.h"
#include "util/os_time.h"
#include "util/u_thread.h"
#include "util/u_memory.h"
#include "util/u_queue.h"
#include <stdio.h>
#include <inttypes.h>
#if DETECT_OS_WINDOWS
#include <windows.h>
#endif
#if DETECT_OS_BSD
#include <sys/types.h>
#include <sys/sysctl.h>
#if DETECT_OS_NETBSD || DETECT_OS_OPENBSD
#include <sys/sched.h>
#else
#include <sys/resource.h>
#endif
#endif


#if DETECT_OS_WINDOWS

static inline uint64_t
filetime_to_scalar(FILETIME ft)
{
   ULARGE_INTEGER uli;
   uli.LowPart = ft.dwLowDateTime;
   uli.HighPart = ft.dwHighDateTime;
   return uli.QuadPart;
}

static bool
get_cpu_stats(unsigned cpu_index, uint64_t *busy_time, uint64_t *total_time)
{
   SYSTEM_INFO sysInfo;
   FILETIME ftNow, ftCreation, ftExit, ftKernel, ftUser;

   GetSystemInfo(&sysInfo);
   assert(sysInfo.dwNumberOfProcessors >= 1);
   if (cpu_index != ALL_CPUS && cpu_index >= sysInfo.dwNumberOfProcessors) {
      /* Tell hud_get_num_cpus there are only this many CPUs. */
      return false;
   }

   /* Get accumulated user and sys time for all threads */
   if (!GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit,
                        &ftKernel, &ftUser))
      return false;

   GetSystemTimeAsFileTime(&ftNow);

   *busy_time = filetime_to_scalar(ftUser) + filetime_to_scalar(ftKernel);
   *total_time = filetime_to_scalar(ftNow) - filetime_to_scalar(ftCreation);

   /* busy_time already has the time accross all cpus.
    * XXX: if we want 100% to mean one CPU, 200% two cpus, eliminate the
    * following line.
    */
   *total_time *= sysInfo.dwNumberOfProcessors;

   /* XXX: we ignore cpu_index, i.e, we assume that the individual CPU usage
    * and the system usage are one and the same.
    */
   return true;
}

#elif DETECT_OS_BSD

static bool
get_cpu_stats(unsigned cpu_index, uint64_t *busy_time, uint64_t *total_time)
{
#if DETECT_OS_NETBSD || DETECT_OS_OPENBSD
   uint64_t cp_time[CPUSTATES];
#else
   long cp_time[CPUSTATES];
#endif
   size_t len;

   if (cpu_index == ALL_CPUS) {
      len = sizeof(cp_time);

#if DETECT_OS_NETBSD
      int mib[] = { CTL_KERN, KERN_CP_TIME };

      if (sysctl(mib, ARRAY_SIZE(mib), cp_time, &len, NULL, 0) == -1)
         return false;
#elif DETECT_OS_OPENBSD
      int mib[] = { CTL_KERN, KERN_CPTIME };
      long sum_cp_time[CPUSTATES];

      len = sizeof(sum_cp_time);
      if (sysctl(mib, ARRAY_SIZE(mib), sum_cp_time, &len, NULL, 0) == -1)
         return false;

      for (int state = 0; state < CPUSTATES; state++)
         cp_time[state] = sum_cp_time[state];
#else
      if (sysctlbyname("kern.cp_time", cp_time, &len, NULL, 0) == -1)
         return false;
#endif
   } else {
#if DETECT_OS_NETBSD
      int mib[] = { CTL_KERN, KERN_CP_TIME, cpu_index };

      len = sizeof(cp_time);
      if (sysctl(mib, ARRAY_SIZE(mib), cp_time, &len, NULL, 0) == -1)
         return false;
#elif DETECT_OS_OPENBSD
      int mib[] = { CTL_KERN, KERN_CPTIME2, cpu_index };

      len = sizeof(cp_time);
      if (sysctl(mib, ARRAY_SIZE(mib), cp_time, &len, NULL, 0) == -1)
         return false;
#else
      long *cp_times = NULL;

      if (sysctlbyname("kern.cp_times", NULL, &len, NULL, 0) == -1)
         return false;

      if (len < (cpu_index + 1) * sizeof(cp_time))
         return false;

      cp_times = malloc(len);

      if (sysctlbyname("kern.cp_times", cp_times, &len, NULL, 0) == -1)
         return false;

      memcpy(cp_time, cp_times + (cpu_index * CPUSTATES),
            sizeof(cp_time));
      free(cp_times);
#endif
   }

   *busy_time = cp_time[CP_USER] + cp_time[CP_NICE] +
      cp_time[CP_SYS] + cp_time[CP_INTR];

   *total_time = *busy_time + cp_time[CP_IDLE];

   return true;
}

#else

static bool
get_cpu_stats(unsigned cpu_index, uint64_t *busy_time, uint64_t *total_time)
{
   char cpuname[32];
   char line[1024];
   FILE *f;

   if (cpu_index == ALL_CPUS)
      strcpy(cpuname, "cpu");
   else
      sprintf(cpuname, "cpu%u", cpu_index);

   f = fopen("/proc/stat", "r");
   if (!f)
      return false;

   while (!feof(f) && fgets(line, sizeof(line), f)) {
      if (strstr(line, cpuname) == line) {
         uint64_t v[12];
         int i, num;

         num = sscanf(line,
                      "%s %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64
                      " %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64
                      " %"PRIu64" %"PRIu64"",
                      cpuname, &v[0], &v[1], &v[2], &v[3], &v[4], &v[5],
                      &v[6], &v[7], &v[8], &v[9], &v[10], &v[11]);
         if (num < 5) {
            fclose(f);
            return false;
         }

         /* user + nice + system */
         *busy_time = v[0] + v[1] + v[2];
         *total_time = *busy_time;

         /* ... + idle + iowait + irq + softirq + ...  */
         for (i = 3; i < num-1; i++) {
            *total_time += v[i];
         }
         fclose(f);
         return true;
      }
   }
   fclose(f);
   return false;
}
#endif


struct cpu_info {
   unsigned cpu_index;
   uint64_t last_cpu_busy, last_cpu_total, last_time;
};

static void
query_cpu_load(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct cpu_info *info = gr->query_data;
   uint64_t now = os_time_get();

   if (info->last_time) {
      if (info->last_time + gr->pane->period <= now) {
         uint64_t cpu_busy, cpu_total;
         double cpu_load;

         get_cpu_stats(info->cpu_index, &cpu_busy, &cpu_total);

         cpu_load = (cpu_busy - info->last_cpu_busy) * 100 /
                    (double)(cpu_total - info->last_cpu_total);
         hud_graph_add_value(gr, cpu_load);

         info->last_cpu_busy = cpu_busy;
         info->last_cpu_total = cpu_total;
         info->last_time = now;
      }
   }
   else {
      /* initialize */
      info->last_time = now;
      get_cpu_stats(info->cpu_index, &info->last_cpu_busy,
                    &info->last_cpu_total);
   }
}

static void
free_query_data(void *p, struct pipe_context *pipe)
{
   FREE(p);
}

void
hud_cpu_graph_install(struct hud_pane *pane, unsigned cpu_index)
{
   struct hud_graph *gr;
   struct cpu_info *info;
   uint64_t busy, total;

   /* see if the cpu exists */
   if (cpu_index != ALL_CPUS && !get_cpu_stats(cpu_index, &busy, &total)) {
      return;
   }

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   if (cpu_index == ALL_CPUS)
      strcpy(gr->name, "cpu");
   else
      sprintf(gr->name, "cpu%u", cpu_index);

   gr->query_data = CALLOC_STRUCT(cpu_info);
   if (!gr->query_data) {
      FREE(gr);
      return;
   }

   gr->query_new_value = query_cpu_load;

   /* Don't use free() as our callback as that messes up Gallium's
    * memory debugger.  Use simple free_query_data() wrapper.
    */
   gr->free_query_data = free_query_data;

   info = gr->query_data;
   info->cpu_index = cpu_index;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 100);
}

int
hud_get_num_cpus(void)
{
   uint64_t busy, total;
   int i = 0;

   while (get_cpu_stats(i, &busy, &total))
      i++;

   return i;
}

struct thread_info {
   bool main_thread;
   int64_t last_time;
   int64_t last_thread_time;
};

static void
query_api_thread_busy_status(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct thread_info *info = gr->query_data;
   int64_t now = os_time_get_nano();

   if (info->last_time) {
      if (info->last_time + gr->pane->period*1000 <= now) {
         int64_t thread_now;

         if (info->main_thread) {
            thread_now = util_current_thread_get_time_nano();
         } else {
            struct util_queue_monitoring *mon = gr->pane->hud->monitored_queue;

            if (mon && mon->queue)
               thread_now = util_queue_get_thread_time_nano(mon->queue, 0);
            else
               thread_now = 0;
         }

         double percent = (thread_now - info->last_thread_time) * 100.0 /
                            (now - info->last_time);

         /* Check if the context changed a thread, so that we don't show
          * a random value. When a thread is changed, the new thread clock
          * is different, which can result in "percent" being very high.
          */
         if (percent > 100.0)
            percent = 0.0;
         hud_graph_add_value(gr, percent);

         info->last_thread_time = thread_now;
         info->last_time = now;
      }
   } else {
      /* initialize */
      info->last_time = now;
      info->last_thread_time = util_current_thread_get_time_nano();
   }
}

void
hud_thread_busy_install(struct hud_pane *pane, const char *name, bool main)
{
   struct hud_graph *gr;

   gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   strcpy(gr->name, name);

   gr->query_data = CALLOC_STRUCT(thread_info);
   if (!gr->query_data) {
      FREE(gr);
      return;
   }

   ((struct thread_info*)gr->query_data)->main_thread = main;
   gr->query_new_value = query_api_thread_busy_status;

   /* Don't use free() as our callback as that messes up Gallium's
    * memory debugger.  Use simple free_query_data() wrapper.
    */
   gr->free_query_data = free_query_data;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 100);
}

struct counter_info {
   enum hud_counter counter;
   int64_t last_time;
};

static unsigned get_counter(struct hud_graph *gr, enum hud_counter counter)
{
   struct util_queue_monitoring *mon = gr->pane->hud->monitored_queue;
   unsigned value;

   if (!mon || !mon->queue)
      return 0;

   /* Reset the counters to 0 to only display values for 1 frame. */
   switch (counter) {
   case HUD_COUNTER_OFFLOADED:
      value = mon->num_offloaded_items;
      mon->num_offloaded_items = 0;
      return value;
   case HUD_COUNTER_DIRECT:
      value = mon->num_direct_items;
      mon->num_direct_items = 0;
      return value;
   case HUD_COUNTER_SYNCS:
      value = mon->num_syncs;
      mon->num_syncs = 0;
      return value;
   case HUD_COUNTER_BATCHES:
      value = mon->num_batches;
      mon->num_batches = 0;
      return value;
   default:
      assert(0);
      return 0;
   }
}

static void
query_thread_counter(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct counter_info *info = gr->query_data;
   int64_t now = os_time_get_nano();
   unsigned value = get_counter(gr, info->counter);

   if (info->last_time) {
      if (info->last_time + gr->pane->period*1000 <= now) {
         hud_graph_add_value(gr, value);
         info->last_time = now;
      }
   } else {
      /* initialize */
      info->last_time = now;
   }
}

void hud_thread_counter_install(struct hud_pane *pane, const char *name,
                                enum hud_counter counter)
{
   struct hud_graph *gr = CALLOC_STRUCT(hud_graph);
   if (!gr)
      return;

   strcpy(gr->name, name);

   gr->query_data = CALLOC_STRUCT(counter_info);
   if (!gr->query_data) {
      FREE(gr);
      return;
   }

   ((struct counter_info*)gr->query_data)->counter = counter;
   gr->query_new_value = query_thread_counter;

   /* Don't use free() as our callback as that messes up Gallium's
    * memory debugger.  Use simple free_query_data() wrapper.
    */
   gr->free_query_data = free_query_data;

   hud_pane_add_graph(pane, gr);
   hud_pane_set_max_value(pane, 100);
}
