#as: -J --divide
#objdump: -drw
#name: x86-64 opcode

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	41 ff 10             	call   \*\(%r8\)
[ 	]*[a-f0-9]+:	ff 10                	call   \*\(%rax\)
[ 	]*[a-f0-9]+:	41 ff 10             	call   \*\(%r8\)
[ 	]*[a-f0-9]+:	ff 10                	call   \*\(%rax\)
[ 	]*[a-f0-9]+:	cb                   	lret
[ 	]*[a-f0-9]+:	48 cb                	lretq
[ 	]*[a-f0-9]+:	c3                   	ret
[ 	]*[a-f0-9]+:	cf                   	iret
[ 	]*[a-f0-9]+:	66 cf                	iretw
[ 	]*[a-f0-9]+:	48 cf                	iretq
[ 	]*[a-f0-9]+:	41 8c 08             	mov    %cs,\(%r8\)
[ 	]*[a-f0-9]+:	8c 08                	mov    %cs,\(%rax\)
[ 	]*[a-f0-9]+:	41 8c 10             	mov    %ss,\(%r8\)
[ 	]*[a-f0-9]+:	8c 10                	mov    %ss,\(%rax\)
[ 	]*[a-f0-9]+:	41 8c 20             	mov    %fs,\(%r8\)
[ 	]*[a-f0-9]+:	8c 20                	mov    %fs,\(%rax\)
[ 	]*[a-f0-9]+:	41 8e 10             	mov    \(%r8\),%ss
[ 	]*[a-f0-9]+:	8e 10                	mov    \(%rax\),%ss
[ 	]*[a-f0-9]+:	41 8e 20             	mov    \(%r8\),%fs
[ 	]*[a-f0-9]+:	8e 20                	mov    \(%rax\),%fs
[ 	]*[a-f0-9]+:	41 c6 00 00          	movb   \$0x0,\(%r8\)
[ 	]*[a-f0-9]+:	c6 00 00             	movb   \$0x0,\(%rax\)
[ 	]*[a-f0-9]+:	66 41 c7 00 00 70    	movw   \$0x7000,\(%r8\)
[ 	]*[a-f0-9]+:	66 c7 00 00 70       	movw   \$0x7000,\(%rax\)
[ 	]*[a-f0-9]+:	41 c7 00 00 00 00 70 	movl   \$0x70000000,\(%r8\)
[ 	]*[a-f0-9]+:	c7 00 00 00 00 70    	movl   \$0x70000000,\(%rax\)
[ 	]*[a-f0-9]+:	41 c6 00 00          	movb   \$0x0,\(%r8\)
[ 	]*[a-f0-9]+:	c6 00 00             	movb   \$0x0,\(%rax\)
[ 	]*[a-f0-9]+:	66 41 c7 00 00 70    	movw   \$0x7000,\(%r8\)
[ 	]*[a-f0-9]+:	66 c7 00 00 70       	movw   \$0x7000,\(%rax\)
[ 	]*[a-f0-9]+:	c7 00 00 00 00 70    	movl   \$0x70000000,\(%rax\)
[ 	]*[a-f0-9]+:	41 c6 00 00          	movb   \$0x0,\(%r8\)
[ 	]*[a-f0-9]+:	c6 00 00             	movb   \$0x0,\(%rax\)
[ 	]*[a-f0-9]+:	66 41 c7 00 00 70    	movw   \$0x7000,\(%r8\)
[ 	]*[a-f0-9]+:	66 c7 00 00 70       	movw   \$0x7000,\(%rax\)
[ 	]*[a-f0-9]+:	41 c7 00 00 00 00 70 	movl   \$0x70000000,\(%r8\)
[ 	]*[a-f0-9]+:	c7 00 00 00 00 70    	movl   \$0x70000000,\(%rax\)
[ 	]*[a-f0-9]+:	49 c7 00 00 00 00 70 	movq   \$0x70000000,\(%r8\)
[ 	]*[a-f0-9]+:	48 c7 00 00 00 00 70 	movq   \$0x70000000,\(%rax\)
[ 	]*[a-f0-9]+:	0f b4 08             	lfs    \(%rax\),%ecx
[ 	]*[a-f0-9]+:	0f b4 01             	lfs    \(%rcx\),%eax
[ 	]*[a-f0-9]+:	66 0f b4 08          	lfs    \(%rax\),%cx
[ 	]*[a-f0-9]+:	66 0f b4 01          	lfs    \(%rcx\),%ax
[ 	]*[a-f0-9]+:	0f b5 11             	lgs    \(%rcx\),%edx
[ 	]*[a-f0-9]+:	0f b5 0a             	lgs    \(%rdx\),%ecx
[ 	]*[a-f0-9]+:	66 0f b5 11          	lgs    \(%rcx\),%dx
[ 	]*[a-f0-9]+:	66 0f b5 0a          	lgs    \(%rdx\),%cx
[ 	]*[a-f0-9]+:	0f b2 1a             	lss    \(%rdx\),%ebx
[ 	]*[a-f0-9]+:	0f b2 13             	lss    \(%rbx\),%edx
[ 	]*[a-f0-9]+:	66 0f b2 1a          	lss    \(%rdx\),%bx
[ 	]*[a-f0-9]+:	66 0f b2 13          	lss    \(%rbx\),%dx
[ 	]*[a-f0-9]+:	41 0f c3 00          	movnti %eax,\(%r8\)
[ 	]*[a-f0-9]+:	0f c3 00             	movnti %eax,\(%rax\)
[ 	]*[a-f0-9]+:	49 0f c3 00          	movnti %rax,\(%r8\)
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti %rax,\(%rax\)
[ 	]*[a-f0-9]+:	4d 0f c3 00          	movnti %r8,\(%r8\)
[ 	]*[a-f0-9]+:	4c 0f c3 00          	movnti %r8,\(%rax\)
[ 	]*[a-f0-9]+:	e2 fe                	loop   .*
[ 	]*[a-f0-9]+:	e2 fe                	loop   .*
[ 	]*[a-f0-9]+:	67 e2 fd             	loopl  .*
[ 	]*[a-f0-9]+:	e3 fe                	jrcxz  .*
[ 	]*[a-f0-9]+:	67 e3 fd             	jecxz  .*
[ 	]*[a-f0-9]+:	41 f6 38             	idivb  \(%r8\)
[ 	]*[a-f0-9]+:	f6 38                	idivb  \(%rax\)
[ 	]*[a-f0-9]+:	66 41 f7 38          	idivw  \(%r8\)
[ 	]*[a-f0-9]+:	66 f7 38             	idivw  \(%rax\)
[ 	]*[a-f0-9]+:	41 f7 38             	idivl  \(%r8\)
[ 	]*[a-f0-9]+:	f7 38                	idivl  \(%rax\)
[ 	]*[a-f0-9]+:	49 f7 38             	idivq  \(%r8\)
[ 	]*[a-f0-9]+:	48 f7 38             	idivq  \(%rax\)
[ 	]*[a-f0-9]+:	41 f6 28             	imulb  \(%r8\)
[ 	]*[a-f0-9]+:	f6 28                	imulb  \(%rax\)
[ 	]*[a-f0-9]+:	66 41 f7 28          	imulw  \(%r8\)
[ 	]*[a-f0-9]+:	66 f7 28             	imulw  \(%rax\)
[ 	]*[a-f0-9]+:	41 f7 28             	imull  \(%r8\)
[ 	]*[a-f0-9]+:	f7 28                	imull  \(%rax\)
[ 	]*[a-f0-9]+:	49 f7 28             	imulq  \(%r8\)
[ 	]*[a-f0-9]+:	48 f7 28             	imulq  \(%rax\)
[ 	]*[a-f0-9]+:	66 41 0f 58 00       	addpd  \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 58 00          	addpd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 45 0f 58 38       	addpd  \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	66 44 0f 58 38       	addpd  \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	66 45 0f 58 00       	addpd  \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	66 44 0f 58 00       	addpd  \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	66 41 0f 58 38       	addpd  \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 58 38          	addpd  \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 58 c0          	addpd  %xmm0,%xmm0
[ 	]*[a-f0-9]+:	66 45 0f 58 ff       	addpd  %xmm15,%xmm15
[ 	]*[a-f0-9]+:	66 45 0f 58 c7       	addpd  %xmm15,%xmm8
[ 	]*[a-f0-9]+:	f2 49 0f 2d 00       	cvtsd2si \(%r8\),%rax
[ 	]*[a-f0-9]+:	f2 48 0f 2d 00       	cvtsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2d 00       	cvtsd2si \(%r8\),%r8
[ 	]*[a-f0-9]+:	f2 4c 0f 2d 00       	cvtsd2si \(%rax\),%r8
[ 	]*[a-f0-9]+:	f2 48 0f 2d c0       	cvtsd2si %xmm0,%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2d c7       	cvtsd2si %xmm15,%r8
[ 	]*[a-f0-9]+:	f2 49 0f 2d c7       	cvtsd2si %xmm15,%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2d c0       	cvtsd2si %xmm8,%r8
[ 	]*[a-f0-9]+:	f2 49 0f 2d c0       	cvtsd2si %xmm8,%rax
[ 	]*[a-f0-9]+:	f2 4c 0f 2d c7       	cvtsd2si %xmm7,%r8
[ 	]*[a-f0-9]+:	f2 48 0f 2d c7       	cvtsd2si %xmm7,%rax
[ 	]*[a-f0-9]+:	f2 4c 0f 2d c0       	cvtsd2si %xmm0,%r8
[ 	]*[a-f0-9]+:	f2 49 0f 2c 00       	cvttsd2si \(%r8\),%rax
[ 	]*[a-f0-9]+:	f2 48 0f 2c 00       	cvttsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2c 00       	cvttsd2si \(%r8\),%r8
[ 	]*[a-f0-9]+:	f2 4c 0f 2c 00       	cvttsd2si \(%rax\),%r8
[ 	]*[a-f0-9]+:	f2 48 0f 2c c0       	cvttsd2si %xmm0,%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2c c7       	cvttsd2si %xmm15,%r8
[ 	]*[a-f0-9]+:	f2 49 0f 2c c7       	cvttsd2si %xmm15,%rax
[ 	]*[a-f0-9]+:	f2 4d 0f 2c c0       	cvttsd2si %xmm8,%r8
[ 	]*[a-f0-9]+:	f2 49 0f 2c c0       	cvttsd2si %xmm8,%rax
[ 	]*[a-f0-9]+:	f2 4c 0f 2c c7       	cvttsd2si %xmm7,%r8
[ 	]*[a-f0-9]+:	f2 48 0f 2c c7       	cvttsd2si %xmm7,%rax
[ 	]*[a-f0-9]+:	f2 4c 0f 2c c0       	cvttsd2si %xmm0,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2d 00       	cvtss2si \(%r8\),%rax
[ 	]*[a-f0-9]+:	f3 48 0f 2d 00       	cvtss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2d 00       	cvtss2si \(%r8\),%r8
[ 	]*[a-f0-9]+:	f3 4c 0f 2d 00       	cvtss2si \(%rax\),%r8
[ 	]*[a-f0-9]+:	f3 48 0f 2d c0       	cvtss2si %xmm0,%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2d c7       	cvtss2si %xmm15,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2d c7       	cvtss2si %xmm15,%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2d c0       	cvtss2si %xmm8,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2d c0       	cvtss2si %xmm8,%rax
[ 	]*[a-f0-9]+:	f3 4c 0f 2d c7       	cvtss2si %xmm7,%r8
[ 	]*[a-f0-9]+:	f3 48 0f 2d c7       	cvtss2si %xmm7,%rax
[ 	]*[a-f0-9]+:	f3 4c 0f 2d c0       	cvtss2si %xmm0,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2c 00       	cvttss2si \(%r8\),%rax
[ 	]*[a-f0-9]+:	f3 48 0f 2c 00       	cvttss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2c 00       	cvttss2si \(%r8\),%r8
[ 	]*[a-f0-9]+:	f3 4c 0f 2c 00       	cvttss2si \(%rax\),%r8
[ 	]*[a-f0-9]+:	f3 48 0f 2c c0       	cvttss2si %xmm0,%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2c c7       	cvttss2si %xmm15,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2c c7       	cvttss2si %xmm15,%rax
[ 	]*[a-f0-9]+:	f3 4d 0f 2c c0       	cvttss2si %xmm8,%r8
[ 	]*[a-f0-9]+:	f3 49 0f 2c c0       	cvttss2si %xmm8,%rax
[ 	]*[a-f0-9]+:	f3 4c 0f 2c c7       	cvttss2si %xmm7,%r8
[ 	]*[a-f0-9]+:	f3 48 0f 2c c7       	cvttss2si %xmm7,%rax
[ 	]*[a-f0-9]+:	f3 4c 0f 2c c0       	cvttss2si %xmm0,%r8
[ 	]*[a-f0-9]+:	f3 41 0f 2a 00       	cvtsi2ssl \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2a 00          	cvtsi2ssl \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 45 0f 2a 38       	cvtsi2ssl \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 2a 38       	cvtsi2ssl \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	f3 45 0f 2a 00       	cvtsi2ssl \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	f3 44 0f 2a 00       	cvtsi2ssl \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	f3 41 0f 2a 38       	cvtsi2ssl \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 2a 38          	cvtsi2ssl \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 2a c0          	cvtsi2ss %eax,%xmm0
[ 	]*[a-f0-9]+:	f3 44 0f 2a f8       	cvtsi2ss %eax,%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 2a c0       	cvtsi2ss %eax,%xmm8
[ 	]*[a-f0-9]+:	f3 0f 2a f8          	cvtsi2ss %eax,%xmm7
[ 	]*[a-f0-9]+:	f3 41 0f 2a 00       	cvtsi2ssl \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2a 00          	cvtsi2ssl \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 45 0f 2a 38       	cvtsi2ssl \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 2a 38       	cvtsi2ssl \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	f3 45 0f 2a 00       	cvtsi2ssl \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	f3 44 0f 2a 00       	cvtsi2ssl \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	f3 41 0f 2a 38       	cvtsi2ssl \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 2a 38          	cvtsi2ssl \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f2 41 0f 2a 00       	cvtsi2sdl \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2a 00          	cvtsi2sdl \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 45 0f 2a 38       	cvtsi2sdl \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	f2 44 0f 2a 38       	cvtsi2sdl \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	f2 45 0f 2a 00       	cvtsi2sdl \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	f2 44 0f 2a 00       	cvtsi2sdl \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	f2 41 0f 2a 38       	cvtsi2sdl \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f 2a 38          	cvtsi2sdl \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f 2a c0          	cvtsi2sd %eax,%xmm0
[ 	]*[a-f0-9]+:	f2 44 0f 2a f8       	cvtsi2sd %eax,%xmm15
[ 	]*[a-f0-9]+:	f2 44 0f 2a c0       	cvtsi2sd %eax,%xmm8
[ 	]*[a-f0-9]+:	f2 0f 2a f8          	cvtsi2sd %eax,%xmm7
[ 	]*[a-f0-9]+:	f2 41 0f 2a 00       	cvtsi2sdl \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2a 00          	cvtsi2sdl \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 45 0f 2a 38       	cvtsi2sdl \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	f2 44 0f 2a 38       	cvtsi2sdl \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	f2 45 0f 2a 00       	cvtsi2sdl \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	f2 44 0f 2a 00       	cvtsi2sdl \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	f2 41 0f 2a 38       	cvtsi2sdl \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f 2a 38          	cvtsi2sdl \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	66 41 0f 6e 00       	movd   \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 6e 00          	movd   \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 45 0f 6e 38       	movd   \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	66 44 0f 6e 38       	movd   \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	66 45 0f 6e 00       	movd   \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	66 44 0f 6e 00       	movd   \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	66 41 0f 6e 38       	movd   \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 6e 38          	movd   \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 6e c0          	movd   %eax,%xmm0
[ 	]*[a-f0-9]+:	66 44 0f 6e f8       	movd   %eax,%xmm15
[ 	]*[a-f0-9]+:	66 44 0f 6e c0       	movd   %eax,%xmm8
[ 	]*[a-f0-9]+:	66 0f 6e f8          	movd   %eax,%xmm7
[ 	]*[a-f0-9]+:	66 41 0f 7e 00       	movd   %xmm0,\(%r8\)
[ 	]*[a-f0-9]+:	66 0f 7e 00          	movd   %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	66 45 0f 7e 38       	movd   %xmm15,\(%r8\)
[ 	]*[a-f0-9]+:	66 44 0f 7e 38       	movd   %xmm15,\(%rax\)
[ 	]*[a-f0-9]+:	66 45 0f 7e 00       	movd   %xmm8,\(%r8\)
[ 	]*[a-f0-9]+:	66 44 0f 7e 00       	movd   %xmm8,\(%rax\)
[ 	]*[a-f0-9]+:	66 41 0f 7e 38       	movd   %xmm7,\(%r8\)
[ 	]*[a-f0-9]+:	66 0f 7e 38          	movd   %xmm7,\(%rax\)
[ 	]*[a-f0-9]+:	66 0f 7e c0          	movd   %xmm0,%eax
[ 	]*[a-f0-9]+:	66 44 0f 7e f8       	movd   %xmm15,%eax
[ 	]*[a-f0-9]+:	66 44 0f 7e c0       	movd   %xmm8,%eax
[ 	]*[a-f0-9]+:	66 0f 7e f8          	movd   %xmm7,%eax
[ 	]*[a-f0-9]+:	66 48 0f 6e c0       	movq   %rax,%xmm0
[ 	]*[a-f0-9]+:	66 49 0f 6e c0       	movq   %r8,%xmm0
[ 	]*[a-f0-9]+:	66 4d 0f 6e f8       	movq   %r8,%xmm15
[ 	]*[a-f0-9]+:	66 48 0f 7e c0       	movq   %xmm0,%rax
[ 	]*[a-f0-9]+:	66 49 0f 7e c0       	movq   %xmm0,%r8
[ 	]*[a-f0-9]+:	66 49 0f 7e f8       	movq   %xmm7,%r8
[ 	]*[a-f0-9]+:	f3 41 0f 7e 00       	movq   \(%r8\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 7e 00          	movq   \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 45 0f 7e 38       	movq   \(%r8\),%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 7e 38       	movq   \(%rax\),%xmm15
[ 	]*[a-f0-9]+:	f3 45 0f 7e 00       	movq   \(%r8\),%xmm8
[ 	]*[a-f0-9]+:	f3 44 0f 7e 00       	movq   \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	f3 41 0f 7e 38       	movq   \(%r8\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 7e 38          	movq   \(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 7e c0          	movq   %xmm0,%xmm0
[ 	]*[a-f0-9]+:	f3 45 0f 7e ff       	movq   %xmm15,%xmm15
[ 	]*[a-f0-9]+:	f3 45 0f 7e c7       	movq   %xmm15,%xmm8
[ 	]*[a-f0-9]+:	f3 41 0f 7e ff       	movq   %xmm15,%xmm7
[ 	]*[a-f0-9]+:	f3 41 0f 7e c7       	movq   %xmm15,%xmm0
[ 	]*[a-f0-9]+:	f3 45 0f 7e f8       	movq   %xmm8,%xmm15
[ 	]*[a-f0-9]+:	f3 45 0f 7e c0       	movq   %xmm8,%xmm8
[ 	]*[a-f0-9]+:	f3 41 0f 7e f8       	movq   %xmm8,%xmm7
[ 	]*[a-f0-9]+:	f3 41 0f 7e c0       	movq   %xmm8,%xmm0
[ 	]*[a-f0-9]+:	f3 44 0f 7e ff       	movq   %xmm7,%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 7e c7       	movq   %xmm7,%xmm8
[ 	]*[a-f0-9]+:	f3 0f 7e ff          	movq   %xmm7,%xmm7
[ 	]*[a-f0-9]+:	f3 0f 7e c7          	movq   %xmm7,%xmm0
[ 	]*[a-f0-9]+:	f3 44 0f 7e f8       	movq   %xmm0,%xmm15
[ 	]*[a-f0-9]+:	f3 44 0f 7e c0       	movq   %xmm0,%xmm8
[ 	]*[a-f0-9]+:	f3 0f 7e f8          	movq   %xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 41 0f d6 00       	movq   %xmm0,\(%r8\)
[ 	]*[a-f0-9]+:	66 0f d6 00          	movq   %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	66 45 0f d6 38       	movq   %xmm15,\(%r8\)
[ 	]*[a-f0-9]+:	66 44 0f d6 38       	movq   %xmm15,\(%rax\)
[ 	]*[a-f0-9]+:	66 45 0f d6 00       	movq   %xmm8,\(%r8\)
[ 	]*[a-f0-9]+:	66 44 0f d6 00       	movq   %xmm8,\(%rax\)
[ 	]*[a-f0-9]+:	66 41 0f d6 38       	movq   %xmm7,\(%r8\)
[ 	]*[a-f0-9]+:	41 0f 6e 00          	movd   \(%r8\),%mm0
[ 	]*[a-f0-9]+:	0f 6e 00             	movd   \(%rax\),%mm0
[ 	]*[a-f0-9]+:	41 0f 6e 38          	movd   \(%r8\),%mm7
[ 	]*[a-f0-9]+:	0f 6e 38             	movd   \(%rax\),%mm7
[ 	]*[a-f0-9]+:	0f 6e c0             	movd   %eax,%mm0
[ 	]*[a-f0-9]+:	0f 6e f8             	movd   %eax,%mm7
[ 	]*[a-f0-9]+:	41 0f 7e 00          	movd   %mm0,\(%r8\)
[ 	]*[a-f0-9]+:	0f 7e 00             	movd   %mm0,\(%rax\)
[ 	]*[a-f0-9]+:	41 0f 7e 38          	movd   %mm7,\(%r8\)
[ 	]*[a-f0-9]+:	0f 7e 38             	movd   %mm7,\(%rax\)
[ 	]*[a-f0-9]+:	0f 7e c0             	movd   %mm0,%eax
[ 	]*[a-f0-9]+:	0f 7e f8             	movd   %mm7,%eax
[ 	]*[a-f0-9]+:	41 0f 6f 00          	movq   \(%r8\),%mm0
[ 	]*[a-f0-9]+:	0f 6f 00             	movq   \(%rax\),%mm0
[ 	]*[a-f0-9]+:	41 0f 6f 38          	movq   \(%r8\),%mm7
[ 	]*[a-f0-9]+:	0f 6f 38             	movq   \(%rax\),%mm7
[ 	]*[a-f0-9]+:	41 0f 7f 00          	movq   %mm0,\(%r8\)
[ 	]*[a-f0-9]+:	0f 7f 00             	movq   %mm0,\(%rax\)
[ 	]*[a-f0-9]+:	41 0f 7f 38          	movq   %mm7,\(%r8\)
[ 	]*[a-f0-9]+:	0f 7f 38             	movq   %mm7,\(%rax\)
[ 	]*[a-f0-9]+:	41 8f 00             	pop    \(%r8\)
[ 	]*[a-f0-9]+:	8f 00                	pop    \(%rax\)
[ 	]*[a-f0-9]+:	0f a1                	pop    %fs
[ 	]*[a-f0-9]+:	0f a1                	pop    %fs
[ 	]*[a-f0-9]+:	0f a9                	pop    %gs
[ 	]*[a-f0-9]+:	0f a9                	pop    %gs
[ 	]*[a-f0-9]+:	9d                   	popf
[ 	]*[a-f0-9]+:	9d                   	popf
[ 	]*[a-f0-9]+:	41 ff 30             	push   \(%r8\)
[ 	]*[a-f0-9]+:	ff 30                	push   \(%rax\)
[ 	]*[a-f0-9]+:	0f a0                	push   %fs
[ 	]*[a-f0-9]+:	0f a0                	push   %fs
[ 	]*[a-f0-9]+:	0f a8                	push   %gs
[ 	]*[a-f0-9]+:	0f a8                	push   %gs
[ 	]*[a-f0-9]+:	9c                   	pushf
[ 	]*[a-f0-9]+:	9c                   	pushf
[ 	]*[a-f0-9]+:	0f 77                	emms
[ 	]*[a-f0-9]+:	0f 0e                	femms
[ 	]*[a-f0-9]+:	0f 08                	invd
[ 	]*[a-f0-9]+:	41 0f 01 38          	invlpg \(%r8\)
[ 	]*[a-f0-9]+:	0f 01 38             	invlpg \(%rax\)
[ 	]*[a-f0-9]+:	41 0f 01 38          	invlpg \(%r8\)
[ 	]*[a-f0-9]+:	0f 01 38             	invlpg \(%rax\)
[ 	]*[a-f0-9]+:	41 0f 01 38          	invlpg \(%r8\)
[ 	]*[a-f0-9]+:	0f 01 38             	invlpg \(%rax\)
[ 	]*[a-f0-9]+:	0f 00 c0             	sldt   %eax
[ 	]*[a-f0-9]+:	0f 00 c0             	sldt   %eax
[ 	]*[a-f0-9]+:	66 0f 00 c0          	sldt   %ax
[ 	]*[a-f0-9]+:	0f 00 00             	sldt   \(%rax\)
[ 	]*[a-f0-9]+:	e6 00                	out    %al,\$0x0
[ 	]*[a-f0-9]+:	66 e7 00             	out    %ax,\$0x0
[ 	]*[a-f0-9]+:	e7 00                	out    %eax,\$0x0
[ 	]*[a-f0-9]+:	66 90                	xchg   %ax,%ax
[ 	]*[a-f0-9]+:	87 c0                	xchg   %eax,%eax
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	48 90                	rex.W nop
[ 	]*[a-f0-9]+:	49 90                	xchg   %rax,%r8
[ 	]*[a-f0-9]+:	41 90                	xchg   %eax,%r8d
[ 	]*[a-f0-9]+:	41 90                	xchg   %eax,%r8d
[ 	]*[a-f0-9]+:	41 91                	xchg   %eax,%r9d
[ 	]*[a-f0-9]+:	41 91                	xchg   %eax,%r9d
[ 	]*[a-f0-9]+:	93                   	xchg   %eax,%ebx
[ 	]*[a-f0-9]+:	93                   	xchg   %eax,%ebx
[ 	]*[a-f0-9]+:	66 41 90             	xchg   %ax,%r8w
[ 	]*[a-f0-9]+:	66 41 90             	xchg   %ax,%r8w
[ 	]*[a-f0-9]+:	66 41 91             	xchg   %ax,%r9w
[ 	]*[a-f0-9]+:	66 41 91             	xchg   %ax,%r9w
[ 	]*[a-f0-9]+:	48 0f 01 e0          	smsw   %rax
[ 	]*[a-f0-9]+:	0f 01 e0             	smsw   %eax
[ 	]*[a-f0-9]+:	66 0f 01 e0          	smsw   %ax
[ 	]*[a-f0-9]+:	0f 01 20             	smsw   \(%rax\)
[ 	]*[a-f0-9]+:	0f 00 c8             	str    %eax
[ 	]*[a-f0-9]+:	0f 00 c8             	str    %eax
[ 	]*[a-f0-9]+:	66 0f 00 c8          	str    %ax
[ 	]*[a-f0-9]+:	0f 00 08             	str    \(%rax\)
[ 	]*[a-f0-9]+:	0f 05                	syscall
[ 	]*[a-f0-9]+:	0f 07                	sysretl
[ 	]*[a-f0-9]+:	48 0f 07             	sysretq
[ 	]*[a-f0-9]+:	0f 01 f8             	swapgs
[ 	]*[a-f0-9]+:	66 68 22 22          	pushw  \$0x2222
[ 	]*[a-f0-9]+:	f1                   	int1
[ 	]*[a-f0-9]+:	cc                   	int3
[ 	]*[a-f0-9]+:	cd 90                	int    \$0x90
[ 	]*[a-f0-9]+:	f6 c9 01             	test   \$(0x)?0*1,%cl
[ 	]*[a-f0-9]+:	66 f7 c9 02 00       	test   \$(0x)?0*2,%cx
[ 	]*[a-f0-9]+:	f7 c9 04 00 00 00    	test   \$(0x)?0*4,%ecx
[ 	]*[a-f0-9]+:	48 f7 c9 08 00 00 00 	test   \$(0x)?0*8,%rcx
[ 	]*[a-f0-9]+:	c0 f0 02             	shl    \$0x2,%al
[ 	]*[a-f0-9]+:	c1 f0 01             	shl    \$0x1,%eax
[ 	]*[a-f0-9]+:	48 c1 f0 01          	shl    \$0x1,%rax
[ 	]*[a-f0-9]+:	d0 f0                	shl    %al
[ 	]*[a-f0-9]+:	d1 f0                	shl    %eax
[ 	]*[a-f0-9]+:	48 d1 f0             	shl    %rax
[ 	]*[a-f0-9]+:	d2 f0                	shl    %cl,%al
[ 	]*[a-f0-9]+:	d3 f0                	shl    %cl,%eax
[ 	]*[a-f0-9]+:	48 d3 f0             	shl    %cl,%rax
#pass
