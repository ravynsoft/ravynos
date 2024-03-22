.section #gk104_builtin_code
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
gk104_div_u32:
   sched 0x28 0x4 0x28 0x4 0x28 0x28 0x28
   bfind u32 $r2 $r1
   long xor b32 $r2 $r2 0x1f
   long mov b32 $r3 0x1
   shl b32 $r2 $r3 clamp $r2
   long cvt u32 $r1 neg u32 $r1
   long mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   sched 0x28 0x28 0x28 0x28 0x28 0x28 0x28
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   sched 0x4 0x28 0x4 0x28 0x28 0x2c 0x4
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   long cvt u32 $r2 neg u32 $r1
   long add $r1 (mul u32 $r1 u32 $r0) $r3
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   sched 0x28 0x2c 0x4 0x20 0x2e 0x28 0x20
   $p0 add b32 $r0 $r0 0x1
   $p0 set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   long ret

// DIV S32, like DIV U32 after taking ABS(inputs)
//
// INPUT:   $r0: dividend, $r1: divisor
// OUTPUT:  $r0: result, $r1: modulus
// CLOBBER: $r2 - $r3, $p0 - $p3
//
gk104_div_s32:
   set $p2 0x1 lt s32 $r0 0x0
   set $p3 0x1 lt s32 $r1 0x0 xor $p2
   sched 0x20 0x28 0x28 0x4 0x28 0x04 0x28
   long cvt s32 $r0 abs s32 $r0
   long cvt s32 $r1 abs s32 $r1
   bfind u32 $r2 $r1
   long xor b32 $r2 $r2 0x1f
   long mov b32 $r3 0x1
   shl b32 $r2 $r3 clamp $r2
   cvt u32 $r1 neg u32 $r1
   sched 0x28 0x28 0x28 0x28 0x28 0x28 0x28
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   sched 0x28 0x28 0x4 0x28 0x04 0x28 0x28
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   long cvt u32 $r2 neg u32 $r1
   long add $r1 (mul u32 $r1 u32 $r0) $r3
   sched 0x2c 0x04 0x28 0x2c 0x04 0x28 0x20
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p0 set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   long $p0 add b32 $r0 $r0 0x1
   long $p3 cvt s32 $r0 neg s32 $r0
   sched 0x04 0x2e 0x04 0x28 0x04 0x20 0x2c
   $p2 cvt s32 $r1 neg s32 $r1
   long ret

