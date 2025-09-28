#as: -march=rv64i_xtheadba
#source: x-thead-ba.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+00c5950b[ 	]+th.addsl[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+02c5950b[ 	]+th.addsl[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+04c5950b[ 	]+th.addsl[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+06c5950b[ 	]+th.addsl[ 	]+a0,a1,a2,3
