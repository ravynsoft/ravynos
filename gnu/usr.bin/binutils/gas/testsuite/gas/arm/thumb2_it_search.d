#name: 32-bit Thumb conditional instructions backward search
#as: -march=armv6kt2
# This test is only valid on ELF based ports.
#skip: *-*-pe *-*-wince
#source: thumb2_it_search.s
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+0 <[^>]+> f3af 8000 	nop.w
0+4 <[^>]+> bf080000 	.word	0xbf080000
0+8 <[^>]+> f3af 8000 	nop.w
