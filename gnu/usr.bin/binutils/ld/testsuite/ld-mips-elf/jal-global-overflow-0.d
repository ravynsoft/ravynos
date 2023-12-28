#name: MIPS JAL to global symbol overflow 0
#source: jal-global-overflow.s
#ld: -Ttext 0x20000000 -e 0x20000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0c001000 	jal	20004000 <bar>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000800 	jal	20002000 <foo>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
[0-9a-f]+ <[^>]*> 0c000800 	jal	20002000 <foo>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c001000 	jal	20004000 <bar>
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
