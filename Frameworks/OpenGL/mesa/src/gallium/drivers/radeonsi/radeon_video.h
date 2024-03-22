/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef RADEON_VIDEO_H
#define RADEON_VIDEO_H

#include "winsys/radeon_winsys.h"
#include "vl/vl_video_buffer.h"

#define RVID_ERR(fmt, args...)                                                                     \
   fprintf(stderr, "EE %s:%d %s UVD - " fmt, __FILE__, __LINE__, __func__, ##args)

#define UVD_FW_1_66_16 ((1 << 24) | (66 << 16) | (16 << 8))

/* video buffer representation */
struct rvid_buffer {
   unsigned usage;
   struct si_resource *res;
};

/* video buffer offset info representation */
struct rvid_buf_offset_info {
   unsigned num_units;
   unsigned old_offset;
   unsigned new_offset;
};

/* generate an stream handle */
unsigned si_vid_alloc_stream_handle(void);

/* create a buffer in the winsys */
bool si_vid_create_buffer(struct pipe_screen *screen, struct rvid_buffer *buffer, unsigned size,
                          unsigned usage);

/* create a tmz buffer in the winsys */
bool si_vid_create_tmz_buffer(struct pipe_screen *screen, struct rvid_buffer *buffer, unsigned size,
                              unsigned usage);

/* destroy a buffer */
void si_vid_destroy_buffer(struct rvid_buffer *buffer);

/* reallocate a buffer, preserving its content */
bool si_vid_resize_buffer(struct pipe_screen *screen, struct radeon_cmdbuf *cs,
                          struct rvid_buffer *new_buf, unsigned new_size,
                          struct rvid_buf_offset_info *buf_ofst_info);

/* clear the buffer with zeros */
void si_vid_clear_buffer(struct pipe_context *context, struct rvid_buffer *buffer);

#endif // RADEON_VIDEO_H
