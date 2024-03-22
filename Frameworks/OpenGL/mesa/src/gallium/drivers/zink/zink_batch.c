#include "zink_batch.h"
#include "zink_context.h"
#include "zink_descriptors.h"
#include "zink_framebuffer.h"
#include "zink_kopper.h"
#include "zink_program.h"
#include "zink_query.h"
#include "zink_resource.h"
#include "zink_screen.h"
#include "zink_surface.h"

#ifdef VK_USE_PLATFORM_METAL_EXT
#include "QuartzCore/CAMetalLayer.h"
#endif

#define MAX_VIEW_COUNT 500

void
debug_describe_zink_batch_state(char *buf, const struct zink_batch_state *ptr)
{
   sprintf(buf, "zink_batch_state");
}

/* this resets the batch usage and tracking for a resource object */
static void
reset_obj(struct zink_screen *screen, struct zink_batch_state *bs, struct zink_resource_object *obj)
{
   /* if no batch usage exists after removing the usage from 'bs', this resource is considered fully idle */
   if (!zink_resource_object_usage_unset(obj, bs)) {
      /* the resource is idle, so reset all access/reordering info */
      obj->unordered_read = true;
      obj->unordered_write = true;
      obj->access = 0;
      obj->unordered_access = 0;
      obj->last_write = 0;
      obj->access_stage = 0;
      obj->unordered_access_stage = 0;
      obj->copies_need_reset = true;
      obj->unsync_access = true;
      /* also prune dead view objects */
      simple_mtx_lock(&obj->view_lock);
      if (obj->is_buffer) {
         while (util_dynarray_contains(&obj->views, VkBufferView))
            VKSCR(DestroyBufferView)(screen->dev, util_dynarray_pop(&obj->views, VkBufferView), NULL);
      } else {
         while (util_dynarray_contains(&obj->views, VkImageView))
            VKSCR(DestroyImageView)(screen->dev, util_dynarray_pop(&obj->views, VkImageView), NULL);
      }
      obj->view_prune_count = 0;
      obj->view_prune_timeline = 0;
      simple_mtx_unlock(&obj->view_lock);
      if (obj->dt)
         zink_kopper_prune_batch_usage(obj->dt, &bs->usage);
   } else if (util_dynarray_num_elements(&obj->views, VkBufferView) > MAX_VIEW_COUNT && !zink_bo_has_unflushed_usage(obj->bo)) {
      /* avoid ballooning from too many views on always-used resources: */
      simple_mtx_lock(&obj->view_lock);
      /* ensure no existing view pruning is queued, double check elements in case pruning just finished */
      if (!obj->view_prune_timeline && util_dynarray_num_elements(&obj->views, VkBufferView) > MAX_VIEW_COUNT) {
         /* prune all existing views */
         obj->view_prune_count = util_dynarray_num_elements(&obj->views, VkBufferView);
         /* prune them when the views will definitely not be in use */
         obj->view_prune_timeline = MAX2(obj->bo->reads.u ? obj->bo->reads.u->usage : 0,
                                         obj->bo->writes.u ? obj->bo->writes.u->usage : 0);
      }
      simple_mtx_unlock(&obj->view_lock);
   }
   /* resource objects are not unrefed here;
    * this is typically the last ref on a resource object, and destruction will
    * usually trigger an ioctl, so defer deletion to the submit thread to avoid blocking
    */
   util_dynarray_append(&bs->unref_resources, struct zink_resource_object*, obj);
}

/* reset all the resource objects in a given batch object list */
static void
reset_obj_list(struct zink_screen *screen, struct zink_batch_state *bs, struct zink_batch_obj_list *list)
{
   for (unsigned i = 0; i < list->num_buffers; i++)
      reset_obj(screen, bs, list->objs[i]);
   list->num_buffers = 0;
}

