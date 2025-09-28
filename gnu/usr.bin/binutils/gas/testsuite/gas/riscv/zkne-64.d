#as: -march=rv64i_zkne
#source: zkne-64.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+32c58533[ 	]+aes64es[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+36c58533[ 	]+aes64esm[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+31459513[ 	]+aes64ks1i[ 	]+a0,a1,0x4
[ 	]+[0-9a-f]+:[ 	]+7ec58533[ 	]+aes64ks2[ 	]+a0,a1,a2
