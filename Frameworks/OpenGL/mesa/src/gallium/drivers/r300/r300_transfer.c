/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "r300_transfer.h"
#include "r300_texture_desc.h"
#include "r300_screen_buffer.h"

#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "util/u_box.h"

struct r300_transfer {
    /* Parent class */
    struct pipe_transfer transfer;

    /* Linear texture. */
    struct r300_resource *linear_texture;
};

/* Convenience cast wrapper. */
static inline struct r300_transfer*
r300_transfer(struct pipe_transfer* transfer)
{
    return (struct r300_transfer*)transfer;
}

/* Copy from a tiled texture to a detiled one. */
static void r300_copy_from_tiled_texture(struct pipe_context *ctx,
                                         struct r300_transfer *r300transfer)
{
    struct pipe_transfer *transfer = (struct pipe_transfer*)r300transfer;
    struct pipe_resource *src = transfer->resource;
    struct pipe_resource *dst = &r300transfer->linear_texture->b;

    if (src->nr_samples <= 1) {
        ctx->resource_copy_region(ctx, dst, 0, 0, 0, 0,
                                  src, transfer->level, &transfer->box);
    } else {
        /* Resolve the resource. */
        struct pipe_blit_info blit;

        memset(&blit, 0, sizeof(blit));
        blit.src.resource = src;
        blit.src.format = src->format;
        blit.src.level = transfer->level;
        blit.src.box = transfer->box;
        blit.dst.resource = dst;
        blit.dst.format = dst->format;
        blit.dst.box.width = transfer->box.width;
        blit.dst.box.height = transfer->box.height;
        blit.dst.box.depth = transfer->box.depth;
        blit.mask = PIPE_MASK_RGBA;
        blit.filter = PIPE_TEX_FILTER_NEAREST;

        ctx->blit(ctx, &blit);
    }
}

/* Copy a detiled texture to a tiled one. */
static void r300_copy_into_tiled_texture(struct pipe_context *ctx,
                                         struct r300_transfer *r300transfer)
{
    struct pipe_transfer *transfer = (struct pipe_transfer*)r300transfer;
    struct pipe_resource *tex = transfer->resource;
    struct pipe_box src_box;

    u_box_3d(0, 0, 0,
             transfer->box.width, transfer->box.height, transfer->box.depth,
             &src_box);

    ctx->resource_copy_region(ctx, tex, transfer->level,
                              transfer->box.x, transfer->box.y, transfer->box.z,
                              &r300transfer->linear_texture->b, 0, &src_box);

    /* XXX remove this. */
    r300_flush(ctx, 0, NULL);
}

