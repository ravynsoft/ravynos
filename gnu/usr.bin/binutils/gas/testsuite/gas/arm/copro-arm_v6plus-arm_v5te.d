#source: copro-arm_v6plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: No ARMv6 ARM CoProcessor Instructions on ARMv5TE
#as: -march=armv5te -EL
#error_output: copro-arm_v6plus-thumb_v6t2plus-unavail.l
