#as: -march=rv64i_xtheadbs
#source: x-thead-bs.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+8805950b[ 	]+th.tst[ 	]+a0,a1,0
[ 	]+[0-9a-f]+:[ 	]+8815950b[ 	]+th.tst[ 	]+a0,a1,1
[ 	]+[0-9a-f]+:[ 	]+89f5950b[ 	]+th.tst[ 	]+a0,a1,31
[ 	]+[0-9a-f]+:[ 	]+8a05950b[ 	]+th.tst[ 	]+a0,a1,32
[ 	]+[0-9a-f]+:[ 	]+8bf5950b[ 	]+th.tst[ 	]+a0,a1,63
