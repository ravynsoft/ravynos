/*
 * Copyright © 2014-2015 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file vc4_job.c
 *
 * Functions for submitting VC4 render jobs to the kernel.
 */

#include <xf86drm.h>
#include "vc4_cl_dump.h"
#include "vc4_context.h"
#include "util/hash_table.h"

static void
vc4_job_free(struct vc4_context *vc4, struct vc4_job *job)
{
        struct vc4_bo **referenced_bos = job->bo_pointers.base;
        for (int i = 0; i < cl_offset(&job->bo_handles) / 4; i++) {
                vc4_bo_unreference(&referenced_bos[i]);
        }

        _mesa_hash_table_remove_key(vc4->jobs, &job->key);

        if (job->color_write) {
                _mesa_hash_table_remove_key(vc4->write_jobs,
                                            job->color_write->texture);
                pipe_surface_reference(&job->color_write, NULL);
        }
        if (job->msaa_color_write) {
                _mesa_hash_table_remove_key(vc4->write_jobs,
                                            job->msaa_color_write->texture);
                pipe_surface_reference(&job->msaa_color_write, NULL);
        }
        if (job->zs_write) {
                _mesa_hash_table_remove_key(vc4->write_jobs,
                                            job->zs_write->texture);
                pipe_surface_reference(&job->zs_write, NULL);
        }
        if (job->msaa_zs_write) {
                _mesa_hash_table_remove_key(vc4->write_jobs,
                                            job->msaa_zs_write->texture);
                pipe_surface_reference(&job->msaa_zs_write, NULL);
        }

        pipe_surface_reference(&job->color_read, NULL);
        pipe_surface_reference(&job->zs_read, NULL);

        if (vc4->job == job)
                vc4->job = NULL;

        ralloc_free(job);
}

static struct vc4_job *
vc4_job_create(struct vc4_context *vc4)
{
        struct vc4_job *job = rzalloc(vc4, struct vc4_job);

        vc4_init_cl(job, &job->bcl);
        vc4_init_cl(job, &job->shader_rec);
        vc4_init_cl(job, &job->uniforms);
        vc4_init_cl(job, &job->bo_handles);
        vc4_init_cl(job, &job->bo_pointers);

        job->draw_min_x = ~0;
        job->draw_min_y = ~0;
        job->draw_max_x = 0;
        job->draw_max_y = 0;

        job->last_gem_handle_hindex = ~0;

        if (vc4->perfmon)
                job->perfmon = vc4->perfmon;

        return job;
}

void
vc4_flush_jobs_writing_resource(struct vc4_context *vc4,
                                struct pipe_resource *prsc)
{
        struct hash_entry *entry = _mesa_hash_table_search(vc4->write_jobs,
                                                           prsc);
        if (entry) {
                struct vc4_job *job = entry->data;
                vc4_job_submit(vc4, job);
        }
}

void
vc4_flush_jobs_reading_resource(struct vc4_context *vc4,
                                struct pipe_resource *prsc)
{
        struct vc4_resource *rsc = vc4_resource(prsc);

        vc4_flush_jobs_writing_resource(vc4, prsc);

        hash_table_foreach(vc4->jobs, entry) {
                struct vc4_job *job = entry->data;

                struct vc4_bo **referenced_bos = job->bo_pointers.base;
                bool found = false;
                for (int i = 0; i < cl_offset(&job->bo_handles) / 4; i++) {
                        if (referenced_bos[i] == rsc->bo) {
                                found = true;
                                break;
                        }
                }
                if (found) {
                        vc4_job_submit(vc4, job);
                        continue;
                }

                /* Also check for the Z/color buffers, since the references to
                 * those are only added immediately before submit.
                 */
                if (job->color_read && !(job->cleared & PIPE_CLEAR_COLOR)) {
                        struct vc4_resource *ctex =
                                vc4_resource(job->color_read->texture);
                        if (ctex->bo == rsc->bo) {
                                vc4_job_submit(vc4, job);
                                continue;
                        }
                }

                if (job->zs_read && !(job->cleared &
                                      (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))) {
                        struct vc4_resource *ztex =
                                vc4_resource(job->zs_read->texture);
                        if (ztex->bo == rsc->bo) {
                                vc4_job_submit(vc4, job);
                                continue;
                        }
                }
        }
}

