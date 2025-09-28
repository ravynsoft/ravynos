#name: Z80 backward relocation
#source: labels.s
#source: relocs.s
#ld: -e 0 -Ttext 0x100 -Tdata 0x200 -s
#objdump: -d

.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000100 <.*>:
 100:[ 	]+78[          	]+ld a,b
 101:[ 	]+79[          	]+ld a,c
 102:[ 	]+7a[          	]+ld a,d
 103:[ 	]+7b[          	]+ld a,e
 104:[ 	]+7c[          	]+ld a,h
 105:[ 	]+7d[          	]+ld a,l
 106:[ 	]+7e[          	]+ld a,\(hl\)
 107:[ 	]+7f[          	]+ld a,a
 108:[ 	]+2f[          	]+cpl
 109:[ 	]+cd 00 01[    	]+call 0x0100
 10c:[ 	]+c4 01 01[    	]+call nz,0x0101
 10f:[ 	]+cc 02 01[    	]+call z,0x0102
 112:[ 	]+d4 03 01[    	]+call nc,0x0103
 115:[ 	]+dc 04 01[    	]+call c,0x0104
 118:[ 	]+e4 05 01[    	]+call po,0x0105
 11b:[ 	]+ec 06 01[    	]+call pe,0x0106
 11e:[ 	]+f4 07 01[    	]+call p,0x0107
 121:[ 	]+fc 08 01[    	]+call m,0x0108
 124:[ 	]+c3 00 01[    	]+jp 0x0100
 127:[ 	]+c2 01 01[    	]+jp nz,0x0101
 12a:[ 	]+ca 02 01[    	]+jp z,0x0102
 12d:[ 	]+d2 03 01[    	]+jp nc,0x0103
 130:[ 	]+da 04 01[    	]+jp c,0x0104
 133:[ 	]+e2 05 01[    	]+jp po,0x0105
 136:[ 	]+ea 06 01[    	]+jp pe,0x0106
 139:[ 	]+f2 07 01[    	]+jp p,0x0107
 13c:[ 	]+fa 08 01[    	]+jp m,0x0108
 13f:[ 	]+dd 6e 05[    	]+ld l,\(ix\+5\)
 142:[ 	]+dd 7e 03[    	]+ld a,\(ix\+3\)
 145:[ 	]+dd 4e fa[    	]+ld c,\(ix\-6\)
 148:[ 	]+dd 46 f9[    	]+ld b,\(ix\-7\)
 14b:[ 	]+fd 75 fb[    	]+ld \(iy\-5\),l
 14e:[ 	]+fd 77 03[    	]+ld \(iy\+3\),a
 151:[ 	]+fd 71 0e[    	]+ld \(iy\+14\),c
 154:[ 	]+fd 70 0f[    	]+ld \(iy\+15\),b
 157:[ 	]+fd 66 5d[    	]+ld h,\(iy\+93\)
 15a:[ 	]+11 34 12[    	]+ld de,0x1234
 15d:[ 	]+21 78 56[    	]+ld hl,0x5678
 160:[ 	]+11 68 24[    	]+ld de,0x2468
 163:[ 	]+21 f0 ac[    	]+ld hl,0xacf0
 166:[ 	]+16 12[       	]+ld d,0x12
 168:[ 	]+1e 34[       	]+ld e,0x34
 16a:[ 	]+26 56[       	]+ld h,0x56
 16c:[ 	]+2e 78[       	]+ld l,0x78
 16e:[ 	]+16 24[       	]+ld d,0x24
 170:[ 	]+1e 68[       	]+ld e,0x68
 172:[ 	]+26 ac[       	]+ld h,0xac
 174:[ 	]+2e f0[       	]+ld l,0xf0
