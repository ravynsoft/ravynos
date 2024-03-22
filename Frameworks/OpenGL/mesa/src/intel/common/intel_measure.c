/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file intel_measure.c
 */

#include "intel_measure.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include "dev/intel_device_info.h"
#include "util/os_time.h"
#include "util/u_debug.h"
#include "util/macros.h"


static const struct debug_control debug_control[] = {
   { "draw",            INTEL_MEASURE_DRAW       },
   { "rt",              INTEL_MEASURE_RENDERPASS },
   { "shader",          INTEL_MEASURE_SHADER     },
   { "batch",           INTEL_MEASURE_BATCH      },
   { "frame",           INTEL_MEASURE_FRAME      },
   { NULL, 0 }
};
static struct intel_measure_config config;

void
intel_measure_init(struct intel_measure_device *device)
{
   static bool once = false;
   const char *env = getenv("INTEL_MEASURE");
   if (unlikely(!once)) {
      once = true;
      memset(&config, 0, sizeof(struct intel_measure_config));
      if (!env)
         return;

      char env_copy[1024];
      strncpy(env_copy, env, 1024);
      env_copy[1023] = '\0';

      config.file = stderr;
      config.flags = parse_debug_string(env_copy, debug_control);
      if (!config.flags)
         config.flags = INTEL_MEASURE_DRAW;
      config.enabled = true;
      config.event_interval = 1;
      config.control_fh = -1;

      /* Overflows of the following defaults will drop data and generate a
       * warning on the output filehandle.
       */

      /* default batch_size allows for 32k renders in a single batch */
      const int MINIMUM_BATCH_SIZE = 1024;
      const int DEFAULT_BATCH_SIZE = 64 * 1024;
      config.batch_size = DEFAULT_BATCH_SIZE;

      /* Default buffer_size allows for 64k batches per line of output in the
       * csv.  Overflow may occur for offscreen workloads or large 'interval'
       * settings.
       */
      const int MINIMUM_BUFFER_SIZE = 1024;
      const int DEFAULT_BUFFER_SIZE = 64 * 1024;
      config.buffer_size = DEFAULT_BUFFER_SIZE;

      const char *filename = strstr(env_copy, "file=");
      const char *start_frame_s = strstr(env_copy, "start=");
      const char *count_frame_s = strstr(env_copy, "count=");
      const char *control_path = strstr(env_copy, "control=");
      const char *interval_s = strstr(env_copy, "interval=");
      const char *batch_size_s = strstr(env_copy, "batch_size=");
      const char *buffer_size_s = strstr(env_copy, "buffer_size=");
      const char *cpu_s = strstr(env_copy, "cpu");
      while (true) {
         char *sep = strrchr(env_copy, ',');
         if (sep == NULL)
            break;
         *sep = '\0';
      }

      if (filename && __normal_user()) {
         filename += 5;
         config.file = fopen(filename, "w");
         if (!config.file) {
            fprintf(stderr, "INTEL_MEASURE failed to open output file %s: %s\n",
                    filename, strerror (errno));
            abort();
         }
      }

      if (start_frame_s) {
         start_frame_s += 6;
         const int start_frame = atoi(start_frame_s);
         if (start_frame < 0) {
            fprintf(stderr, "INTEL_MEASURE start frame may "
                    "not be negative: %d\n", start_frame);
            abort();
         }

         config.start_frame = start_frame;
         config.enabled = false;
      }

      if (count_frame_s) {
         count_frame_s += 6;
         const int count_frame = atoi(count_frame_s);
         if (count_frame <= 0) {
            fprintf(stderr, "INTEL_MEASURE count frame must be positive: %d\n",
                    count_frame);
            abort();
         }

         config.end_frame = config.start_frame + count_frame;
      }

      if (control_path) {
         control_path += 8;
         if (mkfifoat(AT_FDCWD, control_path, O_CREAT | S_IRUSR | S_IWUSR)) {
            if (errno != EEXIST) {
               fprintf(stderr, "INTEL_MEASURE failed to create control "
                       "fifo %s: %s\n", control_path, strerror (errno));
               abort();
            }
         }

         config.control_fh = openat(AT_FDCWD, control_path,
                                    O_RDONLY | O_NONBLOCK);
         if (config.control_fh == -1) {
            fprintf(stderr, "INTEL_MEASURE failed to open control fifo "
                    "%s: %s\n", control_path, strerror (errno));
            abort();
         }

         /* when using a control fifo, do not start until the user triggers
          * capture
          */
         config.enabled = false;
      }

      if (interval_s) {
         interval_s += 9;
         const int event_interval = atoi(interval_s);
         if (event_interval < 1) {
            fprintf(stderr, "INTEL_MEASURE event_interval must be positive: "
                    "%d\n", event_interval);
            abort();
         }
         config.event_interval = event_interval;
      }

      if (batch_size_s) {
         batch_size_s += 11;
         const int batch_size = atoi(batch_size_s);
         if (batch_size < MINIMUM_BATCH_SIZE ) {
            fprintf(stderr, "INTEL_MEASURE minimum batch_size is 1k: "
                    "%d\n", batch_size);
            abort();
         }
         if (batch_size > MINIMUM_BATCH_SIZE * 4 * 1024) {
            fprintf(stderr, "INTEL_MEASURE batch_size limited to 4M: "
                    "%d\n", batch_size);
            abort();
         }

         config.batch_size = batch_size;
      }

      if (buffer_size_s) {
         buffer_size_s += 12;
         const int buffer_size = atoi(buffer_size_s);
         if (buffer_size < MINIMUM_BUFFER_SIZE) {
            fprintf(stderr, "INTEL_MEASURE minimum buffer_size is 1k: "
                    "%d\n", DEFAULT_BUFFER_SIZE);
         }
         if (buffer_size > MINIMUM_BUFFER_SIZE * 1024) {
            fprintf(stderr, "INTEL_MEASURE buffer_size limited to 1M: "
                    "%d\n", buffer_size);
         }

         config.buffer_size = buffer_size;
      }

      if (cpu_s) {
         config.cpu_measure = true;
      }

      if (!config.cpu_measure)
         fputs("draw_start,draw_end,frame,batch,batch_size,renderpass,"
               "event_index,event_count,type,count,vs,tcs,tes,"
               "gs,fs,cs,ms,ts,idle_us,time_us\n",
               config.file);
      else
         fputs("draw_start,frame,batch,batch_size,event_index,event_count,"
               "type,count\n",
               config.file);
   }

   device->config = NULL;
   device->frame = 0;
   device->render_pass_count = 0;
   device->release_batch = NULL;
   pthread_mutex_init(&device->mutex, NULL);
   list_inithead(&device->queued_snapshots);

   if (env)
      device->config = &config;
}

