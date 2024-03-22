.section #gf100_builtin_code
// DIV U32
//
// UNR recurrence (q = a / b):
// look for z such that 2^32 - b <= b * z < 2^32
// then q - 1 <= (a * z) / 2^32 <= q
//
// INPUT:   $r0: dividend, $r1: divisor
// OUTPUT:  $r0: result, $r1: modulus
// CLOBBER: $r2 - $r3, $p0 - $p1
// SIZE:    22 / 14 * 8 bytes
//
gf100_div_u32:
   bfind u32 $r2 $r1
   xor b32 $r2 $r2 0x1f
   mov b32 $r3 0x1
   shl b32 $r2 $r3 clamp $r2
   cvt u32 $r1 neg u32 $r1
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   cvt u32 $r2 neg u32 $r1
   add $r1 (mul u32 $r1 u32 $r0) $r3
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p0 set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   ret

// DIV S32, like DIV U32 after taking ABS(inputs)
//
// INPUT:   $r0: dividend, $r1: divisor
// OUTPUT:  $r0: result, $r1: modulus
// CLOBBER: $r2 - $r3, $p0 - $p3
//
gf100_div_s32:
   set $p2 0x1 lt s32 $r0 0x0
   set $p3 0x1 lt s32 $r1 0x0 xor $p2
   cvt s32 $r0 abs s32 $r0
   cvt s32 $r1 abs s32 $r1
   bfind u32 $r2 $r1
   xor b32 $r2 $r2 0x1f
   mov b32 $r3 0x1
   shl b32 $r2 $r3 clamp $r2
   cvt u32 $r1 neg u32 $r1
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   cvt u32 $r2 neg u32 $r1
   add $r1 (mul u32 $r1 u32 $r0) $r3
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p0 set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p3 cvt s32 $r0 neg s32 $r0
   $p2 cvt s32 $r1 neg s32 $r1
   ret

// RCP F64: Newton Raphson reciprocal(x): r_{i+1} = r_i * (2.0 - x * r_i)
//
// INPUT:   $r0d (x)
// OUTPUT:  $r0d (rcp(x))
// CLOBBER: $r2 - $r7
// SIZE:    9 * 8 bytes
//
gf100_rcp_f64:
   nop
   ret

// RSQ F64: Newton Raphson rsqrt(x): r_{i+1} = r_i * (1.5 - 0.5 * x * r_i * r_i)
//
// INPUT:   $r0d (x)
// OUTPUT:  $r0d (rsqrt(x))
// CLOBBER: $r2 - $r7
// SIZE:    14 * 8 bytes
//
gf100_rsq_f64:
   nop
   ret

.section #gf100_builtin_offsets
.b64 #gf100_div_u32
.b64 #gf100_div_s32
.b64 #gf100_rcp_f64
.b64 #gf100_rsq_f64
