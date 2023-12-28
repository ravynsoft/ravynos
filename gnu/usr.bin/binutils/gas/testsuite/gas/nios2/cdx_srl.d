#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX srl.n
#as: -march=r2

# Test the srl.n and srli.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 5909      	srl.n	r4,r4,r4
0+0002 <[^>]*> 8919      	srli.n	r4,r4,1
0+0004 <[^>]*> 9919      	srli.n	r4,r4,2
0+0006 <[^>]*> a919      	srli.n	r4,r4,3
0+0008 <[^>]*> b919      	srli.n	r4,r4,8
0+000a <[^>]*> c919      	srli.n	r4,r4,12
0+000c <[^>]*> d919      	srli.n	r4,r4,16
0+000e <[^>]*> e919      	srli.n	r4,r4,24
0+0010 <[^>]*> f919      	srli.n	r4,r4,31
0+0012 <[^>]*> 51c9      	srl.n	r7,r7,r16
0+0014 <[^>]*> 5e09      	srl.n	r16,r16,r7
	...
