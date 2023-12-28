#source: inline.s
#as: -a64
#ld: -melf64ppc --hash-style=gnu
#ld_after_inputfiles: tmpdir/funv2.so
#objdump: -dr

.*

Disassembly of section \.text:

.*:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(e9 82 80 18|18 80 82 e9) 	ld      r12,-32744\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 21|21 04 80 4e) 	bctrl
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
#...
.* <my_func@plt>:
.*:	(4b .. .. ..|.. .. .. 4b) 	b       .* <__glink_PLTresolve>
