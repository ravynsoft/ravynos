/*
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

#include <gtest/gtest.h>

#include "virgl_context.h"
#include "virgl_resource.h"
#include "virgl_screen.h"
#include "virgl_staging_mgr.h"
#include "virgl_winsys.h"

#include "util/u_inlines.h"
#include "util/u_memory.h"

struct virgl_hw_res {
    struct pipe_reference reference;
    uint32_t target;
    uint32_t bind;
    uint32_t size;
    void *data;
};

static struct virgl_hw_res *
fake_resource_create(struct virgl_winsys *vws,
                     enum pipe_texture_target target,
                     const void *map_front_private,
                     uint32_t format, uint32_t bind,
                     uint32_t width, uint32_t height,
                     uint32_t depth, uint32_t array_size,
                     uint32_t last_level, uint32_t nr_samples,
                     uint32_t flags, uint32_t size)
{
   struct virgl_hw_res *hw_res = CALLOC_STRUCT(virgl_hw_res);

   pipe_reference_init(&hw_res->reference, 1);

   hw_res->target = target;
   hw_res->bind = bind;
   hw_res->size = size;
   hw_res->data = CALLOC(size, 1);

   return hw_res;
}

static void
fake_resource_reference(struct virgl_winsys *vws,
                        struct virgl_hw_res **dres,
                        struct virgl_hw_res *sres)
{
   struct virgl_hw_res *old = *dres;

   if (pipe_reference(&(*dres)->reference, &sres->reference)) {
      FREE(old->data);
      FREE(old);
   }

   *dres = sres;
}

static void *
fake_resource_map(struct virgl_winsys *vws, struct virgl_hw_res *hw_res)
{
   return hw_res->data;
}

static struct pipe_context *
fake_virgl_context_create()
{
   struct virgl_context *vctx = CALLOC_STRUCT(virgl_context);
   struct virgl_screen *vs = CALLOC_STRUCT(virgl_screen);
   struct virgl_winsys *vws = CALLOC_STRUCT(virgl_winsys);

   vctx->base.screen = &vs->base;
   vs->vws = vws;

   vs->vws->resource_create = fake_resource_create;
   vs->vws->resource_reference = fake_resource_reference;
   vs->vws->resource_map = fake_resource_map;

   return &vctx->base;
}

static void
fake_virgl_context_destroy(struct pipe_context *ctx)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *vs = virgl_screen(ctx->screen);

   FREE(vs->vws);
   FREE(vs);
   FREE(vctx);
}

static void *
resource_map(struct virgl_hw_res *hw_res)
{
   return hw_res->data;
}

static void
release_resources(struct virgl_hw_res *resources[], unsigned len)
{
   for (unsigned i = 0; i < len; ++i)
      fake_resource_reference(NULL, &resources[i], NULL);
}

class VirglStagingMgr : public ::testing::Test
{
protected:
   VirglStagingMgr() : ctx(fake_virgl_context_create())
   {
      virgl_staging_init(&staging, ctx, staging_size);
   }

   ~VirglStagingMgr()
   {
      virgl_staging_destroy(&staging);
      fake_virgl_context_destroy(ctx);
   }

   static const unsigned staging_size;
   struct pipe_context * const ctx;
   struct virgl_staging_mgr staging;
};

const unsigned VirglStagingMgr::staging_size = 4096;

class VirglStagingMgrWithAlignment : public VirglStagingMgr,
                                     public ::testing::WithParamInterface<unsigned>
{
protected:
   VirglStagingMgrWithAlignment() : alignment(GetParam()) {}
   const unsigned alignment;
};

TEST_P(VirglStagingMgrWithAlignment,
       suballocations_are_non_overlapping_in_same_resource)
{
   const unsigned alloc_sizes[] = {16, 450, 79, 240, 128, 1001};
   const unsigned num_resources = sizeof(alloc_sizes) / sizeof(alloc_sizes[0]);
   struct virgl_hw_res *out_resource[num_resources] = {0};
   unsigned expected_offset = 0;
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   for (unsigned i = 0; i < num_resources; ++i) {
      alloc_succeeded =
         virgl_staging_alloc(&staging, alloc_sizes[i], alignment, &out_offset,
                           &out_resource[i], &map_ptr);

      EXPECT_TRUE(alloc_succeeded);
      EXPECT_EQ(out_offset, expected_offset);
      ASSERT_NE(out_resource[i], nullptr);
      if (i > 0) {
         EXPECT_EQ(out_resource[i], out_resource[i - 1]);
      }
      EXPECT_EQ(map_ptr,
            (uint8_t*)resource_map(out_resource[i]) + expected_offset);

      expected_offset += alloc_sizes[i];
      expected_offset = align(expected_offset, alignment);
   }

   release_resources(out_resource, num_resources);
}

INSTANTIATE_TEST_SUITE_P(
   WithAlignment,
   VirglStagingMgrWithAlignment,
   ::testing::Values(1, 16),
   testing::PrintToStringParamName()
);

TEST_F(VirglStagingMgr,
       non_fitting_allocation_reallocates_resource)
{
   struct virgl_hw_res *out_resource[2] = {0};
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   alloc_succeeded =
      virgl_staging_alloc(&staging, staging_size - 1, 1, &out_offset,
                          &out_resource[0], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[0], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[0]));

   alloc_succeeded =
      virgl_staging_alloc(&staging, 2, 1, &out_offset,
                          &out_resource[1], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[1], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[1]));
   /* New resource with same size as old resource. */
   EXPECT_NE(out_resource[1], out_resource[0]);
   EXPECT_EQ(out_resource[1]->size, out_resource[0]->size);

   release_resources(out_resource, 2);
}

