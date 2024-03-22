/*
 * Copyright © 2023 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 *
 * This file exposes some core KMD functionalities in a driver-agnostic way.
 * The drivers are still assumed to be regular DRM drivers, such that some
 * operations can be handled generically.
 *
 * Any operation that's too specific to be abstracted can either have a backend
 * specific helper exposed through pan_kmod_<backend>.h, or no helper at all
 * (in the latter case, users are expected to call the ioctl directly).
 *
 * If some operations are not natively supported by a KMD, the kmod backend
 * should fail or emulate the functionality (if deemed necessary).
 */

#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>

#include "drm-uapi/drm.h"

#include "util/log.h"
#include "util/macros.h"
#include "util/os_file.h"
#include "util/os_mman.h"
#include "util/ralloc.h"
#include "util/simple_mtx.h"
#include "util/sparse_array.h"
#include "util/u_atomic.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct pan_kmod_dev;

/* GPU VM creation flags. */
enum pan_kmod_vm_flags {
   /* Set if you want the VM to automatically assign virtual addresses when
    * pan_kmod_vm_map(). If this flag is set, all pan_kmod_vm_map() calls
    * must have va=PAN_KMOD_VM_MAP_AUTO_VA.
    */
   PAN_KMOD_VM_FLAG_AUTO_VA = BITFIELD_BIT(0),
};

/* Object representing a GPU VM. */
struct pan_kmod_vm {
   /* Combination of pan_kmod_vm_flags flags. */
   uint32_t flags;

   /* The VM handle returned by the KMD. If the KMD supports only one VM per
    * context, this should be zero.
    */
   uint32_t handle;

   /* Device this VM was created from. */
   struct pan_kmod_dev *dev;
};

/* Buffer object flags. */
enum pan_kmod_bo_flags {
   /* Allow GPU execution on this buffer. */
   PAN_KMOD_BO_FLAG_EXECUTABLE = BITFIELD_BIT(0),

   /* Allocate memory when a GPU fault occurs instead of allocating
    * up-front.
    */
   PAN_KMOD_BO_FLAG_ALLOC_ON_FAULT = BITFIELD_BIT(1),

   /* If set, the buffer object will never be CPU-mapped in userspace. */
   PAN_KMOD_BO_FLAG_NO_MMAP = BITFIELD_BIT(2),

   /* Set when the buffer object has been exported. Users don't directly
    * control this flag, it's set when pan_kmod_bo_export() is called.
    */
   PAN_KMOD_BO_FLAG_EXPORTED = BITFIELD_BIT(3),

   /* Set when the buffer object has been impported. Users don't directly
    * control this flag, it's set when pan_kmod_bo_import() is called.
    */
   PAN_KMOD_BO_FLAG_IMPORTED = BITFIELD_BIT(4),

   /* If set, the buffer in mapped GPU-uncached when pan_kmod_vm_map()
    * is called.
    */
   PAN_KMOD_BO_FLAG_GPU_UNCACHED = BITFIELD_BIT(5),
};

/* Buffer object. */
struct pan_kmod_bo {
   /* Atomic reference count. The only reason we need to refcnt BOs at this
    * level is because of how DRM prime import works: the import logic
    * returns the handle of an existing object if the object was previously
    * imported or was created by the driver.
    * In order to prevent call GEM_CLOSE on an object that's still supposed
    * to be active, we need count the number of users left.
    */
   int32_t refcnt;

   /* Size of the buffer object. */
   size_t size;

   /* Handle attached to the buffer object. */
   uint32_t handle;

   /* Combination of pan_kmod_bo_flags flags. */
   uint32_t flags;

   /* If non-NULL, the buffer object can only by mapped on this VM. Typical
    * the case for all internal/non-shareable buffers. The backend can
    * optimize things based on this information. Calling pan_kmod_bo_export()
    * on such buffer objects is forbidden.
    */
   struct pan_kmod_vm *exclusive_vm;

   /* The device this buffer object was created from. */
   struct pan_kmod_dev *dev;

