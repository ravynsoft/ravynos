/*
 * Copyright (C) 2014 Etnaviv Project
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
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "util/os_mman.h"
#include "util/hash_table.h"

#include "etnaviv_priv.h"
#include "etnaviv_drmif.h"

simple_mtx_t etna_device_lock = SIMPLE_MTX_INITIALIZER;

/* set buffer name, and add to table, call w/ etna_drm_table_lock held: */
static void set_name(struct etna_bo *bo, uint32_t name)
{
	simple_mtx_assert_locked(&etna_device_lock);

	bo->name = name;
	/* add ourself into the name table: */
	_mesa_hash_table_insert(bo->dev->name_table, &bo->name, bo);
}

int etna_bo_is_idle(struct etna_bo *bo)
{
	return etna_bo_cpu_prep(bo,
			DRM_ETNA_PREP_READ |
			DRM_ETNA_PREP_WRITE |
			DRM_ETNA_PREP_NOSYNC) == 0;
}

/* Called under etna_drm_table_lock */
static void _etna_bo_free(struct etna_bo *bo)
{
	DEBUG_BO("Del bo:", bo);
	VG_BO_FREE(bo);

	simple_mtx_assert_locked(&etna_device_lock);

	if (bo->va)
		util_vma_heap_free(&bo->dev->address_space, bo->va, bo->size);

	if (bo->map)
		os_munmap(bo->map, bo->size);

	if (bo->handle) {
		struct drm_gem_close req = {
			.handle = bo->handle,
		};

		if (bo->name)
			_mesa_hash_table_remove_key(bo->dev->name_table, &bo->name);

		_mesa_hash_table_remove_key(bo->dev->handle_table, &bo->handle);
		drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
	}

	free(bo);
}

void etna_bo_kill_zombies(struct etna_device *dev)
{
	simple_mtx_assert_locked(&etna_device_lock);

	list_for_each_entry_safe(struct etna_bo, bo, &dev->zombie_list, list) {
		VG_BO_OBTAIN(bo);
		list_del(&bo->list);
		_etna_bo_free(bo);
	}
}


static void etna_bo_cleanup_zombies(struct etna_device *dev)
{
	simple_mtx_assert_locked(&etna_device_lock);

	list_for_each_entry_safe(struct etna_bo, bo, &dev->zombie_list, list) {
		/* Stop once we reach a busy BO - all others past this point were
		 * freed more recently so are likely also busy.
		 */
		if (!etna_bo_is_idle(bo))
			break;

		VG_BO_OBTAIN(bo);
		list_del(&bo->list);
		_etna_bo_free(bo);
	}
}

void etna_bo_free(struct etna_bo *bo) {
	struct etna_device *dev = bo->dev;

	/* If the BO has a userspace managed address we don't free it immediately,
	 * but keep it on a deferred destroy list until all submits with the buffer
	 * have finished, at which point we can reuse the VMA space.
	 */
	if (dev->use_softpin) {
		etna_bo_cleanup_zombies(dev);
		VG_BO_RELEASE(bo);
		list_addtail(&bo->list, &dev->zombie_list);
	} else {
		_etna_bo_free(bo);
	}
}

/* lookup a buffer from it's handle, call w/ etna_drm_table_lock held: */
static struct etna_bo *lookup_bo(void *tbl, uint32_t handle)
{
	struct etna_bo *bo = NULL;
	struct hash_entry *entry;

	simple_mtx_assert_locked(&etna_device_lock);

	entry = _mesa_hash_table_search(tbl, &handle);

	if (entry) {
		/* found, incr refcnt and return: */
		bo = etna_bo_ref(entry->data);

		/* don't break the bucket/zombie list if this bo was found in one */
		if (!list_is_empty(&bo->list)) {
			VG_BO_OBTAIN(bo);
			etna_device_ref(bo->dev);
			list_delinit(&bo->list);
		}
	}

	return bo;
}

