#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX add.n
#as: -march=r2

# Test the add.n and addi.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 4901      	add.n	r4,r4,r4
0+0002 <[^>]*> 0911      	addi.n	r4,r4,1
0+0004 <[^>]*> 1911      	addi.n	r4,r4,2
0+0006 <[^>]*> 2911      	addi.n	r4,r4,4
0+0008 <[^>]*> 3911      	addi.n	r4,r4,8
0+000a <[^>]*> 4911      	addi.n	r4,r4,16
0+000c <[^>]*> 5911      	addi.n	r4,r4,32
0+000e <[^>]*> 6911      	addi.n	r4,r4,64
0+0010 <[^>]*> 7911      	addi.n	r4,r4,128
	...
