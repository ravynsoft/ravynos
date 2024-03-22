/*
 * Copyright © 2021 Intel Corporation
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
#ifndef VK_SYNC_H
#define VK_SYNC_H

#include <stdbool.h>
#include <vulkan/vulkan_core.h>

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_device;
struct vk_sync;

enum vk_sync_features {
   /** Set if a sync type supports the binary mode of operation
    *
    * In binary mode, a vk_sync has two modes: signaled and unsignaled.  If
    * it supports CPU_RESET, it can be changed from signaled to unsignaled on
    * the CPU via vk_sync_reset().  If it supports CPU_SIGNAL, it can be
    * changed from unsignaled to signaled on the CPU via vk_sync_signal().
    *
    * Binary vk_sync types may also support WAIT_PENDING in which they have a
    * third hidden pending state.  Once such a vk_sync has been submitted to
    * the kernel driver for signaling, it is in the pending state and remains
    * there until the work is complete at which point it enters the signaled
    * state.  This pending state is visible across processes for shared
    * vk_sync types.  This is used to by the threaded submit mode to ensure
    * that everything gets submitted to the kernel driver in-order.
    *
    * A vk_sync operates in binary mode if VK_SYNC_IS_TIMELINE is not set
    * in vk_sync::flags.
    */
   VK_SYNC_FEATURE_BINARY              = (1 << 0),

   /** Set if a sync type supports the timeline mode of operation
    *
    * In timeline mode, a vk_sync has a monotonically increasing 64-bit value
    * which represents most recently signaled time point.  Waits are relative
    * to time points.  Instead of waiting for the vk_sync to enter a signaled
    * state, you wait for its 64-bit value to be at least some wait value.
    *
    * Timeline vk_sync types can also support WAIT_PENDING.  In this case, the
    * wait is not for a pending state, as such, but rather for someone to have
    * submitted a kernel request which will signal a time point with at least
    * that value.  Logically, you can think of this as having two timelines,
    * the real timeline and a pending timeline which runs slightly ahead of
    * the real one.  As with binary vk_sync types, this is used by threaded
    * submit to re-order things so that the kernel requests happen in a valid
    * linear order.
    *
    * A vk_sync operates in timeline mode if VK_SYNC_IS_TIMELINE is set in
    * vk_sync::flags.
    */
   VK_SYNC_FEATURE_TIMELINE            = (1 << 1),

   /** Set if this sync supports GPU waits */
   VK_SYNC_FEATURE_GPU_WAIT            = (1 << 2),

   /** Set if a sync type supports multiple GPU waits on one signal state
    *
    * The Vulkan spec for VkSemaphore requires GPU wait and signal operations
    * to have a one-to-one relationship.  This formally described by saying
    * that the VkSemaphore gets implicitly reset on wait.  However, it is
    * often useful to have well-defined multi-wait.  If binary vk_sync
    * supports multi-wait then any number of kernel requests can be submitted
    * which wait on one signal operation.  This also implies that you can
    * signal twice back-to-back (there are 0 waits on the first signal).
    *
    * This feature only applies to binary vk_sync objects.
    */
   VK_SYNC_FEATURE_GPU_MULTI_WAIT      = (1 << 3),

   /** Set if a sync type supports vk_sync_wait() and vk_sync_wait_many() */
   VK_SYNC_FEATURE_CPU_WAIT            = (1 << 4),

   /** Set if a sync type supports vk_sync_reset()
    *
    * This feature only applies to binary vk_sync objects.
    */
   VK_SYNC_FEATURE_CPU_RESET           = (1 << 5),

   /** Set if a sync type supports vk_sync_signal() */
   VK_SYNC_FEATURE_CPU_SIGNAL          = (1 << 6),

   /** Set if sync_type::wait_many supports the VK_SYNC_WAIT_ANY bit
    *
    * vk_sync_wait_many() will support the bit regardless.  If the sync type
    * doesn't support it natively, it will be emulated.
    */
   VK_SYNC_FEATURE_WAIT_ANY            = (1 << 7),

   /** Set if a sync type supports the VK_SYNC_WAIT_PENDING bit
    *
    * See VK_SYNC_FEATURE_BINARY and VK_SYNC_FEATURE_TIMELINE for descriptions
    * of what this does in each case.
    */
   VK_SYNC_FEATURE_WAIT_PENDING        = (1 << 8),

   /** Set if a sync type natively supports wait-before-signal
    *
    * If this is set then the underlying OS primitive supports submitting
    * kernel requests which wait on the vk_sync before submitting a kernel
    * request which would cause that wait to unblock.
    */
   VK_SYNC_FEATURE_WAIT_BEFORE_SIGNAL  = (1 << 9),
};

struct vk_sync_wait;

