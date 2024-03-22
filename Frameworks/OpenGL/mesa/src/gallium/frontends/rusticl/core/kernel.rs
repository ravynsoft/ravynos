use crate::api::icd::*;
use crate::core::device::*;
use crate::core::event::*;
use crate::core::memory::*;
use crate::core::program::*;
use crate::core::queue::*;
use crate::impl_cl_type_trait;

use mesa_rust::compiler::clc::*;
use mesa_rust::compiler::nir::*;
use mesa_rust::nir_pass;
use mesa_rust::pipe::context::RWFlags;
use mesa_rust::pipe::context::ResourceMapType;
use mesa_rust::pipe::resource::*;
use mesa_rust::pipe::screen::ResourceType;
use mesa_rust_gen::*;
use mesa_rust_util::math::*;
use mesa_rust_util::serialize::*;
use rusticl_opencl_gen::*;

use std::cell::RefCell;
use std::cmp;
use std::collections::HashMap;
use std::convert::TryInto;
use std::os::raw::c_void;
use std::ptr;
use std::slice;
use std::sync::Arc;

// ugh, we are not allowed to take refs, so...
#[derive(Clone)]
pub enum KernelArgValue {
    None,
    Constant(Vec<u8>),
    MemObject(Arc<Mem>),
    Sampler(Arc<Sampler>),
    LocalMem(usize),
}

#[derive(Hash, PartialEq, Eq, Clone, Copy)]
pub enum KernelArgType {
    Constant = 0, // for anything passed by value
    Image = 1,
    RWImage = 2,
    Sampler = 3,
    Texture = 4,
    MemGlobal = 5,
    MemConstant = 6,
    MemLocal = 7,
}

#[derive(Hash, PartialEq, Eq, Clone)]
pub enum InternalKernelArgType {
    ConstantBuffer,
    GlobalWorkOffsets,
    PrintfBuffer,
    InlineSampler((cl_addressing_mode, cl_filter_mode, bool)),
    FormatArray,
    OrderArray,
    WorkDim,
}

#[derive(Hash, PartialEq, Eq, Clone)]
pub struct KernelArg {
    spirv: spirv::SPIRVKernelArg,
    pub kind: KernelArgType,
    pub size: usize,
    /// The offset into the input buffer
    pub offset: usize,
    /// The actual binding slot
    pub binding: u32,
    pub dead: bool,
}

#[derive(Hash, PartialEq, Eq, Clone)]
pub struct InternalKernelArg {
    pub kind: InternalKernelArgType,
    pub size: usize,
    pub offset: usize,
}

impl KernelArg {
    fn from_spirv_nir(spirv: &[spirv::SPIRVKernelArg], nir: &mut NirShader) -> Vec<Self> {
        let nir_arg_map: HashMap<_, _> = nir
            .variables_with_mode(
                nir_variable_mode::nir_var_uniform | nir_variable_mode::nir_var_image,
            )
            .map(|v| (v.data.location, v))
            .collect();
        let mut res = Vec::new();

        for (i, s) in spirv.iter().enumerate() {
            let nir = nir_arg_map.get(&(i as i32)).unwrap();
            let kind = match s.address_qualifier {
                clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_PRIVATE => {
                    if unsafe { glsl_type_is_sampler(nir.type_) } {
                        KernelArgType::Sampler
                    } else {
                        KernelArgType::Constant
                    }
                }
                clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_CONSTANT => {
                    KernelArgType::MemConstant
                }
                clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_LOCAL => {
                    KernelArgType::MemLocal
                }
                clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_GLOBAL => {
                    if unsafe { glsl_type_is_image(nir.type_) } {
                        let access = nir.data.access();
                        if access == gl_access_qualifier::ACCESS_NON_WRITEABLE.0 {
                            KernelArgType::Texture
                        } else if access == gl_access_qualifier::ACCESS_NON_READABLE.0 {
                            KernelArgType::Image
                        } else {
                            KernelArgType::RWImage
                        }
                    } else {
                        KernelArgType::MemGlobal
                    }
                }
            };

            res.push(Self {
                spirv: s.clone(),
                size: unsafe { glsl_get_cl_size(nir.type_) } as usize,
                // we'll update it later in the 2nd pass
                kind: kind,
                offset: 0,
                binding: 0,
                dead: true,
            });
        }
        res
    }

    fn assign_locations(
        args: &mut [Self],
        internal_args: &mut [InternalKernelArg],
        nir: &mut NirShader,
    ) {
        for var in nir.variables_with_mode(
            nir_variable_mode::nir_var_uniform | nir_variable_mode::nir_var_image,
        ) {
            if let Some(arg) = args.get_mut(var.data.location as usize) {
                arg.offset = var.data.driver_location as usize;
                arg.binding = var.data.binding;
                arg.dead = false;
            } else {
                internal_args
                    .get_mut(var.data.location as usize - args.len())
                    .unwrap()
                    .offset = var.data.driver_location as usize;
            }
        }
    }

    fn serialize(&self) -> Vec<u8> {
        let mut bin = Vec::new();

        bin.append(&mut self.spirv.serialize());
        bin.extend_from_slice(&self.size.to_ne_bytes());
        bin.extend_from_slice(&self.offset.to_ne_bytes());
        bin.extend_from_slice(&self.binding.to_ne_bytes());
        bin.extend_from_slice(&(self.dead as u8).to_ne_bytes());
        bin.extend_from_slice(&(self.kind as u8).to_ne_bytes());

        bin
    }

