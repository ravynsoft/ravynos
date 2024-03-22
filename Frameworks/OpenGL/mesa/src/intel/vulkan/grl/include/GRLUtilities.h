//
// Copyright (C) 2009-2021 Intel Corporation
//
// SPDX-License-Identifier: MIT
//
//

#pragma once

#include "GRLOCLCompatibility.h"

GRL_NAMESPACE_BEGIN(GRL)

    GRL_INLINE float4 bitShiftLdexp4(float4 x, int4 y)
    {
        y = (y + 127) << 23;
        return x * as_float4(y);
    }

    GRL_INLINE float3 bitShiftLdexp3(float3 x, int3 y)
    {
        y = (y + 127) << 23;
        return x * as_float3(y);
    }

    GRL_INLINE float bitShiftLdexp(float x, int y)
    {
        y = (y + 127) << 23;
        return x * as_float(y);
    }

GRL_NAMESPACE_END(GRL)