enum vk_sync_wait_flags {
   /** Placeholder for 0 to make vk_sync_wait() calls more clear */
   VK_SYNC_WAIT_COMPLETE   = 0,

   /** If set, only wait for the vk_sync operation to be pending
    *
    * See VK_SYNC_FEATURE_BINARY and VK_SYNC_FEATURE_TIMELINE for descriptions
    * of what this does in each case.
    */
   VK_SYNC_WAIT_PENDING    = (1 << 0),

   /** If set, wait for any of of the vk_sync operations to complete
    *
    * This is as opposed to waiting for all of them.  There is no guarantee
    * that vk_sync_wait_many() will return immediately after the first
    * operation completes but it will make a best effort to return as soon as
    * possible.
    */
   VK_SYNC_WAIT_ANY        = (1 << 1),
};

struct vk_sync_type {
   /** Size of this sync type */
   size_t size;

   /** Features supported by this sync type */
   enum vk_sync_features features;

   /** Initialize a vk_sync
    *
    * The base vk_sync will already be initialized and the sync type set
    * before this function is called.  If any OS primitives need to be
    * allocated, that should be done here.
    */
   VkResult (*init)(struct vk_device *device,
                    struct vk_sync *sync,
                    uint64_t initial_value);

   /** Finish a vk_sync
    *
    * This should free any internal data stored in this vk_sync.
    */
   void (*finish)(struct vk_device *device,
                  struct vk_sync *sync);

   /** Signal a vk_sync
    *
    * For non-timeline sync types, value == 0.
    */
   VkResult (*signal)(struct vk_device *device,
                      struct vk_sync *sync,
                      uint64_t value);

   /** Get the timeline value for a vk_sync */
   VkResult (*get_value)(struct vk_device *device,
                         struct vk_sync *sync,
                         uint64_t *value);

   /** Reset a non-timeline vk_sync */
   VkResult (*reset)(struct vk_device *device,
                     struct vk_sync *sync);

   /** Moves the guts of one binary vk_sync to another
    *
    * This moves the current binary vk_sync event from src to dst and resets
    * src.  If dst contained an event, it is discarded.
    *
    * This is required for all binary vk_sync types that can be used for a
    * semaphore wait in conjunction with real timeline semaphores.
    */
   VkResult (*move)(struct vk_device *device,
                    struct vk_sync *dst,
                    struct vk_sync *src);

   /** Wait on a vk_sync
    *
    * For a timeline vk_sync, wait_value is the timeline value to wait for.
    * This function should not return VK_SUCCESS until get_value on that
    * vk_sync would return a value >= wait_value.  A wait_value of zero is
    * allowed in which case the wait is a no-op.  For a non-timeline vk_sync,
    * wait_value should be ignored.
    *
    * This function is optional.  If the sync type needs to support CPU waits,
    * at least one of wait or wait_many must be provided.  If one is missing,
    * it will be implemented in terms of the other.
    */
   VkResult (*wait)(struct vk_device *device,
                    struct vk_sync *sync,
                    uint64_t wait_value,
                    enum vk_sync_wait_flags wait_flags,
                    uint64_t abs_timeout_ns);

   /** Wait for multiple vk_sync events
    *
    * If VK_SYNC_WAIT_ANY is set, it will return after at least one of the
    * wait events is complete instead of waiting for all of them.
    *
    * See wait for more details.
    */
   VkResult (*wait_many)(struct vk_device *device,
                         uint32_t wait_count,
                         const struct vk_sync_wait *waits,
                         enum vk_sync_wait_flags wait_flags,
                         uint64_t abs_timeout_ns);

   /** Permanently imports the given FD into this vk_sync
    *
    * This replaces the guts of the given vk_sync with whatever is in the FD.
    * In a sense, this vk_sync now aliases whatever vk_sync the FD was
    * exported from.
    */
   VkResult (*import_opaque_fd)(struct vk_device *device,
                                struct vk_sync *sync,
                                int fd);

   /** Export the guts of this vk_sync to an FD */
   VkResult (*export_opaque_fd)(struct vk_device *device,
                                struct vk_sync *sync,
                                int *fd);

   /** Imports a sync file into this binary vk_sync
    *
    * If this completes successfully, the vk_sync will now signal whenever
    * the sync file signals.
    *
    * If sync_file == -1, the vk_sync should be signaled immediately.  If
    * the vk_sync_type implements signal, sync_file will never be -1.
    */
   VkResult (*import_sync_file)(struct vk_device *device,
                                struct vk_sync *sync,
                                int sync_file);

   /** Exports the current binary vk_sync state as a sync file.
    *
    * The resulting sync file will contain the current event stored in this
    * binary vk_sync must be turned into a sync file.  If the vk_sync is later
    * modified to contain a new event, the sync file is unaffected.
    */
   VkResult (*export_sync_file)(struct vk_device *device,
                                struct vk_sync *sync,
                                int *sync_file);