const char *
intel_measure_snapshot_string(enum intel_measure_snapshot_type type)
{
   const char *names[] = {
      [INTEL_SNAPSHOT_UNDEFINED]           = "undefined",
      [INTEL_SNAPSHOT_BLIT]                = "blit",
      [INTEL_SNAPSHOT_CCS_AMBIGUATE]       = "ccs ambiguate",
      [INTEL_SNAPSHOT_CCS_COLOR_CLEAR]     = "ccs color clear",
      [INTEL_SNAPSHOT_CCS_PARTIAL_RESOLVE] = "ccs partial resolve",
      [INTEL_SNAPSHOT_CCS_RESOLVE]         = "ccs resolve",
      [INTEL_SNAPSHOT_COMPUTE]             = "compute",
      [INTEL_SNAPSHOT_COPY]                = "copy",
      [INTEL_SNAPSHOT_DRAW]                = "draw",
      [INTEL_SNAPSHOT_HIZ_AMBIGUATE]       = "hiz ambiguate",
      [INTEL_SNAPSHOT_HIZ_CLEAR]           = "hiz clear",
      [INTEL_SNAPSHOT_HIZ_RESOLVE]         = "hiz resolve",
      [INTEL_SNAPSHOT_MCS_AMBIGUATE]       = "mcs ambiguate",
      [INTEL_SNAPSHOT_MCS_COLOR_CLEAR]     = "mcs color clear",
      [INTEL_SNAPSHOT_MCS_PARTIAL_RESOLVE] = "mcs partial resolve",
      [INTEL_SNAPSHOT_SLOW_COLOR_CLEAR]    = "slow color clear",
      [INTEL_SNAPSHOT_SLOW_DEPTH_CLEAR]    = "slow depth clear",
      [INTEL_SNAPSHOT_SECONDARY_BATCH]     = "secondary command buffer",
      [INTEL_SNAPSHOT_END]                 = "end",
   };
   assert(type < ARRAY_SIZE(names));
   assert(names[type] != NULL);
   assert(type != INTEL_SNAPSHOT_UNDEFINED);
   return names[type];
}

/**
 * Indicate to the caller whether a new snapshot should be started.
 *
 * Callers provide rendering state to this method to determine whether the
 * current start event should be skipped. Depending on the configuration
 * flags, a new snapshot may start:
 *  - at every event
 *  - when the program changes
 *  - after a batch is submitted
 *  - at frame boundaries
 *
 * Returns true if a snapshot should be started.
 */
