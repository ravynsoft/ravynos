#as: -march=ez80+adl
#objdump: -d
#name: All eZ80 instructions in ADL cpu mode

.*:.*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+a7\s+and a,a
\s+1:\s+a0\s+and a,b
\s+2:\s+a1\s+and a,c
\s+3:\s+a2\s+and a,d
\s+4:\s+a3\s+and a,e
\s+5:\s+a4\s+and a,h
\s+6:\s+a5\s+and a,l
\s+7:\s+a6\s+and a,\(hl\)
\s+8:\s+e6 aa\s+and a,0xaa
\s+a:\s+dd a6 05\s+and a,\(ix\+5\)
\s+d:\s+fd a6 fb\s+and a,\(iy\-5\)
\s+10:\s+bf\s+cp a,a
\s+11:\s+b8\s+cp a,b
\s+12:\s+b9\s+cp a,c
\s+13:\s+ba\s+cp a,d
\s+14:\s+bb\s+cp a,e
\s+15:\s+bc\s+cp a,h
\s+16:\s+bd\s+cp a,l
\s+17:\s+be\s+cp a,\(hl\)
\s+18:\s+fe aa\s+cp a,0xaa
\s+1a:\s+dd be 05\s+cp a,\(ix\+5\)
\s+1d:\s+fd be fb\s+cp a,\(iy\-5\)
\s+20:\s+b7\s+or a,a
\s+21:\s+b0\s+or a,b
\s+22:\s+b1\s+or a,c
\s+23:\s+b2\s+or a,d
\s+24:\s+b3\s+or a,e
\s+25:\s+b4\s+or a,h
\s+26:\s+b5\s+or a,l
\s+27:\s+b6\s+or a,\(hl\)
\s+28:\s+f6 aa\s+or a,0xaa
\s+2a:\s+dd b6 05\s+or a,\(ix\+5\)
\s+2d:\s+fd b6 fb\s+or a,\(iy\-5\)
\s+30:\s+97\s+sub a,a
\s+31:\s+90\s+sub a,b
\s+32:\s+91\s+sub a,c
\s+33:\s+92\s+sub a,d
\s+34:\s+93\s+sub a,e
\s+35:\s+94\s+sub a,h
\s+36:\s+95\s+sub a,l
\s+37:\s+96\s+sub a,\(hl\)
\s+38:\s+d6 aa\s+sub a,0xaa
\s+3a:\s+dd 96 05\s+sub a,\(ix\+5\)
\s+3d:\s+fd 96 fb\s+sub a,\(iy\-5\)
\s+40:\s+ed 3c\s+tst a,a
\s+42:\s+ed 04\s+tst a,b
\s+44:\s+ed 0c\s+tst a,c
\s+46:\s+ed 14\s+tst a,d
\s+48:\s+ed 1c\s+tst a,e
\s+4a:\s+ed 24\s+tst a,h
\s+4c:\s+ed 2c\s+tst a,l
\s+4e:\s+ed 34\s+tst a,\(hl\)
\s+50:\s+ed 64 0f\s+tst a,0x0f
\s+53:\s+af\s+xor a,a
\s+54:\s+a8\s+xor a,b
\s+55:\s+a9\s+xor a,c
\s+56:\s+aa\s+xor a,d
\s+57:\s+ab\s+xor a,e
\s+58:\s+ac\s+xor a,h
\s+59:\s+ad\s+xor a,l
\s+5a:\s+ae\s+xor a,\(hl\)
\s+5b:\s+ee aa\s+xor a,0xaa
\s+5d:\s+dd ae 05\s+xor a,\(ix\+5\)
\s+60:\s+fd ae fb\s+xor a,\(iy\-5\)
\s+63:\s+ed 78\s+in a,\(bc\)
\s+65:\s+ed 40\s+in b,\(bc\)
\s+67:\s+ed 48\s+in c,\(bc\)
\s+69:\s+ed 50\s+in d,\(bc\)
\s+6b:\s+ed 58\s+in e,\(bc\)
\s+6d:\s+ed 60\s+in h,\(bc\)
\s+6f:\s+ed 68\s+in l,\(bc\)
\s+71:\s+ed 79\s+out \(bc\),a
\s+73:\s+ed 41\s+out \(bc\),b
\s+75:\s+ed 49\s+out \(bc\),c
\s+77:\s+ed 51\s+out \(bc\),d
\s+79:\s+ed 59\s+out \(bc\),e
\s+7b:\s+ed 61\s+out \(bc\),h
\s+7d:\s+ed 69\s+out \(bc\),l
\s+7f:\s+dd 07 f9\s+ld bc,\(ix\-7\)
\s+82:\s+dd 17 f9\s+ld de,\(ix\-7\)
\s+85:\s+dd 27 f9\s+ld hl,\(ix\-7\)
\s+88:\s+dd 37 f9\s+ld ix,\(ix\-7\)
\s+8b:\s+dd 31 f9\s+ld iy,\(ix\-7\)
\s+8e:\s+fd 07 26\s+ld bc,\(iy\+38\)
\s+91:\s+fd 17 26\s+ld de,\(iy\+38\)
\s+94:\s+fd 27 26\s+ld hl,\(iy\+38\)
\s+97:\s+fd 31 26\s+ld ix,\(iy\+38\)
\s+9a:\s+fd 37 26\s+ld iy,\(iy\+38\)
\s+9d:\s+dd 0f 7e\s+ld \(ix\+126\),bc
\s+a0:\s+dd 1f 7e\s+ld \(ix\+126\),de
\s+a3:\s+dd 2f 7e\s+ld \(ix\+126\),hl
\s+a6:\s+dd 3f 7e\s+ld \(ix\+126\),ix
\s+a9:\s+dd 3e 7e\s+ld \(ix\+126\),iy
\s+ac:\s+fd 0f 9e\s+ld \(iy\-98\),bc
\s+af:\s+fd 1f 9e\s+ld \(iy\-98\),de
\s+b2:\s+fd 2f 9e\s+ld \(iy\-98\),hl
\s+b5:\s+fd 3e 9e\s+ld \(iy\-98\),ix
\s+b8:\s+fd 3f 9e\s+ld \(iy\-98\),iy
\s+bb:\s+ed 02 e5\s+lea bc,ix\-27
\s+be:\s+ed 12 e5\s+lea de,ix\-27
\s+c1:\s+ed 22 e5\s+lea hl,ix\-27
\s+c4:\s+ed 32 e5\s+lea ix,ix\-27
\s+c7:\s+ed 55 e5\s+lea iy,ix\-27
\s+ca:\s+ed 03 0c\s+lea bc,iy\+12
\s+cd:\s+ed 13 0c\s+lea de,iy\+12
\s+d0:\s+ed 23 0c\s+lea hl,iy\+12
\s+d3:\s+ed 54 0c\s+lea ix,iy\+12
\s+d6:\s+ed 33 0c\s+lea iy,iy\+12
\s+d9:\s+ed 65 7f\s+pea ix\+127
\s+dc:\s+ed 66 80\s+pea iy\-128
\s+df:\s+ed 38 05\s+in0 a,\(0x05\)
\s+e2:\s+ed 00 05\s+in0 b,\(0x05\)
\s+e5:\s+ed 08 05\s+in0 c,\(0x05\)
\s+e8:\s+ed 10 05\s+in0 d,\(0x05\)
\s+eb:\s+ed 18 05\s+in0 e,\(0x05\)
\s+ee:\s+ed 20 05\s+in0 h,\(0x05\)
\s+f1:\s+ed 28 05\s+in0 l,\(0x05\)
\s+f4:\s+ed 39 05\s+out0 \(0x05\),a
\s+f7:\s+ed 01 05\s+out0 \(0x05\),b
\s+fa:\s+ed 09 05\s+out0 \(0x05\),c
\s+fd:\s+ed 11 05\s+out0 \(0x05\),d
\s+100:\s+ed 19 05\s+out0 \(0x05\),e
\s+103:\s+ed 21 05\s+out0 \(0x05\),h
\s+106:\s+ed 29 05\s+out0 \(0x05\),l
\s+109:\s+ed 4c\s+mlt bc
\s+10b:\s+ed 5c\s+mlt de
\s+10d:\s+ed 6c\s+mlt hl
\s+10f:\s+ed 7c\s+mlt sp
\s+111:\s+ed 74 f0\s+tstio 0xf0
\s+114:\s+ed 76\s+slp
\s+116:\s+ed 7d\s+stmix
\s+118:\s+ed 7e\s+rsmix
\s+11a:\s+ed 82\s+inim
\s+11c:\s+ed 83\s+otim
\s+11e:\s+ed 84\s+ini2
\s+120:\s+ed 8a\s+indm
\s+122:\s+ed 8b\s+otdm
\s+124:\s+ed 8c\s+ind2
\s+126:\s+ed 92\s+inimr
\s+128:\s+ed 93\s+otimr
\s+12a:\s+ed 94\s+ini2r
\s+12c:\s+ed 9a\s+indmr
\s+12e:\s+ed 9b\s+otdmr
\s+130:\s+ed 9c\s+ind2r
\s+132:\s+ed a4\s+outi2
\s+134:\s+ed ac\s+outd2
\s+136:\s+ed b4\s+oti2r
\s+138:\s+ed bc\s+otd2r
\s+13a:\s+ed c2\s+inirx
\s+13c:\s+ed c3\s+otirx
\s+13e:\s+ed ca\s+indrx
\s+140:\s+ed cb\s+otdrx
\s+142:\s+dd 7c\s+ld a,ixh
\s+144:\s+dd 44\s+ld b,ixh
\s+146:\s+dd 4c\s+ld c,ixh
\s+148:\s+dd 54\s+ld d,ixh
\s+14a:\s+dd 5c\s+ld e,ixh
\s+14c:\s+dd 64\s+ld ixh,ixh
\s+14e:\s+dd 6c\s+ld ixl,ixh
\s+150:\s+dd 7d\s+ld a,ixl
\s+152:\s+dd 45\s+ld b,ixl
\s+154:\s+dd 4d\s+ld c,ixl
\s+156:\s+dd 55\s+ld d,ixl
\s+158:\s+dd 5d\s+ld e,ixl
\s+15a:\s+dd 65\s+ld ixh,ixl
\s+15c:\s+dd 6d\s+ld ixl,ixl
\s+15e:\s+fd 7c\s+ld a,iyh
\s+160:\s+fd 44\s+ld b,iyh
\s+162:\s+fd 4c\s+ld c,iyh
\s+164:\s+fd 54\s+ld d,iyh
\s+166:\s+fd 5c\s+ld e,iyh
\s+168:\s+fd 64\s+ld iyh,iyh
\s+16a:\s+fd 6c\s+ld iyl,iyh
\s+16c:\s+fd 7d\s+ld a,iyl
\s+16e:\s+fd 45\s+ld b,iyl
\s+170:\s+fd 4d\s+ld c,iyl
\s+172:\s+fd 55\s+ld d,iyl
\s+174:\s+fd 5d\s+ld e,iyl
\s+176:\s+fd 65\s+ld iyh,iyl
\s+178:\s+fd 6d\s+ld iyl,iyl
\s+17a:\s+dd 67\s+ld ixh,a
\s+17c:\s+dd 60\s+ld ixh,b
\s+17e:\s+dd 61\s+ld ixh,c
\s+180:\s+dd 62\s+ld ixh,d
\s+182:\s+dd 63\s+ld ixh,e
\s+184:\s+dd 64\s+ld ixh,ixh
\s+186:\s+dd 65\s+ld ixh,ixl
\s+188:\s+dd 26 19\s+ld ixh,0x19
\s+18b:\s+dd 6f\s+ld ixl,a
\s+18d:\s+dd 68\s+ld ixl,b
\s+18f:\s+dd 69\s+ld ixl,c
\s+191:\s+dd 6a\s+ld ixl,d
\s+193:\s+dd 6b\s+ld ixl,e
\s+195:\s+dd 6c\s+ld ixl,ixh
\s+197:\s+dd 6d\s+ld ixl,ixl
\s+199:\s+dd 2e 19\s+ld ixl,0x19
\s+19c:\s+fd 67\s+ld iyh,a
\s+19e:\s+fd 60\s+ld iyh,b
\s+1a0:\s+fd 61\s+ld iyh,c
\s+1a2:\s+fd 62\s+ld iyh,d
\s+1a4:\s+fd 63\s+ld iyh,e
\s+1a6:\s+fd 64\s+ld iyh,iyh
\s+1a8:\s+fd 65\s+ld iyh,iyl
\s+1aa:\s+fd 26 19\s+ld iyh,0x19
\s+1ad:\s+fd 6f\s+ld iyl,a
\s+1af:\s+fd 68\s+ld iyl,b
\s+1b1:\s+fd 69\s+ld iyl,c
\s+1b3:\s+fd 6a\s+ld iyl,d
\s+1b5:\s+fd 6b\s+ld iyl,e
\s+1b7:\s+fd 6c\s+ld iyl,iyh
\s+1b9:\s+fd 6d\s+ld iyl,iyl
\s+1bb:\s+fd 2e 19\s+ld iyl,0x19
\s+1be:\s+dd 84\s+add a,ixh
\s+1c0:\s+dd 85\s+add a,ixl
\s+1c2:\s+fd 84\s+add a,iyh
\s+1c4:\s+fd 85\s+add a,iyl
\s+1c6:\s+dd 8c\s+adc a,ixh
\s+1c8:\s+dd 8d\s+adc a,ixl
\s+1ca:\s+fd 8c\s+adc a,iyh
\s+1cc:\s+fd 8d\s+adc a,iyl
\s+1ce:\s+dd bc\s+cp a,ixh
\s+1d0:\s+dd bd\s+cp a,ixl
\s+1d2:\s+fd bc\s+cp a,iyh
\s+1d4:\s+fd bd\s+cp a,iyl
\s+1d6:\s+dd 25\s+dec ixh
\s+1d8:\s+dd 2d\s+dec ixl
\s+1da:\s+fd 25\s+dec iyh
\s+1dc:\s+fd 2d\s+dec iyl
\s+1de:\s+dd 24\s+inc ixh
\s+1e0:\s+dd 2c\s+inc ixl
\s+1e2:\s+fd 24\s+inc iyh
\s+1e4:\s+fd 2c\s+inc iyl
\s+1e6:\s+dd 9c\s+sbc a,ixh
\s+1e8:\s+dd 9d\s+sbc a,ixl
\s+1ea:\s+fd 9c\s+sbc a,iyh
\s+1ec:\s+fd 9d\s+sbc a,iyl
\s+1ee:\s+dd 94\s+sub a,ixh
\s+1f0:\s+dd 95\s+sub a,ixl
\s+1f2:\s+fd 94\s+sub a,iyh
\s+1f4:\s+fd 95\s+sub a,iyl
\s+1f6:\s+dd a4\s+and a,ixh
\s+1f8:\s+dd a5\s+and a,ixl
\s+1fa:\s+fd a4\s+and a,iyh
\s+1fc:\s+fd a5\s+and a,iyl
\s+1fe:\s+dd b4\s+or a,ixh
\s+200:\s+dd b5\s+or a,ixl
\s+202:\s+fd b4\s+or a,iyh
\s+204:\s+fd b5\s+or a,iyl
\s+206:\s+dd ac\s+xor a,ixh
\s+208:\s+dd ad\s+xor a,ixl
\s+20a:\s+fd ac\s+xor a,iyh
\s+20c:\s+fd ad\s+xor a,iyl
\s+20e:\s+ed c7\s+ld i,hl
\s+210:\s+ed d7\s+ld hl,i
\s+212:\s+ed 6d\s+ld mb,a
\s+214:\s+ed 6e\s+ld a,mb
\s+216:\s+8e\s+adc a,\(hl\)
\s+217:\s+dd 8e 09\s+adc a,\(ix\+9\)
\s+21a:\s+fd 8e 09\s+adc a,\(iy\+9\)
\s+21d:\s+ce 03\s+adc a,0x03
\s+21f:\s+8f\s+adc a,a
\s+220:\s+88\s+adc a,b
\s+221:\s+89\s+adc a,c
\s+222:\s+8a\s+adc a,d
\s+223:\s+8b\s+adc a,e
\s+224:\s+8c\s+adc a,h
\s+225:\s+8d\s+adc a,l
\s+226:\s+ed 4a\s+adc hl,bc
\s+228:\s+ed 5a\s+adc hl,de
\s+22a:\s+ed 6a\s+adc hl,hl
\s+22c:\s+ed 7a\s+adc hl,sp
\s+22e:\s+86\s+add a,\(hl\)
\s+22f:\s+dd 86 09\s+add a,\(ix\+9\)
\s+232:\s+fd 86 09\s+add a,\(iy\+9\)
\s+235:\s+c6 03\s+add a,0x03
\s+237:\s+87\s+add a,a
\s+238:\s+80\s+add a,b
\s+239:\s+81\s+add a,c
\s+23a:\s+82\s+add a,d
\s+23b:\s+83\s+add a,e
\s+23c:\s+84\s+add a,h
\s+23d:\s+85\s+add a,l
\s+23e:\s+09\s+add hl,bc
\s+23f:\s+19\s+add hl,de
\s+240:\s+29\s+add hl,hl
\s+241:\s+39\s+add hl,sp
\s+242:\s+dd 09\s+add ix,bc
\s+244:\s+dd 19\s+add ix,de
\s+246:\s+dd 29\s+add ix,ix
\s+248:\s+dd 39\s+add ix,sp
\s+24a:\s+fd 09\s+add iy,bc
\s+24c:\s+fd 19\s+add iy,de
\s+24e:\s+fd 29\s+add iy,iy
\s+250:\s+fd 39\s+add iy,sp
\s+252:\s+a6\s+and a,\(hl\)
\s+253:\s+dd a6 09\s+and a,\(ix\+9\)
\s+256:\s+fd a6 09\s+and a,\(iy\+9\)
\s+259:\s+e6 03\s+and a,0x03
\s+25b:\s+a7\s+and a,a
\s+25c:\s+a0\s+and a,b
\s+25d:\s+a1\s+and a,c
\s+25e:\s+a2\s+and a,d
\s+25f:\s+a3\s+and a,e
\s+260:\s+a4\s+and a,h
\s+261:\s+a5\s+and a,l
\s+262:\s+cb 46\s+bit 0,\(hl\)
\s+264:\s+dd cb 09 46\s+bit 0,\(ix\+9\)
\s+268:\s+fd cb 09 46\s+bit 0,\(iy\+9\)
\s+26c:\s+cb 47\s+bit 0,a
\s+26e:\s+cb 40\s+bit 0,b
\s+270:\s+cb 41\s+bit 0,c
\s+272:\s+cb 42\s+bit 0,d
\s+274:\s+cb 43\s+bit 0,e
\s+276:\s+cb 44\s+bit 0,h
\s+278:\s+cb 45\s+bit 0,l
\s+27a:\s+cb 4e\s+bit 1,\(hl\)
\s+27c:\s+dd cb 09 4e\s+bit 1,\(ix\+9\)
\s+280:\s+fd cb 09 4e\s+bit 1,\(iy\+9\)
\s+284:\s+cb 4f\s+bit 1,a
\s+286:\s+cb 48\s+bit 1,b
\s+288:\s+cb 49\s+bit 1,c
\s+28a:\s+cb 4a\s+bit 1,d
\s+28c:\s+cb 4b\s+bit 1,e
\s+28e:\s+cb 4c\s+bit 1,h
\s+290:\s+cb 4d\s+bit 1,l
\s+292:\s+cb 56\s+bit 2,\(hl\)
\s+294:\s+dd cb 09 56\s+bit 2,\(ix\+9\)
\s+298:\s+fd cb 09 56\s+bit 2,\(iy\+9\)
\s+29c:\s+cb 57\s+bit 2,a
\s+29e:\s+cb 50\s+bit 2,b
\s+2a0:\s+cb 51\s+bit 2,c
\s+2a2:\s+cb 52\s+bit 2,d
\s+2a4:\s+cb 53\s+bit 2,e
\s+2a6:\s+cb 54\s+bit 2,h
\s+2a8:\s+cb 55\s+bit 2,l
\s+2aa:\s+cb 5e\s+bit 3,\(hl\)
\s+2ac:\s+dd cb 09 5e\s+bit 3,\(ix\+9\)
\s+2b0:\s+fd cb 09 5e\s+bit 3,\(iy\+9\)
\s+2b4:\s+cb 5f\s+bit 3,a
\s+2b6:\s+cb 58\s+bit 3,b
\s+2b8:\s+cb 59\s+bit 3,c
\s+2ba:\s+cb 5a\s+bit 3,d
\s+2bc:\s+cb 5b\s+bit 3,e
\s+2be:\s+cb 5c\s+bit 3,h
\s+2c0:\s+cb 5d\s+bit 3,l
\s+2c2:\s+cb 66\s+bit 4,\(hl\)
\s+2c4:\s+dd cb 09 66\s+bit 4,\(ix\+9\)
\s+2c8:\s+fd cb 09 66\s+bit 4,\(iy\+9\)
\s+2cc:\s+cb 67\s+bit 4,a
\s+2ce:\s+cb 60\s+bit 4,b
\s+2d0:\s+cb 61\s+bit 4,c
\s+2d2:\s+cb 62\s+bit 4,d
\s+2d4:\s+cb 63\s+bit 4,e
\s+2d6:\s+cb 64\s+bit 4,h
\s+2d8:\s+cb 65\s+bit 4,l
\s+2da:\s+cb 6e\s+bit 5,\(hl\)
\s+2dc:\s+dd cb 09 6e\s+bit 5,\(ix\+9\)
\s+2e0:\s+fd cb 09 6e\s+bit 5,\(iy\+9\)
\s+2e4:\s+cb 6f\s+bit 5,a
\s+2e6:\s+cb 68\s+bit 5,b
\s+2e8:\s+cb 69\s+bit 5,c
\s+2ea:\s+cb 6a\s+bit 5,d
\s+2ec:\s+cb 6b\s+bit 5,e
\s+2ee:\s+cb 6c\s+bit 5,h
\s+2f0:\s+cb 6d\s+bit 5,l
\s+2f2:\s+cb 76\s+bit 6,\(hl\)
\s+2f4:\s+dd cb 09 76\s+bit 6,\(ix\+9\)
\s+2f8:\s+fd cb 09 76\s+bit 6,\(iy\+9\)
\s+2fc:\s+cb 77\s+bit 6,a
\s+2fe:\s+cb 70\s+bit 6,b
\s+300:\s+cb 71\s+bit 6,c
\s+302:\s+cb 72\s+bit 6,d
\s+304:\s+cb 73\s+bit 6,e
\s+306:\s+cb 74\s+bit 6,h
\s+308:\s+cb 75\s+bit 6,l
\s+30a:\s+cb 7e\s+bit 7,\(hl\)
\s+30c:\s+dd cb 09 7e\s+bit 7,\(ix\+9\)
\s+310:\s+fd cb 09 7e\s+bit 7,\(iy\+9\)
\s+314:\s+cb 7f\s+bit 7,a
\s+316:\s+cb 78\s+bit 7,b
\s+318:\s+cb 79\s+bit 7,c
\s+31a:\s+cb 7a\s+bit 7,d
\s+31c:\s+cb 7b\s+bit 7,e
\s+31e:\s+cb 7c\s+bit 7,h
\s+320:\s+cb 7d\s+bit 7,l
\s+322:\s+cd 56 34 12\s+call 0x123456
\s+326:\s+dc 56 34 12\s+call c,0x123456
\s+32a:\s+fc 56 34 12\s+call m,0x123456
\s+32e:\s+d4 56 34 12\s+call nc,0x123456
\s+332:\s+c4 56 34 12\s+call nz,0x123456
\s+336:\s+f4 56 34 12\s+call p,0x123456
\s+33a:\s+ec 56 34 12\s+call pe,0x123456
\s+33e:\s+e4 56 34 12\s+call po,0x123456
\s+342:\s+cc 56 34 12\s+call z,0x123456
\s+346:\s+3f\s+ccf
\s+347:\s+be\s+cp a,\(hl\)
\s+348:\s+dd be 09\s+cp a,\(ix\+9\)
\s+34b:\s+fd be 09\s+cp a,\(iy\+9\)
\s+34e:\s+fe 03\s+cp a,0x03
\s+350:\s+bf\s+cp a,a
\s+351:\s+b8\s+cp a,b
\s+352:\s+b9\s+cp a,c
\s+353:\s+ba\s+cp a,d
\s+354:\s+bb\s+cp a,e
\s+355:\s+bc\s+cp a,h
\s+356:\s+bd\s+cp a,l
\s+357:\s+ed a9\s+cpd
\s+359:\s+ed b9\s+cpdr
\s+35b:\s+ed a1\s+cpi
\s+35d:\s+ed b1\s+cpir
\s+35f:\s+2f\s+cpl
\s+360:\s+27\s+daa
\s+361:\s+35\s+dec \(hl\)
\s+362:\s+dd 35 09\s+dec \(ix\+9\)
\s+365:\s+fd 35 09\s+dec \(iy\+9\)
\s+368:\s+3d\s+dec a
\s+369:\s+05\s+dec b
\s+36a:\s+0b\s+dec bc
\s+36b:\s+0d\s+dec c
\s+36c:\s+15\s+dec d
\s+36d:\s+1b\s+dec de
\s+36e:\s+1d\s+dec e
\s+36f:\s+25\s+dec h
\s+370:\s+2b\s+dec hl
\s+371:\s+dd 2b\s+dec ix
\s+373:\s+fd 2b\s+dec iy
\s+375:\s+2d\s+dec l
\s+376:\s+3b\s+dec sp
\s+377:\s+f3\s+di
\s+378:\s+10 05\s+djnz 0x037f
\s+37a:\s+fb\s+ei
\s+37b:\s+e3\s+ex \(sp\),hl
\s+37c:\s+dd e3\s+ex \(sp\),ix
\s+37e:\s+fd e3\s+ex \(sp\),iy
\s+380:\s+08\s+ex af,af'
\s+381:\s+eb\s+ex de,hl
\s+382:\s+d9\s+exx
\s+383:\s+76\s+halt
\s+384:\s+ed 46\s+im 0
\s+386:\s+ed 56\s+im 1
\s+388:\s+ed 5e\s+im 2
\s+38a:\s+ed 78\s+in a,\(bc\)
\s+38c:\s+db 03\s+in a,\(0x03\)
\s+38e:\s+ed 40\s+in b,\(bc\)
\s+390:\s+ed 48\s+in c,\(bc\)
\s+392:\s+ed 50\s+in d,\(bc\)
\s+394:\s+ed 58\s+in e,\(bc\)
\s+396:\s+ed 60\s+in h,\(bc\)
\s+398:\s+ed 68\s+in l,\(bc\)
\s+39a:\s+34\s+inc \(hl\)
\s+39b:\s+dd 34 09\s+inc \(ix\+9\)
\s+39e:\s+fd 34 09\s+inc \(iy\+9\)
\s+3a1:\s+3c\s+inc a
\s+3a2:\s+04\s+inc b
\s+3a3:\s+03\s+inc bc
\s+3a4:\s+0c\s+inc c
\s+3a5:\s+14\s+inc d
\s+3a6:\s+13\s+inc de
\s+3a7:\s+1c\s+inc e
\s+3a8:\s+24\s+inc h
\s+3a9:\s+23\s+inc hl
\s+3aa:\s+dd 23\s+inc ix
\s+3ac:\s+fd 23\s+inc iy
\s+3ae:\s+2c\s+inc l
\s+3af:\s+33\s+inc sp
\s+3b0:\s+ed aa\s+ind
\s+3b2:\s+ed ba\s+indr
\s+3b4:\s+ed a2\s+ini
\s+3b6:\s+ed b2\s+inir
\s+3b8:\s+e9\s+jp \(hl\)
\s+3b9:\s+dd e9\s+jp \(ix\)
\s+3bb:\s+fd e9\s+jp \(iy\)
\s+3bd:\s+c3 56 34 12\s+jp 0x123456
\s+3c1:\s+da 56 34 12\s+jp c,0x123456
\s+3c5:\s+fa 56 34 12\s+jp m,0x123456
\s+3c9:\s+d2 56 34 12\s+jp nc,0x123456
\s+3cd:\s+c2 56 34 12\s+jp nz,0x123456
\s+3d1:\s+f2 56 34 12\s+jp p,0x123456
\s+3d5:\s+ea 56 34 12\s+jp pe,0x123456
\s+3d9:\s+e2 56 34 12\s+jp po,0x123456
\s+3dd:\s+ca 56 34 12\s+jp z,0x123456
\s+3e1:\s+18 05\s+jr 0x03e8
\s+3e3:\s+38 05\s+jr c,0x03ea
\s+3e5:\s+30 05\s+jr nc,0x03ec
\s+3e7:\s+20 05\s+jr nz,0x03ee
\s+3e9:\s+28 05\s+jr z,0x03f0
\s+3eb:\s+32 56 34 12\s+ld \(0x123456\),a
\s+3ef:\s+ed 43 56 34 12\s+ld \(0x123456\),bc
\s+3f4:\s+ed 53 56 34 12\s+ld \(0x123456\),de
\s+3f9:\s+22 56 34 12\s+ld \(0x123456\),hl
\s+3fd:\s+dd 22 56 34 12\s+ld \(0x123456\),ix
\s+402:\s+fd 22 56 34 12\s+ld \(0x123456\),iy
\s+407:\s+ed 73 56 34 12\s+ld \(0x123456\),sp
\s+40c:\s+02\s+ld \(bc\),a
\s+40d:\s+12\s+ld \(de\),a
\s+40e:\s+36 03\s+ld \(hl\),0x03
\s+410:\s+77\s+ld \(hl\),a
\s+411:\s+70\s+ld \(hl\),b
\s+412:\s+71\s+ld \(hl\),c
\s+413:\s+72\s+ld \(hl\),d
\s+414:\s+73\s+ld \(hl\),e
\s+415:\s+74\s+ld \(hl\),h
\s+416:\s+75\s+ld \(hl\),l
\s+417:\s+dd 36 09 03\s+ld \(ix\+9\),0x03
\s+41b:\s+dd 77 09\s+ld \(ix\+9\),a
\s+41e:\s+dd 70 09\s+ld \(ix\+9\),b
\s+421:\s+dd 71 09\s+ld \(ix\+9\),c
\s+424:\s+dd 72 09\s+ld \(ix\+9\),d
\s+427:\s+dd 73 09\s+ld \(ix\+9\),e
\s+42a:\s+dd 74 09\s+ld \(ix\+9\),h
\s+42d:\s+dd 75 09\s+ld \(ix\+9\),l
\s+430:\s+fd 36 09 03\s+ld \(iy\+9\),0x03
\s+434:\s+fd 77 09\s+ld \(iy\+9\),a
\s+437:\s+fd 70 09\s+ld \(iy\+9\),b
\s+43a:\s+fd 71 09\s+ld \(iy\+9\),c
\s+43d:\s+fd 72 09\s+ld \(iy\+9\),d
\s+440:\s+fd 73 09\s+ld \(iy\+9\),e
\s+443:\s+fd 74 09\s+ld \(iy\+9\),h
\s+446:\s+fd 75 09\s+ld \(iy\+9\),l
\s+449:\s+3a 56 34 12\s+ld a,\(0x123456\)
\s+44d:\s+0a\s+ld a,\(bc\)
\s+44e:\s+1a\s+ld a,\(de\)
\s+44f:\s+7e\s+ld a,\(hl\)
\s+450:\s+dd 7e 09\s+ld a,\(ix\+9\)
\s+453:\s+fd 7e 09\s+ld a,\(iy\+9\)
\s+456:\s+3e 03\s+ld a,0x03
\s+458:\s+7f\s+ld a,a
\s+459:\s+78\s+ld a,b
\s+45a:\s+79\s+ld a,c
\s+45b:\s+7a\s+ld a,d
\s+45c:\s+7b\s+ld a,e
\s+45d:\s+7c\s+ld a,h
\s+45e:\s+ed 57\s+ld a,i
\s+460:\s+7d\s+ld a,l
\s+461:\s+ed 5f\s+ld a,r
\s+463:\s+46\s+ld b,\(hl\)
\s+464:\s+dd 46 09\s+ld b,\(ix\+9\)
\s+467:\s+fd 46 09\s+ld b,\(iy\+9\)
\s+46a:\s+06 03\s+ld b,0x03
\s+46c:\s+47\s+ld b,a
\s+46d:\s+00\s+nop
\s+46e:\s+41\s+ld b,c
\s+46f:\s+42\s+ld b,d
\s+470:\s+43\s+ld b,e
\s+471:\s+44\s+ld b,h
\s+472:\s+45\s+ld b,l
\s+473:\s+ed 4b 56 34 12\s+ld bc,\(0x123456\)
\s+478:\s+01 56 34 12\s+ld bc,0x123456
\s+47c:\s+4e\s+ld c,\(hl\)
\s+47d:\s+dd 4e 09\s+ld c,\(ix\+9\)
\s+480:\s+fd 4e 09\s+ld c,\(iy\+9\)
\s+483:\s+0e 03\s+ld c,0x03
\s+485:\s+4f\s+ld c,a
\s+486:\s+48\s+ld c,b
\s+487:\s+00\s+nop
\s+488:\s+4a\s+ld c,d
\s+489:\s+4b\s+ld c,e
\s+48a:\s+4c\s+ld c,h
\s+48b:\s+4d\s+ld c,l
\s+48c:\s+56\s+ld d,\(hl\)
\s+48d:\s+dd 56 09\s+ld d,\(ix\+9\)
\s+490:\s+fd 56 09\s+ld d,\(iy\+9\)
\s+493:\s+16 03\s+ld d,0x03
\s+495:\s+57\s+ld d,a
\s+496:\s+50\s+ld d,b
\s+497:\s+51\s+ld d,c
\s+498:\s+00\s+nop
\s+499:\s+53\s+ld d,e
\s+49a:\s+54\s+ld d,h
\s+49b:\s+55\s+ld d,l
\s+49c:\s+ed 5b 56 34 12\s+ld de,\(0x123456\)
\s+4a1:\s+11 56 34 12\s+ld de,0x123456
\s+4a5:\s+5e\s+ld e,\(hl\)
\s+4a6:\s+dd 5e 09\s+ld e,\(ix\+9\)
\s+4a9:\s+fd 5e 09\s+ld e,\(iy\+9\)
\s+4ac:\s+1e 03\s+ld e,0x03
\s+4ae:\s+5f\s+ld e,a
\s+4af:\s+58\s+ld e,b
\s+4b0:\s+59\s+ld e,c
\s+4b1:\s+5a\s+ld e,d
\s+4b2:\s+00\s+nop
\s+4b3:\s+5c\s+ld e,h
\s+4b4:\s+5d\s+ld e,l
\s+4b5:\s+66\s+ld h,\(hl\)
\s+4b6:\s+dd 66 09\s+ld h,\(ix\+9\)
\s+4b9:\s+fd 66 09\s+ld h,\(iy\+9\)
\s+4bc:\s+26 03\s+ld h,0x03
\s+4be:\s+67\s+ld h,a
\s+4bf:\s+60\s+ld h,b
\s+4c0:\s+61\s+ld h,c
\s+4c1:\s+62\s+ld h,d
\s+4c2:\s+63\s+ld h,e
\s+4c3:\s+64\s+ld h,h
\s+4c4:\s+65\s+ld h,l
\s+4c5:\s+2a 56 34 12\s+ld hl,\(0x123456\)
\s+4c9:\s+21 56 34 12\s+ld hl,0x123456
\s+4cd:\s+ed 47\s+ld i,a
\s+4cf:\s+dd 2a 56 34 12\s+ld ix,\(0x123456\)
\s+4d4:\s+dd 21 56 34 12\s+ld ix,0x123456
\s+4d9:\s+fd 2a 56 34 12\s+ld iy,\(0x123456\)
\s+4de:\s+fd 21 56 34 12\s+ld iy,0x123456
\s+4e3:\s+6e\s+ld l,\(hl\)
\s+4e4:\s+dd 6e 09\s+ld l,\(ix\+9\)
\s+4e7:\s+fd 6e 09\s+ld l,\(iy\+9\)
\s+4ea:\s+2e 03\s+ld l,0x03
\s+4ec:\s+6f\s+ld l,a
\s+4ed:\s+68\s+ld l,b
\s+4ee:\s+69\s+ld l,c
\s+4ef:\s+6a\s+ld l,d
\s+4f0:\s+6b\s+ld l,e
\s+4f1:\s+6c\s+ld l,h
\s+4f2:\s+6d\s+ld l,l
\s+4f3:\s+ed 4f\s+ld r,a
\s+4f5:\s+ed 7b 56 34 12\s+ld sp,\(0x123456\)
\s+4fa:\s+31 56 34 12\s+ld sp,0x123456
\s+4fe:\s+f9\s+ld sp,hl
\s+4ff:\s+dd f9\s+ld sp,ix
\s+501:\s+fd f9\s+ld sp,iy
\s+503:\s+ed a8\s+ldd
\s+505:\s+ed b8\s+lddr
\s+507:\s+ed a0\s+ldi
\s+509:\s+ed b0\s+ldir
\s+50b:\s+ed 44\s+neg
\s+50d:\s+00\s+nop
\s+50e:\s+b6\s+or a,\(hl\)
\s+50f:\s+dd b6 09\s+or a,\(ix\+9\)
\s+512:\s+fd b6 09\s+or a,\(iy\+9\)
\s+515:\s+f6 03\s+or a,0x03
\s+517:\s+b7\s+or a,a
\s+518:\s+b0\s+or a,b
\s+519:\s+b1\s+or a,c
\s+51a:\s+b2\s+or a,d
\s+51b:\s+b3\s+or a,e
\s+51c:\s+b4\s+or a,h
\s+51d:\s+b5\s+or a,l
\s+51e:\s+ed bb\s+otdr
\s+520:\s+ed b3\s+otir
\s+522:\s+ed 79\s+out \(bc\),a
\s+524:\s+ed 41\s+out \(bc\),b
\s+526:\s+ed 49\s+out \(bc\),c
\s+528:\s+ed 51\s+out \(bc\),d
\s+52a:\s+ed 59\s+out \(bc\),e
\s+52c:\s+ed 61\s+out \(bc\),h
\s+52e:\s+ed 69\s+out \(bc\),l
\s+530:\s+d3 03\s+out \(0x03\),a
\s+532:\s+ed ab\s+outd
\s+534:\s+ed a3\s+outi
\s+536:\s+f1\s+pop af
\s+537:\s+c1\s+pop bc
\s+538:\s+d1\s+pop de
\s+539:\s+e1\s+pop hl
\s+53a:\s+dd e1\s+pop ix
\s+53c:\s+fd e1\s+pop iy
\s+53e:\s+f5\s+push af
\s+53f:\s+c5\s+push bc
\s+540:\s+d5\s+push de
\s+541:\s+e5\s+push hl
\s+542:\s+dd e5\s+push ix
\s+544:\s+fd e5\s+push iy
\s+546:\s+cb 86\s+res 0,\(hl\)
\s+548:\s+dd cb 09 86\s+res 0,\(ix\+9\)
\s+54c:\s+fd cb 09 86\s+res 0,\(iy\+9\)
\s+550:\s+cb 87\s+res 0,a
\s+552:\s+cb 80\s+res 0,b
\s+554:\s+cb 81\s+res 0,c
\s+556:\s+cb 82\s+res 0,d
\s+558:\s+cb 83\s+res 0,e
\s+55a:\s+cb 84\s+res 0,h
\s+55c:\s+cb 85\s+res 0,l
\s+55e:\s+cb 8e\s+res 1,\(hl\)
\s+560:\s+dd cb 09 8e\s+res 1,\(ix\+9\)
\s+564:\s+fd cb 09 8e\s+res 1,\(iy\+9\)
\s+568:\s+cb 8f\s+res 1,a
\s+56a:\s+cb 88\s+res 1,b
\s+56c:\s+cb 89\s+res 1,c
\s+56e:\s+cb 8a\s+res 1,d
\s+570:\s+cb 8b\s+res 1,e
\s+572:\s+cb 8c\s+res 1,h
\s+574:\s+cb 8d\s+res 1,l
\s+576:\s+cb 96\s+res 2,\(hl\)
\s+578:\s+dd cb 09 96\s+res 2,\(ix\+9\)
\s+57c:\s+fd cb 09 96\s+res 2,\(iy\+9\)
\s+580:\s+cb 97\s+res 2,a
\s+582:\s+cb 90\s+res 2,b
\s+584:\s+cb 91\s+res 2,c
\s+586:\s+cb 92\s+res 2,d
\s+588:\s+cb 93\s+res 2,e
\s+58a:\s+cb 94\s+res 2,h
\s+58c:\s+cb 95\s+res 2,l
\s+58e:\s+cb 9e\s+res 3,\(hl\)
\s+590:\s+dd cb 09 9e\s+res 3,\(ix\+9\)
\s+594:\s+fd cb 09 9e\s+res 3,\(iy\+9\)
\s+598:\s+cb 9f\s+res 3,a
\s+59a:\s+cb 98\s+res 3,b
\s+59c:\s+cb 99\s+res 3,c
\s+59e:\s+cb 9a\s+res 3,d
\s+5a0:\s+cb 9b\s+res 3,e
\s+5a2:\s+cb 9c\s+res 3,h
\s+5a4:\s+cb 9d\s+res 3,l
\s+5a6:\s+cb a6\s+res 4,\(hl\)
\s+5a8:\s+dd cb 09 a6\s+res 4,\(ix\+9\)
\s+5ac:\s+fd cb 09 a6\s+res 4,\(iy\+9\)
\s+5b0:\s+cb a7\s+res 4,a
\s+5b2:\s+cb a0\s+res 4,b
\s+5b4:\s+cb a1\s+res 4,c
\s+5b6:\s+cb a2\s+res 4,d
\s+5b8:\s+cb a3\s+res 4,e
\s+5ba:\s+cb a4\s+res 4,h
\s+5bc:\s+cb a5\s+res 4,l
\s+5be:\s+cb ae\s+res 5,\(hl\)
\s+5c0:\s+dd cb 09 ae\s+res 5,\(ix\+9\)
\s+5c4:\s+fd cb 09 ae\s+res 5,\(iy\+9\)
\s+5c8:\s+cb af\s+res 5,a
\s+5ca:\s+cb a8\s+res 5,b
\s+5cc:\s+cb a9\s+res 5,c
\s+5ce:\s+cb aa\s+res 5,d
\s+5d0:\s+cb ab\s+res 5,e
\s+5d2:\s+cb ac\s+res 5,h
\s+5d4:\s+cb ad\s+res 5,l
\s+5d6:\s+cb b6\s+res 6,\(hl\)
\s+5d8:\s+dd cb 09 b6\s+res 6,\(ix\+9\)
\s+5dc:\s+fd cb 09 b6\s+res 6,\(iy\+9\)
\s+5e0:\s+cb b7\s+res 6,a
\s+5e2:\s+cb b0\s+res 6,b
\s+5e4:\s+cb b1\s+res 6,c
\s+5e6:\s+cb b2\s+res 6,d
\s+5e8:\s+cb b3\s+res 6,e
\s+5ea:\s+cb b4\s+res 6,h
\s+5ec:\s+cb b5\s+res 6,l
\s+5ee:\s+cb be\s+res 7,\(hl\)
\s+5f0:\s+dd cb 09 be\s+res 7,\(ix\+9\)
\s+5f4:\s+fd cb 09 be\s+res 7,\(iy\+9\)
\s+5f8:\s+cb bf\s+res 7,a
\s+5fa:\s+cb b8\s+res 7,b
\s+5fc:\s+cb b9\s+res 7,c
\s+5fe:\s+cb ba\s+res 7,d
\s+600:\s+cb bb\s+res 7,e
\s+602:\s+cb bc\s+res 7,h
\s+604:\s+cb bd\s+res 7,l
\s+606:\s+c9\s+ret
\s+607:\s+d8\s+ret c
\s+608:\s+f8\s+ret m
\s+609:\s+d0\s+ret nc
\s+60a:\s+c0\s+ret nz
\s+60b:\s+f0\s+ret p
\s+60c:\s+e8\s+ret pe
\s+60d:\s+e0\s+ret po
\s+60e:\s+c8\s+ret z
\s+60f:\s+ed 4d\s+reti
\s+611:\s+ed 45\s+retn
\s+613:\s+cb 16\s+rl \(hl\)
\s+615:\s+dd cb 09 16\s+rl \(ix\+9\)
\s+619:\s+fd cb 09 16\s+rl \(iy\+9\)
\s+61d:\s+cb 17\s+rl a
\s+61f:\s+cb 10\s+rl b
\s+621:\s+cb 11\s+rl c
\s+623:\s+cb 12\s+rl d
\s+625:\s+cb 13\s+rl e
\s+627:\s+cb 14\s+rl h
\s+629:\s+cb 15\s+rl l
\s+62b:\s+17\s+rla
\s+62c:\s+cb 06\s+rlc \(hl\)
\s+62e:\s+dd cb 09 06\s+rlc \(ix\+9\)
\s+632:\s+fd cb 09 06\s+rlc \(iy\+9\)
\s+636:\s+cb 07\s+rlc a
\s+638:\s+cb 00\s+rlc b
\s+63a:\s+cb 01\s+rlc c
\s+63c:\s+cb 02\s+rlc d
\s+63e:\s+cb 03\s+rlc e
\s+640:\s+cb 04\s+rlc h
\s+642:\s+cb 05\s+rlc l
\s+644:\s+07\s+rlca
\s+645:\s+ed 6f\s+rld
\s+647:\s+cb 1e\s+rr \(hl\)
\s+649:\s+dd cb 09 1e\s+rr \(ix\+9\)
\s+64d:\s+fd cb 09 1e\s+rr \(iy\+9\)
\s+651:\s+cb 1f\s+rr a
\s+653:\s+cb 18\s+rr b
\s+655:\s+cb 19\s+rr c
\s+657:\s+cb 1a\s+rr d
\s+659:\s+cb 1b\s+rr e
\s+65b:\s+cb 1c\s+rr h
\s+65d:\s+cb 1d\s+rr l
\s+65f:\s+1f\s+rra
\s+660:\s+cb 0e\s+rrc \(hl\)
\s+662:\s+dd cb 09 0e\s+rrc \(ix\+9\)
\s+666:\s+fd cb 09 0e\s+rrc \(iy\+9\)
\s+66a:\s+cb 0f\s+rrc a
\s+66c:\s+cb 08\s+rrc b
\s+66e:\s+cb 09\s+rrc c
\s+670:\s+cb 0a\s+rrc d
\s+672:\s+cb 0b\s+rrc e
\s+674:\s+cb 0c\s+rrc h
\s+676:\s+cb 0d\s+rrc l
\s+678:\s+0f\s+rrca
\s+679:\s+ed 67\s+rrd
\s+67b:\s+c7\s+rst 0x00
\s+67c:\s+cf\s+rst 0x08
\s+67d:\s+d7\s+rst 0x10
\s+67e:\s+df\s+rst 0x18
\s+67f:\s+e7\s+rst 0x20
\s+680:\s+ef\s+rst 0x28
\s+681:\s+f7\s+rst 0x30
\s+682:\s+ff\s+rst 0x38
\s+683:\s+9e\s+sbc a,\(hl\)
\s+684:\s+dd 9e 09\s+sbc a,\(ix\+9\)
\s+687:\s+fd 9e 09\s+sbc a,\(iy\+9\)
\s+68a:\s+de 03\s+sbc a,0x03
\s+68c:\s+9f\s+sbc a,a
\s+68d:\s+98\s+sbc a,b
\s+68e:\s+99\s+sbc a,c
\s+68f:\s+9a\s+sbc a,d
\s+690:\s+9b\s+sbc a,e
\s+691:\s+9c\s+sbc a,h
\s+692:\s+9d\s+sbc a,l
\s+693:\s+ed 42\s+sbc hl,bc
\s+695:\s+ed 52\s+sbc hl,de
\s+697:\s+ed 62\s+sbc hl,hl
\s+699:\s+ed 72\s+sbc hl,sp
\s+69b:\s+37\s+scf
\s+69c:\s+cb c6\s+set 0,\(hl\)
\s+69e:\s+dd cb 09 c6\s+set 0,\(ix\+9\)
\s+6a2:\s+fd cb 09 c6\s+set 0,\(iy\+9\)
\s+6a6:\s+cb c7\s+set 0,a
\s+6a8:\s+cb c0\s+set 0,b
\s+6aa:\s+cb c1\s+set 0,c
\s+6ac:\s+cb c2\s+set 0,d
\s+6ae:\s+cb c3\s+set 0,e
\s+6b0:\s+cb c4\s+set 0,h
\s+6b2:\s+cb c5\s+set 0,l
\s+6b4:\s+cb ce\s+set 1,\(hl\)
\s+6b6:\s+dd cb 09 ce\s+set 1,\(ix\+9\)
\s+6ba:\s+fd cb 09 ce\s+set 1,\(iy\+9\)
\s+6be:\s+cb cf\s+set 1,a
\s+6c0:\s+cb c8\s+set 1,b
\s+6c2:\s+cb c9\s+set 1,c
\s+6c4:\s+cb ca\s+set 1,d
\s+6c6:\s+cb cb\s+set 1,e
\s+6c8:\s+cb cc\s+set 1,h
\s+6ca:\s+cb cd\s+set 1,l
\s+6cc:\s+cb d6\s+set 2,\(hl\)
\s+6ce:\s+dd cb 09 d6\s+set 2,\(ix\+9\)
\s+6d2:\s+fd cb 09 d6\s+set 2,\(iy\+9\)
\s+6d6:\s+cb d7\s+set 2,a
\s+6d8:\s+cb d0\s+set 2,b
\s+6da:\s+cb d1\s+set 2,c
\s+6dc:\s+cb d2\s+set 2,d
\s+6de:\s+cb d3\s+set 2,e
\s+6e0:\s+cb d4\s+set 2,h
\s+6e2:\s+cb d5\s+set 2,l
\s+6e4:\s+cb de\s+set 3,\(hl\)
\s+6e6:\s+dd cb 09 de\s+set 3,\(ix\+9\)
\s+6ea:\s+fd cb 09 de\s+set 3,\(iy\+9\)
\s+6ee:\s+cb df\s+set 3,a
\s+6f0:\s+cb d8\s+set 3,b
\s+6f2:\s+cb d9\s+set 3,c
\s+6f4:\s+cb da\s+set 3,d
\s+6f6:\s+cb db\s+set 3,e
\s+6f8:\s+cb dc\s+set 3,h
\s+6fa:\s+cb dd\s+set 3,l
\s+6fc:\s+cb e6\s+set 4,\(hl\)
\s+6fe:\s+dd cb 09 e6\s+set 4,\(ix\+9\)
\s+702:\s+fd cb 09 e6\s+set 4,\(iy\+9\)
\s+706:\s+cb e7\s+set 4,a
\s+708:\s+cb e0\s+set 4,b
\s+70a:\s+cb e1\s+set 4,c
\s+70c:\s+cb e2\s+set 4,d
\s+70e:\s+cb e3\s+set 4,e
\s+710:\s+cb e4\s+set 4,h
\s+712:\s+cb e5\s+set 4,l
\s+714:\s+cb ee\s+set 5,\(hl\)
\s+716:\s+dd cb 09 ee\s+set 5,\(ix\+9\)
\s+71a:\s+fd cb 09 ee\s+set 5,\(iy\+9\)
\s+71e:\s+cb ef\s+set 5,a
\s+720:\s+cb e8\s+set 5,b
\s+722:\s+cb e9\s+set 5,c
\s+724:\s+cb ea\s+set 5,d
\s+726:\s+cb eb\s+set 5,e
\s+728:\s+cb ec\s+set 5,h
\s+72a:\s+cb ed\s+set 5,l
\s+72c:\s+cb f6\s+set 6,\(hl\)
\s+72e:\s+dd cb 09 f6\s+set 6,\(ix\+9\)
\s+732:\s+fd cb 09 f6\s+set 6,\(iy\+9\)
\s+736:\s+cb f7\s+set 6,a
\s+738:\s+cb f0\s+set 6,b
\s+73a:\s+cb f1\s+set 6,c
\s+73c:\s+cb f2\s+set 6,d
\s+73e:\s+cb f3\s+set 6,e
\s+740:\s+cb f4\s+set 6,h
\s+742:\s+cb f5\s+set 6,l
\s+744:\s+cb fe\s+set 7,\(hl\)
\s+746:\s+dd cb 09 fe\s+set 7,\(ix\+9\)
\s+74a:\s+fd cb 09 fe\s+set 7,\(iy\+9\)
\s+74e:\s+cb ff\s+set 7,a
\s+750:\s+cb f8\s+set 7,b
\s+752:\s+cb f9\s+set 7,c
\s+754:\s+cb fa\s+set 7,d
\s+756:\s+cb fb\s+set 7,e
\s+758:\s+cb fc\s+set 7,h
\s+75a:\s+cb fd\s+set 7,l
\s+75c:\s+cb 26\s+sla \(hl\)
\s+75e:\s+dd cb 09 26\s+sla \(ix\+9\)
\s+762:\s+fd cb 09 26\s+sla \(iy\+9\)
\s+766:\s+cb 27\s+sla a
\s+768:\s+cb 20\s+sla b
\s+76a:\s+cb 21\s+sla c
\s+76c:\s+cb 22\s+sla d
\s+76e:\s+cb 23\s+sla e
\s+770:\s+cb 24\s+sla h
\s+772:\s+cb 25\s+sla l
\s+774:\s+cb 2e\s+sra \(hl\)
\s+776:\s+dd cb 09 2e\s+sra \(ix\+9\)
\s+77a:\s+fd cb 09 2e\s+sra \(iy\+9\)
\s+77e:\s+cb 2f\s+sra a
\s+780:\s+cb 28\s+sra b
\s+782:\s+cb 29\s+sra c
\s+784:\s+cb 2a\s+sra d
\s+786:\s+cb 2b\s+sra e
\s+788:\s+cb 2c\s+sra h
\s+78a:\s+cb 2d\s+sra l
\s+78c:\s+cb 3e\s+srl \(hl\)
\s+78e:\s+dd cb 09 3e\s+srl \(ix\+9\)
\s+792:\s+fd cb 09 3e\s+srl \(iy\+9\)
\s+796:\s+cb 3f\s+srl a
\s+798:\s+cb 38\s+srl b
\s+79a:\s+cb 39\s+srl c
\s+79c:\s+cb 3a\s+srl d
\s+79e:\s+cb 3b\s+srl e
\s+7a0:\s+cb 3c\s+srl h
\s+7a2:\s+cb 3d\s+srl l
\s+7a4:\s+96\s+sub a,\(hl\)
\s+7a5:\s+dd 96 09\s+sub a,\(ix\+9\)
\s+7a8:\s+fd 96 09\s+sub a,\(iy\+9\)
\s+7ab:\s+d6 03\s+sub a,0x03
\s+7ad:\s+97\s+sub a,a
\s+7ae:\s+90\s+sub a,b
\s+7af:\s+91\s+sub a,c
\s+7b0:\s+92\s+sub a,d
\s+7b1:\s+93\s+sub a,e
\s+7b2:\s+94\s+sub a,h
\s+7b3:\s+95\s+sub a,l
\s+7b4:\s+ae\s+xor a,\(hl\)
\s+7b5:\s+dd ae 09\s+xor a,\(ix\+9\)
\s+7b8:\s+fd ae 09\s+xor a,\(iy\+9\)
\s+7bb:\s+ee 03\s+xor a,0x03
\s+7bd:\s+af\s+xor a,a
\s+7be:\s+a8\s+xor a,b
\s+7bf:\s+a9\s+xor a,c
\s+7c0:\s+aa\s+xor a,d
\s+7c1:\s+ab\s+xor a,e
\s+7c2:\s+ac\s+xor a,h
\s+7c3:\s+ad\s+xor a,l
\s+7c4:\s+ed 07\s+ld bc,\(hl\)
\s+7c6:\s+ed 17\s+ld de,\(hl\)
\s+7c8:\s+ed 27\s+ld hl,\(hl\)
\s+7ca:\s+ed 37\s+ld ix,\(hl\)
\s+7cc:\s+ed 31\s+ld iy,\(hl\)
\s+7ce:\s+ed 0f\s+ld \(hl\),bc
\s+7d0:\s+ed 1f\s+ld \(hl\),de
\s+7d2:\s+ed 2f\s+ld \(hl\),hl
\s+7d4:\s+ed 3f\s+ld \(hl\),ix
\s+7d6:\s+ed 3e\s+ld \(hl\),iy
