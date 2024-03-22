/*
 * Copyright © 2015 Intel Corporation
 * Copyright © 2015 Broadcom
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

#ifndef V3DX_PACK_H
#define V3DX_PACK_H

#ifndef V3D_VERSION
#  error "The V3D_VERSION macro must be defined"
#endif

#if (V3D_VERSION == 21)
#  include "cle/v3d_packet_v21_pack.h"
#elif (V3D_VERSION == 42)
#  include "cle/v3d_packet_v42_pack.h"
#elif (V3D_VERSION == 71)
#  include "cle/v3d_packet_v71_pack.h"
#else
#  error "Need to add a pack header include for this v3d version"
#endif

#endif /* V3DX_PACK_H */
