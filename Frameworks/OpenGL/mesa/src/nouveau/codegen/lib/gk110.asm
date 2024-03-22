.section #gk110_builtin_code
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
gk110_div_u32:
   sched 0x28 0x04 0x28 0x04 0x28 0x28 0x28
   bfind u32 $r2 $r1
   xor b32 $r2 $r2 0x1f
   mov b32 $r3 0x1
   shl b32 $r2 $r3 clamp $r2
   cvt u32 $r1 neg u32 $r1
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   sched 0x28 0x28 0x28 0x28 0x28 0x28 0x28
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   sched 0x04 0x28 0x04 0x28 0x28 0x2c 0x04
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   cvt u32 $r2 neg u32 $r1
   add $r1 (mul u32 $r1 u32 $r0) $r3
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   sched 0x28 0x2c 0x04 0x20 0x2e 0x28 0x20
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
gk110_div_s32:
   set $p2 0x1 lt s32 $r0 0x0
   set $p3 0x1 lt s32 $r1 0x0 xor $p2
   sched 0x20 0x28 0x28 0x04 0x28 0x04 0x28
   cvt s32 $r0 abs s32 $r0
   cvt s32 $r1 abs s32 $r1
   bfind u32 $r2 $r1
   xor b32 $r2 $r2 0x1f
   mov b32 $r3 0x1
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
   sched 0x28 0x28 0x04 0x28 0x04 0x28 0x28
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mul $r3 u32 $r1 u32 $r2
   add $r2 (mul high u32 $r2 u32 $r3) $r2
   mov b32 $r3 $r0
   mul high $r0 u32 $r0 u32 $r2
   cvt u32 $r2 neg u32 $r1
   add $r1 (mul u32 $r1 u32 $r0) $r3
   sched 0x2c 0x04 0x28 0x2c 0x04 0x28 0x20
   set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p0 set $p0 0x1 ge u32 $r1 $r2
   $p0 sub b32 $r1 $r1 $r2
   $p0 add b32 $r0 $r0 0x1
   $p3 cvt s32 $r0 neg s32 $r0
   sched 0x04 0x2e 0x28 0x04 0x28 0x28 0x28
   $p2 cvt s32 $r1 neg s32 $r1
   ret

// RCP F64
//
// INPUT:   $r0d
// OUTPUT:  $r0d
// CLOBBER: $r2 - $r9, $p0
//
// The core of RCP and RSQ implementation is Newton-Raphson step, which is
// used to find successively better approximation from an imprecise initial
// value (single precision rcp in RCP and rsqrt64h in RSQ).
//
gk110_rcp_f64:
   // Step 1: classify input according to exponent and value, and calculate
   // result for 0/inf/nan. $r2 holds the exponent value, which starts at
   // bit 52 (bit 20 of the upper half) and is 11 bits in length
   ext u32 $r2 $r1 0xb14
   add b32 $r3 $r2 0xffffffff
   joinat #rcp_rejoin
   // We want to check whether the exponent is 0 or 0x7ff (i.e. NaN, inf,
   // denorm, or 0). Do this by subtracting 1 from the exponent, which will
   // mean that it's > 0x7fd in those cases when doing unsigned comparison
   set b32 $p0 0x1 gt u32 $r3 0x7fd
   // $r3: 0 for norms, 0x36 for denorms, -1 for others
   mov b32 $r3 0x0
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
   set b32 $p0 0x1 eq s32 $r4 0x0
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
   set b32 $p0 0x1 lt s32 $r3 0x0
   $p0 bra #rcp_end
   // Step 2: Before the real calculation goes on, renormalize the values to
   // range [1, 2) by setting exponent field to 0x3ff (the exponent of 1)
   // result in $r6d. The exponent will be recovered later.
   ext u32 $r2 $r1 0xb14
   and b32 $r7 $r1 0x800fffff
   add b32 $r7 $r7 0x3ff00000
   mov b32 $r6 $r0
   sched 0x2b 0x04 0x28 0x28 0x2a 0x2b 0x2e
   // Step 3: Convert new value to float (no overflow will occur due to step
   // 2), calculate rcp and do newton-raphson step once
   cvt rz f32 $r5 f64 $r6d
   rcp f32 $r4 $r5
   mov b32 $r0 0xbf800000
   fma rn f32 $r5 $r4 $r5 $r0
   fma rn f32 $r0 neg $r4 $r5 $r4
   // Step 4: convert result $r0 back to double, do newton-raphson steps
   cvt f64 $r0d f32 $r0
   cvt f64 $r6d f64 neg $r6d
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
   add b32 $r4 $r2 $r3
   ext u32 $r3 $r1 0xb14
   // New exponent in $r3
   add b32 $r3 $r3 $r4
   add b32 $r2 $r3 0xffffffff
   sched 0x28 0x2b 0x28 0x2b 0x28 0x28 0x2b
   // (exponent-1) < 0x7fe (unsigned) means the result is in norm range
   // (same logic as in step 1)
   set b32 $p0 0x1 lt u32 $r2 0x7fe
   (not $p0) bra #rcp_result_inf_or_denorm
   // Norms: convert exponents back and return
   shl b32 $r4 $r4 clamp 0x14
   add b32 $r1 $r4 $r1
   bra #rcp_end
