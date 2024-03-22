use crate::compiler::nir::*;
use crate::pipe::screen::*;
use crate::util::disk_cache::*;

use libc_rust_gen::malloc;
use mesa_rust_gen::*;
use mesa_rust_util::serialize::*;
use mesa_rust_util::string::*;

use std::ffi::CString;
use std::fmt::Debug;
use std::os::raw::c_char;
use std::os::raw::c_void;
use std::ptr;
use std::slice;

const INPUT_STR: *const c_char = b"input.cl\0" as *const u8 as *const c_char;

pub enum SpecConstant {
    None,
}

pub struct SPIRVBin {
    spirv: clc_binary,
    info: Option<clc_parsed_spirv>,
}

#[derive(PartialEq, Eq, Hash, Clone)]
pub struct SPIRVKernelArg {
    pub name: String,
    pub type_name: String,
    pub access_qualifier: clc_kernel_arg_access_qualifier,
    pub address_qualifier: clc_kernel_arg_address_qualifier,
    pub type_qualifier: clc_kernel_arg_type_qualifier,
}

pub struct CLCHeader<'a> {
    pub name: CString,
    pub source: &'a CString,
}

impl<'a> Debug for CLCHeader<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = self.name.to_string_lossy();
        let source = self.source.to_string_lossy();

        f.write_fmt(format_args!("[{name}]:\n{source}"))
    }
}

unsafe fn callback_impl(data: *mut c_void, msg: *const c_char) {
    let data = data as *mut Vec<String>;
    let msgs = unsafe { data.as_mut() }.unwrap();
    msgs.push(c_string_to_string(msg));
}

unsafe extern "C" fn spirv_msg_callback(data: *mut c_void, msg: *const c_char) {
    unsafe {
        callback_impl(data, msg);
    }
}

unsafe extern "C" fn spirv_to_nir_msg_callback(
    data: *mut c_void,
    dbg_level: nir_spirv_debug_level,
    _offset: usize,
    msg: *const c_char,
) {
    if dbg_level >= nir_spirv_debug_level::NIR_SPIRV_DEBUG_LEVEL_WARNING {
        unsafe {
            callback_impl(data, msg);
        }
    }
}

fn create_clc_logger(msgs: &mut Vec<String>) -> clc_logger {
    clc_logger {
        priv_: msgs as *mut Vec<String> as *mut c_void,
        error: Some(spirv_msg_callback),
        warning: Some(spirv_msg_callback),
    }
}

impl SPIRVBin {
    pub fn from_clc(
        source: &CString,
        args: &[CString],
        headers: &[CLCHeader],
        cache: &Option<DiskCache>,
        features: clc_optional_features,
        spirv_extensions: &[CString],
        address_bits: u32,
    ) -> (Option<Self>, String) {
        let mut hash_key = None;
        let has_includes = args.iter().any(|a| a.as_bytes()[0..2] == *b"-I");

        let mut spirv_extensions: Vec<_> = spirv_extensions.iter().map(|s| s.as_ptr()).collect();
        spirv_extensions.push(ptr::null());

        if let Some(cache) = cache {
            if !has_includes {
                let mut key = Vec::new();

                key.extend_from_slice(source.as_bytes());
                args.iter()
                    .for_each(|a| key.extend_from_slice(a.as_bytes()));
                headers.iter().for_each(|h| {
                    key.extend_from_slice(h.name.as_bytes());
                    key.extend_from_slice(h.source.as_bytes());
                });

                // Safety: clc_optional_features is a struct of bools and contains no padding.
                // Sadly we can't guarentee this.
                key.extend(unsafe { as_byte_slice(slice::from_ref(&features)) });

                let mut key = cache.gen_key(&key);
                if let Some(data) = cache.get(&mut key) {
                    return (Some(Self::from_bin(&data)), String::from(""));
                }

                hash_key = Some(key);
            }
        }

        let c_headers: Vec<_> = headers
            .iter()
            .map(|h| clc_named_value {
                name: h.name.as_ptr(),
                value: h.source.as_ptr(),
            })
            .collect();

        let c_args: Vec<_> = args.iter().map(|a| a.as_ptr()).collect();

        let args = clc_compile_args {
            headers: c_headers.as_ptr(),
            num_headers: c_headers.len() as u32,
            source: clc_named_value {
                name: INPUT_STR,
                value: source.as_ptr(),
            },
            args: c_args.as_ptr(),
            num_args: c_args.len() as u32,
            spirv_version: clc_spirv_version::CLC_SPIRV_VERSION_MAX,
            features: features,
            allowed_spirv_extensions: spirv_extensions.as_ptr(),
            address_bits: address_bits,
        };
        let mut msgs: Vec<String> = Vec::new();
        let logger = create_clc_logger(&mut msgs);
        let mut out = clc_binary::default();

        let res = unsafe { clc_compile_c_to_spirv(&args, &logger, &mut out) };

        let res = if res {
            let spirv = SPIRVBin {
                spirv: out,
                info: None,
            };

            // add cache entry
            if !has_includes {
                if let Some(mut key) = hash_key {
                    cache.as_ref().unwrap().put(spirv.to_bin(), &mut key);
                }
            }

            Some(spirv)
        } else {
            None
        };

        (res, msgs.join("\n"))
    }

