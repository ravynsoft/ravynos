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
#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include "rmv/vk_rmv_common.h"
#include "vk_dispatch_table.h"
#include "vk_extensions.h"
#include "vk_object.h"
#include "vk_physical_device_features.h"

#include "util/list.h"
#include "util/simple_mtx.h"
#include "util/u_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_command_buffer_ops;
struct vk_sync;

enum vk_queue_submit_mode {
   /** Submits happen immediately
    *
    * `vkQueueSubmit()` and `vkQueueBindSparse()` call
    * ``vk_queue::driver_submit`` directly for all submits and the last call to
    * ``vk_queue::driver_submit`` will have completed by the time
    * `vkQueueSubmit()` or `vkQueueBindSparse()` return.
    */
   VK_QUEUE_SUBMIT_MODE_IMMEDIATE,

   /** Submits may be deferred until a future `vk_queue_flush()`
    *
    * Submits are added to the queue and `vk_queue_flush()` is called.
    * However, any submits with unsatisfied dependencies will be left on the
    * queue until a future `vk_queue_flush()` call.  This is used for
    * implementing emulated timeline semaphores without threading.
    */
   VK_QUEUE_SUBMIT_MODE_DEFERRED,

   /** Submits will be added to the queue and handled later by a thread
    *
    * This places additional requirements on the vk_sync types used by the
    * driver:
    *
    *    1. All `vk_sync` types which support `VK_SYNC_FEATURE_GPU_WAIT` also
    *       support `VK_SYNC_FEATURE_WAIT_PENDING` so that the threads can
    *       sort out when a given submit has all its dependencies resolved.
    *
    *    2. All binary `vk_sync` types which support `VK_SYNC_FEATURE_GPU_WAIT`
    *       also support `VK_SYNC_FEATURE_CPU_RESET` so we can reset
    *       semaphores after waiting on them.
    *
    *    3. All vk_sync types used as permanent payloads of semaphores support
    *       ``vk_sync_type::move`` so that it can move the pending signal into a
    *       temporary vk_sync and reset the semaphore.
    *
    * This is requied for shared timeline semaphores where we need to handle
    * wait-before-signal by threading in the driver if we ever see an
    * unresolve dependency.
    */
   VK_QUEUE_SUBMIT_MODE_THREADED,

   /** Threaded but only if we need it to resolve dependencies
    *
    * This imposes all the same requirements on `vk_sync` types as
    * `VK_QUEUE_SUBMIT_MODE_THREADED`.
    */
   VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND,
};

/** Base struct for VkDevice */
struct vk_device {
   struct vk_object_base base;

   /** Allocator used to create this device
    *
    * This is used as a fall-back for when a NULL pAllocator is passed into a
    * device-level create function such as vkCreateImage().
    */
   VkAllocationCallbacks alloc;

   /** Pointer to the physical device */
   struct vk_physical_device *physical;

   /** Table of enabled extensions */
   struct vk_device_extension_table enabled_extensions;

   /** Table of enabled features */
   struct vk_features enabled_features;

   /** Device-level dispatch table */
   struct vk_device_dispatch_table dispatch_table;

   /** Command dispatch table
    *
    * This is used for emulated secondary command buffer support.  To use
    * emulated (trace/replay) secondary command buffers:
    *
    *  1. Provide your "real" command buffer dispatch table here.  Because
    *     this doesn't get populated by vk_device_init(), the driver will have
    *     to add the vk_common entrypoints to this table itself.
    *
    *  2. Add vk_enqueue_unless_primary_device_entrypoint_table to your device
    *     level dispatch table.
    */
   const struct vk_device_dispatch_table *command_dispatch_table;

   /** Command buffer vtable when using the common command pool */
   const struct vk_command_buffer_ops *command_buffer_ops;

   /** Driver provided callback for capturing traces
    * 
    * Triggers for this callback are:
    *    - Keyboard input (F12)
    *    - Creation of a trigger file
    *    - Reaching the trace frame
    */
   VkResult (*capture_trace)(VkQueue queue);

   uint32_t current_frame;
   bool trace_hotkey_trigger;
   simple_mtx_t trace_mtx;

   /* For VK_EXT_private_data */
   uint32_t private_data_next_index;

   struct list_head queues;

   struct {
      int lost;
      bool reported;
   } _lost;

   /** Checks the status of this device
    *
    * This is expected to return either VK_SUCCESS or VK_ERROR_DEVICE_LOST.
    * It is called before ``vk_queue::driver_submit`` and after every non-trivial
    * wait operation to ensure the device is still around.  This gives the
    * driver a hook to ask the kernel if its device is still valid.  If the
    * kernel says the device has been lost, it MUST call vk_device_set_lost().
    *
    * This function may be called from any thread at any time.
    */
   VkResult (*check_status)(struct vk_device *device);

