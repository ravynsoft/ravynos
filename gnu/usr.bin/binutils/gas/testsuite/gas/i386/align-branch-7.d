#as: -malign-branch-boundary=32 -malign-branch-prefix-size=4
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
 +[a-f0-9]+:	3e 66 0f 3a 60 00 03 	pcmpestrm \$0x3,%ds:\(%eax\),%xmm0
 +[a-f0-9]+:	3e 3e 89 e5          	ds ds mov %esp,%ebp
 +[a-f0-9]+:	89 bd 1c ff ff ff    	mov    %edi,-0xe4\(%ebp\)
 +[a-f0-9]+:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
 +[a-f0-9]+:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
 +[a-f0-9]+:	89 75 f4             	mov    %esi,-0xc\(%ebp\)
 +[a-f0-9]+:	65 a3 01 00 00 00    	mov    %eax,%gs:0x1
 +[a-f0-9]+:	a8 04                	test   \$0x4,%al
 +[a-f0-9]+:	70 dc                	jo     0 <foo>
#pass
