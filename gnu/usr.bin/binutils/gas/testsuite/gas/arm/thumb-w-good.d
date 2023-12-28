#name: Wide instruction acceptance in Thumb-2 cores
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince *-*-vxworks

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <.text> f7ff fffe 	bl	00000000 <foo>
00000004 <.text\+0x4> f3ef 8000 	mrs	r0, CPSR
00000008 <.text\+0x8> f84d 0d04 	(str(\.w)?	r0, \[sp, #-4\]!|push(\.w)?	\{r0\})
0000000c <.text\+0xc> e92d 4001 	(stmdb(\.w)?	sp!,|push(\.w)?)[ 	]+\{r0, lr\}
00000010 <.text\+0x10> f85d 0b04 	(ldr(\.w)?	r0, \[sp\], #4|pop(\.w)?	\{r0\})
00000014 <.text\+0x14> e8bd 8001 	(ldmia(\.w)?	sp!,|pop(\.w)?)[ 	]+\{r0, pc\}
