#source: x86-64-gotpcrel.s
#name: x86-64 gotpcrel (no relax)
#as: -mrelax-relocations=no
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	3: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	48 8b 04 25 00 00 00 00 	mov    0x0,%rax	b: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	48 8b 05 00 00 00 00 	mov    0x0\(%rip\),%rax        # 16 <_start\+0x16>	12: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	48 8b 81 00 00 00 00 	mov    0x0\(%rcx\),%rax	19: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	ff 15 00 00 00 00    	call   \*0x0\(%rip\)        # 23 <_start\+0x23>	1f: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	ff 90 00 00 00 00    	call   \*0x0\(%rax\)	25: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	ff 25 00 00 00 00    	jmp    \*0x0\(%rip\)        # 2f <_start\+0x2f>	2b: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	ff a1 00 00 00 00    	jmp    \*0x0\(%rcx\)	31: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	38: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	48 8b 04 25 00 00 00 00 	mov    0x0,%rax	40: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	48 8b 05 00 00 00 00 	mov    0x0\(%rip\),%rax        # 4b <_start\+0x4b>	47: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	48 8b 81 00 00 00 00 	mov    0x0\(%rcx\),%rax	4e: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	ff 15 00 00 00 00    	call   \*0x0\(%rip\)        # 58 <_start\+0x58>	54: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	ff 90 00 00 00 00    	call   \*0x0\(%rax\)	5a: R_X86_64_GOTPCREL	foo
[ 	]*[a-f0-9]+:	ff 25 00 00 00 00    	jmp    \*0x0\(%rip\)        # 64 <_start\+0x64>	60: R_X86_64_GOTPCREL	foo-0x4
[ 	]*[a-f0-9]+:	ff a1 00 00 00 00    	jmp    \*0x0\(%rcx\)	66: R_X86_64_GOTPCREL	foo
#pass