/* reset a given batch state */
void
zink_reset_batch_state(struct zink_context *ctx, struct zink_batch_state *bs)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);

   VkResult result = VKSCR(ResetCommandPool)(screen->dev, bs->cmdpool, 0);
   if (result != VK_SUCCESS)
      mesa_loge("ZINK: vkResetCommandPool failed (%s)", vk_Result_to_str(result));
   result = VKSCR(ResetCommandPool)(screen->dev, bs->unsynchronized_cmdpool, 0);
   if (result != VK_SUCCESS)
      mesa_loge("ZINK: vkResetCommandPool failed (%s)", vk_Result_to_str(result));

   /* unref/reset all used resources */
   reset_obj_list(screen, bs, &bs->real_objs);
   reset_obj_list(screen, bs, &bs->slab_objs);
   reset_obj_list(screen, bs, &bs->sparse_objs);
   while (util_dynarray_contains(&bs->swapchain_obj, struct zink_resource_object*)) {
      struct zink_resource_object *obj = util_dynarray_pop(&bs->swapchain_obj, struct zink_resource_object*);
      reset_obj(screen, bs, obj);
   }

   /* this is where bindless texture/buffer ids get recycled */
   for (unsigned i = 0; i < 2; i++) {
      while (util_dynarray_contains(&bs->bindless_releases[i], uint32_t)) {
         uint32_t handle = util_dynarray_pop(&bs->bindless_releases[i], uint32_t);
         bool is_buffer = ZINK_BINDLESS_IS_BUFFER(handle);
         struct util_idalloc *ids = i ? &ctx->di.bindless[is_buffer].img_slots : &ctx->di.bindless[is_buffer].tex_slots;
         util_idalloc_free(ids, is_buffer ? handle - ZINK_MAX_BINDLESS_HANDLES : handle);
      }
   }

   /* queries must only be destroyed once they are inactive */
   set_foreach_remove(&bs->active_queries, entry) {
      struct zink_query *query = (void*)entry->key;
      zink_prune_query(bs, query);
   }
   util_dynarray_foreach(&bs->dead_querypools, VkQueryPool, pool)
      VKSCR(DestroyQueryPool)(screen->dev, *pool, NULL);
   util_dynarray_clear(&bs->dead_querypools);

   util_dynarray_foreach(&bs->dgc.pipelines, VkPipeline, pipeline)
      VKSCR(DestroyPipeline)(screen->dev, *pipeline, NULL);
   util_dynarray_clear(&bs->dgc.pipelines);
   util_dynarray_foreach(&bs->dgc.layouts, VkIndirectCommandsLayoutNV, iclayout)
      VKSCR(DestroyIndirectCommandsLayoutNV)(screen->dev, *iclayout, NULL);
   util_dynarray_clear(&bs->dgc.layouts);

   /* samplers are appended to the batch state in which they are destroyed
    * to ensure deferred deletion without destroying in-use objects
    */
   util_dynarray_foreach(&bs->zombie_samplers, VkSampler, samp) {
      VKSCR(DestroySampler)(screen->dev, *samp, NULL);
   }
   util_dynarray_clear(&bs->zombie_samplers);

   zink_batch_descriptor_reset(screen, bs);

   util_dynarray_foreach(&bs->freed_sparse_backing_bos, struct zink_bo, bo) {
      zink_bo_unref(screen, bo);
   }
   util_dynarray_clear(&bs->freed_sparse_backing_bos);

   /* programs are refcounted and batch-tracked */
   set_foreach_remove(&bs->programs, entry) {
      struct zink_program *pg = (struct zink_program*)entry->key;
      zink_batch_usage_unset(&pg->batch_uses, bs);
      zink_program_reference(screen, &pg, NULL);
   }

   bs->resource_size = 0;
   bs->signal_semaphore = VK_NULL_HANDLE;
   util_dynarray_clear(&bs->wait_semaphore_stages);

   bs->present = VK_NULL_HANDLE;
   /* check the arrays first to avoid locking unnecessarily */
   if (util_dynarray_contains(&bs->acquires, VkSemaphore) || util_dynarray_contains(&bs->wait_semaphores, VkSemaphore)) {
      simple_mtx_lock(&screen->semaphores_lock);
      util_dynarray_append_dynarray(&screen->semaphores, &bs->acquires);
      util_dynarray_clear(&bs->acquires);
      util_dynarray_append_dynarray(&screen->semaphores, &bs->wait_semaphores);
      util_dynarray_clear(&bs->wait_semaphores);
      simple_mtx_unlock(&screen->semaphores_lock);
   }
   if (util_dynarray_contains(&bs->signal_semaphores, VkSemaphore) || util_dynarray_contains(&bs->fd_wait_semaphores, VkSemaphore)) {
      simple_mtx_lock(&screen->semaphores_lock);
      util_dynarray_append_dynarray(&screen->fd_semaphores, &bs->signal_semaphores);
      util_dynarray_clear(&bs->signal_semaphores);
      util_dynarray_append_dynarray(&screen->fd_semaphores, &bs->fd_wait_semaphores);
      util_dynarray_clear(&bs->fd_wait_semaphores);
      simple_mtx_unlock(&screen->semaphores_lock);
   }
   bs->swapchain = NULL;

   util_dynarray_foreach(&bs->fences, struct zink_tc_fence*, mfence)
      zink_fence_reference(screen, mfence, NULL);
   util_dynarray_clear(&bs->fences);

   bs->unordered_write_access = 0;
   bs->unordered_write_stages = 0;

   /* only increment batch generation if previously in-use to avoid false detection of batch completion */
   if (bs->fence.submitted)
      bs->usage.submit_count++;
   /* only reset submitted here so that tc fence desync can pick up the 'completed' flag
    * before the state is reused
    */
   bs->fence.submitted = false;
   bs->has_barriers = false;
   bs->has_unsync = false;
   if (bs->fence.batch_id)
      zink_screen_update_last_finished(screen, bs->fence.batch_id);
   bs->fence.batch_id = 0;
   bs->usage.usage = 0;
   bs->next = NULL;
   bs->last_added_obj = NULL;
}

/* this is where deferred resource unrefs occur */
static void
unref_resources(struct zink_screen *screen, struct zink_batch_state *bs)
{
   while (util_dynarray_contains(&bs->unref_resources, struct zink_resource_object*)) {
      struct zink_resource_object *obj = util_dynarray_pop(&bs->unref_resources, struct zink_resource_object*);
      /* view pruning may be deferred to avoid ballooning */
      if (obj->view_prune_timeline && zink_screen_check_last_finished(screen, obj->view_prune_timeline)) {
         simple_mtx_lock(&obj->view_lock);
         /* check again under lock in case multi-context use is in the same place */
         if (obj->view_prune_timeline && zink_screen_check_last_finished(screen, obj->view_prune_timeline)) {
            /* prune `view_prune_count` views */
            if (obj->is_buffer) {
               VkBufferView *views = obj->views.data;
               for (unsigned i = 0; i < obj->view_prune_count; i++)
                  VKSCR(DestroyBufferView)(screen->dev, views[i], NULL);
            } else {
               VkImageView *views = obj->views.data;
               for (unsigned i = 0; i < obj->view_prune_count; i++)
                  VKSCR(DestroyImageView)(screen->dev, views[i], NULL);
            }
            size_t offset = obj->view_prune_count * sizeof(VkBufferView);
            uint8_t *data = obj->views.data;
            /* shift the view array to the start */
            memcpy(data, data + offset, obj->views.size - offset);
            /* adjust the array size */
            obj->views.size -= offset;
            obj->view_prune_count = 0;
            obj->view_prune_timeline = 0;
         }
         simple_mtx_unlock(&obj->view_lock);
      }
      /* this is typically where resource objects get destroyed */
      zink_resource_object_reference(screen, &obj, NULL);
   }
}

/* utility for resetting a batch state; called on context destruction */
void
zink_clear_batch_state(struct zink_context *ctx, struct zink_batch_state *bs)
{
   bs->fence.completed = true;
   zink_reset_batch_state(ctx, bs);
   unref_resources(zink_screen(ctx->base.screen), bs);
}