// SULDP [for each format]
// $r4d: address
// $r2: surface info (format)
// $p0: access predicate
// $p1, $p2: caching predicate (00: cv, 01: ca, 10: cg)
//
// RGBA32
$p1 suldgb b128 $r0q ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b128 $r0q cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b128 $r0q cv zero u8 g[$r4d] $r2 $p0
long ret
// RGBA16_UNORM
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b128 $r0q ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b128 $r0q cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b128 $r0q cv zero u8 g[$r4d] $r2 $p0
cvt rn f32 $r3 u16 1 $r1
cvt rn f32 $r2 u16 0 $r1
mul f32 $r3 $r3 0x37800074
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt rn f32 $r1 u16 1 $r0
mul f32 $r2 $r2 0x37800074
cvt rn f32 $r0 u16 0 $r0
mul f32 $r1 $r1 0x37800074
mul f32 $r0 $r0 0x37800074
long ret
// RGBA16_SNORM
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
cvt rn f32 $r3 s16 1 $r1
cvt rn f32 $r2 s16 0 $r1
mul f32 $r3 $r3 0x38000187
cvt rn f32 $r1 s16 1 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r2 $r2 0x38000187
cvt rn f32 $r0 s16 0 $r0
mul f32 $r1 $r1 0x38000187
mul f32 $r0 $r0 0x38000187
long ret
// RGBA16_SINT
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
cvt s32 $r3 s16 1 $r1
cvt s32 $r2 s16 0 $r1
cvt s32 $r1 s16 1 $r0
cvt s32 $r0 s16 0 $r0
long ret
// RGBA16_UINT
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
cvt u32 $r3 u16 1 $r1
cvt u32 $r2 u16 0 $r1
cvt u32 $r1 u16 1 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt u32 $r0 u16 0 $r0
long ret
// RGBA16_FLOAT
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
cvt f32 $r3 f16 $r1 1
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt f32 $r2 f16 $r1 0
cvt f32 $r1 f16 $r0 1
cvt f32 $r0 f16 $r0 0
long ret
// RG32_FLOAT
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r2 0x00000000
long mov b32 $r3 0x3f800000
long ret
// RG32_xINT
$p1 suldgb b64 $r0d ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b64 $r0d cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b64 $r0d cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r2 0x00000000
long mov b32 $r3 0x00000001
long ret
// RGB10A2_UNORM
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
ext u32 $r1 $r0 0x0a0a
long mov b32 $r3 0x3f800000
ext u32 $r2 $r0 0x0a14
long and b32 $r0 $r0 0x3ff
cvt rn f32 $r2 u16 0 $r2
cvt rn f32 $r1 u16 0 $r1
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r2 $r2 0x3a802007
cvt rn f32 $r0 u16 0 $r0
mul f32 $r1 $r1 0x3a802007
mul f32 $r0 $r0 0x3a802007
long ret
// RGB10A2_UINT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
ext u32 $r1 $r0 0x0a0a
long mov b32 $r3 0x00000001
ext u32 $r2 $r0 0x0a14
long and b32 $r0 $r0 0x3ff
long ret
// RGBA8_UNORM
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
cvt rn f32 $r3 u8 3 $r0
cvt rn f32 $r2 u8 2 $r0
mul f32 $r3 $r3 0x3b808081
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt rn f32 $r1 u8 1 $r0
mul f32 $r2 $r2 0x3b808081
cvt rn f32 $r0 u8 0 $r0
mul f32 $r1 $r1 0x3b808081
mul f32 $r0 $r0 0x3b808081
long ret
// RGBA8_SNORM
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
cvt rn f32 $r3 s8 3 $r0
cvt rn f32 $r2 s8 2 $r0
mul f32 $r3 $r3 0x3c010204
cvt rn f32 $r1 s8 1 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r2 $r2 0x3c010204
cvt rn f32 $r0 s8 0 $r0
mul f32 $r1 $r1 0x3c010204
mul f32 $r0 $r0 0x3c010204
long ret
// RGBA8_SINT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
cvt s32 $r3 s8 3 $r0
cvt s32 $r2 s8 2 $r0
cvt s32 $r1 s8 1 $r0
cvt s32 $r0 s8 0 $r0
long ret
// RGBA8_UINT
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
cvt u32 $r3 u8 3 $r0
cvt u32 $r2 u8 2 $r0
cvt u32 $r1 u8 1 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt u32 $r0 u8 0 $r0
long ret
// R5G6B5_UNORM
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
ext u32 $r1 $r0 0x0605
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long mov b32 $r3 0x3f800000
ext u32 $r2 $r0 0x050b
long and b32 $r0 $r0 0x1f
cvt rn f32 $r2 u8 0 $r2
cvt rn f32 $r1 u8 0 $r1
mul f32 $r2 $r2 0x3d042108
cvt rn f32 $r0 u8 0 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r1 $r1 0x3c820821
mul f32 $r0 $r0 0x3d042108
long ret
// R5G5B5X1_UNORM
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
ext u32 $r1 $r0 0x0505
ext u32 $r2 $r0 0x050a
long and b32 $r0 $r0 0x1f
long mov b32 $r3 0x3f800000
cvt rn f32 $r2 u8 0 $r2
cvt rn f32 $r1 u8 0 $r1
cvt rn f32 $r0 u8 0 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r2 $r2 0x3d042108
mul f32 $r1 $r1 0x3d042108
mul f32 $r0 $r0 0x3d042108
long ret
// RG16_UNORM
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
cvt rn f32 $r1 u16 1 $r0
cvt rn f32 $r0 u16 0 $r0
mul f32 $r1 $r1 0x37800074
mul f32 $r0 $r0 0x37800074
long mov b32 $r2 0x00000000
long mov b32 $r3 0x3f800000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long ret
// RG16_SNORM
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x3f800000
cvt rn f32 $r1 s16 1 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mov b32 $r2 0x00000000
cvt rn f32 $r0 s16 0 $r0
mul f32 $r1 $r1 0x38000187
mul f32 $r0 $r0 0x38000187
long ret
// RG16_SINT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x00000001
cvt s32 $r1 s16 1 $r0
mov b32 $r2 0x00000000
cvt s32 $r0 s16 0 $r0
long ret
// RG16_UINT
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x00000001
cvt u32 $r1 u16 1 $r0
mov b32 $r2 0x00000000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt u32 $r0 u16 0 $r0
long ret
// RG16_FLOAT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x3f800000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt f32 $r1 f16 $r0 1
mov b32 $r2 0x00000000
cvt f32 $r0 f16 $r0 0
long ret
// R32_FLOAT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x3f800000
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
long ret
// R32_xINT
$p1 suldgb b32 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb b32 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
long ret
// RG8_UNORM
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x3f800000
cvt rn f32 $r1 u8 1 $r0
mov b32 $r2 0x00000000
cvt rn f32 $r0 u8 0 $r0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r1 $r1 0x3b808081
mul f32 $r0 $r0 0x3b808081
long ret
// RG8_SNORM
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long mov b32 $r3 0x3f800000
cvt rn f32 $r1 s8 1 $r0
long mov b32 $r2 0x00000000
cvt rn f32 $r0 s8 0 $r0
mul f32 $r1 $r1 0x3c010204
mul f32 $r0 $r0 0x3c010204
long ret
// RG8_UINT
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
cvt u32 $r1 u8 1 $r0
long mov b32 $r2 0x00000000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt u32 $r0 u8 0 $r0
long ret
// RG8_SINT
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
cvt s32 $r1 s8 1 $r0
long mov b32 $r2 0x00000000
cvt s32 $r0 s8 0 $r0
long ret
// R16_UNORM
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x3f800000
cvt rn f32 $r0 u16 0 $r0
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
mul f32 $r0 $r0 0x37800074
long ret
// R16_SNORM
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x3f800000
cvt rn f32 $r0 s16 0 $r0
long mov b32 $r2 0x00000000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long mov b32 $r1 0x00000000
mul f32 $r0 $r0 0x38000187
long ret
// R16_SINT
$p1 suldgb s16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb s16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb s16 $r0 cv zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long mov b32 $r3 0x00000001
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
long ret
// R16_UINT
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
long ret
// R16_FLOAT
$p1 suldgb u16 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p2 suldgb u16 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u16 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x3f800000
long mov b32 $r2 0x00000000
cvt f32 $r0 f16 $r0 0
mov b32 $r1 0x00000000
long ret
// R8_UNORM
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u8 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u8 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u8 $r0 cv zero u8 g[$r4d] $r2 $p0
mov b32 $r3 0x3f800000
cvt rn f32 $r0 u8 0 $r0
mov b32 $r2 0x00000000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mul f32 $r0 $r0 0x3b808081
mov b32 $r1 0x00000000
long ret
// R8_SNORM
$p1 suldgb u8 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u8 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u8 $r0 cv zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
mov b32 $r3 0x3f800000
cvt rn f32 $r0 s8 0 $r0
mov b32 $r2 0x00000000
mul f32 $r0 $r0 0x3c010204
mov b32 $r1 0x00000000
long ret
// R8_SINT
$p1 suldgb s8 $r0 ca zero u8 g[$r4d] $r2 $p0
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
set $p1 0x1 $p1 xor not $p2
$p2 suldgb s8 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb s8 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
long ret
// R8_UINT
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
$p1 suldgb u8 $r0 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb u8 $r0 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb u8 $r0 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x00000001
long mov b32 $r2 0x00000000
long mov b32 $r1 0x00000000
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long ret
// R11G11B10_FLOAT TODO
$p1 suldgb b32 $r3 ca zero u8 g[$r4d] $r2 $p0
set $p1 0x1 $p1 xor not $p2
$p2 suldgb b32 $r3 cg zero u8 g[$r4d] $r2 $p0
$p1 suldgb b32 $r3 cv zero u8 g[$r4d] $r2 $p0
long mov b32 $r3 0x3f800000
long nop
sched 0x00 0x00 0x00 0x00 0x00 0x00 0x00
long nop
long ret