/* allocate a new buffer object, call w/ etna_drm_table_lock held */
static struct etna_bo *bo_from_handle(struct etna_device *dev,
		uint32_t size, uint32_t handle, uint32_t flags)
{
	struct etna_bo *bo = calloc(sizeof(*bo), 1);

	simple_mtx_assert_locked(&etna_device_lock);

	if (!bo) {
		struct drm_gem_close req = {
			.handle = handle,
		};

		drmIoctl(dev->fd, DRM_IOCTL_GEM_CLOSE, &req);

		return NULL;
	}

	bo->dev = etna_device_ref(dev);
	bo->size = size;
	bo->handle = handle;
	bo->flags = flags;
	p_atomic_set(&bo->refcnt, 1);
	list_inithead(&bo->list);
	/* add ourselves to the handle table: */
	_mesa_hash_table_insert(dev->handle_table, &bo->handle, bo);

	if (dev->use_softpin)
		bo->va = util_vma_heap_alloc(&dev->address_space, bo->size, 4096);

	return bo;
}

/* allocate a new (un-tiled) buffer object */
struct etna_bo *etna_bo_new(struct etna_device *dev, uint32_t size,
		uint32_t flags)
{
	struct etna_bo *bo;
	int ret;
	struct drm_etnaviv_gem_new req = {
			.flags = flags,
	};

	bo = etna_bo_cache_alloc(&dev->bo_cache, &size, flags);
	if (bo)
		return bo;

	req.size = size;
	ret = drmCommandWriteRead(dev->fd, DRM_ETNAVIV_GEM_NEW,
			&req, sizeof(req));
	if (ret)
		return NULL;

	simple_mtx_lock(&etna_device_lock);
	bo = bo_from_handle(dev, size, req.handle, flags);
	bo->reuse = 1;
	simple_mtx_unlock(&etna_device_lock);

	DEBUG_BO("New bo:", bo);
	VG_BO_ALLOC(bo);

	return bo;
}

struct etna_bo *etna_bo_ref(struct etna_bo *bo)
{
	p_atomic_inc(&bo->refcnt);

	return bo;
}

/* import a buffer object from DRI2 name */
struct etna_bo *etna_bo_from_name(struct etna_device *dev,
		uint32_t name)
{
	struct etna_bo *bo;
	struct drm_gem_open req = {
		.name = name,
	};

	simple_mtx_lock(&etna_device_lock);

	/* check name table first, to see if bo is already open: */
	bo = lookup_bo(dev->name_table, name);
	if (bo)
		goto out_unlock;

	if (drmIoctl(dev->fd, DRM_IOCTL_GEM_OPEN, &req)) {
		ERROR_MSG("gem-open failed: %s", strerror(errno));
		goto out_unlock;
	}

	bo = lookup_bo(dev->handle_table, req.handle);
	if (bo)
		goto out_unlock;

	bo = bo_from_handle(dev, req.size, req.handle, 0);
	if (bo) {
		set_name(bo, name);
		DEBUG_BO("New from name:", bo);
		VG_BO_ALLOC(bo);
	}

out_unlock:
	simple_mtx_unlock(&etna_device_lock);

	return bo;
}

/* import a buffer from dmabuf fd, does not take ownership of the
 * fd so caller should close() the fd when it is otherwise done
 * with it (even if it is still using the 'struct etna_bo *')
 */
struct etna_bo *etna_bo_from_dmabuf(struct etna_device *dev, int fd)
{
	struct etna_bo *bo;
	int ret, size;
	uint32_t handle;

	/* take the lock before calling drmPrimeFDToHandle to avoid
	 * racing against etna_bo_del, which might invalidate the
	 * returned handle.
	 */
	simple_mtx_lock(&etna_device_lock);

	ret = drmPrimeFDToHandle(dev->fd, fd, &handle);
	if (ret) {
		simple_mtx_unlock(&etna_device_lock);
		return NULL;
	}

	bo = lookup_bo(dev->handle_table, handle);
	if (bo)
		goto out_unlock;