    // TODO cache linking, parsing is around 25% of link time
    pub fn link(spirvs: &[&SPIRVBin], library: bool) -> (Option<Self>, String) {
        let bins: Vec<_> = spirvs.iter().map(|s| &s.spirv as *const _).collect();

        let linker_args = clc_linker_args {
            in_objs: bins.as_ptr(),
            num_in_objs: bins.len() as u32,
            create_library: library as u32,
        };

        let mut msgs: Vec<String> = Vec::new();
        let logger = create_clc_logger(&mut msgs);

        let mut out = clc_binary::default();
        let res = unsafe { clc_link_spirv(&linker_args, &logger, &mut out) };

        let info;
        if !library {
            let mut pspirv = clc_parsed_spirv::default();
            let res = unsafe { clc_parse_spirv(&out, &logger, &mut pspirv) };

            if res {
                info = Some(pspirv);
            } else {
                info = None;
            }
        } else {
            info = None;
        }

        let res = if res {
            Some(SPIRVBin {
                spirv: out,
                info: info,
            })
        } else {
            None
        };
        (res, msgs.join("\n"))
    }

    pub fn clone_on_validate(&self, options: &clc_validator_options) -> (Option<Self>, String) {
        let mut msgs: Vec<String> = Vec::new();
        let logger = create_clc_logger(&mut msgs);
        let res = unsafe { clc_validate_spirv(&self.spirv, &logger, options) };

        (res.then(|| self.clone()), msgs.join("\n"))
    }

    fn kernel_infos(&self) -> &[clc_kernel_info] {
        match self.info {
            None => &[],
            Some(info) => unsafe { slice::from_raw_parts(info.kernels, info.num_kernels as usize) },
        }
    }

    fn kernel_info(&self, name: &str) -> Option<&clc_kernel_info> {
        self.kernel_infos()
            .iter()
            .find(|i| c_string_to_string(i.name) == name)
    }

    pub fn kernels(&self) -> Vec<String> {
        self.kernel_infos()
            .iter()
            .map(|i| i.name)
            .map(c_string_to_string)
            .collect()
    }

    pub fn vec_type_hint(&self, name: &str) -> Option<String> {
        self.kernel_info(name)
            .filter(|info| [1, 2, 3, 4, 8, 16].contains(&info.vec_hint_size))
            .map(|info| {
                let cltype = match info.vec_hint_type {
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_CHAR => "uchar",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_SHORT => "ushort",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_INT => "uint",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_LONG => "ulong",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_HALF => "half",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_FLOAT => "float",
                    clc_vec_hint_type::CLC_VEC_HINT_TYPE_DOUBLE => "double",
                };

                format!("vec_type_hint({}{})", cltype, info.vec_hint_size)
            })
    }

    pub fn local_size(&self, name: &str) -> Option<String> {
        self.kernel_info(name)
            .filter(|info| info.local_size != [0; 3])
            .map(|info| {
                format!(
                    "reqd_work_group_size({},{},{})",
                    info.local_size[0], info.local_size[1], info.local_size[2]
                )
            })
    }

    pub fn local_size_hint(&self, name: &str) -> Option<String> {
        self.kernel_info(name)
            .filter(|info| info.local_size_hint != [0; 3])
            .map(|info| {
                format!(
                    "work_group_size_hint({},{},{})",
                    info.local_size_hint[0], info.local_size_hint[1], info.local_size_hint[2]
                )
            })
    }