// RCP F64: Newton Raphson reciprocal(x): r_{i+1} = r_i * (2.0 - x * r_i)
//
// INPUT:   $r0d (x)
// OUTPUT:  $r0d (rcp(x))
// CLOBBER: $r2 - $r7
// SIZE:    9 * 8 bytes
//
gk104_rcp_f64:
   // Step 1: classify input according to exponent and value, and calculate
   // result for 0/inf/nan. $r2 holds the exponent value, which starts at
   // bit 52 (bit 20 of the upper half) and is 11 bits in length
   ext u32 $r2 $r1 0xb14
   add b32 $r3 $r2 0xffffffff
   joinat #rcp_rejoin
   // We want to check whether the exponent is 0 or 0x7ff (i.e. NaN, inf,
   // denorm, or 0). Do this by subtracting 1 from the exponent, which will
   // mean that it's > 0x7fd in those cases when doing unsigned comparison
   set $p0 0x1 gt u32 $r3 0x7fd
   // $r3: 0 for norms, 0x36 for denorms, -1 for others
   long mov b32 $r3 0x0
   sched 0x2f 0x04 0x2d 0x2b 0x2f 0x28 0x28
   join (not $p0) nop
   // Process all special values: NaN, inf, denorm, 0
   mov b32 $r3 0xffffffff
   // A number is NaN if its abs value is greater than or unordered with inf
   set $p0 0x1 gtu f64 abs $r0d 0x7ff0000000000000
   (not $p0) bra #rcp_inf_or_denorm_or_zero
   // NaN -> NaN, the next line sets the "quiet" bit of the result. This
   // behavior is both seen on the CPU and the blob
   join or b32 $r1 $r1 0x80000