   /* User private data. Use pan_kmod_bo_{set,get}_user_priv() to access it. */
   void *user_priv;
};

/* List of GPU properties needed by the UMD. */
struct pan_kmod_dev_props {
   /* GPU product ID. */
   uint32_t gpu_prod_id;

   /* GPU revision. */
   uint32_t gpu_revision;

   /* Bitmask encoding the number of shader cores exposed by the GPU. */
   uint64_t shader_present;

   /* Tiler features bits. */
   uint32_t tiler_features;

   /* Memory related feature bits. */
   uint32_t mem_features;

   /* MMU feature bits. */
   uint32_t mmu_features;
#define MMU_FEATURES_VA_BITS(mmu_features) (mmu_features & 0xff)

   /* Texture feature bits. */
   uint32_t texture_features[4];

   /* Maximum number of threads per core. */
   uint32_t thread_tls_alloc;

   /* AFBC feature bits. */
   uint32_t afbc_features;
};

/* Memory allocator for kmod internal allocations. */
struct pan_kmod_allocator {
   /* Allocate and set to zero. */
   void *(*zalloc)(const struct pan_kmod_allocator *allocator, size_t size,
                   bool transient);

   /* Free. */
   void (*free)(const struct pan_kmod_allocator *allocator, void *data);

   /* Private data allocator data. Can be NULL if unused. */
   void *priv;
};

/* Synchronization type. */
enum pan_kmod_sync_type {
   PAN_KMOD_SYNC_TYPE_WAIT = 0,
   PAN_KMOD_SYNC_TYPE_SIGNAL,
};

/* Synchronization operation. */
struct pan_kmod_sync_op {
   /* Type of operation. */
   enum pan_kmod_sync_type type;

   /* Syncobj handle. */
   uint32_t handle;

   /* Syncobj point. Zero for binary syncobjs. */
   uint64_t point;
};

/* Special value passed to pan_kmod_vm_map() to signify the VM it should
 * automatically allocate a VA. Only valid if the VM was created with
 * PAN_KMOD_VM_FLAG_AUTO_VA.
 */
#define PAN_KMOD_VM_MAP_AUTO_VA ~0ull

/* Special value return when the vm_map() operation failed. */
#define PAN_KMOD_VM_MAP_FAILED ~0ull

/* VM operations can be executed in different modes. */
enum pan_kmod_vm_op_mode {
   /* The map/unmap operation is executed immediately, which might cause
    * GPU faults if the GPU was still accessing buffers when we unmap or
    * remap.
    */
   PAN_KMOD_VM_OP_MODE_IMMEDIATE,

   /* The map/unmap operation is executed asynchronously, and the user
    * provides explicit wait/signal sync operations.
    */
   PAN_KMOD_VM_OP_MODE_ASYNC,

   /* The map/unmap operation is executed when the next GPU/VM idle-point
    * is reached. This guarantees fault-free unmap/remap operations when the
    * kmod user doesn't want to deal with synchronizations explicitly.
    */
   PAN_KMOD_VM_OP_MODE_DEFER_TO_NEXT_IDLE_POINT,
};

/* VM operation type. */
enum pan_kmod_vm_op_type {
   /* Map a buffer object. */
   PAN_KMOD_VM_OP_TYPE_MAP,

   /* Unmap a VA range. */
   PAN_KMOD_VM_OP_TYPE_UNMAP,

   /* Do nothing. Used as a way to execute sync operations on a VM queue,
    * without touching the VM.
    */
   PAN_KMOD_VM_OP_TYPE_SYNC_ONLY,
};

/* VM operation data. */
struct pan_kmod_vm_op {
   /* The type of operation being requested. */
   enum pan_kmod_vm_op_type type;

   /* VA range. */
   struct {
      /* Start of the VA range.
       * Must be PAN_KMOD_VM_MAP_AUTO_VA if PAN_KMOD_VM_FLAG_AUTO_VA was set
       * at VM creation time. In that case, the allocated VA is returned
       * in this field.
       */
      uint64_t start;

