#as: -mevexrcig=ru
#objdump: -dw
#name: i386 AVX512F rcig insns
#source: avx512f-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 58 c2 ed ab[ 	]*vcmppd \$0xab,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 58 c2 ed 7b[ 	]*vcmppd \$0x7b,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 58 c2 ed ab[ 	]*vcmpps \$0xab,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 58 c2 ed 7b[ 	]*vcmpps \$0x7b,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f c2 ec ab[ 	]*vcmpsd \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f c2 ec 7b[ 	]*vcmpsd \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f c2 ec ab[ 	]*vcmpss \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f c2 ec 7b[ 	]*vcmpss \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 2f f5[ 	]*vcomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 2f f5[ 	]*vcomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 5f 13 f5[ 	]*vcvtph2ps \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 5f 5a f5[ 	]*vcvtps2pd \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 5f 1d ee ab[ 	]*vcvtps2ph \$0xab,\{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 5f 1d ee 7b[ 	]*vcvtps2ph \$0x7b,\{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5a f4[ 	]*vcvtss2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 5f e6 f5[ 	]*vcvttpd2dq \{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 5b f5[ 	]*vcvttps2dq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 2c c6[ 	]*vcvttsd2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 2c ee[ 	]*vcvttsd2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 2c c6[ 	]*vcvttss2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 2c ee[ 	]*vcvttss2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 42 f5[ 	]*vgetexppd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 42 f5[ 	]*vgetexpps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 5f 43 f4[ 	]*vgetexpsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 5f 43 f4[ 	]*vgetexpss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 26 f5 ab[ 	]*vgetmantpd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 26 f5 7b[ 	]*vgetmantpd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 26 f5 ab[ 	]*vgetmantps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 26 f5 7b[ 	]*vgetmantps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 27 f4 ab[ 	]*vgetmantsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 27 f4 7b[ 	]*vgetmantsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 27 f4 ab[ 	]*vgetmantss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 27 f4 7b[ 	]*vgetmantss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 5f f4[ 	]*vmaxpd \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 5f f4[ 	]*vmaxps \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f 5f f4[ 	]*vmaxsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5f f4[ 	]*vmaxss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 5d f4[ 	]*vminpd \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 5d f4[ 	]*vminps \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f 5d f4[ 	]*vminsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5d f4[ 	]*vminss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 2e f5[ 	]*vucomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 2e f5[ 	]*vucomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 54 f4 ab[ 	]*vfixupimmpd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 54 f4 7b[ 	]*vfixupimmpd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 54 f4 ab[ 	]*vfixupimmps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 54 f4 7b[ 	]*vfixupimmps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 55 f4 ab[ 	]*vfixupimmsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 55 f4 7b[ 	]*vfixupimmsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 55 f4 ab[ 	]*vfixupimmss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 55 f4 7b[ 	]*vfixupimmss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 09 f5 ab[ 	]*vrndscalepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 09 f5 7b[ 	]*vrndscalepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 08 f5 ab[ 	]*vrndscaleps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 08 f5 7b[ 	]*vrndscaleps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 0b f4 ab[ 	]*vrndscalesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 0b f4 7b[ 	]*vrndscalesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 0a f4 ab[ 	]*vrndscaless \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 0a f4 7b[ 	]*vrndscaless \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 78 f5[ 	]*vcvttpd2udq \{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 78 f5[ 	]*vcvttps2udq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 78 c6[ 	]*vcvttss2usi \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 78 ee[ 	]*vcvttss2usi \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 58 c2 ed ab[ 	]*vcmppd \$0xab,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 58 c2 ed 7b[ 	]*vcmppd \$0x7b,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 58 c2 ed ab[ 	]*vcmpps \$0xab,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 58 c2 ed 7b[ 	]*vcmpps \$0x7b,\{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f c2 ec ab[ 	]*vcmpsd \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f c2 ec 7b[ 	]*vcmpsd \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f c2 ec ab[ 	]*vcmpss \$0xab,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f c2 ec 7b[ 	]*vcmpss \$0x7b,\{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 2f f5[ 	]*vcomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 2f f5[ 	]*vcomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 5f 13 f5[ 	]*vcvtph2ps \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 5f 5a f5[ 	]*vcvtps2pd \{sae\},%ymm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 5f 1d ee ab[ 	]*vcvtps2ph \$0xab,\{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 5f 1d ee 7b[ 	]*vcvtps2ph \$0x7b,\{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5a f4[ 	]*vcvtss2sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 5f e6 f5[ 	]*vcvttpd2dq \{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 5b f5[ 	]*vcvttps2dq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 2c c6[ 	]*vcvttsd2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 2c ee[ 	]*vcvttsd2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 2c c6[ 	]*vcvttss2si \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 2c ee[ 	]*vcvttss2si \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 58 42 f5[ 	]*vgetexppd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 58 42 f5[ 	]*vgetexpps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 5f 43 f4[ 	]*vgetexpsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 5f 43 f4[ 	]*vgetexpss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 26 f5 ab[ 	]*vgetmantpd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 26 f5 7b[ 	]*vgetmantpd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 26 f5 ab[ 	]*vgetmantps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 26 f5 7b[ 	]*vgetmantps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 27 f4 ab[ 	]*vgetmantsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 27 f4 7b[ 	]*vgetmantsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 27 f4 ab[ 	]*vgetmantss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 27 f4 7b[ 	]*vgetmantss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 5f f4[ 	]*vmaxpd \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 5f f4[ 	]*vmaxps \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f 5f f4[ 	]*vmaxsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5f f4[ 	]*vmaxss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 5d f4[ 	]*vminpd \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 5d f4[ 	]*vminps \{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 5f 5d f4[ 	]*vminsd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 5f 5d f4[ 	]*vminss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 2e f5[ 	]*vucomisd \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 2e f5[ 	]*vucomiss \{sae\},%xmm5,%xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 54 f4 ab[ 	]*vfixupimmpd \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 54 f4 7b[ 	]*vfixupimmpd \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 54 f4 ab[ 	]*vfixupimmps \$0xab,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 54 f4 7b[ 	]*vfixupimmps \$0x7b,\{sae\},%zmm4,%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 55 f4 ab[ 	]*vfixupimmsd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 55 f4 7b[ 	]*vfixupimmsd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 55 f4 ab[ 	]*vfixupimmss \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 55 f4 7b[ 	]*vfixupimmss \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 09 f5 ab[ 	]*vrndscalepd \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 09 f5 7b[ 	]*vrndscalepd \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 08 f5 ab[ 	]*vrndscaleps \$0xab,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 08 f5 7b[ 	]*vrndscaleps \$0x7b,\{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 0b f4 ab[ 	]*vrndscalesd \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 5f 0b f4 7b[ 	]*vrndscalesd \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 0a f4 ab[ 	]*vrndscaless \$0xab,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 5f 0a f4 7b[ 	]*vrndscaless \$0x7b,\{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 78 f5[ 	]*vcvttpd2udq \{sae\},%zmm5,%ymm6\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 58 78 f5[ 	]*vcvttps2udq \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 78 c6[ 	]*vcvttsd2usi \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 58 78 ee[ 	]*vcvttsd2usi \{sae\},%xmm6,%ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 78 c6[ 	]*vcvttss2usi \{sae\},%xmm6,%eax
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 58 78 ee[ 	]*vcvttss2usi \{sae\},%xmm6,%ebp
#pass
