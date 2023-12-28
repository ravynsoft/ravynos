#as: --divide
#objdump: -dw
#name: .insn (32-bit code)
#xfail: *-*-darwin*

.*: +file format .*

Disassembly of section .text:

0+ <insn>:
[ 	]*[a-f0-9]+:	90[ 	]+nop
[ 	]*[a-f0-9]+:	f3 90[ 	]+pause
[ 	]*[a-f0-9]+:	f3 90[ 	]+pause
[ 	]*[a-f0-9]+:	d9 ee[ 	]+fldz
[ 	]*[a-f0-9]+:	d9 ee[ 	]+fldz
[ 	]*[a-f0-9]+:	f3 0f 01 e8[ 	]+setssbsy
[ 	]*[a-f0-9]+:	8b c1[ 	]+mov    %ecx,%eax
[ 	]*[a-f0-9]+:	66 8b c8[ 	]+mov    %ax,%cx
[ 	]*[a-f0-9]+:	89 48 04[ 	]+mov    %ecx,0x4\(%eax\)
[ 	]*[a-f0-9]+:	8b 0c 05 44 44 00 00[ 	]+mov    0x4444\(,%eax,1\),%ecx
[ 	]*[a-f0-9]+:	66 0f b6 cc[ 	]+movzbw %ah,%cx
[ 	]*[a-f0-9]+:	0f b7 c8[ 	]+movzwl %ax,%ecx
[ 	]*[a-f0-9]+:	64 f0 80 30 01[ 	]+lock xorb \$(0x)?1,%fs:\(%eax\)
[ 	]*[a-f0-9]+:	0f ca[ 	]+bswap  %edx
[ 	]*[a-f0-9]+:	c7 f8 02 00 00 00[ 	]+xbegin [0-9a-f]+ <insn\+.*>
[ 	]*[a-f0-9]+:	e2 f8[ 	]+loop   [0-9a-f]+ <insn\+.*>
[ 	]*[a-f0-9]+:	c5 fc 77[ 	]+vzeroall
[ 	]*[a-f0-9]+:	c4 e1 7c 77[ 	]+vzeroall
[ 	]*[a-f0-9]+:	c5 f1 58 d0[ 	]+vaddpd %xmm0,%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c5 f5 58 d0[ 	]+vaddpd %ymm0,%ymm1,%ymm2
[ 	]*[a-f0-9]+:	c5 f2 58 d0[ 	]+vaddss %xmm0,%xmm1,%xmm2
[ 	]*[a-f0-9]+:	62 f1 76 08 58 50 01[ 	]+\{evex\} vaddss (0x)?4\(%eax\),%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 69 68 19 00[ 	]+vfmaddps %xmm0,\(%ecx\),%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 e9 68 19 00[ 	]+vfmaddps \(%ecx\),%xmm0,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 e9 68 18 10[ 	]+vfmaddps \(%eax\),%xmm1,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 69 48 19 00[ 	]+vpermil2ps \$(0x)?0,%xmm0,\(%ecx\),%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 e9 48 19 02[ 	]+vpermil2ps \$(0x)?2,\(%ecx\),%xmm0,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 e9 48 18 13[ 	]+vpermil2ps \$(0x)?3,\(%eax\),%xmm1,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c5 f8 92 c8[ 	]+kmovw  %eax,%k1
[ 	]*[a-f0-9]+:	c5 f8 93 c1[ 	]+kmovw  %k1,%eax
[ 	]*[a-f0-9]+:	62 f1 74 18 58 d0[ 	]+vaddps \{rn-sae\},%zmm0,%zmm1,%zmm2
[ 	]*[a-f0-9]+:	c4 e2 79 92 1c 48[ 	]+vgatherdps %xmm0,\(%eax,%xmm1,2\),%xmm3
[ 	]*[a-f0-9]+:	62 f2 fd 0c 93 1c 48[ 	]+vgatherqpd \(%eax,%xmm1,2\),%xmm3\{%k4\}
[ 	]*[a-f0-9]+:	62 f2 7d 28 88 48 01[ 	]+vexpandps (0x)?4\(%eax\),%ymm1
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%eax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%eax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%eax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 7c 48 5a 40 01[ 	]+vcvtph2pd 0x10\(%eax\),%zmm0
[ 	]*[a-f0-9]+:	62 f5 7c 58 5a 40 01[ 	]+vcvtph2pd (0x)?2\(%eax\)\{1to8\},%zmm0
[ 	]*[a-f0-9]+:	62 f3 7d 28 66 40 01 ff[ 	]+vfpclasspsy \$0xff,0x20\(%eax\),%k0
[ 	]*[a-f0-9]+:	62 f3 7d 28 66 40 01 ff[ 	]+vfpclasspsy \$0xff,0x20\(%eax\),%k0
[ 	]*[a-f0-9]+:	62 f3 7d 38 66 40 01 ff[ 	]+vfpclassps \$0xff,(0x)?4\(%eax\)\{1to8\},%k0
[ 	]*[a-f0-9]+:	62 f3 7d 38 66 40 01 ff[ 	]+vfpclassps \$0xff,(0x)?4\(%eax\)\{1to8\},%k0
#pass
