#name: eZ80 forward relocation in ADL mode
#source: relocs.s --defsym ADLMODE=1
#source: labels.s
#as: -march=ez80+adl
#ld: -e 0 -Ttext 0x100 -Tdata 0x200
#objdump: -d

.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000100 <.*>:
 100:[ 	]+cd 83 01 00[       	]+call 0x0183
 104:[ 	]+c4 84 01 00[       	]+call nz,0x0184
 108:[ 	]+cc 85 01 00[       	]+call z,0x0185
 10c:[ 	]+d4 86 01 00[       	]+call nc,0x0186
 110:[ 	]+dc 87 01 00[       	]+call c,0x0187
 114:[ 	]+e4 88 01 00[       	]+call po,0x0188
 118:[ 	]+ec 89 01 00[       	]+call pe,0x0189
 11c:[ 	]+f4 8a 01 00[       	]+call p,0x018a
 120:[ 	]+fc 8b 01 00[       	]+call m,0x018b
 124:[ 	]+c3 83 01 00[       	]+jp 0x0183
 128:[ 	]+c2 84 01 00[       	]+jp nz,0x0184
 12c:[ 	]+ca 85 01 00[       	]+jp z,0x0185
 130:[ 	]+d2 86 01 00[       	]+jp nc,0x0186
 134:[ 	]+da 87 01 00[       	]+jp c,0x0187
 138:[ 	]+e2 88 01 00[       	]+jp po,0x0188
 13c:[ 	]+ea 89 01 00[       	]+jp pe,0x0189
 140:[ 	]+f2 8a 01 00[       	]+jp p,0x018a
 144:[ 	]+fa 8b 01 00[       	]+jp m,0x018b
 148:[ 	]+dd 6e 05[          	]+ld l,\(ix\+5\)
 14b:[ 	]+dd 7e 03[          	]+ld a,\(ix\+3\)
 14e:[ 	]+dd 4e fa[          	]+ld c,\(ix\-6\)
 151:[ 	]+dd 46 f9[          	]+ld b,\(ix\-7\)
 154:[ 	]+fd 75 fb[          	]+ld \(iy\-5\),l
 157:[ 	]+fd 77 03[          	]+ld \(iy\+3\),a
 15a:[ 	]+fd 71 0e[          	]+ld \(iy\+14\),c
 15d:[ 	]+fd 70 0f[          	]+ld \(iy\+15\),b
 160:[ 	]+fd 66 5d[          	]+ld h,\(iy\+93\)
 163:[ 	]+49 11 34 12[       	]+ld\.lis de,0x1234
 167:[ 	]+49 21 78 56[       	]+ld\.lis hl,0x5678
 16b:[ 	]+49 11 68 24[       	]+ld\.lis de,0x2468
 16f:[ 	]+49 21 f0 ac[       	]+ld\.lis hl,0xacf0
 173:[ 	]+16 12[             	]+ld d,0x12
 175:[ 	]+1e 34[             	]+ld e,0x34
 177:[ 	]+26 56[             	]+ld h,0x56
 179:[ 	]+2e 78[             	]+ld l,0x78
 17b:[ 	]+16 24[             	]+ld d,0x24
 17d:[ 	]+1e 68[             	]+ld e,0x68
 17f:[ 	]+26 ac[             	]+ld h,0xac
 181:[ 	]+2e f0[             	]+ld l,0xf0

00000183 <label1>:
 183:[ 	]+78[                	]+ld a,b

00000184 <label2>:
 184:[ 	]+79[                	]+ld a,c

00000185 <label3>:
 185:[ 	]+7a[                	]+ld a,d

00000186 <label4>:
 186:[ 	]+7b[                	]+ld a,e

00000187 <label5>:
 187:[ 	]+7c[                	]+ld a,h

00000188 <label6>:
 188:[ 	]+7d[                	]+ld a,l

00000189 <label7>:
 189:[ 	]+7e[                	]+ld a,\(hl\)

0000018a <label8>:
 18a:[ 	]+7f[                	]+ld a,a

0000018b <label9>:
 18b:[ 	]+2f[                	]+cpl
