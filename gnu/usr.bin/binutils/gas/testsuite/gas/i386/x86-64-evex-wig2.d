#as: -mevexwig=1
#objdump: -dw
#name: x86-64 non-WIG EVEX insns with -mevexwig=1

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 f1 36 30 2a f0    	vcvtsi2ss %eax,\{rd-sae\},%xmm25,%xmm6
 +[a-f0-9]+:	62 f1 36 00 2a f0    	vcvtsi2ss %eax,%xmm25,%xmm6
 +[a-f0-9]+:	62 f1 37 00 2a f0    	vcvtsi2sd %eax,%xmm25,%xmm6
 +[a-f0-9]+:	62 f1 36 30 7b f0    	vcvtusi2ss %eax,\{rd-sae\},%xmm25,%xmm6
 +[a-f0-9]+:	62 f1 36 00 7b f0    	vcvtusi2ss %eax,%xmm25,%xmm6
 +[a-f0-9]+:	62 f1 07 08 7b f0    	vcvtusi2sd %eax,%xmm15,%xmm6
#pass
