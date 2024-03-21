#source: ovl.s
#ld: -N -T ovl1.lnk -T ovl.lnk --emit-relocs
#objdump: -D -r -j.text -j.ov_a1 -j.ov_a2 -j.data -j.toe -j.note.spu_name

.*elf32-spu

Disassembly of section \.text:

00000100 <_start>:
.*	ai	\$1,\$1,-32
.*	xor	\$0,\$0,\$0
.*	stqd	\$0,0\(\$1\)
.*	stqd	\$0,16\(\$1\)
.*	brsl	\$0,.* <00000000\.ovl_call\.f1_a1>.*
.*SPU_REL16	f1_a1
.*	brsl	\$0,.* <00000000\.ovl_call\.f2_a1>.*
.*SPU_REL16	f2_a1
.*	brsl	\$0,.* <00000000\.ovl_call\.f1_a2>.*
.*SPU_REL16	f1_a2
.*	ila	\$9,.*
.*SPU_ADDR18	f2_a2
.*	bisl	\$0,\$9
.*	ai	\$1,\$1,32	# 20
.*	br	100 <_start>	# 100
.*SPU_REL16	_start

0000012c <f0>:
.*	bi	\$0

#...
[0-9a-f]+ <__ovly_return>:
#...
[0-9a-f]+ <__ovly_load>:
#...
[0-9a-f]+ <_ovly_debug_event>:
#...
00000330 <00000000\.ovl_call\.f1_a1>:
.*	ila	\$78,1
.*	lnop
.*	ila	\$79,1024	# 400
.*	bra?	.* <__ovly_load>.*

00000340 <00000000\.ovl_call\.f2_a1>:
.*	ila	\$78,1
.*	lnop
.*	ila	\$79,1028	# 404
.*	bra?	.* <__ovly_load>.*

00000350 <00000000.ovl_call.f1_a2>:
.*	ila	\$78,2
.*	lnop
.*	ila	\$79,1024	# 400
.*	bra?	.* <__ovly_load>.*

00000360 <00000000\.ovl_call\.f2_a2>:
.*	ila	\$78,2
.*	lnop
.*	ila	\$79,1060	# 424
.*	bra?	.* <__ovly_load>.*

00000370 <00000000\.ovl_call\.f4_a1>:
.*	ila	\$78,1
.*	lnop
.*	ila	\$79,1040	# 410
.*	bra?	.* <__ovly_load>.*

00000380 <00000000.ovl_call.14:8>:
.*	ila	\$78,2
.*	lnop
.*	ila	\$79,1076	# 434
.*	bra?	.* <__ovly_load>.*

#00000330 <00000000\.ovl_call\.f1_a1>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 04 04 00.*
#
#00000338 <00000000\.ovl_call\.f2_a1>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 04 04 04.*
#
#00000340 <00000000\.ovl_call\.f1_a2>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 08 04 00.*
#
#00000348 <00000000\.ovl_call\.f2_a2>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 08 04 24.*
#
#00000350 <00000000\.ovl_call\.f4_a1>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 04 04 10.*
#
#00000358 <00000000.ovl_call.14:8>:
#.*	bra?sl	\$75,.* <__ovly_load>.*
#.*00 08 04 34.*

Disassembly of section \.ov_a1:

00000400 <f1_a1>:
.*	br	.* <f3_a1>.*
.*SPU_REL16	f3_a1

00000404 <f2_a1>:
.*	ila	\$3,.*
.*SPU_ADDR18	f4_a1
.*	bi	\$0

0000040c <f3_a1>:
.*	bi	\$0

00000410 <f4_a1>:
.*	bi	\$0
	\.\.\.
Disassembly of section \.ov_a2:

00000400 <f1_a2>:
.*	stqd	\$0,16\(\$1\)
.*	stqd	\$1,-32\(\$1\)
.*	ai	\$1,\$1,-32
.*	brsl	\$0,12c <f0>	# 12c
.*SPU_REL16	f0
.*	brsl	\$0,.* <00000000\.ovl_call\.f1_a1>.*
.*SPU_REL16	f1_a1
.*	brsl	\$0,.* <f3_a2>.*
.*SPU_REL16	f3_a2
.*	lqd	\$0,48\(\$1\)	# 30
.*	ai	\$1,\$1,32	# 20
.*	bi	\$0

00000424 <f2_a2>:
.*	ilhu	\$3,.*
.*SPU_ADDR16_HI	f4_a2
.*	iohl	\$3,.*
.*SPU_ADDR16_LO	f4_a2
.*	bi	\$0

00000430 <f3_a2>:
.*	bi	\$0

00000434 <f4_a2>:
.*	br	.* <f3_a2>.*
.*SPU_REL16	f3_a2
	\.\.\.
Disassembly of section .data:

00000440 <_ovly_table-0x10>:
 440:	00 00 00 00 .*
 444:	00 00 00 01 .*
	\.\.\.
00000450 <_ovly_table>:
 450:	00 00 04 00 .*
 454:	00 00 00 20 .*
# 458:	00 00 03 40 .*
 458:	00 00 01 00 .*
 45c:	00 00 00 01 .*
 460:	00 00 04 00 .*
 464:	00 00 00 40 .*
# 468:	00 00 03 60 .*
 468:	00 00 01 20 .*
 46c:	00 00 00 01 .*

00000470 <_ovly_buf_table>:
 470:	00 00 00 00 .*

Disassembly of section \.toe:

00000480 <_EAR_>:
	\.\.\.
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
