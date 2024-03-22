use crate::pipe::context::*;

use mesa_rust_gen::*;

use std::ops::Deref;
use std::os::raw::c_void;
use std::ptr;

pub struct PipeTransfer {
    pipe: *mut pipe_transfer,
    res: *mut pipe_resource,
    ptr: *mut c_void,
    is_buffer: bool,
}

// SAFETY: Transfers are safe to send between threads
unsafe impl Send for PipeTransfer {}

pub struct GuardedPipeTransfer<'a> {
    inner: PipeTransfer,
    ctx: &'a PipeContext,
}

impl<'a> Deref for GuardedPipeTransfer<'a> {
    type Target = PipeTransfer;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<'a> Drop for GuardedPipeTransfer<'a> {
    fn drop(&mut self) {
        if self.is_buffer {
            self.ctx.buffer_unmap(self.inner.pipe);
        } else {
            self.ctx.texture_unmap(self.inner.pipe);
        }
        unsafe { pipe_resource_reference(&mut self.inner.res, ptr::null_mut()) };
    }
}

impl PipeTransfer {
    pub(super) fn new(is_buffer: bool, pipe: *mut pipe_transfer, ptr: *mut c_void) -> Self {
        let mut res: *mut pipe_resource = ptr::null_mut();
        unsafe { pipe_resource_reference(&mut res, (*pipe).resource) }

        Self {
            pipe: pipe,
            res: res,
            ptr: ptr,
            is_buffer: is_buffer,
        }
    }

    pub fn ptr(&self) -> *mut c_void {
        self.ptr
    }

    pub fn row_pitch(&self) -> u32 {
        unsafe { (*self.pipe).stride }
    }

    pub fn slice_pitch(&self) -> usize {
        unsafe { (*self.pipe).layer_stride }
    }

    pub fn bx(&self) -> &pipe_box {
        unsafe { &(*self.pipe).box_ }
    }

    pub fn with_ctx(self, ctx: &PipeContext) -> GuardedPipeTransfer {
        GuardedPipeTransfer {
            inner: self,
            ctx: ctx,
        }
    }
}

// use set_ctx before operating on the PipeTransfer inside a block where it gets droped
impl Drop for PipeTransfer {
    fn drop(&mut self) {
        assert_eq!(ptr::null_mut(), self.res);
    }
}
