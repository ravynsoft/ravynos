#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR relocation against local symbol (n64)
#as: -64
#source: jal-svr4pic-local-newabi.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 27bdfff0 	addiu	sp,sp,-16
[0-9a-f]+ <[^>]*> ffbc0000 	sd	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 3c1c0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MIPS_GPREL16	foo
[ 	]*[0-9a-f]+: R_MIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_MIPS_HI16	\*ABS\*
[0-9a-f]+ <[^>]*> 279c0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MIPS_GPREL16	foo
[ 	]*[0-9a-f]+: R_MIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_MIPS_LO16	\*ABS\*
[0-9a-f]+ <[^>]*> 0399e02d 	daddu	gp,gp,t9
[0-9a-f]+ <[^>]*> ffbf0008 	sd	ra,8\(sp\)
[0-9a-f]+ <[^>]*> df990000 	ld	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT_DISP	\.text\+0x40
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x40
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x40
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	\.text\+0x40
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x40
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x40
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> dfbf0008 	ld	ra,8\(sp\)
[0-9a-f]+ <[^>]*> dfbc0000 	ld	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 03e0000[89] 	jr	ra
[0-9a-f]+ <[^>]*> 27bd0010 	addiu	sp,sp,16
	\.\.\.
[0-9a-f]+ <[^>]*> 03e0000[89] 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
