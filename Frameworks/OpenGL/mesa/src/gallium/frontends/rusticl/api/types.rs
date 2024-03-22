use crate::api::icd::CLResult;
use crate::api::icd::ReferenceCountedAPIPointer;
use crate::core::context::Context;
use crate::core::event::Event;
use crate::core::memory::Mem;
use crate::core::program::Program;
use crate::core::queue::Queue;

use rusticl_opencl_gen::*;

use std::borrow::Borrow;
use std::ffi::c_void;
use std::ffi::CStr;
use std::iter::Product;

macro_rules! cl_callback {
    ($cb:ident($fn_alias:ident) {
        $($p:ident : $ty:ty,)*
    }) => {
        pub type $fn_alias = unsafe extern "C" fn(
            $($p: $ty,)*
        );

        // INVARIANT:
        // All safety requirements on `func` and `data` documented on `$cb::new` are invariants.
        #[allow(dead_code)]
        pub struct $cb {
            func: $fn_alias,
            data: *mut c_void,
        }

        #[allow(dead_code)]
        impl $cb {
            /// Creates a new `$cb`. Returns `Err(CL_INVALID_VALUE)` if `func` is `None`.
            ///
            /// # SAFETY:
            ///
            /// If `func` is `None`, there are no safety requirements. Otherwise:
            ///
            /// - `func` must be a thread-safe fn.
            /// - Passing `data` as the last parameter to `func` must not cause unsoundness.
            /// - CreateContextCB: `func` must be soundly callable as documented on
            ///   [`clCreateContext`] in the OpenCL specification.
            /// - DeleteContextCB: `func` must be soundly callable as documented on
            ///   [`clSetContextDestructorCallback`] in the OpenCL specification.
            /// - EventCB: `func` must be soundly callable as documented on
            ///   [`clSetEventCallback`] in the OpenCL specification.
            /// - MemCB: `func` must be soundly callable as documented on
            ///   [`clSetMemObjectDestructorCallback`] in the OpenCL specification.
            /// - ProgramCB: `func` must be soundly callable as documented on
            ///   [`clBuildProgram`] in the OpenCL specification.
            /// - SVMFreeCb: `func` must be soundly callable as documented on
            ///   [`clEnqueueSVMFree`] in the OpenCL specification.
            ///
            /// [`clCreateContext`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clCreateContext
            /// [`clSetContextDestructorCallback`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clSetContextDestructorCallback
            /// [`clSetEventCallback`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clSetEventCallback
            /// [`clSetMemObjectDestructorCallback`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clSetMemObjectDestructorCallback
            /// [`clBuildProgram`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clBuildProgram
            /// [`clEnqueueSVMFree`]: https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clEnqueueSVMFree
            pub unsafe fn new(func: Option<$fn_alias>, data: *mut c_void) -> CLResult<Self> {
                let Some(func) = func else {
                    return Err(CL_INVALID_VALUE);
                };
                Ok(Self { func, data })
            }

            /// Creates a new Option(`$cb`). Returns:
            /// - `Ok(Some($cb)) if `func` is `Some(_)`.
            /// - `Ok(None)` if `func` is `None` and `data` is `null`.
            /// - `Err(CL_INVALID_VALUE)` if `func` is `None` and `data` is not `null`.
            ///
            /// # SAFETY:
            ///
            /// The safety requirements are identical to those of [`new`].
            pub unsafe fn try_new(func: Option<$fn_alias>, data: *mut c_void) -> CLResult<Option<Self>> {
                let Some(func) = func else {
                    return if data.is_null() {
                        Ok(None)
                    } else {
                        Err(CL_INVALID_VALUE)
                    };
                };
                Ok(Some(Self { func, data }))
            }
        }

        unsafe impl Send for $cb {}
        unsafe impl Sync for $cb {}
    }
}

cl_callback!(
    CreateContextCB(FuncCreateContextCB) {
        errinfo: *const ::std::os::raw::c_char,
        private_info: *const c_void,
        cb: usize,
        user_data: *mut c_void,
    }
);

