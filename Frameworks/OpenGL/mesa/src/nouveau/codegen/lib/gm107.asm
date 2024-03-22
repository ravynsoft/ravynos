.section #gm107_builtin_code
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
gm107_div_u32:
   sched (st 0xd wr 0x0 wt 0x3f) (st 0x1 wt 0x1) (st 0x6)
   flo u32 $r2 $r1
   lop xor 1 $r2 $r2 0x1f
   mov $r3 0x1 0xf
   sched (st 0x1) (st 0xf wr 0x0) (st 0x6 wr 0x0 wt 0x1)
   shl $r2 $r3 $r2
   i2i u32 u32 $r1 neg $r1
   imul u32 u32 $r3 $r1 $r2
   sched (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 wt 0x1)
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   sched (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 wt 0x1)
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   sched (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 wt 0x1) (st 0x6 wr 0x0 rd 0x1 wt 0x1)
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   sched (st 0x6 wt 0x2) (st 0x6 wr 0x0 rd 0x1 wt 0x1) (st 0xf wr 0x0 rd 0x1 wt 0x2)
   mov $r3 $r0 0xf
   imul u32 u32 hi $r0 $r0 $r2
   i2i u32 u32 $r2 neg $r1
   sched (st 0x6 wr 0x0 wt 0x3) (st 0xd wt 0x1) (st 0x1)
   imad u32 u32 $r1 $r1 $r0 $r3
   isetp ge u32 and $p0 1 $r1 $r2 1
   $p0 iadd $r1 $r1 neg $r2
   sched (st 0x5) (st 0xd) (st 0x1)
   $p0 iadd $r0 $r0 0x1
   $p0 isetp ge u32 and $p0 1 $r1 $r2 1
   $p0 iadd $r1 $r1 neg $r2
   sched (st 0x1) (st 0xf) (st 0xf)
   $p0 iadd $r0 $r0 0x1
   ret
   nop 0

// DIV S32, like DIV U32 after taking ABS(inputs)
//
// INPUT:   $r0: dividend, $r1: divisor
// OUTPUT:  $r0: result, $r1: modulus
// CLOBBER: $r2 - $r3, $p0 - $p3
//
gm107_div_s32:
   sched (st 0xd wt 0x3f) (st 0x1) (st 0x1 wr 0x0)
   isetp lt and $p2 0x1 $r0 0 1
   isetp lt xor $p3 1 $r1 0 $p2
   i2i s32 s32 $r0 abs $r0
   sched (st 0xf wr 0x1) (st 0xd wr 0x1 wt 0x2) (st 0x1 wt 0x2)
   i2i s32 s32 $r1 abs $r1
   flo u32 $r2 $r1
   lop xor 1 $r2 $r2 0x1f
   sched (st 0x6) (st 0x1) (st 0xf wr 0x1)
   mov $r3 0x1 0xf
   shl $r2 $r3 $r2
   i2i u32 u32 $r1 neg $r1
   sched (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2)
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   sched (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2)
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   sched (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2) (st 0x6 wr 0x1 wt 0x2)
   imul u32 u32 $r3 $r1 $r2
   imad u32 u32 hi $r2 $r2 $r3 $r2
   imul u32 u32 $r3 $r1 $r2
   sched (st 0x6 wr 0x1 rd 0x2 wt 0x2) (st 0x2 wt 0x5) (st 0x6 wr 0x0 rd 0x1 wt 0x2)
   imad u32 u32 hi $r2 $r2 $r3 $r2
   mov $r3 $r0 0xf
   imul u32 u32 hi $r0 $r0 $r2
   sched (st 0xf wr 0x1 rd 0x2 wt 0x2) (st 0x6 wr 0x0 wt 0x5) (st 0xd wt 0x3)
   i2i u32 u32 $r2 neg $r1
   imad u32 u32 $r1 $r1 $r0 $r3
   isetp ge u32 and $p0 1 $r1 $r2 1
   sched (st 0x1) (st 0x5) (st 0xd)
   $p0 iadd $r1 $r1 neg $r2
   $p0 iadd $r0 $r0 0x1
   $p0 isetp ge u32 and $p0 1 $r1 $r2 1
   sched (st 0x1) (st 0x2) (st 0xf wr 0x0)
   $p0 iadd $r1 $r1 neg $r2
   $p0 iadd $r0 $r0 0x1
   $p3 i2i s32 s32 $r0 neg $r0
   sched (st 0xf wr 0x1) (st 0xf wt 0x3) (st 0xf)
   $p2 i2i s32 s32 $r1 neg $r1
   ret
   nop 0

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
gm107_rcp_f64:
   // Step 1: classify input according to exponent and value, and calculate
   // result for 0/inf/nan. $r2 holds the exponent value, which starts at
   // bit 52 (bit 20 of the upper half) and is 11 bits in length
   sched (st 0x0) (st 0x0) (st 0x0)
   bfe u32 $r2 $r1 0xb14
   iadd32i $r3 $r2 -1
   ssy #rcp_rejoin
   // We want to check whether the exponent is 0 or 0x7ff (i.e. NaN, inf,
   // denorm, or 0). Do this by subtracting 1 from the exponent, which will
   // mean that it's > 0x7fd in those cases when doing unsigned comparison
   sched (st 0x0) (st 0x0) (st 0x0)
   isetp gt u32 and $p0 1 $r3 0x7fd 1
   // $r3: 0 for norms, 0x36 for denorms, -1 for others
   mov $r3 0x0 0xf
   not $p0 sync
   // Process all special values: NaN, inf, denorm, 0
   sched (st 0x0) (st 0x0) (st 0x0)
   mov32i $r3 0xffffffff 0xf
   // A number is NaN if its abs value is greater than or unordered with inf
   dsetp gtu and $p0 1 abs $r0 0x7ff0000000000000 1
   not $p0 bra #rcp_inf_or_denorm_or_zero
   // NaN -> NaN, the next line sets the "quiet" bit of the result. This
   // behavior is both seen on the CPU and the blob
   sched (st 0x0) (st 0x0) (st 0x0)
   lop32i or $r1 $r1 0x80000
   sync
