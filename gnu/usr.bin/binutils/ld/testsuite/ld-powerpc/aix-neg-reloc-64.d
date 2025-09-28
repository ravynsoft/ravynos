#source: aix-tls-reloc.s
#as: -a32
#ld: -b32 -shared -bE:aix-tls-reloc.ex
#objdump: -dr
#target: [is_xcoff_format]

.*

Disassembly of section \.text:

.* <.foo>:
.*:	4e 80 00 20 	blr
.*: R_REF	_foo.ro_-.*
.*:	60 00 00 00 	nop
.*:	60 00 00 00 	nop
.*:	60 00 00 00 	nop
.* <_GLOBAL__F_foo>:
.*:	ff ff ff ff 	fnmadd. f31,f31,f31,f31
.*: R_POS_64	.text-.*
.*: R_NEG	_foo.ro_-.*
.*:	ff ff ff f0 	.long 0xfffffff0
.*:	60 00 00 00 	nop
.*:	60 00 00 00 	nop
