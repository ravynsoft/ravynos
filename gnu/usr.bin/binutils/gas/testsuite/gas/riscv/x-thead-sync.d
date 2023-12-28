#as: -march=rv64i_xtheadsync
#source: x-thead-sync.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+04b5000b[ 	]+th.sfence.vmas[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+0180000b[ 	]+th.sync
[ 	]+[0-9a-f]+:[ 	]+01a0000b[ 	]+th.sync.i
[ 	]+[0-9a-f]+:[ 	]+01b0000b[ 	]+th.sync.is
[ 	]+[0-9a-f]+:[ 	]+0190000b[ 	]+th.sync.s
