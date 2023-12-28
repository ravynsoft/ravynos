#as: -mfix-rm7000 -mgp64 -mabi=64
#objdump: -dz --prefix-addresses
#name: MIPS RM7000 workarounds test 1
#source: fix-rm7000-1.s

.*file format.*

Disassembly.*

0+0000 <[^>]*> move	v0,a0
0+0002 <[^>]*> dmult	a2,v1
0+0006 <[^>]*> ld	a3,0\(s8\)
0+000a <[^>]*> ld	a0,0\(s8\)
0+000e <[^>]*> move	a0,a3
0+0010 <[^>]*> dmult	v0,a3
0+0014 <[^>]*> ld	a0,0\(s8\)
