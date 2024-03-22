use crate::api::icd::CLResult;
use crate::api::types::*;
use crate::core::event::*;
use crate::core::queue::*;

use mesa_rust_util::properties::Properties;
use mesa_rust_util::ptr::CheckedPtr;
use rusticl_opencl_gen::*;

use std::cmp;
use std::convert::TryInto;
use std::ffi::CStr;
use std::ffi::CString;
use std::mem::{size_of, MaybeUninit};
use std::ops::BitAnd;
use std::slice;
use std::sync::Arc;

pub trait CLInfo<I> {
    fn query(&self, q: I, vals: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>>;

    fn get_info(
        &self,
        param_name: I,
        param_value_size: usize,
        param_value: *mut ::std::os::raw::c_void,
        param_value_size_ret: *mut usize,
    ) -> CLResult<()> {
        let arr = if !param_value.is_null() {
            unsafe { slice::from_raw_parts(param_value.cast(), param_value_size) }
        } else {
            &[]
        };

        let d = self.query(param_name, arr)?;
        let size: usize = d.len();

        // CL_INVALID_VALUE [...] if size in bytes specified by param_value_size is < size of return
        // type as specified in the Context Attributes table and param_value is not a NULL value.
        if param_value_size < size && !param_value.is_null() {
            return Err(CL_INVALID_VALUE);
        }

        // param_value_size_ret returns the actual size in bytes of data being queried by param_name.
        // If param_value_size_ret is NULL, it is ignored.
        param_value_size_ret.write_checked(size);

        // param_value is a pointer to memory where the appropriate result being queried is returned.
        // If param_value is NULL, it is ignored.
        unsafe {
            param_value.copy_checked(d.as_ptr().cast(), size);
        }

        Ok(())
    }
}

pub trait CLInfoObj<I, O> {
    fn query(&self, o: O, q: I) -> CLResult<Vec<MaybeUninit<u8>>>;

    fn get_info_obj(
        &self,
        obj: O,
        param_name: I,
        param_value_size: usize,
        param_value: *mut ::std::os::raw::c_void,
        param_value_size_ret: *mut usize,
    ) -> CLResult<()> {
        let d = self.query(obj, param_name)?;
        let size: usize = d.len();

        // CL_INVALID_VALUE [...] if size in bytes specified by param_value_size is < size of return
        // type as specified in the Context Attributes table and param_value is not a NULL value.
        if param_value_size < size && !param_value.is_null() {
            return Err(CL_INVALID_VALUE);
        }

        // param_value_size_ret returns the actual size in bytes of data being queried by param_name.
        // If param_value_size_ret is NULL, it is ignored.
        param_value_size_ret.write_checked(size);

        // param_value is a pointer to memory where the appropriate result being queried is returned.
        // If param_value is NULL, it is ignored.
        unsafe {
            param_value.copy_checked(d.as_ptr().cast(), size);
        }

        Ok(())
    }
}

pub trait CLProp {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>>;
}

macro_rules! cl_prop_for_type {
    ($ty: ty) => {
        impl CLProp for $ty {
            fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
                unsafe { slice::from_raw_parts((self as *const Self).cast(), size_of::<Self>()) }
                    .to_vec()
            }
        }
    };
}

cl_prop_for_type!(cl_char);
cl_prop_for_type!(cl_uchar);
cl_prop_for_type!(cl_ushort);
cl_prop_for_type!(cl_int);
cl_prop_for_type!(cl_uint);
cl_prop_for_type!(cl_ulong);
cl_prop_for_type!(isize);
cl_prop_for_type!(usize);

cl_prop_for_type!(cl_device_integer_dot_product_acceleration_properties_khr);
cl_prop_for_type!(cl_device_pci_bus_info_khr);
cl_prop_for_type!(cl_image_format);
cl_prop_for_type!(cl_name_version);

impl CLProp for bool {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        cl_prop::<cl_bool>(if *self { CL_TRUE } else { CL_FALSE })
    }
}

impl CLProp for &str {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        to_maybeuninit_vec(
            CString::new(*self)
                .unwrap_or_default()
                .into_bytes_with_nul(),
        )
    }
}

impl CLProp for &CStr {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        to_maybeuninit_vec(self.to_bytes_with_nul().to_vec())
    }
}

impl<T> CLProp for Vec<T>
where
    T: CLProp,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        let mut res: Vec<MaybeUninit<u8>> = Vec::new();
        for i in self {
            res.append(&mut i.cl_vec())
        }
        res
    }
}

impl<T> CLProp for &T
where
    T: CLProp,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        T::cl_vec(self)
    }
}

impl<T> CLProp for [T]
where
    T: CLProp,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        let mut res: Vec<MaybeUninit<u8>> = Vec::new();
        for i in self {
            res.append(&mut i.cl_vec())
        }
        res
    }
}