    fn deserialize(bin: &mut &[u8]) -> Option<Self> {
        let spirv = spirv::SPIRVKernelArg::deserialize(bin)?;
        let size = read_ne_usize(bin);
        let offset = read_ne_usize(bin);
        let binding = read_ne_u32(bin);
        let dead = read_ne_u8(bin) == 1;

        let kind = match read_ne_u8(bin) {
            0 => KernelArgType::Constant,
            1 => KernelArgType::Image,
            2 => KernelArgType::RWImage,
            3 => KernelArgType::Sampler,
            4 => KernelArgType::Texture,
            5 => KernelArgType::MemGlobal,
            6 => KernelArgType::MemConstant,
            7 => KernelArgType::MemLocal,
            _ => return None,
        };

        Some(Self {
            spirv: spirv,
            kind: kind,
            size: size,
            offset: offset,
            binding: binding,
            dead: dead,
        })
    }
}

impl InternalKernelArg {
    fn serialize(&self) -> Vec<u8> {
        let mut bin = Vec::new();

        bin.extend_from_slice(&self.size.to_ne_bytes());
        bin.extend_from_slice(&self.offset.to_ne_bytes());

        match self.kind {
            InternalKernelArgType::ConstantBuffer => bin.push(0),
            InternalKernelArgType::GlobalWorkOffsets => bin.push(1),
            InternalKernelArgType::PrintfBuffer => bin.push(2),
            InternalKernelArgType::InlineSampler((addr_mode, filter_mode, norm)) => {
                bin.push(3);
                bin.extend_from_slice(&addr_mode.to_ne_bytes());
                bin.extend_from_slice(&filter_mode.to_ne_bytes());
                bin.push(norm as u8);
            }
            InternalKernelArgType::FormatArray => bin.push(4),
            InternalKernelArgType::OrderArray => bin.push(5),
            InternalKernelArgType::WorkDim => bin.push(6),
        }

        bin
    }

    fn deserialize(bin: &mut &[u8]) -> Option<Self> {
        let size = read_ne_usize(bin);
        let offset = read_ne_usize(bin);

        let kind = match read_ne_u8(bin) {
            0 => InternalKernelArgType::ConstantBuffer,
            1 => InternalKernelArgType::GlobalWorkOffsets,
            2 => InternalKernelArgType::PrintfBuffer,
            3 => {
                let addr_mode = read_ne_u32(bin);
                let filter_mode = read_ne_u32(bin);
                let norm = read_ne_u8(bin) == 1;
                InternalKernelArgType::InlineSampler((addr_mode, filter_mode, norm))
            }
            4 => InternalKernelArgType::FormatArray,
            5 => InternalKernelArgType::OrderArray,
            6 => InternalKernelArgType::WorkDim,
            _ => return None,
        };

        Some(Self {
            kind: kind,
            size: size,
            offset: offset,
        })
    }
}

#[derive(Clone, PartialEq, Eq, Hash)]
pub struct KernelInfo {
    pub args: Vec<KernelArg>,
    pub internal_args: Vec<InternalKernelArg>,
    pub attributes_string: String,
    pub work_group_size: [usize; 3],
    pub subgroup_size: usize,
    pub num_subgroups: usize,
}

pub struct CSOWrapper {
    pub cso_ptr: *mut c_void,
    dev: &'static Device,
}

impl CSOWrapper {
    pub fn new(dev: &'static Device, nir: &NirShader) -> Self {
        let cso_ptr = dev
            .helper_ctx()
            .create_compute_state(nir, nir.shared_size());

        Self {
            cso_ptr: cso_ptr,
            dev: dev,
        }
    }

    pub fn get_cso_info(&self) -> pipe_compute_state_object_info {
        self.dev.helper_ctx().compute_state_info(self.cso_ptr)
    }
}

impl Drop for CSOWrapper {
    fn drop(&mut self) {
        self.dev.helper_ctx().delete_compute_state(self.cso_ptr);
    }
}

pub enum KernelDevStateVariant {
    Cso(CSOWrapper),
    Nir(NirShader),
}

pub struct Kernel {
    pub base: CLObjectBase<CL_INVALID_KERNEL>,
    pub prog: Arc<Program>,
    pub name: String,
    pub values: Vec<RefCell<Option<KernelArgValue>>>,
    pub builds: HashMap<&'static Device, Arc<NirKernelBuild>>,
    pub kernel_info: KernelInfo,
}

impl_cl_type_trait!(cl_kernel, Kernel, CL_INVALID_KERNEL);

fn create_kernel_arr<T>(vals: &[usize], val: T) -> [T; 3]
where
    T: std::convert::TryFrom<usize> + Copy,
    <T as std::convert::TryFrom<usize>>::Error: std::fmt::Debug,
{
    let mut res = [val; 3];
    for (i, v) in vals.iter().enumerate() {
        res[i] = (*v).try_into().expect("64 bit work groups not supported");
    }
    res
}