TEST_F(VirglStagingMgr,
       non_fitting_aligned_allocation_reallocates_resource)
{
   struct virgl_hw_res *out_resource[2] = {0};
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   alloc_succeeded =
      virgl_staging_alloc(&staging, staging_size - 1, 1, &out_offset,
                          &out_resource[0], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[0], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[0]));

   alloc_succeeded =
      virgl_staging_alloc(&staging, 1, 16, &out_offset,
                          &out_resource[1], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[1], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[1]));
   /* New resource with same size as old resource. */
   EXPECT_NE(out_resource[1], out_resource[0]);
   EXPECT_EQ(out_resource[1]->size, out_resource[0]->size);

   release_resources(out_resource, 2);
}

TEST_F(VirglStagingMgr,
       large_non_fitting_allocation_reallocates_large_resource)
{
   struct virgl_hw_res *out_resource[2] = {0};
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   ASSERT_LT(staging_size, 5123);

   alloc_succeeded =
      virgl_staging_alloc(&staging, 5123, 1, &out_offset,
                          &out_resource[0], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[0], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[0]));
   EXPECT_GE(out_resource[0]->size, 5123);

   alloc_succeeded =
      virgl_staging_alloc(&staging, 19345, 1, &out_offset,
                          &out_resource[1], &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   EXPECT_EQ(out_offset, 0);
   ASSERT_NE(out_resource[1], nullptr);
   EXPECT_EQ(map_ptr, resource_map(out_resource[1]));
   /* New resource */
   EXPECT_NE(out_resource[1], out_resource[0]);
   EXPECT_GE(out_resource[1]->size, 19345);

   release_resources(out_resource, 2);
}

TEST_F(VirglStagingMgr, releases_resource_on_destruction)
{
   struct virgl_hw_res *out_resource = NULL;
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   alloc_succeeded =
      virgl_staging_alloc(&staging, 128, 1, &out_offset,
                          &out_resource, &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   ASSERT_NE(out_resource, nullptr);
   /* The resource is referenced both by staging internally,
    * and out_resource.
    */
   EXPECT_EQ(out_resource->reference.count, 2);

   /* Destroying staging releases the internal reference. */
   virgl_staging_destroy(&staging);
   EXPECT_EQ(out_resource->reference.count, 1);

   release_resources(&out_resource, 1);
}

static struct virgl_hw_res *
failing_resource_create(struct virgl_winsys *vws,
                        enum pipe_texture_target target,
                        const void *map_front_private,
                        uint32_t format, uint32_t bind,
                        uint32_t width, uint32_t height,
                        uint32_t depth, uint32_t array_size,
                        uint32_t last_level, uint32_t nr_samples,
                        uint32_t flags, uint32_t size)
{
   return NULL;
}

TEST_F(VirglStagingMgr, fails_gracefully_if_resource_create_fails)
{
   struct virgl_screen *vs = virgl_screen(ctx->screen);
   struct virgl_hw_res *out_resource = NULL;
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   vs->vws->resource_create = failing_resource_create;

   alloc_succeeded =
      virgl_staging_alloc(&staging, 128, 1, &out_offset,
                          &out_resource, &map_ptr);

   EXPECT_FALSE(alloc_succeeded);
   EXPECT_EQ(out_resource, nullptr);
   EXPECT_EQ(map_ptr, nullptr);
}

static void *
failing_resource_map(struct virgl_winsys *vws, struct virgl_hw_res *hw_res)
{
   return NULL;
}

TEST_F(VirglStagingMgr, fails_gracefully_if_map_fails)
{
   struct virgl_screen *vs = virgl_screen(ctx->screen);
   struct virgl_hw_res *out_resource = NULL;
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   vs->vws->resource_map = failing_resource_map;

   alloc_succeeded =
      virgl_staging_alloc(&staging, 128, 1, &out_offset,
                          &out_resource, &map_ptr);

   EXPECT_FALSE(alloc_succeeded);
   EXPECT_EQ(out_resource, nullptr);
   EXPECT_EQ(map_ptr, nullptr);
}

TEST_F(VirglStagingMgr, uses_staging_buffer_resource)
{
   struct virgl_hw_res *out_resource = NULL;
   unsigned out_offset;
   void *map_ptr;
   bool alloc_succeeded;

   alloc_succeeded =
      virgl_staging_alloc(&staging, 128, 1, &out_offset,
                          &out_resource, &map_ptr);

   EXPECT_TRUE(alloc_succeeded);
   ASSERT_NE(out_resource, nullptr);
   EXPECT_EQ(out_resource->target, PIPE_BUFFER);
   EXPECT_EQ(out_resource->bind, VIRGL_BIND_STAGING);

   release_resources(&out_resource, 1);
}