rcp_inf_or_denorm_or_zero:
   and b32 $r4 $r1 0x7ff00000
   // Other values with nonzero in exponent field should be inf
   set $p0 0x1 eq s32 $r4 0x0
   sched 0x2b 0x04 0x2f 0x2d 0x2b 0x2f 0x20
   $p0 bra #rcp_denorm_or_zero
   // +/-Inf -> +/-0
   xor b32 $r1 $r1 0x7ff00000
   join mov b32 $r0 0x0
rcp_denorm_or_zero:
   set $p0 0x1 gtu f64 abs $r0d 0x0
   $p0 bra #rcp_denorm
   // +/-0 -> +/-Inf
   join or b32 $r1 $r1 0x7ff00000
rcp_denorm:
   // non-0 denorms: multiply with 2^54 (the 0x36 in $r3), join with norms
   mul rn f64 $r0d $r0d 0x4350000000000000
   sched 0x2f 0x28 0x2b 0x28 0x28 0x04 0x28
   join mov b32 $r3 0x36
rcp_rejoin:
   // All numbers with -1 in $r3 have their result ready in $r0d, return them
   // others need further calculation
   set $p0 0x1 lt s32 $r3 0x0
   $p0 bra #rcp_end
   // Step 2: Before the real calculation goes on, renormalize the values to
   // range [1, 2) by setting exponent field to 0x3ff (the exponent of 1)
   // result in $r6d. The exponent will be recovered later.
   ext u32 $r2 $r1 0xb14
   and b32 $r7 $r1 0x800fffff
   add b32 $r7 $r7 0x3ff00000
   long mov b32 $r6 $r0
   sched 0x2b 0x04 0x28 0x28 0x2a 0x2b 0x2e
   // Step 3: Convert new value to float (no overflow will occur due to step
   // 2), calculate rcp and do newton-raphson step once
   cvt rz f32 $r5 f64 $r6d
   long rcp f32 $r4 $r5
   mov b32 $r0 0xbf800000
   fma rn f32 $r5 $r4 $r5 $r0
   fma rn f32 $r0 neg $r4 $r5 $r4
   // Step 4: convert result $r0 back to double, do newton-raphson steps
   cvt f64 $r0d f32 $r0
   cvt f64 $r6d neg f64 $r6d
   sched 0x2e 0x29 0x29 0x29 0x29 0x29 0x29
   cvt f64 $r8d f32 0x3f800000
   // 4 Newton-Raphson Steps, tmp in $r4d, result in $r0d
   // The formula used here (and above) is:
   //     RCP_{n + 1} = 2 * RCP_{n} - x * RCP_{n} * RCP_{n}
   // The following code uses 2 FMAs for each step, and it will basically
   // looks like:
   //     tmp = -src * RCP_{n} + 1
   //     RCP_{n + 1} = RCP_{n} * tmp + RCP_{n}
   fma rn f64 $r4d $r6d $r0d $r8d
   fma rn f64 $r0d $r0d $r4d $r0d
   fma rn f64 $r4d $r6d $r0d $r8d
   fma rn f64 $r0d $r0d $r4d $r0d
   fma rn f64 $r4d $r6d $r0d $r8d
   fma rn f64 $r0d $r0d $r4d $r0d
   sched 0x29 0x20 0x28 0x28 0x28 0x28 0x28
   fma rn f64 $r4d $r6d $r0d $r8d
   fma rn f64 $r0d $r0d $r4d $r0d
   // Step 5: Exponent recovery and final processing
   // The exponent is recovered by adding what we added to the exponent.
   // Suppose we want to calculate rcp(x), but we have rcp(cx), then
   //     rcp(x) = c * rcp(cx)
   // The delta in exponent comes from two sources:
   //   1) The renormalization in step 2. The delta is:
   //      0x3ff - $r2
   //   2) (For the denorm input) The 2^54 we multiplied at rcp_denorm, stored
   //      in $r3
   // These 2 sources are calculated in the first two lines below, and then
   // added to the exponent extracted from the result above.
   // Note that after processing, the new exponent may >= 0x7ff (inf)
   // or <= 0 (denorm). Those cases will be handled respectively below
   subr b32 $r2 $r2 0x3ff
   long add b32 $r4 $r2 $r3
   ext u32 $r3 $r1 0xb14
   // New exponent in $r3
   long add b32 $r3 $r3 $r4
   add b32 $r2 $r3 0xffffffff
   sched 0x28 0x2b 0x28 0x2b 0x28 0x28 0x2b
   // (exponent-1) < 0x7fe (unsigned) means the result is in norm range
   // (same logic as in step 1)
   set $p0 0x1 lt u32 $r2 0x7fe
   (not $p0) bra #rcp_result_inf_or_denorm
   // Norms: convert exponents back and return
   shl b32 $r4 $r4 clamp 0x14
   long add b32 $r1 $r4 $r1
   bra #rcp_end
