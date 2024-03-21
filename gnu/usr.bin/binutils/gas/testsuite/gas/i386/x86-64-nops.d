#as: --divide
#objdump: -drw
#name: x86-64 nops

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
[ 	]*[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)
[ 	]*[a-f0-9]+:	0f 1f 44 00 00       	nopl   0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	0f 1f 80 00 00 00 00 	nopl   0x0\(%rax\)
[ 	]*[a-f0-9]+:	0f 1f 84 00 00 00 00 00 	nopl   0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	66 0f 1f 84 00 00 00 00 00 	nopw   0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	2e 66 0f 1f 84 00 00 00 00 00 	cs nopw 0x0\(%rax,%rax,1\)
[ 	]*[a-f0-9]+:	0f 19 ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1a ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1b ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1c ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1d ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1e ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 1f ff             	nop    %edi
[ 	]*[a-f0-9]+:	0f 19 5a 22          	nopl   0x22\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1c 5a 22          	nopl   0x22\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1d 5a 22          	nopl   0x22\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1e 5a 22          	nopl   0x22\(%rdx\)
[ 	]*[a-f0-9]+:	0f 1f 5a 22          	nopl   0x22\(%rdx\)
[ 	]*[a-f0-9]+:	0f 19 9c 1d 11 22 33 44 	nopl   0x44332211\(%rbp,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1c 9c 1d 11 22 33 44 	nopl   0x44332211\(%rbp,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1d 9c 1d 11 22 33 44 	nopl   0x44332211\(%rbp,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1e 9c 1d 11 22 33 44 	nopl   0x44332211\(%rbp,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 1f 9c 1d 11 22 33 44 	nopl   0x44332211\(%rbp,%rbx,1\)
[ 	]*[a-f0-9]+:	0f 19 04 60          	nopl   \(%rax,%riz,2\)
[ 	]*[a-f0-9]+:	0f 1c 0c 60          	nopl   \(%rax,%riz,2\)
[ 	]*[a-f0-9]+:	0f 1d 04 60          	nopl   \(%rax,%riz,2\)
[ 	]*[a-f0-9]+:	0f 1e 04 60          	nopl   \(%rax,%riz,2\)
[ 	]*[a-f0-9]+:	0f 1f 04 60          	nopl   \(%rax,%riz,2\)
[ 	]*[a-f0-9]+:	0f 19 04 59          	nopl   \(%rcx,%rbx,2\)
[ 	]*[a-f0-9]+:	0f 1c 0c 59          	nopl   \(%rcx,%rbx,2\)
[ 	]*[a-f0-9]+:	0f 1d 04 59          	nopl   \(%rcx,%rbx,2\)
[ 	]*[a-f0-9]+:	0f 1e 04 59          	nopl   \(%rcx,%rbx,2\)
[ 	]*[a-f0-9]+:	0f 1f 04 59          	nopl   \(%rcx,%rbx,2\)
[ 	]*[a-f0-9]+:	48 0f 1f c0          	nop    %rax
[ 	]*[a-f0-9]+:	0f 1f c0             	nop    %eax
[ 	]*[a-f0-9]+:	66 0f 1f c0          	nop    %ax
[ 	]*[a-f0-9]+:	48 0f 1f 00          	nopq   \(%rax\)
[ 	]*[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
[ 	]*[a-f0-9]+:	66 0f 1f 00          	nopw   \(%rax\)
[ 	]*[a-f0-9]+:	48 0f 1f c0          	nop    %rax
[ 	]*[a-f0-9]+:	0f 1f c0             	nop    %eax
[ 	]*[a-f0-9]+:	66 0f 1f c0          	nop    %ax
[ 	]*[a-f0-9]+:	49 0f 1f c2          	nop    %r10
[ 	]*[a-f0-9]+:	41 0f 1f c2          	nop    %r10d
[ 	]*[a-f0-9]+:	66 41 0f 1f c2       	nop    %r10w
[ 	]*[a-f0-9]+:	49 0f 1f 02          	nopq   \(%r10\)
[ 	]*[a-f0-9]+:	41 0f 1f 02          	nopl   \(%r10\)
[ 	]*[a-f0-9]+:	66 41 0f 1f 02       	nopw   \(%r10\)
[ 	]*[a-f0-9]+:	49 0f 1f c2          	nop    %r10
[ 	]*[a-f0-9]+:	41 0f 1f c2          	nop    %r10d
[ 	]*[a-f0-9]+:	66 41 0f 1f c2       	nop    %r10w
#pass
