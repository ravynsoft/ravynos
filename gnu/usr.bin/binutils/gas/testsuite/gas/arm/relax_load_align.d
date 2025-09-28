# as: -march=armv6kt2
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]+> f510 707a 	adds.w	r0, r0, #1000	@ 0x3e8
0+004 <[^>]+> 4800      	ldr	r0, \[pc, #0\]	@ \(0+008 <[^>]+>\)
0+006 <[^>]+> 4800      	ldr	r0, \[pc, #0\]	@ \(0+008 <[^>]+>\)