/* utility for managing the singly-linked batch state list */
static void
pop_batch_state(struct zink_context *ctx)
{
   const struct zink_batch_state *bs = ctx->batch_states;
   ctx->batch_states = bs->next;
   ctx->batch_states_count--;
   if (ctx->last_fence == &bs->fence)
      ctx->last_fence = NULL;
}

/* reset all batch states and append to the free state list
 * only usable after a full stall
 */
void
zink_batch_reset_all(struct zink_context *ctx)
{
   while (ctx->batch_states) {
      struct zink_batch_state *bs = ctx->batch_states;
      bs->fence.completed = true;
      pop_batch_state(ctx);
      zink_reset_batch_state(ctx, bs);
      if (ctx->last_free_batch_state)
         ctx->last_free_batch_state->next = bs;
      else
         ctx->free_batch_states = bs;
      ctx->last_free_batch_state = bs;
   }
}

/* called only on context destruction */
void
zink_batch_state_destroy(struct zink_screen *screen, struct zink_batch_state *bs)
{
   if (!bs)
      return;

   util_queue_fence_destroy(&bs->flush_completed);

   cnd_destroy(&bs->usage.flush);
   mtx_destroy(&bs->usage.mtx);

   if (bs->cmdbuf)
      VKSCR(FreeCommandBuffers)(screen->dev, bs->cmdpool, 1, &bs->cmdbuf);
   if (bs->reordered_cmdbuf)
      VKSCR(FreeCommandBuffers)(screen->dev, bs->cmdpool, 1, &bs->reordered_cmdbuf);
   if (bs->cmdpool)
      VKSCR(DestroyCommandPool)(screen->dev, bs->cmdpool, NULL);
   if (bs->unsynchronized_cmdbuf)
      VKSCR(FreeCommandBuffers)(screen->dev, bs->unsynchronized_cmdpool, 1, &bs->unsynchronized_cmdbuf);
   if (bs->unsynchronized_cmdpool)
      VKSCR(DestroyCommandPool)(screen->dev, bs->unsynchronized_cmdpool, NULL);
   free(bs->real_objs.objs);
   free(bs->slab_objs.objs);
   free(bs->sparse_objs.objs);
   util_dynarray_fini(&bs->freed_sparse_backing_bos);
   util_dynarray_fini(&bs->dead_querypools);
   util_dynarray_fini(&bs->dgc.pipelines);
   util_dynarray_fini(&bs->dgc.layouts);
   util_dynarray_fini(&bs->swapchain_obj);
   util_dynarray_fini(&bs->zombie_samplers);
   util_dynarray_fini(&bs->unref_resources);
   util_dynarray_fini(&bs->bindless_releases[0]);
   util_dynarray_fini(&bs->bindless_releases[1]);
   util_dynarray_fini(&bs->acquires);
   util_dynarray_fini(&bs->acquire_flags);
   unsigned num_mfences = util_dynarray_num_elements(&bs->fence.mfences, void *);
   struct zink_tc_fence **mfence = bs->fence.mfences.data;
   for (unsigned i = 0; i < num_mfences; i++) {
      mfence[i]->fence = NULL;
   }
   util_dynarray_fini(&bs->fence.mfences);
   zink_batch_descriptor_deinit(screen, bs);
   ralloc_free(bs);
}

/* batch states are created:
 * - on context creation
 * - dynamically up to a threshold if no free ones are available
 */
static struct zink_batch_state *
create_batch_state(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = rzalloc(NULL, struct zink_batch_state);
   VkCommandPoolCreateInfo cpci = {0};
   cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cpci.queueFamilyIndex = screen->gfx_queue;
   VkResult result;

   VRAM_ALLOC_LOOP(result,
      VKSCR(CreateCommandPool)(screen->dev, &cpci, NULL, &bs->cmdpool),
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkCreateCommandPool failed (%s)", vk_Result_to_str(result));
         goto fail;
      }
   );
   VRAM_ALLOC_LOOP(result,
      VKSCR(CreateCommandPool)(screen->dev, &cpci, NULL, &bs->unsynchronized_cmdpool),
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkCreateCommandPool failed (%s)", vk_Result_to_str(result));
         goto fail;
      }
   );

   VkCommandBuffer cmdbufs[2];
   VkCommandBufferAllocateInfo cbai = {0};
   cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   cbai.commandPool = bs->cmdpool;
   cbai.commandBufferCount = 2;

   VRAM_ALLOC_LOOP(result,
      VKSCR(AllocateCommandBuffers)(screen->dev, &cbai, cmdbufs),
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkAllocateCommandBuffers failed (%s)", vk_Result_to_str(result));
         goto fail;
      }
   );

   bs->cmdbuf = cmdbufs[0];
   bs->reordered_cmdbuf = cmdbufs[1];

   cbai.commandPool = bs->unsynchronized_cmdpool;
   cbai.commandBufferCount = 1;
   VRAM_ALLOC_LOOP(result,
      VKSCR(AllocateCommandBuffers)(screen->dev, &cbai, &bs->unsynchronized_cmdbuf);,
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkAllocateCommandBuffers failed (%s)", vk_Result_to_str(result));
         goto fail;
      }
   );