fn opt_nir(nir: &mut NirShader, dev: &Device, has_explicit_types: bool) {
    let nir_options = unsafe {
        &*dev
            .screen
            .nir_shader_compiler_options(pipe_shader_type::PIPE_SHADER_COMPUTE)
    };

    while {
        let mut progress = false;

        progress |= nir_pass!(nir, nir_copy_prop);
        progress |= nir_pass!(nir, nir_opt_copy_prop_vars);
        progress |= nir_pass!(nir, nir_opt_dead_write_vars);

        if nir_options.lower_to_scalar {
            nir_pass!(
                nir,
                nir_lower_alu_to_scalar,
                nir_options.lower_to_scalar_filter,
                ptr::null(),
            );
            nir_pass!(nir, nir_lower_phis_to_scalar, false);
        }

        progress |= nir_pass!(nir, nir_opt_deref);
        if has_explicit_types {
            progress |= nir_pass!(nir, nir_opt_memcpy);
        }
        progress |= nir_pass!(nir, nir_opt_dce);
        progress |= nir_pass!(nir, nir_opt_undef);
        progress |= nir_pass!(nir, nir_opt_constant_folding);
        progress |= nir_pass!(nir, nir_opt_cse);
        nir_pass!(nir, nir_split_var_copies);
        progress |= nir_pass!(nir, nir_lower_var_copies);
        progress |= nir_pass!(nir, nir_lower_vars_to_ssa);
        nir_pass!(nir, nir_lower_alu);
        progress |= nir_pass!(nir, nir_opt_phi_precision);
        progress |= nir_pass!(nir, nir_opt_algebraic);
        progress |= nir_pass!(
            nir,
            nir_opt_if,
            nir_opt_if_options::nir_opt_if_optimize_phi_true_false,
        );
        progress |= nir_pass!(nir, nir_opt_dead_cf);
        progress |= nir_pass!(nir, nir_opt_remove_phis);
        // we don't want to be too aggressive here, but it kills a bit of CFG
        progress |= nir_pass!(nir, nir_opt_peephole_select, 8, true, true);
        progress |= nir_pass!(
            nir,
            nir_lower_vec3_to_vec4,
            nir_variable_mode::nir_var_mem_generic | nir_variable_mode::nir_var_uniform,
        );

        if nir_options.max_unroll_iterations != 0 {
            progress |= nir_pass!(nir, nir_opt_loop_unroll);
        }
        nir.sweep_mem();
        progress
    } {}
}

/// # Safety
///
/// Only safe to call when `var` is a valid pointer to a valid [`nir_variable`]
unsafe extern "C" fn can_remove_var(var: *mut nir_variable, _: *mut c_void) -> bool {
    // SAFETY: It is the caller's responsibility to provide a valid and aligned pointer
    let var_type = unsafe { (*var).type_ };
    // SAFETY: `nir_variable`'s type invariant guarantees that the `type_` field is valid and
    // properly aligned.
    unsafe {
        !glsl_type_is_image(var_type)
            && !glsl_type_is_texture(var_type)
            && !glsl_type_is_sampler(var_type)
    }
}

