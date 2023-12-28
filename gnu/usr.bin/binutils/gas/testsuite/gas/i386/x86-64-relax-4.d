#objdump: -drw
#notarget: *-*-solaris*

.*: +file format .*


Disassembly of section .text:

0+ <printk>:
 +[a-f0-9]+:	c3                   	ret

Disassembly of section .init.text:

0+ <foo>:
 +[a-f0-9]+:	e8 00 00 00 00       	call   5 <foo\+0x5>	1: R_X86_64_PC32	.text-0x4
 +[a-f0-9]+:	48 8d 05 00 00 00 00 	lea    0x0\(%rip\),%rax        # c <foo\+0xc>	8: R_X86_64_PC32	.text-0x4
#pass
