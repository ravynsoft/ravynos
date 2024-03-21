#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS abs
#source: abs.s
#as: -32

# Test the abs macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4044 fffe 	bgez	a0,[0-9a-f]+ <foo>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0080 2190 	neg	a0,a0
[0-9a-f]+ <[^>]*> 4045 fffe 	bgez	a1,[0-9a-f]+ <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c85      	move	a0,a1
[0-9a-f]+ <[^>]*> 00a0 2190 	neg	a0,a1
	\.\.\.
