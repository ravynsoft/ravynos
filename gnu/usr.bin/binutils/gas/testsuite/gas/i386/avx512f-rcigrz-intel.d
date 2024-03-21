#as: -mevexrcig=rz
#objdump: -dw -Mintel
#name: i386 AVX512F rcig insns (Intel disassembly)
#source: avx512f-rcig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 78 c2 ed ab[ 	]*vcmppd k5,zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 78 c2 ed 7b[ 	]*vcmppd k5,zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 78 c2 ed ab[ 	]*vcmpps k5,zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 78 c2 ed 7b[ 	]*vcmpps k5,zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f c2 ec ab[ 	]*vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f c2 ec 7b[ 	]*vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f c2 ec ab[ 	]*vcmpss k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f c2 ec 7b[ 	]*vcmpss k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 2f f5[ 	]*vcomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 2f f5[ 	]*vcomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 7f 13 f5[ 	]*vcvtph2ps zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 7f 5a f5[ 	]*vcvtps2pd zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 7f 1d ee ab[ 	]*vcvtps2ph ymm6\{k7\},zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 7f 1d ee 7b[ 	]*vcvtps2ph ymm6\{k7\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5a f4[ 	]*vcvtss2sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 7f e6 f5[ 	]*vcvttpd2dq ymm6\{k7\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 5b f5[ 	]*vcvttps2dq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 2c c6[ 	]*vcvttsd2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 2c ee[ 	]*vcvttsd2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 2c c6[ 	]*vcvttss2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 2c ee[ 	]*vcvttss2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 78 42 f5[ 	]*vgetexppd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 78 42 f5[ 	]*vgetexpps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 7f 43 f4[ 	]*vgetexpsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 7f 43 f4[ 	]*vgetexpss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 26 f5 ab[ 	]*vgetmantpd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 26 f5 7b[ 	]*vgetmantpd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 26 f5 ab[ 	]*vgetmantps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 26 f5 7b[ 	]*vgetmantps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 27 f4 ab[ 	]*vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 27 f4 7b[ 	]*vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 27 f4 ab[ 	]*vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 27 f4 7b[ 	]*vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 78 5f f4[ 	]*vmaxpd zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 78 5f f4[ 	]*vmaxps zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f 5f f4[ 	]*vmaxsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5f f4[ 	]*vmaxss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 78 5d f4[ 	]*vminpd zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 78 5d f4[ 	]*vminps zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f 5d f4[ 	]*vminsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5d f4[ 	]*vminss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 2e f5[ 	]*vucomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 2e f5[ 	]*vucomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 78 54 f4 ab[ 	]*vfixupimmpd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 78 54 f4 7b[ 	]*vfixupimmpd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 78 54 f4 ab[ 	]*vfixupimmps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 78 54 f4 7b[ 	]*vfixupimmps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 55 f4 ab[ 	]*vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 55 f4 7b[ 	]*vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 55 f4 ab[ 	]*vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 55 f4 7b[ 	]*vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 09 f5 ab[ 	]*vrndscalepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 09 f5 7b[ 	]*vrndscalepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 08 f5 ab[ 	]*vrndscaleps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 08 f5 7b[ 	]*vrndscaleps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 0b f4 ab[ 	]*vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 0b f4 7b[ 	]*vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 0a f4 ab[ 	]*vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 0a f4 7b[ 	]*vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 78 f5[ 	]*vcvttpd2udq ymm6\{k7\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 78 f5[ 	]*vcvttps2udq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 78 c6[ 	]*vcvttsd2usi eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 78 ee[ 	]*vcvttsd2usi ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 78 c6[ 	]*vcvttss2usi eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 78 ee[ 	]*vcvttss2usi ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 78 c2 ed ab[ 	]*vcmppd k5,zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 78 c2 ed 7b[ 	]*vcmppd k5,zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 78 c2 ed ab[ 	]*vcmpps k5,zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4c 78 c2 ed 7b[ 	]*vcmpps k5,zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f c2 ec ab[ 	]*vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f c2 ec 7b[ 	]*vcmpsd k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f c2 ec ab[ 	]*vcmpss k5\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f c2 ec 7b[ 	]*vcmpss k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 2f f5[ 	]*vcomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 2f f5[ 	]*vcomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 7f 13 f5[ 	]*vcvtph2ps zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 7f 5a f5[ 	]*vcvtps2pd zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 7f 1d ee ab[ 	]*vcvtps2ph ymm6\{k7\},zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 7f 1d ee 7b[ 	]*vcvtps2ph ymm6\{k7\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5a f4[ 	]*vcvtss2sd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 7f e6 f5[ 	]*vcvttpd2dq ymm6\{k7\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 5b f5[ 	]*vcvttps2dq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 2c c6[ 	]*vcvttsd2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 2c ee[ 	]*vcvttsd2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 2c c6[ 	]*vcvttss2si eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 2c ee[ 	]*vcvttss2si ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 78 42 f5[ 	]*vgetexppd zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 78 42 f5[ 	]*vgetexpps zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 7f 43 f4[ 	]*vgetexpsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 7f 43 f4[ 	]*vgetexpss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 26 f5 ab[ 	]*vgetmantpd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 26 f5 7b[ 	]*vgetmantpd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 26 f5 ab[ 	]*vgetmantps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 26 f5 7b[ 	]*vgetmantps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 27 f4 ab[ 	]*vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 27 f4 7b[ 	]*vgetmantsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 27 f4 ab[ 	]*vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 27 f4 7b[ 	]*vgetmantss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 78 5f f4[ 	]*vmaxpd zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 78 5f f4[ 	]*vmaxps zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f 5f f4[ 	]*vmaxsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5f f4[ 	]*vmaxss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 78 5d f4[ 	]*vminpd zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 78 5d f4[ 	]*vminps zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 d7 7f 5d f4[ 	]*vminsd xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 56 7f 5d f4[ 	]*vminss xmm6\{k7\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 2e f5[ 	]*vucomisd xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 2e f5[ 	]*vucomiss xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 78 54 f4 ab[ 	]*vfixupimmpd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 78 54 f4 7b[ 	]*vfixupimmpd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 78 54 f4 ab[ 	]*vfixupimmps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 78 54 f4 7b[ 	]*vfixupimmps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 55 f4 ab[ 	]*vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 55 f4 7b[ 	]*vfixupimmsd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 55 f4 ab[ 	]*vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 55 f4 7b[ 	]*vfixupimmss xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 09 f5 ab[ 	]*vrndscalepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 78 09 f5 7b[ 	]*vrndscalepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 08 f5 ab[ 	]*vrndscaleps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 78 08 f5 7b[ 	]*vrndscaleps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 0b f4 ab[ 	]*vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 7f 0b f4 7b[ 	]*vrndscalesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 0a f4 ab[ 	]*vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 7f 0a f4 7b[ 	]*vrndscaless xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 78 f5[ 	]*vcvttpd2udq ymm6\{k7\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7c 78 78 f5[ 	]*vcvttps2udq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 78 c6[ 	]*vcvttsd2usi eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 78 78 ee[ 	]*vcvttsd2usi ebp,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 78 c6[ 	]*vcvttss2usi eax,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 78 78 ee[ 	]*vcvttss2usi ebp,xmm6\{sae\}
#pass
