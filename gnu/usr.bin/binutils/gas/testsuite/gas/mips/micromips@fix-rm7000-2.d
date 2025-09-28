#as: -32 -mfix-rm7000
#objdump: -dz --prefix-addresses
#name: MIPS RM7000 workarounds test 2
#source: fix-rm7000-2.s

.*file format.*

Disassembly.*
0+0000 <[^>]*> move	v0,a0
0+0002 <[^>]*> dmultu	a1,a3
0+0006 <[^>]*> lb	a0,0\(s8\)
0+000a <[^>]*> dmult	a2,v1
0+000e <[^>]*> lbu	a3,0\(s8\)
0+0012 <[^>]*> move	v0,a0
0+0014 <[^>]*> dmultu	a1,a3
0+0018 <[^>]*> addiu	a0,s8,0
0+001c <[^>]*> move	v0,a0
0+001e <[^>]*> dmult	a2,v1
0+0022 <[^>]*> lh	a3,0\(s8\)
0+0026 <[^>]*> dmultu	a1,a3
0+002a <[^>]*> lhu	a0,0\(s8\)
0+002e <[^>]*> dmult	a2,v1
0+0032 <[^>]*> ll	a3,0\(s8\)
0+0036 <[^>]*> dmultu	a1,a3
0+003a <[^>]*> lld	a0,0\(s8\)
0+003e <[^>]*> dmultu	a1,a3
0+0042 <[^>]*> lw	a0,0\(s8\)
0+0046 <[^>]*> dmult	a2,v1
0+004a <[^>]*> lwr	a3,0\(s8\)
0+004e <[^>]*> dmultu	a1,a3
0+0052 <[^>]*> nop
