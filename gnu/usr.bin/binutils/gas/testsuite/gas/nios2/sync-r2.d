#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 sync
#as: -march=r2
#source: sync.s

.*: +file format elf32-littlenios2

Disassembly of section \.text:
0+0000 <[^>]*> d8000020 	sync

