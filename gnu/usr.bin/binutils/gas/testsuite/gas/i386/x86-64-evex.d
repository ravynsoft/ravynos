#objdump: -dw
#name: x86-64 EVEX insns
#source: evex.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f1 d6 38 2a f0    	vcvtsi2ss %rax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 38 2a f0    	vcvtsi2sd %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 38 2a f0    	vcvtsi2sd %rax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d6 08 7b f0    	vcvtusi2ss %rax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 08 7b f0    	vcvtusi2sd %eax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 08 7b f0    	vcvtusi2sd %rax,%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d6 38 7b f0    	vcvtusi2ss %rax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 57 38 7b f0    	vcvtusi2sd %eax,\{rd-bad\},%xmm5,%xmm6
 +[a-f0-9]+:	62 f1 d7 38 7b f0    	vcvtusi2sd %rax,\{rd-sae\},%xmm5,%xmm6
 +[a-f0-9]+:	62 e1 7e 08 2d c0    	vcvtss2si %xmm0,\(bad\)
 +[a-f0-9]+:	62 e1 7c 08 c2 c0 00 	vcmpeqps %xmm0,%xmm0,\(bad\)
#pass
