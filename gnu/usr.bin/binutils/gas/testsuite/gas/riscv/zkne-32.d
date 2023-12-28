#as: -march=rv32i_zkne
#source: zkne-32.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+a2c58533[ 	]+aes32esi[ 	]+a0,a1,a2,0x2
[ 	]+[0-9a-f]+:[ 	]+a6c58533[ 	]+aes32esmi[ 	]+a0,a1,a2,0x2
