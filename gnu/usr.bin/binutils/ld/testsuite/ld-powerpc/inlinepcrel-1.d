#source: inlinepcrel.s
#as: -a64 -mpower10
#ld: -melf64ppc -shared --hash-style=gnu
#objdump: -dr -Mpower10

.*

Disassembly of section \.text:

.*:
.*:	(04 10 00 01|01 00 10 04) 	pld     r12,65944	# 10318 \[my_func@plt\]
.*:	(e5 80 01 98|98 01 80 e5) 
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 21|21 04 80 4e) 	bctrl
#...
.* <my_func@plt>:
.*:	(4b .. .. ..|.. .. .. 4b) 	b       .* <__glink_PLTresolve>
