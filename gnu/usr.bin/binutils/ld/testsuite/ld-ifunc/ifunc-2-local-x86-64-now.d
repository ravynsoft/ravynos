#source: ifunc-2-local-x86-64.s
#as: --64
#ld: -z now -shared -melf_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw
#target: x86_64-*-*

.*: +file format .*


Disassembly of section .plt:

0+170 <\*ABS\*\+0x190@plt-0x10>:
 +[a-f0-9]+:	ff 35 42 01 20 00    	push   0x200142\(%rip\)        # 2002b8 <_GLOBAL_OFFSET_TABLE_\+0x8>
 +[a-f0-9]+:	ff 25 44 01 20 00    	jmp    \*0x200144\(%rip\)        # 2002c0 <_GLOBAL_OFFSET_TABLE_\+0x10>
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)

0+180 <\*ABS\*\+0x190@plt>:
 +[a-f0-9]+:	ff 25 42 01 20 00    	jmp    \*0x200142\(%rip\)        # 2002c8 <_GLOBAL_OFFSET_TABLE_\+0x18>
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 e0 ff ff ff       	jmp    170 <\*ABS\*\+0x190@plt-0x10>

Disassembly of section .text:

0+190 <foo>:
 +[a-f0-9]+:	c3                   	ret

0+191 <bar>:
 +[a-f0-9]+:	e8 ea ff ff ff       	call   180 <\*ABS\*\+0x190@plt>
 +[a-f0-9]+:	48 8d 05 e3 ff ff ff 	lea    -0x1d\(%rip\),%rax        # 180 <\*ABS\*\+0x190@plt>
 +[a-f0-9]+:	c3                   	ret
#pass
