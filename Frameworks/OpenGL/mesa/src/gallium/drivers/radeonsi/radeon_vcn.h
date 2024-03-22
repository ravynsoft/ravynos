/*
 * Copyright © 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RADEON_VCN_H
#define RADEON_VCN_H

#include "radeon_video.h"

#include "ac_vcn.h"

void rvcn_sq_header(struct radeon_cmdbuf *cs,
                    struct rvcn_sq_var *sq,
                    bool enc);

void rvcn_sq_tail(struct radeon_cmdbuf *cs,
                  struct rvcn_sq_var *sq);
#endif