#define SET_CREATE_OR_FAIL(ptr) \
   if (!_mesa_set_init(ptr, bs, _mesa_hash_pointer, _mesa_key_pointer_equal)) \
      goto fail

   bs->ctx = ctx;

   SET_CREATE_OR_FAIL(&bs->programs);
   SET_CREATE_OR_FAIL(&bs->active_queries);
   SET_CREATE_OR_FAIL(&bs->dmabuf_exports);
   util_dynarray_init(&bs->signal_semaphores, NULL);
   util_dynarray_init(&bs->wait_semaphores, NULL);
   util_dynarray_init(&bs->fd_wait_semaphores, NULL);
   util_dynarray_init(&bs->fences, NULL);
   util_dynarray_init(&bs->dead_querypools, NULL);
   util_dynarray_init(&bs->dgc.pipelines, NULL);
   util_dynarray_init(&bs->dgc.layouts, NULL);
   util_dynarray_init(&bs->wait_semaphore_stages, NULL);
   util_dynarray_init(&bs->fd_wait_semaphore_stages, NULL);
   util_dynarray_init(&bs->zombie_samplers, NULL);
   util_dynarray_init(&bs->freed_sparse_backing_bos, NULL);
   util_dynarray_init(&bs->unref_resources, NULL);
   util_dynarray_init(&bs->acquires, NULL);
   util_dynarray_init(&bs->acquire_flags, NULL);
   util_dynarray_init(&bs->bindless_releases[0], NULL);
   util_dynarray_init(&bs->bindless_releases[1], NULL);
   util_dynarray_init(&bs->swapchain_obj, NULL);
   util_dynarray_init(&bs->fence.mfences, NULL);

   cnd_init(&bs->usage.flush);
   mtx_init(&bs->usage.mtx, mtx_plain);
   simple_mtx_init(&bs->exportable_lock, mtx_plain);
   memset(&bs->buffer_indices_hashlist, -1, sizeof(bs->buffer_indices_hashlist));

   if (!zink_batch_descriptor_init(screen, bs))
      goto fail;

   util_queue_fence_init(&bs->flush_completed);

   return bs;
fail:
   zink_batch_state_destroy(screen, bs);
   return NULL;
}

/* a batch state is considered "free" if it is both submitted and completed */
static inline bool
find_unused_state(struct zink_batch_state *bs)
{
   struct zink_fence *fence = &bs->fence;
   /* we can't reset these from fence_finish because threads */
   bool completed = p_atomic_read(&fence->completed);
   bool submitted = p_atomic_read(&fence->submitted);
   return submitted && completed;
}

/* find a "free" batch state */
static struct zink_batch_state *
get_batch_state(struct zink_context *ctx, struct zink_batch *batch)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = NULL;

   /* try from the ones that are known to be free first */
   if (ctx->free_batch_states) {
      bs = ctx->free_batch_states;
      ctx->free_batch_states = bs->next;
      if (bs == ctx->last_free_batch_state)
         ctx->last_free_batch_state = NULL;
   }
   /* try from the ones that are given back to the screen next */
   if (!bs) {
      simple_mtx_lock(&screen->free_batch_states_lock);
      if (screen->free_batch_states) {
         bs = screen->free_batch_states;
         bs->ctx = ctx;
         screen->free_batch_states = bs->next;
         if (bs == screen->last_free_batch_state)
            screen->last_free_batch_state = NULL;
      }
      simple_mtx_unlock(&screen->free_batch_states_lock);
   }
   /* states are stored sequentially, so if the first one doesn't work, none of them will */
   if (!bs && ctx->batch_states && ctx->batch_states->next) {
      /* only a submitted state can be reused */
      if (p_atomic_read(&ctx->batch_states->fence.submitted) &&
          /* a submitted state must have completed before it can be reused */
          (zink_screen_check_last_finished(screen, ctx->batch_states->fence.batch_id) ||
           p_atomic_read(&ctx->batch_states->fence.completed))) {
         bs = ctx->batch_states;
         pop_batch_state(ctx);
      }
   }
   if (bs) {
      zink_reset_batch_state(ctx, bs);
   } else {
      if (!batch->state) {
         /* this is batch init, so create a few more states for later use */
         for (int i = 0; i < 3; i++) {
            struct zink_batch_state *state = create_batch_state(ctx);
            if (ctx->last_free_batch_state)
               ctx->last_free_batch_state->next = state;
            else
               ctx->free_batch_states = state;
            ctx->last_free_batch_state = state;
         }
      }
      /* no batch states were available: make a new one */
      bs = create_batch_state(ctx);
   }
   return bs;
}

/* reset the batch object: get a new state and unset 'has_work' to disable flushing */
void
zink_reset_batch(struct zink_context *ctx, struct zink_batch *batch)
{
   batch->state = get_batch_state(ctx, batch);
   assert(batch->state);

   batch->has_work = false;
}

