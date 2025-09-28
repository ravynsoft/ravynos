#name: MIPS JALX to unaligned symbol with addend 2
#source: unaligned-jalx-addend-2.s
#source: unaligned-insn.s -mips16
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: unaligned-jalx-addend-0.d
