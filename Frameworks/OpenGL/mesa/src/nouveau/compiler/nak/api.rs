// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::from_nir::*;
use crate::ir::{ShaderIoInfo, ShaderStageInfo};
use crate::sph;

use nak_bindings::*;

use std::cmp::max;
use std::env;
use std::ffi::{CStr, CString};
use std::fmt::Write;
use std::os::raw::c_void;
use std::sync::OnceLock;

#[repr(u8)]
enum DebugFlags {
    Print,
    Serial,
    Spill,
}

pub struct Debug {
    flags: u32,
}

impl Debug {
    fn new() -> Debug {
        let debug_var = "NAK_DEBUG";
        let debug_str = match env::var(debug_var) {
            Ok(s) => s,
            Err(_) => {
                return Debug { flags: 0 };
            }
        };

        let mut flags = 0;
        for flag in debug_str.split(',') {
            match flag.trim() {
                "print" => flags |= 1 << DebugFlags::Print as u8,
                "serial" => flags |= 1 << DebugFlags::Serial as u8,
                "spill" => flags |= 1 << DebugFlags::Spill as u8,
                unk => eprintln!("Unknown NAK_DEBUG flag \"{}\"", unk),
            }
        }
        Debug { flags: flags }
    }
}

pub trait GetDebugFlags {
    fn debug_flags(&self) -> u32;

    fn print(&self) -> bool {
        self.debug_flags() & (1 << DebugFlags::Print as u8) != 0
    }

    fn serial(&self) -> bool {
        self.debug_flags() & (1 << DebugFlags::Serial as u8) != 0
    }

    fn spill(&self) -> bool {
        self.debug_flags() & (1 << DebugFlags::Spill as u8) != 0
    }
}

pub static DEBUG: OnceLock<Debug> = OnceLock::new();

impl GetDebugFlags for OnceLock<Debug> {
    fn debug_flags(&self) -> u32 {
        self.get().unwrap().flags
    }
}

#[no_mangle]
pub extern "C" fn nak_should_print_nir() -> bool {
    DEBUG.print()
}

fn nir_options(dev: &nv_device_info) -> nir_shader_compiler_options {
    let mut op: nir_shader_compiler_options = unsafe { std::mem::zeroed() };

    op.lower_fdiv = true;
    op.lower_flrp16 = true;
    op.lower_flrp32 = true;
    op.lower_flrp64 = true;
    op.lower_bitfield_extract = true;
    op.lower_bitfield_insert = true;
    op.lower_pack_half_2x16 = true;
    op.lower_pack_unorm_2x16 = true;
    op.lower_pack_snorm_2x16 = true;
    op.lower_pack_unorm_4x8 = true;
    op.lower_pack_snorm_4x8 = true;
    op.lower_unpack_half_2x16 = true;
    op.lower_unpack_unorm_2x16 = true;
    op.lower_unpack_snorm_2x16 = true;
    op.lower_unpack_unorm_4x8 = true;
    op.lower_unpack_snorm_4x8 = true;
    op.lower_insert_byte = true;
    op.lower_insert_word = true;
    op.lower_cs_local_index_to_id = true;
    op.lower_device_index_to_zero = true;
    op.lower_isign = true;
    op.lower_uadd_sat = dev.sm < 70;
    op.lower_usub_sat = dev.sm < 70;
    op.lower_iadd_sat = true; // TODO
    op.use_interpolated_input_intrinsics = true;
    op.lower_doubles_options = nir_lower_drcp
        | nir_lower_dsqrt
        | nir_lower_drsq
        | nir_lower_dtrunc
        | nir_lower_dfloor
        | nir_lower_dceil
        | nir_lower_dfract
        | nir_lower_dround_even
        | nir_lower_dsat;
    if dev.sm >= 70 {
        op.lower_doubles_options |= nir_lower_dminmax;
    }
    op.lower_int64_options = !(nir_lower_icmp64
        | nir_lower_iadd64
        | nir_lower_ineg64
        | nir_lower_shift64
        | nir_lower_imul_2x32_64
        | nir_lower_conv64);
    op.lower_ldexp = true;
    op.lower_fmod = true;
    op.lower_ffract = true;
    op.lower_fpow = true;
    op.lower_scmp = true;
    op.lower_uadd_carry = true;
    op.lower_usub_borrow = true;
    op.has_sdot_4x8 = dev.sm >= 70;
    op.has_udot_4x8 = dev.sm >= 70;
    op.has_sudot_4x8 = dev.sm >= 70;
    op.max_unroll_iterations = 32;

    // We set .ftz on f32 by default so we can support fmulz whenever the client
    // doesn't explicitly request denorms.
    op.has_fmulz_no_denorms = true;

    op
}

