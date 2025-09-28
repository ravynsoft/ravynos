# name: STREXH/STREXB. - Thumb
# objdump: -dr --prefix-address --show-raw-insn
# skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section \.text:
0+00 <[^>]+> e8c2 1f50 	strexh	r0, r1, \[r2\]
0+04 <[^>]+> e8c2 1f50 	strexh	r0, r1, \[r2\]
0+08 <[^>]+> e8cd 1f50 	strexh	r0, r1, \[sp\]
0+0c <[^>]+> e8c2 1f40 	strexb	r0, r1, \[r2\]
0+10 <[^>]+> e8c2 1f40 	strexb	r0, r1, \[r2\]
0+14 <[^>]+> e8cd 1f40 	strexb	r0, r1, \[sp\]