impl CreateContextCB {
    pub fn _call(self, err_msg: &CStr, private_info: &[u8]) {
        let err_msg_ptr = err_msg.as_ptr();
        let private_info_ptr = private_info.as_ptr().cast::<c_void>();
        // SAFETY: The first parameter must be a valid pointer to a NUL-terminated C string. We
        // know this is satisfied since that is `CStr`'s type invariant.
        // The second parameter must be a valid pointer to binary data with the length given in the
        // thrid parameter. We know both of these are correct since we just got them from a byte slice.
        // All other requirements are covered by this callback's type invariants.
        unsafe { (self.func)(err_msg_ptr, private_info_ptr, private_info.len(), self.data) };
    }
}

cl_callback!(
    DeleteContextCB(FuncDeleteContextCB) {
        context: cl_context,
        user_data: *mut c_void,
    }
);

impl DeleteContextCB {
    pub fn call(self, ctx: &Context) {
        let cl = cl_context::from_ptr(ctx);
        // SAFETY: `cl` must have pointed to an OpenCL context, which is where we just got it from.
        // All other requirements are covered by this callback's type invariants.
        unsafe { (self.func)(cl, self.data) };
    }
}

cl_callback!(
    EventCB(FuncEventCB) {
        event: cl_event,
        event_command_status: cl_int,
        user_data: *mut c_void,
    }
);

impl EventCB {
    pub fn call(self, event: &Event, status: cl_int) {
        let cl = cl_event::from_ptr(event);
        // SAFETY: `cl` must be a valid pointer to an OpenCL event, which is where we just got it from.
        // All other requirements are covered by this callback's type invariants.
        unsafe { (self.func)(cl, status, self.data) };
    }
}

cl_callback!(
    MemCB(FuncMemCB) {
        memobj: cl_mem,
        user_data: *mut c_void,
    }
);

impl MemCB {
    pub fn call(self, mem: &Mem) {
        let cl = cl_mem::from_ptr(mem);
        // SAFETY: `cl` must have pointed to an OpenCL context, which is where we just got it from.
        // All other requirements are covered by this callback's type invariants.
        unsafe { (self.func)(cl, self.data) };
    }
}

cl_callback!(
    ProgramCB(FuncProgramCB) {
        program: cl_program,
        user_data: *mut c_void,
    }
);

impl ProgramCB {
    pub fn call(self, program: &Program) {
        let cl = cl_program::from_ptr(program);
        // SAFETY: `cl` must have pointed to an OpenCL program, which is where we just got it from.
        // All other requirements are covered by this callback's type invariants.
        unsafe { (self.func)(cl, self.data) };
    }
}

cl_callback!(
    SVMFreeCb(FuncSVMFreeCb) {
        queue: cl_command_queue,
        num_svm_pointers: cl_uint,
        svm_pointers: *mut *mut c_void,
        user_data: *mut c_void,
    }
);

impl SVMFreeCb {
    pub fn call(self, queue: &Queue, svm_pointers: &mut [*mut c_void]) {
        let cl = cl_command_queue::from_ptr(queue);
        // SAFETY: `cl` must be a valid pointer to an OpenCL queue, which is where we just got it from.
        // All other requirements are covered by this callback's type invariants.
        unsafe {
            (self.func)(
                cl,
                svm_pointers.len() as u32,
                svm_pointers.as_mut_ptr(),
                self.data,
            )
        };
    }
}

// a lot of APIs use 3 component vectors passed as C arrays
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct CLVec<T> {
    vals: [T; 3],
}

impl<T: Copy> CLVec<T> {
    pub fn new(vals: [T; 3]) -> Self {
        Self { vals: vals }
    }

    /// # Safety
    ///
    /// This function is intended for use around OpenCL vectors of size 3.
    /// Most commonly for `origin` and `region` API arguments.
    ///
    /// Using it for anything else is undefined.
    pub unsafe fn from_raw(v: *const T) -> Self {
        Self {
            vals: unsafe { *v.cast() },
        }
    }

    pub fn pixels<'a>(&'a self) -> T
    where
        T: Product<&'a T>,
    {
        self.vals.iter().product()
    }
}

impl CLVec<usize> {
    /// returns the offset of point in linear memory.
    pub fn calc_offset<T: Borrow<Self>>(point: T, pitch: [usize; 3]) -> usize {
        *point.borrow() * pitch
    }

