/*
 * Copyright © 2014-2017 Broadcom
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

/** @file v3d_job.c
 *
 * Functions for submitting V3D render jobs to the kernel.
 */

#include <xf86drm.h>
#include <libsync.h>
#include "v3d_context.h"
/* The OQ/semaphore packets are the same across V3D versions. */
#define V3D_VERSION 42
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/common/v3d_macros.h"
#include "util/hash_table.h"
#include "util/ralloc.h"
#include "util/set.h"
#include "broadcom/clif/clif_dump.h"

void
v3d_job_free(struct v3d_context *v3d, struct v3d_job *job)
{
        set_foreach(job->bos, entry) {
                struct v3d_bo *bo = (struct v3d_bo *)entry->key;
                v3d_bo_unreference(&bo);
        }

        _mesa_hash_table_remove_key(v3d->jobs, &job->key);

        if (job->write_prscs) {
                set_foreach(job->write_prscs, entry) {
                        const struct pipe_resource *prsc = entry->key;

                        _mesa_hash_table_remove_key(v3d->write_jobs, prsc);
                }
        }

        for (int i = 0; i < job->nr_cbufs; i++) {
                if (job->cbufs[i]) {
                        _mesa_hash_table_remove_key(v3d->write_jobs,
                                                    job->cbufs[i]->texture);
                        pipe_surface_reference(&job->cbufs[i], NULL);
                }
        }
        if (job->zsbuf) {
                struct v3d_resource *rsc = v3d_resource(job->zsbuf->texture);
                if (rsc->separate_stencil)
                        _mesa_hash_table_remove_key(v3d->write_jobs,
                                                    &rsc->separate_stencil->base);

                _mesa_hash_table_remove_key(v3d->write_jobs,
                                            job->zsbuf->texture);
                pipe_surface_reference(&job->zsbuf, NULL);
        }
        if (job->bbuf)
                pipe_surface_reference(&job->bbuf, NULL);

        if (v3d->job == job)
                v3d->job = NULL;

        v3d_destroy_cl(&job->bcl);
        v3d_destroy_cl(&job->rcl);
        v3d_destroy_cl(&job->indirect);
        v3d_bo_unreference(&job->tile_alloc);
        v3d_bo_unreference(&job->tile_state);

        ralloc_free(job);
}

struct v3d_job *
v3d_job_create(struct v3d_context *v3d)
{
        struct v3d_job *job = rzalloc(v3d, struct v3d_job);

        job->v3d = v3d;

        v3d_init_cl(job, &job->bcl);
        v3d_init_cl(job, &job->rcl);
        v3d_init_cl(job, &job->indirect);

        job->draw_min_x = ~0;
        job->draw_min_y = ~0;
        job->draw_max_x = 0;
        job->draw_max_y = 0;

        job->bos = _mesa_set_create(job,
                                    _mesa_hash_pointer,
                                    _mesa_key_pointer_equal);
        return job;
}

void
v3d_job_add_bo(struct v3d_job *job, struct v3d_bo *bo)
{
        if (!bo)
                return;

        if (_mesa_set_search(job->bos, bo))
                return;

        v3d_bo_reference(bo);
        _mesa_set_add(job->bos, bo);
        job->referenced_size += bo->size;

        uint32_t *bo_handles = (void *)(uintptr_t)job->submit.bo_handles;

        if (job->submit.bo_handle_count >= job->bo_handles_size) {
                job->bo_handles_size = MAX2(4, job->bo_handles_size * 2);
                bo_handles = reralloc(job, bo_handles,
                                      uint32_t, job->bo_handles_size);
                job->submit.bo_handles = (uintptr_t)(void *)bo_handles;
        }
        bo_handles[job->submit.bo_handle_count++] = bo->handle;
}

void
v3d_job_add_write_resource(struct v3d_job *job, struct pipe_resource *prsc)
{
        struct v3d_context *v3d = job->v3d;

        if (!job->write_prscs) {
                job->write_prscs = _mesa_set_create(job,
                                                    _mesa_hash_pointer,
                                                    _mesa_key_pointer_equal);
        }

        _mesa_set_add(job->write_prscs, prsc);
        _mesa_hash_table_insert(v3d->write_jobs, prsc, job);
}

