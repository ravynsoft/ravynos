/**************************************************************************
 *
 * Copyright 2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef _RADEON_TEMPORAL_H
#define _RADEON_TEMPORAL_H

#include "radeon_video.h"
#include "radeon_vcn_enc.h"

#define RENCODE_MAX_TEMPORAL_LAYER_PATTERN_SIZE                                     9

typedef struct rvcn_temporal_layer_pattern_entry_s
{
   unsigned    temporal_id;
   unsigned    reference_index_in_table;
   bool        reference_modification;
   unsigned    frame_num_offset;
   unsigned    poc_offset;
   bool        mark_as_reference;
} rvcn_temporal_layer_pattern_entry_t;

typedef struct rvcn_temporal_layer_pattern_table_s
{
   unsigned    pattern_size;
   rvcn_temporal_layer_pattern_entry_t  pattern_table[RENCODE_MAX_TEMPORAL_LAYER_PATTERN_SIZE];
} rvcn_temporal_layer_pattern_table_t;

static const rvcn_temporal_layer_pattern_table_t  rvcn_temporal_layer_pattern_tables[RENCODE_MAX_NUM_TEMPORAL_LAYERS] =
{
   /* 1 temporal layer */
   {
      2,      /* temporal layer pattern size */
      {
         {
            0,
            0,
            false,
            0,
            0,
            true,
         },
         {
            0,
            0,
            false,
            1,
            2,
            true,
         }
      }
   },
   /* 2 temporal layers */
   {
      3,      /* temporal layer pattern size */
      {
         {
            0,
            0,
            false,
            0,
            0,
            true,
         },
         {
            1,
            0,
            false,
            1,
            2,
            false,
         },
         {
            0,
            0,
            false,
            1,
            4,
            true,
         }
      }
   },
   /* 3 temporal layers */
   {
      5,      /* temporal layer pattern size */
      {
         {
            0,
            0,
            false,
            0,
            0,
            true,
         },
         {
            2,
            0,
            false,
            1,
            2,
            false,
         },
         {
            1,
            0,
            false,
            1,
            4,
            true,
         },
         {
            2,
            2,
            false,
            2,
            6,
            false,
         },
         {
            0,
            0,
            true,
            2,
            8,
            true,
         }
      }
   },
   /* 4 temporal layers */
   {
      9,      /* temporal layer pattern size */
      {
         {
            0,
            0,
            false,
            0,
            0,
            true,
         },
         {
            3,
            0,
            false,
            1,
            2,
            false,
         },
         {
            2,
            0,
            false,
            1,
            4,
            true,
         },
         {
            3,
            2,
            false,
            2,
            6,
            false,
         },
         {
            1,
            0,
            true,
            2,
            8,
            true,
         },
         {
            3,
            4,
            false,
            3,
            10,
            false,
         },
         {
            2,
            4,
            false,
            3,
            12,
            true,
         },
         {
            3,
            6,
            false,
            4,
            14,
            false,
         },
         {
            0,
            0,
            true,
            4,
            16,
            true,
         }
      }
   }
};

#endif // _RADEON_TEMPORAL_H