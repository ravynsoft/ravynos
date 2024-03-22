// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

#![allow(non_upper_case_globals)]

use crate::cfg::CFGBuilder;
use crate::ir::*;
use crate::nir::*;
use crate::sph::{OutputTopology, PixelImap};

use nak_bindings::*;

use std::cmp::max;
use std::collections::{HashMap, HashSet};
use std::ops::Index;

fn init_info_from_nir(nir: &nir_shader, sm: u8) -> ShaderInfo {
    ShaderInfo {
        sm: sm,
        num_gprs: 0,
        num_barriers: 0,
        slm_size: nir.scratch_size,
        uses_global_mem: false,
        writes_global_mem: false,
        // TODO: handle this.
        uses_fp64: false,
        stage: match nir.info.stage() {
            MESA_SHADER_COMPUTE => {
                ShaderStageInfo::Compute(ComputeShaderInfo {
                    local_size: [
                        nir.info.workgroup_size[0].into(),
                        nir.info.workgroup_size[1].into(),
                        nir.info.workgroup_size[2].into(),
                    ],
                    smem_size: nir.info.shared_size.try_into().unwrap(),
                })
            }
            MESA_SHADER_VERTEX => ShaderStageInfo::Vertex,
            MESA_SHADER_FRAGMENT => ShaderStageInfo::Fragment,
            MESA_SHADER_GEOMETRY => {
                let info_gs = unsafe { &nir.info.__bindgen_anon_1.gs };
                let output_topology = match info_gs.output_primitive {
                    MESA_PRIM_POINTS => OutputTopology::PointList,
                    MESA_PRIM_LINE_STRIP => OutputTopology::LineStrip,
                    MESA_PRIM_TRIANGLE_STRIP => OutputTopology::TriangleStrip,
                    _ => panic!(
                        "Invalid GS input primitive {}",
                        info_gs.input_primitive
                    ),
                };

                ShaderStageInfo::Geometry(GeometryShaderInfo {
                    // TODO: Should be set if VK_NV_geometry_shader_passthrough is in use.
                    passthrough_enable: false,
                    stream_out_mask: info_gs.active_stream_mask(),
                    threads_per_input_primitive: info_gs.invocations,
                    output_topology: output_topology,
                    max_output_vertex_count: info_gs.vertices_out,
                })
            }
            MESA_SHADER_TESS_CTRL => {
                let info_tess = unsafe { &nir.info.__bindgen_anon_1.tess };
                ShaderStageInfo::TessellationInit(TessellationInitShaderInfo {
                    per_patch_attribute_count: 6,
                    threads_per_patch: info_tess.tcs_vertices_out,
                })
            }
            MESA_SHADER_TESS_EVAL => ShaderStageInfo::Tessellation,
            _ => panic!("Unknown shader stage"),
        },
        io: match nir.info.stage() {
            MESA_SHADER_COMPUTE => ShaderIoInfo::None,
            MESA_SHADER_FRAGMENT => ShaderIoInfo::Fragment(FragmentIoInfo {
                sysvals_in: SysValInfo {
                    // Required on fragment shaders, otherwise it cause a trap.
                    ab: 1 << 31,
                    c: 0,
                },
                sysvals_in_d: [PixelImap::Unused; 8],
                attr_in: [PixelImap::Unused; 128],
                barycentric_attr_in: [0; 4],
                reads_sample_mask: false,
                uses_kill: false,
                writes_color: 0,
                writes_sample_mask: false,
                writes_depth: false,
                // TODO: Should be set if interlocks are in use. (VK_EXT_fragment_shader_interlock)
                does_interlock: false,
            }),
            MESA_SHADER_VERTEX
            | MESA_SHADER_GEOMETRY
            | MESA_SHADER_TESS_CTRL
            | MESA_SHADER_TESS_EVAL => ShaderIoInfo::Vtg(VtgIoInfo {
                sysvals_in: SysValInfo::default(),
                sysvals_in_d: 0,
                sysvals_out: SysValInfo::default(),
                sysvals_out_d: 0,
                attr_in: [0; 4],
                attr_out: [0; 4],

                // TODO: figure out how to fill this.
                store_req_start: u8::MAX,
                store_req_end: 0,
            }),
            _ => panic!("Unknown shader stage"),
        },
    }
}

fn alloc_ssa_for_nir(b: &mut impl SSABuilder, ssa: &nir_def) -> Vec<SSAValue> {
    let (file, comps) = if ssa.bit_size == 1 {
        (RegFile::Pred, ssa.num_components)
    } else {
        let bits = ssa.bit_size * ssa.num_components;
        (RegFile::GPR, bits.div_ceil(32))
    };

    let mut vec = Vec::new();
    for _ in 0..comps {
        vec.push(b.alloc_ssa(file, 1)[0]);
    }
    vec
}

struct PhiAllocMap<'a> {
    alloc: &'a mut PhiAllocator,
    map: HashMap<(u32, u8), u32>,
}

impl<'a> PhiAllocMap<'a> {
    fn new(alloc: &'a mut PhiAllocator) -> PhiAllocMap<'a> {
        PhiAllocMap {
            alloc: alloc,
            map: HashMap::new(),
        }
    }

    fn get_phi_id(&mut self, phi: &nir_phi_instr, comp: u8) -> u32 {
        *self
            .map
            .entry((phi.def.index, comp))
            .or_insert_with(|| self.alloc.alloc())
    }
}

struct PerSizeFloatControls {
    pub ftz: bool,
    pub rnd_mode: FRndMode,
}

struct ShaderFloatControls {
    pub fp16: PerSizeFloatControls,
    pub fp32: PerSizeFloatControls,
    pub fp64: PerSizeFloatControls,
}

impl Default for ShaderFloatControls {
    fn default() -> Self {
        Self {
            fp16: PerSizeFloatControls {
                ftz: false,
                rnd_mode: FRndMode::NearestEven,
            },
            fp32: PerSizeFloatControls {
                ftz: true, // Default FTZ on fp32
                rnd_mode: FRndMode::NearestEven,
            },
            fp64: PerSizeFloatControls {
                ftz: false,
                rnd_mode: FRndMode::NearestEven,
            },
        }
    }
}

impl ShaderFloatControls {
    fn from_nir(nir: &nir_shader) -> ShaderFloatControls {
        let nir_fc = nir.info.float_controls_execution_mode;
        let mut fc: ShaderFloatControls = Default::default();

        if (nir_fc & FLOAT_CONTROLS_DENORM_PRESERVE_FP16) != 0 {
            fc.fp16.ftz = false;
        } else if (nir_fc & FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP16) != 0 {
            fc.fp16.ftz = true;
        }
        if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP16) != 0 {
            fc.fp16.rnd_mode = FRndMode::NearestEven;
        } else if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP16) != 0 {
            fc.fp16.rnd_mode = FRndMode::Zero;
        }

        if (nir_fc & FLOAT_CONTROLS_DENORM_PRESERVE_FP32) != 0 {
            fc.fp32.ftz = false;
        } else if (nir_fc & FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP32) != 0 {
            fc.fp32.ftz = true;
        }
        if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP32) != 0 {
            fc.fp32.rnd_mode = FRndMode::NearestEven;
        } else if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP32) != 0 {
            fc.fp32.rnd_mode = FRndMode::Zero;
        }

        if (nir_fc & FLOAT_CONTROLS_DENORM_PRESERVE_FP64) != 0 {
            fc.fp64.ftz = false;
        } else if (nir_fc & FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP64) != 0 {
            fc.fp64.ftz = true;
        }
        if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP64) != 0 {
            fc.fp64.rnd_mode = FRndMode::NearestEven;
        } else if (nir_fc & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP64) != 0 {
            fc.fp64.rnd_mode = FRndMode::Zero;
        }

        fc
    }
}

impl Index<FloatType> for ShaderFloatControls {
    type Output = PerSizeFloatControls;

    fn index(&self, idx: FloatType) -> &PerSizeFloatControls {
        match idx {
            FloatType::F16 => &self.fp16,
            FloatType::F32 => &self.fp32,
            FloatType::F64 => &self.fp64,
        }
    }
}

struct ShaderFromNir<'a> {
    nir: &'a nir_shader,
    info: ShaderInfo,
    float_ctl: ShaderFloatControls,
    cfg: CFGBuilder<u32, BasicBlock>,
    label_alloc: LabelAllocator,
    block_label: HashMap<u32, Label>,
    bar_label: HashMap<u32, Label>,
    fs_out_regs: [SSAValue; 34],
    end_block_id: u32,
    ssa_map: HashMap<u32, Vec<SSAValue>>,
    saturated: HashSet<*const nir_def>,
}

impl<'a> ShaderFromNir<'a> {
    fn new(nir: &'a nir_shader, sm: u8) -> Self {
        Self {
            nir: nir,
            info: init_info_from_nir(nir, sm),
            float_ctl: ShaderFloatControls::from_nir(nir),
            cfg: CFGBuilder::new(),
            label_alloc: LabelAllocator::new(),
            block_label: HashMap::new(),
            bar_label: HashMap::new(),
            fs_out_regs: [SSAValue::NONE; 34],
            end_block_id: 0,
            ssa_map: HashMap::new(),
            saturated: HashSet::new(),
        }
    }

    fn get_block_label(&mut self, block: &nir_block) -> Label {
        *self
            .block_label
            .entry(block.index)
            .or_insert_with(|| self.label_alloc.alloc())
    }

    fn get_ssa(&mut self, ssa: &nir_def) -> &[SSAValue] {
        self.ssa_map.get(&ssa.index).unwrap()
    }

    fn set_ssa(&mut self, def: &nir_def, vec: Vec<SSAValue>) {
        if def.bit_size == 1 {
            for s in &vec {
                assert!(s.is_predicate());
            }
        } else {
            for s in &vec {
                assert!(!s.is_predicate());
            }
            let bits =
                usize::from(def.bit_size) * usize::from(def.num_components);
            assert!(vec.len() == bits.div_ceil(32).into());
        }
        self.ssa_map
            .entry(def.index)
            .and_modify(|_| panic!("Cannot set an SSA def twice"))
            .or_insert(vec);
    }

    fn get_ssa_comp(&mut self, def: &nir_def, c: u8) -> (SSARef, u8) {
        let vec = self.get_ssa(def);
        match def.bit_size {
            1 => (vec[usize::from(c)].into(), 0),
            8 => (vec[usize::from(c / 4)].into(), c % 4),
            16 => (vec[usize::from(c / 2)].into(), (c * 2) % 4),
            32 => (vec[usize::from(c)].into(), 0),
            64 => {
                let comps =
                    [vec[usize::from(c) * 2 + 0], vec[usize::from(c) * 2 + 1]];
                (comps.into(), 0)
            }
            _ => panic!("Unsupported bit size: {}", def.bit_size),
        }
    }

    fn get_src(&mut self, src: &nir_src) -> Src {
        SSARef::try_from(self.get_ssa(&src.as_def()))
            .unwrap()
            .into()
    }

    fn get_io_addr_offset(
        &mut self,
        addr: &nir_src,
        imm_bits: u8,
    ) -> (Src, i32) {
        let addr = addr.as_def();
        let addr_offset = unsafe {
            nak_get_io_addr_offset(addr as *const _ as *mut _, imm_bits)
        };

        if let Some(base_def) = std::ptr::NonNull::new(addr_offset.base.def) {
            let base_def = unsafe { base_def.as_ref() };
            let base_comp = u8::try_from(addr_offset.base.comp).unwrap();
            let (base, _) = self.get_ssa_comp(base_def, base_comp);
            (base.into(), addr_offset.offset)
        } else {
            (SrcRef::Zero.into(), addr_offset.offset)
        }
    }

    fn set_dst(&mut self, def: &nir_def, ssa: SSARef) {
        self.set_ssa(def, (*ssa).into());
    }

    fn try_saturate_alu_dst(&mut self, def: &nir_def) -> bool {
        if def.all_uses_are_fsat() {
            self.saturated.insert(def as *const _);
            true
        } else {
            false
        }
    }

    fn alu_src_is_saturated(&self, src: &nir_alu_src) -> bool {
        self.saturated.get(&(src.as_def() as *const _)).is_some()
    }

