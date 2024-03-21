#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS `.option picX' with relaxation 0
#as: -32

# Verify that relaxation is done according to the `.option picX' setting
# at the time the relevant instruction was assembled rather than at
# relaxation time.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c1c0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 279c0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 0399e021 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> 8f820000 	lw	v0,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	bar
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
