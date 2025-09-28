#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR relocation against local symbol
#as: -32
#source: jal-svr4pic-local.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c1c0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 279c0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 0399e021 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> 27bdffe0 	addiu	sp,sp,-32
[0-9a-f]+ <[^>]*> afbf001c 	sw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> afbc0010 	sw	gp,16\(sp\)
[0-9a-f]+ <[^>]*> 8f990000 	lw	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 27390040 	addiu	t9,t9,64
[ 	]*[0-9a-f]+: R_MIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	bar
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 8fbc0010 	lw	gp,16\(sp\)
[0-9a-f]+ <[^>]*> 8fbf001c 	lw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 27bd0020 	addiu	sp,sp,32
[0-9a-f]+ <[^>]*> 03e0000[89] 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03e0000[89] 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
