#source: ibt-plt-2.s
#as: --64 -defsym __64_bit__=1
#ld: -shared -m elf_x86_64 -z ibtplt --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw

.*: +file format .*


Disassembly of section .plt:

0+1f0 <.plt>:
 +[a-f0-9]+:	ff 35 ca 01 20 00    	push   0x2001ca\(%rip\)        # 2003c0 <_GLOBAL_OFFSET_TABLE_\+0x8>
 +[a-f0-9]+:	ff 25 cc 01 20 00    	jmp    \*0x2001cc\(%rip\)        # 2003c8 <_GLOBAL_OFFSET_TABLE_\+0x10>
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 e2 ff ff ff       	jmp    1f0 <.*>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 01 00 00 00       	push   \$0x1
 +[a-f0-9]+:	e9 d2 ff ff ff       	jmp    1f0 <.*>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax

Disassembly of section .plt.sec:

0+220 <bar1@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 a6 01 20 00    	jmp    \*0x2001a6\(%rip\)        # 2003d0 <bar1>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

0+230 <bar2@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 9e 01 20 00    	jmp    \*0x20019e\(%rip\)        # 2003d8 <bar2>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

Disassembly of section .text:

0+240 <foo>:
 +[a-f0-9]+:	48 83 ec 08          	sub    \$0x8,%rsp
 +[a-f0-9]+:	e8 e7 ff ff ff       	call   230 <bar2@plt>
 +[a-f0-9]+:	48 83 c4 08          	add    \$0x8,%rsp
 +[a-f0-9]+:	e9 ce ff ff ff       	jmp    220 <bar1@plt>
#pass
