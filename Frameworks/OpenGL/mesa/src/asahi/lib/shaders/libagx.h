/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBAGX_H
#define LIBAGX_H

/* Define stdint types compatible between the CPU and GPU for shared headers */
#ifndef __OPENCL_VERSION__
#include <stdint.h>
#else
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

typedef ulong uint64_t;
typedef uint uint32_t;
typedef ushort uint16_t;
typedef uint uint8_t;

typedef long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef int int8_t;

/* Define NIR intrinsics for CL */
uint32_t nir_interleave_agx(uint16_t x, uint16_t y);
void nir_doorbell_agx(uint8_t value);
void nir_stack_map_agx(uint16_t index, uint32_t address);
uint32_t nir_stack_unmap_agx(uint16_t index);

#endif

#endif