/**
 * Returns a vc4_job structure for tracking V3D rendering to a particular FBO.
 *
 * If we've already started rendering to this FBO, then return old same job,
 * otherwise make a new one.  If we're beginning rendering to an FBO, make
 * sure that any previous reads of the FBO (or writes to its color/Z surfaces)
 * have been flushed.
 */
struct vc4_job *
vc4_get_job(struct vc4_context *vc4,
            struct pipe_surface *cbuf, struct pipe_surface *zsbuf)
{
        /* Return the existing job for this FBO if we have one */
        struct vc4_job_key local_key = {.cbuf = cbuf, .zsbuf = zsbuf};
        struct hash_entry *entry = _mesa_hash_table_search(vc4->jobs,
                                                           &local_key);
        if (entry)
                return entry->data;

        /* Creating a new job.  Make sure that any previous jobs reading or
         * writing these buffers are flushed.
         */
        if (cbuf)
                vc4_flush_jobs_reading_resource(vc4, cbuf->texture);
        if (zsbuf)
                vc4_flush_jobs_reading_resource(vc4, zsbuf->texture);

        struct vc4_job *job = vc4_job_create(vc4);

        if (cbuf) {
                if (cbuf->texture->nr_samples > 1) {
                        job->msaa = true;
                        pipe_surface_reference(&job->msaa_color_write, cbuf);
                } else {
                        pipe_surface_reference(&job->color_write, cbuf);
                }
        }

        if (zsbuf) {
                if (zsbuf->texture->nr_samples > 1) {
                        job->msaa = true;
                        pipe_surface_reference(&job->msaa_zs_write, zsbuf);
                } else {
                        pipe_surface_reference(&job->zs_write, zsbuf);
                }
        }

        if (job->msaa) {
                job->tile_width = 32;
                job->tile_height = 32;
        } else {
                job->tile_width = 64;
                job->tile_height = 64;
        }

        if (cbuf)
                _mesa_hash_table_insert(vc4->write_jobs, cbuf->texture, job);
        if (zsbuf)
                _mesa_hash_table_insert(vc4->write_jobs, zsbuf->texture, job);

        job->key.cbuf = cbuf;
        job->key.zsbuf = zsbuf;
        _mesa_hash_table_insert(vc4->jobs, &job->key, job);

        return job;
}

struct vc4_job *
vc4_get_job_for_fbo(struct vc4_context *vc4)
{
        if (vc4->job)
                return vc4->job;

        struct pipe_surface *cbuf = vc4->framebuffer.cbufs[0];
        struct pipe_surface *zsbuf = vc4->framebuffer.zsbuf;
        struct vc4_job *job = vc4_get_job(vc4, cbuf, zsbuf);

        /* The dirty flags are tracking what's been updated while vc4->job has
         * been bound, so set them all to ~0 when switching between jobs.  We
         * also need to reset all state at the start of rendering.
         */
        vc4->dirty = ~0;

        /* Set up the read surfaces in the job.  If they aren't actually
         * getting read (due to a clear starting the frame), job->cleared will
         * mask out the read.
         */
        pipe_surface_reference(&job->color_read, cbuf);
        pipe_surface_reference(&job->zs_read, zsbuf);

        /* If we're binding to uninitialized buffers, no need to load their
         * contents before drawing.
         */
        if (cbuf) {
                struct vc4_resource *rsc = vc4_resource(cbuf->texture);
                if (!rsc->writes)
                        job->cleared |= PIPE_CLEAR_COLOR0;
        }

        if (zsbuf) {
                struct vc4_resource *rsc = vc4_resource(zsbuf->texture);
                if (!rsc->writes)
                        job->cleared |= PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL;
        }

        job->draw_tiles_x = DIV_ROUND_UP(vc4->framebuffer.width,
                                         job->tile_width);
        job->draw_tiles_y = DIV_ROUND_UP(vc4->framebuffer.height,
                                         job->tile_height);

        /* Initialize the job with the raster order flags -- each draw will
         * check that we haven't changed the flags, since that requires a
         * flush.
         */
        if (vc4->rasterizer)
                job->flags = vc4->rasterizer->tile_raster_order_flags;

        vc4->job = job;

        return job;
}

