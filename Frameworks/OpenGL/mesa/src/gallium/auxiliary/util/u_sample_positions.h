/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef U_SAMPLE_POSITIONS_H
#define U_SAMPLE_POSITIONS_H

#include "pipe/p_context.h"

#ifdef __cplusplus
extern "C" {
#endif

void
u_default_get_sample_position(struct pipe_context *ctx,
                              unsigned sample_count,
                              unsigned sample_index,
                              float *out_value);

#ifdef __cplusplus
}
#endif

#endif
