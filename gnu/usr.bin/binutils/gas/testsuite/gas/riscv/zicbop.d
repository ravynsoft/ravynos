#as: -march=rv64g_zicbop
#source: zicbop.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+0200e013[ 	]+prefetch\.i[ 	]+32\(ra\)
[ 	]+[0-9a-f]+:[ 	]+80186013[ 	]+prefetch\.r[ 	]+-2048\(a6\)
[ 	]+[0-9a-f]+:[ 	]+7e3fe013[ 	]+prefetch\.w[ 	]+2016\(t6\)
