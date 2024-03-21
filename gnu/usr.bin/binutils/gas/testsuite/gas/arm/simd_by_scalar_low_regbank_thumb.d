#name: VMUL/VMLA/VMLS by scalar reg restriction (Thumb)
#source: simd_by_scalar_low_regbank.s
#as: -march=armv8.2-a+fp16 -mfpu=neon-fp-armv8 -mthumb
#error_output: simd_by_scalar_low_regbank.l
