#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS `.option picX' with relaxation 1
#as: -32

# Verify that relaxation is done according to the `.option picX' setting
# at the time the relevant instruction was assembled rather than at
# relaxation time.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c020000 	lui	v0,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	bar
[0-9a-f]+ <[^>]*> 24420000 	addiu	v0,v0,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	bar
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