void
v3d_flush_jobs_using_bo(struct v3d_context *v3d, struct v3d_bo *bo)
{
        hash_table_foreach(v3d->jobs, entry) {
                struct v3d_job *job = entry->data;

                if (_mesa_set_search(job->bos, bo))
                        v3d_job_submit(v3d, job);
        }
}

void
v3d_job_add_tf_write_resource(struct v3d_job *job, struct pipe_resource *prsc)
{
        v3d_job_add_write_resource(job, prsc);

        if (!job->tf_write_prscs)
                job->tf_write_prscs = _mesa_pointer_set_create(job);

        _mesa_set_add(job->tf_write_prscs, prsc);
}

static bool
v3d_job_writes_resource_from_tf(struct v3d_job *job,
                                struct pipe_resource *prsc)
{
        if (!job->tf_enabled)
                return false;

        if (!job->tf_write_prscs)
                return false;

        return _mesa_set_search(job->tf_write_prscs, prsc) != NULL;
}

void
v3d_flush_jobs_writing_resource(struct v3d_context *v3d,
                                struct pipe_resource *prsc,
                                enum v3d_flush_cond flush_cond,
                                bool is_compute_pipeline)
{
        struct hash_entry *entry = _mesa_hash_table_search(v3d->write_jobs,
                                                           prsc);
        struct v3d_resource *rsc = v3d_resource(prsc);

        /* We need to sync if graphics pipeline reads a resource written
         * by the compute pipeline. The same would be needed for the case of
         * graphics-compute dependency but nowadays all compute jobs
         * are serialized with the previous submitted job.
         */
        if (!is_compute_pipeline && rsc->bo != NULL && rsc->compute_written) {
           v3d->sync_on_last_compute_job = true;
           rsc->compute_written = false;
        }

        if (!entry)
                return;

        struct v3d_job *job = entry->data;

        bool needs_flush;
        switch (flush_cond) {
        case V3D_FLUSH_ALWAYS:
                needs_flush = true;
                break;
        case V3D_FLUSH_NOT_CURRENT_JOB:
                needs_flush = !v3d->job || v3d->job != job;
                break;
        case V3D_FLUSH_DEFAULT:
        default:
                /* For writes from TF in the same job we use the "Wait for TF"
                 * feature provided by the hardware so we don't want to flush.
                 * The exception to this is when the caller is about to map the
                 * resource since in that case we don't have a 'Wait for TF'
                 * command the in command stream. In this scenario the caller
                 * is expected to set 'always_flush' to True.
                 */
                needs_flush = !v3d_job_writes_resource_from_tf(job, prsc);
        }

        if (needs_flush)
                v3d_job_submit(v3d, job);
}

void
v3d_flush_jobs_reading_resource(struct v3d_context *v3d,
                                struct pipe_resource *prsc,
                                enum v3d_flush_cond flush_cond,
                                bool is_compute_pipeline)
{
        struct v3d_resource *rsc = v3d_resource(prsc);

        /* We only need to force the flush on TF writes, which is the only
         * case where we might skip the flush to use the 'Wait for TF'
         * command. Here we are flushing for a read, which means that the
         * caller intends to write to the resource, so we don't care if
         * there was a previous TF write to it.
         */
        v3d_flush_jobs_writing_resource(v3d, prsc, flush_cond,
                                        is_compute_pipeline);

        hash_table_foreach(v3d->jobs, entry) {
                struct v3d_job *job = entry->data;

                if (!_mesa_set_search(job->bos, rsc->bo))
                        continue;

                bool needs_flush;
                switch (flush_cond) {
                case V3D_FLUSH_NOT_CURRENT_JOB:
                        needs_flush = !v3d->job || v3d->job != job;
                        break;
                case V3D_FLUSH_ALWAYS:
                case V3D_FLUSH_DEFAULT:
                default:
                        needs_flush = true;
                }

                if (needs_flush)
                        v3d_job_submit(v3d, job);

                /* Reminder: v3d->jobs is safe to keep iterating even
                 * after deletion of an entry.
                 */
                continue;
        }
}

/**
 * Returns a v3d_job structure for tracking V3D rendering to a particular FBO.
 *
 * If we've already started rendering to this FBO, then return the same job,
 * otherwise make a new one.  If we're beginning rendering to an FBO, make
 * sure that any previous reads of the FBO (or writes to its color/Z surfaces)
 * have been flushed.
 */
