#as: -march=rv64i_zawrs
#source: zawrs.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+00d00073[ 	]+wrs.nto
[ 	]+[0-9a-f]+:[ 	]+01d00073[ 	]+wrs.sto
