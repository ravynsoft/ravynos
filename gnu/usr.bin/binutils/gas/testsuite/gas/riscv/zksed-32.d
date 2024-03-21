#as: -march=rv32i_zksed
#source: zksed.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+b0c58533[ 	]+sm4ed[ 	]+a0,a1,a2,0x2
[ 	]+[0-9a-f]+:[ 	]+b4c58533[ 	]+sm4ks[ 	]+a0,a1,a2,0x2
