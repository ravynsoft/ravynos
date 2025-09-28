#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 2 (ignore branch ISA, n64)
#as: -64 -march=from-abi -mignore-branch-isa
#source: branch-local-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 10000000 	b	0000000000001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 14430000 	bne	v0,v1,0000000000001020 <bar\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 04510000 	bgezal	v0,0000000000001028 <bar\+0x18>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 04500000 	bltzal	v0,0000000000001030 <bar\+0x20>
[ 	]*[0-9a-f]+: R_MIPS_PC16	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
