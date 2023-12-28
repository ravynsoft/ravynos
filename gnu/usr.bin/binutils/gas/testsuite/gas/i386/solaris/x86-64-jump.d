#objdump: -drw
#source: ../x86-64-jump.s
#name: x86-64 jump

.*: +file format .*


Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	eb fe                	jmp    (0x0|0 <.text>)
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    0x7	3: R_X86_64_PC32	xxx-0x4
[ 	]*[a-f0-9]+:	ff 24 25 00 00 00 00 	jmp    \*0x0	a: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	ff e7                	jmp    \*%rdi
[ 	]*[a-f0-9]+:	ff 27                	jmp    \*\(%rdi\)
[ 	]*[a-f0-9]+:	ff 2c bd 00 00 00 00 	ljmp   \*0x0\(,%rdi,4\)	15: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	66 ff 2c bd 00 00 00 00 	ljmpw  \*0x0\(,%rdi,4\)	1d: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	ff 2c 25 00 00 00 00 	ljmp   \*0x0	24: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	66 ff 2c 25 00 00 00 00 	ljmpw  \*0x0	2c: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	e8 cb ff ff ff       	call   0x0
[ 	]*[a-f0-9]+:	e8 00 00 00 00       	call   0x3a	36: R_X86_64_PC32	xxx-0x4
[ 	]*[a-f0-9]+:	ff 14 25 00 00 00 00 	call   \*0x0	3d: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	ff d7                	call   \*%rdi
[ 	]*[a-f0-9]+:	ff 17                	call   \*\(%rdi\)
[ 	]*[a-f0-9]+:	ff 1c bd 00 00 00 00 	lcall  \*0x0\(,%rdi,4\)	48: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	66 ff 1c bd 00 00 00 00 	lcallw \*0x0\(,%rdi,4\)	50: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	ff 1c 25 00 00 00 00 	lcall  \*0x0	57: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	66 ff 1c 25 00 00 00 00 	lcallw \*0x0	5f: R_X86_64_32S	xxx
[ 	]*[a-f0-9]+:	67 e3 00             	jecxz  0x66	65: R_X86_64_PC8	\$\+0x2
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	e3 00                	jrcxz  0x69	68: R_X86_64_PC8	\$\+0x1
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	66 ff 13             	callw  \*\(%rbx\)
[ 	]*[a-f0-9]+:	66 ff 1b             	lcallw \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 1b                	lcall  \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 13                	call   \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 13                	call   \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 1b                	lcall  \*\(%rbx\)
[ 	]*[a-f0-9]+:	66 ff 23             	jmpw   \*\(%rbx\)
[ 	]*[a-f0-9]+:	66 ff 2b             	ljmpw  \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 2b                	ljmp   \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 23                	jmp    \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 23                	jmp    \*\(%rbx\)
[ 	]*[a-f0-9]+:	ff 2b                	ljmp   \*\(%rbx\)
[ 	]*[a-f0-9]+:	eb 00                	jmp    0x[0-9a-f]*
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	67 e3 00             	jecxz  0x[0-9a-f]*
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	e3 00                	jrcxz  0x[0-9a-f]*
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 00                	jmp    0x[0-9a-f]*
#pass