   /** Creates a vk_sync that wraps a memory object
    *
    * This is always a one-shot object so it need not track any additional
    * state.  Since it's intended for synchronizing between processes using
    * implicit synchronization mechanisms, no such tracking would be valid
    * anyway.
    *
    * If `signal_memory` is set, the resulting vk_sync will be used to signal
    * the memory object from a queue ``via vk_queue_submit::signals``.  The common
    * code guarantees that, by the time vkQueueSubmit() returns, the signal
    * operation has been submitted to the kernel via the driver's
    * ``vk_queue::driver_submit`` hook.  This means that any vkQueueSubmit() call
    * which needs implicit synchronization may block.
    *
    * If `signal_memory` is not set, it can be assumed that memory object
    * already has a signal operation pending from some other process and we
    * need only wait on it.
    */
   VkResult (*create_sync_for_memory)(struct vk_device *device,
                                      VkDeviceMemory memory,
                                      bool signal_memory,
                                      struct vk_sync **sync_out);

   /* Set by vk_device_set_drm_fd() */
   int drm_fd;

   /** An enum describing how timeline semaphores work */
   enum vk_device_timeline_mode {
      /** Timeline semaphores are not supported */
      VK_DEVICE_TIMELINE_MODE_NONE,

      /** Timeline semaphores are emulated with vk_timeline
       *
       * In this mode, timeline semaphores are emulated using vk_timeline
       * which is a collection of binary semaphores, one per time point.
       * These timeline semaphores cannot be shared because the data structure
       * exists entirely in userspace.  These timelines are virtually
       * invisible to the driver; all it sees are the binary vk_syncs, one per
       * time point.
       *
       * To handle wait-before-signal, we place all vk_queue_submits in the
       * queue's submit list in vkQueueSubmit() and call vk_device_flush() at
       * key points such as the end of vkQueueSubmit() and vkSemaphoreSignal().
       * This ensures that, as soon as a given submit's dependencies are fully
       * resolvable, it gets submitted to the driver.
       */
      VK_DEVICE_TIMELINE_MODE_EMULATED,

      /** Timeline semaphores are a kernel-assisted emulation
       *
       * In this mode, timeline semaphores are still technically an emulation
       * in the sense that they don't support wait-before-signal natively.
       * Instead, all GPU-waitable objects support a CPU wait-for-pending
       * operation which lets the userspace driver wait until a given event
       * on the (possibly shared) vk_sync is pending.  The event is "pending"
       * if a job has been submitted to the kernel (possibly from a different
       * process) which will signal it.  In vkQueueSubit, we use this wait
       * mode to detect waits which are not yet pending and, the first time we
       * do, spawn a thread to manage the queue.  That thread waits for each
       * submit's waits to all be pending before submitting to the driver
       * queue.
       *
       * We have to be a bit more careful about a few things in this mode.
       * In particular, we can never assume that any given wait operation is
       * pending.  For instance, when we go to export a sync file from a
       * binary semaphore, we need to first wait for it to be pending.  The
       * spec guarantees that the vast majority of these waits return almost
       * immediately, but we do need to insert them for correctness.
       */
      VK_DEVICE_TIMELINE_MODE_ASSISTED,

      /** Timeline semaphores are 100% native
       *
       * In this mode, wait-before-signal is natively supported by the
       * underlying timeline implementation.  We can submit-and-forget and
       * assume that dependencies will get resolved for us by the kernel.
       * Currently, this isn't supported by any Linux primitives.
       */
      VK_DEVICE_TIMELINE_MODE_NATIVE,
   } timeline_mode;

   /** Per-device submit mode
    *
    * This represents the device-wide submit strategy which may be different
    * from the per-queue submit mode.  See vk_queue.submit.mode for more
    * details.
    */
   enum vk_queue_submit_mode submit_mode;

   struct vk_memory_trace_data memory_trace_data;

   mtx_t swapchain_private_mtx;
   struct hash_table *swapchain_private;
   mtx_t swapchain_name_mtx;
   struct hash_table *swapchain_name;
};

VK_DEFINE_HANDLE_CASTS(vk_device, base, VkDevice,
                       VK_OBJECT_TYPE_DEVICE);

/** Initialize a vk_device
 *
 * Along with initializing the data structures in `vk_device`, this function
 * checks that every extension specified by
 * ``VkInstanceCreateInfo::ppEnabledExtensionNames`` is actually supported by
 * the physical device and returns `VK_ERROR_EXTENSION_NOT_PRESENT` if an
 * unsupported extension is requested.  It also checks all the feature struct
 * chained into the `pCreateInfo->pNext` chain against the features returned
 * by `vkGetPhysicalDeviceFeatures2` and returns
 * `VK_ERROR_FEATURE_NOT_PRESENT` if an unsupported feature is requested.
 *
 * :param device:               |out| The device to initialize
 * :param physical_device:      |in|  The physical device
 * :param dispatch_table:       |in|  Device-level dispatch table
 * :param pCreateInfo:          |in|  VkDeviceCreateInfo pointer passed to
 *                                    `vkCreateDevice()`
 * :param alloc:                |in|  Allocation callbacks passed to
 *                                    `vkCreateDevice()`
 */
