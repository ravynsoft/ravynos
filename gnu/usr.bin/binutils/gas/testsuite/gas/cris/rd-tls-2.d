#objdump: -dr
#as: --em=criself --pic
#name: TLS PIC relocs.

.*:     file format .*-cris

Disassembly of section \.text:

0+ <start>:
[ 	]+0:	6f3d 0000 0000 6aaa[ 	]+move\.d \[\$?r3\+0 <start>\],\$?r10
[ 	]+2: R_CRIS_32_GOT_TPREL	extsym
[ 	]+8:	6f8e 0000 0000[ 	]+move\.d 0 <start>,\$?r8
[ 	]+a: R_CRIS_32_GOT_TPREL	extsym5
[ 	]+e:	5f8e 0000[ 	]+move\.w 0x0,\$?r8
[ 	]+10: R_CRIS_16_GOT_TPREL	extsym9
[ 	]+12:	6f3d 0000 0000 6aaa[ 	]+move\.d \[\$?r3\+0 <start>\],\$?r10
[ 	]+14: R_CRIS_32_GOT_GD	extsym
[ 	]+1a:	5fdd 0000 6aaa[ 	]+move\.d \[\$?r13\+0\],\$?r10
[ 	]+1c: R_CRIS_16_GOT_TPREL	extsym13
[ 	]+20:	5fae 0000[ 	]+move\.w 0x0,\$?r10
[ 	]+22: R_CRIS_16_GOT_GD	extsym14
[ 	]+24:	af9e 0000 0000[ 	]+sub\.d 0 <start>,\$?r9
[ 	]+26: R_CRIS_32_DTPREL	extsym4\+0x16
[ 	]+2a:	9f3e 0000[ 	]+sub\.w 0x0,\$?r3
[ 	]+2c: R_CRIS_16_DTPREL	extsym4-0x56
[ 	]+2e:	6f3d 0000 0000 aa4a[ 	]+sub\.d \[\$?r3\+0 <start>\],\$?r4,\$?r10
[ 	]+30: R_CRIS_32_GOT_TPREL	extsym3
[ 	]+36:	af9e 0000 0000[ 	]+sub\.d 0 <start>,\$?r9
[ 	]+38: R_CRIS_32_GOT_GD	extsym4\+0x2a
[ 	]+3c:	af3e 0000 0000[ 	]+sub\.d 0 <start>,\$?r3
[ 	]+3e: R_CRIS_32_GOT_TPREL	extsym4-0x60
[ 	]+42:	6fad 0000 0000 287a[ 	]+add\.d \[\$?r10\+0 <start>\],\$?r7,\$?r8
[ 	]+44: R_CRIS_32_GOT_TPREL	extsym3\+0x38
[ 	]+4a:	6f5d 0000 0000 611a[ 	]+move\.d \[\$?r5\+0 <start>\],\$?r1
[ 	]+4c: R_CRIS_32_GOT_TPREL	extsym6\+0xa
[ 	]+52:	6fad 0000 0000 284a[ 	]+add\.d \[\$?r10\+0 <start>\],\$?r4,\$?r8
[ 	]+54: R_CRIS_32_GOT_TPREL	extsym3-0x230
[ 	]+5a:	6f5d 0000 0000 6cca[ 	]+move\.d \[\$?r5\+0 <start>\],\$?r12
[ 	]+5c: R_CRIS_32_GOT_TPREL	extsym6-0x6e
[ 	]+62:	6f5d 0000 0000 69ce[ 	]+move\.d \[\$?r9=\$?r5\+0 <start>\],\$?r12
[ 	]+64: R_CRIS_32_GOT_TPREL	extsym6-0xdc
[ 	]+6a:	5fcd 0000 a89a[ 	]+sub\.d \[\$?r12\+0\],\$?r9,\$?r8
[ 	]+6c: R_CRIS_16_GOT_TPREL	extsym3-0x9c
[ 	]+70:	5fbd 0000 699a[ 	]+move\.d \[\$?r11\+0\],\$?r9
[ 	]+72: R_CRIS_16_GOT_GD	extsym14-0x100
[ 	]+76:	6fad 0000 0000 287a[ 	]+add\.d \[\$?r10\+0 <start>\],\$?r7,\$?r8
[ 	]+78: R_CRIS_32_GOT_GD	extsym3\+0x38
[ 	]+\.\.\.
