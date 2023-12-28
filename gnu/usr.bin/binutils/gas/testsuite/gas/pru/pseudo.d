#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU pseudo

# Test the pseudo instruction

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 1300e2e1 	mov	r1, sp
0+0004 <[^>]*> 12e0e0e0 	nop
0+0008 <[^>]*> 230100c3 	call	00000400 <[^>]*>
0+000c <[^>]*> 22ea00c3 	call	r10
0+0010 <[^>]*> 20c30000 	ret
0+0014 <[^>]*> d10cac00 	wbc	r12.w1, 12
0+0018 <[^>]*> c8e1ec00 	wbs	r12, r1
