# indirect references, stubs and {non,}_lazy_symbol_pointer sections.
# not applicable to x86_64 mach-o.

	.text
	.globl c
	.globl c1
	.globl c2
c:	nop
c1:	nop
c2:	nop

e:	nop
e1:	nop
e2:	nop

	.data
d:	.space 8
d1:	.space 8
d2:	.space 8

	.private_extern f
	.private_extern f1
	.private_extern f2
f:	.space 8
f1:	.space 8
f2:	.space 8

	.section __dummy, __dummy, symbol_stubs,strip_static_syms,8

	.indirect_symbol a
La:	.space 8

	.indirect_symbol b
Lb:	.space 8

	.indirect_symbol c
Lc:	.space 8

	.indirect_symbol d
Ld:	.space 8

	.indirect_symbol e
Le:	.space 8

	.indirect_symbol f
Lf:	.space 8

	.private_extern g
	.indirect_symbol g
Lg:	.space 8

	.lazy_symbol_pointer

	.indirect_symbol a1
La1:	.space 4

	.indirect_symbol b1
Lb1:	.space 4

	.indirect_symbol c1
Lc1:	.space 4

	.indirect_symbol d1
Ld1:	.space 4

	.indirect_symbol e1
Le1:	.space 4

	.indirect_symbol f1
Lf1:	.space 4

	.private_extern g1
	.indirect_symbol g1
Lg1:	.space 4

	.non_lazy_symbol_pointer

	.indirect_symbol a2
La2:	.space 4

	.indirect_symbol b2
Lb2:	.space 4

	.indirect_symbol c2
Lc2:	.space 4

	.indirect_symbol d2
Ld2:	.space 4

	.indirect_symbol e2
Le2:	.space 4

	.indirect_symbol f2
Lf2:	.space 4

	.private_extern g2
	.indirect_symbol g2
Lg2:	.space 4

	.indirect_symbol f1
Lf11:	.space 4

	.private_extern g1
	.indirect_symbol g1
Lg11:	.space 4

	.indirect_symbol a2
La12:	.space 4

	.indirect_symbol b2
Lb12:	.space 4
