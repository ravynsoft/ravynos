#source: ../x86-64-fma4.s
#objdump: -dw
#name: x86-64 (ILP32) FMA4

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 ed 69 fc 60    	vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 69 39 60    	vfmaddpd \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 68 fc 60    	vfmaddps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 68 39 60    	vfmaddps \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 63 41 68 5c da 01 40 	vfmaddps %xmm4,0x1\(%rdx,%rbx,8\),%xmm7,%xmm11
[ 	]*[a-f0-9]+:	c4 e3 49 68 a4 81 80 00 00 00 80 	vfmaddps %xmm8,0x80\(%rcx,%rax,4\),%xmm6,%xmm4
[ 	]*[a-f0-9]+:	c4 e3 ed 5d fc 60    	vfmaddsubpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5d 39 60    	vfmaddsubpd \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5c fc 60    	vfmaddsubps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 5c 39 60    	vfmaddsubps \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 e9 69 fc 60    	vfmaddpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 69 39 60    	vfmaddpd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 69 39 40    	vfmaddpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 68 fc 60    	vfmaddps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 68 39 60    	vfmaddps \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 68 39 40    	vfmaddps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5d fc 60    	vfmaddsubpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5d 39 60    	vfmaddsubpd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5d 39 40    	vfmaddsubpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5c fc 60    	vfmaddsubps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 5c 39 60    	vfmaddsubps \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 5c 39 40    	vfmaddsubps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6b fc 60    	vfmaddsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6b 39 60    	vfmaddsd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6b 39 40    	vfmaddsd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6a fc 60    	vfmaddss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 6a 39 60    	vfmaddss \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 6a 39 40    	vfmaddss %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 ed 79 fc 60    	vfnmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 79 39 60    	vfnmaddpd \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 78 fc 60    	vfnmaddps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 78 39 60    	vfnmaddps \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7d fc 60    	vfnmsubpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7d 39 60    	vfnmsubpd \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7c fc 60    	vfnmsubps %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 ed 7c 39 60    	vfnmsubps \(%rcx\),%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 e9 79 fc 60    	vfnmaddpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 79 39 60    	vfnmaddpd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 79 39 40    	vfnmaddpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 78 fc 60    	vfnmaddps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 78 39 60    	vfnmaddps \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 78 39 40    	vfnmaddps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7d fc 60    	vfnmsubpd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7d 39 60    	vfnmsubpd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7d 39 40    	vfnmsubpd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7c fc 60    	vfnmsubps %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7c 39 60    	vfnmsubps \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7c 39 40    	vfnmsubps %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7b fc 60    	vfnmaddsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7b 39 60    	vfnmaddsd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7b 39 40    	vfnmaddsd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7f fc 60    	vfnmsubsd %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7f 39 60    	vfnmsubsd \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7f 39 40    	vfnmsubsd %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7a fc 60    	vfnmaddss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7a 39 60    	vfnmaddss \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 69 7a 39 40    	vfnmaddss %xmm4,\(%rcx\),%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7e fc 60    	vfnmsubss %xmm4,%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 e3 e9 7e 39 60    	vfnmsubss \(%rcx\),%xmm6,%xmm2,%xmm7
[ 	]*[a-f0-9]+:	c4 c3 e1 69 64 0d 00 b0[ 	]+vfmaddpd 0x0\(%r13,%rcx,1\),%xmm11,%xmm3,%xmm4
[ 	]*[a-f0-9]+:	c4 c3 f1 69 bc c1 be 00 00 00 90[ 	]+vfmaddpd 0xbe\(%r9,%rax,8\),%xmm9,%xmm1,%xmm7
[ 	]*[a-f0-9]+:	c4 c3 e1 6d 64 0d 00 b0[ 	]+vfmsubpd 0x0\(%r13,%rcx,1\),%xmm11,%xmm3,%xmm4
#pass
