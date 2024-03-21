#as: -mevexrcig=rd
#objdump: -dw
#name: i386 AVX512DQ rcig insns
#source: avx512dq-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 38 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 38 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 38 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 38 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a f5[ 	]*vcvttps2qq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 38 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 38 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 38 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 38 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a f5[ 	]*vcvttps2qq \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm5,%zmm6\{%k7\}
#pass
