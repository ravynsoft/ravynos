
.*:     file format .*

Disassembly of section \.text:

#
# The TLS entries are ordered as follows:
#
#	foo3	(-0x7ff0 + 0x20)
#	foo1	(-0x7ff0 + 0x24)
#	foo2	(-0x7ff0 + 0x28)
#	foo0	(-0x7ff0 + 0x2c)
#
# Any order would be acceptable, but it must match the .got dump.
#
00080c00 <\.text>:
   80c00:	8f848028 	lw	a0,-32728\(gp\)
   80c04:	8f848020 	lw	a0,-32736\(gp\)
   80c08:	8f848024 	lw	a0,-32732\(gp\)
   80c0c:	8f84801c 	lw	a0,-32740\(gp\)
   80c10:	8f848028 	lw	a0,-32728\(gp\)
   80c14:	8f848020 	lw	a0,-32736\(gp\)
   80c18:	8f848024 	lw	a0,-32732\(gp\)
   80c1c:	8f84801c 	lw	a0,-32740\(gp\)
