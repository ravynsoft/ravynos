#objdump: -dw
#name: x86-64 SSE3

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	66 0f d0 01 [ 	]*addsubpd \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f d0 ca [ 	]*addsubpd %xmm2,%xmm1
[ 	]*[a-f0-9]+:	f2 0f d0 13 [ 	]*addsubps \(%rbx\),%xmm2
[ 	]*[a-f0-9]+:	f2 0f d0 dc [ 	]*addsubps %xmm4,%xmm3
[ 	]*[a-f0-9]+:	df 88 90 90 90 00 [ 	]*fisttps 0x909090\(%rax\)
[ 	]*[a-f0-9]+:	db 88 90 90 90 00 [ 	]*fisttpl 0x909090\(%rax\)
[ 	]*[a-f0-9]+:	dd 88 90 90 90 00 [ 	]*fisttpll 0x909090\(%rax\)
[ 	]*[a-f0-9]+:	dd 88 90 90 90 00 [ 	]*fisttpll 0x909090\(%rax\)
[ 	]*[a-f0-9]+:	66 0f 7c 65 00 [ 	]*haddpd 0x0\(%rbp\),%xmm4
[ 	]*[a-f0-9]+:	66 0f 7c ee [ 	]*haddpd %xmm6,%xmm5
[ 	]*[a-f0-9]+:	f2 0f 7c 37 [ 	]*haddps \(%rdi\),%xmm6
[ 	]*[a-f0-9]+:	f2 0f 7c f8 [ 	]*haddps %xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 0f 7d c1 [ 	]*hsubpd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 7d 0a [ 	]*hsubpd \(%rdx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 7d d2 [ 	]*hsubps %xmm2,%xmm2
[ 	]*[a-f0-9]+:	f2 0f 7d 1c 24 [ 	]*hsubps \(%rsp\),%xmm3
[ 	]*[a-f0-9]+:	f2 0f f0 2e [ 	]*lddqu  \(%rsi\),%xmm5
[ 	]*[a-f0-9]+:	0f 01 c8 [ 	]*monitor %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 c8 [ 	]*monitor %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 c8 [ 	]*monitor %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	f2 0f 12 f7 [ 	]*movddup %xmm7,%xmm6
[ 	]*[a-f0-9]+:	f2 0f 12 38 [ 	]*movddup \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 16 01 [ 	]*movshdup \(%rcx\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 16 ca [ 	]*movshdup %xmm2,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 12 13 [ 	]*movsldup \(%rbx\),%xmm2
[ 	]*[a-f0-9]+:	f3 0f 12 dc [ 	]*movsldup %xmm4,%xmm3
[ 	]*[a-f0-9]+:	0f 01 c9 [ 	]*mwait  %eax,%ecx
[ 	]*[a-f0-9]+:	0f 01 c9 [ 	]*mwait  %eax,%ecx
[ 	]*[a-f0-9]+:	0f 01 c9 [ 	]*mwait  %eax,%ecx
[ 	]*[a-f0-9]+:	67 0f 01 c8 [ 	]*monitor %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 c8 [ 	]*monitor %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 c8 [ 	]*monitor %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	f2 0f 12 38 [ 	]*movddup \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f 12 38 [ 	]*movddup \(%rax\),%xmm7
[ 	]*[0-9a-f]+:	0f 01 c8[ 	]+monitor %rax,%ecx,%edx
[ 	]*[0-9a-f]+:	67 0f 01 c8[ 	]+monitor %eax,%ecx,%edx
[ 	]*[0-9a-f]+:	0f 01 c9[ 	]+mwait  %eax,%ecx
#pass
