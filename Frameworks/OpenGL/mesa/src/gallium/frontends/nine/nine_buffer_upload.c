/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * Copyright 2016 Axel Davy <axel.davy@ens.fr>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/* Adapted from u_upload_mgr.
 * Makes suballocations from bigger allocations,
 * while enabling fast mapping. */

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/slab.h"

#include "nine_buffer_upload.h"

#include "nine_debug.h"

#define DBG_CHANNEL (DBG_INDEXBUFFER|DBG_VERTEXBUFFER)

struct nine_buffer_group {
    unsigned refcount; /* How many sub-buffers live inside the buffer */
    struct pipe_resource *resource;
    struct pipe_transfer *transfer;
    uint8_t *map;
    unsigned free_offset; /* Aligned offset to the upload buffer, pointing
                           * at the first unused byte. */
};

struct nine_subbuffer {
    struct nine_buffer_group *parent; /* Can be NULL */
    struct pipe_resource *resource; /* The parent resource if apply */
    unsigned offset; /* Offset inside the resource */
    /* If there is no parent, the resource map. Else NULL. */
    struct pipe_transfer *transfer;
    uint8_t *map;
};

struct nine_buffer_upload {
    struct pipe_context *pipe;
    struct slab_mempool buffer_pool;

    unsigned buffers_size; /* Size of the big allocated buffers */
    unsigned num_buffers;
    struct nine_buffer_group *buffers;
};

static void
nine_upload_create_buffer_group(struct nine_buffer_upload *upload,
                                struct nine_buffer_group *group)
{
    struct pipe_resource resource;
    struct pipe_screen *screen = upload->pipe->screen;
    DBG("Allocating %p %p\n", upload, group);

    memset(&resource, 0, sizeof(resource));
    resource.target = PIPE_BUFFER;
    resource.format = PIPE_FORMAT_R8_UNORM;
    resource.bind = PIPE_BIND_VERTEX_BUFFER;
    resource.usage = PIPE_USAGE_STREAM;
    resource.width0 = upload->buffers_size;
    resource.height0 = 1;
    resource.depth0 = 1;
    resource.array_size = 1;
    resource.flags = PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                     PIPE_RESOURCE_FLAG_MAP_COHERENT;

    group->refcount = 0;
    group->resource = screen->resource_create(screen, &resource);
    if (group->resource == NULL)
        return;

    group->map = pipe_buffer_map_range(upload->pipe, group->resource,
                                       0, upload->buffers_size,
                                       PIPE_MAP_WRITE |
#if DETECT_ARCH_X86
                                       PIPE_MAP_ONCE |
#endif
                                       PIPE_MAP_PERSISTENT |
                                       PIPE_MAP_COHERENT,
                                       &group->transfer);
    if (group->map == NULL) {
        group->transfer = NULL;
        pipe_resource_reference(&group->resource, NULL);
        return;
    }

    group->free_offset = 0;
    DBG("Success: %p %p\n", group->map, group->map+upload->buffers_size);
}

static void
nine_upload_destroy_buffer_group(struct nine_buffer_upload *upload,
                                 struct nine_buffer_group *group)
{
    DBG("%p %p\n", upload, group);
    DBG("Release: %p %p\n", group->map, group->map+upload->buffers_size);
    assert(group->refcount == 0);

    if (group->transfer)
        pipe_buffer_unmap(upload->pipe, group->transfer);
    if (group->resource)
        pipe_resource_reference(&group->resource, NULL);
    group->transfer = NULL;
    group->map = NULL;
}

struct nine_buffer_upload *
nine_upload_create(struct pipe_context *pipe, unsigned buffers_size,
                   unsigned num_buffers)
{
    struct nine_buffer_upload *upload;
    int i;

    DBG("\n");

    if (!pipe->screen->get_param(pipe->screen,
                                 PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT))
        return NULL;

    upload = CALLOC_STRUCT(nine_buffer_upload);

    if (!upload)
        return NULL;

    slab_create(&upload->buffer_pool, sizeof(struct nine_subbuffer), 4096);

    upload->pipe = pipe;
    upload->buffers_size = align(buffers_size, 4096);
    upload->num_buffers = num_buffers;

    upload->buffers = CALLOC(num_buffers, sizeof(struct nine_buffer_group));
    if (!upload->buffers)
        goto buffers_fail;

    for (i = 0; i < num_buffers; i++)
        nine_upload_create_buffer_group(upload, &upload->buffers[i]);

    return upload;

buffers_fail:
    slab_destroy(&upload->buffer_pool);
    FREE(upload);
    return NULL;
}

