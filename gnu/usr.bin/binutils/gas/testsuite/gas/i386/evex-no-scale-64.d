#source: evex-no-scale.s
#objdump: -dw
#name: x86-64 EVEX no disp scaling

.*: +file format .*

Disassembly of section .text:

0+ <disp>:
 +[a-f0-9]+:	62 f1 7c 48 28 05 00 fc ff ff 	vmovaps -0x400\(%rip\),%zmm0        # .*
 +[a-f0-9]+:	62 f1 7c 48 28 04 05 40 00 00 00 	vmovaps 0x40\(,%rax,1\),%zmm0
 +[a-f0-9]+:	62 f1 7c 48 28 04 25 40 00 00 00 	vmovaps 0x40,%zmm0
 +[a-f0-9]+:	67 62 f1 7c 48 28 04 05 40 00 00 00 	vmovaps 0x40\(,%eax,1\),%zmm0
 +[a-f0-9]+:	67 62 f1 7c 48 28 04 25 40 00 00 00 	vmovaps 0x40\(,%eiz,1\),%zmm0
 +[a-f0-9]+:	62 f1 7c 48 28 04 25 40 00 00 00 	vmovaps 0x40,%zmm0
#pass
