#as: -march=rv32iv
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+6e85c257[ 	]+vmslt.vx[ 	]+v4,v8,a1
[ 	]+[0-9a-f]+:[ 	]+76422257[ 	]+vmnot.m[ 	]+v4,v4
[ 	]+[0-9a-f]+:[ 	]+6cc64457[ 	]+vmslt.vx[ 	]+v8,v12,a2,v0.t
[ 	]+[0-9a-f]+:[ 	]+6e802457[ 	]+vmxor.mm[ 	]+v8,v8,v0
[ 	]+[0-9a-f]+:[ 	]+6c85c657[ 	]+vmslt.vx[ 	]+v12,v8,a1,v0.t
[ 	]+[0-9a-f]+:[ 	]+62062057[ 	]+vmandn.mm[ 	]+v0,v0,v12
[ 	]+[0-9a-f]+:[ 	]+6c85c657[ 	]+vmslt.vx[ 	]+v12,v8,a1,v0.t
[ 	]+[0-9a-f]+:[ 	]+62062657[ 	]+vmandn.mm[ 	]+v12,v0,v12
[ 	]+[0-9a-f]+:[ 	]+62402257[ 	]+vmandn.mm[ 	]+v4,v4,v0
[ 	]+[0-9a-f]+:[ 	]+6ac22257[ 	]+vmor.mm[ 	]+v4,v12,v4
[ 	]+[0-9a-f]+:[ 	]+6a85c257[ 	]+vmsltu.vx[ 	]+v4,v8,a1
[ 	]+[0-9a-f]+:[ 	]+76422257[ 	]+vmnot.m[ 	]+v4,v4
[ 	]+[0-9a-f]+:[ 	]+68c64457[ 	]+vmsltu.vx[ 	]+v8,v12,a2,v0.t
[ 	]+[0-9a-f]+:[ 	]+6e802457[ 	]+vmxor.mm[ 	]+v8,v8,v0
[ 	]+[0-9a-f]+:[ 	]+6885c657[ 	]+vmsltu.vx[ 	]+v12,v8,a1,v0.t
[ 	]+[0-9a-f]+:[ 	]+62062057[ 	]+vmandn.mm[ 	]+v0,v0,v12
[ 	]+[0-9a-f]+:[ 	]+6885c657[ 	]+vmsltu.vx[ 	]+v12,v8,a1,v0.t
[ 	]+[0-9a-f]+:[ 	]+62062657[ 	]+vmandn.mm[ 	]+v12,v0,v12
[ 	]+[0-9a-f]+:[ 	]+62402257[ 	]+vmandn.mm[ 	]+v4,v4,v0
[ 	]+[0-9a-f]+:[ 	]+6ac22257[ 	]+vmor.mm[ 	]+v4,v12,v4