rcp_inf_or_denorm_or_zero:
   lop32i and $r4 $r1 0x7ff00000
   sched (st 0x0) (st 0x0) (st 0x0)
   // Other values with nonzero in exponent field should be inf
   isetp eq and $p0 1 $r4 0x0 1
   $p0 bra #rcp_denorm_or_zero
   // +/-Inf -> +/-0
   lop32i xor $r1 $r1 0x7ff00000
   sched (st 0x0) (st 0x0) (st 0x0)
   mov $r0 0x0 0xf
   sync
rcp_denorm_or_zero:
   dsetp gtu and $p0 1 abs $r0 0x0 1
   sched (st 0x0) (st 0x0) (st 0x0)
   $p0 bra #rcp_denorm
   // +/-0 -> +/-Inf
   lop32i or $r1 $r1 0x7ff00000
   sync
rcp_denorm:
   // non-0 denorms: multiply with 2^54 (the 0x36 in $r3), join with norms
   sched (st 0x0) (st 0x0) (st 0x0)
   dmul $r0 $r0 0x4350000000000000
   mov $r3 0x36 0xf
   sync
rcp_rejoin:
   // All numbers with -1 in $r3 have their result ready in $r0d, return them
   // others need further calculation
   sched (st 0x0) (st 0x0) (st 0x0)
   isetp lt and $p0 1 $r3 0x0 1
   $p0 bra #rcp_end
   // Step 2: Before the real calculation goes on, renormalize the values to
   // range [1, 2) by setting exponent field to 0x3ff (the exponent of 1)
   // result in $r6d. The exponent will be recovered later.
   bfe u32 $r2 $r1 0xb14
   sched (st 0x0) (st 0x0) (st 0x0)
   lop32i and $r7 $r1 0x800fffff
   iadd32i $r7 $r7 0x3ff00000
   mov $r6 $r0 0xf
   // Step 3: Convert new value to float (no overflow will occur due to step
   // 2), calculate rcp and do newton-raphson step once
   sched (st 0x0) (st 0x0) (st 0x0)
   f2f ftz f64 f32 $r5 $r6
   mufu rcp $r4 $r5
   mov32i $r0 0xbf800000 0xf
   sched (st 0x0) (st 0x0) (st 0x0)
   ffma $r5 $r4 $r5 $r0
   ffma $r0 $r5 neg $r4 $r4
   // Step 4: convert result $r0 back to double, do newton-raphson steps
   f2f f32 f64 $r0 $r0
   sched (st 0x0) (st 0x0) (st 0x0)
   f2f f64 f64 $r6 neg $r6
   f2f f32 f64 $r8 0x3f800000
   // 4 Newton-Raphson Steps, tmp in $r4d, result in $r0d
   // The formula used here (and above) is:
   //     RCP_{n + 1} = 2 * RCP_{n} - x * RCP_{n} * RCP_{n}
   // The following code uses 2 FMAs for each step, and it will basically
   // looks like:
   //     tmp = -src * RCP_{n} + 1
   //     RCP_{n + 1} = RCP_{n} * tmp + RCP_{n}
   dfma $r4 $r6 $r0 $r8
   sched (st 0x0) (st 0x0) (st 0x0)
   dfma $r0 $r0 $r4 $r0
   dfma $r4 $r6 $r0 $r8
   dfma $r0 $r0 $r4 $r0
   sched (st 0x0) (st 0x0) (st 0x0)
   dfma $r4 $r6 $r0 $r8
   dfma $r0 $r0 $r4 $r0
   dfma $r4 $r6 $r0 $r8
   sched (st 0x0) (st 0x0) (st 0x0)
   dfma $r0 $r0 $r4 $r0
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
   iadd $r2 neg $r2 0x3ff
   iadd $r4 $r2 $r3
   sched (st 0x0) (st 0x0) (st 0x0)
   bfe u32 $r3 $r1 0xb14
   // New exponent in $r3
   iadd $r3 $r3 $r4
   iadd32i $r2 $r3 -1
   // (exponent-1) < 0x7fe (unsigned) means the result is in norm range
   // (same logic as in step 1)
   sched (st 0x0) (st 0x0) (st 0x0)
   isetp lt u32 and $p0 1 $r2 0x7fe 1
   not $p0 bra #rcp_result_inf_or_denorm
   // Norms: convert exponents back and return
   shl $r4 $r4 0x14
   sched (st 0x0) (st 0x0) (st 0x0)
   iadd $r1 $r4 $r1
   bra #rcp_end
