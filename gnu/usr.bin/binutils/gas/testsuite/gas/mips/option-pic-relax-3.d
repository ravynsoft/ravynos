#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS `.option picX' with relaxation 3
#as: -32 --relax-branch
#warning_output: option-pic-relax-3.l

# Verify that relaxation is done according to the `.option picX' setting
# at the time the relevant instruction was assembled rather than at
# relaxation time.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 00801025 	move	v0,a0
[0-9a-f]+ <[^>]*> 08008002 	j	00020008 <.*>
[ 	]*[0-9a-f]+: R_MIPS_26	\.text
[0-9a-f]+ <[^>]*> 00a01825 	move	v1,a1
	\.\.\.
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
