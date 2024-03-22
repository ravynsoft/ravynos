use std::mem;
use std::ptr;

pub trait CheckedPtr<T> {
    /// # Safety
    ///
    /// besides a null check the function can't make sure the pointer is valid
    /// for the entire size
    unsafe fn copy_checked(self, val: *const T, size: usize);
    fn write_checked(self, val: T);
}

impl<T> CheckedPtr<T> for *mut T {
    /// # Safety
    ///
    /// This function follows the same safety rules as `std::ptr::copy` except that it already
    /// checks for a NULL pointer.
    unsafe fn copy_checked(self, val: *const T, size: usize) {
        if !self.is_null() {
            // SAFETY: we move the responsibilities up to the caller
            unsafe {
                ptr::copy(val, self, size);
            }
        }
    }

    fn write_checked(self, val: T) {
        if !self.is_null() {
            unsafe {
                *self = val;
            }
        }
    }
}

// from https://internals.rust-lang.org/t/discussion-on-offset-of/7440/2
#[macro_export]
macro_rules! offset_of {
    ($Struct:path, $field:ident) => {{
        // Using a separate function to minimize unhygienic hazards
        // (e.g. unsafety of #[repr(packed)] field borrows).
        // Uncomment `const` when `const fn`s can juggle pointers.
        /*const*/
        fn offset() -> usize {
            let u = std::mem::MaybeUninit::<$Struct>::uninit();
            // Use pattern-matching to avoid accidentally going through Deref.
            let &$Struct { $field: ref f, .. } = unsafe { &*u.as_ptr() };
            let o = (f as *const _ as usize).wrapping_sub(&u as *const _ as usize);
            // Triple check that we are within `u` still.
            assert!((0..=std::mem::size_of_val(&u)).contains(&o));
            o
        }
        offset()
    }};
}

// Adapted from libstd since std::ptr::is_aligned is still unstable
// See https://github.com/rust-lang/rust/issues/96284
#[must_use]
#[inline]
pub const fn is_aligned<T>(ptr: *const T) -> bool
where
    T: Sized,
{
    let align = mem::align_of::<T>();
    addr(ptr) & (align - 1) == 0
}

// Adapted from libstd since std::ptr::addr is still unstable
// See https://github.com/rust-lang/rust/issues/95228
#[must_use]
#[inline(always)]
pub const fn addr<T>(ptr: *const T) -> usize {
    // The libcore implementations of `addr` and `expose_addr` suggest that, while both transmuting
    // and casting to usize will give you the address of a ptr in the end, they are not identical
    // in their side-effects.
    // A cast "exposes" a ptr, which can potentially cause the compiler to optimize less
    // aggressively around it.
    // Let's trust the libcore devs over clippy on whether a transmute also exposes a ptr.
    #[allow(clippy::transmutes_expressible_as_ptr_casts)]
    // SAFETY: Pointer-to-integer transmutes are valid (if you are okay with losing the
    // provenance).
    unsafe {
        mem::transmute(ptr.cast::<()>())
    }
}
