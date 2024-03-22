use crate::api::icd::*;
use crate::api::types::DeleteContextCB;
use crate::core::device::*;
use crate::core::format::*;
use crate::core::gl::*;
use crate::core::memory::*;
use crate::core::util::*;
use crate::impl_cl_type_trait;

use mesa_rust::pipe::resource::*;
use mesa_rust::pipe::screen::ResourceType;
use mesa_rust_gen::*;
use mesa_rust_util::properties::Properties;
use rusticl_opencl_gen::*;

use std::alloc::Layout;
use std::collections::BTreeMap;
use std::collections::HashMap;
use std::convert::TryInto;
use std::mem;
use std::os::raw::c_void;
use std::sync::Arc;
use std::sync::Mutex;

pub struct Context {
    pub base: CLObjectBase<CL_INVALID_CONTEXT>,
    pub devs: Vec<&'static Device>,
    pub properties: Properties<cl_context_properties>,
    pub dtors: Mutex<Vec<DeleteContextCB>>,
    pub svm_ptrs: Mutex<BTreeMap<*const c_void, Layout>>,
    pub gl_ctx_manager: Option<GLCtxManager>,
}

impl_cl_type_trait!(cl_context, Context, CL_INVALID_CONTEXT);

impl Context {
    pub fn new(
        devs: Vec<&'static Device>,
        properties: Properties<cl_context_properties>,
        gl_ctx_manager: Option<GLCtxManager>,
    ) -> Arc<Context> {
        Arc::new(Self {
            base: CLObjectBase::new(),
            devs: devs,
            properties: properties,
            dtors: Mutex::new(Vec::new()),
            svm_ptrs: Mutex::new(BTreeMap::new()),
            gl_ctx_manager: gl_ctx_manager,
        })
    }

    pub fn create_buffer(
        &self,
        size: usize,
        user_ptr: *mut c_void,
        copy: bool,
        res_type: ResourceType,
    ) -> CLResult<HashMap<&'static Device, Arc<PipeResource>>> {
        let adj_size: u32 = size.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let mut res = HashMap::new();
        for &dev in &self.devs {
            let mut resource = None;

            if !user_ptr.is_null() && !copy {
                resource = dev.screen().resource_create_buffer_from_user(
                    adj_size,
                    user_ptr,
                    PIPE_BIND_GLOBAL,
                )
            }

            if resource.is_none() {
                resource = dev
                    .screen()
                    .resource_create_buffer(adj_size, res_type, PIPE_BIND_GLOBAL)
            }

            let resource = resource.ok_or(CL_OUT_OF_RESOURCES);
            res.insert(dev, Arc::new(resource?));
        }

        if !user_ptr.is_null() {
            res.iter()
                .filter(|(_, r)| copy || !r.is_user)
                .map(|(d, r)| {
                    d.helper_ctx()
                        .exec(|ctx| ctx.buffer_subdata(r, 0, user_ptr, size.try_into().unwrap()))
                })
                .for_each(|f| f.wait());
        }

        Ok(res)
    }

    pub fn create_texture(
        &self,
        desc: &cl_image_desc,
        format: &cl_image_format,
        user_ptr: *mut c_void,
        copy: bool,
        res_type: ResourceType,
    ) -> CLResult<HashMap<&'static Device, Arc<PipeResource>>> {
        let pipe_format = format.to_pipe_format().unwrap();

        let width = desc
            .image_width
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let height = desc
            .image_height
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let depth = desc
            .image_depth
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let array_size = desc
            .image_array_size
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let target = cl_mem_type_to_texture_target(desc.image_type);

        let mut res = HashMap::new();
        for &dev in &self.devs {
            let mut resource = None;
            let enable_bind_as_image =
                (dev.formats[format][&desc.image_type] as u32 & CL_MEM_WRITE_ONLY) != 0;

            // we can't specify custom pitches/slices, so this won't work for non 1D images
            if !user_ptr.is_null() && !copy && desc.image_type == CL_MEM_OBJECT_IMAGE1D {
                resource = dev.screen().resource_create_texture_from_user(
                    width,
                    height,
                    depth,
                    array_size,
                    target,
                    pipe_format,
                    user_ptr,
                    enable_bind_as_image,
                )
            }

            if resource.is_none() {
                resource = dev.screen().resource_create_texture(
                    width,
                    height,
                    depth,
                    array_size,
                    target,
                    pipe_format,
                    res_type,
                    enable_bind_as_image,
                )
            }

            let resource = resource.ok_or(CL_OUT_OF_RESOURCES);
            res.insert(dev, Arc::new(resource?));
        }

        if !user_ptr.is_null() {
            let bx = desc.bx()?;
            let stride = desc.row_pitch()?;
            let layer_stride = desc.slice_pitch();

            res.iter()
                .filter(|(_, r)| copy || !r.is_user)
                .map(|(d, r)| {
                    d.helper_ctx()
                        .exec(|ctx| ctx.texture_subdata(r, &bx, user_ptr, stride, layer_stride))
                })
                .for_each(|f| f.wait());
        }

        Ok(res)
    }

    /// Returns the max allocation size supported by all devices
    pub fn max_mem_alloc(&self) -> u64 {
        self.devs
            .iter()
            .map(|dev| dev.max_mem_alloc())
            .min()
            .unwrap()
    }

    pub fn has_svm_devs(&self) -> bool {
        self.devs.iter().any(|dev| dev.svm_supported())
    }

    pub fn add_svm_ptr(&self, ptr: *mut c_void, layout: Layout) {
        self.svm_ptrs.lock().unwrap().insert(ptr, layout);
    }

    pub fn find_svm_alloc(&self, ptr: *const c_void) -> Option<(*const c_void, Layout)> {
        let lock = self.svm_ptrs.lock().unwrap();
        if let Some((&base, layout)) = lock.range(..=ptr).next_back() {
            // SAFETY: we really just do some pointer math here...
            unsafe {
                // we check if ptr is within [base..base+size)
                // means we can check if ptr - (base + size) < 0
                if ptr.offset_from(base.add(layout.size())) < 0 {
                    return Some((base, *layout));
                }
            }
        }
        None
    }

    pub fn remove_svm_ptr(&self, ptr: *const c_void) -> Option<Layout> {
        self.svm_ptrs.lock().unwrap().remove(&ptr)
    }

    pub fn import_gl_buffer(
        &self,
        handle: u32,
        modifier: u64,
        image_type: cl_mem_object_type,
        gl_target: cl_GLenum,
        format: pipe_format,
        gl_props: GLMemProps,
    ) -> CLResult<HashMap<&'static Device, Arc<PipeResource>>> {
        let mut res = HashMap::new();
        let target = cl_mem_type_to_texture_target_gl(image_type, gl_target);

        for dev in &self.devs {
            let resource = dev
                .screen()
                .resource_import_dmabuf(
                    handle,
                    modifier,
                    target,
                    format,
                    gl_props.stride,
                    gl_props.width,
                    gl_props.height,
                    gl_props.depth,
                    gl_props.array_size,
                )
                .ok_or(CL_OUT_OF_RESOURCES)?;

            res.insert(*dev, Arc::new(resource));
        }

        Ok(res)
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        let cbs = mem::take(self.dtors.get_mut().unwrap());
        for cb in cbs.into_iter().rev() {
            cb.call(self);
        }
    }
}
