use crate::compiler::nir::NirShader;
use crate::pipe::context::*;
use crate::pipe::device::*;
use crate::pipe::resource::*;
use crate::util::disk_cache::*;

use mesa_rust_gen::*;
use mesa_rust_util::has_required_feature;
use mesa_rust_util::string::*;

use std::convert::TryInto;
use std::ffi::CStr;
use std::mem::size_of;
use std::os::raw::c_schar;
use std::os::raw::c_uchar;
use std::os::raw::c_void;
use std::ptr;
use std::sync::Arc;

#[derive(PartialEq)]
pub struct PipeScreen {
    ldev: PipeLoaderDevice,
    screen: *mut pipe_screen,
}

pub const UUID_SIZE: usize = PIPE_UUID_SIZE as usize;
const LUID_SIZE: usize = PIPE_LUID_SIZE as usize;

// until we have a better solution
pub trait ComputeParam<T> {
    fn compute_param(&self, cap: pipe_compute_cap) -> T;
}

macro_rules! compute_param_impl {
    ($ty:ty) => {
        impl ComputeParam<$ty> for PipeScreen {
            fn compute_param(&self, cap: pipe_compute_cap) -> $ty {
                let size = self.compute_param_wrapped(cap, ptr::null_mut());
                let mut d = [0; size_of::<$ty>()];
                if size == 0 {
                    return Default::default();
                }
                assert_eq!(size as usize, d.len());
                self.compute_param_wrapped(cap, d.as_mut_ptr().cast());
                <$ty>::from_ne_bytes(d)
            }
        }
    };
}

compute_param_impl!(u32);
compute_param_impl!(u64);

impl ComputeParam<Vec<u64>> for PipeScreen {
    fn compute_param(&self, cap: pipe_compute_cap) -> Vec<u64> {
        let size = self.compute_param_wrapped(cap, ptr::null_mut());
        let elems = (size / 8) as usize;

        let mut res: Vec<u64> = Vec::new();
        let mut d: Vec<u8> = vec![0; size as usize];

        self.compute_param_wrapped(cap, d.as_mut_ptr().cast());
        for i in 0..elems {
            let offset = i * 8;
            let slice = &d[offset..offset + 8];
            res.push(u64::from_ne_bytes(slice.try_into().expect("")));
        }
        res
    }
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ResourceType {
    Normal,
    Staging,
    Cb0,
}

impl ResourceType {
    fn apply(&self, tmpl: &mut pipe_resource, screen: &PipeScreen) {
        match self {
            Self::Staging => {
                tmpl.set_usage(pipe_resource_usage::PIPE_USAGE_STAGING.0);
                tmpl.flags |= PIPE_RESOURCE_FLAG_MAP_PERSISTENT | PIPE_RESOURCE_FLAG_MAP_COHERENT;
                tmpl.bind |= PIPE_BIND_LINEAR;
            }
            Self::Cb0 => {
                tmpl.flags |= screen.param(pipe_cap::PIPE_CAP_CONSTBUF0_FLAGS) as u32;
                tmpl.bind |= PIPE_BIND_CONSTANT_BUFFER;
            }
            Self::Normal => {}
        }
    }
}

impl PipeScreen {
    pub(super) fn new(ldev: PipeLoaderDevice, screen: *mut pipe_screen) -> Option<Self> {
        if screen.is_null() || !has_required_cbs(screen) {
            return None;
        }

        Some(Self { ldev, screen })
    }

    pub fn create_context(self: &Arc<Self>) -> Option<PipeContext> {
        PipeContext::new(
            unsafe {
                (*self.screen).context_create.unwrap()(
                    self.screen,
                    ptr::null_mut(),
                    PIPE_CONTEXT_COMPUTE_ONLY | PIPE_CONTEXT_NO_LOD_BIAS,
                )
            },
            self,
        )
    }

    fn resource_create(&self, tmpl: &pipe_resource) -> Option<PipeResource> {
        PipeResource::new(
            unsafe { (*self.screen).resource_create.unwrap()(self.screen, tmpl) },
            false,
        )
    }

    fn resource_create_from_user(
        &self,
        tmpl: &pipe_resource,
        mem: *mut c_void,
    ) -> Option<PipeResource> {
        unsafe {
            if let Some(func) = (*self.screen).resource_from_user_memory {
                PipeResource::new(func(self.screen, tmpl, mem), true)
            } else {
                None
            }
        }
    }

