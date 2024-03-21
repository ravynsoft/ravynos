#as:
#objdump: -drw
#name: x86_64 Illegal AMX-COMPLEX insns
#source: x86-64-amx-complex-bad.s

.*: +file format .*


Disassembly of section \.text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d9 6c[ 	]*\(bad\)[ 	]*
[ 	]*[a-f0-9]+:[ 	]*f5[ 	]*cmc.*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 5d 6c[ 	]*\(bad\)[ 	]*
[ 	]*[a-f0-9]+:[ 	]*f5[ 	]*cmc.*
[ 	]*[a-f0-9]+:[ 	]*c4 62 59 6c f5[ 	]*tcmmimfp16ps %tmm4,%tmm5,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 c2 59 6c f5[ 	]*tcmmimfp16ps %tmm4,\(bad\),%tmm6
[ 	]*[a-f0-9]+:[ 	]*c4 e2 31 6c f5[ 	]*tcmmimfp16ps \(bad\),%tmm5,%tmm6
#pass