fn lower_and_optimize_nir(
    dev: &Device,
    nir: &mut NirShader,
    args: &[spirv::SPIRVKernelArg],
    lib_clc: &NirShader,
) -> (Vec<KernelArg>, Vec<InternalKernelArg>) {
    let address_bits_base_type;
    let address_bits_ptr_type;
    let global_address_format;
    let shared_address_format;

    if dev.address_bits() == 64 {
        address_bits_base_type = glsl_base_type::GLSL_TYPE_UINT64;
        address_bits_ptr_type = unsafe { glsl_uint64_t_type() };
        global_address_format = nir_address_format::nir_address_format_64bit_global;
        shared_address_format = nir_address_format::nir_address_format_32bit_offset_as_64bit;
    } else {
        address_bits_base_type = glsl_base_type::GLSL_TYPE_UINT;
        address_bits_ptr_type = unsafe { glsl_uint_type() };
        global_address_format = nir_address_format::nir_address_format_32bit_global;
        shared_address_format = nir_address_format::nir_address_format_32bit_offset;
    }

    let mut lower_state = rusticl_lower_state::default();
    let nir_options = unsafe {
        &*dev
            .screen
            .nir_shader_compiler_options(pipe_shader_type::PIPE_SHADER_COMPUTE)
    };

    nir_pass!(nir, nir_scale_fdiv);
    nir.set_workgroup_size_variable_if_zero();
    nir.structurize();
    while {
        let mut progress = false;
        nir_pass!(nir, nir_split_var_copies);
        progress |= nir_pass!(nir, nir_copy_prop);
        progress |= nir_pass!(nir, nir_opt_copy_prop_vars);
        progress |= nir_pass!(nir, nir_opt_dead_write_vars);
        progress |= nir_pass!(nir, nir_opt_deref);
        progress |= nir_pass!(nir, nir_opt_dce);
        progress |= nir_pass!(nir, nir_opt_undef);
        progress |= nir_pass!(nir, nir_opt_constant_folding);
        progress |= nir_pass!(nir, nir_opt_cse);
        progress |= nir_pass!(nir, nir_lower_vars_to_ssa);
        progress |= nir_pass!(nir, nir_opt_algebraic);
        progress
    } {}
    nir.inline(lib_clc);
    nir.cleanup_functions();
    // that should free up tons of memory
    nir.sweep_mem();

    nir_pass!(nir, nir_dedup_inline_samplers);

    let mut printf_opts = nir_lower_printf_options::default();
    printf_opts.set_treat_doubles_as_floats(false);
    printf_opts.max_buffer_size = dev.printf_buffer_size() as u32;
    nir_pass!(nir, nir_lower_printf, &printf_opts);

    opt_nir(nir, dev, false);

    let mut args = KernelArg::from_spirv_nir(args, nir);
    let mut internal_args = Vec::new();

    let dv_opts = nir_remove_dead_variables_options {
        can_remove_var: Some(can_remove_var),
        can_remove_var_data: ptr::null_mut(),
    };
    nir_pass!(
        nir,
        nir_remove_dead_variables,
        nir_variable_mode::nir_var_uniform
            | nir_variable_mode::nir_var_image
            | nir_variable_mode::nir_var_mem_constant
            | nir_variable_mode::nir_var_mem_shared
            | nir_variable_mode::nir_var_function_temp,
        &dv_opts,
    );

    // asign locations for inline samplers
    let mut last_loc = -1;
    for v in nir
        .variables_with_mode(nir_variable_mode::nir_var_uniform | nir_variable_mode::nir_var_image)
    {
        if unsafe { !glsl_type_is_sampler(v.type_) } {
            last_loc = v.data.location;
            continue;
        }
        let s = unsafe { v.data.anon_1.sampler };
        if s.is_inline_sampler() != 0 {
            last_loc += 1;
            v.data.location = last_loc;

            internal_args.push(InternalKernelArg {
                kind: InternalKernelArgType::InlineSampler(Sampler::nir_to_cl(
                    s.addressing_mode(),
                    s.filter_mode(),
                    s.normalized_coordinates(),
                )),
                offset: 0,
                size: 0,
            });
        } else {
            last_loc = v.data.location;
        }
    }

    nir_pass!(nir, nir_lower_readonly_images_to_tex, true);
    nir_pass!(
        nir,
        nir_lower_cl_images,
        !dev.images_as_deref(),
        !dev.samplers_as_deref(),
    );

    nir.reset_scratch_size();
    nir_pass!(
        nir,
        nir_lower_vars_to_explicit_types,
        nir_variable_mode::nir_var_mem_constant,
        Some(glsl_get_cl_type_size_align),
    );
    nir.extract_constant_initializers();

    // run before gather info
    nir_pass!(nir, nir_lower_system_values);
    let mut compute_options = nir_lower_compute_system_values_options::default();
    compute_options.set_has_base_global_invocation_id(true);
    nir_pass!(nir, nir_lower_compute_system_values, &compute_options);
    nir.gather_info();

    if nir.reads_sysval(gl_system_value::SYSTEM_VALUE_BASE_GLOBAL_INVOCATION_ID) {
        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::GlobalWorkOffsets,
            offset: 0,
            size: (3 * dev.address_bits() / 8) as usize,
        });
        lower_state.base_global_invoc_id_loc = args.len() + internal_args.len() - 1;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            unsafe { glsl_vector_type(address_bits_base_type, 3) },
            lower_state.base_global_invoc_id_loc,
            "base_global_invocation_id",
        );
    }

    if nir.has_constant() {
        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::ConstantBuffer,
            offset: 0,
            size: (dev.address_bits() / 8) as usize,
        });
        lower_state.const_buf_loc = args.len() + internal_args.len() - 1;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            address_bits_ptr_type,
            lower_state.const_buf_loc,
            "constant_buffer_addr",
        );
    }
    if nir.has_printf() {
        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::PrintfBuffer,
            offset: 0,
            size: (dev.address_bits() / 8) as usize,
        });
        lower_state.printf_buf_loc = args.len() + internal_args.len() - 1;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            address_bits_ptr_type,
            lower_state.printf_buf_loc,
            "printf_buffer_addr",
        );
    }

    if nir.num_images() > 0 || nir.num_textures() > 0 {
        let count = nir.num_images() + nir.num_textures();
        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::FormatArray,
            offset: 0,
            size: 2 * count as usize,
        });

        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::OrderArray,
            offset: 0,
            size: 2 * count as usize,
        });

        lower_state.format_arr_loc = args.len() + internal_args.len() - 2;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            unsafe { glsl_array_type(glsl_int16_t_type(), count as u32, 2) },
            lower_state.format_arr_loc,
            "image_formats",
        );

        lower_state.order_arr_loc = args.len() + internal_args.len() - 1;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            unsafe { glsl_array_type(glsl_int16_t_type(), count as u32, 2) },
            lower_state.order_arr_loc,
            "image_orders",
        );
    }

    if nir.reads_sysval(gl_system_value::SYSTEM_VALUE_WORK_DIM) {
        internal_args.push(InternalKernelArg {
            kind: InternalKernelArgType::WorkDim,
            size: 1,
            offset: 0,
        });
        lower_state.work_dim_loc = args.len() + internal_args.len() - 1;
        nir.add_var(
            nir_variable_mode::nir_var_uniform,
            unsafe { glsl_uint8_t_type() },
            lower_state.work_dim_loc,
            "work_dim",
        );
    }

    nir_pass!(
        nir,
        nir_lower_vars_to_explicit_types,
        nir_variable_mode::nir_var_mem_shared
            | nir_variable_mode::nir_var_function_temp
            | nir_variable_mode::nir_var_shader_temp
            | nir_variable_mode::nir_var_uniform
            | nir_variable_mode::nir_var_mem_global
            | nir_variable_mode::nir_var_mem_generic,
        Some(glsl_get_cl_type_size_align),
    );

    opt_nir(nir, dev, true);
    nir_pass!(nir, nir_lower_memcpy);

    nir_pass!(
        nir,
        nir_lower_explicit_io,
        nir_variable_mode::nir_var_mem_global | nir_variable_mode::nir_var_mem_constant,
        global_address_format,
    );

    nir_pass!(nir, rusticl_lower_intrinsics, &mut lower_state);
    nir_pass!(
        nir,
        nir_lower_explicit_io,
        nir_variable_mode::nir_var_mem_shared
            | nir_variable_mode::nir_var_function_temp
            | nir_variable_mode::nir_var_uniform,
        shared_address_format,
    );

    if nir_options.lower_int64_options.0 != 0 {
        nir_pass!(nir, nir_lower_int64);
    }

    if nir_options.lower_uniforms_to_ubo {
        nir_pass!(nir, rusticl_lower_inputs);
    }

    nir_pass!(nir, nir_lower_convert_alu_types, None);

    opt_nir(nir, dev, true);

    /* before passing it into drivers, assign locations as drivers might remove nir_variables or
     * other things we depend on
     */
    KernelArg::assign_locations(&mut args, &mut internal_args, nir);

    /* update the has_variable_shared_mem info as we might have DCEed all of them */
    nir.set_has_variable_shared_mem(
        args.iter()
            .any(|arg| arg.kind == KernelArgType::MemLocal && !arg.dead),
    );
    dev.screen.finalize_nir(nir);

    nir_pass!(nir, nir_opt_dce);
    nir.sweep_mem();

    (args, internal_args)
}

