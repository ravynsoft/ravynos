/*
 * Copyright © 2023 Raspberry Pi Ltd
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

#include "v3d_compiler.h"

#define __gen_user_data void
#define __gen_address_type uint32_t
#define __gen_address_offset(reloc) (*reloc)
#define __gen_emit_reloc(cl, reloc)
#define __gen_unpack_address(cl, s, e) (__gen_unpack_uint(cl, s, e) << (31 - (e - s)))
#include "cle/v3d_packet_v42_pack.h"


/* Typically, this method would wrap calling version-specific variant of this
 * method, but as TMU_CONFIG_PARAMETER_1 doesn't change between v42 and v71,
 * we can assume that p1_packed is the same struct, and use the same method.
 */
void
v3d_pack_unnormalized_coordinates(struct v3d_device_info *devinfo,
                                  uint32_t *p1_packed,
                                  bool unnormalized_coordinates)
{
        assert(devinfo->ver == 71 || devinfo->ver == 42);

        struct V3D42_TMU_CONFIG_PARAMETER_1 p1_unpacked;
        V3D42_TMU_CONFIG_PARAMETER_1_unpack((uint8_t *)p1_packed, &p1_unpacked);
        p1_unpacked.unnormalized_coordinates = unnormalized_coordinates;
        V3D42_TMU_CONFIG_PARAMETER_1_pack(NULL, (uint8_t *)p1_packed,
                                     &p1_unpacked);
}
