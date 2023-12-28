#objdump: -dr --prefix-address --show-raw-insn
#as: -32 -I$srcdir/$subdir
#name: MIPS16 ISA subset disassembly
#source: mips16-sub.s
#dump: mips16-32@mips16-sub.d
