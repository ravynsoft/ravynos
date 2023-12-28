#source: inline.s
#as: -a64
#ld: -melf64ppc --no-toc-opt --hash-style=gnu
#ld_after_inputfiles: tmpdir/funv2.so
#objdump: -dr

.*

Disassembly of section \.text:

.*:
.*:	(f8 41 00 18|18 00 41 f8) 	std     r2,24\(r1\)
.*:	(3d 82 00 00|00 00 82 3d) 	addis   r12,r2,0
.*:	(e9 8c 80 18|18 80 8c e9) 	ld      r12,-32744\(r12\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 21|21 04 80 4e) 	bctrl
.*:	(e8 41 00 18|18 00 41 e8) 	ld      r2,24\(r1\)
#...
.* <my_func@plt>:
.*:	(4b .. .. ..|.. .. .. 4b) 	b       .* <__glink_PLTresolve>
