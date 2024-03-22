/*
 * Copyright (C) 2019-2020 Collabora Ltd.
 * Copyright (C) 2019 Alyssa Rosenzweig
 * Copyright (C) 2014-2017 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __PAN_JC_H__
#define __PAN_JC_H__

#include "genxml/gen_macros.h"
#include "pan_pool.h"

/* Job chain */
struct pan_jc {
   /* The first job in the chain */
   mali_ptr first_job;

   /* The number of jobs in the chain, essentially */
   unsigned job_index;

   /* A CPU-side pointer to the previous job for next_job linking */
   struct mali_job_header_packed *prev_job;

   /* A CPU-side pointer to the first tiler job for dep updates when
    * injecting a reload tiler job.
    */
   struct mali_job_header_packed *first_tiler;
   uint32_t first_tiler_dep1;

   /* The dependency for tiler jobs (i.e. the index of the last emitted
    * tiler job, or zero if none have been emitted) */
   unsigned tiler_dep;

   /* The job index of the WRITE_VALUE job (before it has been created) */
   unsigned write_value_index;
};

#if defined(PAN_ARCH) && PAN_ARCH <= 9
/*
 * There are various types of Mali jobs:
 *
 *  - WRITE_VALUE: generic write primitive, used to zero tiler field
 *  - VERTEX: runs a vertex shader
 *  - TILER: runs tiling and sets up a fragment shader
 *  - FRAGMENT: runs fragment shaders and writes out
 *  - COMPUTE: runs a compute shader
 *  - FUSED: vertex+tiler fused together, implicit intradependency (Bifrost)
 *  - GEOMETRY: runs a geometry shader (unimplemented)
 *  - CACHE_FLUSH: unseen in the wild, theoretically cache flush
 *
 * In between a full batch and a single Mali job is the "job chain", a series
 * of Mali jobs together forming a linked list. Within the job chain, each Mali
 * job can set (up to) two dependencies on other earlier jobs in the chain.
 * This dependency graph forms a scoreboard. The general idea of a scoreboard
 * applies: when there is a data dependency of job B on job A, job B sets one
 * of its dependency indices to job A, ensuring that job B won't start until
 * job A finishes.
 *
 * More specifically, here are a set of rules:
 *
 * - A write value job must appear if and only if there is at least one tiler
 *   job, and tiler jobs must depend on it.
 *
 * - Vertex jobs and tiler jobs are independent.
 *
 * - A tiler job must have a dependency on its data source. If it's getting
 *   data from a vertex job, it depends on the vertex job. If it's getting data
 *   from software, this is null.
 *
 * - Tiler jobs must depend on the write value job (chained or otherwise).
 *
 * - Tiler jobs must be strictly ordered. So each tiler job must depend on the
 *   previous job in the chain.
 *
 * - Jobs linking via next_job has no bearing on order of execution, rather it
 *   just establishes the linked list of jobs, EXCEPT:
 *
 * - A job's dependencies must appear earlier in the linked list (job chain).
 *
 * Justification for each rule:
 *
 * - Write value jobs are used to write a zero into a magic tiling field, which
 *   enables tiling to work. If tiling occurs, they are needed; if it does not,
 *   we cannot emit them since then tiling partially occurs and it's bad.
 *
 * - The hardware has no notion of a "vertex/tiler job" (at least not our
 *   hardware -- other revs have fused jobs, but --- crap, this just got even
 *   more complicated). They are independent units that take in data, process
 *   it, and spit out data.
 *
 * - Any job must depend on its data source, in fact, or risk a
 *   read-before-write hazard. Tiler jobs get their data from vertex jobs, ergo
 *   tiler jobs depend on the corresponding vertex job (if it's there).
 *
 * - The tiler is not thread-safe; this dependency prevents race conditions
 *   between two different jobs trying to write to the tiler outputs at the
 *   same time.
 *
 * - Internally, jobs are scoreboarded; the next job fields just form a linked
 *   list to allow the jobs to be read in; the execution order is from
 *   resolving the dependency fields instead.
 *
 * - The hardware cannot set a dependency on a job it doesn't know about yet,
 *   and dependencies are processed in-order of the next job fields.
 *
 */

/**
 * Does this job use the tiler? Beyond tiler jobs, index-driven vertex shading
 * jobs also do.
 */
