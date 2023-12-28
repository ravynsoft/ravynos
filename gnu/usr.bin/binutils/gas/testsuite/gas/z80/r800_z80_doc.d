#as: -march=r800
#objdump: -d
#name: All Z80 documented instructions for R800
#source: z80_doc.s

.*: .*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+8e\s+adc a,\(hl\)
\s+1:\s+dd 8e 09\s+adc a,\(ix\+9\)
\s+4:\s+fd 8e 09\s+adc a,\(iy\+9\)
\s+7:\s+ce 03\s+adc a,0x03
\s+9:\s+8f\s+adc a,a
\s+a:\s+88\s+adc a,b
\s+b:\s+89\s+adc a,c
\s+c:\s+8a\s+adc a,d
\s+d:\s+8b\s+adc a,e
\s+e:\s+8c\s+adc a,h
\s+f:\s+8d\s+adc a,l
\s+10:\s+ed 4a\s+adc hl,bc
\s+12:\s+ed 5a\s+adc hl,de
\s+14:\s+ed 6a\s+adc hl,hl
\s+16:\s+ed 7a\s+adc hl,sp
\s+18:\s+86\s+add a,\(hl\)
\s+19:\s+dd 86 09\s+add a,\(ix\+9\)
\s+1c:\s+fd 86 09\s+add a,\(iy\+9\)
\s+1f:\s+c6 03\s+add a,0x03
\s+21:\s+87\s+add a,a
\s+22:\s+80\s+add a,b
\s+23:\s+81\s+add a,c
\s+24:\s+82\s+add a,d
\s+25:\s+83\s+add a,e
\s+26:\s+84\s+add a,h
\s+27:\s+85\s+add a,l
\s+28:\s+09\s+add hl,bc
\s+29:\s+19\s+add hl,de
\s+2a:\s+29\s+add hl,hl
\s+2b:\s+39\s+add hl,sp
\s+2c:\s+dd 09\s+add ix,bc
\s+2e:\s+dd 19\s+add ix,de
\s+30:\s+dd 29\s+add ix,ix
\s+32:\s+dd 39\s+add ix,sp
\s+34:\s+fd 09\s+add iy,bc
\s+36:\s+fd 19\s+add iy,de
\s+38:\s+fd 29\s+add iy,iy
\s+3a:\s+fd 39\s+add iy,sp
\s+3c:\s+a6\s+and \(hl\)
\s+3d:\s+dd a6 09\s+and \(ix\+9\)
\s+40:\s+fd a6 09\s+and \(iy\+9\)
\s+43:\s+e6 03\s+and 0x03
\s+45:\s+a7\s+and a
\s+46:\s+a0\s+and b
\s+47:\s+a1\s+and c
\s+48:\s+a2\s+and d
\s+49:\s+a3\s+and e
\s+4a:\s+a4\s+and h
\s+4b:\s+a5\s+and l
\s+4c:\s+cb 46\s+bit 0,\(hl\)
\s+4e:\s+dd cb 09 46\s+bit 0,\(ix\+9\)
\s+52:\s+fd cb 09 46\s+bit 0,\(iy\+9\)
\s+56:\s+cb 47\s+bit 0,a
\s+58:\s+cb 40\s+bit 0,b
\s+5a:\s+cb 41\s+bit 0,c
\s+5c:\s+cb 42\s+bit 0,d
\s+5e:\s+cb 43\s+bit 0,e
\s+60:\s+cb 44\s+bit 0,h
\s+62:\s+cb 45\s+bit 0,l
\s+64:\s+cb 4e\s+bit 1,\(hl\)
\s+66:\s+dd cb 09 4e\s+bit 1,\(ix\+9\)
\s+6a:\s+fd cb 09 4e\s+bit 1,\(iy\+9\)
\s+6e:\s+cb 4f\s+bit 1,a
\s+70:\s+cb 48\s+bit 1,b
\s+72:\s+cb 49\s+bit 1,c
\s+74:\s+cb 4a\s+bit 1,d
\s+76:\s+cb 4b\s+bit 1,e
\s+78:\s+cb 4c\s+bit 1,h
\s+7a:\s+cb 4d\s+bit 1,l
\s+7c:\s+cb 56\s+bit 2,\(hl\)
\s+7e:\s+dd cb 09 56\s+bit 2,\(ix\+9\)
\s+82:\s+fd cb 09 56\s+bit 2,\(iy\+9\)
\s+86:\s+cb 57\s+bit 2,a
\s+88:\s+cb 50\s+bit 2,b
\s+8a:\s+cb 51\s+bit 2,c
\s+8c:\s+cb 52\s+bit 2,d
\s+8e:\s+cb 53\s+bit 2,e
\s+90:\s+cb 54\s+bit 2,h
\s+92:\s+cb 55\s+bit 2,l
\s+94:\s+cb 5e\s+bit 3,\(hl\)
\s+96:\s+dd cb 09 5e\s+bit 3,\(ix\+9\)
\s+9a:\s+fd cb 09 5e\s+bit 3,\(iy\+9\)
\s+9e:\s+cb 5f\s+bit 3,a
\s+a0:\s+cb 58\s+bit 3,b
\s+a2:\s+cb 59\s+bit 3,c
\s+a4:\s+cb 5a\s+bit 3,d
\s+a6:\s+cb 5b\s+bit 3,e
\s+a8:\s+cb 5c\s+bit 3,h
\s+aa:\s+cb 5d\s+bit 3,l
\s+ac:\s+cb 66\s+bit 4,\(hl\)
\s+ae:\s+dd cb 09 66\s+bit 4,\(ix\+9\)
\s+b2:\s+fd cb 09 66\s+bit 4,\(iy\+9\)
\s+b6:\s+cb 67\s+bit 4,a
\s+b8:\s+cb 60\s+bit 4,b
\s+ba:\s+cb 61\s+bit 4,c
\s+bc:\s+cb 62\s+bit 4,d
\s+be:\s+cb 63\s+bit 4,e
\s+c0:\s+cb 64\s+bit 4,h
\s+c2:\s+cb 65\s+bit 4,l
\s+c4:\s+cb 6e\s+bit 5,\(hl\)
\s+c6:\s+dd cb 09 6e\s+bit 5,\(ix\+9\)
\s+ca:\s+fd cb 09 6e\s+bit 5,\(iy\+9\)
\s+ce:\s+cb 6f\s+bit 5,a
\s+d0:\s+cb 68\s+bit 5,b
\s+d2:\s+cb 69\s+bit 5,c
\s+d4:\s+cb 6a\s+bit 5,d
\s+d6:\s+cb 6b\s+bit 5,e
\s+d8:\s+cb 6c\s+bit 5,h
\s+da:\s+cb 6d\s+bit 5,l
\s+dc:\s+cb 76\s+bit 6,\(hl\)
\s+de:\s+dd cb 09 76\s+bit 6,\(ix\+9\)
\s+e2:\s+fd cb 09 76\s+bit 6,\(iy\+9\)
\s+e6:\s+cb 77\s+bit 6,a
\s+e8:\s+cb 70\s+bit 6,b
\s+ea:\s+cb 71\s+bit 6,c
\s+ec:\s+cb 72\s+bit 6,d
\s+ee:\s+cb 73\s+bit 6,e
\s+f0:\s+cb 74\s+bit 6,h
\s+f2:\s+cb 75\s+bit 6,l
\s+f4:\s+cb 7e\s+bit 7,\(hl\)
\s+f6:\s+dd cb 09 7e\s+bit 7,\(ix\+9\)
\s+fa:\s+fd cb 09 7e\s+bit 7,\(iy\+9\)
\s+fe:\s+cb 7f\s+bit 7,a
\s+100:\s+cb 78\s+bit 7,b
\s+102:\s+cb 79\s+bit 7,c
\s+104:\s+cb 7a\s+bit 7,d
\s+106:\s+cb 7b\s+bit 7,e
\s+108:\s+cb 7c\s+bit 7,h
\s+10a:\s+cb 7d\s+bit 7,l
\s+10c:\s+cd 34 12\s+call 0x1234
\s+10f:\s+dc 34 12\s+call c,0x1234
\s+112:\s+fc 34 12\s+call m,0x1234
\s+115:\s+d4 34 12\s+call nc,0x1234
\s+118:\s+c4 34 12\s+call nz,0x1234
\s+11b:\s+f4 34 12\s+call p,0x1234
\s+11e:\s+ec 34 12\s+call pe,0x1234
\s+121:\s+e4 34 12\s+call po,0x1234
\s+124:\s+cc 34 12\s+call z,0x1234
\s+127:\s+3f\s+ccf
\s+128:\s+be\s+cp \(hl\)
\s+129:\s+dd be 09\s+cp \(ix\+9\)
\s+12c:\s+fd be 09\s+cp \(iy\+9\)
\s+12f:\s+fe 03\s+cp 0x03
\s+131:\s+bf\s+cp a
\s+132:\s+b8\s+cp b
\s+133:\s+b9\s+cp c
\s+134:\s+ba\s+cp d
\s+135:\s+bb\s+cp e
\s+136:\s+bc\s+cp h
\s+137:\s+bd\s+cp l
\s+138:\s+ed a9\s+cpd
\s+13a:\s+ed b9\s+cpdr
\s+13c:\s+ed a1\s+cpi
\s+13e:\s+ed b1\s+cpir
\s+140:\s+2f\s+cpl
\s+141:\s+27\s+daa
\s+142:\s+35\s+dec \(hl\)
\s+143:\s+dd 35 09\s+dec \(ix\+9\)
\s+146:\s+fd 35 09\s+dec \(iy\+9\)
\s+149:\s+3d\s+dec a
\s+14a:\s+05\s+dec b
\s+14b:\s+0b\s+dec bc
\s+14c:\s+0d\s+dec c
\s+14d:\s+15\s+dec d
\s+14e:\s+1b\s+dec de
\s+14f:\s+1d\s+dec e
\s+150:\s+25\s+dec h
\s+151:\s+2b\s+dec hl
\s+152:\s+dd 2b\s+dec ix
\s+154:\s+fd 2b\s+dec iy
\s+156:\s+2d\s+dec l
\s+157:\s+3b\s+dec sp
\s+158:\s+f3\s+di
\s+159:\s+10 05\s+djnz 0x0160
\s+15b:\s+fb\s+ei
\s+15c:\s+e3\s+ex \(sp\),hl
\s+15d:\s+dd e3\s+ex \(sp\),ix
\s+15f:\s+fd e3\s+ex \(sp\),iy
\s+161:\s+08\s+ex af,af'
\s+162:\s+eb\s+ex de,hl
\s+163:\s+d9\s+exx
\s+164:\s+76\s+halt
\s+165:\s+ed 46\s+im 0
\s+167:\s+ed 56\s+im 1
\s+169:\s+ed 5e\s+im 2
\s+16b:\s+ed 78\s+in a,\(c\)
\s+16d:\s+db 03\s+in a,\(0x03\)
\s+16f:\s+ed 40\s+in b,\(c\)
\s+171:\s+ed 48\s+in c,\(c\)
\s+173:\s+ed 50\s+in d,\(c\)
\s+175:\s+ed 58\s+in e,\(c\)
\s+177:\s+ed 60\s+in h,\(c\)
\s+179:\s+ed 68\s+in l,\(c\)
\s+17b:\s+34\s+inc \(hl\)
\s+17c:\s+dd 34 09\s+inc \(ix\+9\)
\s+17f:\s+fd 34 09\s+inc \(iy\+9\)
\s+182:\s+3c\s+inc a
\s+183:\s+04\s+inc b
\s+184:\s+03\s+inc bc
\s+185:\s+0c\s+inc c
\s+186:\s+14\s+inc d
\s+187:\s+13\s+inc de
\s+188:\s+1c\s+inc e
\s+189:\s+24\s+inc h
\s+18a:\s+23\s+inc hl
\s+18b:\s+dd 23\s+inc ix
\s+18d:\s+fd 23\s+inc iy
\s+18f:\s+2c\s+inc l
\s+190:\s+33\s+inc sp
\s+191:\s+ed aa\s+ind
\s+193:\s+ed ba\s+indr
\s+195:\s+ed a2\s+ini
\s+197:\s+ed b2\s+inir
\s+199:\s+e9\s+jp \(hl\)
\s+19a:\s+dd e9\s+jp \(ix\)
\s+19c:\s+fd e9\s+jp \(iy\)
\s+19e:\s+c3 34 12\s+jp 0x1234
\s+1a1:\s+da 34 12\s+jp c,0x1234
\s+1a4:\s+fa 34 12\s+jp m,0x1234
\s+1a7:\s+d2 34 12\s+jp nc,0x1234
\s+1aa:\s+c2 34 12\s+jp nz,0x1234
\s+1ad:\s+f2 34 12\s+jp p,0x1234
\s+1b0:\s+ea 34 12\s+jp pe,0x1234
\s+1b3:\s+e2 34 12\s+jp po,0x1234
\s+1b6:\s+ca 34 12\s+jp z,0x1234
\s+1b9:\s+18 05\s+jr 0x01c0
\s+1bb:\s+38 05\s+jr c,0x01c2
\s+1bd:\s+30 05\s+jr nc,0x01c4
\s+1bf:\s+20 05\s+jr nz,0x01c6
\s+1c1:\s+28 05\s+jr z,0x01c8
\s+1c3:\s+32 34 12\s+ld \(0x1234\),a
\s+1c6:\s+ed 43 34 12\s+ld \(0x1234\),bc
\s+1ca:\s+ed 53 34 12\s+ld \(0x1234\),de
\s+1ce:\s+22 34 12\s+ld \(0x1234\),hl
\s+1d1:\s+dd 22 34 12\s+ld \(0x1234\),ix
\s+1d5:\s+fd 22 34 12\s+ld \(0x1234\),iy
\s+1d9:\s+ed 73 34 12\s+ld \(0x1234\),sp
\s+1dd:\s+02\s+ld \(bc\),a
\s+1de:\s+12\s+ld \(de\),a
\s+1df:\s+36 03\s+ld \(hl\),0x03
\s+1e1:\s+77\s+ld \(hl\),a
\s+1e2:\s+70\s+ld \(hl\),b
\s+1e3:\s+71\s+ld \(hl\),c
\s+1e4:\s+72\s+ld \(hl\),d
\s+1e5:\s+73\s+ld \(hl\),e
\s+1e6:\s+74\s+ld \(hl\),h
\s+1e7:\s+75\s+ld \(hl\),l
\s+1e8:\s+dd 36 09 03\s+ld \(ix\+9\),0x03
\s+1ec:\s+dd 77 09\s+ld \(ix\+9\),a
\s+1ef:\s+dd 70 09\s+ld \(ix\+9\),b
\s+1f2:\s+dd 71 09\s+ld \(ix\+9\),c
\s+1f5:\s+dd 72 09\s+ld \(ix\+9\),d
\s+1f8:\s+dd 73 09\s+ld \(ix\+9\),e
\s+1fb:\s+dd 74 09\s+ld \(ix\+9\),h
\s+1fe:\s+dd 75 09\s+ld \(ix\+9\),l
\s+201:\s+fd 36 09 03\s+ld \(iy\+9\),0x03
\s+205:\s+fd 77 09\s+ld \(iy\+9\),a
\s+208:\s+fd 70 09\s+ld \(iy\+9\),b
\s+20b:\s+fd 71 09\s+ld \(iy\+9\),c
\s+20e:\s+fd 72 09\s+ld \(iy\+9\),d
\s+211:\s+fd 73 09\s+ld \(iy\+9\),e
\s+214:\s+fd 74 09\s+ld \(iy\+9\),h
\s+217:\s+fd 75 09\s+ld \(iy\+9\),l
\s+21a:\s+3a 34 12\s+ld a,\(0x1234\)
\s+21d:\s+0a\s+ld a,\(bc\)
\s+21e:\s+1a\s+ld a,\(de\)
\s+21f:\s+7e\s+ld a,\(hl\)
\s+220:\s+dd 7e 09\s+ld a,\(ix\+9\)
\s+223:\s+fd 7e 09\s+ld a,\(iy\+9\)
\s+226:\s+3e 03\s+ld a,0x03
\s+228:\s+7f\s+ld a,a
\s+229:\s+78\s+ld a,b
\s+22a:\s+79\s+ld a,c
\s+22b:\s+7a\s+ld a,d
\s+22c:\s+7b\s+ld a,e
\s+22d:\s+7c\s+ld a,h
\s+22e:\s+ed 57\s+ld a,i
\s+230:\s+7d\s+ld a,l
\s+231:\s+ed 5f\s+ld a,r
\s+233:\s+46\s+ld b,\(hl\)
\s+234:\s+dd 46 09\s+ld b,\(ix\+9\)
\s+237:\s+fd 46 09\s+ld b,\(iy\+9\)
\s+23a:\s+06 03\s+ld b,0x03
\s+23c:\s+47\s+ld b,a
\s+23d:\s+40\s+ld b,b
\s+23e:\s+41\s+ld b,c
\s+23f:\s+42\s+ld b,d
\s+240:\s+43\s+ld b,e
\s+241:\s+44\s+ld b,h
\s+242:\s+45\s+ld b,l
\s+243:\s+ed 4b 34 12\s+ld bc,\(0x1234\)
\s+247:\s+01 34 12\s+ld bc,0x1234
\s+24a:\s+4e\s+ld c,\(hl\)
\s+24b:\s+dd 4e 09\s+ld c,\(ix\+9\)
\s+24e:\s+fd 4e 09\s+ld c,\(iy\+9\)
\s+251:\s+0e 03\s+ld c,0x03
\s+253:\s+4f\s+ld c,a
\s+254:\s+48\s+ld c,b
\s+255:\s+49\s+ld c,c
\s+256:\s+4a\s+ld c,d
\s+257:\s+4b\s+ld c,e
\s+258:\s+4c\s+ld c,h
\s+259:\s+4d\s+ld c,l
\s+25a:\s+56\s+ld d,\(hl\)
\s+25b:\s+dd 56 09\s+ld d,\(ix\+9\)
\s+25e:\s+fd 56 09\s+ld d,\(iy\+9\)
\s+261:\s+16 03\s+ld d,0x03
\s+263:\s+57\s+ld d,a
\s+264:\s+50\s+ld d,b
\s+265:\s+51\s+ld d,c
\s+266:\s+52\s+ld d,d
\s+267:\s+53\s+ld d,e
\s+268:\s+54\s+ld d,h
\s+269:\s+55\s+ld d,l
\s+26a:\s+ed 5b 34 12\s+ld de,\(0x1234\)
\s+26e:\s+11 34 12\s+ld de,0x1234
\s+271:\s+5e\s+ld e,\(hl\)
\s+272:\s+dd 5e 09\s+ld e,\(ix\+9\)
\s+275:\s+fd 5e 09\s+ld e,\(iy\+9\)
\s+278:\s+1e 03\s+ld e,0x03
\s+27a:\s+5f\s+ld e,a
\s+27b:\s+58\s+ld e,b
\s+27c:\s+59\s+ld e,c
\s+27d:\s+5a\s+ld e,d
\s+27e:\s+5b\s+ld e,e
\s+27f:\s+5c\s+ld e,h
\s+280:\s+5d\s+ld e,l
\s+281:\s+66\s+ld h,\(hl\)
\s+282:\s+dd 66 09\s+ld h,\(ix\+9\)
\s+285:\s+fd 66 09\s+ld h,\(iy\+9\)
\s+288:\s+26 03\s+ld h,0x03
\s+28a:\s+67\s+ld h,a
\s+28b:\s+60\s+ld h,b
\s+28c:\s+61\s+ld h,c
\s+28d:\s+62\s+ld h,d
\s+28e:\s+63\s+ld h,e
\s+28f:\s+64\s+ld h,h
\s+290:\s+65\s+ld h,l
\s+291:\s+2a 34 12\s+ld hl,\(0x1234\)
\s+294:\s+21 34 12\s+ld hl,0x1234
\s+297:\s+ed 47\s+ld i,a
\s+299:\s+dd 2a 34 12\s+ld ix,\(0x1234\)
\s+29d:\s+dd 21 34 12\s+ld ix,0x1234
\s+2a1:\s+fd 2a 34 12\s+ld iy,\(0x1234\)
\s+2a5:\s+fd 21 34 12\s+ld iy,0x1234
\s+2a9:\s+6e\s+ld l,\(hl\)
\s+2aa:\s+dd 6e 09\s+ld l,\(ix\+9\)
\s+2ad:\s+fd 6e 09\s+ld l,\(iy\+9\)
\s+2b0:\s+2e 03\s+ld l,0x03
\s+2b2:\s+6f\s+ld l,a
\s+2b3:\s+68\s+ld l,b
\s+2b4:\s+69\s+ld l,c
\s+2b5:\s+6a\s+ld l,d
\s+2b6:\s+6b\s+ld l,e
\s+2b7:\s+6c\s+ld l,h
\s+2b8:\s+6d\s+ld l,l
\s+2b9:\s+ed 4f\s+ld r,a
\s+2bb:\s+ed 7b 34 12\s+ld sp,\(0x1234\)
\s+2bf:\s+31 34 12\s+ld sp,0x1234
\s+2c2:\s+f9\s+ld sp,hl
\s+2c3:\s+dd f9\s+ld sp,ix
\s+2c5:\s+fd f9\s+ld sp,iy
\s+2c7:\s+ed a8\s+ldd
\s+2c9:\s+ed b8\s+lddr
\s+2cb:\s+ed a0\s+ldi
\s+2cd:\s+ed b0\s+ldir
\s+2cf:\s+ed 44\s+neg
\s+2d1:\s+00\s+nop
\s+2d2:\s+b6\s+or \(hl\)
\s+2d3:\s+dd b6 09\s+or \(ix\+9\)
\s+2d6:\s+fd b6 09\s+or \(iy\+9\)
\s+2d9:\s+f6 03\s+or 0x03
\s+2db:\s+b7\s+or a
\s+2dc:\s+b0\s+or b
\s+2dd:\s+b1\s+or c
\s+2de:\s+b2\s+or d
\s+2df:\s+b3\s+or e
\s+2e0:\s+b4\s+or h
\s+2e1:\s+b5\s+or l
\s+2e2:\s+ed bb\s+otdr
\s+2e4:\s+ed b3\s+otir
\s+2e6:\s+ed 79\s+out \(c\),a
\s+2e8:\s+ed 41\s+out \(c\),b
\s+2ea:\s+ed 49\s+out \(c\),c
\s+2ec:\s+ed 51\s+out \(c\),d
\s+2ee:\s+ed 59\s+out \(c\),e
\s+2f0:\s+ed 61\s+out \(c\),h
\s+2f2:\s+ed 69\s+out \(c\),l
\s+2f4:\s+d3 03\s+out \(0x03\),a
\s+2f6:\s+ed ab\s+outd
\s+2f8:\s+ed a3\s+outi
\s+2fa:\s+f1\s+pop af
\s+2fb:\s+c1\s+pop bc
\s+2fc:\s+d1\s+pop de
\s+2fd:\s+e1\s+pop hl
\s+2fe:\s+dd e1\s+pop ix
\s+300:\s+fd e1\s+pop iy
\s+302:\s+f5\s+push af
\s+303:\s+c5\s+push bc
\s+304:\s+d5\s+push de
\s+305:\s+e5\s+push hl
\s+306:\s+dd e5\s+push ix
\s+308:\s+fd e5\s+push iy
\s+30a:\s+cb 86\s+res 0,\(hl\)
\s+30c:\s+dd cb 09 86\s+res 0,\(ix\+9\)
\s+310:\s+fd cb 09 86\s+res 0,\(iy\+9\)
\s+314:\s+cb 87\s+res 0,a
\s+316:\s+cb 80\s+res 0,b
\s+318:\s+cb 81\s+res 0,c
\s+31a:\s+cb 82\s+res 0,d
\s+31c:\s+cb 83\s+res 0,e
\s+31e:\s+cb 84\s+res 0,h
\s+320:\s+cb 85\s+res 0,l
\s+322:\s+cb 8e\s+res 1,\(hl\)
\s+324:\s+dd cb 09 8e\s+res 1,\(ix\+9\)
\s+328:\s+fd cb 09 8e\s+res 1,\(iy\+9\)
\s+32c:\s+cb 8f\s+res 1,a
\s+32e:\s+cb 88\s+res 1,b
\s+330:\s+cb 89\s+res 1,c
\s+332:\s+cb 8a\s+res 1,d
\s+334:\s+cb 8b\s+res 1,e
\s+336:\s+cb 8c\s+res 1,h
\s+338:\s+cb 8d\s+res 1,l
\s+33a:\s+cb 96\s+res 2,\(hl\)
\s+33c:\s+dd cb 09 96\s+res 2,\(ix\+9\)
\s+340:\s+fd cb 09 96\s+res 2,\(iy\+9\)
\s+344:\s+cb 97\s+res 2,a
\s+346:\s+cb 90\s+res 2,b
\s+348:\s+cb 91\s+res 2,c
\s+34a:\s+cb 92\s+res 2,d
\s+34c:\s+cb 93\s+res 2,e
\s+34e:\s+cb 94\s+res 2,h
\s+350:\s+cb 95\s+res 2,l
\s+352:\s+cb 9e\s+res 3,\(hl\)
\s+354:\s+dd cb 09 9e\s+res 3,\(ix\+9\)
\s+358:\s+fd cb 09 9e\s+res 3,\(iy\+9\)
\s+35c:\s+cb 9f\s+res 3,a
\s+35e:\s+cb 98\s+res 3,b
\s+360:\s+cb 99\s+res 3,c
\s+362:\s+cb 9a\s+res 3,d
\s+364:\s+cb 9b\s+res 3,e
\s+366:\s+cb 9c\s+res 3,h
\s+368:\s+cb 9d\s+res 3,l
\s+36a:\s+cb a6\s+res 4,\(hl\)
\s+36c:\s+dd cb 09 a6\s+res 4,\(ix\+9\)
\s+370:\s+fd cb 09 a6\s+res 4,\(iy\+9\)
\s+374:\s+cb a7\s+res 4,a
\s+376:\s+cb a0\s+res 4,b
\s+378:\s+cb a1\s+res 4,c
\s+37a:\s+cb a2\s+res 4,d
\s+37c:\s+cb a3\s+res 4,e
\s+37e:\s+cb a4\s+res 4,h
\s+380:\s+cb a5\s+res 4,l
\s+382:\s+cb ae\s+res 5,\(hl\)
\s+384:\s+dd cb 09 ae\s+res 5,\(ix\+9\)
\s+388:\s+fd cb 09 ae\s+res 5,\(iy\+9\)
\s+38c:\s+cb af\s+res 5,a
\s+38e:\s+cb a8\s+res 5,b
\s+390:\s+cb a9\s+res 5,c
\s+392:\s+cb aa\s+res 5,d
\s+394:\s+cb ab\s+res 5,e
\s+396:\s+cb ac\s+res 5,h
\s+398:\s+cb ad\s+res 5,l
\s+39a:\s+cb b6\s+res 6,\(hl\)
\s+39c:\s+dd cb 09 b6\s+res 6,\(ix\+9\)
\s+3a0:\s+fd cb 09 b6\s+res 6,\(iy\+9\)
\s+3a4:\s+cb b7\s+res 6,a
\s+3a6:\s+cb b0\s+res 6,b
\s+3a8:\s+cb b1\s+res 6,c
\s+3aa:\s+cb b2\s+res 6,d
\s+3ac:\s+cb b3\s+res 6,e
\s+3ae:\s+cb b4\s+res 6,h
\s+3b0:\s+cb b5\s+res 6,l
\s+3b2:\s+cb be\s+res 7,\(hl\)
\s+3b4:\s+dd cb 09 be\s+res 7,\(ix\+9\)
\s+3b8:\s+fd cb 09 be\s+res 7,\(iy\+9\)
\s+3bc:\s+cb bf\s+res 7,a
\s+3be:\s+cb b8\s+res 7,b
\s+3c0:\s+cb b9\s+res 7,c
\s+3c2:\s+cb ba\s+res 7,d
\s+3c4:\s+cb bb\s+res 7,e
\s+3c6:\s+cb bc\s+res 7,h
\s+3c8:\s+cb bd\s+res 7,l
\s+3ca:\s+c9\s+ret
\s+3cb:\s+d8\s+ret c
\s+3cc:\s+f8\s+ret m
\s+3cd:\s+d0\s+ret nc
\s+3ce:\s+c0\s+ret nz
\s+3cf:\s+f0\s+ret p
\s+3d0:\s+e8\s+ret pe
\s+3d1:\s+e0\s+ret po
\s+3d2:\s+c8\s+ret z
\s+3d3:\s+ed 4d\s+reti
\s+3d5:\s+ed 45\s+retn
\s+3d7:\s+cb 16\s+rl \(hl\)
\s+3d9:\s+dd cb 09 16\s+rl \(ix\+9\)
\s+3dd:\s+fd cb 09 16\s+rl \(iy\+9\)
\s+3e1:\s+cb 17\s+rl a
\s+3e3:\s+cb 10\s+rl b
\s+3e5:\s+cb 11\s+rl c
\s+3e7:\s+cb 12\s+rl d
\s+3e9:\s+cb 13\s+rl e
\s+3eb:\s+cb 14\s+rl h
\s+3ed:\s+cb 15\s+rl l
\s+3ef:\s+17\s+rla
\s+3f0:\s+cb 06\s+rlc \(hl\)
\s+3f2:\s+dd cb 09 06\s+rlc \(ix\+9\)
\s+3f6:\s+fd cb 09 06\s+rlc \(iy\+9\)
\s+3fa:\s+cb 07\s+rlc a
\s+3fc:\s+cb 00\s+rlc b
\s+3fe:\s+cb 01\s+rlc c
\s+400:\s+cb 02\s+rlc d
\s+402:\s+cb 03\s+rlc e
\s+404:\s+cb 04\s+rlc h
\s+406:\s+cb 05\s+rlc l
\s+408:\s+07\s+rlca
\s+409:\s+ed 6f\s+rld
\s+40b:\s+cb 1e\s+rr \(hl\)
\s+40d:\s+dd cb 09 1e\s+rr \(ix\+9\)
\s+411:\s+fd cb 09 1e\s+rr \(iy\+9\)
\s+415:\s+cb 1f\s+rr a
\s+417:\s+cb 18\s+rr b
\s+419:\s+cb 19\s+rr c
\s+41b:\s+cb 1a\s+rr d
\s+41d:\s+cb 1b\s+rr e
\s+41f:\s+cb 1c\s+rr h
\s+421:\s+cb 1d\s+rr l
\s+423:\s+1f\s+rra
\s+424:\s+cb 0e\s+rrc \(hl\)
\s+426:\s+dd cb 09 0e\s+rrc \(ix\+9\)
\s+42a:\s+fd cb 09 0e\s+rrc \(iy\+9\)
\s+42e:\s+cb 0f\s+rrc a
\s+430:\s+cb 08\s+rrc b
\s+432:\s+cb 09\s+rrc c
\s+434:\s+cb 0a\s+rrc d
\s+436:\s+cb 0b\s+rrc e
\s+438:\s+cb 0c\s+rrc h
\s+43a:\s+cb 0d\s+rrc l
\s+43c:\s+0f\s+rrca
\s+43d:\s+ed 67\s+rrd
\s+43f:\s+c7\s+rst 0x00
\s+440:\s+cf\s+rst 0x08
\s+441:\s+d7\s+rst 0x10
\s+442:\s+df\s+rst 0x18
\s+443:\s+e7\s+rst 0x20
\s+444:\s+ef\s+rst 0x28
\s+445:\s+f7\s+rst 0x30
\s+446:\s+ff\s+rst 0x38
\s+447:\s+9e\s+sbc a,\(hl\)
\s+448:\s+dd 9e 09\s+sbc a,\(ix\+9\)
\s+44b:\s+fd 9e 09\s+sbc a,\(iy\+9\)
\s+44e:\s+de 03\s+sbc a,0x03
\s+450:\s+9f\s+sbc a,a
\s+451:\s+98\s+sbc a,b
\s+452:\s+99\s+sbc a,c
\s+453:\s+9a\s+sbc a,d
\s+454:\s+9b\s+sbc a,e
\s+455:\s+9c\s+sbc a,h
\s+456:\s+9d\s+sbc a,l
\s+457:\s+ed 42\s+sbc hl,bc
\s+459:\s+ed 52\s+sbc hl,de
\s+45b:\s+ed 62\s+sbc hl,hl
\s+45d:\s+ed 72\s+sbc hl,sp
\s+45f:\s+37\s+scf
\s+460:\s+cb c6\s+set 0,\(hl\)
\s+462:\s+dd cb 09 c6\s+set 0,\(ix\+9\)
\s+466:\s+fd cb 09 c6\s+set 0,\(iy\+9\)
\s+46a:\s+cb c7\s+set 0,a
\s+46c:\s+cb c0\s+set 0,b
\s+46e:\s+cb c1\s+set 0,c
\s+470:\s+cb c2\s+set 0,d
\s+472:\s+cb c3\s+set 0,e
\s+474:\s+cb c4\s+set 0,h
\s+476:\s+cb c5\s+set 0,l
\s+478:\s+cb ce\s+set 1,\(hl\)
\s+47a:\s+dd cb 09 ce\s+set 1,\(ix\+9\)
\s+47e:\s+fd cb 09 ce\s+set 1,\(iy\+9\)
\s+482:\s+cb cf\s+set 1,a
\s+484:\s+cb c8\s+set 1,b
\s+486:\s+cb c9\s+set 1,c
\s+488:\s+cb ca\s+set 1,d
\s+48a:\s+cb cb\s+set 1,e
\s+48c:\s+cb cc\s+set 1,h
\s+48e:\s+cb cd\s+set 1,l
\s+490:\s+cb d6\s+set 2,\(hl\)
\s+492:\s+dd cb 09 d6\s+set 2,\(ix\+9\)
\s+496:\s+fd cb 09 d6\s+set 2,\(iy\+9\)
\s+49a:\s+cb d7\s+set 2,a
\s+49c:\s+cb d0\s+set 2,b
\s+49e:\s+cb d1\s+set 2,c
\s+4a0:\s+cb d2\s+set 2,d
\s+4a2:\s+cb d3\s+set 2,e
\s+4a4:\s+cb d4\s+set 2,h
\s+4a6:\s+cb d5\s+set 2,l
\s+4a8:\s+cb de\s+set 3,\(hl\)
\s+4aa:\s+dd cb 09 de\s+set 3,\(ix\+9\)
\s+4ae:\s+fd cb 09 de\s+set 3,\(iy\+9\)
\s+4b2:\s+cb df\s+set 3,a
\s+4b4:\s+cb d8\s+set 3,b
\s+4b6:\s+cb d9\s+set 3,c
\s+4b8:\s+cb da\s+set 3,d
\s+4ba:\s+cb db\s+set 3,e
\s+4bc:\s+cb dc\s+set 3,h
\s+4be:\s+cb dd\s+set 3,l
\s+4c0:\s+cb e6\s+set 4,\(hl\)
\s+4c2:\s+dd cb 09 e6\s+set 4,\(ix\+9\)
\s+4c6:\s+fd cb 09 e6\s+set 4,\(iy\+9\)
\s+4ca:\s+cb e7\s+set 4,a
\s+4cc:\s+cb e0\s+set 4,b
\s+4ce:\s+cb e1\s+set 4,c
\s+4d0:\s+cb e2\s+set 4,d
\s+4d2:\s+cb e3\s+set 4,e
\s+4d4:\s+cb e4\s+set 4,h
\s+4d6:\s+cb e5\s+set 4,l
\s+4d8:\s+cb ee\s+set 5,\(hl\)
\s+4da:\s+dd cb 09 ee\s+set 5,\(ix\+9\)
\s+4de:\s+fd cb 09 ee\s+set 5,\(iy\+9\)
\s+4e2:\s+cb ef\s+set 5,a
\s+4e4:\s+cb e8\s+set 5,b
\s+4e6:\s+cb e9\s+set 5,c
\s+4e8:\s+cb ea\s+set 5,d
\s+4ea:\s+cb eb\s+set 5,e
\s+4ec:\s+cb ec\s+set 5,h
\s+4ee:\s+cb ed\s+set 5,l
\s+4f0:\s+cb f6\s+set 6,\(hl\)
\s+4f2:\s+dd cb 09 f6\s+set 6,\(ix\+9\)
\s+4f6:\s+fd cb 09 f6\s+set 6,\(iy\+9\)
\s+4fa:\s+cb f7\s+set 6,a
\s+4fc:\s+cb f0\s+set 6,b
\s+4fe:\s+cb f1\s+set 6,c
\s+500:\s+cb f2\s+set 6,d
\s+502:\s+cb f3\s+set 6,e
\s+504:\s+cb f4\s+set 6,h
\s+506:\s+cb f5\s+set 6,l
\s+508:\s+cb fe\s+set 7,\(hl\)
\s+50a:\s+dd cb 09 fe\s+set 7,\(ix\+9\)
\s+50e:\s+fd cb 09 fe\s+set 7,\(iy\+9\)
\s+512:\s+cb ff\s+set 7,a
\s+514:\s+cb f8\s+set 7,b
\s+516:\s+cb f9\s+set 7,c
\s+518:\s+cb fa\s+set 7,d
\s+51a:\s+cb fb\s+set 7,e
\s+51c:\s+cb fc\s+set 7,h
\s+51e:\s+cb fd\s+set 7,l
\s+520:\s+cb 26\s+sla \(hl\)
\s+522:\s+dd cb 09 26\s+sla \(ix\+9\)
\s+526:\s+fd cb 09 26\s+sla \(iy\+9\)
\s+52a:\s+cb 27\s+sla a
\s+52c:\s+cb 20\s+sla b
\s+52e:\s+cb 21\s+sla c
\s+530:\s+cb 22\s+sla d
\s+532:\s+cb 23\s+sla e
\s+534:\s+cb 24\s+sla h
\s+536:\s+cb 25\s+sla l
\s+538:\s+cb 2e\s+sra \(hl\)
\s+53a:\s+dd cb 09 2e\s+sra \(ix\+9\)
\s+53e:\s+fd cb 09 2e\s+sra \(iy\+9\)
\s+542:\s+cb 2f\s+sra a
\s+544:\s+cb 28\s+sra b
\s+546:\s+cb 29\s+sra c
\s+548:\s+cb 2a\s+sra d
\s+54a:\s+cb 2b\s+sra e
\s+54c:\s+cb 2c\s+sra h
\s+54e:\s+cb 2d\s+sra l
\s+550:\s+cb 3e\s+srl \(hl\)
\s+552:\s+dd cb 09 3e\s+srl \(ix\+9\)
\s+556:\s+fd cb 09 3e\s+srl \(iy\+9\)
\s+55a:\s+cb 3f\s+srl a
\s+55c:\s+cb 38\s+srl b
\s+55e:\s+cb 39\s+srl c
\s+560:\s+cb 3a\s+srl d
\s+562:\s+cb 3b\s+srl e
\s+564:\s+cb 3c\s+srl h
\s+566:\s+cb 3d\s+srl l
\s+568:\s+96\s+sub \(hl\)
\s+569:\s+dd 96 09\s+sub \(ix\+9\)
\s+56c:\s+fd 96 09\s+sub \(iy\+9\)
\s+56f:\s+d6 03\s+sub 0x03
\s+571:\s+97\s+sub a
\s+572:\s+90\s+sub b
\s+573:\s+91\s+sub c
\s+574:\s+92\s+sub d
\s+575:\s+93\s+sub e
\s+576:\s+94\s+sub h
\s+577:\s+95\s+sub l
\s+578:\s+ae\s+xor \(hl\)
\s+579:\s+dd ae 09\s+xor \(ix\+9\)
\s+57c:\s+fd ae 09\s+xor \(iy\+9\)
\s+57f:\s+ee 03\s+xor 0x03
\s+581:\s+af\s+xor a
\s+582:\s+a8\s+xor b
\s+583:\s+a9\s+xor c
\s+584:\s+aa\s+xor d
\s+585:\s+ab\s+xor e
\s+586:\s+ac\s+xor h
\s+587:\s+ad\s+xor l
