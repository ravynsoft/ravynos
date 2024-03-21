#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE relocations

.*: +file format elf.*-powerpc.*

Disassembly of section \.text:

00000000 <.text>:
   0:	e8 00       	se_b    0x0
			0: R_PPC_VLE_REL8	sub1
   2:	e9 00       	se_bl   0x2
			2: R_PPC_VLE_REL8	sub1
   4:	e1 00       	se_ble  0x4
			4: R_PPC_VLE_REL8	sub2
   6:	e6 00       	se_beq  0x6
			6: R_PPC_VLE_REL8	sub2
   8:	78 00 00 00 	e_b     0x8
			8: R_PPC_VLE_REL24	sub3
   c:	78 00 00 01 	e_bl    0xc
			c: R_PPC_VLE_REL24	sub4
  10:	7a 05 00 00 	e_ble   cr1,0x10
			10: R_PPC_VLE_REL15	sub5
  14:	7a 1a 00 01 	e_beql  cr2,0x14
			14: R_PPC_VLE_REL15	sub5

  18:	70 20 c0 00 	e_or2i  r1,0
			18: R_PPC_VLE_LO16A	low
  1c:	70 40 c0 00 	e_or2i  r2,0
			1c: R_PPC_VLE_HI16A	high
  20:	70 60 c0 00 	e_or2i  r3,0
			20: R_PPC_VLE_HA16A	high_adjust
  24:	70 80 c0 00 	e_or2i  r4,0
			24: R_PPC_VLE_SDAREL_LO16A	low_sdarel
  28:	70 a0 c0 00 	e_or2i  r5,0
			28: R_PPC_VLE_SDAREL_HI16A	high_sdarel
  2c:	70 40 c0 00 	e_or2i  r2,0
			2c: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  30:	70 20 c8 00 	e_and2i. r1,0
			30: R_PPC_VLE_LO16A	low
  34:	70 40 c8 00 	e_and2i. r2,0
			34: R_PPC_VLE_HI16A	high
  38:	70 60 c8 00 	e_and2i. r3,0
			38: R_PPC_VLE_HA16A	high_adjust
  3c:	70 80 c8 00 	e_and2i. r4,0
			3c: R_PPC_VLE_SDAREL_LO16A	low_sdarel
  40:	70 a0 c8 00 	e_and2i. r5,0
			40: R_PPC_VLE_SDAREL_HI16A	high_sdarel
  44:	70 40 c8 00 	e_and2i. r2,0
			44: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  48:	70 40 c8 00 	e_and2i. r2,0
			48: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  4c:	70 20 d0 00 	e_or2is r1,0
			4c: R_PPC_VLE_LO16A	low
  50:	70 40 d0 00 	e_or2is r2,0
			50: R_PPC_VLE_HI16A	high
  54:	70 60 d0 00 	e_or2is r3,0
			54: R_PPC_VLE_HA16A	high_adjust
  58:	70 80 d0 00 	e_or2is r4,0
			58: R_PPC_VLE_SDAREL_LO16A	low_sdarel
  5c:	70 a0 d0 00 	e_or2is r5,0
			5c: R_PPC_VLE_SDAREL_HI16A	high_sdarel
  60:	70 40 d0 00 	e_or2is r2,0
			60: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  64:	70 20 e0 00 	e_lis   r1,0
			64: R_PPC_VLE_LO16A	low
  68:	70 40 e0 00 	e_lis   r2,0
			68: R_PPC_VLE_HI16A	high
  6c:	70 60 e0 00 	e_lis   r3,0
			6c: R_PPC_VLE_HA16A	high_adjust
  70:	70 80 e0 00 	e_lis   r4,0
			70: R_PPC_VLE_SDAREL_LO16A	low_sdarel
  74:	70 a0 e0 00 	e_lis   r5,0
			74: R_PPC_VLE_SDAREL_HI16A	high_sdarel
  78:	70 40 e0 00 	e_lis   r2,0
			78: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  7c:	70 20 e8 00 	e_and2is. r1,0
			7c: R_PPC_VLE_LO16A	low
  80:	70 40 e8 00 	e_and2is. r2,0
			80: R_PPC_VLE_HI16A	high
  84:	70 60 e8 00 	e_and2is. r3,0
			84: R_PPC_VLE_HA16A	high_adjust
  88:	70 80 e8 00 	e_and2is. r4,0
			88: R_PPC_VLE_SDAREL_LO16A	low_sdarel
  8c:	70 a0 e8 00 	e_and2is. r5,0
			8c: R_PPC_VLE_SDAREL_HI16A	high_sdarel
  90:	70 40 e8 00 	e_and2is. r2,0
			90: R_PPC_VLE_SDAREL_HA16A	high_adjust_sdarel
  94:	70 01 98 00 	e_cmp16i r1,0
			94: R_PPC_VLE_LO16D	low
  98:	70 02 98 00 	e_cmp16i r2,0
			98: R_PPC_VLE_HI16D	high
  9c:	70 03 98 00 	e_cmp16i r3,0
			9c: R_PPC_VLE_HA16D	high_adjust
  a0:	70 04 98 00 	e_cmp16i r4,0
			a0: R_PPC_VLE_SDAREL_LO16D	low_sdarel
  a4:	70 05 98 00 	e_cmp16i r5,0
			a4: R_PPC_VLE_SDAREL_HI16D	high_sdarel
  a8:	70 02 98 00 	e_cmp16i r2,0
			a8: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
  ac:	70 01 a8 00 	e_cmpl16i r1,0
			ac: R_PPC_VLE_LO16D	low
  b0:	70 02 a8 00 	e_cmpl16i r2,0
			b0: R_PPC_VLE_HI16D	high
  b4:	70 03 a8 00 	e_cmpl16i r3,0
			b4: R_PPC_VLE_HA16D	high_adjust
  b8:	70 04 a8 00 	e_cmpl16i r4,0
			b8: R_PPC_VLE_SDAREL_LO16D	low_sdarel
  bc:	70 05 a8 00 	e_cmpl16i r5,0
			bc: R_PPC_VLE_SDAREL_HI16D	high_sdarel
  c0:	70 02 a8 00 	e_cmpl16i r2,0
			c0: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
  c4:	70 01 b0 00 	e_cmph16i r1,0
			c4: R_PPC_VLE_LO16D	low
  c8:	70 02 b0 00 	e_cmph16i r2,0
			c8: R_PPC_VLE_HI16D	high
  cc:	70 03 b0 00 	e_cmph16i r3,0
			cc: R_PPC_VLE_HA16D	high_adjust
  d0:	70 04 b0 00 	e_cmph16i r4,0
			d0: R_PPC_VLE_SDAREL_LO16D	low_sdarel
  d4:	70 05 b0 00 	e_cmph16i r5,0
			d4: R_PPC_VLE_SDAREL_HI16D	high_sdarel
  d8:	70 02 b0 00 	e_cmph16i r2,0
			d8: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
  dc:	70 01 b8 00 	e_cmphl16i r1,0
			dc: R_PPC_VLE_LO16D	low
  e0:	70 02 b8 00 	e_cmphl16i r2,0
			e0: R_PPC_VLE_HI16D	high
  e4:	70 03 b8 00 	e_cmphl16i r3,0
			e4: R_PPC_VLE_HA16D	high_adjust
  e8:	70 04 b8 00 	e_cmphl16i r4,0
			e8: R_PPC_VLE_SDAREL_LO16D	low_sdarel
  ec:	70 05 b8 00 	e_cmphl16i r5,0
			ec: R_PPC_VLE_SDAREL_HI16D	high_sdarel
  f0:	70 02 b8 00 	e_cmphl16i r2,0
			f0: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
  f4:	70 01 88 00 	e_add2i. r1,0
			f4: R_PPC_VLE_LO16D	low
  f8:	70 02 88 00 	e_add2i. r2,0
			f8: R_PPC_VLE_HI16D	high
  fc:	70 03 88 00 	e_add2i. r3,0
			fc: R_PPC_VLE_HA16D	high_adjust
 100:	70 04 88 00 	e_add2i. r4,0
			100: R_PPC_VLE_SDAREL_LO16D	low_sdarel
 104:	70 05 88 00 	e_add2i. r5,0
			104: R_PPC_VLE_SDAREL_HI16D	high_sdarel
 108:	70 02 88 00 	e_add2i. r2,0
			108: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
 10c:	70 01 90 00 	e_add2is r1,0
			10c: R_PPC_VLE_LO16D	low
 110:	70 02 90 00 	e_add2is r2,0
			110: R_PPC_VLE_HI16D	high
 114:	70 03 90 00 	e_add2is r3,0
			114: R_PPC_VLE_HA16D	high_adjust
 118:	70 04 90 00 	e_add2is r4,0
			118: R_PPC_VLE_SDAREL_LO16D	low_sdarel
 11c:	70 05 90 00 	e_add2is r5,0
			11c: R_PPC_VLE_SDAREL_HI16D	high_sdarel
 120:	70 02 90 00 	e_add2is r2,0
			120: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
 124:	70 01 a0 00 	e_mull2i r1,0
			124: R_PPC_VLE_LO16D	low
 128:	70 02 a0 00 	e_mull2i r2,0
			128: R_PPC_VLE_HI16D	high
 12c:	70 03 a0 00 	e_mull2i r3,0
			12c: R_PPC_VLE_HA16D	high_adjust
 130:	70 04 a0 00 	e_mull2i r4,0
			130: R_PPC_VLE_SDAREL_LO16D	low_sdarel
 134:	70 05 a0 00 	e_mull2i r5,0
			134: R_PPC_VLE_SDAREL_HI16D	high_sdarel
 138:	70 02 a0 00 	e_mull2i r2,0
			138: R_PPC_VLE_SDAREL_HA16D	high_adjust_sdarel
