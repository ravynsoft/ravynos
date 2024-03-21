#name: Z80 backward PC relative relocations
#source: labels.s -z80
#source: jr.s -z80
#ld: -e 0 -Ttext 0 -Tdata 0x100 -s
#objdump: -d

.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000000 <.*>:
[   ]+0:[ 	]+78[          	]+ld a,b
[   ]+1:[ 	]+79[          	]+ld a,c
[   ]+2:[ 	]+7a[          	]+ld a,d
[   ]+3:[ 	]+7b[          	]+ld a,e
[   ]+4:[ 	]+7c[          	]+ld a,h
[   ]+5:[ 	]+7d[          	]+ld a,l
[   ]+6:[ 	]+7e[          	]+ld a,\(hl\)
[   ]+7:[ 	]+7f[          	]+ld a,a
[   ]+8:[ 	]+2f[          	]+cpl
[   ]+9:[ 	]+10 f5[       	]+djnz 0x0000
[   ]+b:[ 	]+18 f4[       	]+jr 0x0001
[   ]+d:[ 	]+20 f3[       	]+jr nz,0x0002
[   ]+f:[ 	]+28 f2[       	]+jr z,0x0003
[  ]+11:[ 	]+30 f1[       	]+jr nc,0x0004
[  ]+13:[ 	]+38 f0[       	]+jr c,0x0005
[  ]+15:[ 	]+10 fe[       	]+djnz 0x0015
[  ]+17:[ 	]+18 fe[       	]+jr 0x0017
[  ]+19:[ 	]+20 fe[       	]+jr nz,0x0019
[  ]+1b:[ 	]+28 fe[       	]+jr z,0x001b
[  ]+1d:[ 	]+30 fe[       	]+jr nc,0x001d
[  ]+1f:[ 	]+38 fe[       	]+jr c,0x001f
[  ]+21:[ 	]+10 fe[       	]+djnz 0x0021
[  ]+23:[ 	]+18 fe[       	]+jr 0x0023
[  ]+25:[ 	]+20 fe[       	]+jr nz,0x0025
[  ]+27:[ 	]+28 fe[       	]+jr z,0x0027
[  ]+29:[ 	]+30 fe[       	]+jr nc,0x0029
[  ]+2b:[ 	]+38 fe[       	]+jr c,0x002b
[  ]+2d:[ 	]+10 0a[       	]+djnz 0x0039
[  ]+2f:[ 	]+18 09[       	]+jr 0x003a
[  ]+31:[ 	]+20 08[       	]+jr nz,0x003b
[  ]+33:[ 	]+28 07[       	]+jr z,0x003c
[  ]+35:[ 	]+30 06[       	]+jr nc,0x003d
[  ]+37:[ 	]+38 05[       	]+jr c,0x003e
[  ]+39:[ 	]+c9[          	]+ret
[  ]+3a:[ 	]+c9[          	]+ret
[  ]+3b:[ 	]+c9[          	]+ret
[  ]+3c:[ 	]+c9[          	]+ret
[  ]+3d:[ 	]+c9[          	]+ret
[  ]+3e:[ 	]+c9[          	]+ret