    pub fn args(&self, name: &str) -> Vec<SPIRVKernelArg> {
        match self.kernel_info(name) {
            None => Vec::new(),
            Some(info) => unsafe { slice::from_raw_parts(info.args, info.num_args) }
                .iter()
                .map(|a| SPIRVKernelArg {
                    name: c_string_to_string(a.name),
                    type_name: c_string_to_string(a.type_name),
                    access_qualifier: clc_kernel_arg_access_qualifier(a.access_qualifier),
                    address_qualifier: a.address_qualifier,
                    type_qualifier: clc_kernel_arg_type_qualifier(a.type_qualifier),
                })
                .collect(),
        }
    }

    fn get_spirv_options(
        library: bool,
        clc_shader: *const nir_shader,
        address_bits: u32,
        log: Option<&mut Vec<String>>,
    ) -> spirv_to_nir_options {
        let global_addr_format;
        let offset_addr_format;

        if address_bits == 32 {
            global_addr_format = nir_address_format::nir_address_format_32bit_global;
            offset_addr_format = nir_address_format::nir_address_format_32bit_offset;
        } else {
            global_addr_format = nir_address_format::nir_address_format_64bit_global;
            offset_addr_format = nir_address_format::nir_address_format_32bit_offset_as_64bit;
        }

        let debug = log.map(|log| spirv_to_nir_options__bindgen_ty_1 {
            func: Some(spirv_to_nir_msg_callback),
            private_data: (log as *mut Vec<String>).cast(),
        });

        spirv_to_nir_options {
            create_library: library,
            environment: nir_spirv_execution_environment::NIR_SPIRV_OPENCL,
            clc_shader: clc_shader,
            float_controls_execution_mode: float_controls::FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP32
                as u32,

            caps: spirv_supported_capabilities {
                address: true,
                float16: true,
                float64: true,
                generic_pointers: true,
                groups: true,
                subgroup_shuffle: true,
                int8: true,
                int16: true,
                int64: true,
                kernel: true,
                kernel_image: true,
                kernel_image_read_write: true,
                linkage: true,
                literal_sampler: true,
                printf: true,
                ..Default::default()
            },

            constant_addr_format: global_addr_format,
            global_addr_format: global_addr_format,
            shared_addr_format: offset_addr_format,
            temp_addr_format: offset_addr_format,
            debug: debug.unwrap_or_default(),

            ..Default::default()
        }
    }

    pub fn to_nir(
        &self,
        entry_point: &str,
        nir_options: *const nir_shader_compiler_options,
        libclc: &NirShader,
        spec_constants: &mut [nir_spirv_specialization],
        address_bits: u32,
        log: Option<&mut Vec<String>>,
    ) -> Option<NirShader> {
        let c_entry = CString::new(entry_point.as_bytes()).unwrap();
        let spirv_options = Self::get_spirv_options(false, libclc.get_nir(), address_bits, log);

        let nir = unsafe {
            spirv_to_nir(
                self.spirv.data.cast(),
                self.spirv.size / 4,
                spec_constants.as_mut_ptr(),
                spec_constants.len() as u32,
                gl_shader_stage::MESA_SHADER_KERNEL,
                c_entry.as_ptr(),
                &spirv_options,
                nir_options,
            )
        };

        NirShader::new(nir)
    }

    pub fn get_lib_clc(screen: &PipeScreen) -> Option<NirShader> {
        let nir_options = screen.nir_shader_compiler_options(pipe_shader_type::PIPE_SHADER_COMPUTE);
        let address_bits = screen.compute_param(pipe_compute_cap::PIPE_COMPUTE_CAP_ADDRESS_BITS);
        let spirv_options = Self::get_spirv_options(true, ptr::null(), address_bits, None);
        let shader_cache = DiskCacheBorrowed::as_ptr(&screen.shader_cache());

        NirShader::new(unsafe {
            nir_load_libclc_shader(
                address_bits,
                shader_cache,
                &spirv_options,
                nir_options,
                true,
            )
        })
    }

    pub fn to_bin(&self) -> &[u8] {
        unsafe { slice::from_raw_parts(self.spirv.data.cast(), self.spirv.size) }
    }

