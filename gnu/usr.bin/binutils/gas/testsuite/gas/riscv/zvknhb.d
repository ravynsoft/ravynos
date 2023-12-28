#as: -march=rv64gc_zvknhb
#source: zvknha_zvknhb.s
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+ba862277[ 	]+vsha2ch.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+be862277[ 	]+vsha2cl.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+b6862277[ 	]+vsha2ms.vv[ 	]+v4,v8,v12
