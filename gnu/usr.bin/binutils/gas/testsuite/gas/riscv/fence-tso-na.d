#as: -march=rv32ic
#source: fence-tso.s
#objdump: -dr -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+8330000f[ 	]+fence.tso
#pass