static void
vc4_submit_setup_rcl_surface(struct vc4_job *job,
                             struct drm_vc4_submit_rcl_surface *submit_surf,
                             struct pipe_surface *psurf,
                             bool is_depth, bool is_write)
{
        struct vc4_surface *surf = vc4_surface(psurf);

        if (!surf)
                return;

        struct vc4_resource *rsc = vc4_resource(psurf->texture);
        submit_surf->hindex = vc4_gem_hindex(job, rsc->bo);
        submit_surf->offset = surf->offset;

        if (psurf->texture->nr_samples <= 1) {
                if (is_depth) {
                        submit_surf->bits =
                                VC4_SET_FIELD(VC4_LOADSTORE_TILE_BUFFER_ZS,
                                              VC4_LOADSTORE_TILE_BUFFER_BUFFER);

                } else {
                        submit_surf->bits =
                                VC4_SET_FIELD(VC4_LOADSTORE_TILE_BUFFER_COLOR,
                                              VC4_LOADSTORE_TILE_BUFFER_BUFFER) |
                                VC4_SET_FIELD(vc4_rt_format_is_565(psurf->format) ?
                                              VC4_LOADSTORE_TILE_BUFFER_BGR565 :
                                              VC4_LOADSTORE_TILE_BUFFER_RGBA8888,
                                              VC4_LOADSTORE_TILE_BUFFER_FORMAT);
                }
                submit_surf->bits |=
                        VC4_SET_FIELD(surf->tiling,
                                      VC4_LOADSTORE_TILE_BUFFER_TILING);
        } else {
                assert(!is_write);
                submit_surf->flags |= VC4_SUBMIT_RCL_SURFACE_READ_IS_FULL_RES;
        }

        if (is_write)
                rsc->writes++;
}

static void
vc4_submit_setup_rcl_render_config_surface(struct vc4_job *job,
                                           struct drm_vc4_submit_rcl_surface *submit_surf,
                                           struct pipe_surface *psurf)
{
        struct vc4_surface *surf = vc4_surface(psurf);

        if (!surf)
                return;

        struct vc4_resource *rsc = vc4_resource(psurf->texture);
        submit_surf->hindex = vc4_gem_hindex(job, rsc->bo);
        submit_surf->offset = surf->offset;

        if (psurf->texture->nr_samples <= 1) {
                submit_surf->bits =
                        VC4_SET_FIELD(vc4_rt_format_is_565(surf->base.format) ?
                                      VC4_RENDER_CONFIG_FORMAT_BGR565 :
                                      VC4_RENDER_CONFIG_FORMAT_RGBA8888,
                                      VC4_RENDER_CONFIG_FORMAT) |
                        VC4_SET_FIELD(surf->tiling,
                                      VC4_RENDER_CONFIG_MEMORY_FORMAT);
        }

        rsc->writes++;
}

static void
vc4_submit_setup_rcl_msaa_surface(struct vc4_job *job,
                                  struct drm_vc4_submit_rcl_surface *submit_surf,
                                  struct pipe_surface *psurf)
{
        struct vc4_surface *surf = vc4_surface(psurf);

        if (!surf)
                return;

        struct vc4_resource *rsc = vc4_resource(psurf->texture);
        submit_surf->hindex = vc4_gem_hindex(job, rsc->bo);
        submit_surf->offset = surf->offset;
        submit_surf->bits = 0;
        rsc->writes++;
}

