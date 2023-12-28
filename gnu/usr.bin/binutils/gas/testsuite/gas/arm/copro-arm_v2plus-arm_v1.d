#source: copro-arm_v2plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: No ARMv2 ARM CoProcessor Instructions on ARMv1
#as: -march=armv1 -EL
#error_output: copro-arm_v2plus-thumb_v6t2plus-unavail.l
