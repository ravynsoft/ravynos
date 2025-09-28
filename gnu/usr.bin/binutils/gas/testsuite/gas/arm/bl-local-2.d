#name: bl local conversion to blx
#objdump: -drw --prefix-addresses --show-raw-insn
#target: *-*-*eabi* *-*-nacl*
#as:


.*:     file format .*


Disassembly of section \.text:
0+00 <[^>]+> e12fff1e 	bx	lr
0+04 <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+06 <[^>]+> f7ff effc 	blx	0+ <myfunction>
0+0a <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+0c <[^>]+> f7ff eff8 	blx	0+ <myfunction>
0+10 <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+12 <[^>]+> f7ff eff6 	blx	0+ <myfunction>
0+16 <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+18 <[^>]+> f7ff eff2 	blx	0+ <myfunction>
0+1c <[^>]+> 4770      	bx	lr
0+1e <[^>]+> 46c0      	nop			@ \(mov r8, r8\)
0+20 <[^>]+> fafffffd 	blx	0000001c <mythumbfunction>
