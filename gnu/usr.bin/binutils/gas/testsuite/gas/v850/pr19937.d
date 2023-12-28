#objdump: -dr --prefix-addresses --show-raw-insn
#name: Disassembly of LD.BU 0x4[rN], rM
#as: -mv850e3v5

.*: +file format .*v850.*

Disassembly of section .text:
0+000 <.*> 8a a7 01 00[ 	]+ld.bu[ 	]+0\[r10\], r20
0+004 <.*> 8a a7 05 00[ 	]+ld.bu[ 	]+4\[r10\], r20
0+008 <.*> 8a a7 09 00[ 	]+ld.bu[ 	]+8\[r10\], r20
0+00c <.*> 0a a7 00 00[ 	]+ld.b[ 	]+0\[r10\], r20
0+010 <.*> 0a a7 04 00[ 	]+ld.b[ 	]+4\[r10\], r20
0+014 <.*> 0a a7 08 00[ 	]+ld.b[ 	]+8\[r10\], r20
#pass

