#name: Z180 use unsupported registers as labels
#as: -march=z180 --defsym NO_REG_F= --defsym NO_XYHL=
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
[   ]+a:[ 	]+dd 29[      	]+add ix,ix
[   ]+c:[ 	]+dd 86 01[   	]+add a,\(ix\+1\)
[   ]+f:[ 	]+dd 21 34 12 	ld ix,0x1234
[  ]+13:[ 	]+fd 21 21 43 	ld iy,0x4321
[  ]+17:[ 	]+fd 22 34 12 	ld \(0x1234\),iy
[  ]+1b:[ 	]+fd 77 ff[   	]+ld \(iy\-1\),a
[  ]+1e:[ 	]+ed 5f[      	]+ld a,r
[  ]+20:[ 	]+ed 4f[      	]+ld r,a
[  ]+22:[ 	]+ed 57[      	]+ld a,i
[  ]+24:[ 	]+ed 47[      	]+ld i,a

0+26 <mb>:
[  ]+26:[ 	]+21 26 00[   	]+ld hl,0x0026
[  ]+29:[ 	]+3e 26[      	]+ld a,0x26
[  ]+2b:[ 	]+32 26 00[   	]+ld \(0x0026\),a
#pass
