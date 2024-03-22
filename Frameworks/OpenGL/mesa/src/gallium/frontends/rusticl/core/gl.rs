use crate::api::icd::*;
use crate::api::types::*;
use crate::core::context::*;
use crate::core::device::*;
use crate::core::format::*;
use crate::core::memory::*;
use crate::core::queue::*;
use crate::core::util::*;

use libc_rust_gen::{close, dlsym};
use rusticl_opencl_gen::*;

use mesa_rust::pipe::context::*;
use mesa_rust::pipe::fence::*;
use mesa_rust::pipe::resource::*;
use mesa_rust::pipe::screen::*;

use std::collections::HashMap;
use std::ffi::CStr;
use std::ffi::CString;
use std::mem;
use std::os::raw::c_void;
use std::ptr;
use std::sync::Arc;

type CLGLMappings = Option<HashMap<Arc<PipeResource>, Arc<PipeResource>>>;

pub struct XPlatManager {
    #[cfg(glx)]
    glx_get_proc_addr: PFNGLXGETPROCADDRESSPROC,
    egl_get_proc_addr: PFNEGLGETPROCADDRESSPROC,
}

impl Default for XPlatManager {
    fn default() -> Self {
        Self::new()
    }
}

impl XPlatManager {
    pub fn new() -> Self {
        Self {
            #[cfg(glx)]
            glx_get_proc_addr: Self::get_proc_address_func("glXGetProcAddress"),
            egl_get_proc_addr: Self::get_proc_address_func("eglGetProcAddress"),
        }
    }

    fn get_proc_address_func<T>(name: &str) -> T {
        let cname = CString::new(name).unwrap();
        unsafe {
            let pfn = dlsym(ptr::null_mut(), cname.as_ptr());
            mem::transmute_copy(&pfn)
        }
    }

    #[cfg(glx)]
    unsafe fn get_func_glx(&self, cname: &CStr) -> CLResult<__GLXextFuncPtr> {
        unsafe {
            Ok(self
                .glx_get_proc_addr
                .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?(
                cname.as_ptr().cast(),
            ))
        }
    }

    // in theory it should return CLResult<__GLXextFuncPtr> but luckily it's identical
    #[cfg(not(glx))]
    unsafe fn get_func_glx(&self, _: &CStr) -> CLResult<__eglMustCastToProperFunctionPointerType> {
        Err(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)
    }

