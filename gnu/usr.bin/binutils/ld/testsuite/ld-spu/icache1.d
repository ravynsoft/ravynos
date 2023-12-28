#source: icache1.s
#ld: --soft-icache --num-lines=4 --non-ia-text --auto-overlay=tmpdir/icache1.lnk --auto-relink
#objdump: -D -j.text -j.data -j.bss -j.ovl.init -j.ovly1 -j.ovly2 -j.ovly3 -j.ovly4 -j.ovly5 -j.ovly6 -j.ovly7 -j.ovly8

.* elf32-spu


Disassembly of section \.text:

00000000 <_start>:
.*	41 00 02 03 	ilhu	\$3,4
.*	60 88 00 03 	iohl	\$3,4096	# 1000
.*	32 00 03 80 	br	24.*
0000000c <__icache_br_handler>:
   c:	00 00 00 00 	stop
00000010 <__icache_call_handler>:
	\.\.\.
  20:	00 04 08 00.*
  24:	31 00 02 4b 	brasl	\$75,10 <__icache_call_handler>
  28:	a0 00 00 08.*
  2c:	00 00 fc 80.*
	\.\.\.

Disassembly of section \.data:

.* <(\.data|_edata-0x10)>:
.*	00 04 08 00 	.*
.*	00 04 0d 04 	.*
.*	00 04 0c 00 	.*
.*	00 08 10 00 	.*

Disassembly of section \.bss:

.* <(__icache_tag_array|__bss_start)>:
	\.\.\.

.* <__icache_rewrite_to>:
	\.\.\.

.* <__icache_rewrite_from>:
	\.\.\.

Disassembly of section \.ovl\.init:

00000400 <__icache_fileoff>:
.*	00 00 00 00.*
.*	00 00 02 00.*
	\.\.\.

Disassembly of section \.ovly1:

00000400 <\.ovly1>:
.*	ai	\$1,\$1,64	# 40
.*	lqd	\$0,16\(\$1\)
.*	bi	\$0
	\.\.\.

Disassembly of section \.ovly2:

00000800 <f1>:
.*	40 20 00 00 	nop	\$0
.*	24 00 40 80 	stqd	\$0,16\(\$1\)
.*	1c f0 00 81 	ai	\$1,\$1,-64
.*	24 00 00 81 	stqd	\$1,0\(\$1\)
.*	33 00 78 80 	brsl	\$0,bd4 .*
.*	33 00 7a 00 	brsl	\$0,be4 .*
	\.\.\.
.*	32 00 17 80 	br	bf4 .*
	\.\.\.
 bd0:	00 04 0d 04.*
 bd4:	31 00 01 cb 	brasl	\$75,c .*
 bd8:	a0 00 08 10.*
 bdc:	00 00 e6 00.*
 be0:	00 04 0c 00.*
 be4:	31 00 01 cb 	brasl	\$75,c .*
 be8:	a0 00 08 14.*
 bec:	00 00 07 80.*
 bf0:	00 04 04 00.*
 bf4:	31 00 01 cb 	brasl	\$75,c .*
 bf8:	20 00 0b 38.*
 bfc:	00 7f 0e 80.*

Disassembly of section \.ovly3:

00000c00 <f3>:
	\.\.\.
.*	35 00 00 00 	bi	\$0

00000d04 <f2>:
.*	1c e0 00 81 	ai	\$1,\$1,-128
.*	24 00 00 81 	stqd	\$1,0\(\$1\)
	\.\.\.
.*	1c 20 00 81 	ai	\$1,\$1,128	# 80
.*	35 00 00 00 	bi	\$0
	\.\.\.

Disassembly of section \.ovly4:

00001000 <f5>:
.*	24 00 40 80 	stqd	\$0,16\(\$1\)
.*	24 f8 00 81 	stqd	\$1,-512\(\$1\)
.*	1c 80 00 81 	ai	\$1,\$1,-512
.*	33 7f fe 80 	brsl	\$0,1000 <f5>	# 1000
	\.\.\.
