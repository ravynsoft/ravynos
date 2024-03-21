#as: -march=rv32i
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
#...
[ 	]+40:[ 	]+00000017[ 	]+auipc[ 	]+zero,0x0
[ 	]+44:[ 	]+00002003[ 	]+lw[ 	]+zero,0\(zero\) # 0 .*
