#name: colonless labels
#source: colonless.s -colonless
#objdump: -d

.*:[     ]+file format (coff)|(elf32)\-z80


Disassembly of section \.text:

0+0 <start>:
[   ]+0:[ 	]+3e 00[       	]+ld a,0x00
[   ]+2:[ 	]+28 0d[       	]+jr z,0x0011
[   ]+4:[ 	]+3e 01[       	]+ld a,0x01
[   ]+6:[ 	]+3e 02[       	]+ld a,0x02
[   ]+8:[ 	]+3e 03[       	]+ld a,0x03
[   ]+a:[ 	]+18 f4[       	]+jr 0x0000
[   ]+c:[ 	]+18 f4[       	]+jr 0x0002
[   ]+e:[ 	]+18 f6[       	]+jr 0x0006

0+10 <label>:
[  ]+10:[ 	]+e9[          	]+jp \(hl\)

0+11 <finish>:
[  ]+11:[ 	]+c9[          	]+ret
#pass
