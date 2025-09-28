#PROG: objcopy
#objdump: -M no-aliases -d --prefix-addresses --show-raw-insn
#name: MIPS16 canonical alias disassembly
#as: -mips3
#source: mips16-alias.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 6500      	move	zero,s0
[0-9a-f]+ <[^>]*> 0a08      	addiu	v0,\$pc,32
[0-9a-f]+ <[^>]*> b207      	lw	v0,28\(\$pc\)
[0-9a-f]+ <[^>]*> fe47      	daddiu	v0,\$pc,28
[0-9a-f]+ <[^>]*> fc43      	ld	v0,24\(\$pc\)
	\.\.\.
	\.\.\.