    pub fn resource_create_buffer(
        &self,
        size: u32,
        res_type: ResourceType,
        pipe_bind: u32,
    ) -> Option<PipeResource> {
        let mut tmpl = pipe_resource::default();

        tmpl.set_target(pipe_texture_target::PIPE_BUFFER);
        tmpl.width0 = size;
        tmpl.height0 = 1;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.bind = pipe_bind;

        res_type.apply(&mut tmpl, self);

        self.resource_create(&tmpl)
    }

    pub fn resource_create_buffer_from_user(
        &self,
        size: u32,
        mem: *mut c_void,
        pipe_bind: u32,
    ) -> Option<PipeResource> {
        let mut tmpl = pipe_resource::default();

        tmpl.set_target(pipe_texture_target::PIPE_BUFFER);
        tmpl.width0 = size;
        tmpl.height0 = 1;
        tmpl.depth0 = 1;
        tmpl.array_size = 1;
        tmpl.bind = pipe_bind;

        self.resource_create_from_user(&tmpl, mem)
    }

    pub fn resource_create_texture(
        &self,
        width: u32,
        height: u16,
        depth: u16,
        array_size: u16,
        target: pipe_texture_target,
        format: pipe_format,
        res_type: ResourceType,
        support_image: bool,
    ) -> Option<PipeResource> {
        let mut tmpl = pipe_resource::default();

        tmpl.set_target(target);
        tmpl.set_format(format);
        tmpl.width0 = width;
        tmpl.height0 = height;
        tmpl.depth0 = depth;
        tmpl.array_size = array_size;
        tmpl.bind = PIPE_BIND_SAMPLER_VIEW;

        if support_image {
            tmpl.bind |= PIPE_BIND_SHADER_IMAGE;
        }

        res_type.apply(&mut tmpl, self);

        self.resource_create(&tmpl)
    }

    pub fn resource_create_texture_from_user(
        &self,
        width: u32,
        height: u16,
        depth: u16,
        array_size: u16,
        target: pipe_texture_target,
        format: pipe_format,
        mem: *mut c_void,
        support_image: bool,
    ) -> Option<PipeResource> {
        let mut tmpl = pipe_resource::default();

        tmpl.set_target(target);
        tmpl.set_format(format);
        tmpl.width0 = width;
        tmpl.height0 = height;
        tmpl.depth0 = depth;
        tmpl.array_size = array_size;
        tmpl.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_LINEAR;

        if support_image {
            tmpl.bind |= PIPE_BIND_SHADER_IMAGE;
        }

        self.resource_create_from_user(&tmpl, mem)
    }

    pub fn resource_import_dmabuf(
        &self,
        handle: u32,
        modifier: u64,
        target: pipe_texture_target,
        format: pipe_format,
        stride: u32,
        width: u32,
        height: u16,
        depth: u16,
        array_size: u16,
    ) -> Option<PipeResource> {
        let mut tmpl = pipe_resource::default();
        let mut handle = winsys_handle {
            type_: WINSYS_HANDLE_TYPE_FD,
            handle: handle,
            modifier: modifier,
            format: format as u64,
            stride: stride,
            ..Default::default()
        };

        tmpl.set_target(target);
        tmpl.set_format(format);
        tmpl.width0 = width;
        tmpl.height0 = height;
        tmpl.depth0 = depth;
        tmpl.array_size = array_size;

        unsafe {
            PipeResource::new(
                (*self.screen).resource_from_handle.unwrap()(self.screen, &tmpl, &mut handle, 0),
                false,
            )
        }
    }

    pub fn param(&self, cap: pipe_cap) -> i32 {
        unsafe { (*self.screen).get_param.unwrap()(self.screen, cap) }
    }

    pub fn shader_param(&self, t: pipe_shader_type, cap: pipe_shader_cap) -> i32 {
        unsafe { (*self.screen).get_shader_param.unwrap()(self.screen, t, cap) }
    }

    fn compute_param_wrapped(&self, cap: pipe_compute_cap, ptr: *mut c_void) -> i32 {
        unsafe {
            (*self.screen).get_compute_param.unwrap()(
                self.screen,
                pipe_shader_ir::PIPE_SHADER_IR_NIR,
                cap,
                ptr,
            )
        }
    }

    pub fn driver_name(&self) -> String {
        self.ldev.driver_name()
    }

    pub fn name(&self) -> String {
        unsafe { c_string_to_string((*self.screen).get_name.unwrap()(self.screen)) }
    }

    pub fn device_node_mask(&self) -> Option<u32> {
        unsafe { Some((*self.screen).get_device_node_mask?(self.screen)) }
    }

    pub fn device_uuid(&self) -> Option<[c_uchar; UUID_SIZE]> {
        let mut uuid = [0; UUID_SIZE];
        let ptr = uuid.as_mut_ptr();
        unsafe {
            (*self.screen).get_device_uuid?(self.screen, ptr.cast());
        }

        Some(uuid)
    }