VkResult MUST_CHECK
vk_device_init(struct vk_device *device,
               struct vk_physical_device *physical_device,
               const struct vk_device_dispatch_table *dispatch_table,
               const VkDeviceCreateInfo *pCreateInfo,
               const VkAllocationCallbacks *alloc);

static inline void
vk_device_set_drm_fd(struct vk_device *device, int drm_fd)
{
   device->drm_fd = drm_fd;
}

/** Tears down a vk_device
 *
 * :param device:       |out| The device to tear down
 */
void
vk_device_finish(struct vk_device *device);

/** Enables threaded submit on this device
 *
 * This doesn't ensure that threaded submit will be used.  It just disables
 * the deferred submit option for emulated timeline semaphores and forces them
 * to always use the threaded path.  It also does some checks that the vk_sync
 * types used by the driver work for threaded submit.
 *
 * This must be called before any queues are created.
 */
void vk_device_enable_threaded_submit(struct vk_device *device);

static inline bool
vk_device_supports_threaded_submit(const struct vk_device *device)
{
   return device->submit_mode == VK_QUEUE_SUBMIT_MODE_THREADED ||
          device->submit_mode == VK_QUEUE_SUBMIT_MODE_THREADED_ON_DEMAND;
}

VkResult vk_device_flush(struct vk_device *device);

VkResult PRINTFLIKE(4, 5)
_vk_device_set_lost(struct vk_device *device,
                    const char *file, int line,
                    const char *msg, ...);

#define vk_device_set_lost(device, ...) \
   _vk_device_set_lost(device, __FILE__, __LINE__, __VA_ARGS__)

void _vk_device_report_lost(struct vk_device *device);

static inline bool
vk_device_is_lost_no_report(struct vk_device *device)
{
   return p_atomic_read(&device->_lost.lost) > 0;
}

static inline bool
vk_device_is_lost(struct vk_device *device)
{
   int lost = vk_device_is_lost_no_report(device);
   if (unlikely(lost && !device->_lost.reported))
      _vk_device_report_lost(device);
   return lost;
}

static inline VkResult
vk_device_check_status(struct vk_device *device)
{
   if (vk_device_is_lost(device))
      return VK_ERROR_DEVICE_LOST;

   if (!device->check_status)
      return VK_SUCCESS;

   VkResult result = device->check_status(device);

   assert(result == VK_SUCCESS || result == VK_ERROR_DEVICE_LOST);
   if (result == VK_ERROR_DEVICE_LOST)
      assert(vk_device_is_lost_no_report(device));

   return result;
}

#ifndef _WIN32

uint64_t
vk_clock_gettime(clockid_t clock_id);

static inline uint64_t
vk_time_max_deviation(uint64_t begin, uint64_t end, uint64_t max_clock_period)
{
    /*
     * The maximum deviation is the sum of the interval over which we
     * perform the sampling and the maximum period of any sampled
     * clock. That's because the maximum skew between any two sampled
     * clock edges is when the sampled clock with the largest period is
     * sampled at the end of that period but right at the beginning of the
     * sampling interval and some other clock is sampled right at the
     * beginning of its sampling period and right at the end of the
     * sampling interval. Let's assume the GPU has the longest clock
     * period and that the application is sampling GPU and monotonic:
     *
     *                               s                 e
     *			 w x y z 0 1 2 3 4 5 6 7 8 9 a b c d e f
     *	Raw              -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
     *
     *                               g
     *		  0         1         2         3
     *	GPU       -----_____-----_____-----_____-----_____
     *
     *                                                m
     *					    x y z 0 1 2 3 4 5 6 7 8 9 a b c
     *	Monotonic                           -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
     *
     *	Interval                     <----------------->
     *	Deviation           <-------------------------->
     *
     *		s  = read(raw)       2
     *		g  = read(GPU)       1
     *		m  = read(monotonic) 2
     *		e  = read(raw)       b
     *
     * We round the sample interval up by one tick to cover sampling error
     * in the interval clock
     */

   uint64_t sample_interval = end - begin + 1;

   return sample_interval + max_clock_period;
}

#endif //!_WIN32

PFN_vkVoidFunction
vk_device_get_proc_addr(const struct vk_device *device,
                        const char *name);

bool vk_get_physical_device_core_1_1_property_ext(struct VkBaseOutStructure *ext,
                                                     const VkPhysicalDeviceVulkan11Properties *core);
bool vk_get_physical_device_core_1_2_property_ext(struct VkBaseOutStructure *ext,
                                                     const VkPhysicalDeviceVulkan12Properties *core);
bool vk_get_physical_device_core_1_3_property_ext(struct VkBaseOutStructure *ext,
                                                     const VkPhysicalDeviceVulkan13Properties *core);

#ifdef __cplusplus
}
#endif

#endif /* VK_DEVICE_H */
