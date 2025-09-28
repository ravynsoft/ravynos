#source: pr20253-1.s
#as: --x32
#ld: -pie -melf32_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+120 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+121 <bar>:
 +[a-f0-9]+:	c3                   	ret

0+122 <_start>:
 +[a-f0-9]+:	ff 15 a8 00 20 00    	call   \*0x2000a8\(%rip\)        # 2001d0 <.*>
 +[a-f0-9]+:	ff 25 aa 00 20 00    	jmp    \*0x2000aa\(%rip\)        # 2001d8 <.*>
 +[a-f0-9]+:	48 c7 05 9f 00 20 00 00 00 00 00 	movq   \$0x0,0x20009f\(%rip\)        # 2001d8 <.*>
 +[a-f0-9]+:	48 83 3d 8f 00 20 00 00 	cmpq   \$0x0,0x20008f\(%rip\)        # 2001d0 <.*>
 +[a-f0-9]+:	48 3b 0d 88 00 20 00 	cmp    0x200088\(%rip\),%rcx        # 2001d0 <.*>
 +[a-f0-9]+:	48 3b 0d 89 00 20 00 	cmp    0x200089\(%rip\),%rcx        # 2001d8 <.*>
#pass