struct v3d_job *
v3d_get_job(struct v3d_context *v3d,
            uint32_t nr_cbufs,
            struct pipe_surface **cbufs,
            struct pipe_surface *zsbuf,
            struct pipe_surface *bbuf)
{
        /* Return the existing job for this FBO if we have one */
        struct v3d_job_key local_key = {
                .cbufs = {
                        cbufs[0],
                        cbufs[1],
                        cbufs[2],
                        cbufs[3],
                },
                .zsbuf = zsbuf,
                .bbuf = bbuf,
        };
        struct hash_entry *entry = _mesa_hash_table_search(v3d->jobs,
                                                           &local_key);
        if (entry)
                return entry->data;

        /* Creating a new job.  Make sure that any previous jobs reading or
         * writing these buffers are flushed.
         */
        struct v3d_job *job = v3d_job_create(v3d);
        job->nr_cbufs = nr_cbufs;

        for (int i = 0; i < job->nr_cbufs; i++) {
                if (cbufs[i]) {
                        v3d_flush_jobs_reading_resource(v3d, cbufs[i]->texture,
                                                        V3D_FLUSH_DEFAULT,
                                                        false);
                        pipe_surface_reference(&job->cbufs[i], cbufs[i]);

                        if (cbufs[i]->texture->nr_samples > 1)
                                job->msaa = true;
                }
        }
        if (zsbuf) {
                v3d_flush_jobs_reading_resource(v3d, zsbuf->texture,
                                                V3D_FLUSH_DEFAULT,
                                                false);
                pipe_surface_reference(&job->zsbuf, zsbuf);
                if (zsbuf->texture->nr_samples > 1)
                        job->msaa = true;
        }
        if (bbuf) {
                pipe_surface_reference(&job->bbuf, bbuf);
                if (bbuf->texture->nr_samples > 1)
                        job->msaa = true;
        }

        for (int i = 0; i < job->nr_cbufs; i++) {
                if (cbufs[i])
                        _mesa_hash_table_insert(v3d->write_jobs,
                                                cbufs[i]->texture, job);
        }
        if (zsbuf) {
                _mesa_hash_table_insert(v3d->write_jobs, zsbuf->texture, job);

                struct v3d_resource *rsc = v3d_resource(zsbuf->texture);
                if (rsc->separate_stencil) {
                        v3d_flush_jobs_reading_resource(v3d,
                                                        &rsc->separate_stencil->base,
                                                        V3D_FLUSH_DEFAULT,
                                                        false);
                        _mesa_hash_table_insert(v3d->write_jobs,
                                                &rsc->separate_stencil->base,
                                                job);
                }
        }

       job->double_buffer = V3D_DBG(DOUBLE_BUFFER) && !job->msaa;

        memcpy(&job->key, &local_key, sizeof(local_key));
        _mesa_hash_table_insert(v3d->jobs, &job->key, job);

        return job;
}

struct v3d_job *
v3d_get_job_for_fbo(struct v3d_context *v3d)
{
        if (v3d->job)
                return v3d->job;

        uint32_t nr_cbufs = v3d->framebuffer.nr_cbufs;
        struct pipe_surface **cbufs = v3d->framebuffer.cbufs;
        struct pipe_surface *zsbuf = v3d->framebuffer.zsbuf;
        struct v3d_job *job = v3d_get_job(v3d, nr_cbufs, cbufs, zsbuf, NULL);

        if (v3d->framebuffer.samples >= 1) {
                job->msaa = true;
                job->double_buffer = false;
        }

        v3d_get_tile_buffer_size(&v3d->screen->devinfo,
                                 job->msaa, job->double_buffer,
                                 job->nr_cbufs, job->cbufs, job->bbuf,
                                 &job->tile_width,
                                 &job->tile_height,
                                 &job->internal_bpp);

        /* The dirty flags are tracking what's been updated while v3d->job has
         * been bound, so set them all to ~0 when switching between jobs.  We
         * also need to reset all state at the start of rendering.
         */
        v3d->dirty = ~0;

        /* If we're binding to uninitialized buffers, no need to load their
         * contents before drawing.
         */
        for (int i = 0; i < nr_cbufs; i++) {
                if (cbufs[i]) {
                        struct v3d_resource *rsc = v3d_resource(cbufs[i]->texture);
                        if (!rsc->writes)
                                job->clear |= PIPE_CLEAR_COLOR0 << i;
                }
        }

        if (zsbuf) {
                struct v3d_resource *rsc = v3d_resource(zsbuf->texture);
                if (!rsc->writes)
                        job->clear |= PIPE_CLEAR_DEPTH;

                if (rsc->separate_stencil)
                        rsc = rsc->separate_stencil;

                if (!rsc->writes)
                        job->clear |= PIPE_CLEAR_STENCIL;
        }

        job->draw_tiles_x = DIV_ROUND_UP(v3d->framebuffer.width,
                                         job->tile_width);
        job->draw_tiles_y = DIV_ROUND_UP(v3d->framebuffer.height,
                                         job->tile_height);

        v3d->job = job;

        return job;
}

