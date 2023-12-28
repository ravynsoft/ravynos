#name: Forward relocation
#source: relocs.s
#source: labels.s
#ld: -e 0 -Ttext 0x100 -Tdata 0x200
#objdump: -d


.*:[     ]+file format (coff)|(elf32)\-z80


.* \.text:

00000100 <.*>:
 100:[ 	]+cd 6d 01[    	]+call 0x016d
 103:[ 	]+c4 6e 01[    	]+call nz,0x016e
 106:[ 	]+cc 6f 01[    	]+call z,0x016f
 109:[ 	]+d4 70 01[    	]+call nc,0x0170
 10c:[ 	]+dc 71 01[    	]+call c,0x0171
 10f:[ 	]+e4 72 01[    	]+call po,0x0172
 112:[ 	]+ec 73 01[    	]+call pe,0x0173
 115:[ 	]+f4 74 01[    	]+call p,0x0174
 118:[ 	]+fc 75 01[    	]+call m,0x0175
 11b:[ 	]+c3 6d 01[    	]+jp 0x016d
 11e:[ 	]+c2 6e 01[    	]+jp nz,0x016e
 121:[ 	]+ca 6f 01[    	]+jp z,0x016f
 124:[ 	]+d2 70 01[    	]+jp nc,0x0170
 127:[ 	]+da 71 01[    	]+jp c,0x0171
 12a:[ 	]+e2 72 01[    	]+jp po,0x0172
 12d:[ 	]+ea 73 01[    	]+jp pe,0x0173
 130:[ 	]+f2 74 01[    	]+jp p,0x0174
 133:[ 	]+fa 75 01[    	]+jp m,0x0175
 136:[ 	]+dd 6e 05[    	]+ld l,\(ix\+5\)
 139:[ 	]+dd 7e 03[    	]+ld a,\(ix\+3\)
 13c:[ 	]+dd 4e fa[    	]+ld c,\(ix\-6\)
 13f:[ 	]+dd 46 f9[    	]+ld b,\(ix\-7\)
 142:[ 	]+fd 75 fb[    	]+ld \(iy\-5\),l
 145:[ 	]+fd 77 03[    	]+ld \(iy\+3\),a
 148:[ 	]+fd 71 0e[    	]+ld \(iy\+14\),c
 14b:[ 	]+fd 70 0f[    	]+ld \(iy\+15\),b
 14e:[ 	]+fd 66 5d[    	]+ld h,\(iy\+93\)
 151:[ 	]+11 34 12[    	]+ld de,0x1234
 154:[ 	]+21 78 56[    	]+ld hl,0x5678
 157:[ 	]+11 68 24[    	]+ld de,0x2468
 15a:[ 	]+21 f0 ac[    	]+ld hl,0xacf0
 15d:[ 	]+16 12[       	]+ld d,0x12
 15f:[ 	]+1e 34[       	]+ld e,0x34
 161:[ 	]+26 56[       	]+ld h,0x56
 163:[ 	]+2e 78[       	]+ld l,0x78
 165:[ 	]+16 24[       	]+ld d,0x24
 167:[ 	]+1e 68[       	]+ld e,0x68
 169:[ 	]+26 ac[       	]+ld h,0xac
 16b:[ 	]+2e f0[       	]+ld l,0xf0

0000016d <label1>:
 16d:[ 	]+78[          	]+ld a,b

0000016e <label2>:
 16e:[ 	]+79[          	]+ld a,c

0000016f <label3>:
 16f:[ 	]+7a[          	]+ld a,d

00000170 <label4>:
 170:[ 	]+7b[          	]+ld a,e

00000171 <label5>:
 171:[ 	]+7c[          	]+ld a,h

00000172 <label6>:
 172:[ 	]+7d[          	]+ld a,l

00000173 <label7>:
 173:[ 	]+7e[          	]+ld a,\(hl\)

00000174 <label8>:
 174:[ 	]+7f[          	]+ld a,a

00000175 <label9>:
 175:[ 	]+2f[          	]+cpl
