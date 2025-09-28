#objdump: -dw
#name: i386 FMA4

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 ed 69 fc 60    	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 69 39 60    	vfmaddpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 68 fc 60    	vfmaddps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 68 39 60    	vfmaddps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 59 68 6c da 01 30 	vfmaddps %xmm3,0x1\(%edx,%ebx,8\),%xmm4,%xmm5
[ 	]*[a-f0-9]+:	c4 e3 49 68 8c 81 80 00 00 00 70 	vfmaddps %xmm7,0x80\(%ecx,%eax,4\),%xmm6,%xmm1
[ 	]*[a-f0-9]+:	c4 e3 ed 5d fc 60    	vfmaddsubpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5d 39 60    	vfmaddsubpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5c fc 60    	vfmaddsubps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5c 39 60    	vfmaddsubps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5f fc 60    	vfmsubaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5f 39 60    	vfmsubaddpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5e fc 60    	vfmsubaddps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5e 39 60    	vfmsubaddps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 6d fc 60    	vfmsubpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 6d 39 60    	vfmsubpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 6c fc 60    	vfmsubps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 6c 39 60    	vfmsubps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 e9 69 fc 60    	vfmaddpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 69 39 60    	vfmaddpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 69 39 40    	vfmaddpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 68 fc 60    	vfmaddps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 68 39 60    	vfmaddps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 68 39 40    	vfmaddps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5d fc 60    	vfmaddsubpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5d 39 60    	vfmaddsubpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5d 39 40    	vfmaddsubpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5c fc 60    	vfmaddsubps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5c 39 60    	vfmaddsubps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5c 39 40    	vfmaddsubps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5f fc 60    	vfmsubaddpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5f 39 60    	vfmsubaddpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5f 39 40    	vfmsubaddpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5e fc 60    	vfmsubaddps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5e 39 60    	vfmsubaddps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5e 39 40    	vfmsubaddps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6d fc 60    	vfmsubpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6d 39 60    	vfmsubpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6d 39 40    	vfmsubpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6c fc 60    	vfmsubps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6c 39 60    	vfmsubps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6c 39 40    	vfmsubps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6b fc 60    	vfmaddsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6b 39 60    	vfmaddsd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6b 39 40    	vfmaddsd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6f fc 60    	vfmsubsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6f 39 60    	vfmsubsd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6f 39 40    	vfmsubsd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6a fc 60    	vfmaddss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6a 39 60    	vfmaddss \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6a 39 40    	vfmaddss %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6e fc 60    	vfmsubss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6e 39 60    	vfmsubss \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6e 39 40    	vfmsubss %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 ed 79 fc 60    	vfnmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 79 39 60    	vfnmaddpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 78 fc 60    	vfnmaddps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 78 39 60    	vfnmaddps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7d fc 60    	vfnmsubpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7d 39 60    	vfnmsubpd \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7c fc 60    	vfnmsubps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7c 39 60    	vfnmsubps \(%ecx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 e9 79 fc 60    	vfnmaddpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 79 39 60    	vfnmaddpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 79 39 40    	vfnmaddpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 78 fc 60    	vfnmaddps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 78 39 60    	vfnmaddps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 78 39 40    	vfnmaddps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7d fc 60    	vfnmsubpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7d 39 60    	vfnmsubpd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7d 39 40    	vfnmsubpd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7c fc 60    	vfnmsubps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7c 39 60    	vfnmsubps \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7c 39 40    	vfnmsubps %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7b fc 60    	vfnmaddsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7b 39 60    	vfnmaddsd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7b 39 40    	vfnmaddsd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7f fc 60    	vfnmsubsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7f 39 60    	vfnmsubsd \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7f 39 40    	vfnmsubsd %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7a fc 60    	vfnmaddss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7a 39 60    	vfnmaddss \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7a 39 40    	vfnmaddss %xmm4,\(%ecx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7e fc 60    	vfnmsubss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7e 39 60    	vfnmsubss \(%ecx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7e 39 40    	vfnmsubss %xmm4,\(%ecx\),%xmm2,%xmm7
#pass
