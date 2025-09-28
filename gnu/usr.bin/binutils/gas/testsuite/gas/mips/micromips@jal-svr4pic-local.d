#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR relocation against local symbol
#as: -32
#source: jal-svr4pic-local.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41bc 0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 339c 0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 033c e150 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> 4ff1      	addiu	sp,sp,-32
[0-9a-f]+ <[^>]*> cbe7      	sw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> fb9d 0010 	sw	gp,16\(sp\)
[0-9a-f]+ <[^>]*> ff3c 0000 	lw	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3339 0031 	addiu	t9,t9,49
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 03f9 4f3c 	jalrs	t9
[ 	]*[0-9a-f]+: R_MICROMIPS_JALR	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff9d 0010 	lw	gp,16\(sp\)
[0-9a-f]+ <[^>]*> 4be7      	lw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 459f      	jr	ra
[0-9a-f]+ <[^>]*> 4c11      	addiu	sp,sp,32
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
