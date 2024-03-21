#objdump: -dw
#name: i386 AVX512F YMM registers

.*: +file format .*


Disassembly of section \.text:

00000000 <ymm>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 31 c0[ 	]*vpmovzxbd %xmm0,%zmm0
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 33 c0[ 	]*vpmovzxwd %ymm0,%zmm0
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 48 5a c0[ 	]*vcvtps2pd %ymm0,%zmm0
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 5a c0[ 	]*vcvtpd2ps %zmm0,%ymm0
#pass
