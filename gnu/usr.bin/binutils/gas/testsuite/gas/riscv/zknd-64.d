#as: -march=rv64i_zknd
#source: zknd-64.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+3ac58533[ 	]+aes64ds[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+3ec58533[ 	]+aes64dsm[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+30051513[ 	]+aes64im[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+31459513[ 	]+aes64ks1i[ 	]+a0,a1,0x4
[ 	]+[0-9a-f]+:[ 	]+7ec58533[ 	]+aes64ks2[ 	]+a0,a1,a2
