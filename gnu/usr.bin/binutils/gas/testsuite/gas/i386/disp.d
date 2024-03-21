#as: -J
#objdump: -drw
#name: i386 displacement

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	8b 98 ff ff ff 7f    	mov    0x7fffffff\(%eax\),%ebx
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    -0x80000000\(%eax\),%ebx
[ 	]*[a-f0-9]+:	8b 98 00 00 00 80    	mov    -0x80000000\(%eax\),%ebx
[ 	]*[a-f0-9]+:	8b 1d ff ff ff 7f    	mov    0x7fffffff,%ebx
[ 	]*[a-f0-9]+:	8b 1d 00 00 00 80    	mov    0x80000000,%ebx
[ 	]*[a-f0-9]+:	8b 1d 00 00 00 80    	mov    0x80000000,%ebx
[ 	]*[a-f0-9]+:	a1 ff ff ff 7f       	mov    0x7fffffff,%eax
[ 	]*[a-f0-9]+:	a1 00 00 00 80       	mov    0x80000000,%eax
[ 	]*[a-f0-9]+:	a1 00 00 00 80       	mov    0x80000000,%eax
[ 	]*[a-f0-9]+:	b8 f0 00 e0 0e       	mov    \$0xee000f0,%eax
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    %ebx,0xee000f0\(%eax\)
[ 	]*[a-f0-9]+:	89 98 f0 00 e0 0e    	mov    %ebx,0xee000f0\(%eax\)
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0\(%eax\)
[ 	]*[a-f0-9]+:	65 89 98 f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0\(%eax\)
[ 	]*[a-f0-9]+:	89 1d f0 00 e0 0e    	mov    %ebx,0xee000f0
[ 	]*[a-f0-9]+:	65 89 1d f0 00 e0 0e 	mov    %ebx,%gs:0xee000f0
[ 	]*[a-f0-9]+:	89 1d f0 00 e0 fe    	mov    %ebx,0xfee000f0
[ 	]*[a-f0-9]+:	65 89 1d f0 00 e0 fe 	mov    %ebx,%gs:0xfee000f0
[ 	]*[a-f0-9]+:	a3 f0 00 e0 0e       	mov    %eax,0xee000f0
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 0e    	mov    %eax,%gs:0xee000f0
[ 	]*[a-f0-9]+:	a3 f0 00 e0 fe       	mov    %eax,0xfee000f0
[ 	]*[a-f0-9]+:	65 a3 f0 00 e0 fe    	mov    %eax,%gs:0xfee000f0
[ 	]*[a-f0-9]+:	65 8b 1d f0 00 e0 0e 	mov    %gs:0xee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 0e    	mov    0xee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 0e    	mov    0xee000f0,%ebx
[ 	]*[a-f0-9]+:	65 8b 1d f0 00 e0 fe 	mov    %gs:0xfee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 fe    	mov    0xfee000f0,%ebx
[ 	]*[a-f0-9]+:	8b 1d f0 00 e0 fe    	mov    0xfee000f0,%ebx
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 0e    	mov    %gs:0xee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 0e       	mov    0xee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 0e       	mov    0xee000f0,%eax
[ 	]*[a-f0-9]+:	65 a1 f0 00 e0 fe    	mov    %gs:0xfee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe       	mov    0xfee000f0,%eax
[ 	]*[a-f0-9]+:	a1 f0 00 e0 fe       	mov    0xfee000f0,%eax
#pass
