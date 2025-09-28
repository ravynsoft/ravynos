#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 align_fill
#as: -march=r2
#source: align_fill.s

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> fff8dec4 	addi	sp,sp,-8
0+0004 <[^>]*> 0004e6f7 	stw	fp,4\(sp\)
0+0008 <[^>]*> c41c06e0 	mov	fp,sp
0+000c <[^>]*> c4030020 	mov	r3,zero
0+0010 <[^>]*> c4000020 	nop
0+0014 <[^>]*> c4000020 	nop
0+0018 <[^>]*> c4000020 	nop
0+001c <[^>]*> c4000020 	nop
0+0020 <[^>]*> 000118c4 	addi	r3,r3,1
0+0024 <[^>]*> 006410de 	cmplti	r2,r3,100
0+0028 <[^>]*> fff400a2 	bne	r2,zero,00000020 <[^>]*>
0+002c <[^>]*> 0004e6d7 	ldw	fp,4\(sp\)
0+0030 <[^>]*> 0008dec4 	addi	sp,sp,8
0+0034 <[^>]*> 140007e0 	ret
	...