rcp_result_inf_or_denorm:
   // New exponent >= 0x7ff means that result is inf
   set $p0 0x1 ge s32 $r3 0x7ff
   (not $p0) bra #rcp_result_denorm
   sched 0x20 0x25 0x28 0x2b 0x23 0x25 0x2f
   // Infinity
   and b32 $r1 $r1 0x80000000
   long mov b32 $r0 0x0
   add b32 $r1 $r1 0x7ff00000
   bra #rcp_end
rcp_result_denorm:
   // Denorm result comes from huge input. The greatest possible fp64, i.e.
   // 0x7fefffffffffffff's rcp is 0x0004000000000000, 1/4 of the smallest
   // normal value. Other rcp result should be greater than that. If we
   // set the exponent field to 1, we can recover the result by multiplying
   // it with 1/2 or 1/4. 1/2 is used if the "exponent" $r3 is 0, otherwise
   // 1/4 ($r3 should be -1 then). This is quite tricky but greatly simplifies
   // the logic here.
   set $p0 0x1 ne u32 $r3 0x0
   and b32 $r1 $r1 0x800fffff
   // 0x3e800000: 1/4
   $p0 cvt f64 $r6d f32 0x3e800000
   sched 0x2f 0x28 0x2c 0x2e 0x2a 0x20 0x27
   // 0x3f000000: 1/2
   (not $p0) cvt f64 $r6d f32 0x3f000000
   add b32 $r1 $r1 0x00100000
   mul rn f64 $r0d $r0d $r6d
rcp_end:
   long ret

