#as: -march=rv32i_xtheadfmv
#source: x-thead-fmv.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+5005950b[ 	]+th.fmv.hw.x[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+6005158b[ 	]+th.fmv.x.hw[ 	]+a1,fa0
