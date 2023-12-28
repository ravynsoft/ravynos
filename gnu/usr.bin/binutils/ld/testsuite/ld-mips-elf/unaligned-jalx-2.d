#name: MIPS JALX to unaligned symbol 2
#source: unaligned-jalx-2.s
#source: unaligned-insn.s -mips16
#as: -EB
#ld: -EB -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: unaligned-jalx-0.d
