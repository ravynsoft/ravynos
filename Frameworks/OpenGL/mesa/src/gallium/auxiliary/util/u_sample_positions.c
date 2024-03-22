/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "u_sample_positions.h"

/*
 * This file implement a default version of get_sample_position that returns the
 * standard sample positions, as standardized in both Vulkan and Direct3D and
 * hence almost everyone's hardware.
 */

static const float u_default_sample_positions_1x[][2] = {
   {0.5, 0.5}
};

static const float u_default_sample_positions_2x[][2] = {
   {0.75, 0.75},
   {0.25, 0.25}
};

static const float u_default_sample_positions_4x[][4] = {
   {0.375, 0.125},
   {0.875, 0.375},
   {0.125, 0.625},
   {0.625, 0.875},
};

static const float u_default_sample_positions_8x[][4] = {
   {0.5625, 0.3125},
   {0.4375, 0.6875},
   {0.8125, 0.5625},
   {0.3125, 0.1875},
   {0.1875, 0.8125},
   {0.0625, 0.4375},
   {0.6875, 0.9375},
   {0.9375, 0.0625},

};

static const float u_default_sample_positions_16x[][4] = {
   {0.5625, 0.5625},
   {0.4375, 0.3125},
   {0.3125, 0.625},
   {0.75,   0.4375},
   {0.1875, 0.375},
   {0.625,  0.8125},
   {0.8125, 0.6875},
   {0.6875, 0.1875},
   {0.375,  0.875},
   {0.5,    0.0625},
   {0.25,   0.125},
   {0.125,  0.75},
   {0.0,    0.5},
   {0.9375, 0.25},
   {0.875,  0.9375},
   {0.0625, 0.0},
};

static const float *
u_default_sample_position(unsigned sample_count, unsigned sample_index)
{
   switch (sample_count) {
   case  0:
   case  1: return u_default_sample_positions_1x[sample_index];
   case  2: return u_default_sample_positions_2x[sample_index];
   case  4: return u_default_sample_positions_4x[sample_index];
   case  8: return u_default_sample_positions_8x[sample_index];
   case 16: return u_default_sample_positions_16x[sample_index];
   default: unreachable("Invalid sample count");
   }
}

void
u_default_get_sample_position(struct pipe_context *ctx,
                              unsigned sample_count,
                              unsigned sample_index,
                              float *out_value)
{
   const float *positions =
      u_default_sample_position(sample_count, sample_index);

   out_value[0] = positions[0];
   out_value[1] = positions[1];
}
