#name: eZ80 backward relocation in ADL mode
#source: labels.s
#source: relocs.s --defsym ADLMODE=1
#as: -march=ez80+adl
#ld: -e 0 -Ttext 0x100 -Tdata 0x200 -s
#objdump: -d

.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000100 <.*>:
 100:[ 	]+78[                	]+ld a,b
 101:[ 	]+79[                	]+ld a,c
 102:[ 	]+7a[                	]+ld a,d
 103:[ 	]+7b[                	]+ld a,e
 104:[ 	]+7c[                	]+ld a,h
 105:[ 	]+7d[                	]+ld a,l
 106:[ 	]+7e[                	]+ld a,\(hl\)
 107:[ 	]+7f[                	]+ld a,a
 108:[ 	]+2f[                	]+cpl
 109:[ 	]+cd 00 01 00[       	]+call 0x0100
 10d:[ 	]+c4 01 01 00[       	]+call nz,0x0101
 111:[ 	]+cc 02 01 00[       	]+call z,0x0102
 115:[ 	]+d4 03 01 00[       	]+call nc,0x0103
 119:[ 	]+dc 04 01 00[       	]+call c,0x0104
 11d:[ 	]+e4 05 01 00[       	]+call po,0x0105
 121:[ 	]+ec 06 01 00[       	]+call pe,0x0106
 125:[ 	]+f4 07 01 00[       	]+call p,0x0107
 129:[ 	]+fc 08 01 00[       	]+call m,0x0108
 12d:[ 	]+c3 00 01 00[       	]+jp 0x0100
 131:[ 	]+c2 01 01 00[       	]+jp nz,0x0101
 135:[ 	]+ca 02 01 00[       	]+jp z,0x0102
 139:[ 	]+d2 03 01 00[       	]+jp nc,0x0103
 13d:[ 	]+da 04 01 00[       	]+jp c,0x0104
 141:[ 	]+e2 05 01 00[       	]+jp po,0x0105
 145:[ 	]+ea 06 01 00[       	]+jp pe,0x0106
 149:[ 	]+f2 07 01 00[       	]+jp p,0x0107
 14d:[ 	]+fa 08 01 00[       	]+jp m,0x0108
 151:[ 	]+dd 6e 05[          	]+ld l,\(ix\+5\)
 154:[ 	]+dd 7e 03[          	]+ld a,\(ix\+3\)
 157:[ 	]+dd 4e fa[          	]+ld c,\(ix\-6\)
 15a:[ 	]+dd 46 f9[          	]+ld b,\(ix\-7\)
 15d:[ 	]+fd 75 fb[          	]+ld \(iy\-5\),l
 160:[ 	]+fd 77 03[          	]+ld \(iy\+3\),a
 163:[ 	]+fd 71 0e[          	]+ld \(iy\+14\),c
 166:[ 	]+fd 70 0f[          	]+ld \(iy\+15\),b
 169:[ 	]+fd 66 5d[          	]+ld h,\(iy\+93\)
 16c:[ 	]+49 11 34 12[       	]+ld\.lis de,0x1234
 170:[ 	]+49 21 78 56[       	]+ld\.lis hl,0x5678
 174:[ 	]+49 11 68 24[       	]+ld\.lis de,0x2468
 178:[ 	]+49 21 f0 ac[       	]+ld\.lis hl,0xacf0
 17c:[ 	]+16 12[             	]+ld d,0x12
 17e:[ 	]+1e 34[             	]+ld e,0x34
 180:[ 	]+26 56[             	]+ld h,0x56
 182:[ 	]+2e 78[             	]+ld l,0x78
 184:[ 	]+16 24[             	]+ld d,0x24
 186:[ 	]+1e 68[             	]+ld e,0x68
 188:[ 	]+26 ac[             	]+ld h,0xac
 18a:[ 	]+2e f0[             	]+ld l,0xf0
