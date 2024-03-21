#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 7 (n64)
#as: -64 -march=from-abi
#source: branch-local-7.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000000000001018 <bar\+0x8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	\.text\+0xffc
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0xffc
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0xffc
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0000 02d0 	not	zero,zero
	\.\.\.
