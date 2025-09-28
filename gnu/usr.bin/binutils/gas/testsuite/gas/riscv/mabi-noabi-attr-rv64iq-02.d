#as: -march-attr -march=rv32i
#objdump: -d
#source: mabi-attr-rv64iq.s

.*:[ 	]+file format elf64.*


Disassembly of section .text:

0+000 <foo>:
#...
[ 	]+[0-9a-f]+:[ 	]+00053503[ 	]+ld[ 	]+a0,0\(a0\) # .*