fn deserialize_nir(
    bin: &mut &[u8],
    d: &Device,
) -> Option<(NirShader, Vec<KernelArg>, Vec<InternalKernelArg>)> {
    let nir_len = read_ne_usize(bin);

    let nir = NirShader::deserialize(
        bin,
        nir_len,
        d.screen()
            .nir_shader_compiler_options(pipe_shader_type::PIPE_SHADER_COMPUTE),
    )?;

    let arg_len = read_ne_usize(bin);
    let mut args = Vec::with_capacity(arg_len);
    for _ in 0..arg_len {
        args.push(KernelArg::deserialize(bin)?);
    }

    let arg_len = read_ne_usize(bin);
    let mut internal_args = Vec::with_capacity(arg_len);
    for _ in 0..arg_len {
        internal_args.push(InternalKernelArg::deserialize(bin)?);
    }

    assert!(bin.is_empty());

    Some((nir, args, internal_args))
}

pub(super) fn convert_spirv_to_nir(
    build: &ProgramBuild,
    name: &str,
    args: &[spirv::SPIRVKernelArg],
    dev: &Device,
) -> (KernelInfo, NirShader) {
    let cache = dev.screen().shader_cache();
    let key = build.hash_key(dev, name);

    let res = if let Some(cache) = &cache {
        cache.get(&mut key.unwrap()).and_then(|entry| {
            let mut bin: &[u8] = &entry;
            deserialize_nir(&mut bin, dev)
        })
    } else {
        None
    };

    let (nir, args, internal_args) = if let Some(res) = res {
        res
    } else {
        let mut nir = build.to_nir(name, dev);

        /* this is a hack until we support fp16 properly and check for denorms inside
         * vstore/vload_half
         */
        nir.preserve_fp16_denorms();

        // Set to rtne for now until drivers are able to report their prefered rounding mode, that
        // also matches what we report via the API.
        nir.set_fp_rounding_mode_rtne();

        let (args, internal_args) = lower_and_optimize_nir(dev, &mut nir, args, &dev.lib_clc);

        if let Some(cache) = cache {
            let mut bin = Vec::new();
            let mut nir = nir.serialize();

            bin.extend_from_slice(&nir.len().to_ne_bytes());
            bin.append(&mut nir);

            bin.extend_from_slice(&args.len().to_ne_bytes());
            for arg in &args {
                bin.append(&mut arg.serialize());
            }

            bin.extend_from_slice(&internal_args.len().to_ne_bytes());
            for arg in &internal_args {
                bin.append(&mut arg.serialize());
            }

            cache.put(&bin, &mut key.unwrap());
        }

        (nir, args, internal_args)
    };

    let attributes_string = build.attribute_str(name, dev);
    let wgs = nir.workgroup_size();
    let kernel_info = KernelInfo {
        args: args,
        internal_args: internal_args,
        attributes_string: attributes_string,
        work_group_size: [wgs[0] as usize, wgs[1] as usize, wgs[2] as usize],
        subgroup_size: nir.subgroup_size() as usize,
        num_subgroups: nir.num_subgroups() as usize,
    };

    (kernel_info, nir)
}

fn extract<'a, const S: usize>(buf: &'a mut &[u8]) -> &'a [u8; S] {
    let val;
    (val, *buf) = (*buf).split_at(S);
    // we split of 4 bytes and convert to [u8; 4], so this should be safe
    // use split_array_ref once it's stable
    val.try_into().unwrap()
}

impl Kernel {
    pub fn new(name: String, prog: Arc<Program>) -> Arc<Kernel> {
        let prog_build = prog.build_info();
        let kernel_info = prog_build.kernel_info.get(&name).unwrap().clone();
        let builds = prog_build
            .builds
            .iter()
            .filter_map(|(&dev, b)| b.kernels.get(&name).map(|k| (dev, k.clone())))
            .collect();

        // can't use vec!...
        let values = kernel_info
            .args
            .iter()
            .map(|_| RefCell::new(None))
            .collect();

        Arc::new(Self {
            base: CLObjectBase::new(),
            prog: prog.clone(),
            name: name,
            values: values,
            builds: builds,
            kernel_info: kernel_info,
        })
    }

