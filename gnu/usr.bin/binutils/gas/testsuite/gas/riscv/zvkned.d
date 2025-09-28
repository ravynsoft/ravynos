#as: -march=rv64gc_zvkned
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+a280a277[ 	]+vaesdf.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a680a277[ 	]+vaesdf.vs[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a2802277[ 	]+vaesdm.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a6802277[ 	]+vaesdm.vs[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a281a277[ 	]+vaesef.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a681a277[ 	]+vaesef.vs[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a2812277[ 	]+vaesem.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+a6812277[ 	]+vaesem.vs[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+8a812277[ 	]+vaeskf1.vi[ 	]+v4,v8,2
[ 	]+[0-9a-f]+:[ 	]+8a872277[ 	]+vaeskf1.vi[ 	]+v4,v8,14
[ 	]+[0-9a-f]+:[ 	]+aa812277[ 	]+vaeskf2.vi[ 	]+v4,v8,2
[ 	]+[0-9a-f]+:[ 	]+aa872277[ 	]+vaeskf2.vi[ 	]+v4,v8,14
[ 	]+[0-9a-f]+:[ 	]+a683a277[ 	]+vaesz.vs[ 	]+v4,v8