	/* lseek() to get bo size */
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_CUR);

	bo = bo_from_handle(dev, size, handle, 0);

	DEBUG_BO("New from dmabuf:", bo);
	VG_BO_ALLOC(bo);

out_unlock:
	simple_mtx_unlock(&etna_device_lock);

	return bo;
}

/* destroy a buffer object */
void etna_bo_del(struct etna_bo *bo)
{
	if (!bo)
		return;

	struct etna_device *dev = bo->dev;

	simple_mtx_lock(&etna_device_lock);

	/* Must test under table lock to avoid racing with the from_dmabuf/name
	 * paths, which rely on the BO refcount to be stable over the lookup, so
	 * they can grab a reference when the BO is found in the hash.
	 */
	if (!p_atomic_dec_zero(&bo->refcnt))
	   goto out;

	if (bo->reuse && (etna_bo_cache_free(&dev->bo_cache, bo) == 0))
		goto out;

	etna_bo_free(bo);
	etna_device_del_locked(dev);
out:
	simple_mtx_unlock(&etna_device_lock);
}

/* get the global flink/DRI2 buffer name */
int etna_bo_get_name(struct etna_bo *bo, uint32_t *name)
{
	if (!bo->name) {
		struct drm_gem_flink req = {
			.handle = bo->handle,
		};
		int ret;

		ret = drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_FLINK, &req);
		if (ret) {
			return ret;
		}

		simple_mtx_lock(&etna_device_lock);
		set_name(bo, req.name);
		simple_mtx_unlock(&etna_device_lock);
		bo->reuse = 0;
	}

	*name = bo->name;

	return 0;
}

uint32_t etna_bo_handle(struct etna_bo *bo)
{
	return bo->handle;
}

/* caller owns the dmabuf fd that is returned and is responsible
 * to close() it when done
 */
int etna_bo_dmabuf(struct etna_bo *bo)
{
	int ret, prime_fd;

	ret = drmPrimeHandleToFD(bo->dev->fd, bo->handle, DRM_CLOEXEC,
				&prime_fd);
	if (ret) {
		ERROR_MSG("failed to get dmabuf fd: %d", ret);
		return ret;
	}

	bo->reuse = 0;

	return prime_fd;
}

uint32_t etna_bo_size(struct etna_bo *bo)
{
	return bo->size;
}

uint32_t etna_bo_gpu_va(struct etna_bo *bo)
{
	return bo->va;
}

void *etna_bo_map(struct etna_bo *bo)
{
	if (!bo->map) {
		int ret;
		void *map;
		struct drm_etnaviv_gem_info req = {
			.handle = bo->handle,
		};

		ret = drmCommandWriteRead(bo->dev->fd, DRM_ETNAVIV_GEM_INFO,
					&req, sizeof(req));
		if (ret)
			return NULL;

		map = os_mmap(0, bo->size, PROT_READ | PROT_WRITE,
				  MAP_SHARED, bo->dev->fd, req.offset);
		if (map == MAP_FAILED) {
			ERROR_MSG("mmap failed: %s", strerror(errno));
			return NULL;
		}

		if (p_atomic_cmpxchg(&bo->map, NULL, map))
			munmap(map, bo->size);
	}

	return bo->map;
}

int etna_bo_cpu_prep(struct etna_bo *bo, uint32_t op)
{
	struct drm_etnaviv_gem_cpu_prep req = {
		.handle = bo->handle,
		.op = op,
	};

	get_abs_timeout(&req.timeout, 5000000000);

	return drmCommandWrite(bo->dev->fd, DRM_ETNAVIV_GEM_CPU_PREP,
			&req, sizeof(req));
}

void etna_bo_cpu_fini(struct etna_bo *bo)
{
	struct drm_etnaviv_gem_cpu_fini req = {
		.handle = bo->handle,
	};

	drmCommandWrite(bo->dev->fd, DRM_ETNAVIV_GEM_CPU_FINI,
			&req, sizeof(req));
}