#[no_mangle]
pub extern "C" fn nak_compiler_create(
    dev: *const nv_device_info,
) -> *mut nak_compiler {
    assert!(!dev.is_null());
    let dev = unsafe { &*dev };

    DEBUG.get_or_init(|| Debug::new());

    let nak = Box::new(nak_compiler {
        sm: dev.sm,
        nir_options: nir_options(dev),
    });

    Box::into_raw(nak)
}

#[no_mangle]
pub extern "C" fn nak_compiler_destroy(nak: *mut nak_compiler) {
    unsafe { drop(Box::from_raw(nak)) };
}

#[no_mangle]
pub extern "C" fn nak_debug_flags(_nak: *const nak_compiler) -> u64 {
    DEBUG.debug_flags().into()
}

#[no_mangle]
pub extern "C" fn nak_nir_options(
    nak: *const nak_compiler,
) -> *const nir_shader_compiler_options {
    assert!(!nak.is_null());
    let nak = unsafe { &*nak };
    &nak.nir_options
}

#[repr(C)]
struct ShaderBin {
    bin: nak_shader_bin,
    code: Vec<u32>,
    asm: CString,
}

impl ShaderBin {
    pub fn new(info: nak_shader_info, code: Vec<u32>, asm: &str) -> ShaderBin {
        let asm = CString::new(asm)
            .expect("NAK assembly has unexpected null characters");
        let bin = nak_shader_bin {
            info: info,
            code_size: (code.len() * 4).try_into().unwrap(),
            code: code.as_ptr() as *const c_void,
            asm_str: if asm.is_empty() {
                std::ptr::null()
            } else {
                asm.as_ptr()
            },
        };
        ShaderBin {
            bin: bin,
            code: code,
            asm: asm,
        }
    }
}

#[no_mangle]
pub extern "C" fn nak_shader_bin_destroy(bin: *mut nak_shader_bin) {
    unsafe {
        _ = Box::from_raw(bin as *mut ShaderBin);
    };
}

fn eprint_hex(label: &str, data: &[u32]) {
    eprint!("{}:", label);
    for i in 0..data.len() {
        if (i % 8) == 0 {
            eprintln!("");
            eprint!(" ");
        }
        eprint!(" {:08x}", data[i]);
    }
    eprintln!("");
}

