#as: -march=rv64i_xtheadmac
#source: x-thead-mac.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+20c5950b[ 	]+th.mula[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+28c5950b[ 	]+th.mulah[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+24c5950b[ 	]+th.mulaw[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+22c5950b[ 	]+th.muls[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+2ac5950b[ 	]+th.mulsh[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+26c5950b[ 	]+th.mulsw[ 	]+a0,a1,a2
