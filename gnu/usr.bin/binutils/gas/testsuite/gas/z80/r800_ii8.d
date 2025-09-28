#as: -march=r800
#objdump: -d
#name: halves of index register for R800
#source: z80_ii8.s

.*:.*

Disassembly of section .text:

0+ <.text>:
[  ]+0:[ 	]+dd 7c[       	]+ld a,ixh
[  ]+2:[ 	]+dd 44[       	]+ld b,ixh
[  ]+4:[ 	]+dd 4c[       	]+ld c,ixh
[  ]+6:[ 	]+dd 54[       	]+ld d,ixh
[  ]+8:[ 	]+dd 5c[       	]+ld e,ixh
[  ]+a:[ 	]+dd 64[       	]+ld ixh,ixh
[  ]+c:[ 	]+dd 6c[       	]+ld ixl,ixh
[  ]+e:[ 	]+dd 7d[       	]+ld a,ixl
[ ]+10:[ 	]+dd 45[       	]+ld b,ixl
[ ]+12:[ 	]+dd 4d[       	]+ld c,ixl
[ ]+14:[ 	]+dd 55[       	]+ld d,ixl
[ ]+16:[ 	]+dd 5d[       	]+ld e,ixl
[ ]+18:[ 	]+dd 65[       	]+ld ixh,ixl
[ ]+1a:[ 	]+dd 6d[       	]+ld ixl,ixl
[ ]+1c:[ 	]+fd 7c[       	]+ld a,iyh
[ ]+1e:[ 	]+fd 44[       	]+ld b,iyh
[ ]+20:[ 	]+fd 4c[       	]+ld c,iyh
[ ]+22:[ 	]+fd 54[       	]+ld d,iyh
[ ]+24:[ 	]+fd 5c[       	]+ld e,iyh
[ ]+26:[ 	]+fd 64[       	]+ld iyh,iyh
[ ]+28:[ 	]+fd 6c[       	]+ld iyl,iyh
[ ]+2a:[ 	]+fd 7d[       	]+ld a,iyl
[ ]+2c:[ 	]+fd 45[       	]+ld b,iyl
[ ]+2e:[ 	]+fd 4d[       	]+ld c,iyl
[ ]+30:[ 	]+fd 55[       	]+ld d,iyl
[ ]+32:[ 	]+fd 5d[       	]+ld e,iyl
[ ]+34:[ 	]+fd 65[       	]+ld iyh,iyl
[ ]+36:[ 	]+fd 6d[       	]+ld iyl,iyl
[ ]+38:[ 	]+dd 67[       	]+ld ixh,a
[ ]+3a:[ 	]+dd 60[       	]+ld ixh,b
[ ]+3c:[ 	]+dd 61[       	]+ld ixh,c
[ ]+3e:[ 	]+dd 62[       	]+ld ixh,d
[ ]+40:[ 	]+dd 63[       	]+ld ixh,e
[ ]+42:[ 	]+dd 64[       	]+ld ixh,ixh
[ ]+44:[ 	]+dd 65[       	]+ld ixh,ixl
[ ]+46:[ 	]+dd 26 19[    	]+ld ixh,0x19
[ ]+49:[ 	]+dd 6f[       	]+ld ixl,a
[ ]+4b:[ 	]+dd 68[       	]+ld ixl,b
[ ]+4d:[ 	]+dd 69[       	]+ld ixl,c
[ ]+4f:[ 	]+dd 6a[       	]+ld ixl,d
[ ]+51:[ 	]+dd 6b[       	]+ld ixl,e
[ ]+53:[ 	]+dd 6c[       	]+ld ixl,ixh
[ ]+55:[ 	]+dd 6d[       	]+ld ixl,ixl
[ ]+57:[ 	]+dd 2e 19[    	]+ld ixl,0x19
[ ]+5a:[ 	]+fd 67[       	]+ld iyh,a
[ ]+5c:[ 	]+fd 60[       	]+ld iyh,b
[ ]+5e:[ 	]+fd 61[       	]+ld iyh,c
[ ]+60:[ 	]+fd 62[       	]+ld iyh,d
[ ]+62:[ 	]+fd 63[       	]+ld iyh,e
[ ]+64:[ 	]+fd 64[       	]+ld iyh,iyh
[ ]+66:[ 	]+fd 65[       	]+ld iyh,iyl
[ ]+68:[ 	]+fd 26 19[    	]+ld iyh,0x19
[ ]+6b:[ 	]+fd 6f[       	]+ld iyl,a
[ ]+6d:[ 	]+fd 68[       	]+ld iyl,b
[ ]+6f:[ 	]+fd 69[       	]+ld iyl,c
[ ]+71:[ 	]+fd 6a[       	]+ld iyl,d
[ ]+73:[ 	]+fd 6b[       	]+ld iyl,e
[ ]+75:[ 	]+fd 6c[       	]+ld iyl,iyh
[ ]+77:[ 	]+fd 6d[       	]+ld iyl,iyl
[ ]+79:[ 	]+fd 2e 19[    	]+ld iyl,0x19
[ ]+7c:[ 	]+dd 84[       	]+add a,ixh
[ ]+7e:[ 	]+dd 85[       	]+add a,ixl
[ ]+80:[ 	]+fd 84[       	]+add a,iyh
[ ]+82:[ 	]+fd 85[       	]+add a,iyl
[ ]+84:[ 	]+dd 8c[       	]+adc a,ixh
[ ]+86:[ 	]+dd 8d[       	]+adc a,ixl
[ ]+88:[ 	]+fd 8c[       	]+adc a,iyh
[ ]+8a:[ 	]+fd 8d[       	]+adc a,iyl
[ ]+8c:[ 	]+dd bc[       	]+cp ixh
[ ]+8e:[ 	]+dd bd[       	]+cp ixl
[ ]+90:[ 	]+fd bc[       	]+cp iyh
[ ]+92:[ 	]+fd bd[       	]+cp iyl
[ ]+94:[ 	]+dd 25[       	]+dec ixh
[ ]+96:[ 	]+dd 2d[       	]+dec ixl
[ ]+98:[ 	]+fd 25[       	]+dec iyh
[ ]+9a:[ 	]+fd 2d[       	]+dec iyl
[ ]+9c:[ 	]+dd 24[       	]+inc ixh
[ ]+9e:[ 	]+dd 2c[       	]+inc ixl
[ ]+a0:[ 	]+fd 24[       	]+inc iyh
[ ]+a2:[ 	]+fd 2c[       	]+inc iyl
[ ]+a4:[ 	]+dd 9c[       	]+sbc a,ixh
[ ]+a6:[ 	]+dd 9d[       	]+sbc a,ixl
[ ]+a8:[ 	]+fd 9c[       	]+sbc a,iyh
[ ]+aa:[ 	]+fd 9d[       	]+sbc a,iyl
[ ]+ac:[ 	]+dd 94[       	]+sub ixh
[ ]+ae:[ 	]+dd 95[       	]+sub ixl
[ ]+b0:[ 	]+fd 94[       	]+sub iyh
[ ]+b2:[ 	]+fd 95[       	]+sub iyl
[ ]+b4:[ 	]+dd a4[       	]+and ixh
[ ]+b6:[ 	]+dd a5[       	]+and ixl
[ ]+b8:[ 	]+fd a4[       	]+and iyh
[ ]+ba:[ 	]+fd a5[       	]+and iyl
[ ]+bc:[ 	]+dd b4[       	]+or ixh
[ ]+be:[ 	]+dd b5[       	]+or ixl
[ ]+c0:[ 	]+fd b4[       	]+or iyh
[ ]+c2:[ 	]+fd b5[       	]+or iyl
[ ]+c4:[ 	]+dd ac[       	]+xor ixh
[ ]+c6:[ 	]+dd ad[       	]+xor ixl
[ ]+c8:[ 	]+fd ac[       	]+xor iyh
[ ]+ca:[ 	]+fd ad[       	]+xor iyl
