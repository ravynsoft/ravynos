#as: -J
#objdump: -drw
#name: x86-64 displacement

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	8b 98 ff ff ff 7f    	mov    0x7fffffff\(%rax\),%ebx
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    -0x80000000\(%rax\),%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 00 00 00 80 	mov    0xffffffff80000000,%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 00 00 00 80 	mov    0xffffffff80000000,%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 ff ff ff 7f 	mov    0x7fffffff,%ebx
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 80 	mov    0xffffffff80000000,%eax
[ 	]*[a-f0-9]+:	8b 04 25 00 00 00 80 	mov    0xffffffff80000000,%eax
[ 	]*[a-f0-9]+:	8b 04 25 ff ff ff 7f 	mov    0x7fffffff,%eax
[ 	]*[a-f0-9]+:	a1 00 00 00 80 00 00 00 00 	movabs 0x80000000,%eax
[ 	]*[a-f0-9]+:	b8 f0 00 e0 0e       	mov    \$0xee000f0,%eax
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    %ebx,0xee000f0\(%rax\)
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    %ebx,0xee000f0\(%rax\)
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0\(%rax\)
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0\(%rax\)
[ 	]*[a-f0-9]+:	89 1c 25 f0 00 e0 0e 	mov    %ebx,0xee000f0
[ 	]*[a-f0-9]+:	65 89 1c 25 f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0
[ 	]*[a-f0-9]+:	89 04 25 f0 00 e0 0e 	mov    %eax,0xee000f0
[ 	]*[a-f0-9]+:	65 89 04 25 f0 00 e0 0e 	mov    %eax,%gs:0xee000f0
[ 	]*[a-f0-9]+:	a3 f0 00 e0 fe 00 00 00 00 	movabs %eax,0xfee000f0
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 fe 00 00 00 00 	movabs %eax,%gs:0xfee000f0
[ 	]*[a-f0-9]+:	65 8b 1c 25 f0 00 e0 0e 	mov    %gs:0xee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 f0 00 e0 0e 	mov    0xee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 f0 00 e0 0e 	mov    0xee000f0,%ebx
[ 	]*[a-f0-9]+:	65 8b 04 25 f0 00 e0 0e 	mov    %gs:0xee000f0,%eax
[ 	]*[a-f0-9]+:	8b 04 25 f0 00 e0 0e 	mov    0xee000f0,%eax
[ 	]*[a-f0-9]+:	8b 04 25 f0 00 e0 0e 	mov    0xee000f0,%eax
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 fe 00 00 00 00 	movabs %gs:0xfee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe 00 00 00 00 	movabs 0xfee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe 00 00 00 00 	movabs 0xfee000f0,%eax
#pass
