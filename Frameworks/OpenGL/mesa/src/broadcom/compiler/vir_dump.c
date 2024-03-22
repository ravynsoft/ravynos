/*
 * Copyright © 2016-2017 Broadcom
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

#include "broadcom/common/v3d_device_info.h"
#include "v3d_compiler.h"

/* Prints a human-readable description of the uniform reference. */
void
vir_dump_uniform(enum quniform_contents contents,
                 uint32_t data)
{
        static const char *quniform_names[] = {
                [QUNIFORM_LINE_WIDTH] = "line_width",
                [QUNIFORM_AA_LINE_WIDTH] = "aa_line_width",
                [QUNIFORM_VIEWPORT_X_SCALE] = "vp_x_scale",
                [QUNIFORM_VIEWPORT_Y_SCALE] = "vp_y_scale",
                [QUNIFORM_VIEWPORT_Z_OFFSET] = "vp_z_offset",
                [QUNIFORM_VIEWPORT_Z_SCALE] = "vp_z_scale",
                [QUNIFORM_SHARED_OFFSET] = "shared_offset",
        };

        switch (contents) {
        case QUNIFORM_CONSTANT:
                fprintf(stderr, "0x%08x / %f", data, uif(data));
                break;

        case QUNIFORM_UNIFORM:
                fprintf(stderr, "push[%d]", data);
                break;

        case QUNIFORM_TEXTURE_CONFIG_P1:
                fprintf(stderr, "tex[%d].p1", data);
                break;

        case QUNIFORM_TMU_CONFIG_P0:
                fprintf(stderr, "tex[%d].p0 | 0x%x",
                        v3d_unit_data_get_unit(data),
                        v3d_unit_data_get_offset(data));
                break;

        case QUNIFORM_TMU_CONFIG_P1:
                fprintf(stderr, "tex[%d].p1 | 0x%x",
                        v3d_unit_data_get_unit(data),
                        v3d_unit_data_get_offset(data));
                break;

        case QUNIFORM_IMAGE_TMU_CONFIG_P0:
                fprintf(stderr, "img[%d].p0 | 0x%x",
                        v3d_unit_data_get_unit(data),
                        v3d_unit_data_get_offset(data));
                break;

        case QUNIFORM_TEXTURE_WIDTH:
                fprintf(stderr, "tex[%d].width", data);
                break;
        case QUNIFORM_TEXTURE_HEIGHT:
                fprintf(stderr, "tex[%d].height", data);
                break;
        case QUNIFORM_TEXTURE_DEPTH:
                fprintf(stderr, "tex[%d].depth", data);
                break;
        case QUNIFORM_TEXTURE_ARRAY_SIZE:
                fprintf(stderr, "tex[%d].array_size", data);
                break;
        case QUNIFORM_TEXTURE_LEVELS:
                fprintf(stderr, "tex[%d].levels", data);
                break;

        case QUNIFORM_IMAGE_WIDTH:
                fprintf(stderr, "img[%d].width", data);
                break;
        case QUNIFORM_IMAGE_HEIGHT:
                fprintf(stderr, "img[%d].height", data);
                break;
        case QUNIFORM_IMAGE_DEPTH:
                fprintf(stderr, "img[%d].depth", data);
                break;
        case QUNIFORM_IMAGE_ARRAY_SIZE:
                fprintf(stderr, "img[%d].array_size", data);
                break;

        case QUNIFORM_SPILL_OFFSET:
                fprintf(stderr, "spill_offset");
                break;

        case QUNIFORM_SPILL_SIZE_PER_THREAD:
                fprintf(stderr, "spill_size_per_thread");
                break;

        case QUNIFORM_UBO_ADDR:
                fprintf(stderr, "ubo[%d]+0x%x",
                        v3d_unit_data_get_unit(data),
                        v3d_unit_data_get_offset(data));
                break;

        case QUNIFORM_SSBO_OFFSET:
                fprintf(stderr, "ssbo[%d]", data);
                break;

        case QUNIFORM_GET_SSBO_SIZE:
                fprintf(stderr, "ssbo_size[%d]", data);
                break;

        case QUNIFORM_GET_UBO_SIZE:
                fprintf(stderr, "ubo_size[%d]", data);
                break;

        case QUNIFORM_NUM_WORK_GROUPS:
                fprintf(stderr, "num_wg.%c", data < 3 ? "xyz"[data] : '?');
                break;

        default:
                if (quniform_contents_is_texture_p0(contents)) {
                        fprintf(stderr, "tex[%d].p0: 0x%08x",
                                contents - QUNIFORM_TEXTURE_CONFIG_P0_0,
                                data);
                } else if (contents < ARRAY_SIZE(quniform_names) &&
                           quniform_names[contents]) {
                        fprintf(stderr, "%s",
                                quniform_names[contents]);
                } else {
                        fprintf(stderr, "%d / 0x%08x", contents, data);
                }
        }
}

