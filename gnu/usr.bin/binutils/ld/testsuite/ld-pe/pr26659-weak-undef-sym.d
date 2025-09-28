#source: pr26659-weak-undef-sym.s
#target: x86_64-*-cygwin* x86_64-*-pe x86_64-*-mingw*
#ld: -e0
#objdump: -d

#...
00000001[04]0[04]01000 <foo>:
 *[0-9a-f]+:	55                   	push   %rbp
 *[0-9a-f]+:	48 89 e5             	mov    %rsp,%rbp
 *[0-9a-f]+:	48 83 ec 20          	sub    \$0x20,%rsp
 *[0-9a-f]+:	89 4d 10             	mov    %ecx,0x10\(%rbp\)
 *[0-9a-f]+:	48 8b 05 ee 0f 00 00 	mov    0xfee\(%rip\),%rax        # [0-9a-f]+ <__data_end__>
 *[0-9a-f]+:	48 85 c0             	test   %rax,%rax
 *[0-9a-f]+:	74 05                	je     [0-9a-f]+ <foo\+0x1c>
 *[0-9a-f]+:	e8 e4 ef [fb]f [fb]f       	call   100000000 <__size_of_stack_reserve__\+0xffe00000>
 *[0-9a-f]+:	48 8b 05 ed 0f 00 00 	mov    0xfed\(%rip\),%rax        # [0-9a-f]+ <.refptr.bar2>
 *[0-9a-f]+:	48 85 c0             	test   %rax,%rax
 *[0-9a-f]+:	74 05                	je     [0-9a-f]+ <foo\+0x2d>
 *[0-9a-f]+:	e8 d3 ef [fb]f [fb]f       	call   100000000 <__size_of_stack_reserve__\+0xffe00000>
 *[0-9a-f]+:	8b 45 10             	mov    0x10\(%rbp\),%eax
 *[0-9a-f]+:	0f af c0             	imul   %eax,%eax
 *[0-9a-f]+:	48 83 c4 20          	add    \$0x20,%rsp
 *[0-9a-f]+:	5d                   	pop    %rbp
 *[0-9a-f]+:	c3                   	ret *
#pass
