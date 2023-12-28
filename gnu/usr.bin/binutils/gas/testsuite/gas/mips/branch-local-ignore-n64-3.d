#objdump: -dr --prefix-addresses --show-raw-insn -mmips:isa32r6
#name: MIPS branch local symbol relocation 3 (ignore branch ISA, n64)
#as: -64 -march=from-abi -mignore-branch-isa
#source: branch-local-3.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> c8000000 	bc	0000000000001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MIPS_PC26_S2	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> d8400000 	beqzc	v0,0000000000001020 <bar\+0x10>
[ 	]*[0-9a-f]+: R_MIPS_PC21_S2	foo-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*-0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
