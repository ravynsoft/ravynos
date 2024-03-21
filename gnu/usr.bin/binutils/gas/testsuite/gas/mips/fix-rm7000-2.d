#as: -32 -mfix-rm7000
#objdump: -dz --prefix-addresses
#name: MIPS RM7000 workarounds test 2
#source: fix-rm7000-2.s

.*file format.*

Disassembly.*
0+0000 <[^>]*> move	v0,a0
0+0004 <[^>]*> dmultu	a1,a3
0+0008 <[^>]*> nop
0+000c <[^>]*> nop
0+0010 <[^>]*> nop
0+0014 <[^>]*> lb	a0,0\(s8\)
0+0018 <[^>]*> dmult	a2,v1
0+001c <[^>]*> nop
0+0020 <[^>]*> nop
0+0024 <[^>]*> nop
0+0028 <[^>]*> lbu	a3,0\(s8\)
0+002c <[^>]*> move	v0,a0
0+0030 <[^>]*> dmultu	a1,a3
0+0034 <[^>]*> addiu	a0,s8,0
0+0038 <[^>]*> move	v0,a0
0+003c <[^>]*> dmult	a2,v1
0+0040 <[^>]*> nop
0+0044 <[^>]*> nop
0+0048 <[^>]*> nop
0+004c <[^>]*> lh	a3,0\(s8\)
0+0050 <[^>]*> dmultu	a1,a3
0+0054 <[^>]*> nop
0+0058 <[^>]*> nop
0+005c <[^>]*> nop
0+0060 <[^>]*> lhu	a0,0\(s8\)
0+0064 <[^>]*> dmult	a2,v1
0+0068 <[^>]*> nop
0+006c <[^>]*> nop
0+0070 <[^>]*> nop
0+0074 <[^>]*> ll	a3,0\(s8\)
0+0078 <[^>]*> dmultu	a1,a3
0+007c <[^>]*> nop
0+0080 <[^>]*> nop
0+0084 <[^>]*> nop
0+0088 <[^>]*> lld	a0,0\(s8\)
0+008c <[^>]*> dmultu	a1,a3
0+0090 <[^>]*> nop
0+0094 <[^>]*> nop
0+0098 <[^>]*> nop
0+009c <[^>]*> lw	a0,0\(s8\)
0+00a0 <[^>]*> dmult	a2,v1
0+00a4 <[^>]*> nop
0+00a8 <[^>]*> nop
0+00ac <[^>]*> nop
0+00b0 <[^>]*> lwr	a3,0\(s8\)
0+00b4 <[^>]*> dmultu	a1,a3
0+00b8 <[^>]*> nop
0+00bc <[^>]*> nop
0+00c0 <[^>]*> nop
