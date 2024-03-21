#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 align_fill

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> defffe04 	addi	sp,sp,-8
0+0004 <[^>]*> df000115 	stw	fp,4\(sp\)
0+0008 <[^>]*> d839883a 	mov	fp,sp
0+000c <[^>]*> 0007883a 	mov	r3,zero
0+0010 <[^>]*> 0001883a 	nop
0+0014 <[^>]*> 0001883a 	nop
0+0018 <[^>]*> 0001883a 	nop
0+001c <[^>]*> 0001883a 	nop
0+0020 <[^>]*> 18c00044 	addi	r3,r3,1
0+0024 <[^>]*> 18801910 	cmplti	r2,r3,100
0+0028 <[^>]*> 103ffd1e 	bne	r2,zero,00000020 <[^>]*>
0+002c <[^>]*> df000117 	ldw	fp,4\(sp\)
0+0030 <[^>]*> dec00204 	addi	sp,sp,8
0+0034 <[^>]*> f800283a 	ret
	...
