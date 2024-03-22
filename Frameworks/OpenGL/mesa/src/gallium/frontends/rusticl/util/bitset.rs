pub fn test_bit(bitset: &[u32], bit: u32) -> bool {
    let idx = bit / 32;
    let test = bit % 32;

    bitset[idx as usize] & (1 << test) != 0
}
