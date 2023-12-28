#objdump: -d -mm9s12x --prefix-addresses --reloc
#as: -mm9s12x
#name: s12x extended forms of movb,movw

.*:     file format elf32-m68hc12


Disassembly of section .text:
00000000 <.text> movb	#0x4, 0x00001234 <a1>
00000005 <.text\+0x5> movb	#0x44, 0x0,X
00000009 <.text\+0x9> movb	#0x58, 0xff02,Y
0000000e <.text\+0xe> movb	#0x89, 0x1234,SP
00000014 <.text\+0x14> movb	#0xfe, \[D,X\]
00000018 <.text\+0x18> movb	#0x80, \[0x3456,SP\]
0000001e <.text\+0x1e> movb	0x00001234 <a1>, 0x00003456 <a2>
00000024 <.text\+0x24> movb	0x00003456 <a2>, 0x1,X
00000029 <.text\+0x29> movb	0x00008123 <a3>, 0xff,Y
0000002f <.text\+0x2f> movb	0x0000c567 <a4>, 0x1234,SP
00000036 <.text\+0x36> movb	0x00002987 <a5>, \[D,Y\]
0000003b <.text\+0x3b> movb	0x00001009 <a6>, \[0x8123,SP\]
00000042 <.text\+0x42> movb	1,X\+, 0x00001234 <a1>
00000047 <.text\+0x47> movb	2,-X, 0xf,X
0000004b <.text\+0x4b> movb	7,SP\+, 0xfd,Y
00000050 <.text\+0x50> movb	6,-SP, 0x3456,SP
00000056 <.text\+0x56> movb	0xfff1,Y, \[D,X\]
0000005a <.text\+0x5a> movb	0xd,SP, \[0x2987,SP\]
00000060 <.text\+0x60> movb	\[D,X\], 0x00001234 <a1>
00000065 <.text\+0x65> movb	\[D,Y\], 0xe,X
00000069 <.text\+0x69> movb	\[D,SP\], 0xfd,Y
0000006e <.text\+0x6e> movb	\[D,PC\], 0x3456,SP
00000074 <.text\+0x74> movb	\[D,X\], \[D,X\]
00000078 <.text\+0x78> movb	\[D,Y\], \[0x2987,SP\]
0000007e <.text\+0x7e> movb	\[0x1234,X\], 0x00003456 <a2>
00000085 <.text\+0x85> movb	\[0x3456,Y\], 0xd,X
0000008b <.text\+0x8b> movb	\[0x8123,SP\], 0xfb,Y
00000092 <.text\+0x92> movb	\[0xc567,PC\], 0x8123,SP
0000009a <.text\+0x9a> movb	\[0x2987,X\], \[D,PC\]
000000a0 <.text\+0xa0> movb	\[0x1009,Y\], \[0x2987,SP\]
000000a8 <.text\+0xa8> movw	#0x00001234 <a1>, 0x00001234 <a1>
000000ae <.text\+0xae> movw	#0x00003456 <a2>, 0x0,X
000000b3 <.text\+0xb3> movw	#0x00008123 <a3>, 0xff02,Y
000000b9 <.text\+0xb9> movw	#0x0000c567 <a4>, 0x1234,SP
000000c0 <.text\+0xc0> movw	#0x00002987 <a5>, \[D,X\]
000000c5 <.text\+0xc5> movw	#0x00001009 <a6>, \[0x3456,SP\]
000000cc <.text\+0xcc> movw	0x00001234 <a1>, 0x00003456 <a2>
000000d2 <.text\+0xd2> movw	0x00003456 <a2>, 0x1,X
000000d7 <.text\+0xd7> movw	0x00008123 <a3>, 0xff,Y
000000dd <.text\+0xdd> movw	0x0000c567 <a4>, 0x1234,SP
000000e4 <.text\+0xe4> movw	0x00002987 <a5>, \[D,Y\]
000000e9 <.text\+0xe9> movw	0x00001009 <a6>, \[0x8123,SP\]
000000f0 <.text\+0xf0> movw	1,X\+, 0x00001234 <a1>
000000f5 <.text\+0xf5> movw	2,-X, 0xf,X
000000f9 <.text\+0xf9> movw	7,SP\+, 0xfd,Y
000000fe <.text\+0xfe> movw	6,-SP, 0x3456,SP
00000104 <.text\+0x104> movw	0xfff1,Y, \[D,X\]
00000108 <.text\+0x108> movw	0xd,SP, \[0x2987,SP\]
0000010e <.text\+0x10e> movw	\[D,X\], 0x00001234 <a1>
00000113 <.text\+0x113> movw	\[D,Y\], 0xe,X
00000117 <.text\+0x117> movw	\[D,SP\], 0xfd,Y
0000011c <.text\+0x11c> movw	\[D,PC\], 0x3456,SP
00000122 <.text\+0x122> movw	\[D,X\], \[D,X\]
00000126 <.text\+0x126> movw	\[D,Y\], \[0x2987,SP\]
0000012c <.text\+0x12c> movw	\[0x1234,X\], 0x00003456 <a2>
00000133 <.text\+0x133> movw	\[0x3456,Y\], 0xd,X
00000139 <.text\+0x139> movw	\[0x8123,SP\], 0xfb,Y
00000140 <.text\+0x140> movw	\[0xc567,PC\], 0x8123,SP
00000148 <.text\+0x148> movw	\[0x2987,X\], \[D,PC\]
0000014e <.text\+0x14e> movw	\[0x1009,Y\], \[0x2987,SP\]
