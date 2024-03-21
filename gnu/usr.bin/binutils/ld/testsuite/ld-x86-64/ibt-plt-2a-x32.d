#source: ibt-plt-2.s
#as: --x32
#ld: -shared -m elf32_x86_64 -z ibtplt --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw

.*: +file format .*


Disassembly of section .plt:

0+140 <.plt>:
 +[a-f0-9]+:	ff 35 4a 01 20 00    	push   0x20014a\(%rip\)        # 200290 <_GLOBAL_OFFSET_TABLE_\+0x8>
 +[a-f0-9]+:	ff 25 4c 01 20 00    	jmp    \*0x20014c\(%rip\)        # 200298 <_GLOBAL_OFFSET_TABLE_\+0x10>
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 e2 ff ff ff       	jmp    140 <bar1@plt-0x30>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	68 01 00 00 00       	push   \$0x1
 +[a-f0-9]+:	e9 d2 ff ff ff       	jmp    140 <bar1@plt-0x30>
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax

Disassembly of section .plt.sec:

0+170 <bar1@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 26 01 20 00    	jmp    \*0x200126\(%rip\)        # 2002a0 <bar1>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

0+180 <bar2@plt>:
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	ff 25 1e 01 20 00    	jmp    \*0x20011e\(%rip\)        # 2002a8 <bar2>
 +[a-f0-9]+:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)

Disassembly of section .text:

0+190 <foo>:
 +[a-f0-9]+:	48 83 ec 08          	sub    \$0x8,%rsp
 +[a-f0-9]+:	e8 e7 ff ff ff       	call   180 <bar2@plt>
 +[a-f0-9]+:	48 83 c4 08          	add    \$0x8,%rsp
 +[a-f0-9]+:	e9 ce ff ff ff       	jmp    170 <bar1@plt>
#pass
