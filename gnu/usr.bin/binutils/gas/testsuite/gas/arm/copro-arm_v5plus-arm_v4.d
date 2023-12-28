#source: copro-arm_v5plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: No ARMv5 ARM CoProcessor Instructions on ARMv4
#as: -march=armv4 -EL
#error_output: copro-arm_v5plus-thumb_v6t2plus-unavail.l
