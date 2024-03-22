/*
 * Copyright © 2016-2018 Broadcom
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "util/macros.h"
#include "broadcom/cle/v3d_decoder.h"
#include "clif_dump.h"
#include "clif_private.h"

#define __gen_user_data void
#define __gen_address_type uint32_t
#define __gen_address_offset(reloc) (*reloc)
#define __gen_emit_reloc(cl, reloc)
#define __gen_unpack_address(cl, s, e) (__gen_unpack_uint(cl, s, e) << (31 - (e - s)))
#include "broadcom/cle/v3dx_pack.h"
#include "broadcom/common/v3d_macros.h"

static char *
clif_name(const char *xml_name)
{
        char *name = malloc(strlen(xml_name) + 1);

        int j = 0;
        for (int i = 0; i < strlen(xml_name); i++) {
                if (xml_name[i] == ' ') {
                        name[j++] = '_';
                } else if (xml_name[i] == '(' || xml_name[i] == ')') {
                        /* skip */
                } else {
                        name[j++] = toupper(xml_name[i]);
                }
        }
        name[j++] = 0;

        return name;
}

bool
v3dX(clif_dump_packet)(struct clif_dump *clif, uint32_t offset,
                       const uint8_t *cl, uint32_t *size, bool reloc_mode)
{
        struct v3d_group *inst = v3d_spec_find_instruction(clif->spec, cl);
        if (!inst) {
                out(clif, "0x%08x: Unknown packet %d!\n", offset, *cl);
                return false;
        }

        *size = v3d_group_get_length(inst);

        if (!reloc_mode) {
                char *name = clif_name(v3d_group_get_name(inst));
                out(clif, "%s\n", name);
                free(name);
                v3d_print_group(clif, inst, 0, cl);
        }

        switch (*cl) {
        case V3DX(GL_SHADER_STATE_opcode): {
                struct V3DX(GL_SHADER_STATE) values;
                V3DX(GL_SHADER_STATE_unpack)(cl, &values);

                if (reloc_mode) {
                        struct reloc_worklist_entry *reloc =
                                clif_dump_add_address_to_worklist(clif,
                                                                  reloc_gl_shader_state,
                                                                  values.address);
                        if (reloc) {
                                reloc->shader_state.num_attrs =
                                        values.number_of_attribute_arrays;
                        }
                }
                return true;
        }

#if V3D_VERSION >= 41
        case V3DX(GL_SHADER_STATE_INCLUDING_GS_opcode): {
                struct V3DX(GL_SHADER_STATE_INCLUDING_GS) values;
                V3DX(GL_SHADER_STATE_INCLUDING_GS_unpack)(cl, &values);

                if (reloc_mode) {
                        struct reloc_worklist_entry *reloc =
                                clif_dump_add_address_to_worklist(clif,
                                                                  reloc_gl_including_gs_shader_state,
                                                                  values.address);
                        if (reloc) {
                                reloc->shader_state.num_attrs =
                                        values.number_of_attribute_arrays;
                        }
                }
                return true;
        }
#endif /* V3D_VERSION >= 41 */

#if V3D_VERSION < 40
        case V3DX(STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_EXTENDED_opcode): {
                struct V3DX(STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_EXTENDED) values;
                V3DX(STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_EXTENDED_unpack)(cl, &values);

                if (values.last_tile_of_frame)
                        return false;
                break;
        }
#endif /* V3D_VERSION < 40 */

#if V3D_VERSION > 40
        case V3DX(TRANSFORM_FEEDBACK_SPECS_opcode): {
                struct V3DX(TRANSFORM_FEEDBACK_SPECS) values;
                V3DX(TRANSFORM_FEEDBACK_SPECS_unpack)(cl, &values);
                struct v3d_group *spec = v3d_spec_find_struct(clif->spec,
                                                              "Transform Feedback Output Data Spec");
                assert(spec);

                cl += *size;

                for (int i = 0; i < values.number_of_16_bit_output_data_specs_following; i++) {
                        if (!reloc_mode)
                                v3d_print_group(clif, spec, 0, cl);
                        cl += v3d_group_get_length(spec);
                        *size += v3d_group_get_length(spec);
                }
                if (!reloc_mode)
                        out(clif, "@format ctrllist\n");
                break;
        }
#else /* V3D_VERSION < 40 */
        case V3DX(TRANSFORM_FEEDBACK_ENABLE_opcode): {
                struct V3DX(TRANSFORM_FEEDBACK_ENABLE) values;
                V3DX(TRANSFORM_FEEDBACK_ENABLE_unpack)(cl, &values);
                struct v3d_group *spec = v3d_spec_find_struct(clif->spec,
                                                              "Transform Feedback Output Data Spec");
                struct v3d_group *addr = v3d_spec_find_struct(clif->spec,
                                                              "Transform Feedback Output Address");
                assert(spec);
                assert(addr);

                cl += *size;

                for (int i = 0; i < values.number_of_16_bit_output_data_specs_following; i++) {
                        if (!reloc_mode)
                                v3d_print_group(clif, spec, 0, cl);
                        cl += v3d_group_get_length(spec);
                        *size += v3d_group_get_length(spec);
                }

                for (int i = 0; i < values.number_of_32_bit_output_buffer_address_following; i++) {
                        if (!reloc_mode)
                                v3d_print_group(clif, addr, 0, cl);
                        cl += v3d_group_get_length(addr);
                        *size += v3d_group_get_length(addr);
                }
                break;
        }
#endif /* V3D_VERSION < 40 */

        case V3DX(START_ADDRESS_OF_GENERIC_TILE_LIST_opcode): {
                struct V3DX(START_ADDRESS_OF_GENERIC_TILE_LIST) values;
                V3DX(START_ADDRESS_OF_GENERIC_TILE_LIST_unpack)(cl, &values);
                struct reloc_worklist_entry *reloc =
                        clif_dump_add_address_to_worklist(clif,
                                                          reloc_generic_tile_list,
                                                          values.start);
                reloc->generic_tile_list.end = values.end;
                break;
        }

        case V3DX(HALT_opcode):
                return false;
        }

        return true;
}
