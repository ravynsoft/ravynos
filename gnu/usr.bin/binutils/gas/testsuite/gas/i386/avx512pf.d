#as:
#objdump: -dw
#name: i386 AVX512PF insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c fd 7b 00 00 00 	vgatherpf0dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c fd 7b 00 00 00 	vgatherpf0dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 4c 38 20 	vgatherpf0dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c b9 00 04 00 00 	vgatherpf0dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c fd 7b 00 00 00 	vgatherpf0dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c fd 7b 00 00 00 	vgatherpf0dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 4c 38 40 	vgatherpf0dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c b9 00 04 00 00 	vgatherpf0dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c fd 7b 00 00 00 	vgatherpf0qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c fd 7b 00 00 00 	vgatherpf0qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 4c 38 20 	vgatherpf0qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c b9 00 04 00 00 	vgatherpf0qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c fd 7b 00 00 00 	vgatherpf0qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c fd 7b 00 00 00 	vgatherpf0qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 4c 38 40 	vgatherpf0qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c b9 00 04 00 00 	vgatherpf0qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 fd 7b 00 00 00 	vgatherpf1dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 fd 7b 00 00 00 	vgatherpf1dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 54 38 20 	vgatherpf1dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 b9 00 04 00 00 	vgatherpf1dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 fd 7b 00 00 00 	vgatherpf1dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 fd 7b 00 00 00 	vgatherpf1dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 54 38 40 	vgatherpf1dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 b9 00 04 00 00 	vgatherpf1dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 fd 7b 00 00 00 	vgatherpf1qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 fd 7b 00 00 00 	vgatherpf1qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 54 38 20 	vgatherpf1qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 b9 00 04 00 00 	vgatherpf1qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 fd 7b 00 00 00 	vgatherpf1qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 fd 7b 00 00 00 	vgatherpf1qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 54 38 40 	vgatherpf1qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 b9 00 04 00 00 	vgatherpf1qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac fd 7b 00 00 00 	vscatterpf0dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac fd 7b 00 00 00 	vscatterpf0dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 6c 38 20 	vscatterpf0dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac b9 00 04 00 00 	vscatterpf0dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac fd 7b 00 00 00 	vscatterpf0dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac fd 7b 00 00 00 	vscatterpf0dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 6c 38 40 	vscatterpf0dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac b9 00 04 00 00 	vscatterpf0dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac fd 7b 00 00 00 	vscatterpf0qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac fd 7b 00 00 00 	vscatterpf0qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 6c 38 20 	vscatterpf0qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac b9 00 04 00 00 	vscatterpf0qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac fd 7b 00 00 00 	vscatterpf0qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac fd 7b 00 00 00 	vscatterpf0qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 6c 38 40 	vscatterpf0qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac b9 00 04 00 00 	vscatterpf0qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 fd 7b 00 00 00 	vscatterpf1dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 fd 7b 00 00 00 	vscatterpf1dpd 0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 74 38 20 	vscatterpf1dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 b9 00 04 00 00 	vscatterpf1dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 fd 7b 00 00 00 	vscatterpf1dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 fd 7b 00 00 00 	vscatterpf1dps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 74 38 40 	vscatterpf1dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 b9 00 04 00 00 	vscatterpf1dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 fd 7b 00 00 00 	vscatterpf1qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 fd 7b 00 00 00 	vscatterpf1qpd 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 74 38 20 	vscatterpf1qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 b9 00 04 00 00 	vscatterpf1qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 fd 7b 00 00 00 	vscatterpf1qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 fd 7b 00 00 00 	vscatterpf1qps 0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 74 38 40 	vscatterpf1qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 b9 00 04 00 00 	vscatterpf1qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c fd 85 ff ff ff 	vgatherpf0dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c fd 85 ff ff ff 	vgatherpf0dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 4c 38 20 	vgatherpf0dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 8c b9 00 04 00 00 	vgatherpf0dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c fd 85 ff ff ff 	vgatherpf0dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c fd 85 ff ff ff 	vgatherpf0dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 4c 38 40 	vgatherpf0dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 8c b9 00 04 00 00 	vgatherpf0dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c fd 85 ff ff ff 	vgatherpf0qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c fd 85 ff ff ff 	vgatherpf0qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 4c 38 20 	vgatherpf0qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 8c b9 00 04 00 00 	vgatherpf0qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c fd 85 ff ff ff 	vgatherpf0qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c fd 85 ff ff ff 	vgatherpf0qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 4c 38 40 	vgatherpf0qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 8c b9 00 04 00 00 	vgatherpf0qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 fd 85 ff ff ff 	vgatherpf1dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 fd 85 ff ff ff 	vgatherpf1dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 54 38 20 	vgatherpf1dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 94 b9 00 04 00 00 	vgatherpf1dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 fd 85 ff ff ff 	vgatherpf1dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 fd 85 ff ff ff 	vgatherpf1dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 54 38 40 	vgatherpf1dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 94 b9 00 04 00 00 	vgatherpf1dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 fd 85 ff ff ff 	vgatherpf1qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 fd 85 ff ff ff 	vgatherpf1qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 54 38 20 	vgatherpf1qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 94 b9 00 04 00 00 	vgatherpf1qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 fd 85 ff ff ff 	vgatherpf1qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 fd 85 ff ff ff 	vgatherpf1qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 54 38 40 	vgatherpf1qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 94 b9 00 04 00 00 	vgatherpf1qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac fd 85 ff ff ff 	vscatterpf0dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac fd 85 ff ff ff 	vscatterpf0dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 6c 38 20 	vscatterpf0dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 ac b9 00 04 00 00 	vscatterpf0dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac fd 85 ff ff ff 	vscatterpf0dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac fd 85 ff ff ff 	vscatterpf0dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 6c 38 40 	vscatterpf0dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 ac b9 00 04 00 00 	vscatterpf0dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac fd 85 ff ff ff 	vscatterpf0qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac fd 85 ff ff ff 	vscatterpf0qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 6c 38 20 	vscatterpf0qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 ac b9 00 04 00 00 	vscatterpf0qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac fd 85 ff ff ff 	vscatterpf0qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac fd 85 ff ff ff 	vscatterpf0qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 6c 38 40 	vscatterpf0qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 ac b9 00 04 00 00 	vscatterpf0qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 fd 85 ff ff ff 	vscatterpf1dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 fd 85 ff ff ff 	vscatterpf1dpd -0x7b\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 74 38 20 	vscatterpf1dpd 0x100\(%eax,%ymm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c6 b4 b9 00 04 00 00 	vscatterpf1dpd 0x400\(%ecx,%ymm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 fd 85 ff ff ff 	vscatterpf1dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 fd 85 ff ff ff 	vscatterpf1dps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 74 38 40 	vscatterpf1dps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c6 b4 b9 00 04 00 00 	vscatterpf1dps 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 fd 85 ff ff ff 	vscatterpf1qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 fd 85 ff ff ff 	vscatterpf1qpd -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 74 38 20 	vscatterpf1qpd 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 fd 49 c7 b4 b9 00 04 00 00 	vscatterpf1qpd 0x400\(%ecx,%zmm7,4\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 fd 85 ff ff ff 	vscatterpf1qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 fd 85 ff ff ff 	vscatterpf1qps -0x7b\(%ebp,%zmm7,8\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 74 38 40 	vscatterpf1qps 0x100\(%eax,%zmm7,1\)\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 49 c7 b4 b9 00 04 00 00 	vscatterpf1qps 0x400\(%ecx,%zmm7,4\)\{%k1\}
#pass