// RSQ F64: Newton Raphson rsqrt(x): r_{i+1} = r_i * (1.5 - 0.5 * x * r_i * r_i)
//
// INPUT:   $r0d (x)
// OUTPUT:  $r0d (rsqrt(x))
// CLOBBER: $r2 - $r7
// SIZE:    14 * 8 bytes
//
gk104_rsq_f64:
   // Before getting initial result rsqrt64h, two special cases should be
   // handled first.
   // 1. NaN: set the highest bit in mantissa so it'll be surely recognized
   //    as NaN in rsqrt64h
   set $p0 0x1 gtu f64 abs $r0d 0x7ff0000000000000
   $p0 or b32 $r1 $r1 0x00080000
   and b32 $r2 $r1 0x7fffffff
   sched 0x27 0x20 0x28 0x2c 0x25 0x28 0x28
   // 2. denorms and small normal values: using their original value will
   //    lose precision either at rsqrt64h or the first step in newton-raphson
   //    steps below. Take 2 as a threshold in exponent field, and multiply
   //    with 2^54 if the exponent is smaller or equal. (will multiply 2^27
   //    to recover in the end)
   ext u32 $r3 $r1 0xb14
   set $p1 0x1 le u32 $r3 0x2
   long or b32 $r2 $r0 $r2
   $p1 mul rn f64 $r0d $r0d 0x4350000000000000
   rsqrt64h $r5 $r1
   // rsqrt64h will give correct result for 0/inf/nan, the following logic
   // checks whether the input is one of those (exponent is 0x7ff or all 0
   // except for the sign bit)
   set b32 $r6 ne u32 $r3 0x7ff
   long and b32 $r2 $r2 $r6
   sched 0x28 0x2b 0x20 0x27 0x28 0x2e 0x28
   set $p0 0x1 ne u32 $r2 0x0
   $p0 bra #rsq_norm
   // For 0/inf/nan, make sure the sign bit agrees with input and return
   and b32 $r1 $r1 0x80000000
   long mov b32 $r0 0x0
   long or b32 $r1 $r1 $r5
   long ret
rsq_norm:
   // For others, do 4 Newton-Raphson steps with the formula:
   //     RSQ_{n + 1} = RSQ_{n} * (1.5 - 0.5 * x * RSQ_{n} * RSQ_{n})
   // In the code below, each step is written as:
   //     tmp1 = 0.5 * x * RSQ_{n}
   //     tmp2 = -RSQ_{n} * tmp1 + 0.5
   //     RSQ_{n + 1} = RSQ_{n} * tmp2 + RSQ_{n}
   long mov b32 $r4 0x0
   sched 0x2f 0x29 0x29 0x29 0x29 0x29 0x29
   // 0x3f000000: 1/2
   cvt f64 $r8d f32 0x3f000000
   mul rn f64 $r2d $r0d $r8d
   mul rn f64 $r0d $r2d $r4d
   fma rn f64 $r6d neg $r4d $r0d $r8d
   fma rn f64 $r4d $r4d $r6d $r4d
   mul rn f64 $r0d $r2d $r4d
   fma rn f64 $r6d neg $r4d $r0d $r8d
   sched 0x29 0x29 0x29 0x29 0x29 0x29 0x29
   fma rn f64 $r4d $r4d $r6d $r4d
   mul rn f64 $r0d $r2d $r4d
   fma rn f64 $r6d neg $r4d $r0d $r8d
   fma rn f64 $r4d $r4d $r6d $r4d
   mul rn f64 $r0d $r2d $r4d
   fma rn f64 $r6d neg $r4d $r0d $r8d
   fma rn f64 $r4d $r4d $r6d $r4d
   sched 0x29 0x20 0x28 0x2e 0x00 0x00 0x00
   // Multiply 2^27 to result for small inputs to recover
   $p1 mul rn f64 $r4d $r4d 0x41a0000000000000
   long mov b32 $r1 $r5
   long mov b32 $r0 $r4
   long ret

