/*
 * Copyright (C) 2022 Lima Project
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef H_LIMA_BLIT
#define H_LIMA_BLIT

#include <stdbool.h>

struct util_dynarray;

void
lima_pack_blit_cmd(struct lima_job *job,
                   struct util_dynarray *cmd,
                   struct pipe_surface *psurf,
                   const struct pipe_box *src,
                   const struct pipe_box *dst,
                   unsigned filter,
                   bool scissor,
                   unsigned sample_mask,
                   unsigned mrt_idx);

bool lima_do_blit(struct pipe_context *ctx,
                  const struct pipe_blit_info *blit_info);

#endif

