#source: copro-arm_v5teplus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: No ARMv5TE ARM CoProcessor Instructions on ARMv5
#as: -march=armv5 -EL
#error_output: copro-arm_v5teplus-thumb_v6t2plus-unavail.l
