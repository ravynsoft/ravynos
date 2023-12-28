#as: -march=rv32i_xtheadint
#source: x-thead-int.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+0040000b[ 	]+th.ipush
[ 	]+[0-9a-f]+:[ 	]+0050000b[ 	]+th.ipop