    pub fn from_bin(bin: &[u8]) -> Self {
        unsafe {
            let ptr = malloc(bin.len());
            ptr::copy_nonoverlapping(bin.as_ptr(), ptr.cast(), bin.len());
            let spirv = clc_binary {
                data: ptr,
                size: bin.len(),
            };

            let mut pspirv = clc_parsed_spirv::default();

            let info = if clc_parse_spirv(&spirv, ptr::null(), &mut pspirv) {
                Some(pspirv)
            } else {
                None
            };

            SPIRVBin {
                spirv: spirv,
                info: info,
            }
        }
    }

    pub fn spec_constant(&self, spec_id: u32) -> Option<clc_spec_constant_type> {
        let info = self.info?;
        let spec_constants =
            unsafe { slice::from_raw_parts(info.spec_constants, info.num_spec_constants as usize) };

        spec_constants
            .iter()
            .find(|sc| sc.id == spec_id)
            .map(|sc| sc.type_)
    }

    pub fn print(&self) {
        unsafe {
            clc_dump_spirv(&self.spirv, stderr_ptr());
        }
    }
}

impl Clone for SPIRVBin {
    fn clone(&self) -> Self {
        Self::from_bin(self.to_bin())
    }
}

impl Drop for SPIRVBin {
    fn drop(&mut self) {
        unsafe {
            clc_free_spirv(&mut self.spirv);
            if let Some(info) = &mut self.info {
                clc_free_parsed_spirv(info);
            }
        }
    }
}

impl SPIRVKernelArg {
    pub fn serialize(&self) -> Vec<u8> {
        let mut res = Vec::new();

        let name_arr = self.name.as_bytes();
        let type_name_arr = self.type_name.as_bytes();

        res.extend_from_slice(&name_arr.len().to_ne_bytes());
        res.extend_from_slice(name_arr);
        res.extend_from_slice(&type_name_arr.len().to_ne_bytes());
        res.extend_from_slice(type_name_arr);
        res.extend_from_slice(&u32::to_ne_bytes(self.access_qualifier.0));
        res.extend_from_slice(&u32::to_ne_bytes(self.type_qualifier.0));
        res.push(self.address_qualifier as u8);

        res
    }

    pub fn deserialize(bin: &mut &[u8]) -> Option<Self> {
        let name_len = read_ne_usize(bin);
        let name = read_string(bin, name_len)?;
        let type_len = read_ne_usize(bin);
        let type_name = read_string(bin, type_len)?;
        let access_qualifier = read_ne_u32(bin);
        let type_qualifier = read_ne_u32(bin);

        let address_qualifier = match read_ne_u8(bin) {
            0 => clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_PRIVATE,
            1 => clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_CONSTANT,
            2 => clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_LOCAL,
            3 => clc_kernel_arg_address_qualifier::CLC_KERNEL_ARG_ADDRESS_GLOBAL,
            _ => return None,
        };

        Some(Self {
            name: name,
            type_name: type_name,
            access_qualifier: clc_kernel_arg_access_qualifier(access_qualifier),
            address_qualifier: address_qualifier,
            type_qualifier: clc_kernel_arg_type_qualifier(type_qualifier),
        })
    }
}

pub trait CLCSpecConstantType {
    fn size(self) -> u8;
}

impl CLCSpecConstantType for clc_spec_constant_type {
    fn size(self) -> u8 {
        match self {
            Self::CLC_SPEC_CONSTANT_INT64
            | Self::CLC_SPEC_CONSTANT_UINT64
            | Self::CLC_SPEC_CONSTANT_DOUBLE => 8,
            Self::CLC_SPEC_CONSTANT_INT32
            | Self::CLC_SPEC_CONSTANT_UINT32
            | Self::CLC_SPEC_CONSTANT_FLOAT => 4,
            Self::CLC_SPEC_CONSTANT_INT16 | Self::CLC_SPEC_CONSTANT_UINT16 => 2,
            Self::CLC_SPEC_CONSTANT_INT8
            | Self::CLC_SPEC_CONSTANT_UINT8
            | Self::CLC_SPEC_CONSTANT_BOOL => 1,
            Self::CLC_SPEC_CONSTANT_UNKNOWN => 0,
        }
    }
}
