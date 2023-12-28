#source: pr20253-1.s
#as: --64
#ld: -melf_x86_64 -z max-page-size=0x200000 -z noseparate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+4000e0 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+4000e1 <bar>:
 +[a-f0-9]+:	c3                   	ret

0+4000e2 <_start>:
 +[a-f0-9]+:	ff 15 28 00 20 00    	call   \*0x200028\(%rip\)        # 600110 <.*>
 +[a-f0-9]+:	ff 25 2a 00 20 00    	jmp    \*0x20002a\(%rip\)        # 600118 <.*>
 +[a-f0-9]+:	48 c7 05 1f 00 20 00 00 00 00 00 	movq   \$0x0,0x20001f\(%rip\)        # 600118 <.*>
 +[a-f0-9]+:	48 83 3d 0f 00 20 00 00 	cmpq   \$0x0,0x20000f\(%rip\)        # 600110 <.*>
 +[a-f0-9]+:	48 3b 0d 08 00 20 00 	cmp    0x200008\(%rip\),%rcx        # 600110 <.*>
 +[a-f0-9]+:	48 3b 0d 09 00 20 00 	cmp    0x200009\(%rip\),%rcx        # 600118 <.*>
#pass
