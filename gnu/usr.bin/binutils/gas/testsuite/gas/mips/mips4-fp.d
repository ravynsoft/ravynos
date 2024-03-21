#objdump: -dr --prefix-addresses
#name: MIPS mips4 fp

# Test mips4 fp instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> bc1f	0+0000 <text_label>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bc1f	\$fcc1,0+0000 <text_label>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> bc1t	\$fcc1,0+0000 <text_label>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> c.f.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> c.f.d	\$fcc1,\$f4,\$f6
[0-9a-f]+ <[^>]*> ldxc1	\$f2,a0\(a1\)
[0-9a-f]+ <[^>]*> lwxc1	\$f2,a0\(a1\)
[0-9a-f]+ <[^>]*> madd.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> madd.s	\$f10,\$f8,\$f2,\$f0
[0-9a-f]+ <[^>]*> movf	a0,a1,\$fcc4
[0-9a-f]+ <[^>]*> movf.d	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> movf.s	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> movn.d	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> movn.s	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> movt	a0,a1,\$fcc4
[0-9a-f]+ <[^>]*> movt.d	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> movt.s	\$f4,\$f6,\$fcc0
[0-9a-f]+ <[^>]*> movz.d	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> movz.s	\$f4,\$f6,a2
[0-9a-f]+ <[^>]*> msub.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> msub.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> nmadd.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> nmadd.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> nmsub.d	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> nmsub.s	\$f0,\$f2,\$f4,\$f6
[0-9a-f]+ <[^>]*> prefx	0x4,a0\(a1\)
[0-9a-f]+ <[^>]*> recip.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> recip.s	\$f4,\$f6
[0-9a-f]+ <[^>]*> rsqrt.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> rsqrt.s	\$f4,\$f6
[0-9a-f]+ <[^>]*> sdxc1	\$f4,a0\(a1\)
[0-9a-f]+ <[^>]*> swxc1	\$f4,a0\(a1\)
	\.\.\.
