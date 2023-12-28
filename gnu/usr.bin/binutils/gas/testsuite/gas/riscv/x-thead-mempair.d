#as: -march=rv64gc_xtheadmempair
#source: x-thead-mempair.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+f8b6450b[ 	]+th.ldd[ 	]+a0,a1,\(a2\),0,4
[ 	]+[0-9a-f]+:[ 	]+e2b6450b[ 	]+th.lwd[ 	]+a0,a1,\(a2\),1,3
[ 	]+[0-9a-f]+:[ 	]+f4b6450b[ 	]+th.lwud[ 	]+a0,a1,\(a2\),2,3
[ 	]+[0-9a-f]+:[ 	]+feb6550b[ 	]+th.sdd[ 	]+a0,a1,\(a2\),3,4
[ 	]+[0-9a-f]+:[ 	]+e0b6550b[ 	]+th.swd[ 	]+a0,a1,\(a2\),0,3
