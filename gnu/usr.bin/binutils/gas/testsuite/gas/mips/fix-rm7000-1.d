#as: -mfix-rm7000 -mgp64 -mabi=64
#objdump: -dz --prefix-addresses
#name: MIPS RM7000 workarounds test 1
#source: fix-rm7000-1.s

.*file format.*

Disassembly.*

0+0000 <[^>]*> move	v0,a0
0+0004 <[^>]*> dmult	a2,v1
0+0008 <[^>]*> nop
0+000c <[^>]*> nop
0+0010 <[^>]*> nop
0+0014 <[^>]*> ld	a3,0\(s8\)
0+0018 <[^>]*> ld	a0,0\(s8\)
0+001c <[^>]*> move	a0,a3
0+0020 <[^>]*> dmult	v0,a3
0+0024 <[^>]*> nop
0+0028 <[^>]*> nop
0+002c <[^>]*> nop
0+0030 <[^>]*> ld	a0,0\(s8\)