.*	42 01 00 03 	ila	\$3,200.*
.*	18 00 c0 81 	a	\$1,\$1,\$3
.*	34 00 40 80 	lqd	\$0,16\(\$1\)
.*	35 00 00 00 	bi	\$0
	\.\.\.

Disassembly of section \.ovly5:

00000400 <\.ovly5>:
	\.\.\.
.*	42 01 00 03 	ila	\$3,200 .*
.*	18 00 c0 81 	a	\$1,\$1,\$3
.*	34 00 40 80 	lqd	\$0,16\(\$1\)
.*	30 00 fe 80 	bra	7f4 .*
	\.\.\.
 7f0:	00 04 10 00.*
 7f4:	31 00 01 cb 	brasl	\$75,c .*
 7f8:	a0 00 07 2c.*
 7fc:	00 02 fe 80.*

Disassembly of section \.ovly6:

00000800 <\.ovly6>:
.*	31 01 7a 80 	brasl	\$0,bd4 .*
.*	33 00 7c 00 	brsl	\$0,be4 .*
	\.\.\.
.*	32 00 19 80 	br	bf4 .*
	\.\.\.
 bd0:	00 08 10 00.*
 bd4:	31 00 01 cb 	brasl	\$75,c .*
 bd8:	a0 00 08 00.*
 bdc:	00 03 7a 80.*
 be0:	00 08 10 00.*
 be4:	31 00 01 cb 	brasl	\$75,c .*
 be8:	a0 00 08 04.*
 bec:	00 00 83 80.*
 bf0:	00 08 04 00.*
 bf4:	31 00 01 cb 	brasl	\$75,c .*
 bf8:	20 00 0b 28.*
 bfc:	00 7f 02 80.*

Disassembly of section \.ovly7:

00000c00 <\.ovly7>:
.*	41 7f ff 83 	ilhu	\$3,65535	# ffff
.*	60 f8 30 03 	iohl	\$3,61536	# f060
.*	18 00 c0 84 	a	\$4,\$1,\$3
.*	00 20 00 00 	lnop
.*	04 00 02 01 	ori	\$1,\$4,0
.*	24 00 02 04 	stqd	\$4,0\(\$4\)
.*	33 00 77 80 	brsl	\$0,fd4 .*
.*	33 00 79 00 	brsl	\$0,fe4 .*
.*	34 00 00 81 	lqd	\$1,0\(\$1\)
	\.\.\.
.*	32 00 16 00 	br	ff4 .*
	\.\.\.
     fd0:	00 04 10 00.*
     fd4:	31 00 01 cb 	brasl	\$75,c .*
     fd8:	a0 00 0c 18.*
     fdc:	00 00 0a 80.*
     fe0:	00 08 10 00.*
     fe4:	31 00 01 cb 	brasl	\$75,c .*
     fe8:	a0 00 0c 1c.*
     fec:	00 00 05 80.*
     ff0:	00 08 08 00.*
     ff4:	31 00 01 cb 	brasl	\$75,c .*
     ff8:	20 00 0f 44.*
     ffc:	00 7f 01 80.*

Disassembly of section \.ovly8:

00001000 <f4>:
.*	24 00 40 80 	stqd	\$0,16\(\$1\)
.*	24 f8 00 81 	stqd	\$1,-512\(\$1\)
.*	1c 80 00 81 	ai	\$1,\$1,-512
.*	31 02 7c 80 	brasl	\$0,13e4 .*
	\.\.\.
.*	32 00 18 80 	br	13f4 .*
	\.\.\.
    13e0:	00 04 0d 04.*
    13e4:	31 00 01 cb 	brasl	\$75,c .*
    13e8:	a0 00 10 0c.*
    13ec:	00 03 dc 00.*
    13f0:	00 08 0c 00.*
    13f4:	31 00 01 cb 	brasl	\$75,c .*
    13f8:	20 00 13 30.*
    13fc:	00 7f 02 80.*

#pass
