use std::convert::TryInto;
use std::mem::size_of;

pub fn read_ne_u8(input: &mut &[u8]) -> u8 {
    let (int_bytes, rest) = input.split_at(size_of::<u8>());
    *input = rest;
    u8::from_ne_bytes(int_bytes.try_into().unwrap())
}

pub fn read_ne_u32(input: &mut &[u8]) -> u32 {
    let (int_bytes, rest) = input.split_at(size_of::<u32>());
    *input = rest;
    u32::from_ne_bytes(int_bytes.try_into().unwrap())
}

pub fn read_ne_usize(input: &mut &[u8]) -> usize {
    let (int_bytes, rest) = input.split_at(size_of::<usize>());
    *input = rest;
    usize::from_ne_bytes(int_bytes.try_into().unwrap())
}

pub fn read_string(input: &mut &[u8], len: usize) -> Option<String> {
    let (string_bytes, rest) = input.split_at(len);
    *input = rest;
    String::from_utf8(string_bytes.to_vec()).ok()
}

/// Casts a &[T] to a [&u8] without copying.
/// Inspired by cast_slice from the bytemuck crate. Drop this copy once external crates are supported.
///
/// # Safety
///
/// T must not contain any uninitialized bytes such as padding.
#[inline]
pub unsafe fn as_byte_slice<T>(t: &[T]) -> &[u8] {
    let new_len = core::mem::size_of_val(t) / core::mem::size_of::<u8>();
    unsafe { core::slice::from_raw_parts(t.as_ptr() as *const u8, new_len) }
}
