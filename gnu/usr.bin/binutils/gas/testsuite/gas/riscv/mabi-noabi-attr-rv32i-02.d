#as: -march-attr -march=rv64id
#objdump: -d
#source: mabi-attr-rv32i.s

.*:[ 	]+file format elf32.*


Disassembly of section .text:

0+000 <foo>:
#...
[ 	]+[0-9a-f]+:[ 	]+00052503[ 	]+lw[ 	]+a0,0\(a0\) # .*
