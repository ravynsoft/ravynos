# as:
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]+> bf18[ 	]+it	ne
0+002 <[^>]+> [0-9a-f ]+[ 	]+bne.[nw]	0+0 <[^>]+>
0+00. <[^>]+> bf38[ 	]+it	cc
0+00. <[^>]+> f7ff bff[ab][ 	]+bcc.w	0+0 <[^>]+>
0+00. <[^>]+> bf28[ 	]+it	cs
0+0.. <[^>]+> f7ff fff[78][ 	]+blcs	0+0 <[^>]+>
0+0.. <[^>]+> bfb8[ 	]+it	lt
0+0.. <[^>]+> 47a8[ 	]+blxlt	r5
0+0.. <[^>]+> bf08[ 	]+it	eq
0+0.. <[^>]+> 4740[ 	]+bxeq	r8
0+0.. <[^>]+> bfc8[ 	]+it	gt
0+0.. <[^>]+> e8d4 f001[ 	]+tbbgt	\[r4, r1\]
0+0.. <[^>]+> bfb8[ 	]+it	lt
0+0.. <[^>]+> df00[ 	]+svclt	0
0+0.. <[^>]+> bf08[ 	]+it	eq
0+0.. <[^>]+> f8d0 f000[ 	]+ldreq.w	pc, \[r0\]
0+0.. <[^>]+> bfdc[ 	]+itt	le
0+0.. <[^>]+> be00[ 	]+bkpt	0x0000
0+0.. <[^>]+> bf00[ 	]+nople
0+0.. <[^>]+> bf00[ 	]+nop
#...
