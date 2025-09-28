#source: pr20253-1.s
#as: --x32
#ld: -melf32_x86_64 -z max-page-size=0x200000 -z noseparate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+40008c <foo>:
 +[a-f0-9]+:	c3                   	ret

0+40008d <bar>:
 +[a-f0-9]+:	c3                   	ret

0+40008e <_start>:
 +[a-f0-9]+:	ff 15 2c 00 20 00    	call   \*0x20002c\(%rip\)        # 6000c0 <_start\+0x200032>
 +[a-f0-9]+:	ff 25 2e 00 20 00    	jmp    \*0x20002e\(%rip\)        # 6000c8 <_start\+0x20003a>
 +[a-f0-9]+:	48 c7 05 23 00 20 00 00 00 00 00 	movq   \$0x0,0x200023\(%rip\)        # 6000c8 <_start\+0x20003a>
 +[a-f0-9]+:	48 83 3d 13 00 20 00 00 	cmpq   \$0x0,0x200013\(%rip\)        # 6000c0 <_start\+0x200032>
 +[a-f0-9]+:	48 3b 0d 0c 00 20 00 	cmp    0x20000c\(%rip\),%rcx        # 6000c0 <_start\+0x200032>
 +[a-f0-9]+:	48 3b 0d 0d 00 20 00 	cmp    0x20000d\(%rip\),%rcx        # 6000c8 <_start\+0x20003a>
#pass
