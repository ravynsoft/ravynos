#as: --divide
#objdump: -dwr
#name: .insn (64-bit code)
#xfail: *-*-darwin*

.*: +file format .*

Disassembly of section .text:

0+ <insn>:
[ 	]*[a-f0-9]+:	90[ 	]+nop
[ 	]*[a-f0-9]+:	f3 90[ 	]+pause
[ 	]*[a-f0-9]+:	f3 90[ 	]+pause
[ 	]*[a-f0-9]+:	d9 ee[ 	]+fldz
[ 	]*[a-f0-9]+:	f3 0f 01 e8[ 	]+setssbsy
[ 	]*[a-f0-9]+:	44 8b c1[ 	]+mov    %ecx,%r8d
[ 	]*[a-f0-9]+:	48 8b c8[ 	]+mov    %rax,%rcx
[ 	]*[a-f0-9]+:	41 89 48 08[ 	]+mov    %ecx,0x8\(%r8\)
[ 	]*[a-f0-9]+:	42 8b 0c 05 80 80 00 00[ 	]+mov    0x8080\(,%r8,1\),%ecx
[ 	]*[a-f0-9]+:	66 0f be cc[ 	]+movsbw %ah,%cx
[ 	]*[a-f0-9]+:	0f bf c8[ 	]+movswl %ax,%ecx
[ 	]*[a-f0-9]+:	48 63 c8[ 	]+movslq %eax,%rcx
[ 	]*[a-f0-9]+:	f0 80 35 ((00|ff) ){4}01[ 	]+lock xorb \$(0x)?1,[-x01]+\(%rip\) *# .*: (R_X86_64_PC32	lock-(0x)?5|IMAGE_REL_AMD64_REL32	lock)
[ 	]*[a-f0-9]+:	48 0f ca[ 	]+bswap  %rdx
[ 	]*[a-f0-9]+:	41 0f c8[ 	]+bswap  %r8d
[ 	]*[a-f0-9]+:	c7 f8 02 00 00 00[ 	]+xbegin [0-9a-f]+ <insn\+.*>
[ 	]*[a-f0-9]+:	e2 f8[ 	]+loop   [0-9a-f]+ <insn\+.*>
[ 	]*[a-f0-9]+:	05 00 00 00 00[ 	]+add    \$(0x)?0,%eax	.*: (R_X86_64_32|IMAGE_REL_AMD64_ADDR32)	var
[ 	]*[a-f0-9]+:	48 05 00 00 00 00[ 	]+add    \$(0x)?0,%rax	.*: R_X86_64_32S	var
[ 	]*[a-f0-9]+:	81 3d (00|fc) ((00|ff) ){3}13 12 23 21[ 	]+cmpl   \$0x21231213,[-x04]+\(%rip\) *# .*: (R_X86_64_PC32	var-(0x)?8|IMAGE_REL_AMD64_REL32	var)
[ 	]*[a-f0-9]+:	c5 fc 77[ 	]+vzeroall
[ 	]*[a-f0-9]+:	c4 e1 7c 77[ 	]+vzeroall
[ 	]*[a-f0-9]+:	c4 c1 71 58 d0[ 	]+vaddpd %xmm8,%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c5 b5 58 d0[ 	]+vaddpd %ymm0,%ymm9,%ymm2
[ 	]*[a-f0-9]+:	c5 72 58 d0[ 	]+vaddss %xmm0,%xmm1,%xmm10
[ 	]*[a-f0-9]+:	62 f1 76 08 58 50 01[ 	]+\{evex\} vaddss (0x)?4\(%rax\),%xmm1,%xmm2
[ 	]*[a-f0-9]+:	c4 e3 69 68 19 80[ 	]+vfmaddps %xmm8,\(%rcx\),%xmm2,%xmm3
[ 	]*[a-f0-9]+:	67 c4 e3 e9 68 19 00[ 	]+vfmaddps \(%ecx\),%xmm0,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 c3 e9 68 18 10[ 	]+vfmaddps \(%r8\),%xmm1,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 e3 69 48 19 80[ 	]+vpermil2ps \$(0x)0,%xmm8,\(%rcx\),%xmm2,%xmm3
[ 	]*[a-f0-9]+:	67 c4 e3 e9 48 19 02[ 	]+vpermil2ps \$(0x)2,\(%ecx\),%xmm0,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 c3 e9 48 18 13[ 	]+vpermil2ps \$(0x)3,\(%r8\),%xmm1,%xmm2,%xmm3
[ 	]*[a-f0-9]+:	c4 c1 78 92 c8[ 	]+kmovw  %r8d,%k1
[ 	]*[a-f0-9]+:	c5 78 93 c1[ 	]+kmovw  %k1,%r8d
[ 	]*[a-f0-9]+:	62 b1 74 38 58 d0[ 	]+vaddps \{rd-sae\},%zmm16,%zmm1,%zmm2
[ 	]*[a-f0-9]+:	62 f1 74 10 58 d0[ 	]+vaddps \{rn-sae\},%zmm0,%zmm17,%zmm2
[ 	]*[a-f0-9]+:	62 e1 74 58 58 d0[ 	]+vaddps \{ru-sae\},%zmm0,%zmm1,%zmm18
[ 	]*[a-f0-9]+:	c4 e2 39 92 1c 48[ 	]+vgatherdps %xmm8,\(%rax,%xmm1,2\),%xmm3
[ 	]*[a-f0-9]+:	c4 c2 79 92 1c 48[ 	]+vgatherdps %xmm0,\(%r8,%xmm1,2\),%xmm3
[ 	]*[a-f0-9]+:	c4 a2 79 92 1c 48[ 	]+vgatherdps %xmm0,\(%rax,%xmm9,2\),%xmm3
[ 	]*[a-f0-9]+:	c4 62 79 92 1c 48[ 	]+vgatherdps %xmm0,\(%rax,%xmm1,2\),%xmm11
[ 	]*[a-f0-9]+:	62 d2 fd 0c 93 1c 48[ 	]+vgatherqpd \(%r8,%xmm1,2\),%xmm3\{%k4\}
[ 	]*[a-f0-9]+:	62 b2 fd 0c 93 1c 48[ 	]+vgatherqpd \(%rax,%xmm9,2\),%xmm3\{%k4\}
[ 	]*[a-f0-9]+:	62 f2 fd 04 93 1c 48[ 	]+vgatherqpd \(%rax,%xmm17,2\),%xmm3\{%k4\}
[ 	]*[a-f0-9]+:	62 72 fd 0c 93 1c 48[ 	]+vgatherqpd \(%rax,%xmm1,2\),%xmm11\{%k4\}
[ 	]*[a-f0-9]+:	62 e2 fd 0c 93 1c 48[ 	]+vgatherqpd \(%rax,%xmm1,2\),%xmm19\{%k4\}
[ 	]*[a-f0-9]+:	62 f2 7d 28 88 48 01[ 	]+vexpandps (0x)?4\(%rax\),%ymm1
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 48 5a 40 01[ 	]+vcvtpd2phz 0x40\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%rax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%rax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 fd 58 5a 40 01[ 	]+vcvtpd2ph (0x)?8\(%rax\)\{1to8\},%xmm0
[ 	]*[a-f0-9]+:	62 f5 7c 48 5a 40 01[ 	]+vcvtph2pd 0x10\(%rax\),%zmm0
[ 	]*[a-f0-9]+:	62 f5 7c 58 5a 40 01[ 	]+vcvtph2pd (0x)?2\(%rax\)\{1to8\},%zmm0
#pass
