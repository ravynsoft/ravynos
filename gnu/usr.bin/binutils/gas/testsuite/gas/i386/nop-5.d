#objdump: -drw
#name: i386 .nops 5

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	85 c0                	test   %eax,%eax
 +[a-f0-9]+:	8d 74 26 00          	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d 74 26 00          	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	8d 76 00             	lea    0x0\(%esi\),%esi
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax

Disassembly of section .altinstr_replacement:

0+ <.altinstr_replacement>:
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	e9 fc ff ff ff       	jmp    9 <.altinstr_replacement\+0x9>	9: (R_386_PC)?(DISP)?32	foo
#pass
