#as:
#objdump: -dw
#name: x86_64 AMX-FP16 insns
#source: x86-64-amx-fp16.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 53 5c dc[ 	]*tdpfp16ps %tmm5,%tmm4,%tmm3
[ 	]*[a-f0-9]+:[ 	]*c4 e2 53 5c dc[ 	]*tdpfp16ps %tmm5,%tmm4,%tmm3
#pass
