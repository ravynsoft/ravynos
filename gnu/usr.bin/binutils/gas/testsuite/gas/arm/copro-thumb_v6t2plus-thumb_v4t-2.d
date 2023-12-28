#source: copro-arm_v5plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: No ARMv6T2 Thumb CoProcessor Instructions on ARMv4T (2)
#as: -march=armv4t -mthumb -EL
#error_output: copro-arm_v5plus-thumb_v6t2plus-unavail.l
