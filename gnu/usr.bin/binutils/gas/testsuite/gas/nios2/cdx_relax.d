#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX branch relaxation
#as: -march=r2

# Test relaxation of beqz.n, bnez.n, and br.n instructions to
# equivalent 32-bit instructions when the branch target is out of range.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <label0> 04a3      	bnez.n	r2,00000006 <label1>
0+0002 <[^>]*> 02eb      	beqz.n	r3,00000006 <label1>
0+0004 <[^>]*> 0003      	br.n	00000006 <label1>
0+0006 <label1> 04b800a2 	bne	r2,zero,000004c2 <label2>
0+000a <[^>]*> 04b400ea 	beq	r3,zero,000004c2 <label2>
0+000e <[^>]*> 04b00002 	br	000004c2 <label2>
0+0012 <[^>]*> c4000020 	nop
#...
0+04c2 <label2> fb4000a2 	bne	r2,zero,00000006 <label1>
0+04c6 <[^>]*> fb3c00ea 	beq	r3,zero,00000006 <label1>
0+04ca <[^>]*> fb380002 	br	00000006 <label1>
	...
