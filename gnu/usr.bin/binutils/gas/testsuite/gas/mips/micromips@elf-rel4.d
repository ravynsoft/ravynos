#objdump: --prefix-addresses -dr --show-raw-insn
#name: MIPS ELF reloc 4
#source: elf-rel4.s
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 309c 0000 	addiu	a0,gp,0
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	a
[0-9a-f]+ <[^>]*> 309c 0004 	addiu	a0,gp,4
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	a
[0-9a-f]+ <[^>]*> 309c 0008 	addiu	a0,gp,8
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	a
[0-9a-f]+ <[^>]*> 309c 000c 	addiu	a0,gp,12
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	a
