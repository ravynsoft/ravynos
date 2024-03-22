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

#include "util/hash_table.h"
#include "util/os_file.h"

#include "etnaviv_priv.h"
#include "etnaviv_drmif.h"

/* Declare symbol in case we don't link with etnaviv's gallium driver */
int etna_mesa_debug __attribute__((weak)) = 0;

struct etna_device *etna_device_new(int fd)
{
	struct etna_device *dev;
	struct drm_etnaviv_param req = {
		.param = ETNAVIV_PARAM_SOFTPIN_START_ADDR,
	};
	drmVersionPtr version;
	int ret;

	version = drmGetVersion(fd);
	if (!version) {
		ERROR_MSG("cannot get version: %s", strerror(errno));
		return NULL;
	}

	dev = calloc(sizeof(*dev), 1);
	if (!dev) {
		goto out;
	}

	dev->drm_version = ETNA_DRM_VERSION(version->version_major,
					    version->version_minor);

out:
	drmFreeVersion(version);

	if (!dev)
		return NULL;

	p_atomic_set(&dev->refcnt, 1);
	dev->fd = fd;
	dev->handle_table = _mesa_hash_table_create(NULL, _mesa_hash_u32, _mesa_key_u32_equal);
	dev->name_table = _mesa_hash_table_create(NULL, _mesa_hash_u32, _mesa_key_u32_equal);
	etna_bo_cache_init(&dev->bo_cache);

	ret = drmCommandWriteRead(dev->fd, DRM_ETNAVIV_GET_PARAM, &req, sizeof(req));
	if (!ret && req.value != ~0ULL) {
		const uint64_t _4GB = 1ull << 32;

		list_inithead(&dev->zombie_list);
		util_vma_heap_init(&dev->address_space, req.value, _4GB - req.value);
		dev->use_softpin = 1;
	}

	return dev;
}

/* like etna_device_new() but creates it's own private dup() of the fd
 * which is close()d when the device is finalized. */
struct etna_device *etna_device_new_dup(int fd)
{
	int dup_fd = os_dupfd_cloexec(fd);
	struct etna_device *dev = etna_device_new(dup_fd);

	if (dev)
		dev->closefd = 1;
	else
		close(dup_fd);

	return dev;
}

struct etna_device *etna_device_ref(struct etna_device *dev)
{
	p_atomic_inc(&dev->refcnt);

	return dev;
}

static void etna_device_del_impl(struct etna_device *dev)
{
	etna_bo_cache_cleanup(&dev->bo_cache, 0);

	if (dev->use_softpin) {
		etna_bo_kill_zombies(dev);
		util_vma_heap_finish(&dev->address_space);
	}

	_mesa_hash_table_destroy(dev->handle_table, NULL);
	_mesa_hash_table_destroy(dev->name_table, NULL);

	if (dev->closefd)
		close(dev->fd);

	free(dev);
}

void etna_device_del_locked(struct etna_device *dev)
{
	simple_mtx_assert_locked(&etna_device_lock);

	if (!p_atomic_dec_zero(&dev->refcnt))
		return;

	etna_device_del_impl(dev);
}

void etna_device_del(struct etna_device *dev)
{
	if (!p_atomic_dec_zero(&dev->refcnt))
		return;

	simple_mtx_lock(&etna_device_lock);
	etna_device_del_impl(dev);
	simple_mtx_unlock(&etna_device_lock);
}

int etna_device_fd(struct etna_device *dev)
{
   return dev->fd;
}

bool etnaviv_device_softpin_capable(struct etna_device *dev)
{
	return !!dev->use_softpin;
}

uint32_t etnaviv_device_version(struct etna_device *dev)
{
   return dev->drm_version;
}