    /// returns the scalar size of the described region in linear memory.
    pub fn calc_size<T: Borrow<Self>>(region: T, pitch: [usize; 3]) -> usize {
        (*region.borrow() - [0, 1, 1]) * pitch
    }

    pub fn calc_offset_size<T1: Borrow<Self>, T2: Borrow<Self>>(
        base: T1,
        region: T2,
        pitch: [usize; 3],
    ) -> (usize, usize) {
        (
            Self::calc_offset(base, pitch),
            Self::calc_size(region, pitch),
        )
    }
}

impl<T: Default + Copy> Default for CLVec<T> {
    fn default() -> Self {
        Self {
            vals: [T::default(); 3],
        }
    }
}

// provides a ton of functions
impl<T> std::ops::Deref for CLVec<T> {
    type Target = [T; 3];

    fn deref(&self) -> &Self::Target {
        &self.vals
    }
}

impl<T> std::ops::DerefMut for CLVec<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.vals
    }
}

impl<T: Copy + std::ops::Add<Output = T>> std::ops::Add for CLVec<T> {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        self + other.vals
    }
}

impl<T: Copy + std::ops::Add<Output = T>> std::ops::Add<[T; 3]> for CLVec<T> {
    type Output = Self;

    fn add(self, other: [T; 3]) -> Self {
        Self {
            vals: [self[0] + other[0], self[1] + other[1], self[2] + other[2]],
        }
    }
}

impl<T: Copy + std::ops::Sub<Output = T>> std::ops::Sub<[T; 3]> for CLVec<T> {
    type Output = Self;

    fn sub(self, other: [T; 3]) -> Self {
        Self {
            vals: [self[0] - other[0], self[1] - other[1], self[2] - other[2]],
        }
    }
}

impl<T> std::ops::Mul for CLVec<T>
where
    T: Copy + std::ops::Mul<Output = T> + std::ops::Add<Output = T>,
{
    type Output = T;

    fn mul(self, other: Self) -> T {
        self * other.vals
    }
}

impl<T> std::ops::Mul<[T; 3]> for CLVec<T>
where
    T: Copy + std::ops::Mul<Output = T> + std::ops::Add<Output = T>,
{
    type Output = T;

    fn mul(self, other: [T; 3]) -> T {
        self[0] * other[0] + self[1] * other[1] + self[2] * other[2]
    }
}

impl<S, T> TryInto<[T; 3]> for CLVec<S>
where
    S: Copy,
    T: TryFrom<S>,
    [T; 3]: TryFrom<Vec<T>>,
{
    type Error = cl_int;

    fn try_into(self) -> Result<[T; 3], cl_int> {
        let vec: Result<Vec<T>, _> = self
            .vals
            .iter()
            .map(|v| T::try_from(*v).map_err(|_| CL_OUT_OF_HOST_MEMORY))
            .collect();
        vec?.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)
    }
}

impl<T> From<[T; 3]> for CLVec<T>
where
    T: Copy,
{
    fn from(arr: [T; 3]) -> Self {
        Self::new(arr)
    }
}

#[allow(non_snake_case)]
pub mod IdpAccelProps {
    use rusticl_opencl_gen::cl_bool;
    use rusticl_opencl_gen::cl_device_integer_dot_product_acceleration_properties_khr;
    pub fn new(
        signed_accelerated: cl_bool,
        unsigned_accelerated: cl_bool,
        mixed_signedness_accelerated: cl_bool,
        accumulating_saturating_signed_accelerated: cl_bool,
        accumulating_saturating_unsigned_accelerated: cl_bool,
        accumulating_saturating_mixed_signedness_accelerated: cl_bool,
    ) -> cl_device_integer_dot_product_acceleration_properties_khr {
        cl_device_integer_dot_product_acceleration_properties_khr {
            signed_accelerated,
            unsigned_accelerated,
            mixed_signedness_accelerated,
            accumulating_saturating_signed_accelerated,
            accumulating_saturating_unsigned_accelerated,
            accumulating_saturating_mixed_signedness_accelerated,
        }
    }
}
