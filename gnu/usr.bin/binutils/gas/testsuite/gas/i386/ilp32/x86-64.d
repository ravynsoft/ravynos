#as: -J
#objdump: -dw
#name: x86-64 (ILP32)

.*: +file format .*

Disassembly of section .text:

0+ <.*>:
[ 	]*[a-f0-9]+:	01 ca                	add    %ecx,%edx
[ 	]*[a-f0-9]+:	44 01 ca             	add    %r9d,%edx
[ 	]*[a-f0-9]+:	41 01 ca             	add    %ecx,%r10d
[ 	]*[a-f0-9]+:	48 01 ca             	add    %rcx,%rdx
[ 	]*[a-f0-9]+:	4d 01 ca             	add    %r9,%r10
[ 	]*[a-f0-9]+:	41 01 c0             	add    %eax,%r8d
[ 	]*[a-f0-9]+:	66 41 01 c0          	add    %ax,%r8w
[ 	]*[a-f0-9]+:	49 01 c0             	add    %rax,%r8
[ 	]*[a-f0-9]+:	05 11 22 33 44       	add    \$0x44332211,%eax
[ 	]*[a-f0-9]+:	48 05 11 22 33 f4    	add    \$0xfffffffff4332211,%rax
[ 	]*[a-f0-9]+:	66 05 33 44          	add    \$0x4433,%ax
[ 	]*[a-f0-9]+:	48 05 11 22 33 44    	add    \$0x44332211,%rax
[ 	]*[a-f0-9]+:	00 ca                	add    %cl,%dl
[ 	]*[a-f0-9]+:	00 f7                	add    %dh,%bh
[ 	]*[a-f0-9]+:	40 00 f7             	add    %sil,%dil
[ 	]*[a-f0-9]+:	41 00 f7             	add    %sil,%r15b
[ 	]*[a-f0-9]+:	44 00 f7             	add    %r14b,%dil
[ 	]*[a-f0-9]+:	45 00 f7             	add    %r14b,%r15b
[ 	]*[a-f0-9]+:	50                   	push   %rax
[ 	]*[a-f0-9]+:	41 50                	push   %r8
[ 	]*[a-f0-9]+:	41 59                	pop    %r9
[ 	]*[a-f0-9]+:	04 11                	add    \$0x11,%al
[ 	]*[a-f0-9]+:	80 c4 11             	add    \$0x11,%ah
[ 	]*[a-f0-9]+:	40 80 c4 11          	add    \$0x11,%spl
[ 	]*[a-f0-9]+:	41 80 c0 11          	add    \$0x11,%r8b
[ 	]*[a-f0-9]+:	41 80 c4 11          	add    \$0x11,%r12b
[ 	]*[a-f0-9]+:	0f 20 c0             	mov    %cr0,%rax
[ 	]*[a-f0-9]+:	41 0f 20 c0          	mov    %cr0,%r8
[ 	]*[a-f0-9]+:	44 0f 20 c0          	mov    %cr8,%rax
[ 	]*[a-f0-9]+:	44 0f 22 c0          	mov    %rax,%cr8
[ 	]*[a-f0-9]+:	f3 48 a5             	rep movsq %ds:\(%rsi\),%es:\(%rdi\)
[ 	]*[a-f0-9]+:	66 f3 a5             	rep movsw %ds:\(%rsi\),%es:\(%rdi\)
[ 	]*[a-f0-9]+:	f3 48 a5             	rep movsq %ds:\(%rsi\),%es:\(%rdi\)
[ 	]*[a-f0-9]+:	b0 11                	mov    \$0x11,%al
[ 	]*[a-f0-9]+:	b4 11                	mov    \$0x11,%ah
[ 	]*[a-f0-9]+:	40 b4 11             	mov    \$0x11,%spl
[ 	]*[a-f0-9]+:	41 b4 11             	mov    \$0x11,%r12b
[ 	]*[a-f0-9]+:	b8 44 33 22 11       	mov    \$0x11223344,%eax
[ 	]*[a-f0-9]+:	41 b8 44 33 22 11    	mov    \$0x11223344,%r8d
[ 	]*[a-f0-9]+:	48 b8 88 77 66 55 44 33 22 11 	movabs \$0x1122334455667788,%rax
[ 	]*[a-f0-9]+:	49 b8 88 77 66 55 44 33 22 11 	movabs \$0x1122334455667788,%r8
[ 	]*[a-f0-9]+:	03 00                	add    \(%rax\),%eax
[ 	]*[a-f0-9]+:	41 03 00             	add    \(%r8\),%eax
[ 	]*[a-f0-9]+:	45 03 00             	add    \(%r8\),%r8d
[ 	]*[a-f0-9]+:	49 03 00             	add    \(%r8\),%rax
[ 	]*[a-f0-9]+:	03 05 22 22 22 22    	add    0x22222222\(%rip\),%eax        # 222222c7 <foo\+0x222220d6>
[ 	]*[a-f0-9]+:	03 45 00             	add    0x0\(%rbp\),%eax
[ 	]*[a-f0-9]+:	03 04 25 22 22 22 22 	add    0x22222222,%eax
[ 	]*[a-f0-9]+:	41 03 45 00          	add    0x0\(%r13\),%eax
[ 	]*[a-f0-9]+:	03 04 80             	add    \(%rax,%rax,4\),%eax
[ 	]*[a-f0-9]+:	41 03 04 80          	add    \(%r8,%rax,4\),%eax
[ 	]*[a-f0-9]+:	45 03 04 80          	add    \(%r8,%rax,4\),%r8d
[ 	]*[a-f0-9]+:	43 03 04 80          	add    \(%r8,%r8,4\),%eax
[ 	]*[a-f0-9]+:	46 01 04 81          	add    %r8d,\(%rcx,%r8,4\)
[ 	]*[a-f0-9]+:	03 14 c0             	add    \(%rax,%rax,8\),%edx
[ 	]*[a-f0-9]+:	03 14 c8             	add    \(%rax,%rcx,8\),%edx
[ 	]*[a-f0-9]+:	03 14 d0             	add    \(%rax,%rdx,8\),%edx
[ 	]*[a-f0-9]+:	03 14 d8             	add    \(%rax,%rbx,8\),%edx
[ 	]*[a-f0-9]+:	03 10                	add    \(%rax\),%edx
[ 	]*[a-f0-9]+:	03 14 e8             	add    \(%rax,%rbp,8\),%edx
[ 	]*[a-f0-9]+:	03 14 f0             	add    \(%rax,%rsi,8\),%edx
[ 	]*[a-f0-9]+:	03 14 f8             	add    \(%rax,%rdi,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 c0          	add    \(%rax,%r8,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 c8          	add    \(%rax,%r9,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 d0          	add    \(%rax,%r10,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 d8          	add    \(%rax,%r11,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 e0          	add    \(%rax,%r12,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 e8          	add    \(%rax,%r13,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 f0          	add    \(%rax,%r14,8\),%edx
[ 	]*[a-f0-9]+:	42 03 14 f8          	add    \(%rax,%r15,8\),%edx
[ 	]*[a-f0-9]+:	83 c1 11             	add    \$0x11,%ecx
[ 	]*[a-f0-9]+:	83 00 11             	addl   \$0x11,\(%rax\)
[ 	]*[a-f0-9]+:	48 83 00 11          	addq   \$0x11,\(%rax\)
[ 	]*[a-f0-9]+:	41 83 00 11          	addl   \$0x11,\(%r8\)
[ 	]*[a-f0-9]+:	83 04 81 11          	addl   \$0x11,\(%rcx,%rax,4\)
[ 	]*[a-f0-9]+:	41 83 04 81 11       	addl   \$0x11,\(%r9,%rax,4\)
[ 	]*[a-f0-9]+:	42 83 04 81 11       	addl   \$0x11,\(%rcx,%r8,4\)
[ 	]*[a-f0-9]+:	83 05 22 22 22 22 33 	addl   \$0x33,0x22222222\(%rip\)        # 22222342 <foo\+0x22222151>
[ 	]*[a-f0-9]+:	48 83 05 22 22 22 22 33 	addq   \$0x33,0x22222222\(%rip\)        # 2222234a <foo\+0x22222159>
[ 	]*[a-f0-9]+:	81 05 22 22 22 22 33 33 33 33 	addl   \$0x33333333,0x22222222\(%rip\)        # 22222354 <foo\+0x22222163>
[ 	]*[a-f0-9]+:	48 81 05 22 22 22 22 33 33 33 33 	addq   \$0x33333333,0x22222222\(%rip\)        # 2222235f <foo\+0x2222216e>
[ 	]*[a-f0-9]+:	83 04 c5 22 22 22 22 33 	addl   \$0x33,0x22222222\(,%rax,8\)
[ 	]*[a-f0-9]+:	83 80 22 22 22 22 33 	addl   \$0x33,0x22222222\(%rax\)
[ 	]*[a-f0-9]+:	83 80 22 22 22 22 33 	addl   \$0x33,0x22222222\(%rax\)
[ 	]*[a-f0-9]+:	41 83 04 e8 33       	addl   \$0x33,\(%r8,%rbp,8\)
[ 	]*[a-f0-9]+:	83 04 25 22 22 22 22 33 	addl   \$0x33,0x22222222
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%al
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%eax
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs %al,0x8877665544332211
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs %eax,0x8877665544332211
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%rax
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs %rax,0x8877665544332211
[ 	]*[a-f0-9]+:	48 99                	cqto
[ 	]*[a-f0-9]+:	48 98                	cltq
[ 	]*[a-f0-9]+:	48 63 c0             	movslq %eax,%rax
[ 	]*[a-f0-9]+:	48 0f bf c0          	movswq %ax,%rax
[ 	]*[a-f0-9]+:	48 0f be c0          	movsbq %al,%rax

0+1a7 <bar>:
[ 	]*[a-f0-9]+:	b0 00                	mov    \$0x0,%al
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    \$0x0,%ax
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 00 	mov    0x0,%eax
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    0x0\(%rax\),%eax
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    0x0\(%rip\),%eax        # 1cc <bar\+0x25>
[ 	]*[a-f0-9]+:	b0 00                	mov    \$0x0,%al
[ 	]*[a-f0-9]+:	66 b8 00 00          	mov    \$0x0,%ax
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 00 	mov    0x0,%eax
[ 	]*[a-f0-9]+:	8b 80 00 00 00 00    	mov    0x0\(%rax\),%eax
[ 	]*[a-f0-9]+:	8b 05 00 00 00 00    	mov    0x0\(%rip\),%eax        # 1f1 <foo>

0+1f1 <foo>:
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%al
[ 	]*[a-f0-9]+:	66 a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%ax
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%eax
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%rax
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs %al,0x8877665544332211
[ 	]*[a-f0-9]+:	66 a3 11 22 33 44 55 66 77 88 	movabs %ax,0x8877665544332211
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs %eax,0x8877665544332211
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs %rax,0x8877665544332211
[ 	]*[a-f0-9]+:	a0 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%al
[ 	]*[a-f0-9]+:	66 a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%ax
[ 	]*[a-f0-9]+:	a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%eax
[ 	]*[a-f0-9]+:	48 a1 11 22 33 44 55 66 77 88 	movabs 0x8877665544332211,%rax
[ 	]*[a-f0-9]+:	a2 11 22 33 44 55 66 77 88 	movabs %al,0x8877665544332211
[ 	]*[a-f0-9]+:	66 a3 11 22 33 44 55 66 77 88 	movabs %ax,0x8877665544332211
[ 	]*[a-f0-9]+:	a3 11 22 33 44 55 66 77 88 	movabs %eax,0x8877665544332211
[ 	]*[a-f0-9]+:	48 a3 11 22 33 44 55 66 77 88 	movabs %rax,0x8877665544332211
[ 	]*[a-f0-9]+:	8a 04 25 11 22 33 ff 	mov    0xffffffffff332211,%al
[ 	]*[a-f0-9]+:	66 8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%ax
[ 	]*[a-f0-9]+:	8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%eax
[ 	]*[a-f0-9]+:	48 8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%rax
[ 	]*[a-f0-9]+:	88 04 25 11 22 33 ff 	mov    %al,0xffffffffff332211
[ 	]*[a-f0-9]+:	66 89 04 25 11 22 33 ff 	mov    %ax,0xffffffffff332211
[ 	]*[a-f0-9]+:	89 04 25 11 22 33 ff 	mov    %eax,0xffffffffff332211
[ 	]*[a-f0-9]+:	48 89 04 25 11 22 33 ff 	mov    %rax,0xffffffffff332211
[ 	]*[a-f0-9]+:	8a 04 25 11 22 33 ff 	mov    0xffffffffff332211,%al
[ 	]*[a-f0-9]+:	66 8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%ax
[ 	]*[a-f0-9]+:	8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%eax
[ 	]*[a-f0-9]+:	48 8b 04 25 11 22 33 ff 	mov    0xffffffffff332211,%rax
[ 	]*[a-f0-9]+:	88 04 25 11 22 33 ff 	mov    %al,0xffffffffff332211
[ 	]*[a-f0-9]+:	66 89 04 25 11 22 33 ff 	mov    %ax,0xffffffffff332211
[ 	]*[a-f0-9]+:	89 04 25 11 22 33 ff 	mov    %eax,0xffffffffff332211
[ 	]*[a-f0-9]+:	48 89 04 25 11 22 33 ff 	mov    %rax,0xffffffffff332211
[ 	]*[a-f0-9]+:	48 0f c7 08          	cmpxchg16b \(%rax\)
[ 	]*[a-f0-9]+:	48 0f c7 08          	cmpxchg16b \(%rax\)
[ 	]*[a-f0-9]+:	66 0f be f0          	movsbw %al,%si
[ 	]*[a-f0-9]+:	0f be f0             	movsbl %al,%esi
[ 	]*[a-f0-9]+:	48 0f be f0          	movsbq %al,%rsi
[ 	]*[a-f0-9]+:	0f bf f0             	movswl %ax,%esi
[ 	]*[a-f0-9]+:	48 0f bf f0          	movswq %ax,%rsi
[ 	]*[a-f0-9]+:	48 63 f0             	movslq %eax,%rsi
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f be 10          	movsbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f bf 10          	movswq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzbw %al,%si
[ 	]*[a-f0-9]+:	0f b6 f0             	movzbl %al,%esi
[ 	]*[a-f0-9]+:	48 0f b6 f0          	movzbq %al,%rsi
[ 	]*[a-f0-9]+:	0f b7 f0             	movzwl %ax,%esi
[ 	]*[a-f0-9]+:	48 0f b7 f0          	movzwq %ax,%rsi
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzwq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f be f0          	movsbw %al,%si
[ 	]*[a-f0-9]+:	0f be f0             	movsbl %al,%esi
[ 	]*[a-f0-9]+:	48 0f be f0          	movsbq %al,%rsi
[ 	]*[a-f0-9]+:	0f bf f0             	movswl %ax,%esi
[ 	]*[a-f0-9]+:	48 0f bf f0          	movswq %ax,%rsi
[ 	]*[a-f0-9]+:	48 63 f0             	movslq %eax,%rsi
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f be 10          	movsbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f bf 10          	movswq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzbw %al,%si
[ 	]*[a-f0-9]+:	0f b6 f0             	movzbl %al,%esi
[ 	]*[a-f0-9]+:	48 0f b6 f0          	movzbq %al,%rsi
[ 	]*[a-f0-9]+:	0f b7 f0             	movzwl %ax,%esi
[ 	]*[a-f0-9]+:	48 0f b7 f0          	movzwq %ax,%rsi
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzwq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	f3 0f 7e 0c 24       	movq   \(%rsp\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f 7e 0c 24       	movq   \(%rsp\),%xmm1
[ 	]*[a-f0-9]+:	66 0f d6 0c 24       	movq   %xmm1,\(%rsp\)
[ 	]*[a-f0-9]+:	66 0f d6 0c 24       	movq   %xmm1,\(%rsp\)
[ 	]*[a-f0-9]+:	df e0                	fnstsw %ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw %ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  %ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  %ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw %ax
[ 	]*[a-f0-9]+:	df e0                	fnstsw %ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  %ax
[ 	]*[a-f0-9]+:	9b df e0             	fstsw  %ax
[ 	]*[a-f0-9]+:	66 0f be 00          	movsbw \(%rax\),%ax
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f be 10          	movsbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f bf 10          	movswq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	48 63 10             	movslq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	48 63 00             	movslq \(%rax\),%rax
[ 	]*[a-f0-9]+:	66 0f b6 00          	movzbw \(%rax\),%ax
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%rax\),%dx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b6 10          	movzbq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%rax\),%edx
[ 	]*[a-f0-9]+:	48 0f b7 10          	movzwq \(%rax\),%rdx
[ 	]*[a-f0-9]+:	0f c3 00             	movnti %eax,\(%rax\)
[ 	]*[a-f0-9]+:	0f c3 00             	movnti %eax,\(%rax\)
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti %rax,\(%rax\)
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti %rax,\(%rax\)
[ 	]*[a-f0-9]+:	66 0f be 00          	movsbw \(%rax\),%ax
[ 	]*[a-f0-9]+:	0f be 00             	movsbl \(%rax\),%eax
[ 	]*[a-f0-9]+:	0f bf 00             	movswl \(%rax\),%eax
[ 	]*[a-f0-9]+:	48 0f bf 00          	movswq \(%rax\),%rax
[ 	]*[a-f0-9]+:	48 63 00             	movslq \(%rax\),%rax
[ 	]*[a-f0-9]+:	48 63 00             	movslq \(%rax\),%rax
[ 	]*[a-f0-9]+:	66 0f b6 00          	movzbw \(%rax\),%ax
[ 	]*[a-f0-9]+:	0f b6 00             	movzbl \(%rax\),%eax
[ 	]*[a-f0-9]+:	0f b7 00             	movzwl \(%rax\),%eax
[ 	]*[a-f0-9]+:	48 0f b7 00          	movzwq \(%rax\),%rax
[ 	]*[a-f0-9]+:	0f c3 00             	movnti %eax,\(%rax\)
[ 	]*[a-f0-9]+:	48 0f c3 00          	movnti %rax,\(%rax\)
#pass