/**
 * Submits the job to the kernel and then reinitializes it.
 */
void
vc4_job_submit(struct vc4_context *vc4, struct vc4_job *job)
{
        if (!job->needs_flush)
                goto done;

        /* The RCL setup would choke if the draw bounds cause no drawing, so
         * just drop the drawing if that's the case.
         */
        if (job->draw_max_x <= job->draw_min_x ||
            job->draw_max_y <= job->draw_min_y) {
                goto done;
        }

        if (VC4_DBG(CL)) {
                fprintf(stderr, "BCL:\n");
                vc4_dump_cl(job->bcl.base, cl_offset(&job->bcl), false);
        }

        if (cl_offset(&job->bcl) > 0) {
                /* Increment the semaphore indicating that binning is done and
                 * unblocking the render thread.  Note that this doesn't act
                 * until the FLUSH completes.
                 */
                cl_ensure_space(&job->bcl, 8);
                cl_emit(&job->bcl, INCREMENT_SEMAPHORE, incr);
                /* The FLUSH caps all of our bin lists with a
                 * VC4_PACKET_RETURN.
                 */
                cl_emit(&job->bcl, FLUSH, flush);
        }
        struct drm_vc4_submit_cl submit = {
                .color_read.hindex = ~0,
                .zs_read.hindex = ~0,
                .color_write.hindex = ~0,
                .msaa_color_write.hindex = ~0,
                .zs_write.hindex = ~0,
                .msaa_zs_write.hindex = ~0,
        };

        cl_ensure_space(&job->bo_handles, 6 * sizeof(uint32_t));
        cl_ensure_space(&job->bo_pointers, 6 * sizeof(struct vc4_bo *));

        if (job->resolve & PIPE_CLEAR_COLOR) {
                if (!(job->cleared & PIPE_CLEAR_COLOR)) {
                        vc4_submit_setup_rcl_surface(job, &submit.color_read,
                                                     job->color_read,
                                                     false, false);
                }
                vc4_submit_setup_rcl_render_config_surface(job,
                                                           &submit.color_write,
                                                           job->color_write);
                vc4_submit_setup_rcl_msaa_surface(job,
                                                  &submit.msaa_color_write,
                                                  job->msaa_color_write);
        }
        if (job->resolve & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL)) {
                if (!(job->cleared & (PIPE_CLEAR_DEPTH | PIPE_CLEAR_STENCIL))) {
                        vc4_submit_setup_rcl_surface(job, &submit.zs_read,
                                                     job->zs_read, true, false);
                }
                vc4_submit_setup_rcl_surface(job, &submit.zs_write,
                                             job->zs_write, true, true);
                vc4_submit_setup_rcl_msaa_surface(job, &submit.msaa_zs_write,
                                                  job->msaa_zs_write);
        }

        if (job->msaa) {
                /* This bit controls how many pixels the general
                 * (i.e. subsampled) loads/stores are iterating over
                 * (multisample loads replicate out to the other samples).
                 */
                submit.color_write.bits |= VC4_RENDER_CONFIG_MS_MODE_4X;
                /* Controls whether color_write's
                 * VC4_PACKET_STORE_MS_TILE_BUFFER does 4x decimation
                 */
                submit.color_write.bits |= VC4_RENDER_CONFIG_DECIMATE_MODE_4X;
        }

        submit.bo_handles = (uintptr_t)job->bo_handles.base;
        submit.bo_handle_count = cl_offset(&job->bo_handles) / 4;
        submit.bin_cl = (uintptr_t)job->bcl.base;
        submit.bin_cl_size = cl_offset(&job->bcl);
        submit.shader_rec = (uintptr_t)job->shader_rec.base;
        submit.shader_rec_size = cl_offset(&job->shader_rec);
        submit.shader_rec_count = job->shader_rec_count;
        submit.uniforms = (uintptr_t)job->uniforms.base;
        submit.uniforms_size = cl_offset(&job->uniforms);
	if (job->perfmon)
		submit.perfmonid = job->perfmon->id;

        assert(job->draw_min_x != ~0 && job->draw_min_y != ~0);
        submit.min_x_tile = job->draw_min_x / job->tile_width;
        submit.min_y_tile = job->draw_min_y / job->tile_height;
        submit.max_x_tile = (job->draw_max_x - 1) / job->tile_width;
        submit.max_y_tile = (job->draw_max_y - 1) / job->tile_height;
        submit.width = job->draw_width;
        submit.height = job->draw_height;
        if (job->cleared) {
                submit.flags |= VC4_SUBMIT_CL_USE_CLEAR_COLOR;
                submit.clear_color[0] = job->clear_color[0];
                submit.clear_color[1] = job->clear_color[1];
                submit.clear_z = job->clear_depth;
                submit.clear_s = job->clear_stencil;
        }
        submit.flags |= job->flags;

        if (vc4->screen->has_syncobj) {
                submit.out_sync = vc4->job_syncobj;

                if (vc4->in_fence_fd >= 0) {
                        /* This replaces the fence in the syncobj. */
                        drmSyncobjImportSyncFile(vc4->fd, vc4->in_syncobj,
                                                 vc4->in_fence_fd);
                        submit.in_sync = vc4->in_syncobj;
                        close(vc4->in_fence_fd);
                        vc4->in_fence_fd = -1;
                }
        }

        if (!VC4_DBG(NORAST)) {
                int ret;

                ret = vc4_ioctl(vc4->fd, DRM_IOCTL_VC4_SUBMIT_CL, &submit);
                static bool warned = false;
                if (ret && !warned) {
                        fprintf(stderr, "Draw call returned %s.  "
                                        "Expect corruption.\n", strerror(errno));
                        warned = true;
                } else if (!ret) {
                        vc4->last_emit_seqno = submit.seqno;
                        if (job->perfmon)
                                job->perfmon->last_seqno = submit.seqno;
                }
        }

        if (vc4->last_emit_seqno - vc4->screen->finished_seqno > 5) {
                if (!vc4_wait_seqno(vc4->screen,
                                    vc4->last_emit_seqno - 5,
                                    OS_TIMEOUT_INFINITE,
                                    "job throttling")) {
                        fprintf(stderr, "Job throttling failed\n");
                }
        }

        if (VC4_DBG(ALWAYS_SYNC)) {
                if (!vc4_wait_seqno(vc4->screen, vc4->last_emit_seqno,
                                    OS_TIMEOUT_INFINITE, "sync")) {
                        fprintf(stderr, "Wait failed.\n");
                        abort();
                }
        }

done:
        vc4_job_free(vc4, job);
}

static bool
vc4_job_compare(const void *a, const void *b)
{
        return memcmp(a, b, sizeof(struct vc4_job_key)) == 0;
}

static uint32_t
vc4_job_hash(const void *key)
{
        return _mesa_hash_data(key, sizeof(struct vc4_job_key));
}

int
vc4_job_init(struct vc4_context *vc4)
{
        vc4->jobs = _mesa_hash_table_create(vc4,
                                            vc4_job_hash,
                                            vc4_job_compare);
        vc4->write_jobs = _mesa_hash_table_create(vc4,
                                                  _mesa_hash_pointer,
                                                  _mesa_key_pointer_equal);

        if (vc4->screen->has_syncobj) {
                /* Create the syncobj as signaled since with no job executed
                 * there is nothing to wait on.
                 */
                int ret = drmSyncobjCreate(vc4->fd,
                                           DRM_SYNCOBJ_CREATE_SIGNALED,
                                           &vc4->job_syncobj);
                if (ret) {
                        /* If the screen indicated syncobj support, we should
                         * be able to create a signaled syncobj.
                         * At this point it is too late to pretend the screen
                         * has no syncobj support.
                         */
                        return ret;
                }
        }

        return 0;
}

