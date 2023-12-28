#source: pr20253-1.s
#as: --64
#ld: -shared -melf_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+188 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+189 <bar>:
 +[a-f0-9]+:	c3                   	ret

0+18a <_start>:
 +[a-f0-9]+:	ff 15 08 01 20 00    	call   \*0x200108\(%rip\)        # 200298 <.*>
 +[a-f0-9]+:	ff 25 0a 01 20 00    	jmp    \*0x20010a\(%rip\)        # 2002a0 <.*>
 +[a-f0-9]+:	48 c7 05 ff 00 20 00 00 00 00 00 	movq   \$0x0,0x2000ff\(%rip\)        # 2002a0 <.*>
 +[a-f0-9]+:	48 83 3d ef 00 20 00 00 	cmpq   \$0x0,0x2000ef\(%rip\)        # 200298 <.*>
 +[a-f0-9]+:	48 3b 0d e8 00 20 00 	cmp    0x2000e8\(%rip\),%rcx        # 200298 <.*>
 +[a-f0-9]+:	48 3b 0d e9 00 20 00 	cmp    0x2000e9\(%rip\),%rcx        # 2002a0 <.*>
#pass