void *
r300_texture_transfer_map(struct pipe_context *ctx,
                          struct pipe_resource *texture,
                          unsigned level,
                          unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **transfer)
{
    struct r300_context *r300 = r300_context(ctx);
    struct r300_resource *tex = r300_resource(texture);
    struct r300_transfer *trans;
    bool referenced_cs, referenced_hw;
    enum pipe_format format = tex->b.format;
    char *map;

    referenced_cs =
        r300->rws->cs_is_buffer_referenced(&r300->cs, tex->buf, RADEON_USAGE_READWRITE);
    if (referenced_cs) {
        referenced_hw = true;
    } else {
        referenced_hw =
            !r300->rws->buffer_wait(r300->rws, tex->buf, 0, RADEON_USAGE_READWRITE);
    }

    trans = CALLOC_STRUCT(r300_transfer);
    if (trans) {
        /* Initialize the transfer object. */
        trans->transfer.resource = texture;
        trans->transfer.level = level;
        trans->transfer.usage = usage;
        trans->transfer.box = *box;

        /* If the texture is tiled, we must create a temporary detiled texture
         * for this transfer.
         * Also make write transfers pipelined. */
        if (tex->tex.microtile || tex->tex.macrotile[level] ||
            (referenced_hw && !(usage & PIPE_MAP_READ) &&
             r300_is_blit_supported(texture->format))) {
            struct pipe_resource base;

            if (r300->blitter->running) {
                fprintf(stderr, "r300: ERROR: Blitter recursion in texture_get_transfer.\n");
                os_break();
            }

            memset(&base, 0, sizeof(base));
            base.target = PIPE_TEXTURE_2D;
            base.format = texture->format;
            base.width0 = box->width;
            base.height0 = box->height;
            base.depth0 = 1;
            base.array_size = 1;
            base.usage = PIPE_USAGE_STAGING;
            base.flags = R300_RESOURCE_FLAG_TRANSFER;

            /* We must set the correct texture target and dimensions if needed for a 3D transfer. */
            if (box->depth > 1 && util_max_layer(texture, level) > 0) {
                base.target = texture->target;

                if (base.target == PIPE_TEXTURE_3D) {
                    base.depth0 = util_next_power_of_two(box->depth);
                }
            }

            /* Create the temporary texture. */
            trans->linear_texture = r300_resource(
               ctx->screen->resource_create(ctx->screen,
                                            &base));

            if (!trans->linear_texture) {
                /* Oh crap, the thing can't create the texture.
                 * Let's flush and try again. */
                r300_flush(ctx, 0, NULL);

                trans->linear_texture = r300_resource(
                   ctx->screen->resource_create(ctx->screen,
                                                &base));

                if (!trans->linear_texture) {
                    fprintf(stderr,
                            "r300: Failed to create a transfer object.\n");
                    FREE(trans);
                    return NULL;
                }
            }

            assert(!trans->linear_texture->tex.microtile &&
                   !trans->linear_texture->tex.macrotile[0]);

            /* Set the stride. */
            trans->transfer.stride =
                    trans->linear_texture->tex.stride_in_bytes[0];
            trans->transfer.layer_stride =
                    trans->linear_texture->tex.layer_size_in_bytes[0];

            if (usage & PIPE_MAP_READ) {
                /* We cannot map a tiled texture directly because the data is
                 * in a different order, therefore we do detiling using a blit. */
                r300_copy_from_tiled_texture(ctx, trans);

                /* Always referenced in the blit. */
                r300_flush(ctx, 0, NULL);
            }
        } else {
            /* Unpipelined transfer. */
            trans->transfer.stride = tex->tex.stride_in_bytes[level];
            trans->transfer.layer_stride = tex->tex.layer_size_in_bytes[level];
            trans->transfer.offset = r300_texture_get_offset(tex, level, box->z);

            if (referenced_cs &&
                !(usage & PIPE_MAP_UNSYNCHRONIZED)) {
                r300_flush(ctx, 0, NULL);
            }
        }
    }

    if (trans->linear_texture) {
        /* The detiled texture is of the same size as the region being mapped
         * (no offset needed). */
        map = r300->rws->buffer_map(r300->rws, trans->linear_texture->buf,
                                    &r300->cs, usage);
        if (!map) {
            pipe_resource_reference(
                (struct pipe_resource**)&trans->linear_texture, NULL);
            FREE(trans);
            return NULL;
        }
	*transfer = &trans->transfer;
        return map;
    } else {
        /* Tiling is disabled. */
        map = r300->rws->buffer_map(r300->rws, tex->buf, &r300->cs, usage);
        if (!map) {
            FREE(trans);
            return NULL;
        }

	*transfer = &trans->transfer;
        return map + trans->transfer.offset +
            box->y / util_format_get_blockheight(format) * trans->transfer.stride +
            box->x / util_format_get_blockwidth(format) * util_format_get_blocksize(format);
    }
}

void r300_texture_transfer_unmap(struct pipe_context *ctx,
				 struct pipe_transfer *transfer)
{
    struct r300_transfer *trans = r300_transfer(transfer);

    if (trans->linear_texture) {
        if (transfer->usage & PIPE_MAP_WRITE) {
            r300_copy_into_tiled_texture(ctx, trans);
        }

        pipe_resource_reference(
            (struct pipe_resource**)&trans->linear_texture, NULL);
    }
    FREE(transfer);
}
