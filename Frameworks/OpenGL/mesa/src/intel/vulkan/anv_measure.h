/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ANV_MEASURE_H
#define ANV_MEASURE_H

#include "anv_private.h"
#include "common/intel_measure.h"

void anv_measure_device_init(struct anv_physical_device *device);
void anv_measure_device_destroy(struct anv_physical_device *device);

void anv_measure_init(struct anv_cmd_buffer *cmd_buffer);
void anv_measure_destroy(struct anv_cmd_buffer *cmd_buffer);
void anv_measure_reset(struct anv_cmd_buffer *cmd_buffer);

void _anv_measure_snapshot(struct anv_cmd_buffer *cmd_buffer,
                           enum intel_measure_snapshot_type type,
                           const char *event_name,
                           uint32_t count);

/* ends snapshots before command buffer submission */
void _anv_measure_endcommandbuffer(struct anv_cmd_buffer *cmd_buffer);

/* when measuring render passes, inserts a timestamp */
void _anv_measure_beginrenderpass(struct anv_cmd_buffer *cmd_buffer);

/* tracks frame progression */
void _anv_measure_acquire(struct anv_device *device);

/* should be combined with endcommandbuffer */
void _anv_measure_submit(struct anv_cmd_buffer *cmd_buffer);

void
_anv_measure_add_secondary(struct anv_cmd_buffer *primary,
                           struct anv_cmd_buffer *secondary);

#define anv_measure_acquire(device) \
   if (unlikely(device->physical->measure_device.config)) \
      _anv_measure_acquire(device)

#define anv_measure_snapshot(cmd_buffer, type, event_name, count) \
   if (unlikely(cmd_buffer->measure)) \
      _anv_measure_snapshot(cmd_buffer, type, event_name, count)

#define anv_measure_endcommandbuffer(cmd_buffer) \
   if (unlikely(cmd_buffer->measure)) \
      _anv_measure_endcommandbuffer(cmd_buffer)

#define anv_measure_beginrenderpass(cmd_buffer) \
   if (unlikely(cmd_buffer->measure)) \
      _anv_measure_beginrenderpass(cmd_buffer)

#define anv_measure_submit(cmd_buffer) \
   if (unlikely(cmd_buffer->measure)) \
      _anv_measure_submit(cmd_buffer)

#define anv_measure_add_secondary(primary, secondary) \
   if (unlikely(primary->measure)) \
      _anv_measure_add_secondary(primary, secondary)

#endif   /* ANV_MEASURE_H */