    fn optimize_local_size(&self, d: &Device, grid: &mut [u32; 3], block: &mut [u32; 3]) {
        let mut threads = self.max_threads_per_block(d) as u32;
        let dim_threads = d.max_block_sizes();
        let subgroups = self.preferred_simd_size(d) as u32;

        if !block.contains(&0) {
            for i in 0..3 {
                // we already made sure everything is fine
                grid[i] /= block[i];
            }
            return;
        }

        for i in 0..3 {
            let t = cmp::min(threads, dim_threads[i] as u32);
            let gcd = gcd(t, grid[i]);

            block[i] = gcd;
            grid[i] /= gcd;

            // update limits
            threads /= block[i];
        }

        // if we didn't fill the subgroup we can do a bit better if we have threads remaining
        let total_threads = block[0] * block[1] * block[2];
        if threads != 1 && total_threads < subgroups {
            for i in 0..3 {
                if grid[i] * total_threads < threads {
                    block[i] *= grid[i];
                    grid[i] = 1;
                    // can only do it once as nothing is cleanly divisible
                    break;
                }
            }
        }
    }

    // the painful part is, that host threads are allowed to modify the kernel object once it was
    // enqueued, so return a closure with all req data included.
    pub fn launch(
        self: &Arc<Self>,
        q: &Arc<Queue>,
        work_dim: u32,
        block: &[usize],
        grid: &[usize],
        offsets: &[usize],
    ) -> CLResult<EventSig> {
        let nir_kernel_build = self.builds.get(q.device).unwrap().clone();
        let mut block = create_kernel_arr::<u32>(block, 1);
        let mut grid = create_kernel_arr::<u32>(grid, 1);
        let offsets = create_kernel_arr::<u64>(offsets, 0);
        let mut input: Vec<u8> = Vec::new();
        let mut resource_info = Vec::new();
        // Set it once so we get the alignment padding right
        let static_local_size: u64 = nir_kernel_build.shared_size;
        let mut variable_local_size: u64 = static_local_size;
        let printf_size = q.device.printf_buffer_size() as u32;
        let mut samplers = Vec::new();
        let mut iviews = Vec::new();
        let mut sviews = Vec::new();
        let mut tex_formats: Vec<u16> = Vec::new();
        let mut tex_orders: Vec<u16> = Vec::new();
        let mut img_formats: Vec<u16> = Vec::new();
        let mut img_orders: Vec<u16> = Vec::new();
        let null_ptr: &[u8] = if q.device.address_bits() == 64 {
            &[0; 8]
        } else {
            &[0; 4]
        };

        self.optimize_local_size(q.device, &mut grid, &mut block);

        for (arg, val) in self.kernel_info.args.iter().zip(&self.values) {
            if arg.dead {
                continue;
            }

            if arg.kind != KernelArgType::Image
                && arg.kind != KernelArgType::RWImage
                && arg.kind != KernelArgType::Texture
                && arg.kind != KernelArgType::Sampler
            {
                input.resize(arg.offset, 0);
            }
            match val.borrow().as_ref().unwrap() {
                KernelArgValue::Constant(c) => input.extend_from_slice(c),
                KernelArgValue::MemObject(mem) => {
                    let res = mem.get_res_of_dev(q.device)?;
                    // If resource is a buffer and mem a 2D image, the 2d image was created from a
                    // buffer. Use strides and dimensions of 2d image
                    let app_img_info =
                        if res.as_ref().is_buffer() && mem.mem_type == CL_MEM_OBJECT_IMAGE2D {
                            Some(AppImgInfo::new(
                                mem.image_desc.row_pitch()? / mem.image_elem_size as u32,
                                mem.image_desc.width()?,
                                mem.image_desc.height()?,
                            ))
                        } else {
                            None
                        };
                    if mem.is_buffer() {
                        if q.device.address_bits() == 64 {
                            input.extend_from_slice(&mem.offset.to_ne_bytes());
                        } else {
                            input.extend_from_slice(&(mem.offset as u32).to_ne_bytes());
                        }
                        resource_info.push((res.clone(), arg.offset));
                    } else {
                        let format = mem.pipe_format;
                        let (formats, orders) = if arg.kind == KernelArgType::Image {
                            iviews.push(res.pipe_image_view(
                                format,
                                false,
                                mem.pipe_image_host_access(),
                                app_img_info.as_ref(),
                            ));
                            (&mut img_formats, &mut img_orders)
                        } else if arg.kind == KernelArgType::RWImage {
                            iviews.push(res.pipe_image_view(
                                format,
                                true,
                                mem.pipe_image_host_access(),
                                app_img_info.as_ref(),
                            ));
                            (&mut img_formats, &mut img_orders)
                        } else {
                            sviews.push((res.clone(), format, app_img_info));
                            (&mut tex_formats, &mut tex_orders)
                        };

                        let binding = arg.binding as usize;
                        assert!(binding >= formats.len());

                        formats.resize(binding, 0);
                        orders.resize(binding, 0);

                        formats.push(mem.image_format.image_channel_data_type as u16);
                        orders.push(mem.image_format.image_channel_order as u16);
                    }
                }
                KernelArgValue::LocalMem(size) => {
                    // TODO 32 bit
                    let pot = cmp::min(*size, 0x80);
                    variable_local_size =
                        align(variable_local_size, pot.next_power_of_two() as u64);
                    if q.device.address_bits() == 64 {
                        input.extend_from_slice(&variable_local_size.to_ne_bytes());
                    } else {
                        input.extend_from_slice(&(variable_local_size as u32).to_ne_bytes());
                    }
                    variable_local_size += *size as u64;
                }
                KernelArgValue::Sampler(sampler) => {
                    samplers.push(sampler.pipe());
                }
                KernelArgValue::None => {
                    assert!(
                        arg.kind == KernelArgType::MemGlobal
                            || arg.kind == KernelArgType::MemConstant
                    );
                    input.extend_from_slice(null_ptr);
                }
            }
        }

        // subtract the shader local_size as we only request something on top of that.
        variable_local_size -= static_local_size;

        let mut printf_buf = None;
        for arg in &self.kernel_info.internal_args {
            if arg.offset > input.len() {
                input.resize(arg.offset, 0);
            }
            match arg.kind {
                InternalKernelArgType::ConstantBuffer => {
                    assert!(nir_kernel_build.constant_buffer.is_some());
                    input.extend_from_slice(null_ptr);
                    resource_info.push((
                        nir_kernel_build.constant_buffer.clone().unwrap(),
                        arg.offset,
                    ));
                }
                InternalKernelArgType::GlobalWorkOffsets => {
                    if q.device.address_bits() == 64 {
                        input.extend_from_slice(unsafe { as_byte_slice(&offsets) });
                    } else {
                        input.extend_from_slice(unsafe {
                            as_byte_slice(&[
                                offsets[0] as u32,
                                offsets[1] as u32,
                                offsets[2] as u32,
                            ])
                        });
                    }
                }
                InternalKernelArgType::PrintfBuffer => {
                    let buf = Arc::new(
                        q.device
                            .screen
                            .resource_create_buffer(
                                printf_size,
                                ResourceType::Staging,
                                PIPE_BIND_GLOBAL,
                            )
                            .unwrap(),
                    );

                    input.extend_from_slice(null_ptr);
                    resource_info.push((buf.clone(), arg.offset));

                    printf_buf = Some(buf);
                }
                InternalKernelArgType::InlineSampler(cl) => {
                    samplers.push(Sampler::cl_to_pipe(cl));
                }
                InternalKernelArgType::FormatArray => {
                    input.extend_from_slice(unsafe { as_byte_slice(&tex_formats) });
                    input.extend_from_slice(unsafe { as_byte_slice(&img_formats) });
                }
                InternalKernelArgType::OrderArray => {
                    input.extend_from_slice(unsafe { as_byte_slice(&tex_orders) });
                    input.extend_from_slice(unsafe { as_byte_slice(&img_orders) });
                }
                InternalKernelArgType::WorkDim => {
                    input.extend_from_slice(&[work_dim as u8; 1]);
                }
            }
        }

        Ok(Box::new(move |q, ctx| {
            let mut input = input.clone();
            let mut resources = Vec::with_capacity(resource_info.len());
            let mut globals: Vec<*mut u32> = Vec::new();
            let printf_format = &nir_kernel_build.printf_info;

            let mut sviews: Vec<_> = sviews
                .iter()
                .map(|(s, f, aii)| ctx.create_sampler_view(s, *f, aii.as_ref()))
                .collect();
            let samplers: Vec<_> = samplers
                .iter()
                .map(|s| ctx.create_sampler_state(s))
                .collect();

            for (res, offset) in &resource_info {
                resources.push(res);
                globals.push(unsafe { input.as_mut_ptr().add(*offset) }.cast());
            }

            if let Some(printf_buf) = &printf_buf {
                let init_data: [u8; 1] = [4];
                ctx.buffer_subdata(
                    printf_buf,
                    0,
                    init_data.as_ptr().cast(),
                    init_data.len() as u32,
                );
            }

            let temp_cso;
            let cso = match &nir_kernel_build.nir_or_cso {
                KernelDevStateVariant::Cso(cso) => cso,
                KernelDevStateVariant::Nir(nir) => {
                    temp_cso = CSOWrapper::new(q.device, nir);
                    &temp_cso
                }
            };

            ctx.bind_compute_state(cso.cso_ptr);
            ctx.bind_sampler_states(&samplers);
            ctx.set_sampler_views(&mut sviews);
            ctx.set_shader_images(&iviews);
            ctx.set_global_binding(resources.as_slice(), &mut globals);
            ctx.update_cb0(&input);

            ctx.launch_grid(work_dim, block, grid, variable_local_size as u32);

            ctx.clear_global_binding(globals.len() as u32);
            ctx.clear_shader_images(iviews.len() as u32);
            ctx.clear_sampler_views(sviews.len() as u32);
            ctx.clear_sampler_states(samplers.len() as u32);

            ctx.bind_compute_state(ptr::null_mut());

            ctx.memory_barrier(PIPE_BARRIER_GLOBAL_BUFFER);

            samplers.iter().for_each(|s| ctx.delete_sampler_state(*s));
            sviews.iter().for_each(|v| ctx.sampler_view_destroy(*v));

            if let Some(printf_buf) = &printf_buf {
                let tx = ctx
                    .buffer_map(
                        printf_buf,
                        0,
                        printf_size as i32,
                        RWFlags::RD,
                        ResourceMapType::Normal,
                    )
                    .ok_or(CL_OUT_OF_RESOURCES)?
                    .with_ctx(ctx);
                let mut buf: &[u8] =
                    unsafe { slice::from_raw_parts(tx.ptr().cast(), printf_size as usize) };
                let length = u32::from_ne_bytes(*extract(&mut buf));

                // update our slice to make sure we don't go out of bounds
                buf = &buf[0..(length - 4) as usize];
                if let Some(pf) = printf_format.as_ref() {
                    pf.u_printf(buf)
                }
            }

            Ok(())
        }))
    }

