#as:
#objdump: -drw
#name: x86_64 Illegal AMX insns
#source: x86-64-amx-bad.s

.*: +file format .*


Disassembly of section \.text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 d2 5c[ 	]*\(bad\)
[ 	]*[a-f0-9]+:[ 	]*dc 90 90 90 90 90[ 	]*fcoml.*
[ 	]*[a-f0-9]+:[ 	]*c4 e2 56 5c[ 	]*\(bad\)
[ 	]*[a-f0-9]+:[ 	]*dc 90 90 90 90 90[ 	]*fcoml.*
[ 	]*[a-f0-9]+:[ 	]*c4 62 52 5c dc[ 	]*tdpbf16ps %tmm5,%tmm4,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 c2 52 5c dc[ 	]*tdpbf16ps %tmm5,\(bad\),%tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 32 5c dc[ 	]*tdpbf16ps \(bad\),%tmm4,%tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 7b 4b 09[ 	]*tileloadd \(bad\),%tmm1
[ 	]*[a-f0-9]+:[ 	]*c4 e2 70 5e c9[ 	]*tdpbuud %tmm1/\(bad\),%tmm1/\(bad\),%tmm1/\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 e2 78 5e c9[ 	]*tdpbuud %tmm0,%tmm1/\(bad\),%tmm1/\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 e2 70 5e c8[ 	]*tdpbuud %tmm1/\(bad\),%tmm0,%tmm1/\(bad\)
[ 	]*[a-f0-9]+:[ 	]*c4 e2 70 5e c1[ 	]*tdpbuud %tmm1/\(bad\),%tmm1/\(bad\),%tmm0
#pass
