/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#include "vpe10_vpec.h"

static struct vpec_funcs vpec_funcs = {
    .check_swmode_support    = vpe10_vpec_check_swmode_support,
    .get_dcc_compression_cap = vpe10_vpec_get_dcc_compression_cap,
};

void vpe10_construct_vpec(struct vpe_priv *vpe_priv, struct vpec *vpec)
{
    vpec->vpe_priv = vpe_priv;
    vpec->funcs    = &vpec_funcs;
}

/** functions for capability check */
bool vpe10_vpec_check_swmode_support(struct vpec *vpec, enum vpe_swizzle_mode_values sw_mode)
{
    switch (sw_mode) {
    case VPE_SW_LINEAR:
    case VPE_SW_256B_D:
    case VPE_SW_4KB_D:
    case VPE_SW_64KB_D:
    case VPE_SW_64KB_D_T:
    case VPE_SW_4KB_D_X:
    case VPE_SW_64KB_D_X:
    case VPE_SW_64KB_R_X:
    case VPE_SW_VAR_D_X:
    case VPE_SW_VAR_R_X:
        return true;
    default:
        return false;
    }
}

bool vpe10_vpec_get_dcc_compression_cap(struct vpec *vpec,
    const struct vpe_dcc_surface_param *input, struct vpe_surface_dcc_cap *output)
{
    output->capable = false;
    return output->capable;
}