rcp_result_inf_or_denorm:
   // New exponent >= 0x7ff means that result is inf
   isetp ge and $p0 1 $r3 0x7ff 1
   sched (st 0x0) (st 0x0) (st 0x0)
   not $p0 bra #rcp_result_denorm
   // Infinity
   lop32i and $r1 $r1 0x80000000
   mov $r0 0x0 0xf
   sched (st 0x0) (st 0x0) (st 0x0)
   iadd32i $r1 $r1 0x7ff00000
   bra #rcp_end
rcp_result_denorm:
   // Denorm result comes from huge input. The greatest possible fp64, i.e.
   // 0x7fefffffffffffff's rcp is 0x0004000000000000, 1/4 of the smallest
   // normal value. Other rcp result should be greater than that. If we
   // set the exponent field to 1, we can recover the result by multiplying
   // it with 1/2 or 1/4. 1/2 is used if the "exponent" $r3 is 0, otherwise
   // 1/4 ($r3 should be -1 then). This is quite tricky but greatly simplifies
   // the logic here.
   isetp ne u32 and $p0 1 $r3 0x0 1
   sched (st 0x0) (st 0x0) (st 0x0)
   lop32i and $r1 $r1 0x800fffff
   // 0x3e800000: 1/4
   $p0 f2f f32 f64 $r6 0x3e800000
   // 0x3f000000: 1/2
   not $p0 f2f f32 f64 $r6 0x3f000000
   sched (st 0x0) (st 0x0) (st 0x0)
   iadd32i $r1 $r1 0x00100000
   dmul $r0 $r0 $r6
rcp_end:
   ret

