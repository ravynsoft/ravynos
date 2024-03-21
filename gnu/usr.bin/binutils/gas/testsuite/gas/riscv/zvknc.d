#as: -march=rv64gc_zvknc
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:
0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+a280a277[ 	]+vaesdf.vv[ 	]+v4,v8
[ 	]+[0-9a-f]+:[ 	]+ba862277[ 	]+vsha2ch.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+32862257[ 	+vclmul.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+30862257[ 	]+vclmul.vv[ 	]+v4,v8,v12,v0.t
[ 	]+[0-9a-f]+:[ 	]+3285e257[ 	 ]+vclmul.vx[ 	]+v4,v8,a1
[ 	]+[0-9a-f]+:[ 	]+3085e257[ 	 ]+vclmul.vx[ 	]+v4,v8,a1,v0.t
[ 	]+[0-9a-f]+:[ 	]+36862257[ 	 ]+vclmulh.vv[ 	]+v4,v8,v12
[ 	]+[0-9a-f]+:[ 	]+34862257[ 	 ]+vclmulh.vv[ 	]+v4,v8,v12,v0.t
[ 	]+[0-9a-f]+:[ 	]+3685e257[ 	 ]+vclmulh.vx[ 	]+v4,v8,a1
[ 	]+[0-9a-f]+:[ 	]+3485e257[ 	 ]+vclmulh.vx[ 	]+v4,v8,a1,v0.t