static void
v3d_clif_dump(struct v3d_context *v3d, struct v3d_job *job)
{
        if (!(V3D_DBG(CL) ||
              V3D_DBG(CL_NO_BIN) ||
              V3D_DBG(CLIF)))
                return;

        struct clif_dump *clif = clif_dump_init(&v3d->screen->devinfo,
                                                stderr,
                                                V3D_DBG(CL) ||
                                                V3D_DBG(CL_NO_BIN),
                                                V3D_DBG(CL_NO_BIN));

        set_foreach(job->bos, entry) {
                struct v3d_bo *bo = (void *)entry->key;
                char *name = ralloc_asprintf(NULL, "%s_0x%x",
                                             bo->name, bo->offset);

                v3d_bo_map(bo);
                clif_dump_add_bo(clif, name, bo->offset, bo->size, bo->map);

                ralloc_free(name);
        }

        clif_dump(clif, &job->submit);

        clif_dump_destroy(clif);
}

static void
v3d_read_and_accumulate_primitive_counters(struct v3d_context *v3d)
{
        assert(v3d->prim_counts);

        perf_debug("stalling on TF counts readback\n");
        struct v3d_resource *rsc = v3d_resource(v3d->prim_counts);
        if (v3d_bo_wait(rsc->bo, OS_TIMEOUT_INFINITE, "prim-counts")) {
                uint32_t *map = v3d_bo_map(rsc->bo) + v3d->prim_counts_offset;
                v3d->tf_prims_generated += map[V3D_PRIM_COUNTS_TF_WRITTEN];
                /* When we only have a vertex shader with no primitive
                 * restart, we determine the primitive count in the CPU so
                 * don't update it here again.
                 */
                if (v3d->prog.gs || v3d->prim_restart) {
                        v3d->prims_generated += map[V3D_PRIM_COUNTS_WRITTEN];
                        uint8_t prim_mode =
                                v3d->prog.gs ? v3d->prog.gs->prog_data.gs->out_prim_type
                                             : v3d->prim_mode;
                        uint32_t vertices_written =
                                map[V3D_PRIM_COUNTS_TF_WRITTEN] * mesa_vertices_per_prim(prim_mode);
                        for (int i = 0; i < v3d->streamout.num_targets; i++) {
                                v3d_stream_output_target(v3d->streamout.targets[i])->offset +=
                                        vertices_written;
                        }
                }
        }
}

/**
 * Submits the job to the kernel and then reinitializes it.
 */