bool
intel_measure_state_changed(const struct intel_measure_batch *batch,
                            uint32_t vs, uint32_t tcs, uint32_t tes,
                            uint32_t gs, uint32_t fs, uint32_t cs,
                            uint32_t ms, uint32_t ts)
{
   if (batch->index == 0) {
      /* always record the first event */
      return true;
   }

   const struct intel_measure_snapshot *last_snap =
      &batch->snapshots[batch->index - 1];

   if (config.flags & INTEL_MEASURE_DRAW)
      return true;

   if (batch->index % 2 == 0) {
      /* no snapshot is running, but we have a start event */
      return true;
   }

   if (config.flags & (INTEL_MEASURE_FRAME | INTEL_MEASURE_BATCH)) {
      /* only start collection when index == 0, at the beginning of a batch */
      return false;
   }

   if (config.flags & INTEL_MEASURE_RENDERPASS) {
      bool new_renderpass = !cs && last_snap->renderpass != batch->renderpass;
      bool new_compute_block = cs && last_snap->type != INTEL_SNAPSHOT_COMPUTE;
      return new_renderpass || new_compute_block;
   }

   /* remaining comparisons check the state of the render pipeline for
    * INTEL_MEASURE_PROGRAM
    */
   assert(config.flags & INTEL_MEASURE_SHADER);

   if (!vs && !tcs && !tes && !gs && !fs && !cs && !ms && !ts) {
      /* blorp always changes program */
      return true;
   }

   return (last_snap->vs  != vs ||
           last_snap->tcs != tcs ||
           last_snap->tes != tes ||
           last_snap->gs  != gs ||
           last_snap->fs  != fs ||
           last_snap->cs  != cs ||
           last_snap->ms  != ms ||
           last_snap->ts  != ts);
}

/**
 * Notify intel_measure that a frame is about to begin.
 *
 * Configuration values and the control fifo may commence measurement at frame
 * boundaries.
 */
void
intel_measure_frame_transition(unsigned frame)
{
   if (frame == config.start_frame)
      config.enabled = true;
   else if (frame == config.end_frame)
      config.enabled = false;

   /* user commands to the control fifo will override any start/count
    * environment settings
    */
   if (config.control_fh != -1) {
      while (true) {
         const unsigned BUF_SIZE = 128;
         char buf[BUF_SIZE];
         ssize_t bytes = read(config.control_fh, buf, BUF_SIZE - 1);
         if (bytes == 0)
            break;
         if (bytes == -1) {
            fprintf(stderr, "INTEL_MEASURE failed to read control fifo: %s\n",
                    strerror(errno));
            abort();
         }

         buf[bytes] = '\0';
         char *nptr = buf, *endptr = buf;
         while (*nptr != '\0' && *endptr != '\0') {
            long fcount = strtol(nptr, &endptr, 10);
            if (nptr == endptr) {
               config.enabled = false;
               fprintf(stderr, "INTEL_MEASURE invalid frame count on "
                       "control fifo.\n");
               lseek(config.control_fh, 0, SEEK_END);
               break;
            } else if (fcount == 0) {
               config.enabled = false;
            } else {
               config.enabled = true;
               config.end_frame = frame + fcount;
            }

            nptr = endptr + 1;
         }
      }
   }
}

#define TIMESTAMP_BITS 36
static uint64_t
raw_timestamp_delta(uint64_t time0, uint64_t time1)
{
   if (time0 > time1) {
      return (1ULL << TIMESTAMP_BITS) + time1 - time0;
   } else {
      return time1 - time0;
   }
}

/**
 * Verify that rendering has completed for the batch
 *
 * Rendering is complete when the last timestamp has been written.
*/
bool
intel_measure_ready(struct intel_measure_batch *batch)
{
   assert(batch->timestamps);
   assert(batch->index > 1);
   return (batch->timestamps[batch->index - 1] != 0);
}

/**
 * Submit completed snapshots for buffering.
 *
 * Snapshot data becomes available when asynchronous rendering completes.
 * Depending on configuration, snapshot data may need to be collated before
 * writing to the output file.
 */
static void
intel_measure_push_result(struct intel_measure_device *device,
                          struct intel_measure_batch *batch)
{
   struct intel_measure_ringbuffer *rb = device->ringbuffer;

   uint64_t *timestamps = batch->timestamps;
   assert(timestamps != NULL);
   assert(batch->index == 0 || timestamps[0] != 0);

