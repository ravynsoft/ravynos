#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 argumentless instruction size override
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> e809      	entry
[0-9a-f]+ <[^>]*> e809      	entry
[0-9a-f]+ <[^>]*> ef09      	exit
[0-9a-f]+ <[^>]*> ef09      	exit
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
