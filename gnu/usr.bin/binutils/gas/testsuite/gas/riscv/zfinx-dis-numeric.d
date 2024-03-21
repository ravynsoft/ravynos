#as: -march=rv64ima_zfinx
#source: zfinx-dis-numeric.s
#objdump: -dr -Mnumeric

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+a0c5a553[ 	]+feq.s[ 	]+x10,x11,x12