    pub fn device_luid(&self) -> Option<[c_uchar; LUID_SIZE]> {
        let mut luid = [0; LUID_SIZE];
        let ptr = luid.as_mut_ptr();
        unsafe { (*self.screen).get_device_luid?(self.screen, ptr.cast()) }

        Some(luid)
    }

    pub fn device_vendor(&self) -> String {
        unsafe { c_string_to_string((*self.screen).get_device_vendor.unwrap()(self.screen)) }
    }

    pub fn device_type(&self) -> pipe_loader_device_type {
        unsafe { *self.ldev.ldev }.type_
    }

    pub fn driver_uuid(&self) -> Option<[c_schar; UUID_SIZE]> {
        let mut uuid = [0; UUID_SIZE];
        let ptr = uuid.as_mut_ptr();
        unsafe {
            (*self.screen).get_driver_uuid?(self.screen, ptr.cast());
        }

        Some(uuid)
    }

    pub fn cl_cts_version(&self) -> &CStr {
        unsafe {
            let ptr = (*self.screen)
                .get_cl_cts_version
                .map_or(ptr::null(), |get_cl_cts_version| {
                    get_cl_cts_version(self.screen)
                });
            if ptr.is_null() {
                // this string is good enough to pass the CTS
                CStr::from_bytes_with_nul(b"v0000-01-01-00\0").unwrap()
            } else {
                CStr::from_ptr(ptr)
            }
        }
    }

    pub fn is_format_supported(
        &self,
        format: pipe_format,
        target: pipe_texture_target,
        bindings: u32,
    ) -> bool {
        unsafe {
            (*self.screen).is_format_supported.unwrap()(self.screen, format, target, 0, 0, bindings)
        }
    }

    pub fn get_timestamp(&self) -> u64 {
        // We have get_timestamp in has_required_cbs, so it will exist
        unsafe {
            (*self.screen)
                .get_timestamp
                .expect("get_timestamp should be required")(self.screen)
        }
    }

    pub fn is_res_handle_supported(&self) -> bool {
        unsafe {
            (*self.screen).resource_from_handle.is_some()
                && (*self.screen).resource_get_handle.is_some()
        }
    }

    pub fn nir_shader_compiler_options(
        &self,
        shader: pipe_shader_type,
    ) -> *const nir_shader_compiler_options {
        unsafe {
            (*self.screen).get_compiler_options.unwrap()(
                self.screen,
                pipe_shader_ir::PIPE_SHADER_IR_NIR,
                shader,
            )
            .cast()
        }
    }

    pub fn shader_cache(&self) -> Option<DiskCacheBorrowed> {
        let ptr = if let Some(func) = unsafe { *self.screen }.get_disk_shader_cache {
            unsafe { func(self.screen) }
        } else {
            ptr::null_mut()
        };

        DiskCacheBorrowed::from_ptr(ptr)
    }

    pub fn finalize_nir(&self, nir: &NirShader) {
        if let Some(func) = unsafe { *self.screen }.finalize_nir {
            unsafe {
                func(self.screen, nir.get_nir().cast());
            }
        }
    }

    pub(super) fn unref_fence(&self, mut fence: *mut pipe_fence_handle) {
        unsafe {
            (*self.screen).fence_reference.unwrap()(self.screen, &mut fence, ptr::null_mut());
        }
    }

    pub(super) fn fence_finish(&self, fence: *mut pipe_fence_handle) {
        unsafe {
            (*self.screen).fence_finish.unwrap()(
                self.screen,
                ptr::null_mut(),
                fence,
                OS_TIMEOUT_INFINITE as u64,
            );
        }
    }
}

impl Drop for PipeScreen {
    fn drop(&mut self) {
        unsafe {
            (*self.screen).destroy.unwrap()(self.screen);
        }
    }
}

fn has_required_cbs(screen: *mut pipe_screen) -> bool {
    let screen = unsafe { *screen };
    // Use '&' to evaluate all features and to not stop
    // on first missing one to list all missing features.
    has_required_feature!(screen, context_create)
        & has_required_feature!(screen, destroy)
        & has_required_feature!(screen, fence_finish)
        & has_required_feature!(screen, fence_reference)
        & has_required_feature!(screen, get_compiler_options)
        & has_required_feature!(screen, get_compute_param)
        & has_required_feature!(screen, get_name)
        & has_required_feature!(screen, get_param)
        & has_required_feature!(screen, get_shader_param)
        & has_required_feature!(screen, get_timestamp)
        & has_required_feature!(screen, is_format_supported)
        & has_required_feature!(screen, resource_create)
}
