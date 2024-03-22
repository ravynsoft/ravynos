/*
 * Copyright 2009 VMware, Inc.
 * Copyright 2019 Collabora Ltd.
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

/* virgl staging buffer manager, heavily inspired by u_upload_mgr. */

#ifndef VIRGL_STAGING_MGR_H
#define VIRGL_STAGING_MGR_H

struct pipe_context;
struct virgl_hw_res;
struct virgl_winsys;

#ifdef __cplusplus
extern "C" {
#endif

struct virgl_staging_mgr {
   struct virgl_winsys *vws;
   unsigned default_size;  /* Minimum size of the staging buffer, in bytes. */
   struct virgl_hw_res *hw_res;   /* Staging buffer hw_res. */
   unsigned size;   /* Current staging buffer size. */
   uint8_t *map;    /* Pointer to the mapped staging buffer. */
   unsigned offset; /* Offset pointing at the first unused buffer byte. */
};

/**
 * Init the staging manager.
 *
 * \param staging       Pointer to the staging manager to initialize.
 * \param pipe          Pipe driver.
 * \param default_size  Minimum size of the staging buffer, in bytes.
 */
void
virgl_staging_init(struct virgl_staging_mgr *staging, struct pipe_context *pipe,
                   unsigned default_size);

/**
 * Destroy the staging manager.
 */
void
virgl_staging_destroy(struct virgl_staging_mgr *staging);

/**
 * Sub-allocate new memory from the staging buffer.
 *
 * The returned pointer to the buffer references the internal staging buffer.
 *
 * The returned pointer to staging buffer memory is guaranteed to be valid
 * for as long as the returned buffer is still referenced.
 *
 * \param staging          Staging manager
 * \param size             Size of the allocation.
 * \param alignment        Alignment of the suballocation within the buffer.
 * \param out_offset       Pointer to where the new buffer offset will be returned.
 * \param outbuf           Pointer to where the staging buffer will be returned.
 * \param ptr              Pointer to the allocated memory that is returned.
 * \return                 Whether the allocation is successful.
 */
bool
virgl_staging_alloc(struct virgl_staging_mgr *staging,
                    unsigned size,
                    unsigned alignment,
                    unsigned *out_offset,
                    struct virgl_hw_res **outbuf,
                    void **ptr);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
