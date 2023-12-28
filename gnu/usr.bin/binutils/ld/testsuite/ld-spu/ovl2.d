#source: ovl2.s
#ld: -N -T ovl2.lnk -T ovl.lnk --emit-relocs
#objdump: -D -r -j.text -j.ov_a1 -j.ov_a2 -j.data -j.toe -j.nonalloc -j.note.spu_name

.*elf32-spu

Disassembly of section \.text:

00000100 <_start>:
.*	brsl	\$0,.* <00000000\.ovl_call\.f1_a1>.*
.*SPU_REL16	f1_a1
.*	brsl	\$0,.* <00000000\.ovl_call\.setjmp>.*
.*SPU_REL16	setjmp
.*	br	100 <_start>	# 100
.*SPU_REL16	_start

0000010c <setjmp>:
.*	bi	\$0

00000110 <longjmp>:
.*	bi	\$0

.*00 00 03 40.*
.*SPU_ADDR32	\.ov_a1\+0x14
	\.\.\.
#...
00000320 <00000000\.ovl_call.f1_a1>:
.*	ila	\$78,1
.*	lnop
.*	ila	\$79,1040	# 410
.*	bra?	.* <__ovly_load>.*

00000330 <00000000\.ovl_call.setjmp>:
.*	ila	\$78,0
.*	lnop
.*	ila	\$79,268	# 10c
.*	bra?	.* <__ovly_load>.*

00000340 <00000000\.ovl_call\.13:5>:
.* 	ila	\$78,1
.* 	lnop
.* 	ila	\$79,1044	# 414
.* 	bra?	.* <__ovly_load>.*

00000350 <_SPUEAR_f1_a2>:
.*	ila	\$78,2
.*	lnop
.*	ila	\$79,1040	# 410
.*	bra?	.* <__ovly_load>.*

#00000318 <00000000\.ovl_call.f1_a1>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 04 04 00.*
#
#00000320 <00000000\.ovl_call.setjmp>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 00 01 0c.*
#
#00000328 <_SPUEAR_f1_a2>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 08 04 00.*

Disassembly of section \.ov_a1:

00000400 <00000001\.ovl_call\.14:6>:
.* 	ila	\$78,2
.* 	lnop
.* 	ila	\$79,1044	# 414
.* 	bra?	.* <__ovly_load>.*

00000410 <f1_a1>:
.*	bi	\$0
.*00 00 04 14.*
.*SPU_ADDR32	\.ov_a1\+0x14
.*00 00 04 20.*
.*SPU_ADDR32	\.ov_a1\+0x20
.*00 00 04 00.*
.*SPU_ADDR32	\.ov_a2\+0x14

Disassembly of section \.ov_a2:

00000400 <00000002\.ovl_call\.13:5>:
.*	ila	\$78,1
.*	lnop
.*	ila	\$79,1056	# 420
.*	bra?	.* <__ovly_load>.*

00000410 <f1_a2>:
.*	br	.* <longjmp>.*
.*SPU_REL16	longjmp
.*00 00 04 00.*
.*SPU_ADDR32	\.ov_a1\+0x20
.*00 00 04 1c.*
.*SPU_ADDR32	\.ov_a2\+0x1c
.*00 00 00 00.*

Disassembly of section \.data:

00000420 <_ovly_table-0x10>:
.*00 00 00 00 .*
.*00 00 00 01 .*
	\.\.\.
00000430 <_ovly_table>:
.*00 00 04 00 .*
.*00 00 00 20 .*
#.*00 00 03 10 .*
.*00 00 01 00 .*
.*00 00 00 01 .*
.*00 00 04 00 .*
.*00 00 00 20 .*
#.*00 00 03 20 .*
.*00 00 01 20 .*
.*00 00 00 01 .*

00000450 <_ovly_buf_table>:
.*00 00 00 00 .*

Disassembly of section \.toe:

00000460 <_EAR_>:
	\.\.\.

Disassembly of section .nonalloc:

00000000 <.nonalloc>:
.*00 00 04 14.*
.*SPU_ADDR32	\.ov_a1\+0x14
.*00 00 04 20.*
.*SPU_ADDR32	\.ov_a1\+0x20
.*00 00 04 14.*
.*SPU_ADDR32	\.ov_a2\+0x14
.*00 00 04 1c.*
.*SPU_ADDR32	\.ov_a2\+0x1c

Disassembly of section \.note\.spu_name:

.* <\.note\.spu_name>:
.*:	00 00 00 08 .*
.*:	00 00 00 0c .*
.*:	00 00 00 01 .*
.*:	53 50 55 4e .*
.*:	41 4d 45 00 .*
.*:	74 6d 70 64 .*
.*:	69 72 2f 64 .*
.*:	75 6d 70 00 .*
