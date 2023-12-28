#as: -march=rv64gc_zvkg
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+b2862277[ 	]+vghsh.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+a2c8a277[ 	]+vgmul.vv[ 	]+v4,v12
