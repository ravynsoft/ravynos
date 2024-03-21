#name: bl local instructions for v4t.
#objdump: -drw --prefix-addresses --show-raw-insn
#target: *-*-*eabi* *-*-nacl*
#as:

.*: +file format .*arm.*
Disassembly of section .text:
0+00 <[^>]*> f7ff fffe 	bl	00+18 <[^>]*>	0: R_ARM_THM_CALL	foo2
0+04 <[^>]*> d004      	beq.n	00+10 <[^>]*>
0+06 <[^>]*> e003      	b.n	00+10 <[^>]*>
0+08 <[^>]*> f000 f808 	bl	00+1c <[^>]*>
0+0c <[^>]*> f000 f802 	bl	00+14 <[^>]*>
0+10 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+12 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+14 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
	...
0+18 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+1c <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
