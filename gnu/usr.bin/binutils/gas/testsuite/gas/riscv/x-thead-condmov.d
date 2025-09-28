#as: -march=rv64i_xtheadcondmov
#source: x-thead-condmov.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+40c5950b[ 	]+th.mveqz[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+42c5950b[ 	]+th.mvnez[ 	]+a0,a1,a2