    pub fn access_qualifier(&self, idx: cl_uint) -> cl_kernel_arg_access_qualifier {
        let aq = self.kernel_info.args[idx as usize].spirv.access_qualifier;

        if aq
            == clc_kernel_arg_access_qualifier::CLC_KERNEL_ARG_ACCESS_READ
                | clc_kernel_arg_access_qualifier::CLC_KERNEL_ARG_ACCESS_WRITE
        {
            CL_KERNEL_ARG_ACCESS_READ_WRITE
        } else if aq == clc_kernel_arg_access_qualifier::CLC_KERNEL_ARG_ACCESS_READ {
            CL_KERNEL_ARG_ACCESS_READ_ONLY
        } else if aq == clc_kernel_arg_access_qualifier::CLC_KERNEL_ARG_ACCESS_WRITE {
            CL_KERNEL_ARG_ACCESS_WRITE_ONLY
        } else {
            CL_KERNEL_ARG_ACCESS_NONE
        }
    }

    pub fn address_qualifier(&self, idx: cl_uint) -> cl_kernel_arg_address_qualifier {
        match self.kernel_info.args[idx as usize].spirv.address_qualifier {
            clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_PRIVATE => {
                CL_KERNEL_ARG_ADDRESS_PRIVATE
            }
            clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_CONSTANT => {
                CL_KERNEL_ARG_ADDRESS_CONSTANT
            }
            clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_LOCAL => {
                CL_KERNEL_ARG_ADDRESS_LOCAL
            }
            clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_GLOBAL => {
                CL_KERNEL_ARG_ADDRESS_GLOBAL
            }
        }
    }

