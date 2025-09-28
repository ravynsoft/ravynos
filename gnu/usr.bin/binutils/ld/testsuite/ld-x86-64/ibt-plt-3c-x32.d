#source: ibt-plt-3.s
#as: --x32
#ld: -shared -m elf32_x86_64 -z ibt --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .plt:

[a-f0-9]+ <.plt>:
 +[a-f0-9]+:	ff 35 ([0-9a-f]{2} ){4}[ 	]+push   0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <_GLOBAL_OFFSET_TABLE_\+0x8>
 +[a-f0-9]+:	ff 25 ([0-9a-f]{2} ){4}[ 	]+jmp    \*0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <_GLOBAL_OFFSET_TABLE_\+0x10>
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 e2 ff ff ff       	jmp    [a-f0-9]+ <bar1@plt-0x30>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 01 00 00 00       	push   \$0x1
 +[a-f0-9]+:	e9 d2 ff ff ff       	jmp    [a-f0-9]+ <bar1@plt-0x30>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax

Disassembly of section .plt.sec:

[a-f0-9]+ <bar1@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 ([0-9a-f]{2} ){4}[ 	]+jmp    \*0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <bar1>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

[a-f0-9]+ <bar2@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 ([0-9a-f]{2} ){4}[ 	]+jmp    \*0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <bar2>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

Disassembly of section .text:

[a-f0-9]+ <foo>:
 +[a-f0-9]+:	48 83 ec 08          	sub    \$0x8,%rsp
 +[a-f0-9]+:	e8 e7 ff ff ff       	call   [a-f0-9]+ <bar2@plt>
 +[a-f0-9]+:	48 83 c4 08          	add    \$0x8,%rsp
 +[a-f0-9]+:	e9 ce ff ff ff       	jmp    [a-f0-9]+ <bar1@plt>
#pass
