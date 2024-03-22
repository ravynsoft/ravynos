use mesa_rust_gen::*;

use std::ptr;

#[derive(PartialEq, Eq, Hash)]
pub struct PipeResource {
    pipe: *mut pipe_resource,
    pub is_user: bool,
}

// SAFETY: pipe_resource is considered a thread safe type
unsafe impl Send for PipeResource {}
unsafe impl Sync for PipeResource {}

// Image dimensions provide by application to be used in both
// image and sampler views when image is created from buffer
#[derive(PartialEq, Eq)]
pub struct AppImgInfo {
    row_stride: u32,
    width: u32,
    height: u32,
}

impl AppImgInfo {
    pub fn new(row_stride: u32, width: u32, height: u32) -> AppImgInfo {
        AppImgInfo {
            row_stride: row_stride,
            width: width,
            height: height,
        }
    }
}

impl PipeResource {
    pub(super) fn new(res: *mut pipe_resource, is_user: bool) -> Option<Self> {
        if res.is_null() {
            return None;
        }

        Some(Self {
            pipe: res,
            is_user: is_user,
        })
    }

    pub(super) fn pipe(&self) -> *mut pipe_resource {
        self.pipe
    }

    fn as_ref(&self) -> &pipe_resource {
        unsafe { self.pipe.as_ref().unwrap() }
    }

    pub fn width(&self) -> u32 {
        unsafe { self.pipe.as_ref().unwrap().width0 }
    }

    pub fn height(&self) -> u16 {
        unsafe { self.pipe.as_ref().unwrap().height0 }
    }

    pub fn depth(&self) -> u16 {
        unsafe { self.pipe.as_ref().unwrap().depth0 }
    }

    pub fn array_size(&self) -> u16 {
        unsafe { self.pipe.as_ref().unwrap().array_size }
    }

    pub fn is_buffer(&self) -> bool {
        self.as_ref().target() == pipe_texture_target::PIPE_BUFFER
    }

    pub fn is_linear(&self) -> bool {
        self.as_ref().bind & PIPE_BIND_LINEAR != 0
    }

    pub fn is_staging(&self) -> bool {
        self.as_ref().usage() & pipe_resource_usage::PIPE_USAGE_STAGING.0 != 0
    }

    pub fn pipe_image_view(
        &self,
        format: pipe_format,
        read_write: bool,
        host_access: u16,
        app_img_info: Option<&AppImgInfo>,
    ) -> pipe_image_view {
        let u = if let Some(app_img_info) = app_img_info {
            pipe_image_view__bindgen_ty_1 {
                tex2d_from_buf: pipe_image_view__bindgen_ty_1__bindgen_ty_3 {
                    offset: 0,
                    row_stride: app_img_info.row_stride as u16,
                    width: app_img_info.width as u16,
                    height: app_img_info.height as u16,
                },
            }
        } else if self.is_buffer() {
            pipe_image_view__bindgen_ty_1 {
                buf: pipe_image_view__bindgen_ty_1__bindgen_ty_2 {
                    offset: 0,
                    size: self.as_ref().width0,
                },
            }
        } else {
            let mut tex = pipe_image_view__bindgen_ty_1__bindgen_ty_1::default();
            tex.set_level(0);
            tex.set_first_layer(0);
            if self.as_ref().target() == pipe_texture_target::PIPE_TEXTURE_3D {
                tex.set_last_layer((self.as_ref().depth0 - 1).into());
            } else if self.as_ref().array_size > 0 {
                tex.set_last_layer((self.as_ref().array_size - 1).into());
            } else {
                tex.set_last_layer(0);
            }

            pipe_image_view__bindgen_ty_1 { tex: tex }
        };

        let shader_access = if read_write {
            PIPE_IMAGE_ACCESS_READ_WRITE
        } else {
            PIPE_IMAGE_ACCESS_WRITE
        } as u16;

        let access = if app_img_info.is_some() {
            PIPE_IMAGE_ACCESS_TEX2D_FROM_BUFFER
        } else {
            0
        } as u16;

        pipe_image_view {
            resource: self.pipe(),
            format: format,
            access: access | host_access,
            shader_access: shader_access,
            u: u,
        }
    }

    pub fn pipe_sampler_view_template(
        &self,
        format: pipe_format,
        app_img_info: Option<&AppImgInfo>,
    ) -> pipe_sampler_view {
        let mut res = pipe_sampler_view::default();
        unsafe {
            u_sampler_view_default_template(&mut res, self.pipe, format);
        }

        if let Some(app_img_info) = app_img_info {
            res.u.tex2d_from_buf.offset = 0;
            res.u.tex2d_from_buf.row_stride = app_img_info.row_stride as u16;
            res.u.tex2d_from_buf.width = app_img_info.width as u16;
            res.u.tex2d_from_buf.height = app_img_info.height as u16;

            res.set_is_tex2d_from_buf(true);
        } else if res.target() == pipe_texture_target::PIPE_BUFFER {
            res.u.buf.offset = 0;
            res.u.buf.size = self.as_ref().width0;
        }

        res
    }
}

impl Drop for PipeResource {
    fn drop(&mut self) {
        unsafe { pipe_resource_reference(&mut self.pipe, ptr::null_mut()) }
    }
}