void
zink_batch_bind_db(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch *batch = &ctx->batch;
   unsigned count = 1;
   VkDescriptorBufferBindingInfoEXT infos[2] = {0};
   infos[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
   infos[0].address = batch->state->dd.db->obj->bda;
   infos[0].usage = batch->state->dd.db->obj->vkusage;
   assert(infos[0].usage);

   if (ctx->dd.bindless_init) {
      infos[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
      infos[1].address = ctx->dd.db.bindless_db->obj->bda;
      infos[1].usage = ctx->dd.db.bindless_db->obj->vkusage;
      assert(infos[1].usage);
      count++;
   }
   VKSCR(CmdBindDescriptorBuffersEXT)(batch->state->cmdbuf, count, infos);
   VKSCR(CmdBindDescriptorBuffersEXT)(batch->state->reordered_cmdbuf, count, infos);
   batch->state->dd.db_bound = true;
}

/* called on context creation and after flushing an old batch */
void
zink_start_batch(struct zink_context *ctx, struct zink_batch *batch)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   zink_reset_batch(ctx, batch);

   batch->state->usage.unflushed = true;

   VkCommandBufferBeginInfo cbbi = {0};
   cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

   VkResult result;
   VRAM_ALLOC_LOOP(result,
      VKCTX(BeginCommandBuffer)(batch->state->cmdbuf, &cbbi),
      if (result != VK_SUCCESS)
         mesa_loge("ZINK: vkBeginCommandBuffer failed (%s)", vk_Result_to_str(result));
   );
   VRAM_ALLOC_LOOP(result,
      VKCTX(BeginCommandBuffer)(batch->state->reordered_cmdbuf, &cbbi),
      if (result != VK_SUCCESS)
         mesa_loge("ZINK: vkBeginCommandBuffer failed (%s)", vk_Result_to_str(result));
   );
   VRAM_ALLOC_LOOP(result,
      VKCTX(BeginCommandBuffer)(batch->state->unsynchronized_cmdbuf, &cbbi),
      if (result != VK_SUCCESS)
         mesa_loge("ZINK: vkBeginCommandBuffer failed (%s)", vk_Result_to_str(result));
   );

   batch->state->fence.completed = false;
   if (ctx->last_fence) {
      struct zink_batch_state *last_state = zink_batch_state(ctx->last_fence);
      batch->last_batch_usage = &last_state->usage;
   }

#ifdef HAVE_RENDERDOC_APP_H
   if (VKCTX(CmdInsertDebugUtilsLabelEXT) && screen->renderdoc_api) {
      VkDebugUtilsLabelEXT capture_label;
      /* Magic fallback which lets us bridge the Wine barrier over to Linux RenderDoc. */
      capture_label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
      capture_label.pNext = NULL;
      capture_label.pLabelName = "vr-marker,frame_end,type,application";
      memset(capture_label.color, 0, sizeof(capture_label.color));
      VKCTX(CmdInsertDebugUtilsLabelEXT)(batch->state->unsynchronized_cmdbuf, &capture_label);
      VKCTX(CmdInsertDebugUtilsLabelEXT)(batch->state->reordered_cmdbuf, &capture_label);
      VKCTX(CmdInsertDebugUtilsLabelEXT)(batch->state->cmdbuf, &capture_label);
   }

   unsigned renderdoc_frame = p_atomic_read(&screen->renderdoc_frame);
   if (!(ctx->flags & ZINK_CONTEXT_COPY_ONLY) && screen->renderdoc_api && !screen->renderdoc_capturing &&
        ((screen->renderdoc_capture_all && screen->screen_id == 1) || (renderdoc_frame >= screen->renderdoc_capture_start && renderdoc_frame <= screen->renderdoc_capture_end))) {
      screen->renderdoc_api->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(screen->instance), NULL);
      screen->renderdoc_capturing = true;
   }
#endif

   /* descriptor buffers must always be bound at the start of a batch */
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB && !(ctx->flags & ZINK_CONTEXT_COPY_ONLY))
      zink_batch_bind_db(ctx);
   /* zero init for unordered blits */
   if (screen->info.have_EXT_attachment_feedback_loop_dynamic_state) {
      VKCTX(CmdSetAttachmentFeedbackLoopEnableEXT)(ctx->batch.state->cmdbuf, 0);
      VKCTX(CmdSetAttachmentFeedbackLoopEnableEXT)(ctx->batch.state->reordered_cmdbuf, 0);
      VKCTX(CmdSetAttachmentFeedbackLoopEnableEXT)(ctx->batch.state->unsynchronized_cmdbuf, 0);
   }
}

/* common operations to run post submit; split out for clarity */
static void
post_submit(void *data, void *gdata, int thread_index)
{
   struct zink_batch_state *bs = data;
   struct zink_screen *screen = zink_screen(bs->ctx->base.screen);

   if (bs->is_device_lost) {
      if (bs->ctx->reset.reset)
         bs->ctx->reset.reset(bs->ctx->reset.data, PIPE_GUILTY_CONTEXT_RESET);
      else if (screen->abort_on_hang && !screen->robust_ctx_count)
         /* if nothing can save us, abort */
         abort();
      screen->device_lost = true;
   } else if (bs->ctx->batch_states_count > 5000) {
      /* throttle in case something crazy is happening */
      zink_screen_timeline_wait(screen, bs->fence.batch_id - 2500, OS_TIMEOUT_INFINITE);
   }
   /* this resets the buffer hashlist for the state's next use */
   memset(&bs->buffer_indices_hashlist, -1, sizeof(bs->buffer_indices_hashlist));
}

typedef enum {
   ZINK_SUBMIT_WAIT_ACQUIRE,
   ZINK_SUBMIT_WAIT_FD,
   ZINK_SUBMIT_CMDBUF,
   ZINK_SUBMIT_SIGNAL,
   ZINK_SUBMIT_MAX
} zink_submit;