      /* Size of the VA range */
      size_t size;
   } va;

   union {
      /* Arguments specific to map operations. */
      struct {
         /* Buffer object to map. */
         struct pan_kmod_bo *bo;

         /* Offset in the buffer object. */
         off_t bo_offset;
      } map;
   };

   /* Synchronization operations attached to the VM operation. */
   struct {
      /* Number of synchronization operations. Must be zero if mode is
       * PAN_KMOD_VM_OP_MODE_IMMEDIATE or PAN_KMOD_VM_OP_MODE_WAIT_IDLE.
       */
      uint32_t count;

      /* Array of synchronization operation descriptors. NULL if count is zero. */
      const struct pan_kmod_sync_op *array;
   } syncs;
};

/* VM state. */
enum pan_kmod_vm_state {
   PAN_KMOD_VM_USABLE,
   PAN_KMOD_VM_FAULTY,
};

/* Device flags. */
enum pan_kmod_dev_flags {
   /* Set when the fd passed to pan_kmod_create() is expected to be
    * owned by the device, iff the device creation succeeded.
    */
   PAN_KMOD_DEV_FLAG_OWNS_FD = (1 << 0),
};

/* Encode a virtual address range. */
struct pan_kmod_va_range {
   /* Start of the VA range. */
   uint64_t start;

   /* Size of the VA range. */
   uint64_t size;
};

/* KMD backend vtable.
 *
 * All methods described there are mandatory, unless explicitly flagged as
 * optional.
 */
struct pan_kmod_ops {
   /* Create a pan_kmod_dev object.
    * Return NULL if the creation fails for any reason.
    */
   struct pan_kmod_dev *(*dev_create)(
      int fd, uint32_t flags, const drmVersionPtr version,
      const struct pan_kmod_allocator *allocator);

   /* Destroy a pan_kmod_dev object. */
   void (*dev_destroy)(struct pan_kmod_dev *dev);

   /* Query device properties. */
   void (*dev_query_props)(const struct pan_kmod_dev *dev,
                           struct pan_kmod_dev_props *props);

   /* Query the maxium user VA range.
    * Users are free to use a subset of this range if they need less VA space.
    * This method is optional, when not specified, kmod assumes the whole VA
    * space (extracted from MMU_FEATURES.VA_BITS) is usable.
    */
   struct pan_kmod_va_range (*dev_query_user_va_range)(
      const struct pan_kmod_dev *dev);

   /* Allocate a buffer object.
    * Return NULL if the creation fails for any reason.
    */
   struct pan_kmod_bo *(*bo_alloc)(struct pan_kmod_dev *dev,
                                   struct pan_kmod_vm *exclusive_vm,
                                   size_t size, uint32_t flags);

   /* Free buffer object. */
   void (*bo_free)(struct pan_kmod_bo *bo);

   /* Import a buffer object.
    * Return NULL if the import fails for any reason.
    */
   struct pan_kmod_bo *(*bo_import)(struct pan_kmod_dev *dev, uint32_t handle,
                                    size_t size, uint32_t flags);

   /* Post export operations.
    * Return 0 on success, -1 otherwise.
    * This method is optional.
    */
   int (*bo_export)(struct pan_kmod_bo *bo, int dmabuf_fd);

   /* Get the file offset to use to mmap() a buffer object. */
   off_t (*bo_get_mmap_offset)(struct pan_kmod_bo *bo);

   /* Wait for a buffer object to be ready for read or read/write accesses. */
   bool (*bo_wait)(struct pan_kmod_bo *bo, int64_t timeout_ns,
                   bool for_read_only_access);

   /* Make a buffer object evictable. This method is optional. */
   void (*bo_make_evictable)(struct pan_kmod_bo *bo);

   /* Make the buffer object unevictable. This method is optional. */
   bool (*bo_make_unevictable)(struct pan_kmod_bo *bo);

   /* Create a VM object. */
   struct pan_kmod_vm *(*vm_create)(struct pan_kmod_dev *dev, uint32_t flags,
                                    uint64_t va_start, uint64_t va_range);

