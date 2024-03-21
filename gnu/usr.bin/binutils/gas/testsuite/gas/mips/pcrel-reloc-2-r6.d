#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS R6 local PC-relative relocations 2
#as: -32 -mips32r6 --defsym setmips3=1
#source: pcrel-reloc-1.s
#dump: pcrel-reloc-1-r6.d
