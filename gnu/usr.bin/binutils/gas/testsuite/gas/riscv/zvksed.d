#as: -march=rv64gc_zvksed
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+86802277[ 	]+vsm4k.vi[ 	]+v4,v8,0
[ 	]+[0-9a-f]+:[ 	]+8683a277[ 	]+vsm4k.vi[ 	]+v4,v8,7
[ 	]+[0-9a-f]+:[ 	]+a2882277[ 	]+vsm4r.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a6882277[ 	]+vsm4r.vs[ 	]+v4,v8
