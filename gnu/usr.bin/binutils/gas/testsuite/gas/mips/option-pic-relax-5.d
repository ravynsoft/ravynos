#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS `.option picX' with relaxation 5
#as: -32 -mmicromips --relax-branch
#warning_output: option-pic-relax-5.l

# Verify that relaxation is done according to the `.option picX' setting
# at the time the relevant instruction was assembled rather than at
# relaxation time.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0c44      	move	v0,a0
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c65      	move	v1,a1
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 45bf      	jrc	ra
	\.\.\.