    fn get_func<T>(&self, name: &str) -> CLResult<T> {
        let cname = CString::new(name).unwrap();
        unsafe {
            let raw_func = if name.starts_with("glX") {
                self.get_func_glx(&cname)?
            } else if name.starts_with("egl") {
                self.egl_get_proc_addr
                    .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?(
                    cname.as_ptr().cast()
                )
            } else {
                panic!();
            };

            Ok(mem::transmute_copy(&raw_func))
        }
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropEGLQueryDeviceInfo(
        &self,
    ) -> CLResult<PFNMESAGLINTEROPEGLQUERYDEVICEINFOPROC> {
        self.get_func::<PFNMESAGLINTEROPEGLQUERYDEVICEINFOPROC>("eglGLInteropQueryDeviceInfoMESA")
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropGLXQueryDeviceInfo(
        &self,
    ) -> CLResult<PFNMESAGLINTEROPGLXQUERYDEVICEINFOPROC> {
        self.get_func::<PFNMESAGLINTEROPGLXQUERYDEVICEINFOPROC>("glXGLInteropQueryDeviceInfoMESA")
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropEGLExportObject(&self) -> CLResult<PFNMESAGLINTEROPEGLEXPORTOBJECTPROC> {
        self.get_func::<PFNMESAGLINTEROPEGLEXPORTOBJECTPROC>("eglGLInteropExportObjectMESA")
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropGLXExportObject(&self) -> CLResult<PFNMESAGLINTEROPGLXEXPORTOBJECTPROC> {
        self.get_func::<PFNMESAGLINTEROPGLXEXPORTOBJECTPROC>("glXGLInteropExportObjectMESA")
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropEGLFlushObjects(&self) -> CLResult<PFNMESAGLINTEROPEGLFLUSHOBJECTSPROC> {
        self.get_func::<PFNMESAGLINTEROPEGLFLUSHOBJECTSPROC>("eglGLInteropFlushObjectsMESA")
    }

    #[allow(non_snake_case)]
    pub fn MesaGLInteropGLXFlushObjects(&self) -> CLResult<PFNMESAGLINTEROPGLXFLUSHOBJECTSPROC> {
        self.get_func::<PFNMESAGLINTEROPGLXFLUSHOBJECTSPROC>("glXGLInteropFlushObjectsMESA")
    }
}

#[allow(clippy::upper_case_acronyms)]
#[derive(PartialEq, Eq)]
enum GLCtx {
    EGL(EGLDisplay, EGLContext),
    GLX(*mut _XDisplay, *mut __GLXcontextRec),
}

pub struct GLCtxManager {
    pub interop_dev_info: mesa_glinterop_device_info,
    pub xplat_manager: XPlatManager,
    gl_ctx: GLCtx,
}

impl GLCtxManager {
    pub fn new(
        gl_context: *mut c_void,
        glx_display: *mut _XDisplay,
        egl_display: EGLDisplay,
    ) -> CLResult<Option<Self>> {
        let mut info = mesa_glinterop_device_info {
            version: 3,
            ..Default::default()
        };
        let xplat_manager = XPlatManager::new();

        // More than one of the attributes CL_CGL_SHAREGROUP_KHR, CL_EGL_DISPLAY_KHR,
        // CL_GLX_DISPLAY_KHR, and CL_WGL_HDC_KHR is set to a non-default value.
        if !egl_display.is_null() && !glx_display.is_null() {
            return Err(CL_INVALID_OPERATION);
        }

        if gl_context.is_null() {
            return Ok(None);
        }

        if !egl_display.is_null() {
            let egl_query_device_info_func = xplat_manager
                .MesaGLInteropEGLQueryDeviceInfo()?
                .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

            let err = unsafe {
                egl_query_device_info_func(egl_display.cast(), gl_context.cast(), &mut info)
            };

            if err != MESA_GLINTEROP_SUCCESS as i32 {
                return Err(interop_to_cl_error(err));
            }

            Ok(Some(GLCtxManager {
                gl_ctx: GLCtx::EGL(egl_display.cast(), gl_context),
                interop_dev_info: info,
                xplat_manager: xplat_manager,
            }))
        } else if !glx_display.is_null() && cfg!(glx) {
            let glx_query_device_info_func = xplat_manager
                .MesaGLInteropGLXQueryDeviceInfo()?
                .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

            let err = unsafe {
                glx_query_device_info_func(glx_display.cast(), gl_context.cast(), &mut info)
            };

            if err != MESA_GLINTEROP_SUCCESS as i32 {
                return Err(interop_to_cl_error(err));
            }

            Ok(Some(GLCtxManager {
                gl_ctx: GLCtx::GLX(glx_display.cast(), gl_context.cast()),
                interop_dev_info: info,
                xplat_manager: xplat_manager,
            }))
        } else {
            Err(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)
        }
    }

    pub fn export_object(
        &self,
        cl_ctx: &Arc<Context>,
        target: cl_GLenum,
        flags: u32,
        miplevel: cl_GLint,
        texture: cl_GLuint,
    ) -> CLResult<GLExportManager> {
        let xplat_manager = &self.xplat_manager;
        let mut export_in = mesa_glinterop_export_in {
            version: 2,
            target: target,
            obj: texture,
            miplevel: miplevel as u32,
            access: cl_to_interop_flags(flags),
            ..Default::default()
        };

        let mut export_out = mesa_glinterop_export_out {
            version: 2,
            ..Default::default()
        };

        let mut fd = -1;

        let mut flush_out = mesa_glinterop_flush_out {
            version: 1,
            fence_fd: &mut fd,
            ..Default::default()
        };

        let err = unsafe {
            match &self.gl_ctx {
                GLCtx::EGL(disp, ctx) => {
                    let egl_export_object_func = xplat_manager
                        .MesaGLInteropEGLExportObject()?
                        .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

                    let egl_flush_objects_func = xplat_manager
                        .MesaGLInteropEGLFlushObjects()?
                        .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

                    let err_flush = egl_flush_objects_func(
                        disp.cast(),
                        ctx.cast(),
                        1,
                        &mut export_in,
                        &mut flush_out,
                    );
                    // TODO: use fence_server_sync in ctx inside the queue thread
                    let fence_fd = FenceFd { fd };
                    cl_ctx.devs.iter().for_each(|dev| {
                        let fence = dev.helper_ctx().import_fence(&fence_fd);
                        fence.wait();
                    });

                    if err_flush != 0 {
                        err_flush
                    } else {
                        egl_export_object_func(
                            disp.cast(),
                            ctx.cast(),
                            &mut export_in,
                            &mut export_out,
                        )
                    }
                }
                GLCtx::GLX(disp, ctx) => {
                    let glx_export_object_func = xplat_manager
                        .MesaGLInteropGLXExportObject()?
                        .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

                    let glx_flush_objects_func = xplat_manager
                        .MesaGLInteropGLXFlushObjects()?
                        .ok_or(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)?;

                    let err_flush = glx_flush_objects_func(
                        disp.cast(),
                        ctx.cast(),
                        1,
                        &mut export_in,
                        &mut flush_out,
                    );
                    // TODO: use fence_server_sync in ctx inside the queue thread
                    let fence_fd = FenceFd { fd };
                    cl_ctx.devs.iter().for_each(|dev| {
                        let fence = dev.helper_ctx().import_fence(&fence_fd);
                        fence.wait();
                    });

                    if err_flush != 0 {
                        err_flush
                    } else {
                        glx_export_object_func(
                            disp.cast(),
                            ctx.cast(),
                            &mut export_in,
                            &mut export_out,
                        )
                    }
                }
            }
        };

        if err != MESA_GLINTEROP_SUCCESS as i32 {
            return Err(interop_to_cl_error(err));
        }

        // CL_INVALID_GL_OBJECT if bufobj is not a GL buffer object or is a GL buffer
        // object but does not have an existing data store or the size of the buffer is 0.
        if [GL_ARRAY_BUFFER, GL_TEXTURE_BUFFER].contains(&target) && export_out.buf_size == 0 {
            return Err(CL_INVALID_GL_OBJECT);
        }

        Ok(GLExportManager {
            export_in: export_in,
            export_out: export_out,
        })
    }
}

#[derive(Clone)]
pub struct GLMemProps {
    pub height: u16,
    pub depth: u16,
    pub width: u32,
    pub offset: u32,
    pub array_size: u16,
    pub pixel_size: u8,
    pub stride: u32,
}

impl GLMemProps {
    pub fn size(&self) -> usize {
        self.height as usize
            * self.depth as usize
            * self.array_size as usize
            * self.width as usize
            * self.pixel_size as usize
    }
}

pub struct GLExportManager {
    pub export_in: mesa_glinterop_export_in,
    pub export_out: mesa_glinterop_export_out,
}

impl GLExportManager {
    pub fn get_gl_mem_props(&self) -> CLResult<GLMemProps> {
        let pixel_size = if self.is_gl_buffer() {
            1
        } else {
            format_from_gl(self.export_out.internal_format)
                .ok_or(CL_OUT_OF_HOST_MEMORY)?
                .pixel_size()
                .unwrap()
        };

        let mut height = self.export_out.height as u16;
        let mut depth = self.export_out.depth as u16;
        let mut width = self.export_out.width;
        let mut array_size = 1;
        let mut offset = 0;

        // some fixups
        match self.export_in.target {
            GL_TEXTURE_1D_ARRAY => {
                array_size = height;
                height = 1;
                depth = 1;
            }
            GL_TEXTURE_2D_ARRAY => {
                array_size = depth;
                depth = 1;
            }
            GL_ARRAY_BUFFER | GL_TEXTURE_BUFFER => {
                array_size = 1;
                width = self.export_out.buf_size as u32;
                offset = self.export_out.buf_offset as u32;
                height = 1;
                depth = 1;
            }
            _ => {}
        }
        if is_cube_map_face(self.export_in.target) {
            array_size = 6;
        }

        Ok(GLMemProps {
            height: height,
            depth: depth,
            width: width,
            offset: offset,
            array_size: array_size,
            pixel_size: pixel_size,
            stride: self.export_out.stride,
        })
    }

    pub fn is_gl_buffer(&self) -> bool {
        self.export_out.internal_format == GL_NONE
    }
}

impl Drop for GLExportManager {
    fn drop(&mut self) {
        unsafe {
            close(self.export_out.dmabuf_fd);
        }
    }
}

pub struct GLObject {
    pub gl_object_target: cl_GLenum,
    pub gl_object_type: cl_gl_object_type,
    pub gl_object_name: cl_GLuint,
    pub shadow_map: CLGLMappings,
}

pub fn create_shadow_slice(
    cube_map: &HashMap<&'static Device, Arc<PipeResource>>,
    image_format: cl_image_format,
) -> CLResult<HashMap<&'static Device, Arc<PipeResource>>> {
    let mut slice = HashMap::new();

    for (dev, imported_gl_res) in cube_map {
        let width = imported_gl_res.width();
        let height = imported_gl_res.height();

        let shadow = dev
            .screen()
            .resource_create_texture(
                width,
                height,
                1,
                1,
                cl_mem_type_to_texture_target(CL_MEM_OBJECT_IMAGE2D),
                image_format.to_pipe_format().unwrap(),
                ResourceType::Normal,
                false,
            )
            .ok_or(CL_OUT_OF_HOST_MEMORY)?;

        slice.insert(*dev, Arc::new(shadow));
    }

    Ok(slice)
}

pub fn copy_cube_to_slice(
    q: &Arc<Queue>,
    ctx: &PipeContext,
    mem_objects: &[Arc<Mem>],
) -> CLResult<()> {
    for mem in mem_objects {
        let gl_obj = mem.gl_obj.as_ref().unwrap();
        if !is_cube_map_face(gl_obj.gl_object_target) {
            continue;
        }
        let width = mem.image_desc.image_width;
        let height = mem.image_desc.image_height;

        // Fill in values for doing the copy
        let idx = get_array_slice_idx(gl_obj.gl_object_target);
        let src_origin = CLVec::<usize>::new([0, 0, idx]);
        let dst_offset: [u32; 3] = [0, 0, 0];
        let region = CLVec::<usize>::new([width, height, 1]);
        let src_bx = create_pipe_box(src_origin, region, CL_MEM_OBJECT_IMAGE2D_ARRAY)?;

        let cl_res = mem.get_res_of_dev(q.device)?;
        let gl_res = gl_obj.shadow_map.as_ref().unwrap().get(cl_res).unwrap();

        ctx.resource_copy_region(gl_res.as_ref(), cl_res.as_ref(), &dst_offset, &src_bx);
    }

    Ok(())
}

pub fn copy_slice_to_cube(
    q: &Arc<Queue>,
    ctx: &PipeContext,
    mem_objects: &[Arc<Mem>],
) -> CLResult<()> {
    for mem in mem_objects {
        let gl_obj = mem.gl_obj.as_ref().unwrap();
        if !is_cube_map_face(gl_obj.gl_object_target) {
            continue;
        }
        let width = mem.image_desc.image_width;
        let height = mem.image_desc.image_height;

        // Fill in values for doing the copy
        let idx = get_array_slice_idx(gl_obj.gl_object_target) as u32;
        let src_origin = CLVec::<usize>::new([0, 0, 0]);
        let dst_offset: [u32; 3] = [0, 0, idx];
        let region = CLVec::<usize>::new([width, height, 1]);
        let src_bx = create_pipe_box(src_origin, region, CL_MEM_OBJECT_IMAGE2D_ARRAY)?;

        let cl_res = mem.get_res_of_dev(q.device)?;
        let gl_res = gl_obj.shadow_map.as_ref().unwrap().get(cl_res).unwrap();

        ctx.resource_copy_region(cl_res.as_ref(), gl_res.as_ref(), &dst_offset, &src_bx);
    }

    Ok(())
}

pub fn interop_to_cl_error(error: i32) -> CLError {
    match error.try_into().unwrap() {
        MESA_GLINTEROP_OUT_OF_RESOURCES => CL_OUT_OF_RESOURCES,
        MESA_GLINTEROP_OUT_OF_HOST_MEMORY => CL_OUT_OF_HOST_MEMORY,
        MESA_GLINTEROP_INVALID_OPERATION => CL_INVALID_OPERATION,
        MESA_GLINTEROP_INVALID_CONTEXT | MESA_GLINTEROP_INVALID_DISPLAY => {
            CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR
        }
        MESA_GLINTEROP_INVALID_TARGET | MESA_GLINTEROP_INVALID_OBJECT => CL_INVALID_GL_OBJECT,
        MESA_GLINTEROP_INVALID_MIP_LEVEL => CL_INVALID_MIP_LEVEL,
        _ => CL_OUT_OF_HOST_MEMORY,
    }
}

pub fn cl_to_interop_flags(flags: u32) -> u32 {
    match flags {
        CL_MEM_READ_WRITE => MESA_GLINTEROP_ACCESS_READ_WRITE,
        CL_MEM_READ_ONLY => MESA_GLINTEROP_ACCESS_READ_ONLY,
        CL_MEM_WRITE_ONLY => MESA_GLINTEROP_ACCESS_WRITE_ONLY,
        _ => 0,
    }
}

pub fn target_from_gl(target: u32) -> CLResult<(u32, u32)> {
    // CL_INVALID_IMAGE_FORMAT_DESCRIPTOR if the OpenGL texture
    // internal format does not map to a supported OpenCL image format.
    Ok(match target {
        GL_ARRAY_BUFFER => (CL_MEM_OBJECT_BUFFER, CL_GL_OBJECT_BUFFER),
        GL_TEXTURE_BUFFER => (CL_MEM_OBJECT_IMAGE1D_BUFFER, CL_GL_OBJECT_TEXTURE_BUFFER),
        GL_RENDERBUFFER => (CL_MEM_OBJECT_IMAGE2D, CL_GL_OBJECT_RENDERBUFFER),
        GL_TEXTURE_1D => (CL_MEM_OBJECT_IMAGE1D, CL_GL_OBJECT_TEXTURE1D),
        GL_TEXTURE_1D_ARRAY => (CL_MEM_OBJECT_IMAGE1D_ARRAY, CL_GL_OBJECT_TEXTURE1D_ARRAY),
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        | GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        | GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        | GL_TEXTURE_CUBE_MAP_POSITIVE_X
        | GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        | GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        | GL_TEXTURE_2D
        | GL_TEXTURE_RECTANGLE => (CL_MEM_OBJECT_IMAGE2D, CL_GL_OBJECT_TEXTURE2D),
        GL_TEXTURE_2D_ARRAY => (CL_MEM_OBJECT_IMAGE2D_ARRAY, CL_GL_OBJECT_TEXTURE2D_ARRAY),
        GL_TEXTURE_3D => (CL_MEM_OBJECT_IMAGE3D, CL_GL_OBJECT_TEXTURE3D),
        _ => return Err(CL_INVALID_VALUE),
    })
}

pub fn is_valid_gl_texture(target: u32) -> bool {
    matches!(
        target,
        GL_TEXTURE_1D
            | GL_TEXTURE_1D_ARRAY
            | GL_TEXTURE_BUFFER
            | GL_TEXTURE_2D_ARRAY
            | GL_TEXTURE_3D
    ) || is_valid_gl_texture_2d(target)
}

pub fn is_valid_gl_texture_2d(target: u32) -> bool {
    matches!(
        target,
        GL_TEXTURE_2D
            | GL_TEXTURE_RECTANGLE
            | GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            | GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            | GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
            | GL_TEXTURE_CUBE_MAP_POSITIVE_X
            | GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            | GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    )
}

pub fn get_array_slice_idx(target: u32) -> usize {
    match target {
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        | GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        | GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        | GL_TEXTURE_CUBE_MAP_POSITIVE_X
        | GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        | GL_TEXTURE_CUBE_MAP_POSITIVE_Z => (target - GL_TEXTURE_CUBE_MAP_POSITIVE_X) as usize,
        _ => 0,
    }
}

pub fn is_cube_map_face(target: u32) -> bool {
    matches!(
        target,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            | GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            | GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
            | GL_TEXTURE_CUBE_MAP_POSITIVE_X
            | GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            | GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    )
}
