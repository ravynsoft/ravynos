#as:
#objdump: -drw
#name: x86_64 Illegal AMX-FP16 insns
#source: x86-64-amx-fp16-bad.s

.*: +file format .*


Disassembly of section \.text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d3 5c[ 	]*\(bad\)[ 	]*
[ 	]*[a-f0-9]+:[ 	]*dc 90 90 90 90 90[ 	]*fcoml.*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 57 5c[ 	]*\(bad\)[ 	]*
[ 	]*[a-f0-9]+:[ 	]*dc 90 90 90 90 90[ 	]*fcoml.*
[ 	]*[a-f0-9]+:[ 	]*c4 62 53 5c dc[ 	]*tdpfp16ps %tmm5,%tmm4,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 c2 53 5c dc[ 	]*tdpfp16ps %tmm5,\(bad\),%tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 33 5c dc[ 	]*tdpfp16ps \(bad\),%tmm4,%tmm3
#pass
