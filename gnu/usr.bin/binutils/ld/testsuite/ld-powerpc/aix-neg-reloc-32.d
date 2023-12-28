#source: aix-tls-reloc.s
#as: -a32
#ld: -b32 -shared -bE:aix-tls-reloc.ex
#objdump: -dr
#target: [is_xcoff_format]

.*

Disassembly of section \.text:

.* <.foo>:
.*:	4e 80 00 20 	br
.*: R_REF	_foo.ro_-.*
.*:	60 00 00 00 	oril    r0,r0,0
.*:	60 00 00 00 	oril    r0,r0,0
.*:	60 00 00 00 	oril    r0,r0,0
.* <_GLOBAL__F_foo>:
.*:	ff ff ff f0 	.long 0xfffffff0
.*: R_POS	.text-.*
.*: R_NEG	_foo.ro_-.*
.*:	60 00 00 00 	oril    r0,r0,0
.*:	60 00 00 00 	oril    r0,r0,0
.*:	60 00 00 00 	oril    r0,r0,0
