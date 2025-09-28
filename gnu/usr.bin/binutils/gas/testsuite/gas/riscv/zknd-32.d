#as: -march=rv32i_zknd
#source: zknd-32.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+aac58533[ 	]+aes32dsi[ 	]+a0,a1,a2,0x2
[ 	]+[0-9a-f]+:[ 	]+aec58533[ 	]+aes32dsmi[ 	]+a0,a1,a2,0x2