rcp_result_inf_or_denorm:
   // New exponent >= 0x7ff means that result is inf
   set b32 $p0 0x1 ge s32 $r3 0x7ff
   (not $p0) bra #rcp_result_denorm
   sched 0x20 0x25 0x28 0x2b 0x23 0x25 0x2f
   // Infinity
   and b32 $r1 $r1 0x80000000
   mov b32 $r0 0x0
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
   set b32 $p0 0x1 ne u32 $r3 0x0
   and b32 $r1 $r1 0x800fffff
   // 0x3e800000: 1/4
   $p0 cvt f64 $r6d f32 0x3e800000
   sched 0x2f 0x28 0x2c 0x2e 0x2a 0x20 0x27
   // 0x3f000000: 1/2
   (not $p0) cvt f64 $r6d f32 0x3f000000
   add b32 $r1 $r1 0x00100000
   mul rn f64 $r0d $r0d $r6d
rcp_end:
   ret

// RSQ F64
//
// INPUT:   $r0d
// OUTPUT:  $r0d
// CLOBBER: $r2 - $r9, $p0 - $p1
//
gk110_rsq_f64:
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
   set b32 $p1 0x1 le u32 $r3 0x2
   or b32 $r2 $r0 $r2
   $p1 mul rn f64 $r0d $r0d 0x4350000000000000
   rsqrt64h f32 $r5 $r1
   // rsqrt64h will give correct result for 0/inf/nan, the following logic
   // checks whether the input is one of those (exponent is 0x7ff or all 0
   // except for the sign bit)
   set b32 $r6 ne u32 $r3 0x7ff
   and b32 $r2 $r2 $r6
   sched 0x28 0x2b 0x20 0x27 0x28 0x2e 0x28
   set b32 $p0 0x1 ne u32 $r2 0x0
   $p0 bra #rsq_norm
   // For 0/inf/nan, make sure the sign bit agrees with input and return
   and b32 $r1 $r1 0x80000000
   mov b32 $r0 0x0
   or b32 $r1 $r1 $r5
   ret
rsq_norm:
   // For others, do 4 Newton-Raphson steps with the formula:
   //     RSQ_{n + 1} = RSQ_{n} * (1.5 - 0.5 * x * RSQ_{n} * RSQ_{n})
   // In the code below, each step is written as:
   //     tmp1 = 0.5 * x * RSQ_{n}
   //     tmp2 = -RSQ_{n} * tmp1 + 0.5
   //     RSQ_{n + 1} = RSQ_{n} * tmp2 + RSQ_{n}
   mov b32 $r4 0x0
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
   mov b32 $r1 $r5
   mov b32 $r0 $r4
   ret

.section #gk110_builtin_offsets
.b64 #gk110_div_u32
.b64 #gk110_div_s32
.b64 #gk110_rcp_f64
.b64 #gk110_rsq_f64