// RSQ F64
//
// INPUT:   $r0d
// OUTPUT:  $r0d
// CLOBBER: $r2 - $r9, $p0 - $p1
//
gm107_rsq_f64:
   // Before getting initial result rsqrt64h, two special cases should be
   // handled first.
   // 1. NaN: set the highest bit in mantissa so it'll be surely recognized
   //    as NaN in rsqrt64h
   sched (st 0xd wr 0x0 wt 0x3f) (st 0xd wt 0x1) (st 0xd)
   dsetp gtu and $p0 1 abs $r0 0x7ff0000000000000 1
   $p0 lop32i or $r1 $r1 0x00080000
   lop32i and $r2 $r1 0x7fffffff
   // 2. denorms and small normal values: using their original value will
   //    lose precision either at rsqrt64h or the first step in newton-raphson
   //    steps below. Take 2 as a threshold in exponent field, and multiply
   //    with 2^54 if the exponent is smaller or equal. (will multiply 2^27
   //    to recover in the end)
   sched (st 0xd) (st 0xd) (st 0xd)
   bfe u32 $r3 $r1 0xb14
   isetp le u32 and $p1 1 $r3 0x2 1
   lop or 1 $r2 $r0 $r2
   sched (st 0xd wr 0x0) (st 0xd wr 0x0 wt 0x1) (st 0xd)
   $p1 dmul $r0 $r0 0x4350000000000000
   mufu rsq64h $r5 $r1
   // rsqrt64h will give correct result for 0/inf/nan, the following logic
   // checks whether the input is one of those (exponent is 0x7ff or all 0
   // except for the sign bit)
   iset ne u32 and $r6 $r3 0x7ff 1
   sched (st 0xd) (st 0xd) (st 0xd)
   lop and 1 $r2 $r2 $r6
   isetp ne u32 and $p0 1 $r2 0x0 1
   $p0 bra #rsq_norm
   // For 0/inf/nan, make sure the sign bit agrees with input and return
   sched (st 0xd) (st 0xd) (st 0xd wt 0x1)
   lop32i and $r1 $r1 0x80000000
   mov $r0 0x0 0xf
   lop or 1 $r1 $r1 $r5
   sched (st 0xd) (st 0xf) (st 0xf)
   ret
   nop 0
   nop 0
rsq_norm:
   // For others, do 4 Newton-Raphson steps with the formula:
   //     RSQ_{n + 1} = RSQ_{n} * (1.5 - 0.5 * x * RSQ_{n} * RSQ_{n})
   // In the code below, each step is written as:
   //     tmp1 = 0.5 * x * RSQ_{n}
   //     tmp2 = -RSQ_{n} * tmp1 + 0.5
   //     RSQ_{n + 1} = RSQ_{n} * tmp2 + RSQ_{n}
   sched (st 0xd) (st 0xd wr 0x1) (st 0xd wr 0x1 rd 0x0 wt 0x3)
   mov $r4 0x0 0xf
   // 0x3f000000: 1/2
   f2f f32 f64 $r8 0x3f000000
   dmul $r2 $r0 $r8
   sched (st 0xd wr 0x0 wt 0x3) (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1)
   dmul $r0 $r2 $r4
   dfma $r6 $r0 neg $r4 $r8
   dfma $r4 $r4 $r6 $r4
   sched (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1)
   dmul $r0 $r2 $r4
   dfma $r6 $r0 neg $r4 $r8
   dfma $r4 $r4 $r6 $r4
   sched (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1)
   dmul $r0 $r2 $r4
   dfma $r6 $r0 neg $r4 $r8
   dfma $r4 $r4 $r6 $r4
   sched (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1) (st 0xd wr 0x0 wt 0x1)
   dmul $r0 $r2 $r4
   dfma $r6 $r0 neg $r4 $r8
   dfma $r4 $r4 $r6 $r4
   // Multiply 2^27 to result for small inputs to recover
   sched (st 0xd wr 0x0 wt 0x1) (st 0xd wt 0x1) (st 0xd)
   $p1 dmul $r4 $r4 0x41a0000000000000
   mov $r1 $r5 0xf
   mov $r0 $r4 0xf
   sched (st 0xd) (st 0xf) (st 0xf)
   ret
   nop 0
   nop 0

.section #gm107_builtin_offsets
.b64 #gm107_div_u32
.b64 #gm107_div_s32
.b64 #gm107_rcp_f64
.b64 #gm107_rsq_f64
