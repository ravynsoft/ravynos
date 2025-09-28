#name: Local BLX instructions
#objdump: -drw --prefix-addresses --show-raw-insn
#target: *-*-*eabi* *-*-nacl*
#as:
#warning_output: blx-local.l
# Test assembler resolution of blx and bl instructions in ARM mode.
.*: +file format .*arm.*

Disassembly of section .text:
0+00 <[^>]*> fa000006 	blx	00000020 <foo>
0+04 <[^>]*> eb000007 	bl	00000028 <foo2>
0+08 <[^>]*> fa000004 	blx	00000020 <foo>
0+0c <[^>]*> eb000005 	bl	00000028 <foo2>
0+10 <[^>]*> fa00000b 	blx	00000044 <fooundefarm>
0+14 <[^>]*> eb00000a 	bl	00000044 <fooundefarm>
0+18 <[^>]*> fa000001 	blx	00000024 <fooundefthumb>
0+1c <[^>]*> eb000000 	bl	00000024 <fooundefthumb>
0+20 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+22 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+24 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+26 <[^>]*> 46c0      	nop			@ \(mov r8, r8\)
0+28 <[^>]*> 0bfffffd 	bleq	00000024 <fooundefthumb>
0+2c <[^>]*> 0afffffc 	beq	00000024 <fooundefthumb>
0+30 <[^>]*> eafffffb 	b	00000024 <fooundefthumb>
0+34 <[^>]*> 0bfffffe 	bleq	00000020 <foo>	34: R_ARM_JUMP24	foo
0+38 <[^>]*> 0afffffe 	beq	00000020 <foo>	38: R_ARM_JUMP24	foo
0+3c <[^>]*> eafffffe 	b	00000020 <foo>	3c: R_ARM_JUMP24	foo
0+40 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+44 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
