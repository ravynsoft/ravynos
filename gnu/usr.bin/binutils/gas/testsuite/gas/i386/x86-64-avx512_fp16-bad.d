#as:
#objdump: -drw
#name: x86_64 AVX512_FP16 DAD insns
#source: x86-64-avx512_fp16-bad.s

.*: +file format .*


Disassembly of section \.text:

0+ <\.text>:
[ 	]*[a-f0-9]+:[ 	]*62 06 17 40 56 f6[ 	]*vfcmaddcph %zmm30,%zmm29,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*62 f6 67 48 56 19[ 	]*vfcmaddcph \(%rcx\),%zmm3,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*62 f6 6f 08 56 d3[ 	]*vfcmaddcph %xmm3,%xmm2,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*62 f6 6f 08 57 db[ 	]*vfcmaddcsh %xmm3,%xmm2,\(bad\)
[ 	]*[a-f0-9]+:[ 	]*62 f6 6f 08 57 d3[ 	]*vfcmaddcsh %xmm3,%xmm2,\(bad\)
#pass