//
// Trap handler.
// Requires at least 4 GPRs and 32 bytes of l[] memory to temporarily save GPRs.
// Low 32 bytes of l[] memory shouldn't be used if resumability is required.
//
// Trap info:
// 0x000: mutex
// 0x004: PC
// 0x008: trapstat
// 0x00c: warperr
// 0x010: tidx
// 0x014: tidy
// 0x018: tidz
// 0x01c: ctaidx
// 0x020: ctaidy
// 0x024: ctaidz
// 0x030: $r0q
// 0x130: $flags
// 0x140: s[]
//
st b128 wb l[0x00] $r0q
// check state of the warp and continue if it didn't cause the trap
long mov b32 $r1 $trapstat
long mov b32 $r3 $warperr
mov $r2 $flags mask 0xffff
and b32 0 $c $r1 $r3
e $c bra #end_cont
// spill control flow stack to l[]
long mov b32 $r3 16
spill_cfstack:
preret #end_exit
sub b32 $r3 $c $r3 0x1
lg $c bra #spill_cfstack
// retrieve pointer to trap info
mov b32 $r0 c0[0x1900]
mov b32 $r1 c0[0x1904]
// we only let a single faulting thread store its state
mov b32 $r3 0x1
exch b32 $r3 g[$r0d] $r3
joinat #end_exit
set $p0 0x1 eq u32 $r3 0x1
join $p0 nop
// store $c and $p registers
st b32 wb g[$r0d+0x130] $r2
// store $trapstat and $warperr
long mov b32 $r2 $trapstat
long mov b32 $r3 $warperr
st b64 wb g[$r0d+0x8] $r2d
// store registers
st b128 wb g[$r0d+0x40] $r4q
st b128 wb g[$r0d+0x50] $r8q
st b128 wb g[$r0d+0x60] $r12q
st b128 wb g[$r0d+0x70] $r16q
st b128 wb g[$r0d+0x80] $r20q
st b128 wb g[$r0d+0x90] $r24q
st b128 wb g[$r0d+0xa0] $r28q
st b128 wb g[$r0d+0xb0] $r32q
st b128 wb g[$r0d+0xc0] $r36q
st b128 wb g[$r0d+0xd0] $r40q
st b128 wb g[$r0d+0xe0] $r44q
st b128 wb g[$r0d+0xf0] $r48q
st b128 wb g[$r0d+0x100] $r52q
st b128 wb g[$r0d+0x110] $r56q
st b128 wb g[$r0d+0x120] $r60q
ld b64 $r2d cs l[0x0]
st b64 wb g[$r0d+0x30] $r2d
ld b64 $r2d cs l[0x8]
st b64 wb g[$r0d+0x38] $r2d
// store thread id
long mov b32 $r2 $tidx
long mov b32 $r3 $tidy
st b64 wb g[$r0d+0x10] $r2d
long mov b32 $r2 $tidz
long mov b32 $r3 $ctaidx
st b64 wb g[$r0d+0x18] $r2d
long mov b32 $r2 $ctaidy
long mov b32 $r3 $ctaidz
st b64 wb g[$r0d+0x20] $r2d
// store shared memory (in reverse order so $r0d is base again at the end)
long mov b32 $r3 $smemsz
sub b32 $r3 $c $r3 0x4
s $c bra #shared_done
add b32 $r0 $c $r0 $r3
add b32 $r1 $r1 0x0 $c
shared_loop:
long ld b32 $r2 s[$r3]
long st b32 wb g[$r0d+0x140] $r2
sub b32 $r0 $c $r0 0x4
sub b32 $r1 $r1 0x0 $c
sub b32 $r3 $c $r3 0x4
lg $c bra #shared_loop
shared_done:
// search the stack for trap entry to retrieve PC
mov b32 $r0 c0[0x1908]
mov b32 $r1 c0[0x190c]
membar sys
// invalidate caches so we can read stack entries via g[]
cctl ivall 0 l[0]
cctl ivall 0 g[$r0d]
// get offsets
mov b32 $r2 $physid
ext u32 $r3 $r2 0x0814 // MP id
ext u32 $r2 $r2 0x0608 // warp id
mul $r2 u32 $r2 u32 c0[0x1914] // warp offset
mul $r3 u32 $r3 u32 c0[0x1910] // MP offset
add b32 $r2 $r2 $r3 // MP + warp offset
add b32 $r0 $c $r0 $r2
add b32 $r1 $r1 0x0 $c
search_cstack:
mov b32 $r3 c0[0x1918] // cstack size
ld u8 $r2 cv g[$r0d+0x8]
set $p0 0x1 eq u32 $r2 0xa
$p0 bra #entry_found
add b32 $r0 $c $r0 0x10
add b32 $r1 $r1 0x0 $c
sub b32 $r3 $c $r3 0x10
lg $c bra #search_cstack
bra #end_exit
entry_found:
// load PC (may be unaligned and spread out)
ld b32 $r2 cv g[$r0d]
mov b32 $r0 c0[0x1900]
mov b32 $r1 c0[0x1904]
st b32 wb g[$r0d+0x4] $r2
join nop
// invalidate caches and exit
end_exit:
cctl ivall 0 g[0]
bpt pause 0x0
rtt terminate
end_cont:
bpt pause 0x0
mov $flags $r2 mask 0xffff
ld b128 $r0q cs l[0x00]
rtt

.section #gk104_builtin_offsets
.b64 #gk104_div_u32
.b64 #gk104_div_s32
.b64 #gk104_rcp_f64
.b64 #gk104_rsq_f64
