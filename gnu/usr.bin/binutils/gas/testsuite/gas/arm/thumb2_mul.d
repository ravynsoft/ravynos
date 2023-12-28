# as: -march=armv6kt2
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]+> bf04      	itt	eq
0+002 <[^>]+> 4348      	muleq	r0, r1
0+004 <[^>]+> 4348      	muleq	r0, r1
0+006 <[^>]+> bf02      	ittt	eq
0+008 <[^>]+> fb00 f008 	muleq.w	r0, r0, r8
0+00c <[^>]+> fb08 f000 	muleq.w	r0, r8, r0
0+010 <[^>]+> fb00 f808 	muleq.w	r8, r0, r8
0+014 <[^>]+> bf04      	itt	eq
0+016 <[^>]+> fb01 f001 	muleq.w	r0, r1, r1
0+01a <[^>]+> fb01 f002 	muleq.w	r0, r1, r2
0+01e <[^>]+> bf04      	itt	eq
0+020 <[^>]+> fb01 f000 	muleq.w	r0, r1, r0
0+024 <[^>]+> fb00 f001 	muleq.w	r0, r0, r1