   /* Destroy a VM object. */
   void (*vm_destroy)(struct pan_kmod_vm *vm);

   /* Execute VM operations.
    * Return 0 if the submission suceeds, -1 otherwise.
    * For PAN_KMOD_VM_OP_MODE_IMMEDIATE submissions, the return value also
    * reflects the successfulness of the VM operation, for other modes,
    * if any of the VM operation fails, the VM might be flagged as unusable
    * and users should create a new VM to recover.
    */
   int (*vm_bind)(struct pan_kmod_vm *vm, enum pan_kmod_vm_op_mode mode,
                  struct pan_kmod_vm_op *ops, uint32_t op_count);

   /* Query the VM state.
    * This method is optional. When missing the VM is assumed to always be
    * usable.
    */
   enum pan_kmod_vm_state (*vm_query_state)(struct pan_kmod_vm *vm);
};

/* KMD information. */
struct pan_kmod_driver {
   /* KMD version. */
   struct {
      uint32_t major;
      uint32_t minor;
   } version;
};

/* Device object. */
struct pan_kmod_dev {
   /* FD attached to the device. */
   int fd;

   /* Device flags. */
   uint32_t flags;

   /* KMD backing this device. */
   struct pan_kmod_driver driver;

   /* kmod backend ops assigned at device creation. */
   const struct pan_kmod_ops *ops;

   /* DRM prime import returns the handle of a pre-existing GEM if we are
    * importing an object that was created by us or previously imported.
    * We need to make sure we return the same pan_kmod_bo in that case,
    * otherwise freeing one pan_kmod_bo will make all other BOs sharing
    * the same handle invalid.
    */
   struct {
      struct util_sparse_array array;
      simple_mtx_t lock;
   } handle_to_bo;

   /* Allocator attached to the device. */
   const struct pan_kmod_allocator *allocator;

   /* User private data. Use pan_kmod_dev_{set,get}_user_priv() to access it. */
   void *user_priv;
};

struct pan_kmod_dev *
pan_kmod_dev_create(int fd, uint32_t flags,
                    const struct pan_kmod_allocator *allocator);

void pan_kmod_dev_destroy(struct pan_kmod_dev *dev);

static inline void
pan_kmod_dev_query_props(const struct pan_kmod_dev *dev,
                         struct pan_kmod_dev_props *props)
{
   dev->ops->dev_query_props(dev, props);
}

static inline struct pan_kmod_va_range
pan_kmod_dev_query_user_va_range(const struct pan_kmod_dev *dev)
{
   if (dev->ops->dev_query_user_va_range)
      return dev->ops->dev_query_user_va_range(dev);

   struct pan_kmod_dev_props props;

   pan_kmod_dev_query_props(dev, &props);
   return (struct pan_kmod_va_range){
      .start = 0,
      .size = 1ull << MMU_FEATURES_VA_BITS(props.mmu_features),
   };
}

static inline void
pan_kmod_dev_set_user_priv(struct pan_kmod_dev *dev, void *data)
{
   dev->user_priv = data;
}

static inline void *
pan_kmod_dev_get_user_priv(struct pan_kmod_dev *dev)
{
   return dev->user_priv;
}

struct pan_kmod_bo *pan_kmod_bo_alloc(struct pan_kmod_dev *dev,
                                      struct pan_kmod_vm *exclusive_vm,
                                      size_t size, uint32_t flags);

static inline struct pan_kmod_bo *
pan_kmod_bo_get(struct pan_kmod_bo *bo)
{
   if (!bo)
      return NULL;

   ASSERTED int32_t refcnt = p_atomic_inc_return(&bo->refcnt);

   /* If refcnt was zero before our increment, we're in trouble. */
   assert(refcnt > 1);

   return bo;
}

void pan_kmod_bo_put(struct pan_kmod_bo *bo);

