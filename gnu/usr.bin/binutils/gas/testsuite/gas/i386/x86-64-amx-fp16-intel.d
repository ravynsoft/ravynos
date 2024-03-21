#as:
#objdump: -d -Mintel
#name: x86_64 AMX-FP16 insns (Intel disassembly)
#source: x86-64-amx-fp16.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 53 5c dc[ 	]*tdpfp16ps tmm3,tmm4,tmm5
[ 	]*[a-f0-9]+:[ 	]*c4 e2 53 5c dc[ 	]*tdpfp16ps tmm3,tmm4,tmm5
#pass