static inline bool
job_uses_tiling(enum mali_job_type type)
{
#if PAN_ARCH >= 9
   if (type == MALI_JOB_TYPE_MALLOC_VERTEX)
      return true;
#endif

#if PAN_ARCH >= 6
   if (type == MALI_JOB_TYPE_INDEXED_VERTEX)
      return true;
#endif

   return (type == MALI_JOB_TYPE_TILER);
}

/* Generates, uploads, and queues a a new job. All fields are written in order
 * except for next_job accounting (TODO: Should we be clever and defer the
 * upload of the header here until next job to keep the access pattern totally
 * linear? Or is that just a micro op at this point?). Returns the generated
 * index for dep management.
 *
 * Inject is used to inject a job at the front, for wallpapering. If you are
 * not wallpapering and set this, dragons will eat you. */

static inline unsigned
pan_jc_add_job(struct pan_pool *pool, struct pan_jc *jc,
               enum mali_job_type type, bool barrier, bool suppress_prefetch,
               unsigned local_dep, unsigned global_dep,
               const struct panfrost_ptr *job, bool inject)
{
   if (job_uses_tiling(type)) {
      /* Tiler jobs must be chained, and on Midgard, the first tiler
       * job must depend on the write value job, whose index we
       * reserve now */

      if (PAN_ARCH <= 5 && !jc->write_value_index)
         jc->write_value_index = ++jc->job_index;

      if (jc->tiler_dep && !inject)
         global_dep = jc->tiler_dep;
      else if (PAN_ARCH <= 5)
         global_dep = jc->write_value_index;
   }

   /* Assign the index */
   unsigned index = ++jc->job_index;

   pan_pack(job->cpu, JOB_HEADER, header) {
      header.type = type;
      header.barrier = barrier;
      header.suppress_prefetch = suppress_prefetch;
      header.index = index;
      header.dependency_1 = local_dep;
      header.dependency_2 = global_dep;

      if (inject)
         header.next = jc->first_job;
   }

   if (inject) {
      assert(type == MALI_JOB_TYPE_TILER && "only for blit shaders");

      if (jc->first_tiler) {
         /* Manual update of the dep2 field. This is bad,
          * don't copy this pattern.
          */
         jc->first_tiler->opaque[5] =
            jc->first_tiler_dep1 | (index << 16);
      }

      jc->first_tiler = (void *)job->cpu;
      jc->first_tiler_dep1 = local_dep;
      jc->first_job = job->gpu;
      return index;
   }

   /* Form a chain */
   if (job_uses_tiling(type)) {
      if (!jc->first_tiler) {
         jc->first_tiler = (void *)job->cpu;
         jc->first_tiler_dep1 = local_dep;
      }
      jc->tiler_dep = index;
   }

   if (jc->prev_job) {
      /* Manual update of the next pointer. This is bad, don't copy
       * this pattern.
       * TODO: Find a way to defer last job header emission until we
       * have a new job to queue or the batch is ready for execution.
       */
      jc->prev_job->opaque[6] = job->gpu;
      jc->prev_job->opaque[7] = job->gpu >> 32;
   } else {
      jc->first_job = job->gpu;
   }

   jc->prev_job = (struct mali_job_header_packed *)job->cpu;
   return index;
}

/* Generates a write value job, used to initialize the tiler structures. Note
 * this is called right before frame submission. */

static inline struct panfrost_ptr
pan_jc_initialize_tiler(struct pan_pool *pool,
                        struct pan_jc *jc,
                        mali_ptr polygon_list)
{
   struct panfrost_ptr transfer = {0};

   /* Check if we even need tiling */
   if (PAN_ARCH >= 6 || !jc->first_tiler)
      return transfer;

   /* Okay, we do. Let's generate it. We'll need the job's polygon list
    * regardless of size. */

   transfer = pan_pool_alloc_desc(pool, WRITE_VALUE_JOB);

   pan_section_pack(transfer.cpu, WRITE_VALUE_JOB, HEADER, header) {
      header.type = MALI_JOB_TYPE_WRITE_VALUE;
      header.index = jc->write_value_index;
      header.next = jc->first_job;
   }

   pan_section_pack(transfer.cpu, WRITE_VALUE_JOB, PAYLOAD, payload) {
      payload.address = polygon_list;
      payload.type = MALI_WRITE_VALUE_TYPE_ZERO;
   }

   jc->first_job = transfer.gpu;
   return transfer;
}
#endif /* PAN_ARCH */

#endif
