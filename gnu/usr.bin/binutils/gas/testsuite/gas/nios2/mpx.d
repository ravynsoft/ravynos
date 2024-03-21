#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 mpx
#as: -march=r2

# Test the MPX instructions

.*: +file format elf32-littlenios2
Disassembly of section .text:
0+0000 <[^>]*> dc000020 	ldsex	zero,\(zero\)
0+0004 <[^>]*> dc0007e0 	ldsex	zero,\(ra\)
0+0008 <[^>]*> dc1f0020 	ldsex	ra,\(zero\)
0+000c <[^>]*> fc000020 	stsex	zero,zero,\(zero\)
0+0010 <[^>]*> fc1fffe0 	stsex	ra,ra,\(ra\)
0+0014 <[^>]*> fc04fc20 	stsex	r4,ra,\(r16\)