#[no_mangle]
pub extern "C" fn nak_compile_shader(
    nir: *mut nir_shader,
    dump_asm: bool,
    nak: *const nak_compiler,
    robust2_modes: nir_variable_mode,
    fs_key: *const nak_fs_key,
) -> *mut nak_shader_bin {
    unsafe { nak_postprocess_nir(nir, nak, robust2_modes, fs_key) };
    let nak = unsafe { &*nak };
    let nir = unsafe { &*nir };
    let fs_key = if fs_key.is_null() {
        None
    } else {
        Some(unsafe { &*fs_key })
    };

    let mut s = nak_shader_from_nir(nir, nak.sm);

    if DEBUG.print() {
        eprintln!("NAK IR:\n{}", &s);
    }

    s.opt_bar_prop();
    if DEBUG.print() {
        eprintln!("NAK IR after opt_bar_prop:\n{}", &s);
    }

    s.opt_copy_prop();
    if DEBUG.print() {
        eprintln!("NAK IR after opt_copy_prop:\n{}", &s);
    }

    s.opt_lop();
    if DEBUG.print() {
        eprintln!("NAK IR after opt_lop:\n{}", &s);
    }

    s.opt_dce();
    if DEBUG.print() {
        eprintln!("NAK IR after dce:\n{}", &s);
    }

    s.opt_out();
    if DEBUG.print() {
        eprintln!("NAK IR after opt_out:\n{}", &s);
    }

    s.legalize();
    if DEBUG.print() {
        eprintln!("NAK IR after legalize:\n{}", &s);
    }

    s.assign_regs();
    if DEBUG.print() {
        eprintln!("NAK IR after assign_regs:\n{}", &s);
    }

    s.lower_ineg();
    s.lower_par_copies();
    s.lower_copy_swap();
    s.opt_jump_thread();
    s.calc_instr_deps();

    if DEBUG.print() {
        eprintln!("NAK IR:\n{}", &s);
    }

    s.gather_global_mem_usage();

    let info = nak_shader_info {
        stage: nir.info.stage(),
        num_gprs: if s.info.sm >= 70 {
            max(4, s.info.num_gprs + 2)
        } else {
            max(4, s.info.num_gprs)
        },
        num_barriers: s.info.num_barriers,
        slm_size: s.info.slm_size,
        __bindgen_anon_1: match &s.info.stage {
            ShaderStageInfo::Compute(cs_info) => {
                nak_shader_info__bindgen_ty_1 {
                    cs: nak_shader_info__bindgen_ty_1__bindgen_ty_1 {
                        local_size: [
                            cs_info.local_size[0],
                            cs_info.local_size[1],
                            cs_info.local_size[2],
                        ],
                        smem_size: cs_info.smem_size,
                    },
                }
            }
            ShaderStageInfo::Fragment => {
                let fs_info = match &s.info.io {
                    ShaderIoInfo::Fragment(io) => io,
                    _ => unreachable!(),
                };

                let nir_fs_info = unsafe { &nir.info.__bindgen_anon_1.fs };
                nak_shader_info__bindgen_ty_1 {
                    fs: nak_shader_info__bindgen_ty_1__bindgen_ty_2 {
                        writes_depth: fs_info.writes_depth,
                        reads_sample_mask: fs_info.reads_sample_mask,
                        post_depth_coverage: nir_fs_info.post_depth_coverage(),
                        uses_sample_shading: nir_fs_info.uses_sample_shading(),
                        early_fragment_tests: nir_fs_info
                            .early_fragment_tests(),
                    },
                }
            }
            ShaderStageInfo::Tessellation => {
                let nir_ts_info = unsafe { &nir.info.__bindgen_anon_1.tess };
                nak_shader_info__bindgen_ty_1 {
                    ts: nak_shader_info__bindgen_ty_1__bindgen_ty_3 {
                        domain: match nir_ts_info._primitive_mode {
                            TESS_PRIMITIVE_TRIANGLES => NAK_TS_DOMAIN_TRIANGLE,
                            TESS_PRIMITIVE_QUADS => NAK_TS_DOMAIN_QUAD,
                            TESS_PRIMITIVE_ISOLINES => NAK_TS_DOMAIN_ISOLINE,
                            _ => panic!("Invalid tess_primitive_mode"),
                        },

                        spacing: match nir_ts_info.spacing() {
                            TESS_SPACING_EQUAL => NAK_TS_SPACING_INTEGER,
                            TESS_SPACING_FRACTIONAL_ODD => {
                                NAK_TS_SPACING_FRACT_ODD
                            }
                            TESS_SPACING_FRACTIONAL_EVEN => {
                                NAK_TS_SPACING_FRACT_EVEN
                            }
                            _ => panic!("Invalid gl_tess_spacing"),
                        },

                        prims: if nir_ts_info.point_mode() {
                            NAK_TS_PRIMS_POINTS
                        } else if nir_ts_info._primitive_mode
                            == TESS_PRIMITIVE_ISOLINES
                        {
                            NAK_TS_PRIMS_LINES
                        } else if nir_ts_info.ccw() {
                            NAK_TS_PRIMS_TRIANGLES_CCW
                        } else {
                            NAK_TS_PRIMS_TRIANGLES_CW
                        },
                    },
                }
            }
            _ => nak_shader_info__bindgen_ty_1 { dummy: 0 },
        },
        vtg: match &s.info.stage {
            ShaderStageInfo::Geometry(_)
            | ShaderStageInfo::Tessellation
            | ShaderStageInfo::Vertex => {
                let writes_layer =
                    nir.info.outputs_written & (1 << VARYING_SLOT_LAYER) != 0;
                let num_clip = nir.info.clip_distance_array_size();
                let num_cull = nir.info.cull_distance_array_size();
                let clip_enable = (1_u32 << num_clip) - 1;
                let cull_enable = ((1_u32 << num_cull) - 1) << num_clip;
                nak_shader_info__bindgen_ty_2 {
                    writes_layer: writes_layer,
                    clip_enable: clip_enable.try_into().unwrap(),
                    cull_enable: cull_enable.try_into().unwrap(),
                    xfb: unsafe { nak_xfb_from_nir(nir.xfb_info) },
                }
            }
            _ => unsafe { std::mem::zeroed() },
        },
        hdr: sph::encode_header(&s.info, fs_key),
    };

    let mut asm = String::new();
    if dump_asm {
        write!(asm, "{}", s).expect("Failed to dump assembly");
    }

    let code = if nak.sm >= 70 {
        s.encode_sm70()
    } else if nak.sm >= 50 {
        s.encode_sm50()
    } else {
        panic!("Unsupported shader model");
    };

    if DEBUG.print() {
        let stage_name = unsafe {
            let c_name = _mesa_shader_stage_to_string(info.stage as u32);
            CStr::from_ptr(c_name).to_str().expect("Invalid UTF-8")
        };
        let instruction_count = if nak.sm >= 70 {
            code.len() / 4
        } else if nak.sm >= 50 {
            (code.len() / 8) * 3
        } else {
            unreachable!()
        };

        eprintln!("Stage: {}", stage_name);
        eprintln!("Instruction count: {}", instruction_count);
        eprintln!("Num GPRs: {}", info.num_gprs);
        eprintln!("SLM size: {}", info.slm_size);

        if info.stage != MESA_SHADER_COMPUTE {
            eprint_hex("Header", &info.hdr);
        }

        eprint_hex("Encoded shader", &code);
    }

    let bin = Box::new(ShaderBin::new(info, code, &asm));
    Box::into_raw(bin) as *mut nak_shader_bin
}
