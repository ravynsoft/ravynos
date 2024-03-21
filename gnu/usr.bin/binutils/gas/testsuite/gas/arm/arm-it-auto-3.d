#name: ARM IT automatic instruction generation 3
#as: -mthumb -march=armv7a -mimplicit-it=always
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text.one:
00000000 <.text.one> 2800      	cmp	r0, #0
00000002 <.text.one\+0x2> bf08      	it	eq
00000004 <.text.one\+0x4> 3102      	addeq	r1, #2

Disassembly of section .text.two:
00000000 <.text.two> bf08      	it	eq
00000002 <.text.two\+0x2> 3103      	addeq	r1, #3