   for (int i = 0; i < batch->index; i += 2) {
      const struct intel_measure_snapshot *begin = &batch->snapshots[i];
      const struct intel_measure_snapshot *end = &batch->snapshots[i+1];

      assert (end->type == INTEL_SNAPSHOT_END);

      if (begin->type == INTEL_SNAPSHOT_SECONDARY_BATCH) {
         assert(begin->secondary != NULL);
         begin->secondary->batch_count = batch->batch_count;
         begin->secondary->batch_size = 0;
         begin->secondary->primary_renderpass = batch->renderpass;
         intel_measure_push_result(device, begin->secondary);
         continue;
      }

      const uint64_t prev_end_ts = rb->results[rb->head].end_ts;

      /* advance ring buffer */
      if (++rb->head == config.buffer_size)
         rb->head = 0;
      if (rb->head == rb->tail) {
         static bool warned = false;
         if (unlikely(!warned)) {
            fprintf(config.file,
                    "WARNING: Buffered data exceeds INTEL_MEASURE limit: %d. "
                    "Data has been dropped. "
                    "Increase setting with INTEL_MEASURE=buffer_size={count}\n",
                    config.buffer_size);
            warned = true;
         }
         break;
      }

      struct intel_measure_buffered_result *buffered_result =
         &rb->results[rb->head];

      memset(buffered_result, 0, sizeof(*buffered_result));
      memcpy(&buffered_result->snapshot, begin,
             sizeof(struct intel_measure_snapshot));
      buffered_result->start_ts = timestamps[i];
      buffered_result->end_ts = timestamps[i+1];
      buffered_result->idle_duration =
         raw_timestamp_delta(prev_end_ts, buffered_result->start_ts);
      buffered_result->frame = batch->frame;
      buffered_result->batch_count = batch->batch_count;
      buffered_result->batch_size = batch->batch_size;
      buffered_result->primary_renderpass = batch->primary_renderpass;
      buffered_result->event_index = i / 2;
      buffered_result->snapshot.event_count = end->event_count;
   }
}

static unsigned
ringbuffer_size(const struct intel_measure_ringbuffer *rb)
{
   unsigned head = rb->head;
   if (head < rb->tail)
      head += config.buffer_size;
   return head - rb->tail;
}

static const struct intel_measure_buffered_result *
ringbuffer_pop(struct intel_measure_ringbuffer *rb)
{
   if (rb->tail == rb->head) {
      /* encountered ringbuffer overflow while processing events */
      return NULL;
   }

   if (++rb->tail == config.buffer_size)
      rb->tail = 0;
   return &rb->results[rb->tail];
}

static const struct intel_measure_buffered_result *
ringbuffer_peek(const struct intel_measure_ringbuffer *rb, unsigned index)
{
   int result_offset = rb->tail + index + 1;
   if (result_offset >= config.buffer_size)
      result_offset -= config.buffer_size;
   return &rb->results[result_offset];
}


/**
 * Determine the number of buffered events that must be combined for the next
 * line of csv output. Returns 0 if more events are needed.
 */
static unsigned
buffered_event_count(struct intel_measure_device *device)
{
   const struct intel_measure_ringbuffer *rb = device->ringbuffer;
   const unsigned buffered_event_count = ringbuffer_size(rb);
   if (buffered_event_count == 0) {
      /* no events to collect */
      return 0;
   }

   /* count the number of buffered events required to meet the configuration */
   if (config.flags & (INTEL_MEASURE_DRAW |
                       INTEL_MEASURE_RENDERPASS |
                       INTEL_MEASURE_SHADER)) {
      /* For these flags, every buffered event represents a line in the
       * output.  None of these events span batches.  If the event interval
       * crosses a batch boundary, then the next interval starts with the new
       * batch.
       */
      return 1;
   }

   const unsigned start_frame = ringbuffer_peek(rb, 0)->frame;
   if (config.flags & INTEL_MEASURE_BATCH) {
      /* each buffered event is a command buffer.  The number of events to
       * process is the same as the interval, unless the interval crosses a
       * frame boundary
       */
      if (buffered_event_count < config.event_interval) {
         /* not enough events */
         return 0;
      }

      /* Imperfect frame tracking requires us to allow for *older* frames */
      if (ringbuffer_peek(rb, config.event_interval - 1)->frame <= start_frame) {
         /* No frame transition.  The next {interval} events should be combined. */
         return config.event_interval;
      }

      /* Else a frame transition occurs within the interval.  Find the
       * transition, so the following line of output begins with the batch
       * that starts the new frame.
       */
      for (int event_index = 1;
           event_index <= config.event_interval;
           ++event_index) {
         if (ringbuffer_peek(rb, event_index)->frame > start_frame)
            return event_index;
      }

      assert(false);
   }

   /* Else we need to search buffered events to find the matching frame
    * transition for our interval.
    */
   assert(config.flags & INTEL_MEASURE_FRAME);
   for (int event_index = 1;
        event_index < buffered_event_count;
        ++event_index) {
      const int latest_frame = ringbuffer_peek(rb, event_index)->frame;
      if (latest_frame - start_frame >= config.event_interval)
         return event_index;
   }

   return 0;
}

