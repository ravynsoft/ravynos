#name: Z80N forward relocation
#as: -march=z80n --defsym Z80N=1
#source: relocs.s
#source: labels.s
#ld: -e 0 -Ttext 0x100 -Tdata 0x200
#objdump: -d


.*:[     ]+file format (coff|elf32)\-z80


.* \.text:

0+100 <.*>:
[ ]+100:[ 	]+cd 7d 01[    	]+call 0x017d
[ ]+103:[ 	]+c4 7e 01[    	]+call nz,0x017e
[ ]+106:[ 	]+cc 7f 01[    	]+call z,0x017f
[ ]+109:[ 	]+d4 80 01[    	]+call nc,0x0180
[ ]+10c:[ 	]+dc 81 01[    	]+call c,0x0181
[ ]+10f:[ 	]+e4 82 01[    	]+call po,0x0182
[ ]+112:[ 	]+ec 83 01[    	]+call pe,0x0183
[ ]+115:[ 	]+f4 84 01[    	]+call p,0x0184
[ ]+118:[ 	]+fc 85 01[    	]+call m,0x0185
[ ]+11b:[ 	]+c3 7d 01[    	]+jp 0x017d
[ ]+11e:[ 	]+c2 7e 01[    	]+jp nz,0x017e
[ ]+121:[ 	]+ca 7f 01[    	]+jp z,0x017f
[ ]+124:[ 	]+d2 80 01[    	]+jp nc,0x0180
[ ]+127:[ 	]+da 81 01[    	]+jp c,0x0181
[ ]+12a:[ 	]+e2 82 01[    	]+jp po,0x0182
[ ]+12d:[ 	]+ea 83 01[    	]+jp pe,0x0183
[ ]+130:[ 	]+f2 84 01[    	]+jp p,0x0184
[ ]+133:[ 	]+fa 85 01[    	]+jp m,0x0185
[ ]+136:[ 	]+dd 6e 05[    	]+ld l,\(ix\+5\)
[ ]+139:[ 	]+dd 7e 03[    	]+ld a,\(ix\+3\)
[ ]+13c:[ 	]+dd 4e fa[    	]+ld c,\(ix\-6\)
[ ]+13f:[ 	]+dd 46 f9[    	]+ld b,\(ix\-7\)
[ ]+142:[ 	]+fd 75 fb[    	]+ld \(iy\-5\),l
[ ]+145:[ 	]+fd 77 03[    	]+ld \(iy\+3\),a
[ ]+148:[ 	]+fd 71 0e[    	]+ld \(iy\+14\),c
[ ]+14b:[ 	]+fd 70 0f[    	]+ld \(iy\+15\),b
[ ]+14e:[ 	]+fd 66 5d[    	]+ld h,\(iy\+93\)
[ ]+151:[ 	]+11 34 12[    	]+ld de,0x1234
[ ]+154:[ 	]+21 78 56[    	]+ld hl,0x5678
[ ]+157:[ 	]+11 68 24[    	]+ld de,0x2468
[ ]+15a:[ 	]+21 f0 ac[    	]+ld hl,0xacf0
[ ]+15d:[ 	]+16 12[       	]+ld d,0x12
[ ]+15f:[ 	]+1e 34[       	]+ld e,0x34
[ ]+161:[ 	]+26 56[       	]+ld h,0x56
[ ]+163:[ 	]+2e 78[       	]+ld l,0x78
[ ]+165:[ 	]+16 24[       	]+ld d,0x24
[ ]+167:[ 	]+1e 68[       	]+ld e,0x68
[ ]+169:[ 	]+26 ac[       	]+ld h,0xac
[ ]+16b:[ 	]+2e f0[       	]+ld l,0xf0
[ ]+16d:[ 	]+ed 8a 01 7d[ 	]+push 0x017d
[ ]+171:[ 	]+ed 8a 12 34[ 	]+push 0x1234
[ ]+175:[ 	]+ed 91 ab cd[ 	]+nextreg 0xab,0xcd
[ ]+179:[ 	]+ed 92 ef[    	]+nextreg 0xef,a
[ ]+17c:[ 	]+7f[          	]+ld a,a

0+17d <label1>:
[ ]+17d:[ 	]+78[          	]+ld a,b

0+17e <label2>:
[ ]+17e:[ 	]+79[          	]+ld a,c

0+17f <label3>:
[ ]+17f:[ 	]+7a[          	]+ld a,d

0+180 <label4>:
[ ]+180:[ 	]+7b[          	]+ld a,e

0+181 <label5>:
[ ]+181:[ 	]+7c[          	]+ld a,h

0+182 <label6>:
[ ]+182:[ 	]+7d[          	]+ld a,l

0+183 <label7>:
[ ]+183:[ 	]+7e[          	]+ld a,\(hl\)

0+184 <label8>:
[ ]+184:[ 	]+7f[          	]+ld a,a

0+185 <label9>:
[ ]+185:[ 	]+2f[          	]+cpl
#pass
