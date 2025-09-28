#name: SDCC compatibility mode
#as: -sdcc
#objdump: -d -j _CODE

.*:[     ]+file format (coff)|(elf32)\-z80

Disassembly of section _CODE:

0+0 <_start>:
[   ]+0:[ 	]+21 04 00[    	]+ld hl,0x0004
[   ]+3:[ 	]+8f[          	]+adc a,a
[   ]+4:[ 	]+88[          	]+adc a,b
[   ]+5:[ 	]+89[          	]+adc a,c
[   ]+6:[ 	]+8a[          	]+adc a,d
[   ]+7:[ 	]+8b[          	]+adc a,e
[   ]+8:[ 	]+8c[          	]+adc a,h
[   ]+9:[ 	]+8d[          	]+adc a,l
[   ]+a:[ 	]+dd 8c[       	]+adc a,ixh
[   ]+c:[ 	]+dd 8d[       	]+adc a,ixl
[   ]+e:[ 	]+fd 8c[       	]+adc a,iyh
[  ]+10:[ 	]+fd 8d[       	]+adc a,iyl
[  ]+12:[ 	]+ce a5[       	]+adc a,0xa5
[  ]+14:[ 	]+8e[          	]+adc a,\(hl\)
[  ]+15:[ 	]+dd 8e 05[    	]+adc a,\(ix\+5\)
[  ]+18:[ 	]+fd 8e fe[    	]+adc a,\(iy\-2\)
[  ]+1b:[ 	]+87[          	]+add a,a
[  ]+1c:[ 	]+80[          	]+add a,b
[  ]+1d:[ 	]+81[          	]+add a,c
[  ]+1e:[ 	]+82[          	]+add a,d
[  ]+1f:[ 	]+83[          	]+add a,e
[  ]+20:[ 	]+84[          	]+add a,h
[  ]+21:[ 	]+85[          	]+add a,l
[  ]+22:[ 	]+dd 84[       	]+add a,ixh
[  ]+24:[ 	]+dd 85[       	]+add a,ixl
[  ]+26:[ 	]+fd 84[       	]+add a,iyh
[  ]+28:[ 	]+fd 85[       	]+add a,iyl
[  ]+2a:[ 	]+c6 a5[       	]+add a,0xa5
[  ]+2c:[ 	]+86[          	]+add a,\(hl\)
[  ]+2d:[ 	]+dd 86 05[    	]+add a,\(ix\+5\)
[  ]+30:[ 	]+fd 86 fe[    	]+add a,\(iy\-2\)
[  ]+33:[ 	]+a7[          	]+and a
[  ]+34:[ 	]+a0[          	]+and b
[  ]+35:[ 	]+a1[          	]+and c
[  ]+36:[ 	]+a2[          	]+and d
[  ]+37:[ 	]+a3[          	]+and e
[  ]+38:[ 	]+a4[          	]+and h
[  ]+39:[ 	]+a5[          	]+and l
[  ]+3a:[ 	]+dd a4[       	]+and ixh
[  ]+3c:[ 	]+dd a5[       	]+and ixl
[  ]+3e:[ 	]+fd a4[       	]+and iyh
[  ]+40:[ 	]+fd a5[       	]+and iyl
[  ]+42:[ 	]+e6 a5[       	]+and 0xa5
[  ]+44:[ 	]+a6[          	]+and \(hl\)
[  ]+45:[ 	]+dd a6 05[    	]+and \(ix\+5\)
[  ]+48:[ 	]+fd a6 fe[    	]+and \(iy\-2\)
[  ]+4b:[ 	]+bf[          	]+cp a
[  ]+4c:[ 	]+b8[          	]+cp b
[  ]+4d:[ 	]+b9[          	]+cp c
[  ]+4e:[ 	]+ba[          	]+cp d
[  ]+4f:[ 	]+bb[          	]+cp e
[  ]+50:[ 	]+bc[          	]+cp h
[  ]+51:[ 	]+bd[          	]+cp l
[  ]+52:[ 	]+dd bc[       	]+cp ixh
[  ]+54:[ 	]+dd bd[       	]+cp ixl
[  ]+56:[ 	]+fd bc[       	]+cp iyh
[  ]+58:[ 	]+fd bd[       	]+cp iyl
[  ]+5a:[ 	]+fe a5[       	]+cp 0xa5
[  ]+5c:[ 	]+be[          	]+cp \(hl\)
[  ]+5d:[ 	]+dd be 05[    	]+cp \(ix\+5\)
[  ]+60:[ 	]+fd be fe[    	]+cp \(iy\-2\)
[  ]+63:[ 	]+b7[          	]+or a
[  ]+64:[ 	]+b0[          	]+or b
[  ]+65:[ 	]+b1[          	]+or c
[  ]+66:[ 	]+b2[          	]+or d
[  ]+67:[ 	]+b3[          	]+or e
[  ]+68:[ 	]+b4[          	]+or h
[  ]+69:[ 	]+b5[          	]+or l
[  ]+6a:[ 	]+dd b4[       	]+or ixh
[  ]+6c:[ 	]+dd b5[       	]+or ixl
[  ]+6e:[ 	]+fd b4[       	]+or iyh
[  ]+70:[ 	]+fd b5[       	]+or iyl
[  ]+72:[ 	]+f6 a5[       	]+or 0xa5
[  ]+74:[ 	]+b6[          	]+or \(hl\)
[  ]+75:[ 	]+dd b6 05[    	]+or \(ix\+5\)
[  ]+78:[ 	]+fd b6 fe[    	]+or \(iy\-2\)
[  ]+7b:[ 	]+9f[          	]+sbc a,a
[  ]+7c:[ 	]+98[          	]+sbc a,b
[  ]+7d:[ 	]+99[          	]+sbc a,c
[  ]+7e:[ 	]+9a[          	]+sbc a,d
[  ]+7f:[ 	]+9b[          	]+sbc a,e
[  ]+80:[ 	]+9c[          	]+sbc a,h
[  ]+81:[ 	]+9d[          	]+sbc a,l
[  ]+82:[ 	]+dd 9c[       	]+sbc a,ixh
[  ]+84:[ 	]+dd 9d[       	]+sbc a,ixl
[  ]+86:[ 	]+fd 9c[       	]+sbc a,iyh
[  ]+88:[ 	]+fd 9d[       	]+sbc a,iyl
[  ]+8a:[ 	]+de a5[       	]+sbc a,0xa5
[  ]+8c:[ 	]+9e[          	]+sbc a,\(hl\)
[  ]+8d:[ 	]+dd 9e 05[    	]+sbc a,\(ix\+5\)
[  ]+90:[ 	]+fd 9e fe[    	]+sbc a,\(iy\-2\)
[  ]+93:[ 	]+97[          	]+sub a
[  ]+94:[ 	]+90[          	]+sub b
[  ]+95:[ 	]+91[          	]+sub c
[  ]+96:[ 	]+92[          	]+sub d
[  ]+97:[ 	]+93[          	]+sub e
[  ]+98:[ 	]+94[          	]+sub h
[  ]+99:[ 	]+95[          	]+sub l
[  ]+9a:[ 	]+dd 94[       	]+sub ixh
[  ]+9c:[ 	]+dd 95[       	]+sub ixl
[  ]+9e:[ 	]+fd 94[       	]+sub iyh
[  ]+a0:[ 	]+fd 95[       	]+sub iyl
[  ]+a2:[ 	]+d6 a5[       	]+sub 0xa5
[  ]+a4:[ 	]+96[          	]+sub \(hl\)
[  ]+a5:[ 	]+dd 96 05[    	]+sub \(ix\+5\)
[  ]+a8:[ 	]+fd 96 fe[    	]+sub \(iy\-2\)
[  ]+ab:[ 	]+af[          	]+xor a
[  ]+ac:[ 	]+a8[          	]+xor b
[  ]+ad:[ 	]+a9[          	]+xor c
[  ]+ae:[ 	]+aa[          	]+xor d
[  ]+af:[ 	]+ab[          	]+xor e
[  ]+b0:[ 	]+ac[          	]+xor h
[  ]+b1:[ 	]+ad[          	]+xor l
[  ]+b2:[ 	]+dd ac[       	]+xor ixh
[  ]+b4:[ 	]+dd ad[       	]+xor ixl
[  ]+b6:[ 	]+fd ac[       	]+xor iyh
[  ]+b8:[ 	]+fd ad[       	]+xor iyl
[  ]+ba:[ 	]+ee a5[       	]+xor 0xa5
[  ]+bc:[ 	]+ae[          	]+xor \(hl\)
[  ]+bd:[ 	]+dd ae 05[    	]+xor \(ix\+5\)
[  ]+c0:[ 	]+fd ae fe[    	]+xor \(iy\-2\)
[  ]+c3:[ 	]+c3 03 00[    	]+jp 0x0003
[  ]+c6:[ 	]+c3 1b 00[    	]+jp 0x001b
[  ]+c9:[ 	]+c3 33 00[    	]+jp 0x0033
[  ]+cc:[ 	]+c3 4b 00[    	]+jp 0x004b
[  ]+cf:[ 	]+c3 7b 00[    	]+jp 0x007b
[  ]+d2:[ 	]+c3 93 00[    	]+jp 0x0093
[  ]+d5:[ 	]+c3 ab 00[    	]+jp 0x00ab
0+d8 <_func>:
[  ]+d8:[ 	]+21 00 00[    	]+ld hl,0x0000
[  ]+db:[ 	]+36 00[       	]+ld \(hl\),0x00
[  ]+dd:[ 	]+23[          	]+inc hl
[  ]+de:[ 	]+36 00[       	]+ld \(hl\),0x00
[  ]+e0:[ 	]+18 fb[       	]+jr 0x00dd

0+e2 <_finish>:
[  ]+e2:[ 	]+fd 7e 02[    	]+ld a,\(iy\+2\)
[  ]+e5:[ 	]+dd 77 ff[    	]+ld \(ix\-1\),a
[  ]+e8:[ 	]+3a 34 12[    	]+ld a,\(0x1234\)

[  ]+eb:[ 	]+c9[          	]+ret
[  ]+ec:[ 	]+27[          	]+daa
[  ]+ed:[ 	]+1f[          	]+rra
[  ]+ee:[ 	]+2f[          	]+cpl