/**
 * Take result_count events from the ringbuffer and output them as a single
 * line.
 */
static void
print_combined_results(struct intel_measure_device *measure_device,
                       int result_count,
                       const struct intel_device_info *info)
{
   if (result_count == 0)
      return;

   struct intel_measure_ringbuffer *result_rb = measure_device->ringbuffer;
   assert(ringbuffer_size(result_rb) >= result_count);
   const struct intel_measure_buffered_result* start_result =
      ringbuffer_pop(result_rb);
   const struct intel_measure_buffered_result* current_result = start_result;

   if (start_result == NULL)
      return;
   --result_count;

   uint64_t duration_ts = raw_timestamp_delta(start_result->start_ts,
                                              current_result->end_ts);
   unsigned event_count = start_result->snapshot.event_count;
   while (result_count-- > 0) {
      assert(ringbuffer_size(result_rb) > 0);
      current_result = ringbuffer_pop(result_rb);
      if (current_result == NULL)
         return;
      duration_ts += raw_timestamp_delta(current_result->start_ts,
                                         current_result->end_ts);
      event_count += current_result->snapshot.event_count;
   }

   uint64_t duration_idle_ns =
      intel_device_info_timebase_scale(info, start_result->idle_duration);
   uint64_t duration_time_ns =
      intel_device_info_timebase_scale(info, duration_ts);
   const struct intel_measure_snapshot *begin = &start_result->snapshot;
   uint32_t renderpass = (start_result->primary_renderpass)
      ? start_result->primary_renderpass : begin->renderpass;
   fprintf(config.file, "%"PRIu64",%"PRIu64",%u,%u,%"PRIu64",%u,%u,%u,%s,%u,"
           "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,%.3lf,%.3lf\n",
           start_result->start_ts, current_result->end_ts,
           start_result->frame,
           start_result->batch_count, start_result->batch_size,
           renderpass, start_result->event_index, event_count,
           begin->event_name, begin->count,
           begin->vs, begin->tcs, begin->tes, begin->gs,
           begin->fs, begin->cs, begin->ms, begin->ts,
           (double)duration_idle_ns / 1000.0,
           (double)duration_time_ns / 1000.0);
}

/**
 * Write data for a cpu event.
 */
void
intel_measure_print_cpu_result(unsigned int frame,
                               unsigned int batch_count,
                               uint64_t batch_size,
                               unsigned int event_index,
                               unsigned int event_count,
                               unsigned int count,
                               const char* event_name)
{
   assert(config.cpu_measure);
   uint64_t start_ns = os_time_get_nano();

   fprintf(config.file, "%"PRIu64",%u,%3u,%"PRIu64",%3u,%u,%s,%u\n",
           start_ns, frame, batch_count, batch_size,
           event_index, event_count, event_name, count);
}

/**
 * Empty the ringbuffer of events that can be printed.
 */
static void
intel_measure_print(struct intel_measure_device *device,
                    const struct intel_device_info *info)
{
   while (true) {
      const int events_to_combine = buffered_event_count(device);
      if (events_to_combine == 0)
         break;
      print_combined_results(device, events_to_combine, info);
   }
}

/**
 * Collect snapshots from completed command buffers and submit them to
 * intel_measure for printing.
 */
void
intel_measure_gather(struct intel_measure_device *measure_device,
                     const struct intel_device_info *info)
{
   pthread_mutex_lock(&measure_device->mutex);

   /* Iterate snapshots and collect if ready.  Each snapshot queue will be
    * in-order, but we must determine which queue has the oldest batch.
    */
   /* iterate snapshots and collect if ready */
   while (!list_is_empty(&measure_device->queued_snapshots)) {
      struct intel_measure_batch *batch =
         list_first_entry(&measure_device->queued_snapshots,
                          struct intel_measure_batch, link);

      if (!intel_measure_ready(batch)) {
         /* command buffer has begun execution on the gpu, but has not
          * completed.
          */
         break;
      }

      list_del(&batch->link);
      assert(batch->index % 2 == 0);

      intel_measure_push_result(measure_device, batch);

      batch->index = 0;
      batch->frame = 0;
      if (measure_device->release_batch)
         measure_device->release_batch(batch);
   }

   intel_measure_print(measure_device, info);
   pthread_mutex_unlock(&measure_device->mutex);
}
