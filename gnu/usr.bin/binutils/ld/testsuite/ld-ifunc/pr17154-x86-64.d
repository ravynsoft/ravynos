#source: pr17154-x86.s
#as: --64
#ld: -shared -melf_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw
#target: x86_64-*-*

#...
0+240 <.*>:
 +[a-f0-9]+:	ff 35 5a 01 20 00    	push   0x20015a\(%rip\)        # 2003a0 <_GLOBAL_OFFSET_TABLE_\+0x8>
 +[a-f0-9]+:	ff 25 5c 01 20 00    	jmp    \*0x20015c\(%rip\)        # 2003a8 <_GLOBAL_OFFSET_TABLE_\+0x10>
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%rax\)

0+250 <\*ABS\*\+0x29a@plt>:
 +[a-f0-9]+:	ff 25 5a 01 20 00    	jmp    \*0x20015a\(%rip\)        # 2003b0 <_GLOBAL_OFFSET_TABLE_\+0x18>
 +[a-f0-9]+:	68 03 00 00 00       	push   \$0x3
 +[a-f0-9]+:	e9 e0 ff ff ff       	jmp    240 <\*ABS\*\+0x29a@plt-0x10>

0+260 <func1@plt>:
 +[a-f0-9]+:	ff 25 52 01 20 00    	jmp    \*0x200152\(%rip\)        # 2003b8 <func1>
 +[a-f0-9]+:	68 00 00 00 00       	push   \$0x0
 +[a-f0-9]+:	e9 d0 ff ff ff       	jmp    240 <\*ABS\*\+0x29a@plt-0x10>

0+270 <func2@plt>:
 +[a-f0-9]+:	ff 25 4a 01 20 00    	jmp    \*0x20014a\(%rip\)        # 2003c0 <func2>
 +[a-f0-9]+:	68 01 00 00 00       	push   \$0x1
 +[a-f0-9]+:	e9 c0 ff ff ff       	jmp    240 <\*ABS\*\+0x29a@plt-0x10>

0+280 <\*ABS\*\+0x290@plt>:
 +[a-f0-9]+:	ff 25 42 01 20 00    	jmp    \*0x200142\(%rip\)        # 2003c8 <_GLOBAL_OFFSET_TABLE_\+0x30>
 +[a-f0-9]+:	68 02 00 00 00       	push   \$0x2
 +[a-f0-9]+:	e9 b0 ff ff ff       	jmp    240 <\*ABS\*\+0x29a@plt-0x10>

Disassembly of section .text:

0+290 <resolve1>:
 +[a-f0-9]+:	e8 cb ff ff ff       	call   260 <func1@plt>

0+295 <g1>:
 +[a-f0-9]+:	e9 e6 ff ff ff       	jmp    280 <\*ABS\*\+0x290@plt>

0+29a <resolve2>:
 +[a-f0-9]+:	e8 d1 ff ff ff       	call   270 <func2@plt>

0+29f <g2>:
 +[a-f0-9]+:	e9 ac ff ff ff       	jmp    250 <\*ABS\*\+0x29a@plt>
#pass