   /** Permanently imports the given handle or name into this vk_sync
    *
    * This replaces the guts of the given vk_sync with whatever is in the object.
    * In a sense, this vk_sync now aliases whatever vk_sync the handle was
    * exported from.
    */
   VkResult (*import_win32_handle)(struct vk_device *device,
                                   struct vk_sync *sync,
                                   void *handle,
                                   const wchar_t *name);

   /** Export the guts of this vk_sync to a handle and/or name */
   VkResult (*export_win32_handle)(struct vk_device *device,
                                   struct vk_sync *sync,
                                   void **handle);

   /** Vulkan puts these as creation params instead of export params */
   VkResult (*set_win32_export_params)(struct vk_device *device,
                                       struct vk_sync *sync,
                                       const void *security_attributes,
                                       uint32_t access,
                                       const wchar_t *name);
};

enum vk_sync_flags {
   /** Set if the vk_sync is a timeline */
   VK_SYNC_IS_TIMELINE  = (1 << 0),

   /** Set if the vk_sync can have its payload shared */
   VK_SYNC_IS_SHAREABLE = (1 << 1),

   /** Set if the vk_sync has a shared payload */
   VK_SYNC_IS_SHARED    = (1 << 2),
};

struct vk_sync {
   const struct vk_sync_type *type;
   enum vk_sync_flags flags;
};

/* See VkSemaphoreSubmitInfo */
struct vk_sync_wait {
   struct vk_sync *sync;
   VkPipelineStageFlags2 stage_mask;
   uint64_t wait_value;
};

/* See VkSemaphoreSubmitInfo */
struct vk_sync_signal {
   struct vk_sync *sync;
   VkPipelineStageFlags2 stage_mask;
   uint64_t signal_value;
};

VkResult MUST_CHECK vk_sync_init(struct vk_device *device,
                                 struct vk_sync *sync,
                                 const struct vk_sync_type *type,
                                 enum vk_sync_flags flags,
                                 uint64_t initial_value);

void vk_sync_finish(struct vk_device *device,
                    struct vk_sync *sync);

VkResult MUST_CHECK vk_sync_create(struct vk_device *device,
                                   const struct vk_sync_type *type,
                                   enum vk_sync_flags flags,
                                   uint64_t initial_value,
                                   struct vk_sync **sync_out);

void vk_sync_destroy(struct vk_device *device,
                     struct vk_sync *sync);

VkResult MUST_CHECK vk_sync_signal(struct vk_device *device,
                                   struct vk_sync *sync,
                                   uint64_t value);

VkResult MUST_CHECK vk_sync_get_value(struct vk_device *device,
                                      struct vk_sync *sync,
                                      uint64_t *value);

VkResult MUST_CHECK vk_sync_reset(struct vk_device *device,
                                  struct vk_sync *sync);

VkResult MUST_CHECK vk_sync_wait(struct vk_device *device,
                                 struct vk_sync *sync,
                                 uint64_t wait_value,
                                 enum vk_sync_wait_flags wait_flags,
                                 uint64_t abs_timeout_ns);

VkResult MUST_CHECK vk_sync_wait_many(struct vk_device *device,
                                      uint32_t wait_count,
                                      const struct vk_sync_wait *waits,
                                      enum vk_sync_wait_flags wait_flags,
                                      uint64_t abs_timeout_ns);

VkResult MUST_CHECK vk_sync_import_opaque_fd(struct vk_device *device,
                                             struct vk_sync *sync,
                                             int fd);

VkResult MUST_CHECK vk_sync_export_opaque_fd(struct vk_device *device,
                                             struct vk_sync *sync,
                                             int *fd);

VkResult MUST_CHECK vk_sync_import_sync_file(struct vk_device *device,
                                             struct vk_sync *sync,
                                             int sync_file);

VkResult MUST_CHECK vk_sync_export_sync_file(struct vk_device *device,
                                             struct vk_sync *sync,
                                             int *sync_file);

VkResult MUST_CHECK vk_sync_import_win32_handle(struct vk_device *device,
                                                struct vk_sync *sync,
                                                void *handle,
                                                const wchar_t *name);

VkResult MUST_CHECK vk_sync_export_win32_handle(struct vk_device *device,
                                                struct vk_sync *sync,
                                                void **handle);

VkResult MUST_CHECK vk_sync_set_win32_export_params(struct vk_device *device,
                                                    struct vk_sync *sync,
                                                    const void *security_attributes,
                                                    uint32_t access,
                                                    const wchar_t *name);

VkResult MUST_CHECK vk_sync_move(struct vk_device *device,
                                 struct vk_sync *dst,
                                 struct vk_sync *src);

#ifdef __cplusplus
}
#endif

#endif /* VK_SYNC_H */
