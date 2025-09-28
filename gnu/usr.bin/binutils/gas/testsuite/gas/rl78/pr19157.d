#objdump: -d --prefix-addresses --show-raw-insn
#name: PR19157: RL78: zero offset omitted in stack based addressing

.*: +file format .*rl78.*

Disassembly of section .text:
0x0+000 88 00[	 ]+mov[	 ]+a, \[sp\+0\]
0x0+002 88 00[	 ]+mov[	 ]+a, \[sp\+0\]
0x0+004 88 01[	 ]+mov[	 ]+a, \[sp\+1\]
0x0+006 a8 00[	 ]+movw[	 ]+ax, \[sp\+0\]
0x0+008 a8 00[	 ]+movw[	 ]+ax, \[sp\+0\]
0x0+00a a8 02[	 ]+movw[	 ]+ax, \[sp\+2\]
0x0+00c c8 00 09[	 ]+mov[	 ]+\[sp\+0\], #9
0x0+00f c8 00 09[	 ]+mov[	 ]+\[sp\+0\], #9
0x0+012 c8 01 09[	 ]+mov[	 ]+\[sp\+1\], #9
