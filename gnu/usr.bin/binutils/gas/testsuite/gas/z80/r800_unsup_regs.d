#name: R800 use unsupported registers as labels
#as: -march=r800
#objdump: -d
#source: unsup_regs.s

.*: .*

Disassembly of section \.text:

0+00 <_start>:
[   ]+0:[ 	]+dd 7d[      	]+ld a,ixl
[   ]+2:[ 	]+dd 67[      	]+ld ixh,a
[   ]+4:[ 	]+dd 44[      	]+ld b,ixh
[   ]+6:[ 	]+dd 68[      	]+ld ixl,b
[   ]+8:[ 	]+fd 4d[      	]+ld c,iyl
[   ]+a:[ 	]+fd 61[      	]+ld iyh,c
[   ]+c:[ 	]+fd 54[      	]+ld d,iyh
[   ]+e:[ 	]+fd 6a[      	]+ld iyl,d
[  ]+10:[ 	]+ed 70[      	]+in f,\(c\)
[  ]+12:[ 	]+dd 29[      	]+add ix,ix
[  ]+14:[ 	]+dd 86 01[   	]+add a,\(ix\+1\)
[  ]+17:[ 	]+dd 21 34 12 	ld ix,0x1234
[  ]+1b:[ 	]+fd 21 21 43 	ld iy,0x4321
[  ]+1f:[ 	]+fd 22 34 12 	ld \(0x1234\),iy
[  ]+23:[ 	]+fd 77 ff[   	]+ld \(iy\-1\),a
[  ]+26:[ 	]+ed 5f[      	]+ld a,r
[  ]+28:[ 	]+ed 4f[      	]+ld r,a
[  ]+2a:[ 	]+ed 57[      	]+ld a,i
[  ]+2c:[ 	]+ed 47[      	]+ld i,a

0+2e <mb>:
[  ]+2e:[ 	]+21 2e 00[   	]+ld hl,0x002e
[  ]+31:[ 	]+3e 2e[      	]+ld a,0x2e
[  ]+33:[ 	]+32 2e 00[   	]+ld \(0x002e\),a
#pass