    pub fn type_qualifier(&self, idx: cl_uint) -> cl_kernel_arg_type_qualifier {
        let tq = self.kernel_info.args[idx as usize].spirv.type_qualifier;
        let zero = clc_kernel_arg_type_qualifier(0);
        let mut res = CL_KERNEL_ARG_TYPE_NONE;

        if tq & clc_kernel_arg_type_qualifier::CLC_KERNEL_ARG_TYPE_CONST != zero {
            res |= CL_KERNEL_ARG_TYPE_CONST;
        }

        if tq & clc_kernel_arg_type_qualifier::CLC_KERNEL_ARG_TYPE_RESTRICT != zero {
            res |= CL_KERNEL_ARG_TYPE_RESTRICT;
        }

        if tq & clc_kernel_arg_type_qualifier::CLC_KERNEL_ARG_TYPE_VOLATILE != zero {
            res |= CL_KERNEL_ARG_TYPE_VOLATILE;
        }

        res.into()
    }

    pub fn work_group_size(&self) -> [usize; 3] {
        self.kernel_info.work_group_size
    }

    pub fn num_subgroups(&self) -> usize {
        self.kernel_info.num_subgroups
    }

    pub fn subgroup_size(&self) -> usize {
        self.kernel_info.subgroup_size
    }

    pub fn arg_name(&self, idx: cl_uint) -> &String {
        &self.kernel_info.args[idx as usize].spirv.name
    }

    pub fn arg_type_name(&self, idx: cl_uint) -> &String {
        &self.kernel_info.args[idx as usize].spirv.type_name
    }

    pub fn priv_mem_size(&self, dev: &Device) -> cl_ulong {
        self.builds.get(dev).unwrap().info.private_memory as cl_ulong
    }

    pub fn max_threads_per_block(&self, dev: &Device) -> usize {
        self.builds.get(dev).unwrap().info.max_threads as usize
    }

    pub fn preferred_simd_size(&self, dev: &Device) -> usize {
        self.builds.get(dev).unwrap().info.preferred_simd_size as usize
    }

    pub fn local_mem_size(&self, dev: &Device) -> cl_ulong {
        // TODO include args
        self.builds.get(dev).unwrap().shared_size as cl_ulong
    }

    pub fn has_svm_devs(&self) -> bool {
        self.prog.devs.iter().any(|dev| dev.svm_supported())
    }

    pub fn subgroup_sizes(&self, dev: &Device) -> Vec<usize> {
        SetBitIndices::from_msb(self.builds.get(dev).unwrap().info.simd_sizes)
            .map(|bit| 1 << bit)
            .collect()
    }

    pub fn subgroups_for_block(&self, dev: &Device, block: &[usize]) -> usize {
        let subgroup_size = self.subgroup_size_for_block(dev, block);
        if subgroup_size == 0 {
            return 0;
        }

        let threads = block.iter().product();
        div_round_up(threads, subgroup_size)
    }

    pub fn subgroup_size_for_block(&self, dev: &Device, block: &[usize]) -> usize {
        let subgroup_sizes = self.subgroup_sizes(dev);
        if subgroup_sizes.is_empty() {
            return 0;
        }

        if subgroup_sizes.len() == 1 {
            return subgroup_sizes[0];
        }

        let block = [
            *block.first().unwrap_or(&1) as u32,
            *block.get(1).unwrap_or(&1) as u32,
            *block.get(2).unwrap_or(&1) as u32,
        ];

        match &self.builds.get(dev).unwrap().nir_or_cso {
            KernelDevStateVariant::Cso(cso) => {
                dev.helper_ctx()
                    .compute_state_subgroup_size(cso.cso_ptr, &block) as usize
            }
            _ => {
                panic!()
            }
        }
    }
}

impl Clone for Kernel {
    fn clone(&self) -> Self {
        Self {
            base: CLObjectBase::new(),
            prog: self.prog.clone(),
            name: self.name.clone(),
            values: self.values.clone(),
            builds: self.builds.clone(),
            kernel_info: self.kernel_info.clone(),
        }
    }
}