void
nine_upload_destroy(struct nine_buffer_upload *upload)
{
    int i;

    DBG("%p\n", upload);

    for (i = 0; i < upload->num_buffers; i++)
        nine_upload_destroy_buffer_group(upload, &upload->buffers[i]);
    slab_destroy(&upload->buffer_pool);
    FREE(upload);
}

struct nine_subbuffer *
nine_upload_create_buffer(struct nine_buffer_upload *upload,
                          unsigned buffer_size)
{
    struct nine_subbuffer *buf = slab_alloc_st(&upload->buffer_pool);
    struct nine_buffer_group *group = NULL;
    unsigned size = align(buffer_size, 4096);
    int i = 0;

    DBG("%p %d\n", upload, buffer_size);

    if (!buf)
        return NULL;

    for (i = 0; i < upload->num_buffers; i++) {
        group = &upload->buffers[i];
        if (group->resource &&
            group->free_offset + size <= upload->buffers_size)
            break;
    }

    if (i == upload->num_buffers) {
        /* Allocate lonely buffer */
        struct pipe_resource resource;
        struct pipe_screen *screen = upload->pipe->screen;

        DBG("Allocating buffer\n");
        buf->parent = NULL;

        memset(&resource, 0, sizeof(resource));
        resource.target = PIPE_BUFFER;
        resource.format = PIPE_FORMAT_R8_UNORM;
        resource.bind = PIPE_BIND_VERTEX_BUFFER;
        resource.usage = PIPE_USAGE_STREAM;
        resource.width0 = buffer_size;
        resource.height0 = 1;
        resource.depth0 = 1;
        resource.array_size = 1;
        resource.flags = PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                         PIPE_RESOURCE_FLAG_MAP_COHERENT;

        buf->resource = screen->resource_create(screen, &resource);
        if (buf->resource == NULL) {
            slab_free_st(&upload->buffer_pool, buf);
            return NULL;
        }

        buf->map = pipe_buffer_map_range(upload->pipe, buf->resource,
                                         0, buffer_size,
                                         PIPE_MAP_WRITE |
#if DETECT_ARCH_X86
                                         PIPE_MAP_ONCE |
#endif
                                         PIPE_MAP_PERSISTENT |
                                         PIPE_MAP_COHERENT,
                                         &buf->transfer);
        if (buf->map == NULL) {
            pipe_resource_reference(&buf->resource, NULL);
            slab_free_st(&upload->buffer_pool, buf);
            return NULL;
        }
        buf->offset = 0;
        return buf;
    }

    DBG("Using buffer group %d\n", i);

    buf->parent = group;
    buf->resource = NULL;
    pipe_resource_reference(&buf->resource, group->resource);
    buf->offset = group->free_offset;

    group->free_offset += size;
    group->refcount += 1;

    return buf;
}

void
nine_upload_release_buffer(struct nine_buffer_upload *upload,
                           struct nine_subbuffer *buf)
{
    DBG("%p %p %p\n", upload, buf, buf->parent);

    if (buf->parent) {
        pipe_resource_reference(&buf->resource, NULL);
        buf->parent->refcount--;
        if (buf->parent->refcount == 0) {
            /* Allocate new buffer */
            nine_upload_destroy_buffer_group(upload, buf->parent);
            nine_upload_create_buffer_group(upload, buf->parent);
        }
    } else {
        /* lonely buffer */
        if (buf->transfer)
            pipe_buffer_unmap(upload->pipe, buf->transfer);
        pipe_resource_reference(&buf->resource, NULL);
    }

    slab_free_st(&upload->buffer_pool, buf);
}

uint8_t *
nine_upload_buffer_get_map(struct nine_subbuffer *buf)
{
    if (buf->parent) {
        DBG("%d\n", buf->parent->refcount);
        return buf->parent->map + buf->offset;
    }
    /* lonely buffer */
    return buf->map;
}

struct pipe_resource *
nine_upload_buffer_resource_and_offset(struct nine_subbuffer *buf,
                                       unsigned *offset)
{
    *offset = buf->offset;
    return buf->resource;
}