void
v3d_job_submit(struct v3d_context *v3d, struct v3d_job *job)
{
        struct v3d_screen *screen = v3d->screen;
        struct v3d_device_info *devinfo = &screen->devinfo;

        if (!job->needs_flush)
                goto done;

        /* The GL_PRIMITIVES_GENERATED query is included with
         * OES_geometry_shader.
         */
        job->needs_primitives_generated =
                v3d->n_primitives_generated_queries_in_flight > 0 &&
                v3d->prog.gs;

        if (job->needs_primitives_generated)
                v3d_ensure_prim_counts_allocated(v3d);

        v3d_X(devinfo, emit_rcl)(job);

        if (cl_offset(&job->bcl) > 0)
                v3d_X(devinfo, bcl_epilogue)(v3d, job);

        if (v3d->in_fence_fd >= 0) {
                /* PIPE_CAP_NATIVE_FENCE */
                if (drmSyncobjImportSyncFile(v3d->fd, v3d->in_syncobj,
                                             v3d->in_fence_fd)) {
                   fprintf(stderr, "Failed to import native fence.\n");
                } else {
                   job->submit.in_sync_bcl = v3d->in_syncobj;
                }
                close(v3d->in_fence_fd);
                v3d->in_fence_fd = -1;
        } else {
                /* While the RCL will implicitly depend on the last RCL to have
                 * finished, we also need to block on any previous TFU job we
                 * may have dispatched.
                 */
                job->submit.in_sync_rcl = v3d->out_sync;
        }

        /* Update the sync object for the last rendering by our context. */
        job->submit.out_sync = v3d->out_sync;

        job->submit.bcl_end = job->bcl.bo->offset + cl_offset(&job->bcl);
        job->submit.rcl_end = job->rcl.bo->offset + cl_offset(&job->rcl);

        if (v3d->active_perfmon) {
                assert(screen->has_perfmon);
                job->submit.perfmon_id = v3d->active_perfmon->kperfmon_id;
        }

        /* If we are submitting a job with a different perfmon, we need to
         * ensure the previous one fully finishes before starting this;
         * otherwise it would wrongly mix counter results.
         */
        if (v3d->active_perfmon != v3d->last_perfmon) {
                v3d->last_perfmon = v3d->active_perfmon;
                job->submit.in_sync_bcl = v3d->out_sync;
        }

        job->submit.flags = 0;
        if (job->tmu_dirty_rcl && screen->has_cache_flush)
                job->submit.flags |= DRM_V3D_SUBMIT_CL_FLUSH_CACHE;

        /* On V3D 4.1, the tile alloc/state setup moved to register writes
         * instead of binner packets.
         */
        if (devinfo->ver >= 42) {
                v3d_job_add_bo(job, job->tile_alloc);
                job->submit.qma = job->tile_alloc->offset;
                job->submit.qms = job->tile_alloc->size;

                v3d_job_add_bo(job, job->tile_state);
                job->submit.qts = job->tile_state->offset;
        }

        v3d_clif_dump(v3d, job);

        if (!V3D_DBG(NORAST)) {
                int ret;

                ret = v3d_ioctl(v3d->fd, DRM_IOCTL_V3D_SUBMIT_CL, &job->submit);
                static bool warned = false;
                if (ret && !warned) {
                        fprintf(stderr, "Draw call returned %s.  "
                                        "Expect corruption.\n", strerror(errno));
                        warned = true;
                } else if (!ret) {
                        if (v3d->active_perfmon)
                                v3d->active_perfmon->job_submitted = true;
                }

                /* If we are submitting a job in the middle of transform
                 * feedback or there is a primitives generated query with a
                 * geometry shader then we need to read the primitive counts
                 * and accumulate them, otherwise they will be reset at the
                 * start of the next draw when we emit the Tile Binning Mode
                 * Configuration packet.
                 *
                 * If the job doesn't have any TF draw calls, then we know
                 * the primitive count must be zero and we can skip stalling
                 * for this. This also fixes a problem because it seems that
                 * in this scenario the counters are not reset with the Tile
                 * Binning Mode Configuration packet, which would translate
                 * to us reading an obsolete (possibly non-zero) value from
                 * the GPU counters.
                 */
                if (job->needs_primitives_generated ||
                    (v3d->streamout.num_targets &&
                     job->tf_draw_calls_queued > 0))
                        v3d_read_and_accumulate_primitive_counters(v3d);
        }

done:
        v3d_job_free(v3d, job);
}

static bool
v3d_job_compare(const void *a, const void *b)
{
        return memcmp(a, b, sizeof(struct v3d_job_key)) == 0;
}

static uint32_t
v3d_job_hash(const void *key)
{
        return _mesa_hash_data(key, sizeof(struct v3d_job_key));
}

void
v3d_job_init(struct v3d_context *v3d)
{
        v3d->jobs = _mesa_hash_table_create(v3d,
                                            v3d_job_hash,
                                            v3d_job_compare);
        v3d->write_jobs = _mesa_hash_table_create(v3d,
                                                  _mesa_hash_pointer,
                                                  _mesa_key_pointer_equal);
}

