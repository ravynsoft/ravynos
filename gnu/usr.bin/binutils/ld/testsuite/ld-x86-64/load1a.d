#source: load1.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -z max-page-size=0x200000 -z noseparate-code
#objdump: -dw --sym

.*: +file format .*

SYMBOL TABLE:
#...
0+60019e l     O .data	0+1 bar
#...
0+60019f g     O .data	0+1 foo
#...

Disassembly of section .text:

0+4000b0 <_start>:
[ 	]*[a-f0-9]+:	81 d0 9e 01 60 00    	adc    \$0x60019e,%eax
[ 	]*[a-f0-9]+:	81 c3 9e 01 60 00    	add    \$0x60019e,%ebx
[ 	]*[a-f0-9]+:	81 e1 9e 01 60 00    	and    \$0x60019e,%ecx
[ 	]*[a-f0-9]+:	81 fa 9e 01 60 00    	cmp    \$0x60019e,%edx
[ 	]*[a-f0-9]+:	81 ce 9e 01 60 00    	or     \$0x60019e,%esi
[ 	]*[a-f0-9]+:	81 df 9e 01 60 00    	sbb    \$0x60019e,%edi
[ 	]*[a-f0-9]+:	81 ed 9e 01 60 00    	sub    \$0x60019e,%ebp
[ 	]*[a-f0-9]+:	41 81 f0 9e 01 60 00 	xor    \$0x60019e,%r8d
[ 	]*[a-f0-9]+:	41 f7 c7 9e 01 60 00 	test   \$0x60019e,%r15d
[ 	]*[a-f0-9]+:	48 81 d0 9e 01 60 00 	adc    \$0x60019e,%rax
[ 	]*[a-f0-9]+:	48 81 c3 9e 01 60 00 	add    \$0x60019e,%rbx
[ 	]*[a-f0-9]+:	48 81 e1 9e 01 60 00 	and    \$0x60019e,%rcx
[ 	]*[a-f0-9]+:	48 81 fa 9e 01 60 00 	cmp    \$0x60019e,%rdx
[ 	]*[a-f0-9]+:	48 81 cf 9e 01 60 00 	or     \$0x60019e,%rdi
[ 	]*[a-f0-9]+:	48 81 de 9e 01 60 00 	sbb    \$0x60019e,%rsi
[ 	]*[a-f0-9]+:	48 81 ed 9e 01 60 00 	sub    \$0x60019e,%rbp
[ 	]*[a-f0-9]+:	49 81 f0 9e 01 60 00 	xor    \$0x60019e,%r8
[ 	]*[a-f0-9]+:	49 f7 c7 9e 01 60 00 	test   \$0x60019e,%r15
[ 	]*[a-f0-9]+:	81 d0 9f 01 60 00    	adc    \$0x60019f,%eax
[ 	]*[a-f0-9]+:	81 c3 9f 01 60 00    	add    \$0x60019f,%ebx
[ 	]*[a-f0-9]+:	81 e1 9f 01 60 00    	and    \$0x60019f,%ecx
[ 	]*[a-f0-9]+:	81 fa 9f 01 60 00    	cmp    \$0x60019f,%edx
[ 	]*[a-f0-9]+:	81 ce 9f 01 60 00    	or     \$0x60019f,%esi
[ 	]*[a-f0-9]+:	81 df 9f 01 60 00    	sbb    \$0x60019f,%edi
[ 	]*[a-f0-9]+:	81 ed 9f 01 60 00    	sub    \$0x60019f,%ebp
[ 	]*[a-f0-9]+:	41 81 f0 9f 01 60 00 	xor    \$0x60019f,%r8d
[ 	]*[a-f0-9]+:	41 f7 c7 9f 01 60 00 	test   \$0x60019f,%r15d
[ 	]*[a-f0-9]+:	48 81 d0 9f 01 60 00 	adc    \$0x60019f,%rax
[ 	]*[a-f0-9]+:	48 81 c3 9f 01 60 00 	add    \$0x60019f,%rbx
[ 	]*[a-f0-9]+:	48 81 e1 9f 01 60 00 	and    \$0x60019f,%rcx
[ 	]*[a-f0-9]+:	48 81 fa 9f 01 60 00 	cmp    \$0x60019f,%rdx
[ 	]*[a-f0-9]+:	48 81 cf 9f 01 60 00 	or     \$0x60019f,%rdi
[ 	]*[a-f0-9]+:	48 81 de 9f 01 60 00 	sbb    \$0x60019f,%rsi
[ 	]*[a-f0-9]+:	48 81 ed 9f 01 60 00 	sub    \$0x60019f,%rbp
[ 	]*[a-f0-9]+:	49 81 f0 9f 01 60 00 	xor    \$0x60019f,%r8
[ 	]*[a-f0-9]+:	49 f7 c7 9f 01 60 00 	test   \$0x60019f,%r15
#pass
