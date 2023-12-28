#objdump: -dr --prefix-addresses --show-raw-insn
#name: PRU misc

# Test the miscellaneous instruction

.*: +file format elf32-pru

Disassembly of section .text:
0+0000 <[^>]*> 2a000000 	halt
0+0004 <[^>]*> 3e800000 	slp	1
0+0008 <[^>]*> 3e000000 	slp	0
0+000c <[^>]*> 2701e1e0 	lmbd	r0, r1, 1
0+0010 <[^>]*> 2700e100 	lmbd	r0.b0, r1, 0
0+0014 <[^>]*> 2642e1e0 	lmbd	r0, r1, sp.b2
