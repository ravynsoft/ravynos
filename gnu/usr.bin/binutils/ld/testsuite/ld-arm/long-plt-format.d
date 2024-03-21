.*:     file format elf32-.*


Disassembly of section .plt:

.* <.plt>:
 .*:	.*
 .*:	.*
 .*:	.*
 .*:	.*
 .*:	.* 	.word	.*
.* <foo@plt>:
 .*:	.* 	add	ip, pc, #-268435456	@ 0xf0000000
 .*:	.* 	add	ip, ip, #0, 12
 .*:	.* 	add	ip, ip, #0, 20
 .*:	.* 	ldr	pc, [ip, #[0-9]*]!	@ 0x.*