impl<T, const I: usize> CLProp for [T; I]
where
    T: CLProp,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        let mut res: Vec<MaybeUninit<u8>> = Vec::new();
        for i in self {
            res.append(&mut i.cl_vec())
        }
        res
    }
}

impl<T> CLProp for *const T {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        (*self as usize).cl_vec()
    }
}

impl<T> CLProp for *mut T {
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        (*self as usize).cl_vec()
    }
}

impl<T> CLProp for Properties<T>
where
    T: CLProp + Default,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        let mut res: Vec<MaybeUninit<u8>> = Vec::new();
        for (k, v) in &self.props {
            res.append(&mut k.cl_vec());
            res.append(&mut v.cl_vec());
        }
        res.append(&mut T::default().cl_vec());
        res
    }
}

impl<T> CLProp for Option<T>
where
    T: CLProp,
{
    fn cl_vec(&self) -> Vec<MaybeUninit<u8>> {
        self.as_ref().map_or(Vec::new(), |v| v.cl_vec())
    }
}

pub fn cl_prop<T: CLProp>(v: T) -> Vec<MaybeUninit<u8>>
where
    T: Sized,
{
    v.cl_vec()
}

const CL_DEVICE_TYPES: u32 = CL_DEVICE_TYPE_ACCELERATOR
    | CL_DEVICE_TYPE_CPU
    | CL_DEVICE_TYPE_GPU
    | CL_DEVICE_TYPE_CUSTOM
    | CL_DEVICE_TYPE_DEFAULT;

pub fn check_cl_device_type(val: cl_device_type) -> CLResult<()> {
    let v: u32 = val.try_into().or(Err(CL_INVALID_DEVICE_TYPE))?;
    if v == CL_DEVICE_TYPE_ALL || v & CL_DEVICE_TYPES == v {
        return Ok(());
    }
    Err(CL_INVALID_DEVICE_TYPE)
}

pub const CL_IMAGE_TYPES: [cl_mem_object_type; 6] = [
    CL_MEM_OBJECT_IMAGE1D,
    CL_MEM_OBJECT_IMAGE2D,
    CL_MEM_OBJECT_IMAGE3D,
    CL_MEM_OBJECT_IMAGE1D_ARRAY,
    CL_MEM_OBJECT_IMAGE2D_ARRAY,
    CL_MEM_OBJECT_IMAGE1D_BUFFER,
];

pub fn check_cl_bool<T: PartialEq + TryInto<cl_uint>>(val: T) -> Option<bool> {
    let c: u32 = val.try_into().ok()?;
    if c != CL_TRUE && c != CL_FALSE {
        return None;
    }
    Some(c == CL_TRUE)
}

pub fn event_list_from_cl(
    q: &Arc<Queue>,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
) -> CLResult<Vec<Arc<Event>>> {
    // CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and num_events_in_wait_list > 0, or
    // event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in
    // event_wait_list are not valid events.
    if event_wait_list.is_null() && num_events_in_wait_list > 0
        || !event_wait_list.is_null() && num_events_in_wait_list == 0
    {
        return Err(CL_INVALID_EVENT_WAIT_LIST);
    }

    let res = Event::from_cl_arr(event_wait_list, num_events_in_wait_list)
        .map_err(|_| CL_INVALID_EVENT_WAIT_LIST)?;

    // CL_INVALID_CONTEXT if context associated with command_queue and events in event_list are not
    // the same.
    if res.iter().any(|e| e.context != q.context) {
        return Err(CL_INVALID_CONTEXT);
    }

    Ok(res)
}

pub fn to_maybeuninit_vec<T: Copy>(v: Vec<T>) -> Vec<MaybeUninit<T>> {
    // In my tests the compiler was smart enough to turn this into a noop
    v.into_iter().map(MaybeUninit::new).collect()
}

pub fn checked_compare(a: usize, o: cmp::Ordering, b: u64) -> bool {
    if usize::BITS > u64::BITS {
        a.cmp(&(b as usize)) == o
    } else {
        (a as u64).cmp(&b) == o
    }
}

pub fn is_alligned<T>(ptr: *const T, alignment: usize) -> bool {
    ptr as usize & (alignment - 1) == 0
}

pub fn bit_check<A: BitAnd<Output = A> + PartialEq + Default, B: Into<A>>(a: A, b: B) -> bool {
    a & b.into() != A::default()
}

