#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 MT ASE subset disassembly
#as: -32 -I$srcdir/$subdir

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f0c0 3010 	sll	s0,3
[0-9a-f]+ <[^>]*> f026      	extend	0x26
[0-9a-f]+ <[^>]*> 6701      	move	s0,at
[0-9a-f]+ <[^>]*> f026      	extend	0x26
[0-9a-f]+ <[^>]*> 6701      	move	s0,at
[0-9a-f]+ <[^>]*> f022      	extend	0x22
[0-9a-f]+ <[^>]*> 6741      	move	v0,at
[0-9a-f]+ <[^>]*> f027      	extend	0x27
[0-9a-f]+ <[^>]*> 6701      	move	s0,at
[0-9a-f]+ <[^>]*> f027      	extend	0x27
[0-9a-f]+ <[^>]*> 6701      	move	s0,at
[0-9a-f]+ <[^>]*> f023      	extend	0x23
[0-9a-f]+ <[^>]*> 6741      	move	v0,at
[0-9a-f]+ <[^>]*> f026      	extend	0x26
[0-9a-f]+ <[^>]*> 6700      	move	s0,zero
[0-9a-f]+ <[^>]*> f026      	extend	0x26
[0-9a-f]+ <[^>]*> 6700      	move	s0,zero
[0-9a-f]+ <[^>]*> f022      	extend	0x22
[0-9a-f]+ <[^>]*> 6740      	move	v0,zero
[0-9a-f]+ <[^>]*> f027      	extend	0x27
[0-9a-f]+ <[^>]*> 6700      	move	s0,zero
[0-9a-f]+ <[^>]*> f027      	extend	0x27
[0-9a-f]+ <[^>]*> 6700      	move	s0,zero
[0-9a-f]+ <[^>]*> f023      	extend	0x23
[0-9a-f]+ <[^>]*> 6740      	move	v0,zero
	\.\.\.
