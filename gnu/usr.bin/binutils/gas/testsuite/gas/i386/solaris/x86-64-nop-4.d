#source: ../nop-4.s
#objdump: -drw
#name: x86-64 .nops 4

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	85 c0                	test   %eax,%eax
 +[a-f0-9]+:	66 0f 1f 84 00 00 00 00 00 	nopw   0x0\(%rax,%rax,1\)
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax
 +[a-f0-9]+:	31 c0                	xor    %eax,%eax

Disassembly of section .altinstr_replacement:

0+ <.altinstr_replacement>:
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	89 c0                	mov    %eax,%eax
 +[a-f0-9]+:	e9 00 00 00 00       	jmp    b <_start\+0xb>	7: R_X86_64_PC32	foo-0x4
#pass
