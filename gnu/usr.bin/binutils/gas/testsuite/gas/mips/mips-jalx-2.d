#objdump: -d
#as:
#name: mips jalx-2

.*:     file format .*

Disassembly of section .text:

[ 0-9a-f]+ <text_sym>:
[ 0-9a-f]+:	74000000 	jalx	0 <.[^>]*>
[ 0-9a-f]+:	00000000 	nop

[ 0-9a-f]+ <.[^>]*>:
[ 0-9a-f]+:	6500      	nop
	\.\.\.