static void
submit_queue(void *data, void *gdata, int thread_index)
{
   struct zink_batch_state *bs = data;
   struct zink_context *ctx = bs->ctx;
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkSubmitInfo si[ZINK_SUBMIT_MAX] = {0};
   VkSubmitInfo *submit = si;
   int num_si = ZINK_SUBMIT_MAX;
   while (!bs->fence.batch_id)
      bs->fence.batch_id = (uint32_t)p_atomic_inc_return(&screen->curr_batch);
   bs->usage.usage = bs->fence.batch_id;
   bs->usage.unflushed = false;

   uint64_t batch_id = bs->fence.batch_id;
   /* first submit is just for acquire waits since they have a separate array */
   for (unsigned i = 0; i < ARRAY_SIZE(si); i++)
      si[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   si[ZINK_SUBMIT_WAIT_ACQUIRE].waitSemaphoreCount = util_dynarray_num_elements(&bs->acquires, VkSemaphore);
   si[ZINK_SUBMIT_WAIT_ACQUIRE].pWaitSemaphores = bs->acquires.data;
   while (util_dynarray_num_elements(&bs->acquire_flags, VkPipelineStageFlags) < si[ZINK_SUBMIT_WAIT_ACQUIRE].waitSemaphoreCount) {
      VkPipelineStageFlags mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      util_dynarray_append(&bs->acquire_flags, VkPipelineStageFlags, mask);
   }
   assert(util_dynarray_num_elements(&bs->acquires, VkSemaphore) <= util_dynarray_num_elements(&bs->acquire_flags, VkPipelineStageFlags));
   si[ZINK_SUBMIT_WAIT_ACQUIRE].pWaitDstStageMask = bs->acquire_flags.data;

   si[ZINK_SUBMIT_WAIT_FD].waitSemaphoreCount = util_dynarray_num_elements(&bs->fd_wait_semaphores, VkSemaphore);
   si[ZINK_SUBMIT_WAIT_FD].pWaitSemaphores = bs->fd_wait_semaphores.data;
   while (util_dynarray_num_elements(&bs->fd_wait_semaphore_stages, VkPipelineStageFlags) < si[ZINK_SUBMIT_WAIT_FD].waitSemaphoreCount) {
      VkPipelineStageFlags mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      util_dynarray_append(&bs->fd_wait_semaphore_stages, VkPipelineStageFlags, mask);
   }
   assert(util_dynarray_num_elements(&bs->fd_wait_semaphores, VkSemaphore) <= util_dynarray_num_elements(&bs->fd_wait_semaphore_stages, VkPipelineStageFlags));
   si[ZINK_SUBMIT_WAIT_FD].pWaitDstStageMask = bs->fd_wait_semaphore_stages.data;

   if (si[ZINK_SUBMIT_WAIT_ACQUIRE].waitSemaphoreCount == 0) {
      num_si--;
      submit++;
      if (si[ZINK_SUBMIT_WAIT_FD].waitSemaphoreCount == 0) {
         num_si--;
         submit++;
      }
   }

   /* then the real submit */
   si[ZINK_SUBMIT_CMDBUF].waitSemaphoreCount = util_dynarray_num_elements(&bs->wait_semaphores, VkSemaphore);
   si[ZINK_SUBMIT_CMDBUF].pWaitSemaphores = bs->wait_semaphores.data;
   si[ZINK_SUBMIT_CMDBUF].pWaitDstStageMask = bs->wait_semaphore_stages.data;
   VkCommandBuffer cmdbufs[3];
   unsigned c = 0;
   if (bs->has_unsync)
      cmdbufs[c++] = bs->unsynchronized_cmdbuf;
   if (bs->has_barriers)
      cmdbufs[c++] = bs->reordered_cmdbuf;
   cmdbufs[c++] = bs->cmdbuf;
   si[ZINK_SUBMIT_CMDBUF].pCommandBuffers = cmdbufs;
   si[ZINK_SUBMIT_CMDBUF].commandBufferCount = c;
   /* assorted signal submit from wsi/externals */
   si[ZINK_SUBMIT_CMDBUF].signalSemaphoreCount = util_dynarray_num_elements(&bs->signal_semaphores, VkSemaphore);
   si[ZINK_SUBMIT_CMDBUF].pSignalSemaphores = bs->signal_semaphores.data;

   /* then the signal submit with the timeline (fence) semaphore */
   VkSemaphore signals[3];
   si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount = !!bs->signal_semaphore;
   signals[0] = bs->signal_semaphore;
   si[ZINK_SUBMIT_SIGNAL].pSignalSemaphores = signals;
   VkTimelineSemaphoreSubmitInfo tsi = {0};
   uint64_t signal_values[2] = {0};
   tsi.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
   si[ZINK_SUBMIT_SIGNAL].pNext = &tsi;
   tsi.pSignalSemaphoreValues = signal_values;
   signal_values[si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount] = batch_id;
   signals[si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount++] = screen->sem;
   tsi.signalSemaphoreValueCount = si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount;

   if (bs->present)
      signals[si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount++] = bs->present;
   tsi.signalSemaphoreValueCount = si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount;


   VkResult result;
   VRAM_ALLOC_LOOP(result,
      VKSCR(EndCommandBuffer)(bs->cmdbuf),
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkEndCommandBuffer failed (%s)", vk_Result_to_str(result));
         bs->is_device_lost = true;
         goto end;
      }
   );
   if (bs->has_barriers) {
      if (bs->unordered_write_access) {
         VkMemoryBarrier mb;
         mb.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
         mb.pNext = NULL;
         mb.srcAccessMask = bs->unordered_write_access;
         mb.dstAccessMask = 0;
         VKSCR(CmdPipelineBarrier)(bs->reordered_cmdbuf,
                                   bs->unordered_write_stages, 0,
                                   0, 1, &mb, 0, NULL, 0, NULL);
      }
      VRAM_ALLOC_LOOP(result,
         VKSCR(EndCommandBuffer)(bs->reordered_cmdbuf),
         if (result != VK_SUCCESS) {
            mesa_loge("ZINK: vkEndCommandBuffer failed (%s)", vk_Result_to_str(result));
            bs->is_device_lost = true;
            goto end;
         }
      );
   }
   if (bs->has_unsync) {
      VRAM_ALLOC_LOOP(result,
         VKSCR(EndCommandBuffer)(bs->unsynchronized_cmdbuf),
         if (result != VK_SUCCESS) {
            mesa_loge("ZINK: vkEndCommandBuffer failed (%s)", vk_Result_to_str(result));
            bs->is_device_lost = true;
            goto end;
         }
      );
   }

   if (!si[ZINK_SUBMIT_SIGNAL].signalSemaphoreCount)
      num_si--;

   simple_mtx_lock(&screen->queue_lock);
   VRAM_ALLOC_LOOP(result,
      VKSCR(QueueSubmit)(screen->queue, num_si, submit, VK_NULL_HANDLE),
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkQueueSubmit failed (%s)", vk_Result_to_str(result));
         bs->is_device_lost = true;
      }
   );
   simple_mtx_unlock(&screen->queue_lock);

   unsigned i = 0;
   VkSemaphore *sem = bs->signal_semaphores.data;
   set_foreach_remove(&bs->dmabuf_exports, entry) {
      struct zink_resource *res = (void*)entry->key;
      for (; res; res = zink_resource(res->base.b.next))
         zink_screen_import_dmabuf_semaphore(screen, res, sem[i++]);

      struct pipe_resource *pres = (void*)entry->key;
      pipe_resource_reference(&pres, NULL);
   }

   bs->usage.submit_count++;
end:
   cnd_broadcast(&bs->usage.flush);

   p_atomic_set(&bs->fence.submitted, true);
   unref_resources(screen, bs);
}

