#name: ARM IT automatic instruction generation 2
#as: -mthumb -march=armv7a -mimplicit-it=always
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <.text> 3a40      	subs	r2, #64.*
00000002 <.text\+0x2> bfa1      	itttt	ge
00000004 <.text\+0x4> e8a0 500a 	stmiage.w	r0!, {r1, r3, ip, lr}
00000008 <.text\+0x8> e8a0 500a 	stmiage.w	r0!, {r1, r3, ip, lr}
0000000c <.text\+0xc> e8a0 500a 	stmiage.w	r0!, {r1, r3, ip, lr}
00000010 <.text\+0x10> e8a0 500a 	stmiage.w	r0!, {r1, r3, ip, lr}
00000014 <.text\+0x14> dcf4      	bgt.n	00000000 <.text>
