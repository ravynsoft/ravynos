#as: -march=z80n
#objdump: -d

.*:[     ]+file format (coff|elf32)\-z80


Disassembly of section \.text:

00000000 <\.text>:
[   ]+0:[ 	]+8e[          	]+adc a,\(hl\)
[   ]+1:[ 	]+dd 8e 09[    	]+adc a,\(ix\+9\)
[   ]+4:[ 	]+fd 8e 09[    	]+adc a,\(iy\+9\)
[   ]+7:[ 	]+ce 03[       	]+adc a,0x03
[   ]+9:[ 	]+8f[          	]+adc a,a
[   ]+a:[ 	]+88[          	]+adc a,b
[   ]+b:[ 	]+89[          	]+adc a,c
[   ]+c:[ 	]+8a[          	]+adc a,d
[   ]+d:[ 	]+8b[          	]+adc a,e
[   ]+e:[ 	]+8c[          	]+adc a,h
[   ]+f:[ 	]+8d[          	]+adc a,l
[  ]+10:[ 	]+ed 4a[       	]+adc hl,bc
[  ]+12:[ 	]+ed 5a[       	]+adc hl,de
[  ]+14:[ 	]+ed 6a[       	]+adc hl,hl
[  ]+16:[ 	]+ed 7a[       	]+adc hl,sp
[  ]+18:[ 	]+86[          	]+add a,\(hl\)
[  ]+19:[ 	]+dd 86 09[    	]+add a,\(ix\+9\)
[  ]+1c:[ 	]+fd 86 09[    	]+add a,\(iy\+9\)
[  ]+1f:[ 	]+c6 03[       	]+add a,0x03
[  ]+21:[ 	]+87[          	]+add a,a
[  ]+22:[ 	]+80[          	]+add a,b
[  ]+23:[ 	]+81[          	]+add a,c
[  ]+24:[ 	]+82[          	]+add a,d
[  ]+25:[ 	]+83[          	]+add a,e
[  ]+26:[ 	]+84[          	]+add a,h
[  ]+27:[ 	]+85[          	]+add a,l
[  ]+28:[ 	]+09[          	]+add hl,bc
[  ]+29:[ 	]+19[          	]+add hl,de
[  ]+2a:[ 	]+29[          	]+add hl,hl
[  ]+2b:[ 	]+39[          	]+add hl,sp
[  ]+2c:[ 	]+dd 09[       	]+add ix,bc
[  ]+2e:[ 	]+dd 19[       	]+add ix,de
[  ]+30:[ 	]+dd 29[       	]+add ix,ix
[  ]+32:[ 	]+dd 39[       	]+add ix,sp
[  ]+34:[ 	]+fd 09[       	]+add iy,bc
[  ]+36:[ 	]+fd 19[       	]+add iy,de
[  ]+38:[ 	]+fd 29[       	]+add iy,iy
[  ]+3a:[ 	]+fd 39[       	]+add iy,sp
[  ]+3c:[ 	]+a6[          	]+and \(hl\)
[  ]+3d:[ 	]+dd a6 09[    	]+and \(ix\+9\)
[  ]+40:[ 	]+fd a6 09[    	]+and \(iy\+9\)
[  ]+43:[ 	]+e6 03[       	]+and 0x03
[  ]+45:[ 	]+a7[          	]+and a
[  ]+46:[ 	]+a0[          	]+and b
[  ]+47:[ 	]+a1[          	]+and c
[  ]+48:[ 	]+a2[          	]+and d
[  ]+49:[ 	]+a3[          	]+and e
[  ]+4a:[ 	]+a4[          	]+and h
[  ]+4b:[ 	]+a5[          	]+and l
[  ]+4c:[ 	]+cb 46[       	]+bit 0,\(hl\)
[  ]+4e:[ 	]+dd cb 09 46[ 	]+bit 0,\(ix\+9\)
[  ]+52:[ 	]+fd cb 09 46[ 	]+bit 0,\(iy\+9\)
[  ]+56:[ 	]+cb 47[       	]+bit 0,a
[  ]+58:[ 	]+cb 40[       	]+bit 0,b
[  ]+5a:[ 	]+cb 41[       	]+bit 0,c
[  ]+5c:[ 	]+cb 42[       	]+bit 0,d
[  ]+5e:[ 	]+cb 43[       	]+bit 0,e
[  ]+60:[ 	]+cb 44[       	]+bit 0,h
[  ]+62:[ 	]+cb 45[       	]+bit 0,l
[  ]+64:[ 	]+cb 4e[       	]+bit 1,\(hl\)
[  ]+66:[ 	]+dd cb 09 4e[ 	]+bit 1,\(ix\+9\)
[  ]+6a:[ 	]+fd cb 09 4e[ 	]+bit 1,\(iy\+9\)
[  ]+6e:[ 	]+cb 4f[       	]+bit 1,a
[  ]+70:[ 	]+cb 48[       	]+bit 1,b
[  ]+72:[ 	]+cb 49[       	]+bit 1,c
[  ]+74:[ 	]+cb 4a[       	]+bit 1,d
[  ]+76:[ 	]+cb 4b[       	]+bit 1,e
[  ]+78:[ 	]+cb 4c[       	]+bit 1,h
[  ]+7a:[ 	]+cb 4d[       	]+bit 1,l
[  ]+7c:[ 	]+cb 56[       	]+bit 2,\(hl\)
[  ]+7e:[ 	]+dd cb 09 56[ 	]+bit 2,\(ix\+9\)
[  ]+82:[ 	]+fd cb 09 56[ 	]+bit 2,\(iy\+9\)
[  ]+86:[ 	]+cb 57[       	]+bit 2,a
[  ]+88:[ 	]+cb 50[       	]+bit 2,b
[  ]+8a:[ 	]+cb 51[       	]+bit 2,c
[  ]+8c:[ 	]+cb 52[       	]+bit 2,d
[  ]+8e:[ 	]+cb 53[       	]+bit 2,e
[  ]+90:[ 	]+cb 54[       	]+bit 2,h
[  ]+92:[ 	]+cb 55[       	]+bit 2,l
[  ]+94:[ 	]+cb 5e[       	]+bit 3,\(hl\)
[  ]+96:[ 	]+dd cb 09 5e[ 	]+bit 3,\(ix\+9\)
[  ]+9a:[ 	]+fd cb 09 5e[ 	]+bit 3,\(iy\+9\)
[  ]+9e:[ 	]+cb 5f[       	]+bit 3,a
[  ]+a0:[ 	]+cb 58[       	]+bit 3,b
[  ]+a2:[ 	]+cb 59[       	]+bit 3,c
[  ]+a4:[ 	]+cb 5a[       	]+bit 3,d
[  ]+a6:[ 	]+cb 5b[       	]+bit 3,e
[  ]+a8:[ 	]+cb 5c[       	]+bit 3,h
[  ]+aa:[ 	]+cb 5d[       	]+bit 3,l
[  ]+ac:[ 	]+cb 66[       	]+bit 4,\(hl\)
[  ]+ae:[ 	]+dd cb 09 66[ 	]+bit 4,\(ix\+9\)
[  ]+b2:[ 	]+fd cb 09 66[ 	]+bit 4,\(iy\+9\)
[  ]+b6:[ 	]+cb 67[       	]+bit 4,a
[  ]+b8:[ 	]+cb 60[       	]+bit 4,b
[  ]+ba:[ 	]+cb 61[       	]+bit 4,c
[  ]+bc:[ 	]+cb 62[       	]+bit 4,d
[  ]+be:[ 	]+cb 63[       	]+bit 4,e
[  ]+c0:[ 	]+cb 64[       	]+bit 4,h
[  ]+c2:[ 	]+cb 65[       	]+bit 4,l
[  ]+c4:[ 	]+cb 6e[       	]+bit 5,\(hl\)
[  ]+c6:[ 	]+dd cb 09 6e[ 	]+bit 5,\(ix\+9\)
[  ]+ca:[ 	]+fd cb 09 6e[ 	]+bit 5,\(iy\+9\)
[  ]+ce:[ 	]+cb 6f[       	]+bit 5,a
[  ]+d0:[ 	]+cb 68[       	]+bit 5,b
[  ]+d2:[ 	]+cb 69[       	]+bit 5,c
[  ]+d4:[ 	]+cb 6a[       	]+bit 5,d
[  ]+d6:[ 	]+cb 6b[       	]+bit 5,e
[  ]+d8:[ 	]+cb 6c[       	]+bit 5,h
[  ]+da:[ 	]+cb 6d[       	]+bit 5,l
[  ]+dc:[ 	]+cb 76[       	]+bit 6,\(hl\)
[  ]+de:[ 	]+dd cb 09 76[ 	]+bit 6,\(ix\+9\)
[  ]+e2:[ 	]+fd cb 09 76[ 	]+bit 6,\(iy\+9\)
[  ]+e6:[ 	]+cb 77[       	]+bit 6,a
[  ]+e8:[ 	]+cb 70[       	]+bit 6,b
[  ]+ea:[ 	]+cb 71[       	]+bit 6,c
[  ]+ec:[ 	]+cb 72[       	]+bit 6,d
[  ]+ee:[ 	]+cb 73[       	]+bit 6,e
[  ]+f0:[ 	]+cb 74[       	]+bit 6,h
[  ]+f2:[ 	]+cb 75[       	]+bit 6,l
[  ]+f4:[ 	]+cb 7e[       	]+bit 7,\(hl\)
[  ]+f6:[ 	]+dd cb 09 7e[ 	]+bit 7,\(ix\+9\)
[  ]+fa:[ 	]+fd cb 09 7e[ 	]+bit 7,\(iy\+9\)
[  ]+fe:[ 	]+cb 7f[       	]+bit 7,a
[ ]+100:[ 	]+cb 78[       	]+bit 7,b
[ ]+102:[ 	]+cb 79[       	]+bit 7,c
[ ]+104:[ 	]+cb 7a[       	]+bit 7,d
[ ]+106:[ 	]+cb 7b[       	]+bit 7,e
[ ]+108:[ 	]+cb 7c[       	]+bit 7,h
[ ]+10a:[ 	]+cb 7d[       	]+bit 7,l
[ ]+10c:[ 	]+cd 34 12[    	]+call 0x1234
[ ]+10f:[ 	]+dc 34 12[    	]+call c,0x1234
[ ]+112:[ 	]+fc 34 12[    	]+call m,0x1234
[ ]+115:[ 	]+d4 34 12[    	]+call nc,0x1234
[ ]+118:[ 	]+c4 34 12[    	]+call nz,0x1234
[ ]+11b:[ 	]+f4 34 12[    	]+call p,0x1234
[ ]+11e:[ 	]+ec 34 12[    	]+call pe,0x1234
[ ]+121:[ 	]+e4 34 12[    	]+call po,0x1234
[ ]+124:[ 	]+cc 34 12[    	]+call z,0x1234
[ ]+127:[ 	]+3f[          	]+ccf
[ ]+128:[ 	]+be[          	]+cp \(hl\)
[ ]+129:[ 	]+dd be 09[    	]+cp \(ix\+9\)
[ ]+12c:[ 	]+fd be 09[    	]+cp \(iy\+9\)
[ ]+12f:[ 	]+fe 03[       	]+cp 0x03
[ ]+131:[ 	]+bf[          	]+cp a
[ ]+132:[ 	]+b8[          	]+cp b
[ ]+133:[ 	]+b9[          	]+cp c
[ ]+134:[ 	]+ba[          	]+cp d
[ ]+135:[ 	]+bb[          	]+cp e
[ ]+136:[ 	]+bc[          	]+cp h
[ ]+137:[ 	]+bd[          	]+cp l
[ ]+138:[ 	]+ed a9[       	]+cpd
[ ]+13a:[ 	]+ed b9[       	]+cpdr
[ ]+13c:[ 	]+ed a1[       	]+cpi
[ ]+13e:[ 	]+ed b1[       	]+cpir
[ ]+140:[ 	]+2f[          	]+cpl
[ ]+141:[ 	]+27[          	]+daa
[ ]+142:[ 	]+35[          	]+dec \(hl\)
[ ]+143:[ 	]+dd 35 09[    	]+dec \(ix\+9\)
[ ]+146:[ 	]+fd 35 09[    	]+dec \(iy\+9\)
[ ]+149:[ 	]+3d[          	]+dec a
[ ]+14a:[ 	]+05[          	]+dec b
[ ]+14b:[ 	]+0b[          	]+dec bc
[ ]+14c:[ 	]+0d[          	]+dec c
[ ]+14d:[ 	]+15[          	]+dec d
[ ]+14e:[ 	]+1b[          	]+dec de
[ ]+14f:[ 	]+1d[          	]+dec e
[ ]+150:[ 	]+25[          	]+dec h
[ ]+151:[ 	]+2b[          	]+dec hl
[ ]+152:[ 	]+dd 2b[       	]+dec ix
[ ]+154:[ 	]+fd 2b[       	]+dec iy
[ ]+156:[ 	]+2d[          	]+dec l
[ ]+157:[ 	]+3b[          	]+dec sp
[ ]+158:[ 	]+f3[          	]+di
[ ]+159:[ 	]+10 05[       	]+djnz 0x0160
[ ]+15b:[ 	]+fb[          	]+ei
[ ]+15c:[ 	]+e3[          	]+ex \(sp\),hl
[ ]+15d:[ 	]+dd e3[       	]+ex \(sp\),ix
[ ]+15f:[ 	]+fd e3[       	]+ex \(sp\),iy
[ ]+161:[ 	]+08[          	]+ex af,af'
[ ]+162:[ 	]+eb[          	]+ex de,hl
[ ]+163:[ 	]+d9[          	]+exx
[ ]+164:[ 	]+76[          	]+halt
[ ]+165:[ 	]+ed 46[       	]+im 0
[ ]+167:[ 	]+ed 56[       	]+im 1
[ ]+169:[ 	]+ed 5e[       	]+im 2
[ ]+16b:[ 	]+ed 78[       	]+in a,\(c\)
[ ]+16d:[ 	]+db 03[       	]+in a,\(0x03\)
[ ]+16f:[ 	]+ed 40[       	]+in b,\(c\)
[ ]+171:[ 	]+ed 48[       	]+in c,\(c\)
[ ]+173:[ 	]+ed 50[       	]+in d,\(c\)
[ ]+175:[ 	]+ed 58[       	]+in e,\(c\)
[ ]+177:[ 	]+ed 60[       	]+in h,\(c\)
[ ]+179:[ 	]+ed 68[       	]+in l,\(c\)
[ ]+17b:[ 	]+34[          	]+inc \(hl\)
[ ]+17c:[ 	]+dd 34 09[    	]+inc \(ix\+9\)
[ ]+17f:[ 	]+fd 34 09[    	]+inc \(iy\+9\)
[ ]+182:[ 	]+3c[          	]+inc a
[ ]+183:[ 	]+04[          	]+inc b
[ ]+184:[ 	]+03[          	]+inc bc
[ ]+185:[ 	]+0c[          	]+inc c
[ ]+186:[ 	]+14[          	]+inc d
[ ]+187:[ 	]+13[          	]+inc de
[ ]+188:[ 	]+1c[          	]+inc e
[ ]+189:[ 	]+24[          	]+inc h
[ ]+18a:[ 	]+23[          	]+inc hl
[ ]+18b:[ 	]+dd 23[       	]+inc ix
[ ]+18d:[ 	]+fd 23[       	]+inc iy
[ ]+18f:[ 	]+2c[          	]+inc l
[ ]+190:[ 	]+33[          	]+inc sp
[ ]+191:[ 	]+ed aa[       	]+ind
[ ]+193:[ 	]+ed ba[       	]+indr
[ ]+195:[ 	]+ed a2[       	]+ini
[ ]+197:[ 	]+ed b2[       	]+inir
[ ]+199:[ 	]+e9[          	]+jp \(hl\)
[ ]+19a:[ 	]+dd e9[       	]+jp \(ix\)
[ ]+19c:[ 	]+fd e9[       	]+jp \(iy\)
[ ]+19e:[ 	]+c3 34 12[    	]+jp 0x1234
[ ]+1a1:[ 	]+da 34 12[    	]+jp c,0x1234
[ ]+1a4:[ 	]+fa 34 12[    	]+jp m,0x1234
[ ]+1a7:[ 	]+d2 34 12[    	]+jp nc,0x1234
[ ]+1aa:[ 	]+c2 34 12[    	]+jp nz,0x1234
[ ]+1ad:[ 	]+f2 34 12[    	]+jp p,0x1234
[ ]+1b0:[ 	]+ea 34 12[    	]+jp pe,0x1234
[ ]+1b3:[ 	]+e2 34 12[    	]+jp po,0x1234
[ ]+1b6:[ 	]+ca 34 12[    	]+jp z,0x1234
[ ]+1b9:[ 	]+18 05[       	]+jr 0x01c0
[ ]+1bb:[ 	]+38 05[       	]+jr c,0x01c2
[ ]+1bd:[ 	]+30 05[       	]+jr nc,0x01c4
[ ]+1bf:[ 	]+20 05[       	]+jr nz,0x01c6
[ ]+1c1:[ 	]+28 05[       	]+jr z,0x01c8
[ ]+1c3:[ 	]+32 34 12[    	]+ld \(0x1234\),a
[ ]+1c6:[ 	]+ed 43 34 12[ 	]+ld \(0x1234\),bc
[ ]+1ca:[ 	]+ed 53 34 12[ 	]+ld \(0x1234\),de
[ ]+1ce:[ 	]+22 34 12[    	]+ld \(0x1234\),hl
[ ]+1d1:[ 	]+dd 22 34 12[ 	]+ld \(0x1234\),ix
[ ]+1d5:[ 	]+fd 22 34 12[ 	]+ld \(0x1234\),iy
[ ]+1d9:[ 	]+ed 73 34 12[ 	]+ld \(0x1234\),sp
[ ]+1dd:[ 	]+02[          	]+ld \(bc\),a
[ ]+1de:[ 	]+12[          	]+ld \(de\),a
[ ]+1df:[ 	]+36 03[       	]+ld \(hl\),0x03
[ ]+1e1:[ 	]+77[          	]+ld \(hl\),a
[ ]+1e2:[ 	]+70[          	]+ld \(hl\),b
[ ]+1e3:[ 	]+71[          	]+ld \(hl\),c
[ ]+1e4:[ 	]+72[          	]+ld \(hl\),d
[ ]+1e5:[ 	]+73[          	]+ld \(hl\),e
[ ]+1e6:[ 	]+74[          	]+ld \(hl\),h
[ ]+1e7:[ 	]+75[          	]+ld \(hl\),l
[ ]+1e8:[ 	]+dd 36 09 03[ 	]+ld \(ix\+9\),0x03
[ ]+1ec:[ 	]+dd 77 09[    	]+ld \(ix\+9\),a
[ ]+1ef:[ 	]+dd 70 09[    	]+ld \(ix\+9\),b
[ ]+1f2:[ 	]+dd 71 09[    	]+ld \(ix\+9\),c
[ ]+1f5:[ 	]+dd 72 09[    	]+ld \(ix\+9\),d
[ ]+1f8:[ 	]+dd 73 09[    	]+ld \(ix\+9\),e
[ ]+1fb:[ 	]+dd 74 09[    	]+ld \(ix\+9\),h
[ ]+1fe:[ 	]+dd 75 09[    	]+ld \(ix\+9\),l
[ ]+201:[ 	]+fd 36 09 03[ 	]+ld \(iy\+9\),0x03
[ ]+205:[ 	]+fd 77 09[    	]+ld \(iy\+9\),a
[ ]+208:[ 	]+fd 70 09[    	]+ld \(iy\+9\),b
[ ]+20b:[ 	]+fd 71 09[    	]+ld \(iy\+9\),c
[ ]+20e:[ 	]+fd 72 09[    	]+ld \(iy\+9\),d
[ ]+211:[ 	]+fd 73 09[    	]+ld \(iy\+9\),e
[ ]+214:[ 	]+fd 74 09[    	]+ld \(iy\+9\),h
[ ]+217:[ 	]+fd 75 09[    	]+ld \(iy\+9\),l
[ ]+21a:[ 	]+3a 34 12[    	]+ld a,\(0x1234\)
[ ]+21d:[ 	]+0a[          	]+ld a,\(bc\)
[ ]+21e:[ 	]+1a[          	]+ld a,\(de\)
[ ]+21f:[ 	]+7e[          	]+ld a,\(hl\)
[ ]+220:[ 	]+dd 7e 09[    	]+ld a,\(ix\+9\)
[ ]+223:[ 	]+fd 7e 09[    	]+ld a,\(iy\+9\)
[ ]+226:[ 	]+3e 03[       	]+ld a,0x03
[ ]+228:[ 	]+7f[          	]+ld a,a
[ ]+229:[ 	]+78[          	]+ld a,b
[ ]+22a:[ 	]+79[          	]+ld a,c
[ ]+22b:[ 	]+7a[          	]+ld a,d
[ ]+22c:[ 	]+7b[          	]+ld a,e
[ ]+22d:[ 	]+7c[          	]+ld a,h
[ ]+22e:[ 	]+ed 57[       	]+ld a,i
[ ]+230:[ 	]+7d[          	]+ld a,l
[ ]+231:[ 	]+ed 5f[       	]+ld a,r
[ ]+233:[ 	]+46[          	]+ld b,\(hl\)
[ ]+234:[ 	]+dd 46 09[    	]+ld b,\(ix\+9\)
[ ]+237:[ 	]+fd 46 09[    	]+ld b,\(iy\+9\)
[ ]+23a:[ 	]+06 03[       	]+ld b,0x03
[ ]+23c:[ 	]+47[          	]+ld b,a
[ ]+23d:[ 	]+40[          	]+ld b,b
[ ]+23e:[ 	]+41[          	]+ld b,c
[ ]+23f:[ 	]+42[          	]+ld b,d
[ ]+240:[ 	]+43[          	]+ld b,e
[ ]+241:[ 	]+44[          	]+ld b,h
[ ]+242:[ 	]+45[          	]+ld b,l
[ ]+243:[ 	]+ed 4b 34 12[ 	]+ld bc,\(0x1234\)
[ ]+247:[ 	]+01 34 12[    	]+ld bc,0x1234
[ ]+24a:[ 	]+4e[          	]+ld c,\(hl\)
[ ]+24b:[ 	]+dd 4e 09[    	]+ld c,\(ix\+9\)
[ ]+24e:[ 	]+fd 4e 09[    	]+ld c,\(iy\+9\)
[ ]+251:[ 	]+0e 03[       	]+ld c,0x03
[ ]+253:[ 	]+4f[          	]+ld c,a
[ ]+254:[ 	]+48[          	]+ld c,b
[ ]+255:[ 	]+49[          	]+ld c,c
[ ]+256:[ 	]+4a[          	]+ld c,d
[ ]+257:[ 	]+4b[          	]+ld c,e
[ ]+258:[ 	]+4c[          	]+ld c,h
[ ]+259:[ 	]+4d[          	]+ld c,l
[ ]+25a:[ 	]+56[          	]+ld d,\(hl\)
[ ]+25b:[ 	]+dd 56 09[    	]+ld d,\(ix\+9\)
[ ]+25e:[ 	]+fd 56 09[    	]+ld d,\(iy\+9\)
[ ]+261:[ 	]+16 03[       	]+ld d,0x03
[ ]+263:[ 	]+57[          	]+ld d,a
[ ]+264:[ 	]+50[          	]+ld d,b
[ ]+265:[ 	]+51[          	]+ld d,c
[ ]+266:[ 	]+52[          	]+ld d,d
[ ]+267:[ 	]+53[          	]+ld d,e
[ ]+268:[ 	]+54[          	]+ld d,h
[ ]+269:[ 	]+55[          	]+ld d,l
[ ]+26a:[ 	]+ed 5b 34 12[ 	]+ld de,\(0x1234\)
[ ]+26e:[ 	]+11 34 12[    	]+ld de,0x1234
[ ]+271:[ 	]+5e[          	]+ld e,\(hl\)
[ ]+272:[ 	]+dd 5e 09[    	]+ld e,\(ix\+9\)
[ ]+275:[ 	]+fd 5e 09[    	]+ld e,\(iy\+9\)
[ ]+278:[ 	]+1e 03[       	]+ld e,0x03
[ ]+27a:[ 	]+5f[          	]+ld e,a
[ ]+27b:[ 	]+58[          	]+ld e,b
[ ]+27c:[ 	]+59[          	]+ld e,c
[ ]+27d:[ 	]+5a[          	]+ld e,d
[ ]+27e:[ 	]+5b[          	]+ld e,e
[ ]+27f:[ 	]+5c[          	]+ld e,h
[ ]+280:[ 	]+5d[          	]+ld e,l
[ ]+281:[ 	]+66[          	]+ld h,\(hl\)
[ ]+282:[ 	]+dd 66 09[    	]+ld h,\(ix\+9\)
[ ]+285:[ 	]+fd 66 09[    	]+ld h,\(iy\+9\)
[ ]+288:[ 	]+26 03[       	]+ld h,0x03
[ ]+28a:[ 	]+67[          	]+ld h,a
[ ]+28b:[ 	]+60[          	]+ld h,b
[ ]+28c:[ 	]+61[          	]+ld h,c
[ ]+28d:[ 	]+62[          	]+ld h,d
[ ]+28e:[ 	]+63[          	]+ld h,e
[ ]+28f:[ 	]+64[          	]+ld h,h
[ ]+290:[ 	]+65[          	]+ld h,l
[ ]+291:[ 	]+2a 34 12[    	]+ld hl,\(0x1234\)
[ ]+294:[ 	]+21 34 12[    	]+ld hl,0x1234
[ ]+297:[ 	]+ed 47[       	]+ld i,a
[ ]+299:[ 	]+dd 2a 34 12[ 	]+ld ix,\(0x1234\)
[ ]+29d:[ 	]+dd 21 34 12[ 	]+ld ix,0x1234
[ ]+2a1:[ 	]+fd 2a 34 12[ 	]+ld iy,\(0x1234\)
[ ]+2a5:[ 	]+fd 21 34 12[ 	]+ld iy,0x1234
[ ]+2a9:[ 	]+6e[          	]+ld l,\(hl\)
[ ]+2aa:[ 	]+dd 6e 09[    	]+ld l,\(ix\+9\)
[ ]+2ad:[ 	]+fd 6e 09[    	]+ld l,\(iy\+9\)
[ ]+2b0:[ 	]+2e 03[       	]+ld l,0x03
[ ]+2b2:[ 	]+6f[          	]+ld l,a
[ ]+2b3:[ 	]+68[          	]+ld l,b
[ ]+2b4:[ 	]+69[          	]+ld l,c
[ ]+2b5:[ 	]+6a[          	]+ld l,d
[ ]+2b6:[ 	]+6b[          	]+ld l,e
[ ]+2b7:[ 	]+6c[          	]+ld l,h
[ ]+2b8:[ 	]+6d[          	]+ld l,l
[ ]+2b9:[ 	]+ed 4f[       	]+ld r,a
[ ]+2bb:[ 	]+ed 7b 34 12[ 	]+ld sp,\(0x1234\)
[ ]+2bf:[ 	]+31 34 12[    	]+ld sp,0x1234
[ ]+2c2:[ 	]+f9[          	]+ld sp,hl
[ ]+2c3:[ 	]+dd f9[       	]+ld sp,ix
[ ]+2c5:[ 	]+fd f9[       	]+ld sp,iy
[ ]+2c7:[ 	]+ed a8[       	]+ldd
[ ]+2c9:[ 	]+ed b8[       	]+lddr
[ ]+2cb:[ 	]+ed a0[       	]+ldi
[ ]+2cd:[ 	]+ed b0[       	]+ldir
[ ]+2cf:[ 	]+ed 44[       	]+neg
[ ]+2d1:[ 	]+00[          	]+nop
[ ]+2d2:[ 	]+b6[          	]+or \(hl\)
[ ]+2d3:[ 	]+dd b6 09[    	]+or \(ix\+9\)
[ ]+2d6:[ 	]+fd b6 09[    	]+or \(iy\+9\)
[ ]+2d9:[ 	]+f6 03[       	]+or 0x03
[ ]+2db:[ 	]+b7[          	]+or a
[ ]+2dc:[ 	]+b0[          	]+or b
[ ]+2dd:[ 	]+b1[          	]+or c
[ ]+2de:[ 	]+b2[          	]+or d
[ ]+2df:[ 	]+b3[          	]+or e
[ ]+2e0:[ 	]+b4[          	]+or h
[ ]+2e1:[ 	]+b5[          	]+or l
[ ]+2e2:[ 	]+ed bb[       	]+otdr
[ ]+2e4:[ 	]+ed b3[       	]+otir
[ ]+2e6:[ 	]+ed 79[       	]+out \(c\),a
[ ]+2e8:[ 	]+ed 41[       	]+out \(c\),b
[ ]+2ea:[ 	]+ed 49[       	]+out \(c\),c
[ ]+2ec:[ 	]+ed 51[       	]+out \(c\),d
[ ]+2ee:[ 	]+ed 59[       	]+out \(c\),e
[ ]+2f0:[ 	]+ed 61[       	]+out \(c\),h
[ ]+2f2:[ 	]+ed 69[       	]+out \(c\),l
[ ]+2f4:[ 	]+d3 03[       	]+out \(0x03\),a
[ ]+2f6:[ 	]+ed ab[       	]+outd
[ ]+2f8:[ 	]+ed a3[       	]+outi
[ ]+2fa:[ 	]+f1[          	]+pop af
[ ]+2fb:[ 	]+c1[          	]+pop bc
[ ]+2fc:[ 	]+d1[          	]+pop de
[ ]+2fd:[ 	]+e1[          	]+pop hl
[ ]+2fe:[ 	]+dd e1[       	]+pop ix
[ ]+300:[ 	]+fd e1[       	]+pop iy
[ ]+302:[ 	]+f5[          	]+push af
[ ]+303:[ 	]+c5[          	]+push bc
[ ]+304:[ 	]+d5[          	]+push de
[ ]+305:[ 	]+e5[          	]+push hl
[ ]+306:[ 	]+dd e5[       	]+push ix
[ ]+308:[ 	]+fd e5[       	]+push iy
[ ]+30a:[ 	]+cb 86[       	]+res 0,\(hl\)
[ ]+30c:[ 	]+dd cb 09 86[ 	]+res 0,\(ix\+9\)
[ ]+310:[ 	]+fd cb 09 86[ 	]+res 0,\(iy\+9\)
[ ]+314:[ 	]+cb 87[       	]+res 0,a
[ ]+316:[ 	]+cb 80[       	]+res 0,b
[ ]+318:[ 	]+cb 81[       	]+res 0,c
[ ]+31a:[ 	]+cb 82[       	]+res 0,d
[ ]+31c:[ 	]+cb 83[       	]+res 0,e
[ ]+31e:[ 	]+cb 84[       	]+res 0,h
[ ]+320:[ 	]+cb 85[       	]+res 0,l
[ ]+322:[ 	]+cb 8e[       	]+res 1,\(hl\)
[ ]+324:[ 	]+dd cb 09 8e[ 	]+res 1,\(ix\+9\)
[ ]+328:[ 	]+fd cb 09 8e[ 	]+res 1,\(iy\+9\)
[ ]+32c:[ 	]+cb 8f[       	]+res 1,a
[ ]+32e:[ 	]+cb 88[       	]+res 1,b
[ ]+330:[ 	]+cb 89[       	]+res 1,c
[ ]+332:[ 	]+cb 8a[       	]+res 1,d
[ ]+334:[ 	]+cb 8b[       	]+res 1,e
[ ]+336:[ 	]+cb 8c[       	]+res 1,h
[ ]+338:[ 	]+cb 8d[       	]+res 1,l
[ ]+33a:[ 	]+cb 96[       	]+res 2,\(hl\)
[ ]+33c:[ 	]+dd cb 09 96[ 	]+res 2,\(ix\+9\)
[ ]+340:[ 	]+fd cb 09 96[ 	]+res 2,\(iy\+9\)
[ ]+344:[ 	]+cb 97[       	]+res 2,a
[ ]+346:[ 	]+cb 90[       	]+res 2,b
[ ]+348:[ 	]+cb 91[       	]+res 2,c
[ ]+34a:[ 	]+cb 92[       	]+res 2,d
[ ]+34c:[ 	]+cb 93[       	]+res 2,e
[ ]+34e:[ 	]+cb 94[       	]+res 2,h
[ ]+350:[ 	]+cb 95[       	]+res 2,l
[ ]+352:[ 	]+cb 9e[       	]+res 3,\(hl\)
[ ]+354:[ 	]+dd cb 09 9e[ 	]+res 3,\(ix\+9\)
[ ]+358:[ 	]+fd cb 09 9e[ 	]+res 3,\(iy\+9\)
[ ]+35c:[ 	]+cb 9f[       	]+res 3,a
[ ]+35e:[ 	]+cb 98[       	]+res 3,b
[ ]+360:[ 	]+cb 99[       	]+res 3,c
[ ]+362:[ 	]+cb 9a[       	]+res 3,d
[ ]+364:[ 	]+cb 9b[       	]+res 3,e
[ ]+366:[ 	]+cb 9c[       	]+res 3,h
[ ]+368:[ 	]+cb 9d[       	]+res 3,l
[ ]+36a:[ 	]+cb a6[       	]+res 4,\(hl\)
[ ]+36c:[ 	]+dd cb 09 a6[ 	]+res 4,\(ix\+9\)
[ ]+370:[ 	]+fd cb 09 a6[ 	]+res 4,\(iy\+9\)
[ ]+374:[ 	]+cb a7[       	]+res 4,a
[ ]+376:[ 	]+cb a0[       	]+res 4,b
[ ]+378:[ 	]+cb a1[       	]+res 4,c
[ ]+37a:[ 	]+cb a2[       	]+res 4,d
[ ]+37c:[ 	]+cb a3[       	]+res 4,e
[ ]+37e:[ 	]+cb a4[       	]+res 4,h
[ ]+380:[ 	]+cb a5[       	]+res 4,l
[ ]+382:[ 	]+cb ae[       	]+res 5,\(hl\)
[ ]+384:[ 	]+dd cb 09 ae[ 	]+res 5,\(ix\+9\)
[ ]+388:[ 	]+fd cb 09 ae[ 	]+res 5,\(iy\+9\)
[ ]+38c:[ 	]+cb af[       	]+res 5,a
[ ]+38e:[ 	]+cb a8[       	]+res 5,b
[ ]+390:[ 	]+cb a9[       	]+res 5,c
[ ]+392:[ 	]+cb aa[       	]+res 5,d
[ ]+394:[ 	]+cb ab[       	]+res 5,e
[ ]+396:[ 	]+cb ac[       	]+res 5,h
[ ]+398:[ 	]+cb ad[       	]+res 5,l
[ ]+39a:[ 	]+cb b6[       	]+res 6,\(hl\)
[ ]+39c:[ 	]+dd cb 09 b6[ 	]+res 6,\(ix\+9\)
[ ]+3a0:[ 	]+fd cb 09 b6[ 	]+res 6,\(iy\+9\)
[ ]+3a4:[ 	]+cb b7[       	]+res 6,a
[ ]+3a6:[ 	]+cb b0[       	]+res 6,b
[ ]+3a8:[ 	]+cb b1[       	]+res 6,c
[ ]+3aa:[ 	]+cb b2[       	]+res 6,d
[ ]+3ac:[ 	]+cb b3[       	]+res 6,e
[ ]+3ae:[ 	]+cb b4[       	]+res 6,h
[ ]+3b0:[ 	]+cb b5[       	]+res 6,l
[ ]+3b2:[ 	]+cb be[       	]+res 7,\(hl\)
[ ]+3b4:[ 	]+dd cb 09 be[ 	]+res 7,\(ix\+9\)
[ ]+3b8:[ 	]+fd cb 09 be[ 	]+res 7,\(iy\+9\)
[ ]+3bc:[ 	]+cb bf[       	]+res 7,a
[ ]+3be:[ 	]+cb b8[       	]+res 7,b
[ ]+3c0:[ 	]+cb b9[       	]+res 7,c
[ ]+3c2:[ 	]+cb ba[       	]+res 7,d
[ ]+3c4:[ 	]+cb bb[       	]+res 7,e
[ ]+3c6:[ 	]+cb bc[       	]+res 7,h
[ ]+3c8:[ 	]+cb bd[       	]+res 7,l
[ ]+3ca:[ 	]+c9[          	]+ret
[ ]+3cb:[ 	]+d8[          	]+ret c
[ ]+3cc:[ 	]+f8[          	]+ret m
[ ]+3cd:[ 	]+d0[          	]+ret nc
[ ]+3ce:[ 	]+c0[          	]+ret nz
[ ]+3cf:[ 	]+f0[          	]+ret p
[ ]+3d0:[ 	]+e8[          	]+ret pe
[ ]+3d1:[ 	]+e0[          	]+ret po
[ ]+3d2:[ 	]+c8[          	]+ret z
[ ]+3d3:[ 	]+ed 4d[       	]+reti
[ ]+3d5:[ 	]+ed 45[       	]+retn
[ ]+3d7:[ 	]+cb 16[       	]+rl \(hl\)
[ ]+3d9:[ 	]+dd cb 09 16[ 	]+rl \(ix\+9\)
[ ]+3dd:[ 	]+fd cb 09 16[ 	]+rl \(iy\+9\)
[ ]+3e1:[ 	]+cb 17[       	]+rl a
[ ]+3e3:[ 	]+cb 10[       	]+rl b
[ ]+3e5:[ 	]+cb 11[       	]+rl c
[ ]+3e7:[ 	]+cb 12[       	]+rl d
[ ]+3e9:[ 	]+cb 13[       	]+rl e
[ ]+3eb:[ 	]+cb 14[       	]+rl h
[ ]+3ed:[ 	]+cb 15[       	]+rl l
[ ]+3ef:[ 	]+17[          	]+rla
[ ]+3f0:[ 	]+cb 06[       	]+rlc \(hl\)
[ ]+3f2:[ 	]+dd cb 09 06[ 	]+rlc \(ix\+9\)
[ ]+3f6:[ 	]+fd cb 09 06[ 	]+rlc \(iy\+9\)
[ ]+3fa:[ 	]+cb 07[       	]+rlc a
[ ]+3fc:[ 	]+cb 00[       	]+rlc b
[ ]+3fe:[ 	]+cb 01[       	]+rlc c
[ ]+400:[ 	]+cb 02[       	]+rlc d
[ ]+402:[ 	]+cb 03[       	]+rlc e
[ ]+404:[ 	]+cb 04[       	]+rlc h
[ ]+406:[ 	]+cb 05[       	]+rlc l
[ ]+408:[ 	]+07[          	]+rlca
[ ]+409:[ 	]+ed 6f[       	]+rld
[ ]+40b:[ 	]+cb 1e[       	]+rr \(hl\)
[ ]+40d:[ 	]+dd cb 09 1e[ 	]+rr \(ix\+9\)
[ ]+411:[ 	]+fd cb 09 1e[ 	]+rr \(iy\+9\)
[ ]+415:[ 	]+cb 1f[       	]+rr a
[ ]+417:[ 	]+cb 18[       	]+rr b
[ ]+419:[ 	]+cb 19[       	]+rr c
[ ]+41b:[ 	]+cb 1a[       	]+rr d
[ ]+41d:[ 	]+cb 1b[       	]+rr e
[ ]+41f:[ 	]+cb 1c[       	]+rr h
[ ]+421:[ 	]+cb 1d[       	]+rr l
[ ]+423:[ 	]+1f[          	]+rra
[ ]+424:[ 	]+cb 0e[       	]+rrc \(hl\)
[ ]+426:[ 	]+dd cb 09 0e[ 	]+rrc \(ix\+9\)
[ ]+42a:[ 	]+fd cb 09 0e[ 	]+rrc \(iy\+9\)
[ ]+42e:[ 	]+cb 0f[       	]+rrc a
[ ]+430:[ 	]+cb 08[       	]+rrc b
[ ]+432:[ 	]+cb 09[       	]+rrc c
[ ]+434:[ 	]+cb 0a[       	]+rrc d
[ ]+436:[ 	]+cb 0b[       	]+rrc e
[ ]+438:[ 	]+cb 0c[       	]+rrc h
[ ]+43a:[ 	]+cb 0d[       	]+rrc l
[ ]+43c:[ 	]+0f[          	]+rrca
[ ]+43d:[ 	]+ed 67[       	]+rrd
[ ]+43f:[ 	]+c7[          	]+rst 0x00
[ ]+440:[ 	]+cf[          	]+rst 0x08
[ ]+441:[ 	]+d7[          	]+rst 0x10
[ ]+442:[ 	]+df[          	]+rst 0x18
[ ]+443:[ 	]+e7[          	]+rst 0x20
[ ]+444:[ 	]+ef[          	]+rst 0x28
[ ]+445:[ 	]+f7[          	]+rst 0x30
[ ]+446:[ 	]+ff[          	]+rst 0x38
[ ]+447:[ 	]+9e[          	]+sbc a,\(hl\)
[ ]+448:[ 	]+dd 9e 09[    	]+sbc a,\(ix\+9\)
[ ]+44b:[ 	]+fd 9e 09[    	]+sbc a,\(iy\+9\)
[ ]+44e:[ 	]+de 03[       	]+sbc a,0x03
[ ]+450:[ 	]+9f[          	]+sbc a,a
[ ]+451:[ 	]+98[          	]+sbc a,b
[ ]+452:[ 	]+99[          	]+sbc a,c
[ ]+453:[ 	]+9a[          	]+sbc a,d
[ ]+454:[ 	]+9b[          	]+sbc a,e
[ ]+455:[ 	]+9c[          	]+sbc a,h
[ ]+456:[ 	]+9d[          	]+sbc a,l
[ ]+457:[ 	]+ed 42[       	]+sbc hl,bc
[ ]+459:[ 	]+ed 52[       	]+sbc hl,de
[ ]+45b:[ 	]+ed 62[       	]+sbc hl,hl
[ ]+45d:[ 	]+ed 72[       	]+sbc hl,sp
[ ]+45f:[ 	]+37[          	]+scf
[ ]+460:[ 	]+cb c6[       	]+set 0,\(hl\)
[ ]+462:[ 	]+dd cb 09 c6[ 	]+set 0,\(ix\+9\)
[ ]+466:[ 	]+fd cb 09 c6[ 	]+set 0,\(iy\+9\)
[ ]+46a:[ 	]+cb c7[       	]+set 0,a
[ ]+46c:[ 	]+cb c0[       	]+set 0,b
[ ]+46e:[ 	]+cb c1[       	]+set 0,c
[ ]+470:[ 	]+cb c2[       	]+set 0,d
[ ]+472:[ 	]+cb c3[       	]+set 0,e
[ ]+474:[ 	]+cb c4[       	]+set 0,h
[ ]+476:[ 	]+cb c5[       	]+set 0,l
[ ]+478:[ 	]+cb ce[       	]+set 1,\(hl\)
[ ]+47a:[ 	]+dd cb 09 ce[ 	]+set 1,\(ix\+9\)
[ ]+47e:[ 	]+fd cb 09 ce[ 	]+set 1,\(iy\+9\)
[ ]+482:[ 	]+cb cf[       	]+set 1,a
[ ]+484:[ 	]+cb c8[       	]+set 1,b
[ ]+486:[ 	]+cb c9[       	]+set 1,c
[ ]+488:[ 	]+cb ca[       	]+set 1,d
[ ]+48a:[ 	]+cb cb[       	]+set 1,e
[ ]+48c:[ 	]+cb cc[       	]+set 1,h
[ ]+48e:[ 	]+cb cd[       	]+set 1,l
[ ]+490:[ 	]+cb d6[       	]+set 2,\(hl\)
[ ]+492:[ 	]+dd cb 09 d6[ 	]+set 2,\(ix\+9\)
[ ]+496:[ 	]+fd cb 09 d6[ 	]+set 2,\(iy\+9\)
[ ]+49a:[ 	]+cb d7[       	]+set 2,a
[ ]+49c:[ 	]+cb d0[       	]+set 2,b
[ ]+49e:[ 	]+cb d1[       	]+set 2,c
[ ]+4a0:[ 	]+cb d2[       	]+set 2,d
[ ]+4a2:[ 	]+cb d3[       	]+set 2,e
[ ]+4a4:[ 	]+cb d4[       	]+set 2,h
[ ]+4a6:[ 	]+cb d5[       	]+set 2,l
[ ]+4a8:[ 	]+cb de[       	]+set 3,\(hl\)
[ ]+4aa:[ 	]+dd cb 09 de[ 	]+set 3,\(ix\+9\)
[ ]+4ae:[ 	]+fd cb 09 de[ 	]+set 3,\(iy\+9\)
[ ]+4b2:[ 	]+cb df[       	]+set 3,a
[ ]+4b4:[ 	]+cb d8[       	]+set 3,b
[ ]+4b6:[ 	]+cb d9[       	]+set 3,c
[ ]+4b8:[ 	]+cb da[       	]+set 3,d
[ ]+4ba:[ 	]+cb db[       	]+set 3,e
[ ]+4bc:[ 	]+cb dc[       	]+set 3,h
[ ]+4be:[ 	]+cb dd[       	]+set 3,l
[ ]+4c0:[ 	]+cb e6[       	]+set 4,\(hl\)
[ ]+4c2:[ 	]+dd cb 09 e6[ 	]+set 4,\(ix\+9\)
[ ]+4c6:[ 	]+fd cb 09 e6[ 	]+set 4,\(iy\+9\)
[ ]+4ca:[ 	]+cb e7[       	]+set 4,a
[ ]+4cc:[ 	]+cb e0[       	]+set 4,b
[ ]+4ce:[ 	]+cb e1[       	]+set 4,c
[ ]+4d0:[ 	]+cb e2[       	]+set 4,d
[ ]+4d2:[ 	]+cb e3[       	]+set 4,e
[ ]+4d4:[ 	]+cb e4[       	]+set 4,h
[ ]+4d6:[ 	]+cb e5[       	]+set 4,l
[ ]+4d8:[ 	]+cb ee[       	]+set 5,\(hl\)
[ ]+4da:[ 	]+dd cb 09 ee[ 	]+set 5,\(ix\+9\)
[ ]+4de:[ 	]+fd cb 09 ee[ 	]+set 5,\(iy\+9\)
[ ]+4e2:[ 	]+cb ef[       	]+set 5,a
[ ]+4e4:[ 	]+cb e8[       	]+set 5,b
[ ]+4e6:[ 	]+cb e9[       	]+set 5,c
[ ]+4e8:[ 	]+cb ea[       	]+set 5,d
[ ]+4ea:[ 	]+cb eb[       	]+set 5,e
[ ]+4ec:[ 	]+cb ec[       	]+set 5,h
[ ]+4ee:[ 	]+cb ed[       	]+set 5,l
[ ]+4f0:[ 	]+cb f6[       	]+set 6,\(hl\)
[ ]+4f2:[ 	]+dd cb 09 f6[ 	]+set 6,\(ix\+9\)
[ ]+4f6:[ 	]+fd cb 09 f6[ 	]+set 6,\(iy\+9\)
[ ]+4fa:[ 	]+cb f7[       	]+set 6,a
[ ]+4fc:[ 	]+cb f0[       	]+set 6,b
[ ]+4fe:[ 	]+cb f1[       	]+set 6,c
[ ]+500:[ 	]+cb f2[       	]+set 6,d
[ ]+502:[ 	]+cb f3[       	]+set 6,e
[ ]+504:[ 	]+cb f4[       	]+set 6,h
[ ]+506:[ 	]+cb f5[       	]+set 6,l
[ ]+508:[ 	]+cb fe[       	]+set 7,\(hl\)
[ ]+50a:[ 	]+dd cb 09 fe[ 	]+set 7,\(ix\+9\)
[ ]+50e:[ 	]+fd cb 09 fe[ 	]+set 7,\(iy\+9\)
[ ]+512:[ 	]+cb ff[       	]+set 7,a
[ ]+514:[ 	]+cb f8[       	]+set 7,b
[ ]+516:[ 	]+cb f9[       	]+set 7,c
[ ]+518:[ 	]+cb fa[       	]+set 7,d
[ ]+51a:[ 	]+cb fb[       	]+set 7,e
[ ]+51c:[ 	]+cb fc[       	]+set 7,h
[ ]+51e:[ 	]+cb fd[       	]+set 7,l
[ ]+520:[ 	]+cb 26[       	]+sla \(hl\)
[ ]+522:[ 	]+dd cb 09 26[ 	]+sla \(ix\+9\)
[ ]+526:[ 	]+fd cb 09 26[ 	]+sla \(iy\+9\)
[ ]+52a:[ 	]+cb 27[       	]+sla a
[ ]+52c:[ 	]+cb 20[       	]+sla b
[ ]+52e:[ 	]+cb 21[       	]+sla c
[ ]+530:[ 	]+cb 22[       	]+sla d
[ ]+532:[ 	]+cb 23[       	]+sla e
[ ]+534:[ 	]+cb 24[       	]+sla h
[ ]+536:[ 	]+cb 25[       	]+sla l
[ ]+538:[ 	]+cb 2e[       	]+sra \(hl\)
[ ]+53a:[ 	]+dd cb 09 2e[ 	]+sra \(ix\+9\)
[ ]+53e:[ 	]+fd cb 09 2e[ 	]+sra \(iy\+9\)
[ ]+542:[ 	]+cb 2f[       	]+sra a
[ ]+544:[ 	]+cb 28[       	]+sra b
[ ]+546:[ 	]+cb 29[       	]+sra c
[ ]+548:[ 	]+cb 2a[       	]+sra d
[ ]+54a:[ 	]+cb 2b[       	]+sra e
[ ]+54c:[ 	]+cb 2c[       	]+sra h
[ ]+54e:[ 	]+cb 2d[       	]+sra l
[ ]+550:[ 	]+cb 3e[       	]+srl \(hl\)
[ ]+552:[ 	]+dd cb 09 3e[ 	]+srl \(ix\+9\)
[ ]+556:[ 	]+fd cb 09 3e[ 	]+srl \(iy\+9\)
[ ]+55a:[ 	]+cb 3f[       	]+srl a
[ ]+55c:[ 	]+cb 38[       	]+srl b
[ ]+55e:[ 	]+cb 39[       	]+srl c
[ ]+560:[ 	]+cb 3a[       	]+srl d
[ ]+562:[ 	]+cb 3b[       	]+srl e
[ ]+564:[ 	]+cb 3c[       	]+srl h
[ ]+566:[ 	]+cb 3d[       	]+srl l
[ ]+568:[ 	]+96[          	]+sub \(hl\)
[ ]+569:[ 	]+dd 96 09[    	]+sub \(ix\+9\)
[ ]+56c:[ 	]+fd 96 09[    	]+sub \(iy\+9\)
[ ]+56f:[ 	]+d6 03[       	]+sub 0x03
[ ]+571:[ 	]+97[          	]+sub a
[ ]+572:[ 	]+90[          	]+sub b
[ ]+573:[ 	]+91[          	]+sub c
[ ]+574:[ 	]+92[          	]+sub d
[ ]+575:[ 	]+93[          	]+sub e
[ ]+576:[ 	]+94[          	]+sub h
[ ]+577:[ 	]+95[          	]+sub l
[ ]+578:[ 	]+ae[          	]+xor \(hl\)
[ ]+579:[ 	]+dd ae 09[    	]+xor \(ix\+9\)
[ ]+57c:[ 	]+fd ae 09[    	]+xor \(iy\+9\)
[ ]+57f:[ 	]+ee 03[       	]+xor 0x03
[ ]+581:[ 	]+af[          	]+xor a
[ ]+582:[ 	]+a8[          	]+xor b
[ ]+583:[ 	]+a9[          	]+xor c
[ ]+584:[ 	]+aa[          	]+xor d
[ ]+585:[ 	]+ab[          	]+xor e
[ ]+586:[ 	]+ac[          	]+xor h
[ ]+587:[ 	]+ad[          	]+xor l
[ ]+588:[ 	]+dd 7c[       	]+ld a,ixh
[ ]+58a:[ 	]+dd 44[       	]+ld b,ixh
[ ]+58c:[ 	]+dd 4c[       	]+ld c,ixh
[ ]+58e:[ 	]+dd 54[       	]+ld d,ixh
[ ]+590:[ 	]+dd 5c[       	]+ld e,ixh
[ ]+592:[ 	]+dd 64[       	]+ld ixh,ixh
[ ]+594:[ 	]+dd 6c[       	]+ld ixl,ixh
[ ]+596:[ 	]+dd 7d[       	]+ld a,ixl
[ ]+598:[ 	]+dd 45[       	]+ld b,ixl
[ ]+59a:[ 	]+dd 4d[       	]+ld c,ixl
[ ]+59c:[ 	]+dd 55[       	]+ld d,ixl
[ ]+59e:[ 	]+dd 5d[       	]+ld e,ixl
[ ]+5a0:[ 	]+dd 65[       	]+ld ixh,ixl
[ ]+5a2:[ 	]+dd 6d[       	]+ld ixl,ixl
[ ]+5a4:[ 	]+fd 7c[       	]+ld a,iyh
[ ]+5a6:[ 	]+fd 44[       	]+ld b,iyh
[ ]+5a8:[ 	]+fd 4c[       	]+ld c,iyh
[ ]+5aa:[ 	]+fd 54[       	]+ld d,iyh
[ ]+5ac:[ 	]+fd 5c[       	]+ld e,iyh
[ ]+5ae:[ 	]+fd 64[       	]+ld iyh,iyh
[ ]+5b0:[ 	]+fd 6c[       	]+ld iyl,iyh
[ ]+5b2:[ 	]+fd 7d[       	]+ld a,iyl
[ ]+5b4:[ 	]+fd 45[       	]+ld b,iyl
[ ]+5b6:[ 	]+fd 4d[       	]+ld c,iyl
[ ]+5b8:[ 	]+fd 55[       	]+ld d,iyl
[ ]+5ba:[ 	]+fd 5d[       	]+ld e,iyl
[ ]+5bc:[ 	]+fd 65[       	]+ld iyh,iyl
[ ]+5be:[ 	]+fd 6d[       	]+ld iyl,iyl
[ ]+5c0:[ 	]+dd 67[       	]+ld ixh,a
[ ]+5c2:[ 	]+dd 60[       	]+ld ixh,b
[ ]+5c4:[ 	]+dd 61[       	]+ld ixh,c
[ ]+5c6:[ 	]+dd 62[       	]+ld ixh,d
[ ]+5c8:[ 	]+dd 63[       	]+ld ixh,e
[ ]+5ca:[ 	]+dd 64[       	]+ld ixh,ixh
[ ]+5cc:[ 	]+dd 65[       	]+ld ixh,ixl
[ ]+5ce:[ 	]+dd 26 19[    	]+ld ixh,0x19
[ ]+5d1:[ 	]+dd 6f[       	]+ld ixl,a
[ ]+5d3:[ 	]+dd 68[       	]+ld ixl,b
[ ]+5d5:[ 	]+dd 69[       	]+ld ixl,c
[ ]+5d7:[ 	]+dd 6a[       	]+ld ixl,d
[ ]+5d9:[ 	]+dd 6b[       	]+ld ixl,e
[ ]+5db:[ 	]+dd 6c[       	]+ld ixl,ixh
[ ]+5dd:[ 	]+dd 6d[       	]+ld ixl,ixl
[ ]+5df:[ 	]+dd 2e 19[    	]+ld ixl,0x19
[ ]+5e2:[ 	]+fd 67[       	]+ld iyh,a
[ ]+5e4:[ 	]+fd 60[       	]+ld iyh,b
[ ]+5e6:[ 	]+fd 61[       	]+ld iyh,c
[ ]+5e8:[ 	]+fd 62[       	]+ld iyh,d
[ ]+5ea:[ 	]+fd 63[       	]+ld iyh,e
[ ]+5ec:[ 	]+fd 64[       	]+ld iyh,iyh
[ ]+5ee:[ 	]+fd 65[       	]+ld iyh,iyl
[ ]+5f0:[ 	]+fd 26 19[    	]+ld iyh,0x19
[ ]+5f3:[ 	]+fd 6f[       	]+ld iyl,a
[ ]+5f5:[ 	]+fd 68[       	]+ld iyl,b
[ ]+5f7:[ 	]+fd 69[       	]+ld iyl,c
[ ]+5f9:[ 	]+fd 6a[       	]+ld iyl,d
[ ]+5fb:[ 	]+fd 6b[       	]+ld iyl,e
[ ]+5fd:[ 	]+fd 6c[       	]+ld iyl,iyh
[ ]+5ff:[ 	]+fd 6d[       	]+ld iyl,iyl
[ ]+601:[ 	]+fd 2e 19[    	]+ld iyl,0x19
[ ]+604:[ 	]+dd 84[       	]+add a,ixh
[ ]+606:[ 	]+dd 85[       	]+add a,ixl
[ ]+608:[ 	]+fd 84[       	]+add a,iyh
[ ]+60a:[ 	]+fd 85[       	]+add a,iyl
[ ]+60c:[ 	]+dd 8c[       	]+adc a,ixh
[ ]+60e:[ 	]+dd 8d[       	]+adc a,ixl
[ ]+610:[ 	]+fd 8c[       	]+adc a,iyh
[ ]+612:[ 	]+fd 8d[       	]+adc a,iyl
[ ]+614:[ 	]+dd bc[       	]+cp ixh
[ ]+616:[ 	]+dd bd[       	]+cp ixl
[ ]+618:[ 	]+fd bc[       	]+cp iyh
[ ]+61a:[ 	]+fd bd[       	]+cp iyl
[ ]+61c:[ 	]+dd 25[       	]+dec ixh
[ ]+61e:[ 	]+dd 2d[       	]+dec ixl
[ ]+620:[ 	]+fd 25[       	]+dec iyh
[ ]+622:[ 	]+fd 2d[       	]+dec iyl
[ ]+624:[ 	]+dd 24[       	]+inc ixh
[ ]+626:[ 	]+dd 2c[       	]+inc ixl
[ ]+628:[ 	]+fd 24[       	]+inc iyh
[ ]+62a:[ 	]+fd 2c[       	]+inc iyl
[ ]+62c:[ 	]+dd 9c[       	]+sbc a,ixh
[ ]+62e:[ 	]+dd 9d[       	]+sbc a,ixl
[ ]+630:[ 	]+fd 9c[       	]+sbc a,iyh
[ ]+632:[ 	]+fd 9d[       	]+sbc a,iyl
[ ]+634:[ 	]+dd 94[       	]+sub ixh
[ ]+636:[ 	]+dd 95[       	]+sub ixl
[ ]+638:[ 	]+fd 94[       	]+sub iyh
[ ]+63a:[ 	]+fd 95[       	]+sub iyl
[ ]+63c:[ 	]+dd a4[       	]+and ixh
[ ]+63e:[ 	]+dd a5[       	]+and ixl
[ ]+640:[ 	]+fd a4[       	]+and iyh
[ ]+642:[ 	]+fd a5[       	]+and iyl
[ ]+644:[ 	]+dd b4[       	]+or ixh
[ ]+646:[ 	]+dd b5[       	]+or ixl
[ ]+648:[ 	]+fd b4[       	]+or iyh
[ ]+64a:[ 	]+fd b5[       	]+or iyl
[ ]+64c:[ 	]+dd ac[       	]+xor ixh
[ ]+64e:[ 	]+dd ad[       	]+xor ixl
[ ]+650:[ 	]+fd ac[       	]+xor iyh
[ ]+652:[ 	]+fd ad[       	]+xor iyl
[ ]+654:[ 	]+ed 70[       	]+in f,\(c\)
[ ]+656:[ 	]+ed 70[       	]+in f,\(c\)
[ ]+658:[ 	]+ed 71[       	]+out \(c\),0
[ ]+65a:[ 	]+dd cb 08 07[ 	]+rlc \(ix\+8\),a
[ ]+65e:[ 	]+dd cb 08 00[ 	]+rlc \(ix\+8\),b
[ ]+662:[ 	]+dd cb 08 01[ 	]+rlc \(ix\+8\),c
[ ]+666:[ 	]+dd cb 08 02[ 	]+rlc \(ix\+8\),d
[ ]+66a:[ 	]+dd cb 08 03[ 	]+rlc \(ix\+8\),e
[ ]+66e:[ 	]+dd cb 08 04[ 	]+rlc \(ix\+8\),h
[ ]+672:[ 	]+dd cb 08 05[ 	]+rlc \(ix\+8\),l
[ ]+676:[ 	]+fd cb 08 07[ 	]+rlc \(iy\+8\),a
[ ]+67a:[ 	]+fd cb 08 00[ 	]+rlc \(iy\+8\),b
[ ]+67e:[ 	]+fd cb 08 01[ 	]+rlc \(iy\+8\),c
[ ]+682:[ 	]+fd cb 08 02[ 	]+rlc \(iy\+8\),d
[ ]+686:[ 	]+fd cb 08 03[ 	]+rlc \(iy\+8\),e
[ ]+68a:[ 	]+fd cb 08 04[ 	]+rlc \(iy\+8\),h
[ ]+68e:[ 	]+fd cb 08 05[ 	]+rlc \(iy\+8\),l
[ ]+692:[ 	]+dd cb 08 0f[ 	]+rrc \(ix\+8\),a
[ ]+696:[ 	]+dd cb 08 08[ 	]+rrc \(ix\+8\),b
[ ]+69a:[ 	]+dd cb 08 09[ 	]+rrc \(ix\+8\),c
[ ]+69e:[ 	]+dd cb 08 0a[ 	]+rrc \(ix\+8\),d
[ ]+6a2:[ 	]+dd cb 08 0b[ 	]+rrc \(ix\+8\),e
[ ]+6a6:[ 	]+dd cb 08 0c[ 	]+rrc \(ix\+8\),h
[ ]+6aa:[ 	]+dd cb 08 0d[ 	]+rrc \(ix\+8\),l
[ ]+6ae:[ 	]+fd cb 08 0f[ 	]+rrc \(iy\+8\),a
[ ]+6b2:[ 	]+fd cb 08 08[ 	]+rrc \(iy\+8\),b
[ ]+6b6:[ 	]+fd cb 08 09[ 	]+rrc \(iy\+8\),c
[ ]+6ba:[ 	]+fd cb 08 0a[ 	]+rrc \(iy\+8\),d
[ ]+6be:[ 	]+fd cb 08 0b[ 	]+rrc \(iy\+8\),e
[ ]+6c2:[ 	]+fd cb 08 0c[ 	]+rrc \(iy\+8\),h
[ ]+6c6:[ 	]+fd cb 08 0d[ 	]+rrc \(iy\+8\),l
[ ]+6ca:[ 	]+dd cb 08 17[ 	]+rl \(ix\+8\),a
[ ]+6ce:[ 	]+dd cb 08 10[ 	]+rl \(ix\+8\),b
[ ]+6d2:[ 	]+dd cb 08 11[ 	]+rl \(ix\+8\),c
[ ]+6d6:[ 	]+dd cb 08 12[ 	]+rl \(ix\+8\),d
[ ]+6da:[ 	]+dd cb 08 13[ 	]+rl \(ix\+8\),e
[ ]+6de:[ 	]+dd cb 08 14[ 	]+rl \(ix\+8\),h
[ ]+6e2:[ 	]+dd cb 08 15[ 	]+rl \(ix\+8\),l
[ ]+6e6:[ 	]+fd cb 08 17[ 	]+rl \(iy\+8\),a
[ ]+6ea:[ 	]+fd cb 08 10[ 	]+rl \(iy\+8\),b
[ ]+6ee:[ 	]+fd cb 08 11[ 	]+rl \(iy\+8\),c
[ ]+6f2:[ 	]+fd cb 08 12[ 	]+rl \(iy\+8\),d
[ ]+6f6:[ 	]+fd cb 08 13[ 	]+rl \(iy\+8\),e
[ ]+6fa:[ 	]+fd cb 08 14[ 	]+rl \(iy\+8\),h
[ ]+6fe:[ 	]+fd cb 08 15[ 	]+rl \(iy\+8\),l
[ ]+702:[ 	]+dd cb 08 1f[ 	]+rr \(ix\+8\),a
[ ]+706:[ 	]+dd cb 08 18[ 	]+rr \(ix\+8\),b
[ ]+70a:[ 	]+dd cb 08 19[ 	]+rr \(ix\+8\),c
[ ]+70e:[ 	]+dd cb 08 1a[ 	]+rr \(ix\+8\),d
[ ]+712:[ 	]+dd cb 08 1b[ 	]+rr \(ix\+8\),e
[ ]+716:[ 	]+dd cb 08 1c[ 	]+rr \(ix\+8\),h
[ ]+71a:[ 	]+dd cb 08 1d[ 	]+rr \(ix\+8\),l
[ ]+71e:[ 	]+fd cb 08 1f[ 	]+rr \(iy\+8\),a
[ ]+722:[ 	]+fd cb 08 18[ 	]+rr \(iy\+8\),b
[ ]+726:[ 	]+fd cb 08 19[ 	]+rr \(iy\+8\),c
[ ]+72a:[ 	]+fd cb 08 1a[ 	]+rr \(iy\+8\),d
[ ]+72e:[ 	]+fd cb 08 1b[ 	]+rr \(iy\+8\),e
[ ]+732:[ 	]+fd cb 08 1c[ 	]+rr \(iy\+8\),h
[ ]+736:[ 	]+fd cb 08 1d[ 	]+rr \(iy\+8\),l
[ ]+73a:[ 	]+dd cb 08 27[ 	]+sla \(ix\+8\),a
[ ]+73e:[ 	]+dd cb 08 20[ 	]+sla \(ix\+8\),b
[ ]+742:[ 	]+dd cb 08 21[ 	]+sla \(ix\+8\),c
[ ]+746:[ 	]+dd cb 08 22[ 	]+sla \(ix\+8\),d
[ ]+74a:[ 	]+dd cb 08 23[ 	]+sla \(ix\+8\),e
[ ]+74e:[ 	]+dd cb 08 24[ 	]+sla \(ix\+8\),h
[ ]+752:[ 	]+dd cb 08 25[ 	]+sla \(ix\+8\),l
[ ]+756:[ 	]+fd cb 08 27[ 	]+sla \(iy\+8\),a
[ ]+75a:[ 	]+fd cb 08 20[ 	]+sla \(iy\+8\),b
[ ]+75e:[ 	]+fd cb 08 21[ 	]+sla \(iy\+8\),c
[ ]+762:[ 	]+fd cb 08 22[ 	]+sla \(iy\+8\),d
[ ]+766:[ 	]+fd cb 08 23[ 	]+sla \(iy\+8\),e
[ ]+76a:[ 	]+fd cb 08 24[ 	]+sla \(iy\+8\),h
[ ]+76e:[ 	]+fd cb 08 25[ 	]+sla \(iy\+8\),l
[ ]+772:[ 	]+dd cb 08 2f[ 	]+sra \(ix\+8\),a
[ ]+776:[ 	]+dd cb 08 28[ 	]+sra \(ix\+8\),b
[ ]+77a:[ 	]+dd cb 08 29[ 	]+sra \(ix\+8\),c
[ ]+77e:[ 	]+dd cb 08 2a[ 	]+sra \(ix\+8\),d
[ ]+782:[ 	]+dd cb 08 2b[ 	]+sra \(ix\+8\),e
[ ]+786:[ 	]+dd cb 08 2c[ 	]+sra \(ix\+8\),h
[ ]+78a:[ 	]+dd cb 08 2d[ 	]+sra \(ix\+8\),l
[ ]+78e:[ 	]+fd cb 08 2f[ 	]+sra \(iy\+8\),a
[ ]+792:[ 	]+fd cb 08 28[ 	]+sra \(iy\+8\),b
[ ]+796:[ 	]+fd cb 08 29[ 	]+sra \(iy\+8\),c
[ ]+79a:[ 	]+fd cb 08 2a[ 	]+sra \(iy\+8\),d
[ ]+79e:[ 	]+fd cb 08 2b[ 	]+sra \(iy\+8\),e
[ ]+7a2:[ 	]+fd cb 08 2c[ 	]+sra \(iy\+8\),h
[ ]+7a6:[ 	]+fd cb 08 2d[ 	]+sra \(iy\+8\),l
[ ]+7aa:[ 	]+dd cb 08 37[ 	]+sli \(ix\+8\),a
[ ]+7ae:[ 	]+dd cb 08 30[ 	]+sli \(ix\+8\),b
[ ]+7b2:[ 	]+dd cb 08 31[ 	]+sli \(ix\+8\),c
[ ]+7b6:[ 	]+dd cb 08 32[ 	]+sli \(ix\+8\),d
[ ]+7ba:[ 	]+dd cb 08 33[ 	]+sli \(ix\+8\),e
[ ]+7be:[ 	]+dd cb 08 34[ 	]+sli \(ix\+8\),h
[ ]+7c2:[ 	]+dd cb 08 35[ 	]+sli \(ix\+8\),l
[ ]+7c6:[ 	]+fd cb 08 37[ 	]+sli \(iy\+8\),a
[ ]+7ca:[ 	]+fd cb 08 30[ 	]+sli \(iy\+8\),b
[ ]+7ce:[ 	]+fd cb 08 31[ 	]+sli \(iy\+8\),c
[ ]+7d2:[ 	]+fd cb 08 32[ 	]+sli \(iy\+8\),d
[ ]+7d6:[ 	]+fd cb 08 33[ 	]+sli \(iy\+8\),e
[ ]+7da:[ 	]+fd cb 08 34[ 	]+sli \(iy\+8\),h
[ ]+7de:[ 	]+fd cb 08 35[ 	]+sli \(iy\+8\),l
[ ]+7e2:[ 	]+dd cb 08 3f[ 	]+srl \(ix\+8\),a
[ ]+7e6:[ 	]+dd cb 08 38[ 	]+srl \(ix\+8\),b
[ ]+7ea:[ 	]+dd cb 08 39[ 	]+srl \(ix\+8\),c
[ ]+7ee:[ 	]+dd cb 08 3a[ 	]+srl \(ix\+8\),d
[ ]+7f2:[ 	]+dd cb 08 3b[ 	]+srl \(ix\+8\),e
[ ]+7f6:[ 	]+dd cb 08 3c[ 	]+srl \(ix\+8\),h
[ ]+7fa:[ 	]+dd cb 08 3d[ 	]+srl \(ix\+8\),l
[ ]+7fe:[ 	]+fd cb 08 3f[ 	]+srl \(iy\+8\),a
[ ]+802:[ 	]+fd cb 08 38[ 	]+srl \(iy\+8\),b
[ ]+806:[ 	]+fd cb 08 39[ 	]+srl \(iy\+8\),c
[ ]+80a:[ 	]+fd cb 08 3a[ 	]+srl \(iy\+8\),d
[ ]+80e:[ 	]+fd cb 08 3b[ 	]+srl \(iy\+8\),e
[ ]+812:[ 	]+fd cb 08 3c[ 	]+srl \(iy\+8\),h
[ ]+816:[ 	]+fd cb 08 3d[ 	]+srl \(iy\+8\),l
[ ]+81a:[ 	]+dd cb 08 87[ 	]+res 0,\(ix\+8\),a
[ ]+81e:[ 	]+dd cb 08 80[ 	]+res 0,\(ix\+8\),b
[ ]+822:[ 	]+dd cb 08 81[ 	]+res 0,\(ix\+8\),c
[ ]+826:[ 	]+dd cb 08 82[ 	]+res 0,\(ix\+8\),d
[ ]+82a:[ 	]+dd cb 08 83[ 	]+res 0,\(ix\+8\),e
[ ]+82e:[ 	]+dd cb 08 84[ 	]+res 0,\(ix\+8\),h
[ ]+832:[ 	]+dd cb 08 85[ 	]+res 0,\(ix\+8\),l
[ ]+836:[ 	]+fd cb 08 87[ 	]+res 0,\(iy\+8\),a
[ ]+83a:[ 	]+fd cb 08 80[ 	]+res 0,\(iy\+8\),b
[ ]+83e:[ 	]+fd cb 08 81[ 	]+res 0,\(iy\+8\),c
[ ]+842:[ 	]+fd cb 08 82[ 	]+res 0,\(iy\+8\),d
[ ]+846:[ 	]+fd cb 08 83[ 	]+res 0,\(iy\+8\),e
[ ]+84a:[ 	]+fd cb 08 84[ 	]+res 0,\(iy\+8\),h
[ ]+84e:[ 	]+fd cb 08 85[ 	]+res 0,\(iy\+8\),l
[ ]+852:[ 	]+dd cb 08 8f[ 	]+res 1,\(ix\+8\),a
[ ]+856:[ 	]+dd cb 08 88[ 	]+res 1,\(ix\+8\),b
[ ]+85a:[ 	]+dd cb 08 89[ 	]+res 1,\(ix\+8\),c
[ ]+85e:[ 	]+dd cb 08 8a[ 	]+res 1,\(ix\+8\),d
[ ]+862:[ 	]+dd cb 08 8b[ 	]+res 1,\(ix\+8\),e
[ ]+866:[ 	]+dd cb 08 8c[ 	]+res 1,\(ix\+8\),h
[ ]+86a:[ 	]+dd cb 08 8d[ 	]+res 1,\(ix\+8\),l
[ ]+86e:[ 	]+fd cb 08 8f[ 	]+res 1,\(iy\+8\),a
[ ]+872:[ 	]+fd cb 08 88[ 	]+res 1,\(iy\+8\),b
[ ]+876:[ 	]+fd cb 08 89[ 	]+res 1,\(iy\+8\),c
[ ]+87a:[ 	]+fd cb 08 8a[ 	]+res 1,\(iy\+8\),d
[ ]+87e:[ 	]+fd cb 08 8b[ 	]+res 1,\(iy\+8\),e
[ ]+882:[ 	]+fd cb 08 8c[ 	]+res 1,\(iy\+8\),h
[ ]+886:[ 	]+fd cb 08 8d[ 	]+res 1,\(iy\+8\),l
[ ]+88a:[ 	]+dd cb 08 97[ 	]+res 2,\(ix\+8\),a
[ ]+88e:[ 	]+dd cb 08 90[ 	]+res 2,\(ix\+8\),b
[ ]+892:[ 	]+dd cb 08 91[ 	]+res 2,\(ix\+8\),c
[ ]+896:[ 	]+dd cb 08 92[ 	]+res 2,\(ix\+8\),d
[ ]+89a:[ 	]+dd cb 08 93[ 	]+res 2,\(ix\+8\),e
[ ]+89e:[ 	]+dd cb 08 94[ 	]+res 2,\(ix\+8\),h
[ ]+8a2:[ 	]+dd cb 08 95[ 	]+res 2,\(ix\+8\),l
[ ]+8a6:[ 	]+fd cb 08 97[ 	]+res 2,\(iy\+8\),a
[ ]+8aa:[ 	]+fd cb 08 90[ 	]+res 2,\(iy\+8\),b
[ ]+8ae:[ 	]+fd cb 08 91[ 	]+res 2,\(iy\+8\),c
[ ]+8b2:[ 	]+fd cb 08 92[ 	]+res 2,\(iy\+8\),d
[ ]+8b6:[ 	]+fd cb 08 93[ 	]+res 2,\(iy\+8\),e
[ ]+8ba:[ 	]+fd cb 08 94[ 	]+res 2,\(iy\+8\),h
[ ]+8be:[ 	]+fd cb 08 95[ 	]+res 2,\(iy\+8\),l
[ ]+8c2:[ 	]+dd cb 08 9f[ 	]+res 3,\(ix\+8\),a
[ ]+8c6:[ 	]+dd cb 08 98[ 	]+res 3,\(ix\+8\),b
[ ]+8ca:[ 	]+dd cb 08 99[ 	]+res 3,\(ix\+8\),c
[ ]+8ce:[ 	]+dd cb 08 9a[ 	]+res 3,\(ix\+8\),d
[ ]+8d2:[ 	]+dd cb 08 9b[ 	]+res 3,\(ix\+8\),e
[ ]+8d6:[ 	]+dd cb 08 9c[ 	]+res 3,\(ix\+8\),h
[ ]+8da:[ 	]+dd cb 08 9d[ 	]+res 3,\(ix\+8\),l
[ ]+8de:[ 	]+fd cb 08 9f[ 	]+res 3,\(iy\+8\),a
[ ]+8e2:[ 	]+fd cb 08 98[ 	]+res 3,\(iy\+8\),b
[ ]+8e6:[ 	]+fd cb 08 99[ 	]+res 3,\(iy\+8\),c
[ ]+8ea:[ 	]+fd cb 08 9a[ 	]+res 3,\(iy\+8\),d
[ ]+8ee:[ 	]+fd cb 08 9b[ 	]+res 3,\(iy\+8\),e
[ ]+8f2:[ 	]+fd cb 08 9c[ 	]+res 3,\(iy\+8\),h
[ ]+8f6:[ 	]+fd cb 08 9d[ 	]+res 3,\(iy\+8\),l
[ ]+8fa:[ 	]+dd cb 08 a7[ 	]+res 4,\(ix\+8\),a
[ ]+8fe:[ 	]+dd cb 08 a0[ 	]+res 4,\(ix\+8\),b
[ ]+902:[ 	]+dd cb 08 a1[ 	]+res 4,\(ix\+8\),c
[ ]+906:[ 	]+dd cb 08 a2[ 	]+res 4,\(ix\+8\),d
[ ]+90a:[ 	]+dd cb 08 a3[ 	]+res 4,\(ix\+8\),e
[ ]+90e:[ 	]+dd cb 08 a4[ 	]+res 4,\(ix\+8\),h
[ ]+912:[ 	]+dd cb 08 a5[ 	]+res 4,\(ix\+8\),l
[ ]+916:[ 	]+fd cb 08 a7[ 	]+res 4,\(iy\+8\),a
[ ]+91a:[ 	]+fd cb 08 a0[ 	]+res 4,\(iy\+8\),b
[ ]+91e:[ 	]+fd cb 08 a1[ 	]+res 4,\(iy\+8\),c
[ ]+922:[ 	]+fd cb 08 a2[ 	]+res 4,\(iy\+8\),d
[ ]+926:[ 	]+fd cb 08 a3[ 	]+res 4,\(iy\+8\),e
[ ]+92a:[ 	]+fd cb 08 a4[ 	]+res 4,\(iy\+8\),h
[ ]+92e:[ 	]+fd cb 08 a5[ 	]+res 4,\(iy\+8\),l
[ ]+932:[ 	]+dd cb 08 af[ 	]+res 5,\(ix\+8\),a
[ ]+936:[ 	]+dd cb 08 a8[ 	]+res 5,\(ix\+8\),b
[ ]+93a:[ 	]+dd cb 08 a9[ 	]+res 5,\(ix\+8\),c
[ ]+93e:[ 	]+dd cb 08 aa[ 	]+res 5,\(ix\+8\),d
[ ]+942:[ 	]+dd cb 08 ab[ 	]+res 5,\(ix\+8\),e
[ ]+946:[ 	]+dd cb 08 ac[ 	]+res 5,\(ix\+8\),h
[ ]+94a:[ 	]+dd cb 08 ad[ 	]+res 5,\(ix\+8\),l
[ ]+94e:[ 	]+fd cb 08 af[ 	]+res 5,\(iy\+8\),a
[ ]+952:[ 	]+fd cb 08 a8[ 	]+res 5,\(iy\+8\),b
[ ]+956:[ 	]+fd cb 08 a9[ 	]+res 5,\(iy\+8\),c
[ ]+95a:[ 	]+fd cb 08 aa[ 	]+res 5,\(iy\+8\),d
[ ]+95e:[ 	]+fd cb 08 ab[ 	]+res 5,\(iy\+8\),e
[ ]+962:[ 	]+fd cb 08 ac[ 	]+res 5,\(iy\+8\),h
[ ]+966:[ 	]+fd cb 08 ad[ 	]+res 5,\(iy\+8\),l
[ ]+96a:[ 	]+dd cb 08 b7[ 	]+res 6,\(ix\+8\),a
[ ]+96e:[ 	]+dd cb 08 b0[ 	]+res 6,\(ix\+8\),b
[ ]+972:[ 	]+dd cb 08 b1[ 	]+res 6,\(ix\+8\),c
[ ]+976:[ 	]+dd cb 08 b2[ 	]+res 6,\(ix\+8\),d
[ ]+97a:[ 	]+dd cb 08 b3[ 	]+res 6,\(ix\+8\),e
[ ]+97e:[ 	]+dd cb 08 b4[ 	]+res 6,\(ix\+8\),h
[ ]+982:[ 	]+dd cb 08 b5[ 	]+res 6,\(ix\+8\),l
[ ]+986:[ 	]+fd cb 08 b7[ 	]+res 6,\(iy\+8\),a
[ ]+98a:[ 	]+fd cb 08 b0[ 	]+res 6,\(iy\+8\),b
[ ]+98e:[ 	]+fd cb 08 b1[ 	]+res 6,\(iy\+8\),c
[ ]+992:[ 	]+fd cb 08 b2[ 	]+res 6,\(iy\+8\),d
[ ]+996:[ 	]+fd cb 08 b3[ 	]+res 6,\(iy\+8\),e
[ ]+99a:[ 	]+fd cb 08 b4[ 	]+res 6,\(iy\+8\),h
[ ]+99e:[ 	]+fd cb 08 b5[ 	]+res 6,\(iy\+8\),l
[ ]+9a2:[ 	]+dd cb 08 bf[ 	]+res 7,\(ix\+8\),a
[ ]+9a6:[ 	]+dd cb 08 b8[ 	]+res 7,\(ix\+8\),b
[ ]+9aa:[ 	]+dd cb 08 b9[ 	]+res 7,\(ix\+8\),c
[ ]+9ae:[ 	]+dd cb 08 ba[ 	]+res 7,\(ix\+8\),d
[ ]+9b2:[ 	]+dd cb 08 bb[ 	]+res 7,\(ix\+8\),e
[ ]+9b6:[ 	]+dd cb 08 bc[ 	]+res 7,\(ix\+8\),h
[ ]+9ba:[ 	]+dd cb 08 bd[ 	]+res 7,\(ix\+8\),l
[ ]+9be:[ 	]+fd cb 08 bf[ 	]+res 7,\(iy\+8\),a
[ ]+9c2:[ 	]+fd cb 08 b8[ 	]+res 7,\(iy\+8\),b
[ ]+9c6:[ 	]+fd cb 08 b9[ 	]+res 7,\(iy\+8\),c
[ ]+9ca:[ 	]+fd cb 08 ba[ 	]+res 7,\(iy\+8\),d
[ ]+9ce:[ 	]+fd cb 08 bb[ 	]+res 7,\(iy\+8\),e
[ ]+9d2:[ 	]+fd cb 08 bc[ 	]+res 7,\(iy\+8\),h
[ ]+9d6:[ 	]+fd cb 08 bd[ 	]+res 7,\(iy\+8\),l
[ ]+9da:[ 	]+dd cb 08 c7[ 	]+set 0,\(ix\+8\),a
[ ]+9de:[ 	]+dd cb 08 c0[ 	]+set 0,\(ix\+8\),b
[ ]+9e2:[ 	]+dd cb 08 c1[ 	]+set 0,\(ix\+8\),c
[ ]+9e6:[ 	]+dd cb 08 c2[ 	]+set 0,\(ix\+8\),d
[ ]+9ea:[ 	]+dd cb 08 c3[ 	]+set 0,\(ix\+8\),e
[ ]+9ee:[ 	]+dd cb 08 c4[ 	]+set 0,\(ix\+8\),h
[ ]+9f2:[ 	]+dd cb 08 c5[ 	]+set 0,\(ix\+8\),l
[ ]+9f6:[ 	]+fd cb 08 c7[ 	]+set 0,\(iy\+8\),a
[ ]+9fa:[ 	]+fd cb 08 c0[ 	]+set 0,\(iy\+8\),b
[ ]+9fe:[ 	]+fd cb 08 c1[ 	]+set 0,\(iy\+8\),c
[ ]+a02:[ 	]+fd cb 08 c2[ 	]+set 0,\(iy\+8\),d
[ ]+a06:[ 	]+fd cb 08 c3[ 	]+set 0,\(iy\+8\),e
[ ]+a0a:[ 	]+fd cb 08 c4[ 	]+set 0,\(iy\+8\),h
[ ]+a0e:[ 	]+fd cb 08 c5[ 	]+set 0,\(iy\+8\),l
[ ]+a12:[ 	]+dd cb 08 cf[ 	]+set 1,\(ix\+8\),a
[ ]+a16:[ 	]+dd cb 08 c8[ 	]+set 1,\(ix\+8\),b
[ ]+a1a:[ 	]+dd cb 08 c9[ 	]+set 1,\(ix\+8\),c
[ ]+a1e:[ 	]+dd cb 08 ca[ 	]+set 1,\(ix\+8\),d
[ ]+a22:[ 	]+dd cb 08 cb[ 	]+set 1,\(ix\+8\),e
[ ]+a26:[ 	]+dd cb 08 cc[ 	]+set 1,\(ix\+8\),h
[ ]+a2a:[ 	]+dd cb 08 cd[ 	]+set 1,\(ix\+8\),l
[ ]+a2e:[ 	]+fd cb 08 cf[ 	]+set 1,\(iy\+8\),a
[ ]+a32:[ 	]+fd cb 08 c8[ 	]+set 1,\(iy\+8\),b
[ ]+a36:[ 	]+fd cb 08 c9[ 	]+set 1,\(iy\+8\),c
[ ]+a3a:[ 	]+fd cb 08 ca[ 	]+set 1,\(iy\+8\),d
[ ]+a3e:[ 	]+fd cb 08 cb[ 	]+set 1,\(iy\+8\),e
[ ]+a42:[ 	]+fd cb 08 cc[ 	]+set 1,\(iy\+8\),h
[ ]+a46:[ 	]+fd cb 08 cd[ 	]+set 1,\(iy\+8\),l
[ ]+a4a:[ 	]+dd cb 08 d7[ 	]+set 2,\(ix\+8\),a
[ ]+a4e:[ 	]+dd cb 08 d0[ 	]+set 2,\(ix\+8\),b
[ ]+a52:[ 	]+dd cb 08 d1[ 	]+set 2,\(ix\+8\),c
[ ]+a56:[ 	]+dd cb 08 d2[ 	]+set 2,\(ix\+8\),d
[ ]+a5a:[ 	]+dd cb 08 d3[ 	]+set 2,\(ix\+8\),e
[ ]+a5e:[ 	]+dd cb 08 d4[ 	]+set 2,\(ix\+8\),h
[ ]+a62:[ 	]+dd cb 08 d5[ 	]+set 2,\(ix\+8\),l
[ ]+a66:[ 	]+fd cb 08 d7[ 	]+set 2,\(iy\+8\),a
[ ]+a6a:[ 	]+fd cb 08 d0[ 	]+set 2,\(iy\+8\),b
[ ]+a6e:[ 	]+fd cb 08 d1[ 	]+set 2,\(iy\+8\),c
[ ]+a72:[ 	]+fd cb 08 d2[ 	]+set 2,\(iy\+8\),d
[ ]+a76:[ 	]+fd cb 08 d3[ 	]+set 2,\(iy\+8\),e
[ ]+a7a:[ 	]+fd cb 08 d4[ 	]+set 2,\(iy\+8\),h
[ ]+a7e:[ 	]+fd cb 08 d5[ 	]+set 2,\(iy\+8\),l
[ ]+a82:[ 	]+dd cb 08 df[ 	]+set 3,\(ix\+8\),a
[ ]+a86:[ 	]+dd cb 08 d8[ 	]+set 3,\(ix\+8\),b
[ ]+a8a:[ 	]+dd cb 08 d9[ 	]+set 3,\(ix\+8\),c
[ ]+a8e:[ 	]+dd cb 08 da[ 	]+set 3,\(ix\+8\),d
[ ]+a92:[ 	]+dd cb 08 db[ 	]+set 3,\(ix\+8\),e
[ ]+a96:[ 	]+dd cb 08 dc[ 	]+set 3,\(ix\+8\),h
[ ]+a9a:[ 	]+dd cb 08 dd[ 	]+set 3,\(ix\+8\),l
[ ]+a9e:[ 	]+fd cb 08 df[ 	]+set 3,\(iy\+8\),a
[ ]+aa2:[ 	]+fd cb 08 d8[ 	]+set 3,\(iy\+8\),b
[ ]+aa6:[ 	]+fd cb 08 d9[ 	]+set 3,\(iy\+8\),c
[ ]+aaa:[ 	]+fd cb 08 da[ 	]+set 3,\(iy\+8\),d
[ ]+aae:[ 	]+fd cb 08 db[ 	]+set 3,\(iy\+8\),e
[ ]+ab2:[ 	]+fd cb 08 dc[ 	]+set 3,\(iy\+8\),h
[ ]+ab6:[ 	]+fd cb 08 dd[ 	]+set 3,\(iy\+8\),l
[ ]+aba:[ 	]+dd cb 08 e7[ 	]+set 4,\(ix\+8\),a
[ ]+abe:[ 	]+dd cb 08 e0[ 	]+set 4,\(ix\+8\),b
[ ]+ac2:[ 	]+dd cb 08 e1[ 	]+set 4,\(ix\+8\),c
[ ]+ac6:[ 	]+dd cb 08 e2[ 	]+set 4,\(ix\+8\),d
[ ]+aca:[ 	]+dd cb 08 e3[ 	]+set 4,\(ix\+8\),e
[ ]+ace:[ 	]+dd cb 08 e4[ 	]+set 4,\(ix\+8\),h
[ ]+ad2:[ 	]+dd cb 08 e5[ 	]+set 4,\(ix\+8\),l
[ ]+ad6:[ 	]+fd cb 08 e7[ 	]+set 4,\(iy\+8\),a
[ ]+ada:[ 	]+fd cb 08 e0[ 	]+set 4,\(iy\+8\),b
[ ]+ade:[ 	]+fd cb 08 e1[ 	]+set 4,\(iy\+8\),c
[ ]+ae2:[ 	]+fd cb 08 e2[ 	]+set 4,\(iy\+8\),d
[ ]+ae6:[ 	]+fd cb 08 e3[ 	]+set 4,\(iy\+8\),e
[ ]+aea:[ 	]+fd cb 08 e4[ 	]+set 4,\(iy\+8\),h
[ ]+aee:[ 	]+fd cb 08 e5[ 	]+set 4,\(iy\+8\),l
[ ]+af2:[ 	]+dd cb 08 ef[ 	]+set 5,\(ix\+8\),a
[ ]+af6:[ 	]+dd cb 08 e8[ 	]+set 5,\(ix\+8\),b
[ ]+afa:[ 	]+dd cb 08 e9[ 	]+set 5,\(ix\+8\),c
[ ]+afe:[ 	]+dd cb 08 ea[ 	]+set 5,\(ix\+8\),d
[ ]+b02:[ 	]+dd cb 08 eb[ 	]+set 5,\(ix\+8\),e
[ ]+b06:[ 	]+dd cb 08 ec[ 	]+set 5,\(ix\+8\),h
[ ]+b0a:[ 	]+dd cb 08 ed[ 	]+set 5,\(ix\+8\),l
[ ]+b0e:[ 	]+fd cb 08 ef[ 	]+set 5,\(iy\+8\),a
[ ]+b12:[ 	]+fd cb 08 e8[ 	]+set 5,\(iy\+8\),b
[ ]+b16:[ 	]+fd cb 08 e9[ 	]+set 5,\(iy\+8\),c
[ ]+b1a:[ 	]+fd cb 08 ea[ 	]+set 5,\(iy\+8\),d
[ ]+b1e:[ 	]+fd cb 08 eb[ 	]+set 5,\(iy\+8\),e
[ ]+b22:[ 	]+fd cb 08 ec[ 	]+set 5,\(iy\+8\),h
[ ]+b26:[ 	]+fd cb 08 ed[ 	]+set 5,\(iy\+8\),l
[ ]+b2a:[ 	]+dd cb 08 f7[ 	]+set 6,\(ix\+8\),a
[ ]+b2e:[ 	]+dd cb 08 f0[ 	]+set 6,\(ix\+8\),b
[ ]+b32:[ 	]+dd cb 08 f1[ 	]+set 6,\(ix\+8\),c
[ ]+b36:[ 	]+dd cb 08 f2[ 	]+set 6,\(ix\+8\),d
[ ]+b3a:[ 	]+dd cb 08 f3[ 	]+set 6,\(ix\+8\),e
[ ]+b3e:[ 	]+dd cb 08 f4[ 	]+set 6,\(ix\+8\),h
[ ]+b42:[ 	]+dd cb 08 f5[ 	]+set 6,\(ix\+8\),l
[ ]+b46:[ 	]+fd cb 08 f7[ 	]+set 6,\(iy\+8\),a
[ ]+b4a:[ 	]+fd cb 08 f0[ 	]+set 6,\(iy\+8\),b
[ ]+b4e:[ 	]+fd cb 08 f1[ 	]+set 6,\(iy\+8\),c
[ ]+b52:[ 	]+fd cb 08 f2[ 	]+set 6,\(iy\+8\),d
[ ]+b56:[ 	]+fd cb 08 f3[ 	]+set 6,\(iy\+8\),e
[ ]+b5a:[ 	]+fd cb 08 f4[ 	]+set 6,\(iy\+8\),h
[ ]+b5e:[ 	]+fd cb 08 f5[ 	]+set 6,\(iy\+8\),l
[ ]+b62:[ 	]+dd cb 08 ff[ 	]+set 7,\(ix\+8\),a
[ ]+b66:[ 	]+dd cb 08 f8[ 	]+set 7,\(ix\+8\),b
[ ]+b6a:[ 	]+dd cb 08 f9[ 	]+set 7,\(ix\+8\),c
[ ]+b6e:[ 	]+dd cb 08 fa[ 	]+set 7,\(ix\+8\),d
[ ]+b72:[ 	]+dd cb 08 fb[ 	]+set 7,\(ix\+8\),e
[ ]+b76:[ 	]+dd cb 08 fc[ 	]+set 7,\(ix\+8\),h
[ ]+b7a:[ 	]+dd cb 08 fd[ 	]+set 7,\(ix\+8\),l
[ ]+b7e:[ 	]+fd cb 08 ff[ 	]+set 7,\(iy\+8\),a
[ ]+b82:[ 	]+fd cb 08 f8[ 	]+set 7,\(iy\+8\),b
[ ]+b86:[ 	]+fd cb 08 f9[ 	]+set 7,\(iy\+8\),c
[ ]+b8a:[ 	]+fd cb 08 fa[ 	]+set 7,\(iy\+8\),d
[ ]+b8e:[ 	]+fd cb 08 fb[ 	]+set 7,\(iy\+8\),e
[ ]+b92:[ 	]+fd cb 08 fc[ 	]+set 7,\(iy\+8\),h
[ ]+b96:[ 	]+fd cb 08 fd[ 	]+set 7,\(iy\+8\),l
[ ]+b9a:[ 	]+cb 37[       	]+sli a
[ ]+b9c:[ 	]+cb 30[       	]+sli b
[ ]+b9e:[ 	]+cb 31[       	]+sli c
[ ]+ba0:[ 	]+cb 32[       	]+sli d
[ ]+ba2:[ 	]+cb 33[       	]+sli e
[ ]+ba4:[ 	]+cb 34[       	]+sli h
[ ]+ba6:[ 	]+cb 35[       	]+sli l
[ ]+ba8:[ 	]+cb 36[       	]+sli \(hl\)
[ ]+baa:[ 	]+dd cb 07 36[ 	]+sli \(ix\+7\)
[ ]+bae:[ 	]+fd cb f7 36[ 	]+sli \(iy\-9\)
[ ]+bb2:[ 	]+cb 37[       	]+sli a
[ ]+bb4:[ 	]+cb 30[       	]+sli b
[ ]+bb6:[ 	]+cb 31[       	]+sli c
[ ]+bb8:[ 	]+cb 32[       	]+sli d
[ ]+bba:[ 	]+cb 33[       	]+sli e
[ ]+bbc:[ 	]+cb 34[       	]+sli h
[ ]+bbe:[ 	]+cb 35[       	]+sli l
[ ]+bc0:[ 	]+cb 36[       	]+sli \(hl\)
[ ]+bc2:[ 	]+dd cb 07 36[ 	]+sli \(ix\+7\)
[ ]+bc6:[ 	]+fd cb f7 36[ 	]+sli \(iy\-9\)
[ ]+bca:[ 	]+cb 37[       	]+sli a
[ ]+bcc:[ 	]+cb 30[       	]+sli b
[ ]+bce:[ 	]+cb 31[       	]+sli c
[ ]+bd0:[ 	]+cb 32[       	]+sli d
[ ]+bd2:[ 	]+cb 33[       	]+sli e
[ ]+bd4:[ 	]+cb 34[       	]+sli h
[ ]+bd6:[ 	]+cb 35[       	]+sli l
[ ]+bd8:[ 	]+cb 36[       	]+sli \(hl\)
[ ]+bda:[ 	]+dd cb 07 36[ 	]+sli \(ix\+7\)
[ ]+bde:[ 	]+fd cb f7 36[ 	]+sli \(iy\-9\)
[ ]+be2:[ 	]+ed a4[       	]+ldix
[ ]+be4:[ 	]+ed a5[       	]+ldws
[ ]+be6:[ 	]+ed b4[       	]+ldirx
[ ]+be8:[ 	]+ed ac[       	]+lddx
[ ]+bea:[ 	]+ed bc[       	]+lddrx
[ ]+bec:[ 	]+ed b7[       	]+ldpirx
[ ]+bee:[ 	]+ed 90[       	]+outinb
[ ]+bf0:[ 	]+ed 30[       	]+mul d,e
[ ]+bf2:[ 	]+ed 31[       	]+add hl,a
[ ]+bf4:[ 	]+ed 32[       	]+add de,a
[ ]+bf6:[ 	]+ed 33[       	]+add bc,a
[ ]+bf8:[ 	]+ed 34 af be[ 	]+add hl,0xbeaf
[ ]+bfc:[ 	]+ed 35 ad de[ 	]+add de,0xdead
[ ]+c00:[ 	]+ed 36 34 12[ 	]+add bc,0x1234
[ ]+c04:[ 	]+ed 23[       	]+swapnib
[ ]+c06:[ 	]+ed 24[       	]+mirror
[ ]+c08:[ 	]+ed 8a 12 34[ 	]+push 0x1234
[ ]+c0c:[ 	]+ed 91 12 34[ 	]+nextreg 0x12,0x34
[ ]+c10:[ 	]+ed 92 56[    	]+nextreg 0x56,a
[ ]+c13:[ 	]+ed 93[       	]+pixeldn
[ ]+c15:[ 	]+ed 94[       	]+pixelad
[ ]+c17:[ 	]+ed 95[       	]+setae
[ ]+c19:[ 	]+ed 27 78[    	]+test 0x78
[ ]+c1c:[ 	]+ed 28[       	]+bsla de,b
[ ]+c1e:[ 	]+ed 29[       	]+bsra de,b
[ ]+c20:[ 	]+ed 2a[       	]+bsrl de,b
[ ]+c22:[ 	]+ed 2b[       	]+bsrf de,b
[ ]+c24:[ 	]+ed 2c[       	]+bslc de,b
[ ]+c26:[ 	]+ed 98[       	]+jp \(c\)
[ ]+c28:[ 	]+ed 8a 00 00[ 	]+push 0x0000
[ ]+c2c:[ 	]+ed 30[       	]+mul d,e
[ ]+c2e:[ 	]+ed 23[       	]+swapnib
[ ]+c30:[ 	]+ed 27 ab[    	]+test 0xab
#pass
