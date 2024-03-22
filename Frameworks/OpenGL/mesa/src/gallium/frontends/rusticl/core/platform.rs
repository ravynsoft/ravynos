use crate::api::icd::CLResult;
use crate::api::icd::DISPATCH;
use crate::core::device::*;
use crate::core::version::*;

use mesa_rust_gen::*;
use rusticl_opencl_gen::*;

use std::env;
use std::sync::Arc;
use std::sync::Once;

#[repr(C)]
pub struct Platform {
    dispatch: &'static cl_icd_dispatch,
    pub devs: Vec<Arc<Device>>,
}

pub struct PlatformDebug {
    pub allow_invalid_spirv: bool,
    pub clc: bool,
    pub program: bool,
    pub sync_every_event: bool,
}

pub struct PlatformFeatures {
    pub fp16: bool,
    pub fp64: bool,
}

static PLATFORM_ENV_ONCE: Once = Once::new();
static PLATFORM_ONCE: Once = Once::new();

macro_rules! gen_cl_exts {
    (@COUNT $e:expr) => { 1 };
    (@COUNT $e:expr, $($es:expr),+) => { 1 + gen_cl_exts!(@COUNT $($es),*) };

    (@CONCAT $e:tt) => { $e };
    (@CONCAT $e:tt, $($es:tt),+) => { concat!($e, ' ', gen_cl_exts!(@CONCAT $($es),*)) };

    ([$(($major:expr, $minor:expr, $patch:expr, $ext:tt)$(,)?)+]) => {
        pub static PLATFORM_EXTENSION_STR: &str = concat!(gen_cl_exts!(@CONCAT $($ext),*));
        pub static PLATFORM_EXTENSIONS: [cl_name_version; gen_cl_exts!(@COUNT $($ext),*)] = [
            $(mk_cl_version_ext($major, $minor, $patch, $ext)),+
        ];
    }
}
gen_cl_exts!([
    (1, 0, 0, "cl_khr_byte_addressable_store"),
    (1, 0, 0, "cl_khr_create_command_queue"),
    (1, 0, 0, "cl_khr_expect_assume"),
    (1, 0, 0, "cl_khr_extended_versioning"),
    (1, 0, 0, "cl_khr_icd"),
    (1, 0, 0, "cl_khr_il_program"),
    (1, 0, 0, "cl_khr_spirv_no_integer_wrap_decoration"),
]);

static mut PLATFORM: Platform = Platform {
    dispatch: &DISPATCH,
    devs: Vec::new(),
};
static mut PLATFORM_DBG: PlatformDebug = PlatformDebug {
    allow_invalid_spirv: false,
    clc: false,
    program: false,
    sync_every_event: false,
};
static mut PLATFORM_FEATURES: PlatformFeatures = PlatformFeatures {
    fp16: false,
    fp64: false,
};

fn load_env() {
    let debug = unsafe { &mut PLATFORM_DBG };
    if let Ok(debug_flags) = env::var("RUSTICL_DEBUG") {
        for flag in debug_flags.split(',') {
            match flag {
                "allow_invalid_spirv" => debug.allow_invalid_spirv = true,
                "clc" => debug.clc = true,
                "program" => debug.program = true,
                "sync" => debug.sync_every_event = true,
                "" => (),
                _ => eprintln!("Unknown RUSTICL_DEBUG flag found: {}", flag),
            }
        }
    }

    let features = unsafe { &mut PLATFORM_FEATURES };
    if let Ok(feature_flags) = env::var("RUSTICL_FEATURES") {
        for flag in feature_flags.split(',') {
            match flag {
                "fp16" => features.fp16 = true,
                "fp64" => features.fp64 = true,
                "" => (),
                _ => eprintln!("Unknown RUSTICL_FEATURES flag found: {}", flag),
            }
        }
    }
}

impl Platform {
    pub fn as_ptr(&self) -> cl_platform_id {
        (self as *const Self) as cl_platform_id
    }

    pub fn get() -> &'static Self {
        debug_assert!(PLATFORM_ONCE.is_completed());
        // SAFETY: no mut references exist at this point
        unsafe { &PLATFORM }
    }

    pub fn dbg() -> &'static PlatformDebug {
        debug_assert!(PLATFORM_ENV_ONCE.is_completed());
        unsafe { &PLATFORM_DBG }
    }

    pub fn features() -> &'static PlatformFeatures {
        debug_assert!(PLATFORM_ENV_ONCE.is_completed());
        unsafe { &PLATFORM_FEATURES }
    }

    fn init(&mut self) {
        unsafe {
            glsl_type_singleton_init_or_ref();
        }

        self.devs = Device::all().collect();
    }

    pub fn init_once() {
        PLATFORM_ENV_ONCE.call_once(load_env);
        // SAFETY: no concurrent static mut access due to std::Once
        PLATFORM_ONCE.call_once(|| unsafe { PLATFORM.init() });
    }
}

impl Drop for Platform {
    fn drop(&mut self) {
        unsafe {
            glsl_type_singleton_decref();
        }
    }
}

pub trait GetPlatformRef {
    fn get_ref(&self) -> CLResult<&'static Platform>;
}

impl GetPlatformRef for cl_platform_id {
    fn get_ref(&self) -> CLResult<&'static Platform> {
        if !self.is_null() && *self == Platform::get().as_ptr() {
            Ok(Platform::get())
        } else {
            Err(CL_INVALID_PLATFORM)
        }
    }
}
