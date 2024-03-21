#as: -mevexrcig=rz
#objdump: -dw
#name: x86_64 AVX512DQ rcig insns
#source: x86-64-avx512dq-rcig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 78 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 78 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 78 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 78 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7a f5[ 	]*vcvttps2qq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 50 f4 ab[ 	]*vrangepd \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 50 f4 7b[ 	]*vrangepd \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 50 f4 ab[ 	]*vrangeps \$0xab,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 50 f4 7b[ 	]*vrangeps \$0x7b,\{sae\},%zmm28,%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 51 f4 ab[ 	]*vrangesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 51 f4 7b[ 	]*vrangesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 51 f4 ab[ 	]*vrangess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 51 f4 7b[ 	]*vrangess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 78 56 f5 ab[ 	]*vreducepd \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 78 56 f5 7b[ 	]*vreducepd \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 78 56 f5 ab[ 	]*vreduceps \$0xab,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 78 56 f5 7b[ 	]*vreduceps \$0x7b,\{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 57 f4 ab[ 	]*vreducesd \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 95 70 57 f4 7b[ 	]*vreducesd \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 57 f4 ab[ 	]*vreducess \$0xab,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 03 15 70 57 f4 7b[ 	]*vreducess \$0x7b,\{sae\},%xmm28,%xmm29,%xmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7a f5[ 	]*vcvttpd2qq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 78 f5[ 	]*vcvttpd2uqq \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7a f5[ 	]*vcvttps2qq \{sae\},%ymm29,%zmm30
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 78 f5[ 	]*vcvttps2uqq \{sae\},%ymm29,%zmm30
#pass
