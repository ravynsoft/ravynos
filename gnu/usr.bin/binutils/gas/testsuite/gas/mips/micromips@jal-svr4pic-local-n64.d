#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR relocation against local symbol (n64)
#as: -64
#source: jal-svr4pic-local-newabi.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4ff9      	addiu	sp,sp,-16
[0-9a-f]+ <[^>]*> db9d 0000 	sd	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 41bc 0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	foo
[ 	]*[0-9a-f]+: R_MICROMIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_MICROMIPS_HI16	\*ABS\*
[0-9a-f]+ <[^>]*> 339c 0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MICROMIPS_GPREL16	foo
[ 	]*[0-9a-f]+: R_MICROMIPS_SUB	\*ABS\*
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\*ABS\*
[0-9a-f]+ <[^>]*> 5b3c e150 	daddu	gp,gp,t9
[0-9a-f]+ <[^>]*> dbfd 0008 	sd	ra,8\(sp\)
[0-9a-f]+ <[^>]*> df3c 0000 	ld	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT_DISP	\.text\+0x31
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x31
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x31
[0-9a-f]+ <[^>]*> 03f9 4f3c 	jalrs	t9
[ 	]*[0-9a-f]+: R_MICROMIPS_JALR	\.text\+0x31
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x31
[ 	]*[0-9a-f]+: R_MIPS_NONE	\*ABS\*\+0x31
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> dffd 0008 	ld	ra,8\(sp\)
[0-9a-f]+ <[^>]*> df9d 0000 	ld	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 459f      	jr	ra
[0-9a-f]+ <[^>]*> 4c09      	addiu	sp,sp,16
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
