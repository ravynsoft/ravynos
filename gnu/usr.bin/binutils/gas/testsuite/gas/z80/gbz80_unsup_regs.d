#name: GBZ80 use unsupported registers as labels
#as: -march=gbz80 --defsym NO_XYHL= --defsym NO_REG_F= --defsym NO_REG_R= --defsym NO_REG_I= --defsym NO_INDEX=
#objdump: -d
#source: unsup_regs.s

.*: .*

Disassembly of section \.text:

0+00 <_start>:
[   ]+0:[ 	]+3e 02[      	]+ld a,0x02

0+02 <ixl>:
[   ]+2:[ 	]+06 04[      	]+ld b,0x04

0+04 <ixh>:
[   ]+4:[ 	]+0e 06[      	]+ld c,0x06

0+06 <iyl>:
[   ]+6:[ 	]+16 08[      	]+ld d,0x08

0+08 <f>:
[   ]+8:[ 	]+3e 08[      	]+ld a,0x08

0+0a <ix>:
[   ]+a:[ 	]+21 0a 00[   	]+ld hl,0x000a

0+0d <iy>:
[   ]+d:[ 	]+01 0d 00[   	]+ld bc,0x000d
[  ]+10:[ 	]+fa 09 00[   	]+ld a,\(0x0009\)
[  ]+13:[ 	]+ea 0e 00[   	]+ld \(0x000e\),a

0+16 <r>:
[  ]+16:[ 	]+3e 16[      	]+ld a,0x16
[  ]+18:[ 	]+ea 16 00[   	]+ld \(0x0016\),a

0+1b <i>:
[  ]+1b:[ 	]+3e 1b[      	]+ld a,0x1b
[  ]+1d:[ 	]+ea 1b 00[   	]+ld \(0x001b\),a

0+20 <mb>:
[  ]+20:[ 	]+21 20 00[   	]+ld hl,0x0020
[  ]+23:[ 	]+3e 20[      	]+ld a,0x20
[  ]+25:[ 	]+ea 20 00[   	]+ld \(0x0020\),a
#pass