static void
vir_print_reg(struct v3d_compile *c, const struct qinst *inst,
              struct qreg reg)
{
        switch (reg.file) {

        case QFILE_NULL:
                fprintf(stderr, "null");
                break;

        case QFILE_LOAD_IMM:
                fprintf(stderr, "0x%08x (%f)", reg.index, uif(reg.index));
                break;

        case QFILE_REG:
                fprintf(stderr, "rf%d", reg.index);
                break;

        case QFILE_MAGIC:
                fprintf(stderr, "%s",
                        v3d_qpu_magic_waddr_name(c->devinfo, reg.index));
                break;

        case QFILE_SMALL_IMM: {
                uint32_t unpacked;
                bool ok = v3d_qpu_small_imm_unpack(c->devinfo,
                                                   inst->qpu.raddr_b,
                                                   &unpacked);
                assert(ok); (void) ok;

                int8_t *p = (int8_t *)&inst->qpu.raddr_b;
                if (*p >= -16 && *p <= 15)
                        fprintf(stderr, "%d", unpacked);
                else
                        fprintf(stderr, "%f", uif(unpacked));
                break;
        }

        case QFILE_TEMP:
                fprintf(stderr, "t%d", reg.index);
                break;
        }
}

static void
vir_dump_sig_addr(const struct v3d_device_info *devinfo,
                  const struct v3d_qpu_instr *instr)
{
        if (!instr->sig_magic)
                fprintf(stderr, ".rf%d", instr->sig_addr);
        else {
                const char *name =
                         v3d_qpu_magic_waddr_name(devinfo, instr->sig_addr);
                if (name)
                        fprintf(stderr, ".%s", name);
                else
                        fprintf(stderr, ".UNKNOWN%d", instr->sig_addr);
        }
}

static void
vir_dump_sig(struct v3d_compile *c, struct qinst *inst)
{
        struct v3d_qpu_sig *sig = &inst->qpu.sig;

        if (sig->thrsw)
                fprintf(stderr, "; thrsw");
        if (sig->ldvary) {
                fprintf(stderr, "; ldvary");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->ldvpm)
                fprintf(stderr, "; ldvpm");
        if (sig->ldtmu) {
                fprintf(stderr, "; ldtmu");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->ldtlb) {
                fprintf(stderr, "; ldtlb");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->ldtlbu) {
                fprintf(stderr, "; ldtlbu");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->ldunif)
                fprintf(stderr, "; ldunif");
        if (sig->ldunifrf) {
                fprintf(stderr, "; ldunifrf");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->ldunifa)
                fprintf(stderr, "; ldunifa");
        if (sig->ldunifarf) {
                fprintf(stderr, "; ldunifarf");
                vir_dump_sig_addr(c->devinfo, &inst->qpu);
        }
        if (sig->wrtmuc)
                fprintf(stderr, "; wrtmuc");
}

static void
vir_dump_alu(struct v3d_compile *c, struct qinst *inst)
{
        struct v3d_qpu_instr *instr = &inst->qpu;
        int nsrc = vir_get_nsrc(inst);
        enum v3d_qpu_input_unpack unpack[2];

        if (inst->qpu.alu.add.op != V3D_QPU_A_NOP) {
                fprintf(stderr, "%s", v3d_qpu_add_op_name(instr->alu.add.op));
                fprintf(stderr, "%s", v3d_qpu_cond_name(instr->flags.ac));
                fprintf(stderr, "%s", v3d_qpu_pf_name(instr->flags.apf));
                fprintf(stderr, "%s", v3d_qpu_uf_name(instr->flags.auf));
                fprintf(stderr, " ");

                vir_print_reg(c, inst, inst->dst);
                fprintf(stderr, "%s", v3d_qpu_pack_name(instr->alu.add.output_pack));

                unpack[0] = instr->alu.add.a.unpack;
                unpack[1] = instr->alu.add.b.unpack;
        } else {
                fprintf(stderr, "%s", v3d_qpu_mul_op_name(instr->alu.mul.op));
                fprintf(stderr, "%s", v3d_qpu_cond_name(instr->flags.mc));
                fprintf(stderr, "%s", v3d_qpu_pf_name(instr->flags.mpf));
                fprintf(stderr, "%s", v3d_qpu_uf_name(instr->flags.muf));
                fprintf(stderr, " ");

                vir_print_reg(c, inst, inst->dst);
                fprintf(stderr, "%s", v3d_qpu_pack_name(instr->alu.mul.output_pack));

                unpack[0] = instr->alu.mul.a.unpack;
                unpack[1] = instr->alu.mul.b.unpack;
        }

        for (int i = 0; i < nsrc; i++) {
                fprintf(stderr, ", ");
                vir_print_reg(c, inst, inst->src[i]);
                fprintf(stderr, "%s", v3d_qpu_unpack_name(unpack[i]));
        }

        vir_dump_sig(c, inst);
}

