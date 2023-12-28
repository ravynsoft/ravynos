#as: -mevexrcig=ru
#objdump: -dw -Mintel
#name: i386 AVX512DQ rcig insns (Intel disassembly)
#source: avx512dq-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 f5 ab[ 	]*vreducepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 f5 7b[ 	]*vreducepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 f5 ab[ 	]*vreduceps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 f5 7b[ 	]*vreduceps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a f5[ 	]*vcvttpd2qq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 f5 ab[ 	]*vreducepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 f5 7b[ 	]*vreducepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 f5 ab[ 	]*vreduceps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 f5 7b[ 	]*vreduceps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a f5[ 	]*vcvttpd2qq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5\{sae\}
#pass