/* called during flush */
void
zink_end_batch(struct zink_context *ctx, struct zink_batch *batch)
{
   if (!ctx->queries_disabled)
      zink_suspend_queries(ctx, batch);


   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (ctx->tc && !ctx->track_renderpasses)
      tc_driver_internal_flush_notify(ctx->tc);
   struct zink_batch_state *bs;

   /* oom flushing is triggered to handle stupid piglit tests like streaming-texture-leak */
   if (ctx->oom_flush || ctx->batch_states_count > 25) {
      assert(!ctx->batch_states_count || ctx->batch_states);
      while (ctx->batch_states) {
         bs = ctx->batch_states;
         struct zink_fence *fence = &bs->fence;
         /* once an incomplete state is reached, no more will be complete */
         if (!zink_check_batch_completion(ctx, fence->batch_id))
            break;

         pop_batch_state(ctx);
         zink_reset_batch_state(ctx, bs);
         if (ctx->last_free_batch_state)
            ctx->last_free_batch_state->next = bs;
         else
            ctx->free_batch_states = bs;
         ctx->last_free_batch_state = bs;
      }
      if (ctx->batch_states_count > 50)
         ctx->oom_flush = true;
   }

   bs = batch->state;
   if (ctx->last_fence)
      zink_batch_state(ctx->last_fence)->next = bs;
   else {
      assert(!ctx->batch_states);
      ctx->batch_states = bs;
   }
   ctx->last_fence = &bs->fence;
   ctx->batch_states_count++;
   batch->work_count = 0;

   /* this is swapchain presentation semaphore handling */
   if (batch->swapchain) {
      if (zink_kopper_acquired(batch->swapchain->obj->dt, batch->swapchain->obj->dt_idx) && !batch->swapchain->obj->present) {
         batch->state->present = zink_kopper_present(screen, batch->swapchain);
         batch->state->swapchain = batch->swapchain;
      }
      batch->swapchain = NULL;
   }

   if (screen->device_lost)
      return;

   if (ctx->tc) {
      set_foreach(&bs->active_queries, entry)
         zink_query_sync(ctx, (void*)entry->key);
   }

   set_foreach(&bs->dmabuf_exports, entry) {
      struct zink_resource *res = (void*)entry->key;
      if (screen->info.have_KHR_synchronization2) {
         VkImageMemoryBarrier2 imb;
         zink_resource_image_barrier2_init(&imb, res, res->layout, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
         imb.srcQueueFamilyIndex = screen->gfx_queue;
         imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         VkDependencyInfo dep = {
            VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            NULL,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &imb
         };
         VKCTX(CmdPipelineBarrier2)(bs->cmdbuf, &dep);
      } else {
         VkImageMemoryBarrier imb;
         zink_resource_image_barrier_init(&imb, res, res->layout, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
         imb.srcQueueFamilyIndex = screen->gfx_queue;
         imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
         VKCTX(CmdPipelineBarrier)(
            bs->cmdbuf,
            res->obj->access_stage,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &imb
         );
      }
      res->queue = VK_QUEUE_FAMILY_FOREIGN_EXT;

      for (; res; res = zink_resource(res->base.b.next)) {
         VkSemaphore sem = zink_create_exportable_semaphore(screen);
         if (sem)
            util_dynarray_append(&ctx->batch.state->signal_semaphores, VkSemaphore, sem);
      }
   }

   if (screen->threaded_submit) {
      util_queue_add_job(&screen->flush_queue, bs, &bs->flush_completed,
                         submit_queue, post_submit, 0);
   } else {
      submit_queue(bs, NULL, 0);
      post_submit(bs, NULL, 0);
   }
#ifdef HAVE_RENDERDOC_APP_H
   if (!(ctx->flags & ZINK_CONTEXT_COPY_ONLY) && screen->renderdoc_capturing && p_atomic_read(&screen->renderdoc_frame) > screen->renderdoc_capture_end) {
      screen->renderdoc_api->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(screen->instance), NULL);
      screen->renderdoc_capturing = false;
   }
#endif
}

static int
batch_find_resource(struct zink_batch_state *bs, struct zink_resource_object *obj, struct zink_batch_obj_list *list)
{
   unsigned hash = obj->bo->unique_id & (BUFFER_HASHLIST_SIZE-1);
   int i = bs->buffer_indices_hashlist[hash];

   /* not found or found */
   if (i < 0 || (i < list->num_buffers && list->objs[i] == obj))
      return i;

   /* Hash collision, look for the BO in the list of list->objs linearly. */
   for (int i = list->num_buffers - 1; i >= 0; i--) {
      if (list->objs[i] == obj) {
         /* Put this buffer in the hash list.
          * This will prevent additional hash collisions if there are
          * several consecutive lookup_buffer calls for the same buffer.
          *
          * Example: Assuming list->objs A,B,C collide in the hash list,
          * the following sequence of list->objs:
          *         AAAAAAAAAAABBBBBBBBBBBBBBCCCCCCCC
          * will collide here: ^ and here:   ^,
          * meaning that we should get very few collisions in the end. */
         bs->buffer_indices_hashlist[hash] = i & (BUFFER_HASHLIST_SIZE-1);
         return i;
      }
   }
   return -1;
}

void
zink_batch_reference_resource_rw(struct zink_batch *batch, struct zink_resource *res, bool write)
{
   /* if the resource already has usage of any sort set for this batch, */
   if (!zink_resource_usage_matches(res, batch->state) ||
       /* or if it's bound somewhere */
       !zink_resource_has_binds(res))
      /* then it already has a batch ref and doesn't need one here */
      zink_batch_reference_resource(batch, res);
   zink_batch_resource_usage_set(batch, res, write, res->obj->is_buffer);
}

void
zink_batch_add_wait_semaphore(struct zink_batch *batch, VkSemaphore sem)
{
   util_dynarray_append(&batch->state->acquires, VkSemaphore, sem);
}

static bool
batch_ptr_add_usage(struct zink_batch *batch, struct set *s, void *ptr)
{
   bool found = false;
   _mesa_set_search_or_add(s, ptr, &found);
   return !found;
}

/* this is a vague, handwave-y estimate */
ALWAYS_INLINE static void
check_oom_flush(struct zink_context *ctx, const struct zink_batch *batch)
{
   const VkDeviceSize resource_size = batch->state->resource_size;
   if (resource_size >= zink_screen(ctx->base.screen)->clamp_video_mem) {
       ctx->oom_flush = true;
       ctx->oom_stall = true;
    }
}

/* this adds a ref (batch tracking) */
void
zink_batch_reference_resource(struct zink_batch *batch, struct zink_resource *res)
{
   if (!zink_batch_reference_resource_move(batch, res))
      zink_resource_object_reference(NULL, NULL, res->obj);
}

/* this adds batch usage */
bool
zink_batch_reference_resource_move(struct zink_batch *batch, struct zink_resource *res)
{
   struct zink_batch_state *bs = batch->state;

   simple_mtx_lock(&batch->ref_lock);
   /* swapchains are special */
   if (zink_is_swapchain(res)) {
      struct zink_resource_object **swapchains = bs->swapchain_obj.data;
      unsigned count = util_dynarray_num_elements(&bs->swapchain_obj, struct zink_resource_object*);
      for (unsigned i = 0; i < count; i++) {
         if (swapchains[i] == res->obj) {
            simple_mtx_unlock(&batch->ref_lock);
            return true;
         }
      }
      util_dynarray_append(&bs->swapchain_obj, struct zink_resource_object*, res->obj);
      simple_mtx_unlock(&batch->ref_lock);
      return false;
   }
   /* Fast exit for no-op calls.
    * This is very effective with suballocators and linear uploaders that
    * are outside of the winsys.
    */
   if (res->obj == bs->last_added_obj) {
      simple_mtx_unlock(&batch->ref_lock);
      return true;
   }

   struct zink_bo *bo = res->obj->bo;
   struct zink_batch_obj_list *list;
   if (!(res->base.b.flags & PIPE_RESOURCE_FLAG_SPARSE)) {
      if (!bo->mem) {
         list = &bs->slab_objs;
      } else {
         list = &bs->real_objs;
      }
   } else {
      list = &bs->sparse_objs;
   }
   int idx = batch_find_resource(bs, res->obj, list);
   if (idx >= 0) {
      simple_mtx_unlock(&batch->ref_lock);
      return true;
   }

   if (list->num_buffers >= list->max_buffers) {
      unsigned new_max = MAX2(list->max_buffers + 16, (unsigned)(list->max_buffers * 1.3));
      struct zink_resource_object **objs = realloc(list->objs, new_max * sizeof(void*));
      if (!objs) {
         /* things are about to go dramatically wrong anyway */
         mesa_loge("zink: buffer list realloc failed due to oom!\n");
         abort();
      }
      list->objs = objs;
      list->max_buffers = new_max;
   }
   idx = list->num_buffers++;
   list->objs[idx] = res->obj;
   unsigned hash = bo->unique_id & (BUFFER_HASHLIST_SIZE-1);
   bs->buffer_indices_hashlist[hash] = idx & 0x7fff;
   bs->last_added_obj = res->obj;
   if (!(res->base.b.flags & PIPE_RESOURCE_FLAG_SPARSE)) {
      bs->resource_size += res->obj->size;
   } else {
      /* Sparse backing pages are not directly referenced by the batch as
       * there can be a lot of them.
       * Instead, they are kept referenced in one of two ways:
       * - While they are committed, they are directly referenced from the
       *   resource's state.
       * - Upon de-commit, they are added to the freed_sparse_backing_bos
       *   list, which will defer destroying the resource until the batch
       *   performing unbind finishes.
       */
   }
   check_oom_flush(batch->state->ctx, batch);
   batch->has_work = true;
   simple_mtx_unlock(&batch->ref_lock);
   return false;
}

/* this is how programs achieve deferred deletion */
void
zink_batch_reference_program(struct zink_batch *batch,
                             struct zink_program *pg)
{
   if (zink_batch_usage_matches(pg->batch_uses, batch->state) ||
       !batch_ptr_add_usage(batch, &batch->state->programs, pg))
      return;
   pipe_reference(NULL, &pg->reference);
   zink_batch_usage_set(&pg->batch_uses, batch->state);
   batch->has_work = true;
}

/* a fast (hopefully) way to check whether a given batch has completed */
bool
zink_screen_usage_check_completion(struct zink_screen *screen, const struct zink_batch_usage *u)
{
   if (!zink_batch_usage_exists(u))
      return true;
   if (zink_batch_usage_is_unflushed(u))
      return false;

   return zink_screen_timeline_wait(screen, u->usage, 0);
}

/* an even faster check that doesn't ioctl */
bool
zink_screen_usage_check_completion_fast(struct zink_screen *screen, const struct zink_batch_usage *u)
{
   if (!zink_batch_usage_exists(u))
      return true;
   if (zink_batch_usage_is_unflushed(u))
      return false;

   return zink_screen_check_last_finished(screen, u->usage);
}

bool
zink_batch_usage_check_completion(struct zink_context *ctx, const struct zink_batch_usage *u)
{
   if (!zink_batch_usage_exists(u))
      return true;
   if (zink_batch_usage_is_unflushed(u))
      return false;
   return zink_check_batch_completion(ctx, u->usage);
}

static void
batch_usage_wait(struct zink_context *ctx, struct zink_batch_usage *u, bool trywait)
{
   if (!zink_batch_usage_exists(u))
      return;
   if (zink_batch_usage_is_unflushed(u)) {
      if (likely(u == &ctx->batch.state->usage))
         ctx->base.flush(&ctx->base, NULL, PIPE_FLUSH_HINT_FINISH);
      else { //multi-context
         mtx_lock(&u->mtx);
         if (trywait) {
            struct timespec ts = {0, 10000};
            cnd_timedwait(&u->flush, &u->mtx, &ts);
         } else
            cnd_wait(&u->flush, &u->mtx);
         mtx_unlock(&u->mtx);
      }
   }
   zink_wait_on_batch(ctx, u->usage);
}

void
zink_batch_usage_wait(struct zink_context *ctx, struct zink_batch_usage *u)
{
   batch_usage_wait(ctx, u, false);
}

void
zink_batch_usage_try_wait(struct zink_context *ctx, struct zink_batch_usage *u)
{
   batch_usage_wait(ctx, u, true);
}
