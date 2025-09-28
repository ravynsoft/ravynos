#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PIC branch relaxation with offset
#as: -32 -relax-branch
#warning_output: relax-offset.l
#source: relax-offset.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41bc 0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MICROMIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 339c 0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 033c e150 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> 40a4 fffe 	bnezc	a0,0000000c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0025 	addiu	at,at,37
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
[0-9a-f]+ <[^>]*> 0000 8b7c 	syscall
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
