#name: ARM V7-A+MP instructions
#as: -march=armv7-a+mp
#objdump: -dr --prefix-addresses --show-raw-insn
#source: arch7ar-mp.s

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f590f000 	pldw	\[r0\]
0[0-9a-f]+ <[^>]+> f59ef000 	pldw	\[lr\]
0[0-9a-f]+ <[^>]+> f591f000 	pldw	\[r1\]
0[0-9a-f]+ <[^>]+> f590ffff 	pldw	\[r0, #4095\]	@ 0xfff
0[0-9a-f]+ <[^>]+> f510ffff 	pldw	\[r0, #-4095\]	@ 0xfffff001
0[0-9a-f]+ <[^>]+> f790f000 	pldw	\[r0, r0\]
0[0-9a-f]+ <[^>]+> f791f000 	pldw	\[r1, r0\]
0[0-9a-f]+ <[^>]+> f79ef000 	pldw	\[lr, r0\]
0[0-9a-f]+ <[^>]+> f790f001 	pldw	\[r0, r1\]
0[0-9a-f]+ <[^>]+> f790f00e 	pldw	\[r0, lr\]
0[0-9a-f]+ <[^>]+> f790f100 	pldw	\[r0, r0, lsl #2\]
0[0-9a-f]+ <[^>]+> f8b0 f000 	pldw	\[r0\]
0[0-9a-f]+ <[^>]+> f8be f000 	pldw	\[lr\]
0[0-9a-f]+ <[^>]+> f8b1 f000 	pldw	\[r1\]
0[0-9a-f]+ <[^>]+> f8b0 ffff 	pldw	\[r0, #4095\]	@ 0xfff
0[0-9a-f]+ <[^>]+> f830 fcff 	pldw	\[r0, #-255\]
0[0-9a-f]+ <[^>]+> f830 f000 	pldw	\[r0, r0\]
0[0-9a-f]+ <[^>]+> f831 f000 	pldw	\[r1, r0\]
0[0-9a-f]+ <[^>]+> f83e f000 	pldw	\[lr, r0\]
0[0-9a-f]+ <[^>]+> f830 f001 	pldw	\[r0, r1\]
0[0-9a-f]+ <[^>]+> f830 f00e 	pldw	\[r0, lr\]
0[0-9a-f]+ <[^>]+> f830 f030 	pldw	\[r0, r0, lsl #3\]
