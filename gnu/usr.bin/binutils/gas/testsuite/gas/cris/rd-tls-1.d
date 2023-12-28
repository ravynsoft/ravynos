#objdump: -dr
#as: --em=criself
#name: TLS non-PIC relocs.

.*:     file format .*-cris

Disassembly of section \.text:

0+ <start>:
[ 	]+0:[ 	]+af1e 0000 0000[ 	]+sub\.d 0 <start>,\$?r1
[ 	]+2:[ 	]+R_CRIS_32_GD	x
[ 	]+6:[ 	]+2f9e 0000 0000[ 	]+add\.d 0 <start>,\$?r9
[ 	]+8:[ 	]+R_CRIS_32_GD	extsym2
[ 	]+c:[ 	]+6f3d 0000 0000 6aaa[ 	]+move\.d \[\$?r3\+0 <start>\],\$?r10
[ 	]+e:[ 	]+R_CRIS_32_TPREL	extsym
[ 	]+14:[ 	]+5fae 0000[ 	]+move\.w 0x0,\$?r10
[ 	]+16:[ 	]+R_CRIS_16_TPREL	extsym14\+0x4d
[ 	]+18:[ 	]+af9e 0000 0000[ 	]+sub\.d 0 <start>,\$?r9
[ 	]+1a:[ 	]+R_CRIS_32_GD	extsym4\+0x2a
[ 	]+1e:[ 	]+af3e 0000 0000[ 	]+sub\.d 0 <start>,\$?r3
[ 	]+20:[ 	]+R_CRIS_32_GD	extsym4-0x60
[ 	]+24:[ 	]+6f3d 0000 0000 67de[ 	]+move\.d \[\$?r7=\$?r3\+0 <start>\],\$?r13
[ 	]+26:[ 	]+R_CRIS_32_GD	extsym10-0x14a
[ 	]+2c:[ 	]+5fbd 0000 699a[ 	]+move\.d \[\$?r11\+0\],\$?r9
[ 	]+2e:[ 	]+R_CRIS_16_TPREL	extsym14-0x100
[ 	]+32:[ 	]+6fad 0000 0000 287a[ 	]+add\.d \[\$?r10\+0 <start>\],\$?r7,\$?r8
[ 	]+34:[ 	]+R_CRIS_32_TPREL	extsym3\+0x38
[ 	]+3a:[ 	]+7f0d 0000 0000 611a[ 	]+move.d \[0 <start>],\$?r1
[ 	]+3c:[ 	]+R_CRIS_32_IE[ 	]+extsym5
[ 	]+42:[ 	]+2fbe 0000 0000[ 	]+add\.d 0 <start>,\$?r11
[ 	]+44:[ 	]+R_CRIS_32_IE[ 	]+extsym7
