/*
 * Copyright © 2022 Raspberry Pi Ltd
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

/**
 * V3D on-disk shader cache.
 */

#include "v3d_context.h"

#include "util/blob.h"
#include "util/u_upload_mgr.h"

#ifdef ENABLE_SHADER_CACHE

static uint32_t
v3d_key_size(gl_shader_stage stage)
{
        static const int key_size[] = {
                [MESA_SHADER_VERTEX] = sizeof(struct v3d_vs_key),
                [MESA_SHADER_GEOMETRY] = sizeof(struct v3d_gs_key),
                [MESA_SHADER_FRAGMENT] = sizeof(struct v3d_fs_key),
                [MESA_SHADER_COMPUTE] = sizeof(struct v3d_key),
        };

        assert(stage >= 0 &&
               stage < ARRAY_SIZE(key_size) &&
               key_size[stage]);

        return key_size[stage];
}

void v3d_disk_cache_init(struct v3d_screen *screen)
{
        char *renderer;

        ASSERTED int len =
                asprintf(&renderer, "V3D %d.%d",
                         screen->devinfo.ver / 10,
                         screen->devinfo.ver % 10);
        assert(len > 0);

        const struct build_id_note *note =
                build_id_find_nhdr_for_addr(v3d_disk_cache_init);
        assert(note && build_id_length(note) == 20);

        const uint8_t *id_sha1 = build_id_data(note);
        assert(id_sha1);

        char timestamp[41];
        _mesa_sha1_format(timestamp, id_sha1);

        screen->disk_cache = disk_cache_create(renderer, timestamp, v3d_mesa_debug);

        free(renderer);
}

static void
v3d_disk_cache_compute_key(struct disk_cache *cache,
                           const struct v3d_key *key,
                           cache_key cache_key,
                           const struct v3d_uncompiled_shader *uncompiled)
{
        assert(cache);

        assert(uncompiled->base.type == PIPE_SHADER_IR_NIR);
        nir_shader *nir = uncompiled->base.ir.nir;

        uint32_t ckey_size = v3d_key_size(nir->info.stage);
        struct v3d_key *ckey = malloc(ckey_size);
        memcpy(ckey, key, ckey_size);

        struct blob blob;
        blob_init(&blob);
        blob_write_bytes(&blob, ckey, ckey_size);
        blob_write_bytes(&blob, uncompiled->sha1, 20);

        disk_cache_compute_key(cache, blob.data, blob.size, cache_key);

        blob_finish(&blob);
        free(ckey);
}

struct v3d_compiled_shader *
v3d_disk_cache_retrieve(struct v3d_context *v3d,
                        const struct v3d_key *key,
                        const struct v3d_uncompiled_shader *uncompiled)
{
        struct v3d_screen *screen = v3d->screen;
        struct disk_cache *cache = screen->disk_cache;

        if (!cache)
                return NULL;

        assert(uncompiled->base.type == PIPE_SHADER_IR_NIR);
        nir_shader *nir = uncompiled->base.ir.nir;

        cache_key cache_key;
        v3d_disk_cache_compute_key(cache, key, cache_key, uncompiled);

        size_t buffer_size;
        void *buffer = disk_cache_get(cache, cache_key, &buffer_size);

        if (V3D_DBG(CACHE)) {
                char sha1[41];
                _mesa_sha1_format(sha1, cache_key);
                fprintf(stderr, "[v3d on-disk cache] %s %s\n",
                        buffer ? "hit" : "miss",
                        sha1);
        }

        if (!buffer)
                return NULL;

        /* Load data */
        struct blob_reader blob;
        blob_reader_init(&blob, buffer, buffer_size);

        uint32_t prog_data_size = v3d_prog_data_size(nir->info.stage);
        const void *prog_data = blob_read_bytes(&blob, prog_data_size);
        if (blob.overrun)
                return NULL;

        uint32_t ulist_count = blob_read_uint32(&blob);
        uint32_t ulist_contents_size = ulist_count * sizeof(enum quniform_contents);
        const void *ulist_contents = blob_read_bytes(&blob, ulist_contents_size);
        if (blob.overrun)
                return NULL;

        uint32_t ulist_data_size = ulist_count * sizeof(uint32_t);
        const void *ulist_data = blob_read_bytes(&blob, ulist_data_size);
        if (blob.overrun)
                return NULL;

        uint32_t qpu_size = blob_read_uint32(&blob);
        const void *qpu_insts =
                blob_read_bytes(&blob, qpu_size);
        if (blob.overrun)
                return NULL;

        /* Assemble data */
        struct v3d_compiled_shader *shader = rzalloc(NULL, struct v3d_compiled_shader);

        shader->prog_data.base = rzalloc_size(shader, prog_data_size);
        memcpy(shader->prog_data.base, prog_data, prog_data_size);

        shader->prog_data.base->uniforms.count = ulist_count;

        shader->prog_data.base->uniforms.contents =
                ralloc_array(shader->prog_data.base, enum quniform_contents, ulist_count);
        memcpy(shader->prog_data.base->uniforms.contents, ulist_contents, ulist_contents_size);

        shader->prog_data.base->uniforms.data =
                ralloc_array(shader->prog_data.base, uint32_t, ulist_count);
        memcpy(shader->prog_data.base->uniforms.data, ulist_data, ulist_data_size);

        u_upload_data(v3d->state_uploader, 0, qpu_size, 8,
                      qpu_insts, &shader->offset, &shader->resource);

        free(buffer);

        return shader;
}

void
v3d_disk_cache_store(struct v3d_context *v3d,
                     const struct v3d_key *key,
                     const struct v3d_uncompiled_shader *uncompiled,
                     const struct v3d_compiled_shader *shader,
                     uint64_t *qpu_insts,
                     uint32_t qpu_size)
{
        struct v3d_screen *screen = v3d->screen;
        struct disk_cache *cache = screen->disk_cache;

        if (!cache)
                return;

        assert(uncompiled->base.type == PIPE_SHADER_IR_NIR);
        nir_shader *nir = uncompiled->base.ir.nir;

        cache_key cache_key;
        v3d_disk_cache_compute_key(cache, key, cache_key, uncompiled);

        if (V3D_DBG(CACHE)) {
                char sha1[41];
                _mesa_sha1_format(sha1, cache_key);
                fprintf(stderr, "[v3d on-disk cache] storing %s\n", sha1);
        }

        struct blob blob;
        blob_init(&blob);

        blob_write_bytes(&blob, shader->prog_data.base, v3d_prog_data_size(nir->info.stage));
        uint32_t ulist_count = shader->prog_data.base->uniforms.count;
        blob_write_uint32(&blob, ulist_count);
        blob_write_bytes(&blob,
                         shader->prog_data.base->uniforms.contents,
                         ulist_count * sizeof(enum quniform_contents));
        blob_write_bytes(&blob,
                         shader->prog_data.base->uniforms.data,
                         ulist_count * sizeof(uint32_t));

        blob_write_uint32(&blob, qpu_size);
        blob_write_bytes(&blob, qpu_insts, qpu_size);

        disk_cache_put(cache, cache_key, blob.data, blob.size, NULL);

        blob_finish(&blob);
}

#endif /* ENABLE_SHADER_CACHE */

