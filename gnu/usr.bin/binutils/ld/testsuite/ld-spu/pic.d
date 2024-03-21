#source: pic.s
#source: picdef.s
#ld: --emit-relocs
#objdump: -D -r

.*elf32-spu


Disassembly of section \.text:

00000000 <before>:
	\.\.\.

00000008 <_start>:
   8:	42 00 08 02 	ila	\$2,10 <_start\+0x8>
			8: SPU_ADDR18	\.text\+0x10
   c:	33 00 00 fe 	brsl	\$126,10 <_start\+0x8>
			c: SPU_REL16	\.text\+0x10
  10:	08 1f 81 7e 	sf	\$126,\$2,\$126
  14:	42 00 02 04 	ila	\$4,4 <before\+0x4>
			14: SPU_ADDR18	\.text\+0x4
  18:	42 00 42 05 	ila	\$5,84 <end>
			18: SPU_ADDR18	\.text\+0x84
  1c:	42 00 04 06 	ila	\$6,8 <_start>
			1c: SPU_ADDR18	_start
  20:	42 00 42 07 	ila	\$7,84 <end>
			20: SPU_ADDR18	\.text\+0x84
  24:	18 1f 82 04 	a	\$4,\$4,\$126
			24: SPU_ADD_PIC	before\+0x4
  28:	18 1f 82 85 	a	\$5,\$5,\$126
			28: SPU_ADD_PIC	after-0x4
  2c:	18 1f 83 06 	a	\$6,\$6,\$126
			2c: SPU_ADD_PIC	_start
  30:	18 1f 83 87 	a	\$7,\$7,\$126
			30: SPU_ADD_PIC	end
  34:	42 00 00 0e 	ila	\$14,0
			34: SPU_ADDR18	\.text
  38:	18 1f 87 0e 	a	\$14,\$14,\$126
			38: SPU_ADD_PIC	before
  3c:	42 00 00 03 	ila	\$3,0
			3c: SPU_ADDR18	undef
  40:	1c 00 01 83 	ai	\$3,\$3,0
			40: SPU_ADD_PIC	undef
  44:	41 00 00 07 	ilhu	\$7,0
			44: SPU_ADDR16_HI	ext
  48:	60 ab 3c 07 	iohl	\$7,22136	# 5678
			48: SPU_ADDR16_LO	ext
  4c:	18 1f 83 84 	a	\$4,\$7,\$126
			4c: SPU_ADD_PIC	ext
  50:	42 00 80 09 	ila	\$9,100 <loc>
			50: SPU_ADDR18	\.data
  54:	18 1f 84 85 	a	\$5,\$9,\$126
			54: SPU_ADD_PIC	loc
  58:	42 00 88 08 	ila	\$8,110 <glob>
			58: SPU_ADDR18	glob
  5c:	18 1f 84 06 	a	\$6,\$8,\$126
			5c: SPU_ADD_PIC	glob
  60:	42 00 90 09 	ila	\$9,120 .*
			60: SPU_ADDR18	_end
  64:	18 1f 84 89 	a	\$9,\$9,\$126
			64: SPU_ADD_PIC	_end
  68:	12 02 39 85 	hbrr	7c <acall>,1234 <abscall>	# 1234
			68: SPU_REL16	abscall
  6c:	33 ff f2 82 	lqr	\$2,0 <before>
			6c: SPU_REL16	undef
  70:	23 ff f2 02 	stqr	\$2,0 <before>
			70: SPU_REL16	undef
  74:	33 8a c0 83 	lqr	\$3,5678 <ext>	# 5678
			74: SPU_REL16	ext
  78:	33 8a c2 04 	lqr	\$4,5688 <ext\+0x10>	# 5688
			78: SPU_REL16	ext\+0x10

0000007c <acall>:
  7c:	33 02 37 00 	brsl	\$0,1234 <abscall>	# 1234
			7c: SPU_REL16	abscall
  80:	32 02 36 80 	br	1234 <abscall>	# 1234
			80: SPU_REL16	abscall

00000084 <end>:
  84:	00 00 00 00 	stop

00000088 <after>:
  88:	00 00 00 00 	stop

Disassembly of section \.data:

00000100 <loc>:
 100:	00 00 00 01 	stop
	\.\.\.

00000110 <glob>:
 110:	00 00 00 02 	stop
	\.\.\.

Disassembly of section \.note\.spu_name:

00000000 <\.note\.spu_name>:
.*
.*
.*
.*
.*
.*
.*
#pass
