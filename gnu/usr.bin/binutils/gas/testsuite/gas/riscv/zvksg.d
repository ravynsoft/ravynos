#as: -march=rv64gc_zvksg
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+86802277[ 	]+vsm4k.vi[ 	]+v4,v8,0
[ 	]+[0-9a-f]+:[ 	]+ae802277[ 	]+vsm3c.vi[ 	]+v4,v8,0
[ 	]+[0-9a-f]+:[ 	]+b2862277[ 	]+vghsh.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+a2c8a277[ 	]+vgmul.vv[ 	]+v4,v12
