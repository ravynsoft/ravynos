#name: Z80 forward PC relative relocations
#source: jr.s -z80
#source: labels.s -z80
#ld: -e 0 -Ttext 0 -Tdata 0x100
#objdump: -d


.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000000 <.*>:
[   ]+0:[ 	]+10 34[       	]+djnz 0x0036
[   ]+2:[ 	]+18 33[       	]+jr 0x0037
[   ]+4:[ 	]+20 32[       	]+jr nz,0x0038
[   ]+6:[ 	]+28 31[       	]+jr z,0x0039
[   ]+8:[ 	]+30 30[       	]+jr nc,0x003a
[   ]+a:[ 	]+38 2f[       	]+jr c,0x003b
[   ]+c:[ 	]+10 fe[       	]+djnz 0x000c
[   ]+e:[ 	]+18 fe[       	]+jr 0x000e
[  ]+10:[ 	]+20 fe[       	]+jr nz,0x0010
[  ]+12:[ 	]+28 fe[       	]+jr z,0x0012
[  ]+14:[ 	]+30 fe[       	]+jr nc,0x0014
[  ]+16:[ 	]+38 fe[       	]+jr c,0x0016
[  ]+18:[ 	]+10 fe[       	]+djnz 0x0018
[  ]+1a:[ 	]+18 fe[       	]+jr 0x001a
[  ]+1c:[ 	]+20 fe[       	]+jr nz,0x001c
[  ]+1e:[ 	]+28 fe[       	]+jr z,0x001e
[  ]+20:[ 	]+30 fe[       	]+jr nc,0x0020
[  ]+22:[ 	]+38 fe[       	]+jr c,0x0022
[  ]+24:[ 	]+10 0a[       	]+djnz 0x0030
[  ]+26:[ 	]+18 09[       	]+jr 0x0031
[  ]+28:[ 	]+20 08[       	]+jr nz,0x0032
[  ]+2a:[ 	]+28 07[       	]+jr z,0x0033
[  ]+2c:[ 	]+30 06[       	]+jr nc,0x0034
[  ]+2e:[ 	]+38 05[       	]+jr c,0x0035
[  ]+30:[ 	]+c9[          	]+ret
[  ]+31:[ 	]+c9[          	]+ret
[  ]+32:[ 	]+c9[          	]+ret
[  ]+33:[ 	]+c9[          	]+ret
[  ]+34:[ 	]+c9[          	]+ret
[  ]+35:[ 	]+c9[          	]+ret

00000036 <label1>:
[  ]+36:[ 	]+78[          	]+ld a,b

00000037 <label2>:
[  ]+37:[ 	]+79[          	]+ld a,c

00000038 <label3>:
[  ]+38:[ 	]+7a[          	]+ld a,d

00000039 <label4>:
[  ]+39:[ 	]+7b[          	]+ld a,e

0000003a <label5>:
[  ]+3a:[ 	]+7c[          	]+ld a,h

0000003b <label6>:
[  ]+3b:[ 	]+7d[          	]+ld a,l

0000003c <label7>:
[  ]+3c:[ 	]+7e[          	]+ld a,\(hl\)

0000003d <label8>:
[  ]+3d:[ 	]+7f[          	]+ld a,a

0000003e <label9>:
[  ]+3e:[ 	]+2f[          	]+cpl
