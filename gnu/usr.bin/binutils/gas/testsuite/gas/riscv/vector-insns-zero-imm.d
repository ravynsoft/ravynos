#as: -march=rv32iv
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+768fb257[ 	]+vmsle.vi[ 	]+v4,v8,-1
[ 	]+[0-9a-f]+:[ 	]+748fb257[ 	]+vmsle.vi[ 	]+v4,v8,-1,v0.t
[ 	]+[0-9a-f]+:[ 	]+66840257[ 	]+vmsne.vv[ 	]+v4,v8,v8
[ 	]+[0-9a-f]+:[ 	]+64840257[ 	]+vmsne.vv[ 	]+v4,v8,v8,v0.t
[ 	]+[0-9a-f]+:[ 	]+7e8fb257[ 	]+vmsgt.vi[ 	]+v4,v8,-1
[ 	]+[0-9a-f]+:[ 	]+7c8fb257[ 	]+vmsgt.vi[ 	]+v4,v8,-1,v0.t
[ 	]+[0-9a-f]+:[ 	]+62840257[ 	]+vmseq.vv[ 	]+v4,v8,v8
[ 	]+[0-9a-f]+:[ 	]+60840257[ 	]+vmseq.vv[ 	]+v4,v8,v8,v0.t