// Taken from "Appendix D: Checking for Memory Copy Overlap"
// src_offset and dst_offset are additions to support sub-buffers
pub fn check_copy_overlap(
    src_origin: &CLVec<usize>,
    src_offset: usize,
    dst_origin: &CLVec<usize>,
    dst_offset: usize,
    region: &CLVec<usize>,
    row_pitch: usize,
    slice_pitch: usize,
) -> bool {
    let slice_size = (region[1] - 1) * row_pitch + region[0];
    let block_size = (region[2] - 1) * slice_pitch + slice_size;
    let src_start =
        src_origin[2] * slice_pitch + src_origin[1] * row_pitch + src_origin[0] + src_offset;
    let src_end = src_start + block_size;
    let dst_start =
        dst_origin[2] * slice_pitch + dst_origin[1] * row_pitch + dst_origin[0] + dst_offset;
    let dst_end = dst_start + block_size;

    /* No overlap if dst ends before src starts or if src ends
     * before dst starts.
     */
    if (dst_end <= src_start) || (src_end <= dst_start) {
        return false;
    }

    /* No overlap if region[0] for dst or src fits in the gap
     * between region[0] and row_pitch.
     */
    {
        let src_dx = (src_origin[0] + src_offset) % row_pitch;
        let dst_dx = (dst_origin[0] + dst_offset) % row_pitch;
        if ((dst_dx >= src_dx + region[0]) && (dst_dx + region[0] <= src_dx + row_pitch))
            || ((src_dx >= dst_dx + region[0]) && (src_dx + region[0] <= dst_dx + row_pitch))
        {
            return false;
        }
    }

    /* No overlap if region[1] for dst or src fits in the gap
     * between region[1] and slice_pitch.
     */
    {
        let src_dy = (src_origin[1] * row_pitch + src_origin[0] + src_offset) % slice_pitch;
        let dst_dy = (dst_origin[1] * row_pitch + dst_origin[0] + dst_offset) % slice_pitch;
        if ((dst_dy >= src_dy + slice_size) && (dst_dy + slice_size <= src_dy + slice_pitch))
            || ((src_dy >= dst_dy + slice_size) && (src_dy + slice_size <= dst_dy + slice_pitch))
        {
            return false;
        }
    }

    /* Otherwise src and dst overlap. */
    true
}

pub mod cl_slice {
    use crate::api::util::CLResult;
    use mesa_rust_util::ptr::addr;
    use rusticl_opencl_gen::CL_INVALID_VALUE;
    use std::mem;
    use std::slice;

    /// Wrapper around [`std::slice::from_raw_parts`] that returns `Err(CL_INVALID_VALUE)` if any of these conditions is met:
    /// - `data` is null
    /// - `data` is not correctly aligned for `T`
    /// - `len * std::mem::size_of::<T>()` is larger than `isize::MAX`
    /// - `data` + `len * std::mem::size_of::<T>()` wraps around the address space
    ///
    /// # Safety
    /// The behavior is undefined if any of the other requirements imposed by
    /// [`std::slice::from_raw_parts`] is violated.
    #[inline]
    pub unsafe fn from_raw_parts<'a, T>(data: *const T, len: usize) -> CLResult<&'a [T]> {
        if allocation_obviously_invalid(data, len) {
            return Err(CL_INVALID_VALUE);
        }

        // SAFETY: We've checked that `data` is not null and properly aligned. We've also checked
        // that the total size in bytes does not exceed `isize::MAX` and that adding that size to
        // `data` does not wrap around the address space.
        //
        // The caller has to uphold the other safety requirements imposed by [`std::slice::from_raw_parts`].
        unsafe { Ok(slice::from_raw_parts(data, len)) }
    }

    /// Wrapper around [`std::slice::from_raw_parts_mut`] that returns `Err(CL_INVALID_VALUE)` if any of these conditions is met:
    /// - `data` is null
    /// - `data` is not correctly aligned for `T`
    /// - `len * std::mem::size_of::<T>()` is larger than `isize::MAX`
    /// - `data` + `len * std::mem::size_of::<T>()` wraps around the address space
    ///
    /// # Safety
    /// The behavior is undefined if any of the other requirements imposed by
    /// [`std::slice::from_raw_parts_mut`] is violated.
    #[inline]
    pub unsafe fn from_raw_parts_mut<'a, T>(data: *mut T, len: usize) -> CLResult<&'a mut [T]> {
        if allocation_obviously_invalid(data, len) {
            return Err(CL_INVALID_VALUE);
        }

        // SAFETY: We've checked that `data` is not null and properly aligned. We've also checked
        // that the total size in bytes does not exceed `isize::MAX` and that adding that size to
        // `data` does not wrap around the address space.
        //
        // The caller has to uphold the other safety requirements imposed by [`std::slice::from_raw_parts_mut`].
        unsafe { Ok(slice::from_raw_parts_mut(data, len)) }
    }

    #[must_use]
    fn allocation_obviously_invalid<T>(data: *const T, len: usize) -> bool {
        let Some(total_size) = mem::size_of::<T>().checked_mul(len) else {
            return true;
        };
        data.is_null()
            || !mesa_rust_util::ptr::is_aligned(data)
            || total_size > isize::MAX as usize
            || addr(data).checked_add(total_size).is_none()
    }
}
