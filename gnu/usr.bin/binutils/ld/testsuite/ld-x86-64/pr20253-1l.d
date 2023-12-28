#source: pr20253-1.s
#as: --x32
#ld: -shared -melf32_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+100 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+101 <bar>:
 +[a-f0-9]+:	c3                   	ret

0+102 <_start>:
 +[a-f0-9]+:	ff 15 98 00 20 00    	call   \*0x200098\(%rip\)        # 2001a0 <.*>
 +[a-f0-9]+:	ff 25 9a 00 20 00    	jmp    \*0x20009a\(%rip\)        # 2001a8 <.*>
 +[a-f0-9]+:	48 c7 05 8f 00 20 00 00 00 00 00 	movq   \$0x0,0x20008f\(%rip\)        # 2001a8 <.*>
 +[a-f0-9]+:	48 83 3d 7f 00 20 00 00 	cmpq   \$0x0,0x20007f\(%rip\)        # 2001a0 <.*>
 +[a-f0-9]+:	48 3b 0d 78 00 20 00 	cmp    0x200078\(%rip\),%rcx        # 2001a0 <.*>
 +[a-f0-9]+:	48 3b 0d 79 00 20 00 	cmp    0x200079\(%rip\),%rcx        # 2001a8 <.*>
#pass