    fn parse_alu(&mut self, b: &mut impl SSABuilder, alu: &nir_alu_instr) {
        // Handle vectors and pack ops as a special case since they're the only
        // ALU ops that can produce more than 16B. They are also the only ALU
        // ops which we allow to consume small (8 and 16-bit) vector data
        // scattered across multiple dwords
        match alu.op {
            nir_op_mov
            | nir_op_pack_32_4x8_split
            | nir_op_pack_32_2x16_split
            | nir_op_pack_64_2x32_split
            | nir_op_vec2
            | nir_op_vec3
            | nir_op_vec4
            | nir_op_vec5
            | nir_op_vec8
            | nir_op_vec16 => {
                let src_bit_size = alu.get_src(0).src.bit_size();
                let bits = usize::from(alu.def.num_components)
                    * usize::from(alu.def.bit_size);

                // Collect the sources into a vec with src_bit_size per SSA
                // value in the vec.  This implicitly makes 64-bit sources look
                // like two 32-bit values
                let mut srcs = Vec::new();
                if alu.op == nir_op_mov {
                    let src = alu.get_src(0);
                    for c in 0..alu.def.num_components {
                        let s = src.swizzle[usize::from(c)];
                        let (src, byte) =
                            self.get_ssa_comp(src.src.as_def(), s);
                        for ssa in src.iter() {
                            srcs.push((*ssa, byte));
                        }
                    }
                } else {
                    for src in alu.srcs_as_slice().iter() {
                        let s = src.swizzle[0];
                        let (src, byte) =
                            self.get_ssa_comp(src.src.as_def(), s);
                        for ssa in src.iter() {
                            srcs.push((*ssa, byte));
                        }
                    }
                }

                let mut comps = Vec::new();
                match src_bit_size {
                    1 | 32 | 64 => {
                        for (ssa, _) in srcs {
                            comps.push(ssa);
                        }
                    }
                    8 => {
                        for dc in 0..bits.div_ceil(32) {
                            let mut psrc = [Src::new_zero(); 4];
                            let mut psel = [0_u8; 4];

                            for b in 0..4 {
                                let sc = dc * 4 + b;
                                if sc < srcs.len() {
                                    let (ssa, byte) = srcs[sc];
                                    for i in 0..4_u8 {
                                        let psrc_i = &mut psrc[usize::from(i)];
                                        if *psrc_i == Src::new_zero() {
                                            *psrc_i = ssa.into();
                                        } else if *psrc_i != Src::from(ssa) {
                                            continue;
                                        }
                                        psel[b] = i * 4 + byte;
                                    }
                                }
                            }
                            comps.push(b.prmt4(psrc, psel)[0]);
                        }
                    }
                    16 => {
                        for dc in 0..bits.div_ceil(32) {
                            let mut psrc = [Src::new_zero(); 2];
                            let mut psel = [0_u8; 4];

                            for w in 0..2 {
                                let sc = dc * 2 + w;
                                if sc < srcs.len() {
                                    let (ssa, byte) = srcs[sc];
                                    let w_u8 = u8::try_from(w).unwrap();
                                    psrc[w] = ssa.into();
                                    psel[w * 2 + 0] = (w_u8 * 4) + byte;
                                    psel[w * 2 + 1] = (w_u8 * 4) + byte + 1;
                                }
                            }
                            comps.push(b.prmt(psrc[0], psrc[1], psel)[0]);
                        }
                    }
                    _ => panic!("Unknown bit size: {src_bit_size}"),
                }

                self.set_ssa(&alu.def, comps);
                return;
            }
            _ => (),
        }

        let mut srcs: Vec<Src> = Vec::new();
        for (i, alu_src) in alu.srcs_as_slice().iter().enumerate() {
            let bit_size = alu_src.src.bit_size();
            let comps = alu.src_components(i.try_into().unwrap());
            let ssa = self.get_ssa(&alu_src.src.as_def());

            match bit_size {
                1 => {
                    assert!(comps == 1);
                    let s = usize::from(alu_src.swizzle[0]);
                    srcs.push(ssa[s].into());
                }
                8 => {
                    assert!(comps <= 4);
                    let s = alu_src.swizzle[0];
                    let dw = ssa[usize::from(s / 4)];

                    let mut prmt = [4_u8; 4];
                    for c in 0..comps {
                        let cs = alu_src.swizzle[usize::from(c)];
                        assert!(s / 4 == cs / 4);
                        prmt[usize::from(c)] = cs;
                    }
                    srcs.push(b.prmt(dw.into(), 0.into(), prmt).into());
                }
                16 => {
                    assert!(comps <= 2);
                    let s = alu_src.swizzle[0];
                    let dw = ssa[usize::from(s / 2)];

                    let mut prmt = [0_u8; 4];
                    for c in 0..comps {
                        let cs = alu_src.swizzle[usize::from(c)];
                        assert!(s / 2 == cs / 2);
                        prmt[usize::from(c) * 2 + 0] = cs * 2 + 0;
                        prmt[usize::from(c) * 2 + 1] = cs * 2 + 1;
                    }
                    // TODO: Some ops can handle swizzles
                    srcs.push(b.prmt(dw.into(), 0.into(), prmt).into());
                }
                32 => {
                    assert!(comps == 1);
                    let s = usize::from(alu_src.swizzle[0]);
                    srcs.push(ssa[s].into());
                }
                64 => {
                    assert!(comps == 1);
                    let s = usize::from(alu_src.swizzle[0]);
                    srcs.push([ssa[s * 2], ssa[s * 2 + 1]].into());
                }
                _ => panic!("Invalid bit size: {bit_size}"),
            }
        }

        let dst: SSARef = match alu.op {
            nir_op_b2b1 => {
                assert!(alu.get_src(0).bit_size() == 32);
                b.isetp(IntCmpType::I32, IntCmpOp::Ne, srcs[0], 0.into())
            }
            nir_op_b2b32 | nir_op_b2i8 | nir_op_b2i16 | nir_op_b2i32 => {
                b.sel(srcs[0].bnot(), 0.into(), 1.into())
            }
            nir_op_b2i64 => {
                let lo = b.sel(srcs[0].bnot(), 0.into(), 1.into());
                let hi = b.copy(0.into());
                [lo[0], hi[0]].into()
            }
            nir_op_b2f32 => {
                b.sel(srcs[0].bnot(), 0.0_f32.into(), 1.0_f32.into())
            }
            nir_op_b2f64 => {
                let lo = b.copy(0.into());
                let hi = b.sel(srcs[0].bnot(), 0.into(), 0x3ff00000.into());
                [lo[0], hi[0]].into()
            }
            nir_op_bcsel => b.sel(srcs[0], srcs[1], srcs[2]),
            nir_op_bit_count => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpPopC {
                    dst: dst.into(),
                    src: srcs[0],
                });
                dst
            }
            nir_op_bitfield_reverse => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpBrev {
                    dst: dst.into(),
                    src: srcs[0],
                });
                dst
            }
            nir_op_extract_u8 | nir_op_extract_i8 | nir_op_extract_u16
            | nir_op_extract_i16 => {
                let src1 = alu.get_src(1);
                let elem = src1.src.comp_as_uint(src1.swizzle[0]).unwrap();
                let elem = u8::try_from(elem).unwrap();

                match alu.op {
                    nir_op_extract_u8 => {
                        assert!(elem < 4);
                        let byte = elem;
                        let zero = 4;
                        b.prmt(srcs[0], 0.into(), [byte, zero, zero, zero])
                    }
                    nir_op_extract_i8 => {
                        assert!(elem < 4);
                        let byte = elem;
                        let sign = byte | 0x8;
                        b.prmt(srcs[0], 0.into(), [byte, sign, sign, sign])
                    }
                    nir_op_extract_u16 => {
                        assert!(elem < 2);
                        let byte = elem * 2;
                        let zero = 4;
                        b.prmt(srcs[0], 0.into(), [byte, byte + 1, zero, zero])
                    }
                    nir_op_extract_i16 => {
                        assert!(elem < 2);
                        let byte = elem * 2;
                        let sign = (byte + 1) | 0x8;
                        b.prmt(srcs[0], 0.into(), [byte, byte + 1, sign, sign])
                    }
                    _ => panic!("Unknown extract op: {}", alu.op),
                }
            }
            nir_op_f2f16 | nir_op_f2f16_rtne | nir_op_f2f16_rtz
            | nir_op_f2f32 | nir_op_f2f64 => {
                let src_bits = alu.get_src(0).src.bit_size();
                let dst_bits = alu.def.bit_size();
                let src_type = FloatType::from_bits(src_bits.into());
                let dst_type = FloatType::from_bits(dst_bits.into());

                let dst = b.alloc_ssa(RegFile::GPR, dst_bits.div_ceil(32));
                b.push_op(OpF2F {
                    dst: dst.into(),
                    src: srcs[0],
                    src_type: FloatType::from_bits(src_bits.into()),
                    dst_type: dst_type,
                    rnd_mode: match alu.op {
                        nir_op_f2f16_rtne => FRndMode::NearestEven,
                        nir_op_f2f16_rtz => FRndMode::Zero,
                        _ => self.float_ctl[dst_type].rnd_mode,
                    },
                    ftz: if src_bits < dst_bits {
                        self.float_ctl[src_type].ftz
                    } else {
                        self.float_ctl[dst_type].ftz
                    },
                    high: false,
                });
                dst
            }
            nir_op_find_lsb => {
                let tmp = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpBrev {
                    dst: tmp.into(),
                    src: srcs[0],
                });
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFlo {
                    dst: dst.into(),
                    src: tmp.into(),
                    signed: alu.op == nir_op_ifind_msb,
                    return_shift_amount: true,
                });
                dst
            }
            nir_op_f2i8 | nir_op_f2i16 | nir_op_f2i32 | nir_op_f2i64
            | nir_op_f2u8 | nir_op_f2u16 | nir_op_f2u32 | nir_op_f2u64 => {
                let src_bits = usize::from(alu.get_src(0).bit_size());
                let dst_bits = alu.def.bit_size();
                let src_type = FloatType::from_bits(src_bits);
                let dst = b.alloc_ssa(RegFile::GPR, dst_bits.div_ceil(32));
                let dst_is_signed = alu.info().output_type & 2 != 0;
                b.push_op(OpF2I {
                    dst: dst.into(),
                    src: srcs[0],
                    src_type: src_type,
                    dst_type: IntType::from_bits(
                        dst_bits.into(),
                        dst_is_signed,
                    ),
                    rnd_mode: FRndMode::Zero,
                    ftz: self.float_ctl[src_type].ftz,
                });
                dst
            }
            nir_op_fabs | nir_op_fadd | nir_op_fneg => {
                let (x, y) = match alu.op {
                    nir_op_fabs => (srcs[0].fabs(), 0.0_f32.into()),
                    nir_op_fadd => (srcs[0], srcs[1]),
                    nir_op_fneg => (Src::new_zero().fneg(), srcs[0].fneg()),
                    _ => panic!("Unhandled case"),
                };
                let ftype = FloatType::from_bits(alu.def.bit_size().into());
                let dst;
                if alu.def.bit_size() == 64 {
                    dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpDAdd {
                        dst: dst.into(),
                        srcs: [x, y],
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                    });
                } else if alu.def.bit_size() == 32 {
                    dst = b.alloc_ssa(RegFile::GPR, 1);
                    b.push_op(OpFAdd {
                        dst: dst.into(),
                        srcs: [x, y],
                        saturate: self.try_saturate_alu_dst(&alu.def),
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                        ftz: self.float_ctl[ftype].ftz,
                    });
                } else {
                    panic!("Unsupported float type: f{}", alu.def.bit_size());
                }
                dst
            }
            nir_op_fceil | nir_op_ffloor | nir_op_fround_even
            | nir_op_ftrunc => {
                assert!(alu.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                let ty = FloatType::from_bits(alu.def.bit_size().into());
                let rnd_mode = match alu.op {
                    nir_op_fceil => FRndMode::PosInf,
                    nir_op_ffloor => FRndMode::NegInf,
                    nir_op_ftrunc => FRndMode::Zero,
                    nir_op_fround_even => FRndMode::NearestEven,
                    _ => unreachable!(),
                };
                b.push_op(OpFRnd {
                    dst: dst.into(),
                    src: srcs[0],
                    src_type: ty,
                    dst_type: ty,
                    rnd_mode,
                    ftz: self.float_ctl[ty].ftz,
                });
                dst
            }
            nir_op_fcos => {
                let frac_1_2pi = 1.0 / (2.0 * std::f32::consts::PI);
                let tmp = b.fmul(srcs[0], frac_1_2pi.into());
                b.mufu(MuFuOp::Cos, tmp.into())
            }
            nir_op_feq | nir_op_fge | nir_op_flt | nir_op_fneu => {
                let src_type =
                    FloatType::from_bits(alu.get_src(0).bit_size().into());
                let cmp_op = match alu.op {
                    nir_op_feq => FloatCmpOp::OrdEq,
                    nir_op_fge => FloatCmpOp::OrdGe,
                    nir_op_flt => FloatCmpOp::OrdLt,
                    nir_op_fneu => FloatCmpOp::UnordNe,
                    _ => panic!("Usupported float comparison"),
                };

                let dst = b.alloc_ssa(RegFile::Pred, 1);
                if alu.get_src(0).bit_size() == 64 {
                    b.push_op(OpDSetP {
                        dst: dst.into(),
                        set_op: PredSetOp::And,
                        cmp_op: cmp_op,
                        srcs: [srcs[0], srcs[1]],
                        accum: SrcRef::True.into(),
                    });
                } else if alu.get_src(0).bit_size() == 32 {
                    b.push_op(OpFSetP {
                        dst: dst.into(),
                        set_op: PredSetOp::And,
                        cmp_op: cmp_op,
                        srcs: [srcs[0], srcs[1]],
                        accum: SrcRef::True.into(),
                        ftz: self.float_ctl[src_type].ftz,
                    });
                } else {
                    panic!(
                        "Unsupported float type: f{}",
                        alu.get_src(0).bit_size()
                    );
                }
                dst
            }
            nir_op_fexp2 => b.mufu(MuFuOp::Exp2, srcs[0]),
            nir_op_ffma => {
                let ftype = FloatType::from_bits(alu.def.bit_size().into());
                let dst;
                if alu.def.bit_size() == 64 {
                    debug_assert!(!self.float_ctl[ftype].ftz);
                    dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpDFma {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1], srcs[2]],
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                    });
                } else if alu.def.bit_size() == 32 {
                    dst = b.alloc_ssa(RegFile::GPR, 1);
                    b.push_op(OpFFma {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1], srcs[2]],
                        saturate: self.try_saturate_alu_dst(&alu.def),
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                        // The hardware doesn't like FTZ+DNZ and DNZ implies FTZ
                        // anyway so only set one of the two bits.
                        ftz: self.float_ctl[ftype].ftz,
                        dnz: false,
                    });
                } else {
                    panic!("Unsupported float type: f{}", alu.def.bit_size());
                }
                dst
            }
            nir_op_ffmaz => {
                assert!(alu.def.bit_size() == 32);
                // DNZ implies FTZ so we need FTZ set or this is invalid
                assert!(self.float_ctl.fp32.ftz);
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFFma {
                    dst: dst.into(),
                    srcs: [srcs[0], srcs[1], srcs[2]],
                    saturate: self.try_saturate_alu_dst(&alu.def),
                    rnd_mode: self.float_ctl.fp32.rnd_mode,
                    // The hardware doesn't like FTZ+DNZ and DNZ implies FTZ
                    // anyway so only set one of the two bits.
                    ftz: false,
                    dnz: true,
                });
                dst
            }
            nir_op_flog2 => {
                assert!(alu.def.bit_size() == 32);
                b.mufu(MuFuOp::Log2, srcs[0])
            }
            nir_op_fmax | nir_op_fmin => {
                let dst;
                if alu.def.bit_size() == 64 {
                    dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpDMnMx {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1]],
                        min: (alu.op == nir_op_fmin).into(),
                    });
                } else if alu.def.bit_size() == 32 {
                    dst = b.alloc_ssa(RegFile::GPR, 1);
                    b.push_op(OpFMnMx {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1]],
                        min: (alu.op == nir_op_fmin).into(),
                        ftz: self.float_ctl.fp32.ftz,
                    });
                } else {
                    panic!("Unsupported float type: f{}", alu.def.bit_size());
                }
                dst
            }
            nir_op_fmul => {
                let ftype = FloatType::from_bits(alu.def.bit_size().into());
                let dst;
                if alu.def.bit_size() == 64 {
                    debug_assert!(!self.float_ctl[ftype].ftz);
                    dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpDMul {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1]],
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                    });
                } else if alu.def.bit_size() == 32 {
                    dst = b.alloc_ssa(RegFile::GPR, 1);
                    b.push_op(OpFMul {
                        dst: dst.into(),
                        srcs: [srcs[0], srcs[1]],
                        saturate: self.try_saturate_alu_dst(&alu.def),
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                        ftz: self.float_ctl[ftype].ftz,
                        dnz: false,
                    });
                } else {
                    panic!("Unsupported float type: f{}", alu.def.bit_size());
                }
                dst
            }
            nir_op_fmulz => {
                assert!(alu.def.bit_size() == 32);
                // DNZ implies FTZ so we need FTZ set or this is invalid
                assert!(self.float_ctl.fp32.ftz);
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFMul {
                    dst: dst.into(),
                    srcs: [srcs[0], srcs[1]],
                    saturate: self.try_saturate_alu_dst(&alu.def),
                    rnd_mode: self.float_ctl.fp32.rnd_mode,
                    // The hardware doesn't like FTZ+DNZ and DNZ implies FTZ
                    // anyway so only set one of the two bits.
                    ftz: false,
                    dnz: true,
                });
                dst
            }
            nir_op_fquantize2f16 => {
                let tmp = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpF2F {
                    dst: tmp.into(),
                    src: srcs[0],
                    src_type: FloatType::F32,
                    dst_type: FloatType::F16,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: true,
                    high: false,
                });
                assert!(alu.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpF2F {
                    dst: dst.into(),
                    src: tmp.into(),
                    src_type: FloatType::F16,
                    dst_type: FloatType::F32,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: true,
                    high: false,
                });
                dst
            }
            nir_op_frcp => {
                assert!(alu.def.bit_size() == 32);
                b.mufu(MuFuOp::Rcp, srcs[0])
            }
            nir_op_frsq => {
                assert!(alu.def.bit_size() == 32);
                b.mufu(MuFuOp::Rsq, srcs[0])
            }
            nir_op_fsat => {
                assert!(alu.def.bit_size() == 32);
                if self.alu_src_is_saturated(&alu.srcs_as_slice()[0]) {
                    b.copy(srcs[0])
                } else {
                    let ftype = FloatType::from_bits(alu.def.bit_size().into());
                    let dst = b.alloc_ssa(RegFile::GPR, 1);
                    b.push_op(OpFAdd {
                        dst: dst.into(),
                        srcs: [srcs[0], 0.into()],
                        saturate: true,
                        rnd_mode: self.float_ctl[ftype].rnd_mode,
                        ftz: self.float_ctl[ftype].ftz,
                    });
                    dst
                }
            }
            nir_op_fsign => {
                if alu.def.bit_size() == 64 {
                    let lz = b.dsetp(FloatCmpOp::OrdLt, srcs[0], 0.into());
                    let gz = b.dsetp(FloatCmpOp::OrdGt, srcs[0], 0.into());
                    let hi = b.sel(lz.into(), 0xbff00000.into(), 0.into());
                    let hi = b.sel(gz.into(), 0x3ff00000.into(), hi.into());
                    let lo = b.copy(0.into());
                    [lo[0], hi[0]].into()
                } else if alu.def.bit_size() == 32 {
                    let lz = b.fset(FloatCmpOp::OrdLt, srcs[0], 0.into());
                    let gz = b.fset(FloatCmpOp::OrdGt, srcs[0], 0.into());
                    b.fadd(gz.into(), Src::from(lz).fneg())
                } else {
                    panic!("Unsupported float type: f{}", alu.def.bit_size());
                }
            }
            nir_op_fsin => {
                let frac_1_2pi = 1.0 / (2.0 * std::f32::consts::PI);
                let tmp = b.fmul(srcs[0], frac_1_2pi.into());
                b.mufu(MuFuOp::Sin, tmp.into())
            }
            nir_op_fsqrt => b.mufu(MuFuOp::Sqrt, srcs[0]),
            nir_op_i2f16 | nir_op_i2f32 | nir_op_i2f64 => {
                let src_bits = alu.get_src(0).src.bit_size();
                let dst_bits = alu.def.bit_size();
                let dst_type = FloatType::from_bits(dst_bits.into());
                let dst = b.alloc_ssa(RegFile::GPR, dst_bits.div_ceil(32));
                b.push_op(OpI2F {
                    dst: dst.into(),
                    src: srcs[0],
                    dst_type: dst_type,
                    src_type: IntType::from_bits(src_bits.into(), true),
                    rnd_mode: self.float_ctl[dst_type].rnd_mode,
                });
                dst
            }
            nir_op_i2i8 | nir_op_i2i16 | nir_op_i2i32 | nir_op_i2i64
            | nir_op_u2u8 | nir_op_u2u16 | nir_op_u2u32 | nir_op_u2u64 => {
                let src_bits = alu.get_src(0).src.bit_size();
                let dst_bits = alu.def.bit_size();

                let mut prmt = [0_u8; 8];
                match alu.op {
                    nir_op_i2i8 | nir_op_i2i16 | nir_op_i2i32
                    | nir_op_i2i64 => {
                        let sign = ((src_bits / 8) - 1) | 0x8;
                        for i in 0..8 {
                            if i < (src_bits / 8) {
                                prmt[usize::from(i)] = i;
                            } else {
                                prmt[usize::from(i)] = sign;
                            }
                        }
                    }
                    nir_op_u2u8 | nir_op_u2u16 | nir_op_u2u32
                    | nir_op_u2u64 => {
                        for i in 0..8 {
                            if i < (src_bits / 8) {
                                prmt[usize::from(i)] = i;
                            } else {
                                prmt[usize::from(i)] = 4;
                            }
                        }
                    }
                    _ => panic!("Invalid integer conversion: {}", alu.op),
                }
                let prmt_lo: [u8; 4] = prmt[0..4].try_into().unwrap();
                let prmt_hi: [u8; 4] = prmt[4..8].try_into().unwrap();

                let src = srcs[0].as_ssa().unwrap();
                if src_bits == 64 {
                    if dst_bits == 64 {
                        *src
                    } else {
                        b.prmt(src[0].into(), src[1].into(), prmt_lo)
                    }
                } else {
                    if dst_bits == 64 {
                        let lo = b.prmt(src[0].into(), 0.into(), prmt_lo);
                        let hi = b.prmt(src[0].into(), 0.into(), prmt_hi);
                        [lo[0], hi[0]].into()
                    } else {
                        b.prmt(src[0].into(), 0.into(), prmt_lo)
                    }
                }
            }
            nir_op_iabs => b.iabs(srcs[0]),
            nir_op_iadd => match alu.def.bit_size {
                32 => b.iadd(srcs[0], srcs[1]),
                64 => b.iadd64(srcs[0], srcs[1]),
                x => panic!("unsupported bit size for nir_op_iadd: {x}"),
            },
            nir_op_iand => b.lop2(LogicOp2::And, srcs[0], srcs[1]),
            nir_op_ieq => {
                if alu.get_src(0).bit_size() == 1 {
                    b.lop2(LogicOp2::Xor, srcs[0], srcs[1].bnot())
                } else if alu.get_src(0).bit_size() == 64 {
                    b.isetp64(IntCmpType::I32, IntCmpOp::Eq, srcs[0], srcs[1])
                } else {
                    assert!(alu.get_src(0).bit_size() == 32);
                    b.isetp(IntCmpType::I32, IntCmpOp::Eq, srcs[0], srcs[1])
                }
            }
            nir_op_ifind_msb | nir_op_ufind_msb => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFlo {
                    dst: dst.into(),
                    src: srcs[0],
                    signed: alu.op == nir_op_ifind_msb,
                    return_shift_amount: false,
                });
                dst
            }
            nir_op_ige | nir_op_ilt | nir_op_uge | nir_op_ult => {
                let x = *srcs[0].as_ssa().unwrap();
                let y = *srcs[1].as_ssa().unwrap();
                let (cmp_type, cmp_op) = match alu.op {
                    nir_op_ige => (IntCmpType::I32, IntCmpOp::Ge),
                    nir_op_ilt => (IntCmpType::I32, IntCmpOp::Lt),
                    nir_op_uge => (IntCmpType::U32, IntCmpOp::Ge),
                    nir_op_ult => (IntCmpType::U32, IntCmpOp::Lt),
                    _ => panic!("Not an integer comparison"),
                };
                if alu.get_src(0).bit_size() == 64 {
                    b.isetp64(cmp_type, cmp_op, x.into(), y.into())
                } else {
                    assert!(alu.get_src(0).bit_size() == 32);
                    b.isetp(cmp_type, cmp_op, x.into(), y.into())
                }
            }
            nir_op_imax | nir_op_imin | nir_op_umax | nir_op_umin => {
                let (tp, min) = match alu.op {
                    nir_op_imax => (IntCmpType::I32, SrcRef::False),
                    nir_op_imin => (IntCmpType::I32, SrcRef::True),
                    nir_op_umax => (IntCmpType::U32, SrcRef::False),
                    nir_op_umin => (IntCmpType::U32, SrcRef::True),
                    _ => panic!("Not an integer min/max"),
                };
                assert!(alu.def.bit_size() == 32);
                b.imnmx(tp, srcs[0], srcs[1], min.into())
            }
            nir_op_imul => {
                assert!(alu.def.bit_size() == 32);
                b.imul(srcs[0], srcs[1])
            }
            nir_op_imul_2x32_64 | nir_op_umul_2x32_64 => {
                let signed = alu.op == nir_op_imul_2x32_64;
                b.imul_2x32_64(srcs[0], srcs[1], signed)
            }
            nir_op_imul_high | nir_op_umul_high => {
                let signed = alu.op == nir_op_imul_high;
                let dst64 = b.imul_2x32_64(srcs[0], srcs[1], signed);
                dst64[1].into()
            }
            nir_op_ine => {
                if alu.get_src(0).bit_size() == 1 {
                    b.lop2(LogicOp2::Xor, srcs[0], srcs[1])
                } else if alu.get_src(0).bit_size() == 64 {
                    b.isetp64(IntCmpType::I32, IntCmpOp::Ne, srcs[0], srcs[1])
                } else {
                    assert!(alu.get_src(0).bit_size() == 32);
                    b.isetp(IntCmpType::I32, IntCmpOp::Ne, srcs[0], srcs[1])
                }
            }
            nir_op_ineg => {
                if alu.def.bit_size == 64 {
                    let x = srcs[0].as_ssa().unwrap();
                    let sum = b.alloc_ssa(RegFile::GPR, 2);
                    let carry = b.alloc_ssa(RegFile::Pred, 1);
                    b.push_op(OpIAdd3 {
                        dst: sum[0].into(),
                        overflow: [carry.into(), Dst::None],
                        srcs: [0.into(), Src::from(x[0]).ineg(), 0.into()],
                    });
                    b.push_op(OpIAdd3X {
                        dst: sum[1].into(),
                        overflow: [Dst::None, Dst::None],
                        srcs: [0.into(), Src::from(x[1]).bnot(), 0.into()],
                        carry: [carry.into(), SrcRef::False.into()],
                    });
                    sum
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.ineg(srcs[0])
                }
            }
            nir_op_inot => {
                if alu.def.bit_size() == 1 {
                    b.lop2(LogicOp2::PassB, true.into(), srcs[0].bnot())
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.lop2(LogicOp2::PassB, 0.into(), srcs[0].bnot())
                }
            }
            nir_op_ior => b.lop2(LogicOp2::Or, srcs[0], srcs[1]),
            nir_op_ishl => {
                let x = *srcs[0].as_ssa().unwrap();
                let shift = srcs[1];
                if alu.def.bit_size() == 64 {
                    // For 64-bit shifts, we have to use clamp mode so we need
                    // to mask the shift in order satisfy NIR semantics.
                    let shift = b.lop2(LogicOp2::And, shift, 0x3f.into());
                    let dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpShf {
                        dst: dst[0].into(),
                        low: 0.into(),
                        high: x[0].into(),
                        shift: shift.into(),
                        right: false,
                        wrap: false,
                        data_type: IntType::U32,
                        dst_high: true,
                    });
                    b.push_op(OpShf {
                        dst: dst[1].into(),
                        low: x[0].into(),
                        high: x[1].into(),
                        shift: shift.into(),
                        right: false,
                        wrap: false,
                        data_type: IntType::U64,
                        dst_high: true,
                    });
                    dst
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.shl(srcs[0], srcs[1])
                }
            }
            nir_op_ishr => {
                let x = *srcs[0].as_ssa().unwrap();
                let shift = srcs[1];
                if alu.def.bit_size() == 64 {
                    // For 64-bit shifts, we have to use clamp mode so we need
                    // to mask the shift in order satisfy NIR semantics.
                    let shift = b.lop2(LogicOp2::And, shift, 0x3f.into());
                    let dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpShf {
                        dst: dst[0].into(),
                        low: x[0].into(),
                        high: x[1].into(),
                        shift: shift.into(),
                        right: true,
                        wrap: false,
                        data_type: IntType::I64,
                        dst_high: false,
                    });
                    b.push_op(OpShf {
                        dst: dst[1].into(),
                        low: x[0].into(),
                        high: x[1].into(),
                        shift: shift.into(),
                        right: true,
                        wrap: false,
                        data_type: IntType::I32,
                        dst_high: true,
                    });
                    dst
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.shr(srcs[0], srcs[1], true)
                }
            }
            nir_op_ixor => b.lop2(LogicOp2::Xor, srcs[0], srcs[1]),
            nir_op_pack_half_2x16_split => {
                assert!(alu.get_src(0).bit_size() == 32);
                let low = b.alloc_ssa(RegFile::GPR, 1);
                let high = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpF2F {
                    dst: low.into(),
                    src: srcs[0],
                    src_type: FloatType::F32,
                    dst_type: FloatType::F16,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: false,
                    high: false,
                });

                let src_bits = usize::from(alu.get_src(1).bit_size());
                let src_type = FloatType::from_bits(src_bits);
                assert!(matches!(src_type, FloatType::F32));
                b.push_op(OpF2F {
                    dst: high.into(),
                    src: srcs[1],
                    src_type: FloatType::F32,
                    dst_type: FloatType::F16,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: false,
                    high: false,
                });

                b.prmt(low.into(), high.into(), [0, 1, 4, 5])
            }
            nir_op_sdot_4x8_iadd => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIDp4 {
                    dst: dst.into(),
                    src_types: [IntType::I8, IntType::I8],
                    srcs: [srcs[0], srcs[1], srcs[2]],
                });
                dst
            }
            nir_op_sudot_4x8_iadd => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIDp4 {
                    dst: dst.into(),
                    src_types: [IntType::I8, IntType::U8],
                    srcs: [srcs[0], srcs[1], srcs[2]],
                });
                dst
            }
            nir_op_udot_4x8_uadd => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIDp4 {
                    dst: dst.into(),
                    src_types: [IntType::U8, IntType::U8],
                    srcs: [srcs[0], srcs[1], srcs[2]],
                });
                dst
            }
            nir_op_u2f16 | nir_op_u2f32 | nir_op_u2f64 => {
                let src_bits = alu.get_src(0).src.bit_size();
                let dst_bits = alu.def.bit_size();
                let dst_type = FloatType::from_bits(dst_bits.into());
                let dst = b.alloc_ssa(RegFile::GPR, dst_bits.div_ceil(32));
                b.push_op(OpI2F {
                    dst: dst.into(),
                    src: srcs[0],
                    dst_type: dst_type,
                    src_type: IntType::from_bits(src_bits.into(), false),
                    rnd_mode: self.float_ctl[dst_type].rnd_mode,
                });
                dst
            }
            nir_op_uadd_sat => {
                let x = srcs[0].as_ssa().unwrap();
                let y = srcs[1].as_ssa().unwrap();
                let sum_lo = b.alloc_ssa(RegFile::GPR, 1);
                let ovf_lo = b.alloc_ssa(RegFile::Pred, 1);
                b.push_op(OpIAdd3 {
                    dst: sum_lo.into(),
                    overflow: [ovf_lo.into(), Dst::None],
                    srcs: [0.into(), x[0].into(), y[0].into()],
                });
                if alu.def.bit_size() == 64 {
                    let sum_hi = b.alloc_ssa(RegFile::GPR, 1);
                    let ovf_hi = b.alloc_ssa(RegFile::Pred, 1);
                    b.push_op(OpIAdd3X {
                        dst: sum_hi.into(),
                        overflow: [ovf_hi.into(), Dst::None],
                        srcs: [0.into(), x[1].into(), y[1].into()],
                        carry: [ovf_lo.into(), false.into()],
                    });
                    let lo =
                        b.sel(ovf_hi.into(), u32::MAX.into(), sum_lo.into());
                    let hi =
                        b.sel(ovf_hi.into(), u32::MAX.into(), sum_hi.into());
                    [lo[0], hi[0]].into()
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.sel(ovf_lo.into(), u32::MAX.into(), sum_lo.into())
                }
            }
            nir_op_usub_sat => {
                let x = srcs[0].as_ssa().unwrap();
                let y = srcs[1].as_ssa().unwrap();
                let sum_lo = b.alloc_ssa(RegFile::GPR, 1);
                let ovf_lo = b.alloc_ssa(RegFile::Pred, 1);
                // The result of OpIAdd3X is the 33-bit value
                //
                //  s|o = x + !y + 1
                //
                // The overflow bit of this result is true if and only if the
                // subtract did NOT overflow.
                b.push_op(OpIAdd3 {
                    dst: sum_lo.into(),
                    overflow: [ovf_lo.into(), Dst::None],
                    srcs: [0.into(), x[0].into(), Src::from(y[0]).ineg()],
                });
                if alu.def.bit_size() == 64 {
                    let sum_hi = b.alloc_ssa(RegFile::GPR, 1);
                    let ovf_hi = b.alloc_ssa(RegFile::Pred, 1);
                    b.push_op(OpIAdd3X {
                        dst: sum_hi.into(),
                        overflow: [ovf_hi.into(), Dst::None],
                        srcs: [0.into(), x[1].into(), Src::from(y[1]).bnot()],
                        carry: [ovf_lo.into(), false.into()],
                    });
                    let lo = b.sel(ovf_hi.into(), sum_lo.into(), 0.into());
                    let hi = b.sel(ovf_hi.into(), sum_hi.into(), 0.into());
                    [lo[0], hi[0]].into()
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.sel(ovf_lo.into(), sum_lo.into(), 0.into())
                }
            }
            nir_op_unpack_32_2x16_split_x => {
                b.prmt(srcs[0], 0.into(), [0, 1, 4, 4])
            }
            nir_op_unpack_32_2x16_split_y => {
                b.prmt(srcs[0], 0.into(), [2, 3, 4, 4])
            }
            nir_op_unpack_64_2x32_split_x => {
                let src0_x = srcs[0].as_ssa().unwrap()[0];
                b.copy(src0_x.into())
            }
            nir_op_unpack_64_2x32_split_y => {
                let src0_y = srcs[0].as_ssa().unwrap()[1];
                b.copy(src0_y.into())
            }
            nir_op_unpack_half_2x16_split_x
            | nir_op_unpack_half_2x16_split_y => {
                assert!(alu.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpF2F {
                    dst: dst[0].into(),
                    src: srcs[0],
                    src_type: FloatType::F16,
                    dst_type: FloatType::F32,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: false,
                    high: alu.op == nir_op_unpack_half_2x16_split_y,
                });

                dst
            }
            nir_op_ushr => {
                let x = *srcs[0].as_ssa().unwrap();
                let shift = srcs[1];
                if alu.def.bit_size() == 64 {
                    // For 64-bit shifts, we have to use clamp mode so we need
                    // to mask the shift in order satisfy NIR semantics.
                    let shift = b.lop2(LogicOp2::And, shift, 0x3f.into());
                    let dst = b.alloc_ssa(RegFile::GPR, 2);
                    b.push_op(OpShf {
                        dst: dst[0].into(),
                        low: x[0].into(),
                        high: x[1].into(),
                        shift: shift.into(),
                        right: true,
                        wrap: false,
                        data_type: IntType::U64,
                        dst_high: false,
                    });
                    b.push_op(OpShf {
                        dst: dst[1].into(),
                        low: x[0].into(),
                        high: x[1].into(),
                        shift: shift.into(),
                        right: true,
                        wrap: false,
                        data_type: IntType::U32,
                        dst_high: true,
                    });
                    dst
                } else {
                    assert!(alu.def.bit_size() == 32);
                    b.shr(srcs[0], srcs[1], false)
                }
            }
            nir_op_fddx | nir_op_fddx_coarse | nir_op_fddx_fine => {
                // TODO: Real coarse derivatives

                assert!(alu.def.bit_size() == 32);
                let ftype = FloatType::F32;
                let scratch = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpShfl {
                    dst: scratch[0].into(),
                    in_bounds: Dst::None,
                    src: srcs[0],
                    lane: 1_u32.into(),
                    c: (0x3_u32 | 0x1c_u32 << 8).into(),
                    op: ShflOp::Bfly,
                });

                let dst = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpFSwzAdd {
                    dst: dst[0].into(),
                    srcs: [scratch[0].into(), srcs[0]],
                    ops: [
                        FSwzAddOp::SubLeft,
                        FSwzAddOp::SubRight,
                        FSwzAddOp::SubLeft,
                        FSwzAddOp::SubRight,
                    ],
                    rnd_mode: self.float_ctl[ftype].rnd_mode,
                    ftz: self.float_ctl[ftype].ftz,
                });

                dst
            }
            nir_op_fddy | nir_op_fddy_coarse | nir_op_fddy_fine => {
                // TODO: Real coarse derivatives

                assert!(alu.def.bit_size() == 32);
                let ftype = FloatType::F32;
                let scratch = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpShfl {
                    dst: scratch[0].into(),
                    in_bounds: Dst::None,
                    src: srcs[0],
                    lane: 2_u32.into(),
                    c: (0x3_u32 | 0x1c_u32 << 8).into(),
                    op: ShflOp::Bfly,
                });

                let dst = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpFSwzAdd {
                    dst: dst[0].into(),
                    srcs: [scratch[0].into(), srcs[0]],
                    ops: [
                        FSwzAddOp::SubLeft,
                        FSwzAddOp::SubLeft,
                        FSwzAddOp::SubRight,
                        FSwzAddOp::SubRight,
                    ],
                    rnd_mode: self.float_ctl[ftype].rnd_mode,
                    ftz: self.float_ctl[ftype].ftz,
                });

                dst
            }
            _ => panic!("Unsupported ALU instruction: {}", alu.info().name()),
        };
        self.set_dst(&alu.def, dst);
    }

    fn parse_jump(&mut self, _b: &mut impl SSABuilder, _jump: &nir_jump_instr) {
        // Nothing to do
    }

    fn parse_tex(&mut self, b: &mut impl SSABuilder, tex: &nir_tex_instr) {
        let dim = match tex.sampler_dim {
            GLSL_SAMPLER_DIM_1D => {
                if tex.is_array {
                    TexDim::Array1D
                } else {
                    TexDim::_1D
                }
            }
            GLSL_SAMPLER_DIM_2D => {
                if tex.is_array {
                    TexDim::Array2D
                } else {
                    TexDim::_2D
                }
            }
            GLSL_SAMPLER_DIM_3D => {
                assert!(!tex.is_array);
                TexDim::_3D
            }
            GLSL_SAMPLER_DIM_CUBE => {
                if tex.is_array {
                    TexDim::ArrayCube
                } else {
                    TexDim::Cube
                }
            }
            GLSL_SAMPLER_DIM_BUF => TexDim::_1D,
            GLSL_SAMPLER_DIM_MS => {
                if tex.is_array {
                    TexDim::Array2D
                } else {
                    TexDim::_2D
                }
            }
            _ => panic!("Unsupported texture dimension: {}", tex.sampler_dim),
        };

        let srcs = tex.srcs_as_slice();
        assert!(srcs[0].src_type == nir_tex_src_backend1);
        if srcs.len() > 1 {
            assert!(srcs.len() == 2);
            assert!(srcs[1].src_type == nir_tex_src_backend2);
        }

        let flags: nak_nir_tex_flags =
            unsafe { std::mem::transmute_copy(&tex.backend_flags) };

        let mask = tex.def.components_read();
        let mask = u8::try_from(mask).unwrap();

        let dst_comps = u8::try_from(mask.count_ones()).unwrap();
        let dst = b.alloc_ssa(RegFile::GPR, dst_comps);

        // On Volta and later, the destination is split in two
        let mut dsts = [Dst::None; 2];
        if dst_comps > 2 && b.sm() >= 70 {
            dsts[0] = SSARef::try_from(&dst[0..2]).unwrap().into();
            dsts[1] = SSARef::try_from(&dst[2..]).unwrap().into();
        } else {
            dsts[0] = dst.into();
        }

        if tex.op == nir_texop_hdr_dim_nv {
            let src = self.get_src(&srcs[0].src);
            b.push_op(OpTxq {
                dsts: dsts,
                src: src,
                query: TexQuery::Dimension,
                mask: mask,
            });
        } else if tex.op == nir_texop_tex_type_nv {
            let src = self.get_src(&srcs[0].src);
            b.push_op(OpTxq {
                dsts: dsts,
                src: src,
                query: TexQuery::TextureType,
                mask: mask,
            });
        } else {
            let lod_mode = match flags.lod_mode() {
                NAK_NIR_LOD_MODE_AUTO => TexLodMode::Auto,
                NAK_NIR_LOD_MODE_ZERO => TexLodMode::Zero,
                NAK_NIR_LOD_MODE_BIAS => TexLodMode::Bias,
                NAK_NIR_LOD_MODE_LOD => TexLodMode::Lod,
                NAK_NIR_LOD_MODE_CLAMP => TexLodMode::Clamp,
                NAK_NIR_LOD_MODE_BIAS_CLAMP => TexLodMode::BiasClamp,
                _ => panic!("Invalid LOD mode"),
            };

            let offset_mode = match flags.offset_mode() {
                NAK_NIR_OFFSET_MODE_NONE => Tld4OffsetMode::None,
                NAK_NIR_OFFSET_MODE_AOFFI => Tld4OffsetMode::AddOffI,
                NAK_NIR_OFFSET_MODE_PER_PX => Tld4OffsetMode::PerPx,
                _ => panic!("Invalid offset mode"),
            };

            let srcs = [self.get_src(&srcs[0].src), self.get_src(&srcs[1].src)];

            if tex.op == nir_texop_txd {
                assert!(lod_mode == TexLodMode::Auto);
                assert!(offset_mode != Tld4OffsetMode::PerPx);
                assert!(!flags.has_z_cmpr());
                b.push_op(OpTxd {
                    dsts: dsts,
                    resident: Dst::None,
                    srcs: srcs,
                    dim: dim,
                    offset: offset_mode == Tld4OffsetMode::AddOffI,
                    mask: mask,
                });
            } else if tex.op == nir_texop_lod {
                assert!(offset_mode == Tld4OffsetMode::None);
                b.push_op(OpTmml {
                    dsts: dsts,
                    srcs: srcs,
                    dim: dim,
                    mask: mask,
                });
            } else if tex.op == nir_texop_txf || tex.op == nir_texop_txf_ms {
                assert!(offset_mode != Tld4OffsetMode::PerPx);
                b.push_op(OpTld {
                    dsts: dsts,
                    resident: Dst::None,
                    srcs: srcs,
                    dim: dim,
                    lod_mode: lod_mode,
                    is_ms: tex.op == nir_texop_txf_ms,
                    offset: offset_mode == Tld4OffsetMode::AddOffI,
                    mask: mask,
                });
            } else if tex.op == nir_texop_tg4 {
                b.push_op(OpTld4 {
                    dsts: dsts,
                    resident: Dst::None,
                    srcs: srcs,
                    dim: dim,
                    comp: tex.component().try_into().unwrap(),
                    offset_mode: offset_mode,
                    z_cmpr: flags.has_z_cmpr(),
                    mask: mask,
                });
            } else {
                assert!(offset_mode != Tld4OffsetMode::PerPx);
                b.push_op(OpTex {
                    dsts: dsts,
                    resident: Dst::None,
                    srcs: srcs,
                    dim: dim,
                    lod_mode: lod_mode,
                    z_cmpr: flags.has_z_cmpr(),
                    offset: offset_mode == Tld4OffsetMode::AddOffI,
                    mask: mask,
                });
            }
        }

        let mut di = 0_usize;
        let mut nir_dst = Vec::new();
        for i in 0..tex.def.num_components() {
            if mask & (1 << i) == 0 {
                nir_dst.push(b.copy(0.into())[0]);
            } else {
                nir_dst.push(dst[di].into());
                di += 1;
            }
        }
        self.set_ssa(&tex.def.as_def(), nir_dst);
    }

    fn get_atomic_type(&self, intrin: &nir_intrinsic_instr) -> AtomType {
        let bit_size = intrin.def.bit_size();
        match intrin.atomic_op() {
            nir_atomic_op_iadd => AtomType::U(bit_size),
            nir_atomic_op_imin => AtomType::I(bit_size),
            nir_atomic_op_umin => AtomType::U(bit_size),
            nir_atomic_op_imax => AtomType::I(bit_size),
            nir_atomic_op_umax => AtomType::U(bit_size),
            nir_atomic_op_iand => AtomType::U(bit_size),
            nir_atomic_op_ior => AtomType::U(bit_size),
            nir_atomic_op_ixor => AtomType::U(bit_size),
            nir_atomic_op_xchg => AtomType::U(bit_size),
            nir_atomic_op_fadd => AtomType::F(bit_size),
            nir_atomic_op_fmin => AtomType::F(bit_size),
            nir_atomic_op_fmax => AtomType::F(bit_size),
            nir_atomic_op_cmpxchg => AtomType::U(bit_size),
            _ => panic!("Unsupported NIR atomic op"),
        }
    }

    fn get_atomic_op(&self, intrin: &nir_intrinsic_instr) -> AtomOp {
        match intrin.atomic_op() {
            nir_atomic_op_iadd => AtomOp::Add,
            nir_atomic_op_imin => AtomOp::Min,
            nir_atomic_op_umin => AtomOp::Min,
            nir_atomic_op_imax => AtomOp::Max,
            nir_atomic_op_umax => AtomOp::Max,
            nir_atomic_op_iand => AtomOp::And,
            nir_atomic_op_ior => AtomOp::Or,
            nir_atomic_op_ixor => AtomOp::Xor,
            nir_atomic_op_xchg => AtomOp::Exch,
            nir_atomic_op_fadd => AtomOp::Add,
            nir_atomic_op_fmin => AtomOp::Min,
            nir_atomic_op_fmax => AtomOp::Max,
            nir_atomic_op_cmpxchg => AtomOp::CmpExch,
            _ => panic!("Unsupported NIR atomic op"),
        }
    }

    fn get_eviction_priority(
        &mut self,
        access: gl_access_qualifier,
    ) -> MemEvictionPriority {
        if self.info.sm >= 70 && access & ACCESS_NON_TEMPORAL != 0 {
            MemEvictionPriority::First
        } else {
            MemEvictionPriority::Normal
        }
    }

    fn get_image_dim(&mut self, intrin: &nir_intrinsic_instr) -> ImageDim {
        let is_array = intrin.image_array();
        let image_dim = intrin.image_dim();
        match intrin.image_dim() {
            GLSL_SAMPLER_DIM_1D => {
                if is_array {
                    ImageDim::_1DArray
                } else {
                    ImageDim::_1D
                }
            }
            GLSL_SAMPLER_DIM_2D => {
                if is_array {
                    ImageDim::_2DArray
                } else {
                    ImageDim::_2D
                }
            }
            GLSL_SAMPLER_DIM_3D => {
                assert!(!is_array);
                ImageDim::_3D
            }
            GLSL_SAMPLER_DIM_CUBE => ImageDim::_2DArray,
            GLSL_SAMPLER_DIM_BUF => {
                assert!(!is_array);
                ImageDim::_1DBuffer
            }
            _ => panic!("Unsupported image dimension: {}", image_dim),
        }
    }

    fn get_image_coord(
        &mut self,
        intrin: &nir_intrinsic_instr,
        dim: ImageDim,
    ) -> Src {
        let vec = self.get_ssa(intrin.get_src(1).as_def());
        // let sample = self.get_src(&srcs[2]);
        let comps = usize::from(dim.coord_comps());
        SSARef::try_from(&vec[0..comps]).unwrap().into()
    }

    fn parse_intrinsic(
        &mut self,
        b: &mut impl SSABuilder,
        intrin: &nir_intrinsic_instr,
    ) {
        let srcs = intrin.srcs_as_slice();
        match intrin.intrinsic {
            nir_intrinsic_al2p_nv => {
                let offset = self.get_src(&srcs[0]);
                let addr = u16::try_from(intrin.base()).unwrap();

                let flags = intrin.flags();
                let flags: nak_nir_attr_io_flags =
                    unsafe { std::mem::transmute_copy(&flags) };

                let access = AttrAccess {
                    addr: addr,
                    comps: 1,
                    patch: flags.patch(),
                    output: flags.output(),
                    phys: false,
                };

                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpAL2P {
                    dst: dst.into(),
                    offset: offset,
                    access: access,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_ald_nv | nir_intrinsic_ast_nv => {
                let addr = u16::try_from(intrin.base()).unwrap();
                let base = u16::try_from(intrin.range_base()).unwrap();
                let range = u16::try_from(intrin.range()).unwrap();
                let range = base..(base + range);

                let flags = intrin.flags();
                let flags: nak_nir_attr_io_flags =
                    unsafe { std::mem::transmute_copy(&flags) };
                assert!(!flags.patch() || !flags.phys());

                if let ShaderIoInfo::Vtg(io) = &mut self.info.io {
                    if flags.patch() {
                        match &mut self.info.stage {
                            ShaderStageInfo::TessellationInit(stage) => {
                                assert!(flags.output());
                                stage.per_patch_attribute_count = max(
                                    stage.per_patch_attribute_count,
                                    (range.end / 4).try_into().unwrap(),
                                );
                            }
                            ShaderStageInfo::Tessellation => (),
                            _ => panic!("Patch I/O not supported"),
                        }
                    } else {
                        if flags.output() {
                            if intrin.intrinsic == nir_intrinsic_ast_nv {
                                io.mark_store_req(range.clone());
                            }
                            io.mark_attrs_written(range);
                        } else {
                            io.mark_attrs_read(range);
                        }
                    }
                } else {
                    panic!("Must be a VTG stage");
                }

                let access = AttrAccess {
                    addr: addr,
                    comps: intrin.num_components,
                    patch: flags.patch(),
                    output: flags.output(),
                    phys: flags.phys(),
                };

                if intrin.intrinsic == nir_intrinsic_ald_nv {
                    let vtx = self.get_src(&srcs[0]);
                    let offset = self.get_src(&srcs[1]);

                    assert!(intrin.def.bit_size() == 32);
                    let dst = b.alloc_ssa(RegFile::GPR, access.comps);
                    b.push_op(OpALd {
                        dst: dst.into(),
                        vtx: vtx,
                        offset: offset,
                        access: access,
                    });
                    self.set_dst(&intrin.def, dst);
                } else if intrin.intrinsic == nir_intrinsic_ast_nv {
                    assert!(srcs[0].bit_size() == 32);
                    let data = self.get_src(&srcs[0]);
                    let vtx = self.get_src(&srcs[1]);
                    let offset = self.get_src(&srcs[2]);

                    b.push_op(OpASt {
                        data: data,
                        vtx: vtx,
                        offset: offset,
                        access: access,
                    });
                } else {
                    panic!("Invalid VTG I/O intrinsic");
                }
            }
            nir_intrinsic_ballot => {
                assert!(srcs[0].bit_size() == 1);
                let src = self.get_src(&srcs[0]);

                assert!(intrin.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpVote {
                    op: VoteOp::Any,
                    ballot: dst.into(),
                    vote: Dst::None,
                    pred: src,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_bar_break_nv => {
                let src = self.get_src(&srcs[0]);
                let bar_in = b.bmov_to_bar(src);

                let bar_out = b.alloc_ssa(RegFile::Bar, 1);
                b.push_op(OpBreak {
                    bar_out: bar_out.into(),
                    bar_in: bar_in.into(),
                    cond: SrcRef::True.into(),
                });

                self.set_dst(&intrin.def, b.bmov_to_gpr(bar_out.into()));
            }
            nir_intrinsic_bar_set_nv => {
                let label = self.label_alloc.alloc();
                let old = self.bar_label.insert(intrin.def.index, label);
                assert!(old.is_none());

                let bar_clear = b.alloc_ssa(RegFile::Bar, 1);
                b.push_op(OpBClear {
                    dst: bar_clear.into(),
                });

                let bar_out = b.alloc_ssa(RegFile::Bar, 1);
                b.push_op(OpBSSy {
                    bar_out: bar_out.into(),
                    bar_in: bar_clear.into(),
                    cond: SrcRef::True.into(),
                    target: label,
                });

                self.set_dst(&intrin.def, b.bmov_to_gpr(bar_out.into()));
            }
            nir_intrinsic_bar_sync_nv => {
                let src = self.get_src(&srcs[0]);

                let bar = b.bmov_to_bar(src);
                b.push_op(OpBSync {
                    bar: bar.into(),
                    cond: SrcRef::True.into(),
                });

                let bar_set_idx = &srcs[1].as_def().index;
                if let Some(label) = self.bar_label.get(bar_set_idx) {
                    b.push_op(OpNop {
                        label: Some(*label),
                    });
                }
            }
            nir_intrinsic_bindless_image_atomic
            | nir_intrinsic_bindless_image_atomic_swap => {
                let handle = self.get_src(&srcs[0]);
                let dim = self.get_image_dim(intrin);
                let coord = self.get_image_coord(intrin, dim);
                // let sample = self.get_src(&srcs[2]);
                let atom_type = self.get_atomic_type(intrin);
                let atom_op = self.get_atomic_op(intrin);

                assert!(
                    intrin.def.bit_size() == 32 || intrin.def.bit_size() == 64
                );
                assert!(intrin.def.num_components() == 1);
                let dst = b.alloc_ssa(RegFile::GPR, intrin.def.bit_size() / 32);

                let data = if intrin.intrinsic
                    == nir_intrinsic_bindless_image_atomic_swap
                {
                    if intrin.def.bit_size() == 64 {
                        SSARef::from([
                            self.get_ssa(srcs[3].as_def())[0],
                            self.get_ssa(srcs[3].as_def())[1],
                            self.get_ssa(srcs[4].as_def())[0],
                            self.get_ssa(srcs[4].as_def())[1],
                        ])
                        .into()
                    } else {
                        SSARef::from([
                            self.get_ssa(srcs[3].as_def())[0],
                            self.get_ssa(srcs[4].as_def())[0],
                        ])
                        .into()
                    }
                } else {
                    self.get_src(&srcs[3])
                };

                b.push_op(OpSuAtom {
                    dst: dst.into(),
                    resident: Dst::None,
                    handle: handle,
                    coord: coord,
                    data: data,
                    atom_op: atom_op,
                    atom_type: atom_type,
                    image_dim: dim,
                    mem_order: MemOrder::Strong(MemScope::System),
                    mem_eviction_priority: self
                        .get_eviction_priority(intrin.access()),
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_bindless_image_load => {
                let handle = self.get_src(&srcs[0]);
                let dim = self.get_image_dim(intrin);
                let coord = self.get_image_coord(intrin, dim);
                // let sample = self.get_src(&srcs[2]);

                let comps = u8::try_from(intrin.num_components).unwrap();
                assert!(intrin.def.bit_size() == 32);
                assert!(comps == 1 || comps == 2 || comps == 4);

                let dst = b.alloc_ssa(RegFile::GPR, comps);

                b.push_op(OpSuLd {
                    dst: dst.into(),
                    resident: Dst::None,
                    image_dim: dim,
                    mem_order: MemOrder::Strong(MemScope::System),
                    mem_eviction_priority: self
                        .get_eviction_priority(intrin.access()),
                    mask: (1 << comps) - 1,
                    handle: handle,
                    coord: coord,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_bindless_image_store => {
                let handle = self.get_src(&srcs[0]);
                let dim = self.get_image_dim(intrin);
                let coord = self.get_image_coord(intrin, dim);
                // let sample = self.get_src(&srcs[2]);
                let data = self.get_src(&srcs[3]);

                let comps = u8::try_from(intrin.num_components).unwrap();
                assert!(srcs[3].bit_size() == 32);
                assert!(comps == 1 || comps == 2 || comps == 4);

                b.push_op(OpSuSt {
                    image_dim: dim,
                    mem_order: MemOrder::Strong(MemScope::System),
                    mem_eviction_priority: self
                        .get_eviction_priority(intrin.access()),
                    mask: (1 << comps) - 1,
                    handle: handle,
                    coord: coord,
                    data: data,
                });
            }
            nir_intrinsic_demote
            | nir_intrinsic_discard
            | nir_intrinsic_terminate => {
                if let ShaderIoInfo::Fragment(info) = &mut self.info.io {
                    info.uses_kill = true;
                } else {
                    panic!("OpKill is only available in fragment shaders");
                }
                b.push_op(OpKill {});

                if intrin.intrinsic == nir_intrinsic_terminate {
                    b.push_op(OpExit {});
                }
            }
            nir_intrinsic_demote_if
            | nir_intrinsic_discard_if
            | nir_intrinsic_terminate_if => {
                if let ShaderIoInfo::Fragment(info) = &mut self.info.io {
                    info.uses_kill = true;
                } else {
                    panic!("OpKill is only available in fragment shaders");
                }
                let cond = self.get_ssa(&srcs[0].as_def())[0];
                b.predicate(cond.into()).push_op(OpKill {});

                if intrin.intrinsic == nir_intrinsic_terminate_if {
                    b.predicate(cond.into()).push_op(OpExit {});
                }
            }
            nir_intrinsic_global_atomic => {
                let bit_size = intrin.def.bit_size();
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let data = self.get_src(&srcs[1]);
                let atom_type = self.get_atomic_type(intrin);
                let atom_op = self.get_atomic_op(intrin);

                assert!(intrin.def.num_components() == 1);
                let dst = b.alloc_ssa(RegFile::GPR, bit_size.div_ceil(32));

                b.push_op(OpAtom {
                    dst: dst.into(),
                    addr: addr,
                    cmpr: 0.into(),
                    data: data,
                    atom_op: atom_op,
                    atom_type: atom_type,
                    addr_offset: offset,
                    mem_space: MemSpace::Global(MemAddrType::A64),
                    mem_order: MemOrder::Strong(MemScope::System),
                    mem_eviction_priority: MemEvictionPriority::Normal, // Note: no intrinic access
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_global_atomic_swap => {
                assert!(intrin.atomic_op() == nir_atomic_op_cmpxchg);
                let bit_size = intrin.def.bit_size();
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let cmpr = self.get_src(&srcs[1]);
                let data = self.get_src(&srcs[2]);
                let atom_type = AtomType::U(bit_size);

                assert!(intrin.def.num_components() == 1);
                let dst = b.alloc_ssa(RegFile::GPR, bit_size.div_ceil(32));

                b.push_op(OpAtom {
                    dst: dst.into(),
                    addr: addr,
                    cmpr: cmpr,
                    data: data,
                    atom_op: AtomOp::CmpExch,
                    atom_type: atom_type,
                    addr_offset: offset,
                    mem_space: MemSpace::Global(MemAddrType::A64),
                    mem_order: MemOrder::Strong(MemScope::System),
                    mem_eviction_priority: MemEvictionPriority::Normal, // Note: no intrinic access
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_ipa_nv => {
                let addr = u16::try_from(intrin.base()).unwrap();

                let flags = intrin.flags();
                let flags: nak_nir_ipa_flags =
                    unsafe { std::mem::transmute_copy(&flags) };

                let mode = match flags.interp_mode() {
                    NAK_INTERP_MODE_PERSPECTIVE => PixelImap::Perspective,
                    NAK_INTERP_MODE_SCREEN_LINEAR => PixelImap::ScreenLinear,
                    NAK_INTERP_MODE_CONSTANT => PixelImap::Constant,
                    _ => panic!("Unsupported interp mode"),
                };

                let freq = match flags.interp_freq() {
                    NAK_INTERP_FREQ_PASS => InterpFreq::Pass,
                    NAK_INTERP_FREQ_PASS_MUL_W => InterpFreq::PassMulW,
                    NAK_INTERP_FREQ_CONSTANT => InterpFreq::Constant,
                    NAK_INTERP_FREQ_STATE => InterpFreq::State,
                    _ => panic!("Invalid interp freq"),
                };

                let loc = match flags.interp_loc() {
                    NAK_INTERP_LOC_DEFAULT => InterpLoc::Default,
                    NAK_INTERP_LOC_CENTROID => InterpLoc::Centroid,
                    NAK_INTERP_LOC_OFFSET => InterpLoc::Offset,
                    _ => panic!("Invalid interp loc"),
                };

                let inv_w = if freq == InterpFreq::PassMulW {
                    self.get_src(&srcs[0])
                } else {
                    0.into()
                };

                let offset = if loc == InterpLoc::Offset {
                    self.get_src(&srcs[1])
                } else {
                    0.into()
                };

                let ShaderIoInfo::Fragment(io) = &mut self.info.io else {
                    panic!("OpIpa is only used for fragment shaders");
                };

                io.mark_attr_read(addr, mode);

                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIpa {
                    dst: dst.into(),
                    addr: addr,
                    freq: freq,
                    loc: loc,
                    inv_w: inv_w,
                    offset: offset,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_isberd_nv => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIsberd {
                    dst: dst.into(),
                    idx: self.get_src(&srcs[0]),
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_barycentric_at_offset_nv => (),
            nir_intrinsic_load_barycentric_centroid => (),
            nir_intrinsic_load_barycentric_pixel => (),
            nir_intrinsic_load_barycentric_sample => (),
            nir_intrinsic_load_global | nir_intrinsic_load_global_constant => {
                let size_B =
                    (intrin.def.bit_size() / 8) * intrin.def.num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let order =
                    if intrin.intrinsic == nir_intrinsic_load_global_constant {
                        MemOrder::Constant
                    } else {
                        MemOrder::Strong(MemScope::System)
                    };
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Global(MemAddrType::A64),
                    order: order,
                    eviction_priority: self
                        .get_eviction_priority(intrin.access()),
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 32);
                let dst = b.alloc_ssa(RegFile::GPR, size_B.div_ceil(4));

                b.push_op(OpLd {
                    dst: dst.into(),
                    addr: addr,
                    offset: offset,
                    access: access,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_ldtram_nv => {
                let ShaderIoInfo::Fragment(io) = &mut self.info.io else {
                    panic!("ldtram_nv is only used for fragment shaders");
                };

                assert!(
                    intrin.def.bit_size() == 32
                        && intrin.def.num_components == 2
                );

                let flags = intrin.flags();
                let use_c = flags != 0;

                let addr = u16::try_from(intrin.base()).unwrap();

                io.mark_barycentric_attr_in(addr);

                let dst = b.alloc_ssa(RegFile::GPR, 2);
                b.push_op(OpLdTram {
                    dst: dst.into(),
                    addr,
                    use_c,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_sample_id => {
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpPixLd {
                    dst: dst.into(),
                    val: PixVal::MyIndex,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_sample_mask_in => {
                if let ShaderIoInfo::Fragment(info) = &mut self.info.io {
                    info.reads_sample_mask = true;
                } else {
                    panic!(
                        "sample_mask_in is only available in fragment shaders"
                    );
                }

                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpPixLd {
                    dst: dst.into(),
                    val: PixVal::CovMask,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_tess_coord_xy => {
                // Loading gl_TessCoord in tessellation evaluation shaders is
                // weird.  It's treated as a per-vertex output which is indexed
                // by LANEID.
                match &self.info.stage {
                    ShaderStageInfo::Tessellation => (),
                    _ => panic!(
                        "load_tess_coord is only available in tessellation \
                         shaders"
                    ),
                };

                assert!(intrin.def.bit_size() == 32);
                assert!(intrin.def.num_components() == 2);

                let vtx = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpS2R {
                    dst: vtx.into(),
                    idx: 0,
                });

                let access = AttrAccess {
                    addr: NAK_ATTR_TESS_COORD,
                    comps: 2,
                    patch: false,
                    output: true,
                    phys: false,
                };

                // This is recorded as a patch output in parse_shader() because
                // the hardware requires it be in the SPH, whether we use it or
                // not.

                let dst = b.alloc_ssa(RegFile::GPR, access.comps);
                b.push_op(OpALd {
                    dst: dst.into(),
                    vtx: vtx.into(),
                    offset: 0.into(),
                    access: access,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_scratch => {
                let size_B =
                    (intrin.def.bit_size() / 8) * intrin.def.num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Local,
                    order: MemOrder::Strong(MemScope::CTA),
                    eviction_priority: MemEvictionPriority::Normal,
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let dst = b.alloc_ssa(RegFile::GPR, size_B.div_ceil(4));

                b.push_op(OpLd {
                    dst: dst.into(),
                    addr: addr,
                    offset: offset,
                    access: access,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_shared => {
                let size_B =
                    (intrin.def.bit_size() / 8) * intrin.def.num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Shared,
                    order: MemOrder::Strong(MemScope::CTA),
                    eviction_priority: MemEvictionPriority::Normal,
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let offset = offset + intrin.base();
                let dst = b.alloc_ssa(RegFile::GPR, size_B.div_ceil(4));

                b.push_op(OpLd {
                    dst: dst.into(),
                    addr: addr,
                    offset: offset,
                    access: access,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_sysval_nv => {
                let idx = u8::try_from(intrin.base()).unwrap();
                debug_assert!(intrin.def.num_components == 1);
                let dst = b.alloc_ssa(RegFile::GPR, intrin.def.bit_size() / 32);
                if intrin.def.bit_size() == 32 {
                    b.push_op(OpS2R {
                        dst: dst.into(),
                        idx: idx,
                    });
                } else if intrin.def.bit_size() == 64 {
                    b.push_op(OpCS2R {
                        dst: dst.into(),
                        idx: idx,
                    });
                } else {
                    panic!("Unknown sysval_nv bit size");
                }
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_load_ubo => {
                let size_B =
                    (intrin.def.bit_size() / 8) * intrin.def.num_components();
                let idx = srcs[0];

                let (off, off_imm) = self.get_io_addr_offset(&srcs[1], 16);
                let (off, off_imm) =
                    if let Ok(off_imm_u16) = u16::try_from(off_imm) {
                        (off, off_imm_u16)
                    } else {
                        (self.get_src(&srcs[1]), 0)
                    };

                let dst = b.alloc_ssa(RegFile::GPR, size_B.div_ceil(4));

                if let Some(idx_imm) = idx.as_uint() {
                    let idx_imm: u8 = idx_imm.try_into().unwrap();
                    let cb = CBufRef {
                        buf: CBuf::Binding(idx_imm),
                        offset: off_imm,
                    };
                    if off.is_zero() {
                        for (i, comp) in dst.iter().enumerate() {
                            let i = u16::try_from(i).unwrap();
                            b.copy_to((*comp).into(), cb.offset(i * 4).into());
                        }
                    } else {
                        b.push_op(OpLdc {
                            dst: dst.into(),
                            cb: cb.into(),
                            offset: off,
                            mem_type: MemType::from_size(size_B, false),
                        });
                    }
                } else {
                    panic!("Indirect UBO indices not yet supported");
                }
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_barrier => {
                let modes = intrin.memory_modes();
                let semantics = intrin.memory_semantics();
                if (modes & nir_var_mem_global) != 0
                    && (semantics & NIR_MEMORY_RELEASE) != 0
                {
                    b.push_op(OpCCtl {
                        op: CCtlOp::WBAll,
                        mem_space: MemSpace::Global(MemAddrType::A64),
                        addr: 0.into(),
                        addr_offset: 0,
                    });
                }
                match intrin.execution_scope() {
                    SCOPE_NONE => (),
                    SCOPE_WORKGROUP => {
                        assert!(
                            self.nir.info.stage() == MESA_SHADER_COMPUTE
                                || self.nir.info.stage() == MESA_SHADER_KERNEL
                        );
                        self.info.num_barriers = 1;
                        b.push_op(OpBar {});
                    }
                    _ => panic!("Unhandled execution scope"),
                }
                if intrin.memory_scope() != SCOPE_NONE {
                    let mem_scope = match intrin.memory_scope() {
                        SCOPE_INVOCATION | SCOPE_SUBGROUP => MemScope::CTA,
                        SCOPE_WORKGROUP | SCOPE_QUEUE_FAMILY | SCOPE_DEVICE => {
                            MemScope::GPU
                        }
                        _ => panic!("Unhandled memory scope"),
                    };
                    b.push_op(OpMemBar { scope: mem_scope });
                }
                if (modes & nir_var_mem_global) != 0
                    && (semantics & NIR_MEMORY_ACQUIRE) != 0
                {
                    b.push_op(OpCCtl {
                        op: CCtlOp::IVAll,
                        mem_space: MemSpace::Global(MemAddrType::A64),
                        addr: 0.into(),
                        addr_offset: 0,
                    });
                }
            }
            nir_intrinsic_quad_broadcast
            | nir_intrinsic_read_invocation
            | nir_intrinsic_shuffle
            | nir_intrinsic_shuffle_down
            | nir_intrinsic_shuffle_up
            | nir_intrinsic_shuffle_xor => {
                assert!(srcs[0].bit_size() == 32);
                assert!(srcs[0].num_components() == 1);
                let data = self.get_src(&srcs[0]);

                assert!(srcs[1].bit_size() == 32);
                let idx = self.get_src(&srcs[1]);

                assert!(intrin.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);

                b.push_op(OpShfl {
                    dst: dst.into(),
                    in_bounds: Dst::None,
                    src: data,
                    lane: idx,
                    c: match intrin.intrinsic {
                        nir_intrinsic_quad_broadcast => 0x1c_03.into(),
                        nir_intrinsic_shuffle_up => 0.into(),
                        _ => 0x1f.into(),
                    },
                    op: match intrin.intrinsic {
                        nir_intrinsic_shuffle_down => ShflOp::Down,
                        nir_intrinsic_shuffle_up => ShflOp::Up,
                        nir_intrinsic_shuffle_xor => ShflOp::Bfly,
                        _ => ShflOp::Idx,
                    },
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_quad_swap_horizontal
            | nir_intrinsic_quad_swap_vertical
            | nir_intrinsic_quad_swap_diagonal => {
                assert!(srcs[0].bit_size() == 32);
                assert!(srcs[0].num_components() == 1);
                let data = self.get_src(&srcs[0]);

                assert!(intrin.def.bit_size() == 32);
                let dst = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpShfl {
                    dst: dst.into(),
                    in_bounds: Dst::None,
                    src: data,
                    lane: match intrin.intrinsic {
                        nir_intrinsic_quad_swap_horizontal => 1_u32.into(),
                        nir_intrinsic_quad_swap_vertical => 2_u32.into(),
                        nir_intrinsic_quad_swap_diagonal => 3_u32.into(),
                        op => panic!("Unknown quad intrinsic {}", op),
                    },
                    c: 0x1c_03.into(),
                    op: ShflOp::Bfly,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_shared_atomic => {
                let bit_size = intrin.def.bit_size();
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let data = self.get_src(&srcs[1]);
                let atom_type = self.get_atomic_type(intrin);
                let atom_op = self.get_atomic_op(intrin);

                assert!(intrin.def.num_components() == 1);
                let dst = b.alloc_ssa(RegFile::GPR, bit_size.div_ceil(32));

                b.push_op(OpAtom {
                    dst: dst.into(),
                    addr: addr,
                    cmpr: 0.into(),
                    data: data,
                    atom_op: atom_op,
                    atom_type: atom_type,
                    addr_offset: offset,
                    mem_space: MemSpace::Shared,
                    mem_order: MemOrder::Strong(MemScope::CTA),
                    mem_eviction_priority: MemEvictionPriority::Normal,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_shared_atomic_swap => {
                assert!(intrin.atomic_op() == nir_atomic_op_cmpxchg);
                let bit_size = intrin.def.bit_size();
                let (addr, offset) = self.get_io_addr_offset(&srcs[0], 24);
                let cmpr = self.get_src(&srcs[1]);
                let data = self.get_src(&srcs[2]);
                let atom_type = AtomType::U(bit_size);

                assert!(intrin.def.num_components() == 1);
                let dst = b.alloc_ssa(RegFile::GPR, bit_size.div_ceil(32));

                b.push_op(OpAtom {
                    dst: dst.into(),
                    addr: addr,
                    cmpr: cmpr,
                    data: data,
                    atom_op: AtomOp::CmpExch,
                    atom_type: atom_type,
                    addr_offset: offset,
                    mem_space: MemSpace::Shared,
                    mem_order: MemOrder::Strong(MemScope::CTA),
                    mem_eviction_priority: MemEvictionPriority::Normal,
                });
                self.set_dst(&intrin.def, dst);
            }
            nir_intrinsic_store_global => {
                let data = self.get_src(&srcs[0]);
                let size_B =
                    (srcs[0].bit_size() / 8) * srcs[0].num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Global(MemAddrType::A64),
                    order: MemOrder::Strong(MemScope::System),
                    eviction_priority: self
                        .get_eviction_priority(intrin.access()),
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[1], 32);

                b.push_op(OpSt {
                    addr: addr,
                    data: data,
                    offset: offset,
                    access: access,
                });
            }
            nir_intrinsic_store_output => {
                let ShaderIoInfo::Fragment(_) = &mut self.info.io else {
                    panic!("load_input is only used for fragment shaders");
                };
                let data = self.get_src(&srcs[0]);

                let addr = u16::try_from(intrin.base()).unwrap()
                    + u16::try_from(srcs[1].as_uint().unwrap()).unwrap()
                    + 4 * u16::try_from(intrin.component()).unwrap();
                assert!(addr % 4 == 0);

                for c in 0..usize::from(intrin.num_components) {
                    let idx = usize::from(addr / 4) + usize::from(c);
                    self.fs_out_regs[idx] = data.as_ssa().unwrap()[c];
                }
            }
            nir_intrinsic_store_scratch => {
                let data = self.get_src(&srcs[0]);
                let size_B =
                    (srcs[0].bit_size() / 8) * srcs[0].num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Local,
                    order: MemOrder::Strong(MemScope::CTA),
                    eviction_priority: MemEvictionPriority::Normal,
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[1], 24);

                b.push_op(OpSt {
                    addr: addr,
                    data: data,
                    offset: offset,
                    access: access,
                });
            }
            nir_intrinsic_store_shared => {
                let data = self.get_src(&srcs[0]);
                let size_B =
                    (srcs[0].bit_size() / 8) * srcs[0].num_components();
                assert!(u32::from(size_B) <= intrin.align());
                let access = MemAccess {
                    mem_type: MemType::from_size(size_B, false),
                    space: MemSpace::Shared,
                    order: MemOrder::Strong(MemScope::CTA),
                    eviction_priority: MemEvictionPriority::Normal,
                };
                let (addr, offset) = self.get_io_addr_offset(&srcs[1], 24);
                let offset = offset + intrin.base();

                b.push_op(OpSt {
                    addr: addr,
                    data: data,
                    offset: offset,
                    access: access,
                });
            }
            nir_intrinsic_emit_vertex_nv | nir_intrinsic_end_primitive_nv => {
                assert!(intrin.def.bit_size() == 32);
                assert!(intrin.def.num_components() == 1);

                let dst = b.alloc_ssa(RegFile::GPR, 1);
                let handle = self.get_src(&srcs[0]);
                let stream_id = intrin.stream_id();

                b.push_op(OpOut {
                    dst: dst.into(),
                    handle: handle,
                    stream: stream_id.into(),
                    out_type: if intrin.intrinsic
                        == nir_intrinsic_emit_vertex_nv
                    {
                        OutType::Emit
                    } else {
                        OutType::Cut
                    },
                });
                self.set_dst(&intrin.def, dst);
            }

            nir_intrinsic_final_primitive_nv => {
                let handle = self.get_src(&srcs[0]);

                if self.info.sm >= 70 {
                    b.push_op(OpOutFinal { handle: handle });
                }
            }
            nir_intrinsic_vote_all
            | nir_intrinsic_vote_any
            | nir_intrinsic_vote_ieq => {
                assert!(srcs[0].bit_size() == 1);
                let src = self.get_src(&srcs[0]);

                assert!(intrin.def.bit_size() == 1);
                let dst = b.alloc_ssa(RegFile::Pred, 1);

                b.push_op(OpVote {
                    op: match intrin.intrinsic {
                        nir_intrinsic_vote_all => VoteOp::All,
                        nir_intrinsic_vote_any => VoteOp::Any,
                        nir_intrinsic_vote_ieq => VoteOp::Eq,
                        _ => panic!("Unknown vote intrinsic"),
                    },
                    ballot: Dst::None,
                    vote: dst.into(),
                    pred: src,
                });
                self.set_dst(&intrin.def, dst);
            }
            _ => panic!(
                "Unsupported intrinsic instruction: {}",
                intrin.info().name()
            ),
        }
    }

    fn parse_load_const(
        &mut self,
        b: &mut impl SSABuilder,
        load_const: &nir_load_const_instr,
    ) {
        let values = &load_const.values();

        let mut dst = Vec::new();
        match load_const.def.bit_size {
            1 => {
                for c in 0..load_const.def.num_components {
                    let imm_b1 = unsafe { values[usize::from(c)].b };
                    dst.push(b.copy(imm_b1.into())[0]);
                }
            }
            8 => {
                for dw in 0..load_const.def.num_components.div_ceil(4) {
                    let mut imm_u32 = 0;
                    for b in 0..4 {
                        let c = dw * 4 + b;
                        if c < load_const.def.num_components {
                            let imm_u8 = unsafe { values[usize::from(c)].u8_ };
                            imm_u32 |= u32::from(imm_u8) << b * 8;
                        }
                    }
                    dst.push(b.copy(imm_u32.into())[0]);
                }
            }
            16 => {
                for dw in 0..load_const.def.num_components.div_ceil(2) {
                    let mut imm_u32 = 0;
                    for w in 0..2 {
                        let c = dw * 2 + w;
                        if c < load_const.def.num_components {
                            let imm_u16 =
                                unsafe { values[usize::from(c)].u16_ };
                            imm_u32 |= u32::from(imm_u16) << w * 16;
                        }
                    }
                    dst.push(b.copy(imm_u32.into())[0]);
                }
            }
            32 => {
                for c in 0..load_const.def.num_components {
                    let imm_u32 = unsafe { values[usize::from(c)].u32_ };
                    dst.push(b.copy(imm_u32.into())[0]);
                }
            }
            64 => {
                for c in 0..load_const.def.num_components {
                    let imm_u64 = unsafe { values[c as usize].u64_ };
                    dst.push(b.copy((imm_u64 as u32).into())[0]);
                    dst.push(b.copy(((imm_u64 >> 32) as u32).into())[0]);
                }
            }
            _ => panic!("Unknown bit size: {}", load_const.def.bit_size),
        }

        self.set_ssa(&load_const.def, dst);
    }

    fn parse_undef(
        &mut self,
        b: &mut impl SSABuilder,
        undef: &nir_undef_instr,
    ) {
        let dst = alloc_ssa_for_nir(b, &undef.def);
        for c in &dst {
            b.push_op(OpUndef { dst: (*c).into() });
        }
        self.set_ssa(&undef.def, dst);
    }

    fn store_fs_outputs(&mut self, b: &mut impl SSABuilder) {
        let ShaderIoInfo::Fragment(info) = &mut self.info.io else {
            return;
        };

        for i in 0..32 {
            // Assume that colors have to come a vec4 at a time
            if !self.fs_out_regs[i].is_none() {
                info.writes_color |= 0xf << (i & !3)
            }
        }
        let mask_idx = (NAK_FS_OUT_SAMPLE_MASK / 4) as usize;
        info.writes_sample_mask = !self.fs_out_regs[mask_idx].is_none();
        let depth_idx = (NAK_FS_OUT_DEPTH / 4) as usize;
        info.writes_depth = !self.fs_out_regs[depth_idx].is_none();

        let mut srcs = Vec::new();
        for i in 0..32 {
            if info.writes_color & (1 << i) != 0 {
                if self.fs_out_regs[i].is_none() {
                    srcs.push(0.into());
                } else {
                    srcs.push(self.fs_out_regs[i].into());
                }
            }
        }

        // These always come together for some reason
        if info.writes_sample_mask || info.writes_depth {
            if info.writes_sample_mask {
                srcs.push(self.fs_out_regs[mask_idx].into());
            } else {
                srcs.push(0.into());
            }
            if info.writes_depth {
                // Saturate depth writes.
                //
                // TODO: This seems wrong in light of unrestricted depth but
                // it's needed to pass CTS tests for now.
                let depth = self.fs_out_regs[depth_idx];
                let sat_depth = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFAdd {
                    dst: sat_depth.into(),
                    srcs: [depth.into(), 0.into()],
                    saturate: true,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: false,
                });
                srcs.push(sat_depth.into());
            }
        }

        b.push_op(OpFSOut { srcs: srcs });
    }

    fn parse_block<'b>(
        &mut self,
        ssa_alloc: &mut SSAValueAllocator,
        phi_map: &mut PhiAllocMap<'b>,
        nb: &nir_block,
    ) {
        let mut b = SSAInstrBuilder::new(self.info.sm, ssa_alloc);

        if nb.index == 0 && self.nir.info.shared_size > 0 {
            // The blob seems to always do a BSYNC before accessing shared
            // memory.  Perhaps this is to ensure that our allocation is
            // actually available and not in use by another thread?
            let label = self.label_alloc.alloc();
            let bar_clear = b.alloc_ssa(RegFile::Bar, 1);

            b.push_op(OpBClear {
                dst: bar_clear.into(),
            });

            let bar = b.alloc_ssa(RegFile::Bar, 1);
            b.push_op(OpBSSy {
                bar_out: bar.into(),
                bar_in: bar_clear.into(),
                cond: SrcRef::True.into(),
                target: label,
            });

            b.push_op(OpBSync {
                bar: bar.into(),
                cond: SrcRef::True.into(),
            });

            b.push_op(OpNop { label: Some(label) });
        }

        let mut phi = OpPhiDsts::new();
        for ni in nb.iter_instr_list() {
            if ni.type_ == nir_instr_type_phi {
                let np = ni.as_phi().unwrap();
                let dst = alloc_ssa_for_nir(&mut b, np.def.as_def());
                for (i, dst) in dst.iter().enumerate() {
                    let phi_id = phi_map.get_phi_id(np, i.try_into().unwrap());
                    phi.dsts.push(phi_id, (*dst).into());
                }
                self.set_ssa(np.def.as_def(), dst);
            } else {
                break;
            }
        }

        if !phi.dsts.is_empty() {
            b.push_op(phi);
        }

        for ni in nb.iter_instr_list() {
            match ni.type_ {
                nir_instr_type_alu => {
                    self.parse_alu(&mut b, ni.as_alu().unwrap())
                }
                nir_instr_type_jump => {
                    self.parse_jump(&mut b, ni.as_jump().unwrap())
                }
                nir_instr_type_tex => {
                    self.parse_tex(&mut b, ni.as_tex().unwrap())
                }
                nir_instr_type_intrinsic => {
                    self.parse_intrinsic(&mut b, ni.as_intrinsic().unwrap())
                }
                nir_instr_type_load_const => {
                    self.parse_load_const(&mut b, ni.as_load_const().unwrap())
                }
                nir_instr_type_undef => {
                    self.parse_undef(&mut b, ni.as_undef().unwrap())
                }
                nir_instr_type_phi => (),
                _ => panic!("Unsupported instruction type"),
            }
        }

        let succ = nb.successors();
        for sb in succ {
            let sb = match sb {
                Some(b) => b,
                None => continue,
            };

            let mut phi = OpPhiSrcs::new();

            for i in sb.iter_instr_list() {
                let np = match i.as_phi() {
                    Some(phi) => phi,
                    None => break,
                };

                for ps in np.iter_srcs() {
                    if ps.pred().index == nb.index {
                        let src = *self.get_src(&ps.src).as_ssa().unwrap();
                        for (i, src) in src.iter().enumerate() {
                            let phi_id =
                                phi_map.get_phi_id(np, i.try_into().unwrap());
                            phi.srcs.push(phi_id, (*src).into());
                        }
                        break;
                    }
                }
            }

            if !phi.srcs.is_empty() {
                b.push_op(phi);
            }
        }

        if let Some(ni) = nb.following_if() {
            // The fall-through edge has to come first
            self.cfg.add_edge(nb.index, ni.first_then_block().index);
            self.cfg.add_edge(nb.index, ni.first_else_block().index);

            let mut bra = Instr::new_boxed(OpBra {
                target: self.get_block_label(ni.first_else_block()),
            });

            let cond = self.get_ssa(&ni.condition.as_def())[0];
            bra.pred = cond.into();
            // This is the branch to jump to the else
            bra.pred.pred_inv = true;

            b.push_instr(bra);
        } else {
            assert!(succ[1].is_none());
            let s0 = succ[0].unwrap();
            if s0.index == self.end_block_id {
                self.store_fs_outputs(&mut b);
                b.push_op(OpExit {});
            } else {
                self.cfg.add_edge(nb.index, s0.index);
                b.push_op(OpBra {
                    target: self.get_block_label(s0),
                });
            }
        }

        let mut bb = BasicBlock::new(self.get_block_label(nb));
        bb.instrs.append(&mut b.as_vec());
        self.cfg.add_node(nb.index, bb);
    }

    fn parse_if<'b>(
        &mut self,
        ssa_alloc: &mut SSAValueAllocator,
        phi_map: &mut PhiAllocMap<'b>,
        ni: &nir_if,
    ) {
        self.parse_cf_list(ssa_alloc, phi_map, ni.iter_then_list());
        self.parse_cf_list(ssa_alloc, phi_map, ni.iter_else_list());
    }

    fn parse_loop<'b>(
        &mut self,
        ssa_alloc: &mut SSAValueAllocator,
        phi_map: &mut PhiAllocMap<'b>,
        nl: &nir_loop,
    ) {
        self.parse_cf_list(ssa_alloc, phi_map, nl.iter_body());
    }

    fn parse_cf_list<'b>(
        &mut self,
        ssa_alloc: &mut SSAValueAllocator,
        phi_map: &mut PhiAllocMap<'b>,
        list: ExecListIter<nir_cf_node>,
    ) {
        for node in list {
            match node.type_ {
                nir_cf_node_block => {
                    let nb = node.as_block().unwrap();
                    self.parse_block(ssa_alloc, phi_map, nb);
                }
                nir_cf_node_if => {
                    let ni = node.as_if().unwrap();
                    self.parse_if(ssa_alloc, phi_map, ni);
                }
                nir_cf_node_loop => {
                    let nl = node.as_loop().unwrap();
                    self.parse_loop(ssa_alloc, phi_map, nl);
                }
                _ => panic!("Invalid inner CF node type"),
            }
        }
    }

    pub fn parse_function_impl(&mut self, nfi: &nir_function_impl) -> Function {
        let mut ssa_alloc = SSAValueAllocator::new();
        self.end_block_id = nfi.end_block().index;

        let mut phi_alloc = PhiAllocator::new();
        let mut phi_map = PhiAllocMap::new(&mut phi_alloc);

        self.parse_cf_list(&mut ssa_alloc, &mut phi_map, nfi.iter_body());

        let cfg = std::mem::take(&mut self.cfg).as_cfg();
        assert!(cfg.len() > 0);
        for i in 0..cfg.len() {
            if cfg[i].falls_through() {
                assert!(cfg.succ_indices(i)[0] == i + 1);
            }
        }

        Function {
            ssa_alloc: ssa_alloc,
            phi_alloc: phi_alloc,
            blocks: cfg,
        }
    }

    pub fn parse_shader(mut self) -> Shader {
        let mut functions = Vec::new();
        for nf in self.nir.iter_functions() {
            if let Some(nfi) = nf.get_impl() {
                let f = self.parse_function_impl(nfi);
                functions.push(f);
            }
        }

        // Tessellation evaluation shaders MUST claim to read gl_TessCoord or
        // the hardware will throw an SPH error.
        match &self.info.stage {
            ShaderStageInfo::Tessellation => match &mut self.info.io {
                ShaderIoInfo::Vtg(io) => {
                    let tc = NAK_ATTR_TESS_COORD;
                    io.mark_attrs_written(tc..(tc + 8));
                }
                _ => panic!("Tessellation must have ShaderIoInfo::Vtg"),
            },
            _ => (),
        }

        Shader {
            info: self.info,
            functions: functions,
        }
    }
}

pub fn nak_shader_from_nir(ns: &nir_shader, sm: u8) -> Shader {
    ShaderFromNir::new(ns, sm).parse_shader()
}
