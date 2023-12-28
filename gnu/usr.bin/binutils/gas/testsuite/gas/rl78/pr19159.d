#objdump: -d --prefix-addresses --show-raw-insn
#name: PR19159: RL78: zero offset omitted in DE based addressing

.*: +file format .*rl78.*

Disassembly of section .text:
0x0+000 89[	 ]+mov[	 ]+a, \[de\]
0x0+001 8a 00[	 ]+mov[	 ]+a, \[de\+0\]
0x0+003 8a 01[	 ]+mov[	 ]+a, \[de\+1\]
0x0+005 99[	 ]+mov[	 ]+\[de], a
0x0+006 9a 00[	 ]+mov[	 ]+\[de\+0\], a
0x0+008 9a 01[	 ]+mov[	 ]+\[de\+1\], a
0x0+00a a9[	 ]+movw[	 ]+ax, \[de\]
0x0+00b aa 00[	 ]+movw[	 ]+ax, \[de\+0\]
0x0+00d aa 01[	 ]+movw[	 ]+ax, \[de\+1\]
