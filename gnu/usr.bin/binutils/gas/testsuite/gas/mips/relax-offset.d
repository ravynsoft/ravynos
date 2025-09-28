#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PIC branch relaxation with offset
#as: -32 -relax-branch
#warning_output: relax-offset.l

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c1c0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 279c0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 0399e021 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> 14800004 	bnez	a0,00000020 <foo\+0x20>
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 8f810002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 24210034 	addiu	at,at,52
[ 	]*[0-9a-f]+: R_MIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 00200008 	jr	at
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 0000000c 	syscall
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