static inline void *
pan_kmod_bo_cmdxchg_user_priv(struct pan_kmod_bo *bo, void *old_data,
                              void *new_data)
{
   return (void *)p_atomic_cmpxchg((uintptr_t *)&bo->user_priv,
                                   (uintptr_t)old_data, (uintptr_t)new_data);
}

static inline void
pan_kmod_bo_set_user_priv(struct pan_kmod_bo *bo, void *data)
{
   bo->user_priv = data;
}

static inline void *
pan_kmod_bo_get_user_priv(const struct pan_kmod_bo *bo)
{
   return bo->user_priv;
}

struct pan_kmod_bo *pan_kmod_bo_import(struct pan_kmod_dev *dev, int fd,
                                       uint32_t flags);

static inline int
pan_kmod_bo_export(struct pan_kmod_bo *bo)
{
   int fd;

   if (drmPrimeHandleToFD(bo->dev->fd, bo->handle, DRM_CLOEXEC, &fd)) {
      mesa_loge("drmPrimeHandleToFD() failed (err=%d)", errno);
      return -1;
   }

   if (bo->dev->ops->bo_export && bo->dev->ops->bo_export(bo, fd)) {
      close(fd);
      return -1;
   }

   bo->flags |= PAN_KMOD_BO_FLAG_EXPORTED;
   return fd;
}

static inline bool
pan_kmod_bo_wait(struct pan_kmod_bo *bo, int64_t timeout_ns,
                 bool for_read_only_access)
{
   return bo->dev->ops->bo_wait(bo, timeout_ns, for_read_only_access);
}

static inline void
pan_kmod_bo_make_evictable(struct pan_kmod_bo *bo)
{
   if (bo->dev->ops->bo_make_evictable)
      bo->dev->ops->bo_make_evictable(bo);
}

static inline bool
pan_kmod_bo_make_unevictable(struct pan_kmod_bo *bo)
{
   if (bo->dev->ops->bo_make_unevictable)
      return bo->dev->ops->bo_make_unevictable(bo);

   return true;
}

static inline void *
pan_kmod_bo_mmap(struct pan_kmod_bo *bo, off_t bo_offset, size_t size, int prot,
                 int flags, void *host_addr)
{
   off_t mmap_offset;

   if (bo_offset + size > bo->size)
      return MAP_FAILED;

   mmap_offset = bo->dev->ops->bo_get_mmap_offset(bo);
   if (mmap_offset < 0)
      return MAP_FAILED;

   host_addr = os_mmap(host_addr, size, prot, flags, bo->dev->fd,
                       mmap_offset + bo_offset);
   if (host_addr == MAP_FAILED)
      mesa_loge("mmap() failed (err=%d)", errno);

   return host_addr;
}

static inline size_t
pan_kmod_bo_size(struct pan_kmod_bo *bo)
{
   return bo->size;
}

static inline uint32_t
pan_kmod_bo_handle(struct pan_kmod_bo *bo)
{
   return bo->handle;
}

static inline struct pan_kmod_vm *
pan_kmod_vm_create(struct pan_kmod_dev *dev, uint32_t flags, uint64_t va_start,
                   uint64_t va_range)
{
   return dev->ops->vm_create(dev, flags, va_start, va_range);
}

static inline void
pan_kmod_vm_destroy(struct pan_kmod_vm *vm)
{
   vm->dev->ops->vm_destroy(vm);
}

static inline int
pan_kmod_vm_bind(struct pan_kmod_vm *vm, enum pan_kmod_vm_op_mode mode,
                 struct pan_kmod_vm_op *ops, uint32_t op_count)
{
   return vm->dev->ops->vm_bind(vm, mode, ops, op_count);
}

static inline enum pan_kmod_vm_state
pan_kmod_vm_query_state(struct pan_kmod_vm *vm)
{
   if (vm->dev->ops->vm_query_state)
      return vm->dev->ops->vm_query_state(vm);

   return PAN_KMOD_VM_USABLE;
}

static inline uint32_t
pan_kmod_vm_handle(struct pan_kmod_vm *vm)
{
   return vm->handle;
}

#if defined(__cplusplus)
} // extern "C"
#endif