void
vir_dump_inst(struct v3d_compile *c, struct qinst *inst)
{
        struct v3d_qpu_instr *instr = &inst->qpu;

        switch (inst->qpu.type) {
        case V3D_QPU_INSTR_TYPE_ALU:
                vir_dump_alu(c, inst);
                break;
        case V3D_QPU_INSTR_TYPE_BRANCH:
                fprintf(stderr, "b");
                if (instr->branch.ub)
                        fprintf(stderr, "u");

                fprintf(stderr, "%s",
                        v3d_qpu_branch_cond_name(instr->branch.cond));
                fprintf(stderr, "%s", v3d_qpu_msfign_name(instr->branch.msfign));

                switch (instr->branch.bdi) {
                case V3D_QPU_BRANCH_DEST_ABS:
                        fprintf(stderr, "  zero_addr+0x%08x", instr->branch.offset);
                        break;

                case V3D_QPU_BRANCH_DEST_REL:
                        fprintf(stderr, "  %d", instr->branch.offset);
                        break;

                case V3D_QPU_BRANCH_DEST_LINK_REG:
                        fprintf(stderr, "  lri");
                        break;

                case V3D_QPU_BRANCH_DEST_REGFILE:
                        fprintf(stderr, "  rf%d", instr->branch.raddr_a);
                        break;
                }

                if (instr->branch.ub) {
                        switch (instr->branch.bdu) {
                        case V3D_QPU_BRANCH_DEST_ABS:
                                fprintf(stderr, ", a:unif");
                                break;

                        case V3D_QPU_BRANCH_DEST_REL:
                                fprintf(stderr, ", r:unif");
                                break;

                        case V3D_QPU_BRANCH_DEST_LINK_REG:
                                fprintf(stderr, ", lri");
                                break;

                        case V3D_QPU_BRANCH_DEST_REGFILE:
                                fprintf(stderr, ", rf%d", instr->branch.raddr_a);
                                break;
                        }
                }
                break;
        }

        if (vir_has_uniform(inst)) {
                fprintf(stderr, " (");
                vir_dump_uniform(c->uniform_contents[inst->uniform],
                                 c->uniform_data[inst->uniform]);
                fprintf(stderr, ")");
        }
}

void
vir_dump(struct v3d_compile *c)
{
        int ip = 0;
        int pressure = 0;

        vir_for_each_block(block, c) {
                fprintf(stderr, "BLOCK %d:\n", block->index);
                vir_for_each_inst(inst, block) {
                        if (c->live_intervals_valid) {
                                for (int i = 0; i < c->num_temps; i++) {
                                        if (c->temp_start[i] == ip)
                                                pressure++;
                                }

                                fprintf(stderr, "P%4d ", pressure);

                                bool first = true;

                                for (int i = 0; i < c->num_temps; i++) {
                                        if (c->temp_start[i] != ip)
                                                continue;

                                        if (first) {
                                                first = false;
                                        } else {
                                                fprintf(stderr, ", ");
                                        }
                                        if (BITSET_TEST(c->spillable, i))
                                                fprintf(stderr, "S%4d", i);
                                        else
                                                fprintf(stderr, "U%4d", i);
                                }

                                if (first)
                                        fprintf(stderr, "      ");
                                else
                                        fprintf(stderr, " ");
                        }

                        if (c->live_intervals_valid) {
                                bool first = true;

                                for (int i = 0; i < c->num_temps; i++) {
                                        if (c->temp_end[i] != ip)
                                                continue;

                                        if (first) {
                                                first = false;
                                        } else {
                                                fprintf(stderr, ", ");
                                        }
                                        fprintf(stderr, "E%4d", i);
                                        pressure--;
                                }

                                if (first)
                                        fprintf(stderr, "      ");
                                else
                                        fprintf(stderr, " ");
                        }

                        vir_dump_inst(c, inst);
                        fprintf(stderr, "\n");
                        ip++;
                }
                if (block->successors[1]) {
                        fprintf(stderr, "-> BLOCK %d, %d\n",
                                block->successors[0]->index,
                                block->successors[1]->index);
                } else if (block->successors[0]) {
                        fprintf(stderr, "-> BLOCK %d\n",
                                block->successors[0]->index);
                }
        }
}
