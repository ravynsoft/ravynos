/*
 * Copyright © 2014 Broadcom
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

#include "util/u_math.h"
#include "util/u_prim.h"
#include "util/macros.h"
#include "vc4_cl_dump.h"
#include "kernel/vc4_packet.h"

#include "broadcom/cle/v3d_decoder.h"
#include "broadcom/clif/clif_dump.h"

void
vc4_dump_cl(void *cl, uint32_t size, bool is_render)
{
        struct v3d_device_info devinfo = {
                /* While the driver supports V3D 2.1 and 2.6, we haven't split
                 * off a 2.6 XML yet (there are a couple of fields different
                 * in render target formatting)
                 */
                .ver = 21,
        };
        struct v3d_spec *spec = v3d_spec_load(&devinfo);

        struct clif_dump *clif = clif_dump_init(&devinfo, stderr, true, false);

        uint32_t offset = 0, hw_offset = 0;
        uint8_t *p = cl;

        while (offset < size) {
                struct v3d_group *inst = v3d_spec_find_instruction(spec, p);
                uint8_t header = *p;
                uint32_t length;

                if (inst == NULL) {
                        fprintf(stderr, "0x%08x 0x%08x: Unknown packet 0x%02x (%d)!\n",
                                offset, hw_offset, header, header);
                        return;
                }

                length = v3d_group_get_length(inst);

                fprintf(stderr, "0x%08x 0x%08x: 0x%02x %s\n",
                        offset, hw_offset, header, v3d_group_get_name(inst));

                v3d_print_group(clif, inst, offset, p);

                switch (header) {
                case VC4_PACKET_HALT:
                case VC4_PACKET_STORE_MS_TILE_BUFFER_AND_EOF:
                        return;
                default:
                        break;
                }

                offset += length;
                if (header != VC4_PACKET_GEM_HANDLES)
                        hw_offset += length;
                p += length;
        }

        clif_dump_destroy(clif);
}

